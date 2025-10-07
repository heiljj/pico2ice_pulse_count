from pexpect import fdpexpect
import os
import re

bitstream_path = ""
device_path = ""

with open(bitstream_path, "rb") as f:
    b = f.read()

if os.path.exists("log"):
    os.remove("log")
log = open("log", "wb")

txt = "".join(map(chr, b))

tty = os.open(device_path, os.O_RDWR)
p = fdpexpect.fdspawn(tty, timeout=15, logfile=log)

with open(device_path, "wb") as f:
    f.write(b)

p.send("DONEDONEDONEDONE")
p.expect("uploaded")
value = p.expect("pulses: [0-9]*")

with open("log", "r") as f:
    contents = f.read()
    print(f"pulses: {re.search("pulses: ([0-9]*)", contents).group(1)}")
os.close(tty)
log.close()

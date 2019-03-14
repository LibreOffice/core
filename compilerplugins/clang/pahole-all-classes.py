#!/usr/bin/python3
#
# Find holes in structures, so that we can pack them and improve our memory density.
#
# In order to make this work, you need to
# (1) first run the unusedfields loplugin to generate a log file
# (2) install the pahole stuff into your gdb, I used this one: https://github.com/PhilArmstrong/pahole-gdb
# (3) ./compilerplugins/clang/pahole-all-classes.py > ./compilerplugins/clang/pahole.results

import _thread
import io
import os
import subprocess
import time
import re

# search for all the class names in the file produced by the unusedfields loplugin
#a = subprocess.Popen("grep 'definition:' workdir/loplugin.unusedfields.log | sort -u", stdout=subprocess.PIPE, shell=True)
a = subprocess.Popen("cat n1", stdout=subprocess.PIPE, shell=True)

classSourceLocDict = dict()
classSet = set()
with a.stdout as txt:
    for line in txt:
        tokens = line.decode('utf8').strip().split("\t")
        className = tokens[2].strip()
        srcLoc = tokens[5].strip()
        if "anonymous" in className: continue
        # for now, just check the stuff in /sc/inc
        if not srcLoc.startswith("sc/inc/"):
                continue
        if className in classSet: continue
        classSourceLocDict[srcLoc] = className
        classSet.add(className)

gdbProc = subprocess.Popen("gdb", stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True)

stdin = io.TextIOWrapper(gdbProc.stdin, 'utf-8')

# make gdb load all the debugging info
stdin.write("set confirm off\n")
for filename in sorted(os.listdir('instdir/program')):
    if filename.endswith(".so"):
        stdin.write("add-symbol-file instdir/program/" + filename + "\n")
stdin.flush()


# Some of the pahole commands are going to fail, and I cannot read the error stream and the input stream
# together because python has no way of (easily) doing a non-blocking read.
# So I have to write the commands out using a background thread, and then read the entire resulting
# stream out below.
def write_pahole_commands():
    for srcLoc in sorted(classSourceLocDict.keys()):
        className = classSourceLocDict[srcLoc]
        stdin.write("echo " + className + " " + srcLoc + "\n")
        stdin.write("pahole " + className + "\n")
        stdin.flush()
    stdin.write("echo all-done\n")
    stdin.flush()
    stdin.close() # only way to make it flush the last echo command

_thread.start_new_thread( write_pahole_commands, () )

time.sleep(2)

# Use generator because lines often end up merged together in gdb's output, and we need
# to split them up, and that creates a mess in the parsing logic.
def read_generator():
    while True:
        line = gdbProc.stdout.readline().decode('utf8').strip()
        for split in line.split("(gdb)"):
            split = split.strip()
            if len(split) == 0: continue
            if "all-done" in split: return
            yield split

firstLineRegex = re.compile("/\*\s+(\d+)\s+\*/ struct")
holeLineRegex = re.compile("/\* XXX (\d+) bit hole, try to pack \*/")
structLines = list()
foundHole = False
cumulativeHoleBits = 0
structSize = 0
for line in read_generator():
    structLines.append(line)
    firstLineMatch = firstLineRegex.match(line)
    if firstLineMatch:
        structSize = int(firstLineMatch.group(1))
    holeLineMatch = holeLineRegex.match(line)
    if holeLineMatch:
        foundHole = True
        cumulativeHoleBits += int(holeLineMatch.group(1))
    if line == "}":
        # Ignore very large structs, packing those is not going to help much, and
        # re-organising them can make them much less readable.
        if foundHole and len(structLines) < 12 and structSize < 100:
            # I don't understand the rules here properly, but it appears that (on a 64-bit machine) struct sizes
            # have to be at least 4-byte aligned, so only show stuff that either results in changes of more than 4 bytes,
            # or results in a new size that is 4-byte aligned
            newStructSize = structSize - (cumulativeHoleBits / 8)
            if (cumulativeHoleBits / 8) >= 4 or newStructSize % 4 == 0:
                # print("Found one " + str(structSize) + " " + str(cumulativeHoleBits/8) + " " + str(newStructSize%4))
                for line in structLines:
                    print(line)
        structLines.clear()
        foundHole = False
        cumulativeHoleBits = 0
        structSize = 0

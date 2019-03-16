#!/usr/bin/python3
#
# Find holes in structures, so that we can pack them and improve our memory density.
#
# In order to make this work, you need to
# (1) Be operating in a workspace where you have a __NON-DEBUG__ build of LibreOffice, but __WITH SYMBOLS__.
#     (A debug build has different sizes for some things in the standard library.)
# (2) First run the unusedfields loplugin to generate a log file
# (3) Install the pahole stuff into your gdb, I used this one:
#     https://github.com/PhilArmstrong/pahole-gdb
# (4) Edit the loop near the top of the script to only produce results for one of our modules.
#     Note that this will make GDB soak up about 8G of RAM, which is why I don't do more than one module at a time
# (5) Run the script
#     ./compilerplugins/clang/pahole-all-classes.py > ./compilerplugins/clang/pahole.results
#

import _thread
import io
import os
import subprocess
import time
import re

# search for all the class names in the file produced by the unusedfields loplugin
#a = subprocess.Popen("grep 'definition:' workdir/loplugin.unusedfields.log | sort -u", stdout=subprocess.PIPE, shell=True)
a = subprocess.Popen("cat ../libo/n1", stdout=subprocess.PIPE, shell=True)

classSourceLocDict = dict()
classSet = set()
with a.stdout as txt:
    for line in txt:
        tokens = line.decode('utf8').strip().split("\t")
        className = tokens[2].strip()
        srcLoc = tokens[5].strip()
        # ignore things like unions
        if "anonymous" in className: continue
        # ignore duplicates
        if className in classSet: continue
        # for now, just check the stuff in /sc/inc
        if srcLoc.startswith("a"):
            classSourceLocDict[srcLoc] = className
            classSet.add(className)
a.terminate()

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
fieldLineRegex = re.compile("/\*\s+(\d+)\s+(\d+)\s+\*/ ")
holeLineRegex = re.compile("/\* XXX (\d+) bit hole, try to pack \*/")
# sometimes pahole can't determine the size of a sub-struct, and then it returns bad data
bogusLineRegex = re.compile("/\*\s+\d+\s+0\s+\*/")
structLines = list()
foundHole = False
cumulativeHoleBits = 0
structSize = 0
foundBogusLine = False
# pahole doesn't report space at the end of the structure, so work it out myself
sizeOfFields = 0
for line in read_generator():
    structLines.append(line)
    firstLineMatch = firstLineRegex.match(line)
    if firstLineMatch:
        structSize = int(firstLineMatch.group(1))
    holeLineMatch = holeLineRegex.match(line)
    if holeLineMatch:
        foundHole = True
        cumulativeHoleBits += int(holeLineMatch.group(1))
    fieldLineMatch = fieldLineRegex.match(line)
    if fieldLineMatch:
        fieldSize = int(fieldLineMatch.group(2))
        sizeOfFields = int(fieldLineMatch.group(1)) + fieldSize
    if bogusLineRegex.match(line):
        foundBogusLine = True
    if line == "}":
        # Ignore very large structs, packing those is not going to help much, and
        # re-organising them can make them much less readable.
        if foundHole and len(structLines) < 12 and structSize < 100 and not foundBogusLine:
            # Verify that we have enough hole-space that removing it will result in a structure
            # that still satisfies alignment requirements, otherwise the compiler will just put empty
            # space at the end of the struct.
            # TODO improve detection of the required alignment for a structure
            potentialSpace = (cumulativeHoleBits / 8) + (sizeOfFields - structSize)
            if potentialSpace >= 8:
                for line in structLines:
                    print(line)
                if (sizeOfFields - structSize) > 0:
                    print("hole at end of struct: " + str(sizeOfFields - structSize))
        #  reset state
        structLines.clear()
        foundHole = False
        cumulativeHoleBits = 0
        structSize = 0
        foundBogusLine = False
        actualStructSize = 0

gdbProc.terminate()

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
# (4) Run the script
#     ./compilerplugins/clang/pahole-all-classes.py
#

import _thread
import io
import os
import subprocess
import re

# search for all the class names in the file produced by the unusedfields loplugin
#a = subprocess.Popen("grep 'definition:' workdir/loplugin.unusedfields.log | sort -u", stdout=subprocess.PIPE, shell=True)
a = subprocess.Popen("cat n1", stdout=subprocess.PIPE, shell=True)

classSet = set()
classSourceLocDict = dict()
locToClassDict = dict()
with a.stdout as txt:
    for line in txt:
        tokens = line.decode('utf8').strip().split("\t")
        className = tokens[2].strip()
        srcLoc = tokens[5].strip()
        # ignore things like unions
        if "anonymous" in className: continue
        # ignore duplicates
        if className in classSet: continue
        classSet.add(className)
        classSourceLocDict[className] = srcLoc
        locToClassDict[srcLoc] = className
a.terminate()

# Some of the pahole commands are going to fail, and I cannot read the error stream and the input stream
# together because python has no way of (easily) doing a non-blocking read.
# So I have to write the commands out using a background thread, and then read the entire resulting
# stream out below.
def write_pahole_commands(classes):
    for className in classes:
        stdin.write("echo " + className + " " + classSourceLocDict[className] + "\n")
        stdin.write("pahole " + className + "\n")
        stdin.flush()
    stdin.write("echo all-done\n")
    stdin.flush()
    stdin.close() # only way to make it flush the last echo command

# Use generator because lines often end up merged together in gdb's output, and we need
# to split them up, and that creates a mess in the parsing logic.
def read_generator(gdbOutput):
    while True:
        line = gdbOutput.readline()
        if line == "": return # end of file
        line = line.decode('utf8').strip()
        print("gdb: " + line)
        for split in line.split("(gdb)"):
            split = split.strip()
            if len(split) == 0: continue
            if "all-done" in split: return
            yield split

# build list of classes sorted by source location to increase the chances of
# processing stuff stored in the same DSO together
sortedLocs = sorted(locToClassDict.keys())
classList = list()
for src in sortedLocs:
    if "/inc/" in src or "include/" in src:
        classList.append(locToClassDict[src])

with open("compilerplugins/clang/pahole.results", "wt") as f:
    # Process 400 classes at a time, otherwise gdb's memory usage blows up and kills the machine
    # This number is chosen to make gdb peak at around 8G.
    while len(classList) > 0:

        currClassList = classList[0:500]
        classList = classList[500:]

        gdbProc = subprocess.Popen("gdb", stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True)

        stdin = io.TextIOWrapper(gdbProc.stdin, 'utf-8')

        # make gdb load all the debugging info
        stdin.write("set confirm off\n")
        # make gdb not wrap output and mess up my parsing
        stdin.write("set width unlimited\n")
        for filename in sorted(os.listdir('instdir/program')):
            if filename.endswith(".so"):
                stdin.write("add-symbol-file instdir/program/" + filename + "\n")
        stdin.flush()


        _thread.start_new_thread( write_pahole_commands, (currClassList,) )

        firstLineRegex = re.compile(r"/\*\s+(\d+)\s+\*/ struct") # /* 16 */ struct Foo
        fieldLineRegex = re.compile(r"/\*\s+(\d+)\s+(\d+)\s+\*/ ") # /* 12 8 */ class rtl::OUString aName
        holeLineRegex = re.compile(r"/\* XXX (\d+) bit hole, try to pack \*/")
        # sometimes pahole can't determine the size of a sub-struct, and then it returns bad data
        bogusLineRegex = re.compile(r"/\*\s+\d+\s+0\s+\*/")
        structLines = list()
        foundHole = False
        cumulativeHoleBits = 0
        alignedStructSize = 0
        foundBogusLine = False
        # pahole doesn't report space at the end of the structure, so work it out myself
        sizeOfStructWithoutPadding = 0
        for line in read_generator(gdbProc.stdout):
            structLines.append(line)
            firstLineMatch = firstLineRegex.match(line)
            if firstLineMatch:
                alignedStructSize = int(firstLineMatch.group(1))
                structLines.clear()
                structLines.append(line)
            holeLineMatch = holeLineRegex.match(line)
            if holeLineMatch:
                foundHole = True
                cumulativeHoleBits += int(holeLineMatch.group(1))
            fieldLineMatch = fieldLineRegex.match(line)
            if fieldLineMatch:
                fieldPosInBytes = int(fieldLineMatch.group(1))
                fieldSizeInBytes = int(fieldLineMatch.group(2))
                sizeOfStructWithoutPadding = fieldPosInBytes + fieldSizeInBytes
            if bogusLineRegex.match(line):
                foundBogusLine = True
            if line == "}":
                # Ignore very large structs, packing those is not going to help much, and
                # re-organising them can make them much less readable.
                if foundHole and len(structLines) < 16 and alignedStructSize < 100 and not foundBogusLine:
                    # Verify that, after packing, and compiler alignment, the new structure will be actually smaller.
                    # Sometimes, we can save space, but the compiler will align the structure such that we don't
                    # actually save any space.
                    # TODO improve detection of the required alignment for a structure
                    holeAtEnd = alignedStructSize - sizeOfStructWithoutPadding
                    potentialSpace = (cumulativeHoleBits / 8) + holeAtEnd
                    if potentialSpace >= 8:
                        for line in structLines:
                            f.write(line + "\n")
                        if holeAtEnd > 0:
                            f.write("hole at end of struct: " + str(holeAtEnd) + "\n")
                        f.write("\n")
                #  reset state
                structLines.clear()
                foundHole = False
                cumulativeHoleBits = 0
                structSize = 0
                foundBogusLine = False
                actualStructSize = 0

        gdbProc.terminate()

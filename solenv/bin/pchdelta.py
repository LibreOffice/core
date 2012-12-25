#!/usr/bin/env python
# *************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
# *************************************************************


# ------------------------------------------------------------------------------
# Hacky little delta debug tool to figure out the proper includes for a pch file
#
# Usage:
#
# pchdelta.py <pch_target> <dir1> [<dir2> <dir3> ...]
#
# <pch_target>      File to perform delta debugging on. The section to test
#                   is delimeted by '//---MARKER---' lines.
# <dir1> .. <dirn>  Sequence of directories to run dmake in to test if the
#                   modification works
#
# Examples:
#
# pchdelta.py inc/pch/precompiled_sfx2.hxx inc source/dialog
#
#  Run pchdelta inside sfx2 first building the pch files and then files in
# source/dialog
#
# ------------------------------------------------------------------------------

import os
import os.path
import sys

# C++
MARKER="//---MARKER---\n"

# dmake
#MARKER="#---MARKER---\n"

# ------------------------------------------------------------------------------
# Sequentially build all argument directories from scratch

def testSequenceBuild(dirlist):
    cwd = os.path.abspath(os.getcwd())
    for path in dirlist:
        os.chdir(path)
        buildcommand = "dmake -u"
        buildcommand += " >>" + cwd + "/buildlog.txt 2>&1"
        buildresult = os.system(buildcommand)
        os.chdir(cwd)
        if buildresult != 0:
            return False
    return True

# ------------------------------------------------------------------------------
# Dump out the delta file with corresponding markers

def writePch(pchname, header, footer, acceptedlines, testlines):
    outputfile = file(pchname, "w")
    outputfile.write(header)
    outputfile.write(MARKER)
    outputfile.write("\n".join(acceptedlines))
    if len(testlines) > 0:
        outputfile.write("\n\n//---Candidate marker---\n")
        outputfile.write("\n".join(testlines) + "\n")
        outputfile.write("//---Candidate marker end---\n")
    outputfile.write(MARKER)
    outputfile.write(footer)
    outputfile.close()


# ------------------------------------------------------------------------------
# Recursive tester routine. Test the segment given and if an error is
# encountered splits the segment into <fanout> subsegment and recurses. Failing
# one liners are rejected. The set of accepted lines are built sequentially from
# the beginning.

def binaryTest(dirlist, lines, pchname, header, footer, acceptedlines, indent, startpoint):
    linecount = len(lines)
    if linecount == 0:
        return
    # Test if this slice passes the buildtest
    writePch(pchname, header, footer, acceptedlines, lines)
    if testSequenceBuild(dirlist):
        return acceptedlines + lines

    # Reject one liners
    if linecount == 1:
        print(indent + "Rejected: " + lines[0])
        return acceptedlines

    # Recurse with multiline slices
    fanout = 4
    splits = []
    for i in range(3):
        splits.append(linecount * (i + 1) / fanout)
    splits.append(linecount)

    splitstart = 0
    for splitend in splits:
        # avoid splitting in case we have no resulting lines
        if (splitend - splitstart) == 0:
            continue
        splitslice = lines[splitstart:splitend]
        print(indent + "[" + str(startpoint + splitstart) + ":" + str(startpoint + splitend) + "] (" + str(splitend - splitstart) + ")")
        acceptedlines = binaryTest(dirlist, splitslice, pchname, header, footer, acceptedlines, indent + " ", startpoint + splitstart)
        splitstart = splitend

    return acceptedlines

# ------------------------------------------------------------------------------
# Main entry point

if len(sys.argv) < 3:
    print("Usage: " + sys.argv[0] + " <pch_target> <dir1> [<dir2> <dir3> ...]")
    sys.exit(1)

pchname = os.path.abspath(sys.argv[1])
dirlist = sys.argv[2:]

# remove old build log file
if os.path.exists("buildlog.txt"):
    os.remove("buildlog.txt")

# test for corner case of everything working from the start
if testSequenceBuild(dirlist):
    print("pch working, nothing to do.")
    sys.exit(0)

# Open the header file for reading
inputfile = file(pchname, "r+")
inputdata = inputfile.read()
inputfile.close()

segments = inputdata.split(MARKER)
header = segments[0]
footer = segments[2]
lines = segments[1].split("\n")

writePch(pchname + "_backup", header, footer, lines, [])

# test for corner case of no convergence possible
writePch(pchname, header, footer, [], [])
if not testSequenceBuild(dirlist):
    writePch(pchname, header, footer, lines, [])
    print("Building with no candidate lines failed. Convergence questionable, aborting.")
    sys.exit(0)

# Starting pruning
print("Starting evaluation of " + str(len(lines)) + " lines")
acceptedlines = binaryTest(dirlist, lines, pchname, header, footer, [], "", 0)
writePch(pchname, header, footer, acceptedlines, [])

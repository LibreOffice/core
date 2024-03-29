#!/usr/bin/python3
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

### script to help debug leaks of reference counted objects

## I. to use it, first override acquire() and release()

# Foo * g_pTrackedFoo = 0;

# Foo::Foo()
#   static int nFoos = 0;
#     if (++nFoos == 42) // track instance #42
#       g_pTrackedFoo = this;

# void Foo::acquire()
#   if (this == g_pTrackedFoo)
#     ; // set gdb breakpoint here
#   Foo_Base::acquire()

# void Foo::release()
#   if (this == g_pTrackedFoo)
#      ; // set gdb breakpoint here
#   Foo_Base::release()

## II. run test/soffice in gdb and set breakpoints in acquire/release
##     with a command to print the backtrace

# set logging enabled on
# break foo.cxx:123
# break foo.cxx:234

# command 1 2
# bt
# c
# end
# run

## III. now feed logfile gdb.txt into this script

# bin/refcount_leak.py < gdb.txt

###

from operator import itemgetter
import re
import sys

threshold = 2

class Trace:
    clock = 0 # global counter
    # frames: list of stack frames, beginning with outermost
    def __init__(self, lines):
        lines.reverse()
        self.frames = lines
        Trace.clock += 1
        self.clock = Trace.clock

def addTrace(traces, lines):
    if not(traces is None) and len(lines) > 0:
        traces.append(Trace(lines))

def readGdbLog(infile):
    traces_acquire = []
    traces_release = []
    current = None
    lines = []
    apattern = re.compile("^Breakpoint.*::acquire")
    rpattern = re.compile("^Breakpoint.*::release")
    for line in infile:
        if apattern.match(line):
            addTrace(current, lines)
            lines = []
            current = traces_acquire
        if rpattern.match(line):
            addTrace(current, lines)
            lines = []
            current = traces_release
        if line.startswith("#"):
            # strip #123 stack frame number, and newline
            lines.append(line[line.index("0x"):-1])
    addTrace(current, lines)
    print("# parsed traces acquire: ", len(traces_acquire))
    print("# parsed traces release: ", len(traces_release))
    return (traces_acquire, traces_release)

def getFunction(frame):
    start = frame.index(" in ") + len(" in ")
    try:
        end = frame.index(" at ", start)
    except ValueError:
        # argh... stack frames may be split across multiple lines if
        # a parameter has a fancy pretty printer
        return frame[start:]
    return frame[start:end]


def matchStack(trace_acquire, trace_release):
    if trace_release.clock < trace_acquire.clock:
        return None # acquire must precede release
    common = 0
    refpattern = re.compile(r"::Reference<.*>::Reference\(")
    for (frame1, frame2) in zip(trace_release.frames, trace_acquire.frames):
        if frame1 == frame2:
            common += 1
        else:
            if getFunction(frame1) == getFunction(frame2):
                common += 1
            acquireframes = len(trace_acquire.frames)
            # there is sometimes a dozen frames of UNO type related junk
            # on the stack where the acquire() happens, which breaks the
            # matching; try to avoid that
            for i in range(common, acquireframes):
                if refpattern.search(trace_acquire.frames[i]):
                    acquireframes = i+1 # cut off junk above Reference ctor
                    break
            score = max(len(trace_release.frames), acquireframes) - common
            # smaller score is better
            return (score, trace_release.clock - trace_acquire.clock)

# brute force greedy n^2 matching
def matchStacks(traces_acquire, traces_release):
    matches = []
    for release in traces_release:
        for acquire in traces_acquire:
            score = matchStack(acquire, release)
            if score is not None:
                matches.append((score, acquire, release))
    matches.sort(key=itemgetter(0))
    return matches

def bestMatches(traces_acquire, traces_release, matches):
    traces_aunmatched = traces_acquire
    traces_runmatched = traces_release
    bestmatches = []
    for (score,acquire,release) in matches:
        if not(acquire in traces_aunmatched and release in traces_runmatched):
            continue
        traces_aunmatched.remove(acquire)
        traces_runmatched.remove(release)
        bestmatches.append((score,acquire,release))
    print("# unmatched acquire: ", len(traces_aunmatched))
    print("# unmatched release: ", len(traces_runmatched))
    return (bestmatches,traces_aunmatched,traces_runmatched)

def printTrace(trace):
    for frame in reversed(trace.frames):
        print("  ", frame)

def printMatched(bestmatches):
    for (score,acquire,release) in reversed(bestmatches):
        print("\n*** Matched trace with score: ", score)
        print(" acquire: ")
        printTrace(acquire)
        print(" release: ")
        printTrace(release)

def printUnmatched(traces, prefix):
    for trace in traces:
        print("\n*** Unmatched trace (", prefix, "):")
        printTrace(trace)

if __name__ == "__main__":
    (traces_acquire, traces_release) = readGdbLog(sys.stdin)
    matches = matchStacks(traces_acquire, traces_release)
    (bestmatches,traces_au,traces_ru) = bestMatches(traces_acquire, traces_release, matches)
    # print output, sorted with the most suspicious stuff first:
    printUnmatched(traces_au, "acquire")
    printUnmatched(traces_ru, "release")
    printMatched(bestmatches)

# vim:set shiftwidth=4 softtabstop=4 expandtab:

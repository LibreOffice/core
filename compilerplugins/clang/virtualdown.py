#!/usr/bin/python

import io
import re
import sys

definitionSet = set()
definitionToSourceLocationMap = dict()
callSet = set()


with io.open("workdir/loplugin.virtualdown.log", "rb", buffering=1024*1024) as txt:
    for line in txt:
        tokens = line.strip().split("\t")
        if tokens[0] == "definition:":
            fullMethodName = tokens[1]
            sourceLocation = tokens[2]
            definitionSet.add(fullMethodName)
            definitionToSourceLocationMap[fullMethodName] = sourceLocation
        elif tokens[0] == "call:":
            fullMethodName = tokens[1]
            callSet.add(fullMethodName)
        else:
            print( "unknown line: " + line)

unnecessaryVirtualSet = set()

for clazz in (definitionSet - callSet):
#    if clazz.startswith("canvas::"): continue
#    if clazz == "basegfx::unotools::UnoPolyPolygon::void-modifying()const": continue
    # ignore external code
    if definitionToSourceLocationMap[clazz].startswith("external/"): continue

    unnecessaryVirtualSet.add((clazz,definitionToSourceLocationMap[clazz] ))


# sort the results using a "natural order" so sequences like [item1,item2,item10] sort nicely
def natural_sort_key(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]
# sort by both the source-line and the datatype, so the output file ordering is stable
# when we have multiple items on the same source line
def v_sort_key(v):
    return natural_sort_key(v[1]) + [v[0]]

# sort results by name and line number
tmp1list = sorted(unnecessaryVirtualSet, key=lambda v: v_sort_key(v))

with open("compilerplugins/clang/virtualdown.results", "wt") as f:
    for t in tmp1list:
        f.write( t[1] + "\n" )
        f.write( "    " + t[0] + "\n" )
    # add an empty line at the end to make it easier for the removevirtuals plugin to mmap() the output file
    f.write("\n")


#!/usr/bin/python

import sys
import re
import io

callDict = dict() # callInfo tuple -> callValue

# clang does not always use exactly the same numbers in the type-parameter vars it generates
# so I need to substitute them to ensure we can match correctly.
normalizeTypeParamsRegex = re.compile(r"type-parameter-\d+-\d+")
def normalizeTypeParams( line ):
    return normalizeTypeParamsRegex.sub("type-parameter-?-?", line)

# reading as binary (since we known it is pure ascii) is much faster than reading as unicode
with io.open("workdir/loplugin.virtualdead.log", "rb", buffering=1024*1024) as txt:
    for line in txt:
        try:
            tokens = line.strip().split("\t")
            nameAndParams = normalizeTypeParams(tokens[1])
            sourceLocation = tokens[2]
            returnValue = tokens[3]
            callInfo = (nameAndParams, sourceLocation)
            if not callInfo in callDict:
                callDict[callInfo] = set()
            callDict[callInfo].add(returnValue)
        except IndexError:
            print "problem with line " + line.strip()
            raise

tmp1list = list()
for callInfo, callValues in callDict.iteritems():
    nameAndParams = callInfo[1]
    if len(callValues) != 1:
        continue
    callValue = next(iter(callValues))
    if "unknown-stmt" in callValue:
        continue
    if "unknown3" in callValue:
        continue
    if "unknown4" in callValue:
        continue
    if "pure" in callValue:
        continue
    sourceLoc = callInfo[1]
    if sourceLoc.startswith("workdir/"):
        continue
    functionSig = callInfo[0]
    tmp1list.append((sourceLoc, functionSig, callValue))


# sort results by filename:lineno
def natural_sort_key(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]
tmp1list.sort(key=lambda v: natural_sort_key(v[0]))

# print out the results
with open("compilerplugins/clang/virtualdead.results", "wt") as f:
    for v in tmp1list:
        f.write(v[0] + "\n")
        f.write("    " + v[1] + "\n")
        f.write("    " + v[2] + "\n")

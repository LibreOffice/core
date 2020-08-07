#!/usr/bin/python2

import sys
import re
import io

callDict = dict() # callInfo tuple -> callValue
definitionToSourceLocationMap = dict()
paramSet = set() # paraminfo tuple

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
            if tokens[0] == "virtual:":
                nameAndParams = normalizeTypeParams(tokens[1])
                sourceLocation = tokens[2]
                returnValue = tokens[3]
                callInfo = (nameAndParams, sourceLocation)
                if not callInfo in callDict:
                    callDict[callInfo] = set()
                callDict[callInfo].add(returnValue)
                definitionToSourceLocationMap[nameAndParams] = sourceLocation
            elif tokens[0] == "param:":
                name = normalizeTypeParams(tokens[1])
                if len(tokens)>2:
                    bitfield = tokens[2]
                    paramSet.add((name,bitfield))
            else:
                print( "unknown line: " + line)
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
    if "unknown2" in callValue:
        continue
    if "unknown3" in callValue:
        continue
    if "unknown4" in callValue:
        continue
    if "pure" in callValue:
        continue
    srcloc = callInfo[1]
    if srcloc.startswith("workdir/"): continue
    # ignore Qt stuff
    if srcloc.startswith("Gui/"): continue
    if srcloc.startswith("Widgets/"): continue
    if srcloc.startswith("Core/"): continue
    if srcloc.startswith("/Qt"): continue
    functionSig = callInfo[0]
    tmp1list.append((srcloc, functionSig, callValue))

def merge_bitfield(a, b):
    if len(a) == 0: return b
    ret = ""
    for i, c in enumerate(b):
        if c == "1" or a[i] == "1":
            ret += "1"
        else:
            ret += "0"
    return ret;
tmp2dict = dict()
tmp2list = list()
for paramInfo in paramSet:
    name = paramInfo[0]
    bitfield = paramInfo[1]
    if re.match( r"\w+ com::", name): continue
    if re.match( r"\w+ ooo::vba::", name): continue
    if re.match( r"\w+ orcus::", name): continue
    if re.match( r"\w+ std::", name): continue
    if not name in tmp2dict:
        tmp2dict[name] = bitfield
    else:
        tmp2dict[name] = merge_bitfield(tmp2dict[name], bitfield)
for name, bitfield in tmp2dict.iteritems():
    srcloc = definitionToSourceLocationMap[name]
    # ignore Qt stuff
    if srcloc.startswith("Gui/"): continue
    if srcloc.startswith("Widgets/"): continue
    if srcloc.startswith("Core/"): continue
    if srcloc.startswith("/Qt"): continue
    # ignore external stuff
    if srcloc.startswith("workdir/"): continue
    # referenced by generated code in workdir/
    if srcloc.startswith("writerfilter/source/ooxml/OOXMLFactory.hxx"): continue
    if "0" in bitfield:
        tmp2list.append((srcloc, name, bitfield))

# sort results by filename:lineno
def natural_sort_key(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]
tmp1list.sort(key=lambda v: natural_sort_key(v[0]))
tmp2list.sort(key=lambda v: natural_sort_key(v[0]))

# print out the results
with open("compilerplugins/clang/virtualdead.results", "wt") as f:
    for v in tmp1list:
        f.write(v[0] + "\n")
        f.write("    " + v[1] + "\n")
        f.write("    " + v[2] + "\n")
with open("compilerplugins/clang/virtualdead.unusedparams.results", "wt") as f:
    for v in tmp2list:
        f.write(v[0] + "\n")
        f.write("    " + v[1] + "\n")
        f.write("    " + v[2] + "\n")

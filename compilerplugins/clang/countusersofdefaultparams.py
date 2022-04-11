#!/usr/bin/python

import sys
import re
import io

definitionToSourceLocationMap = dict()
callDict = dict()

# clang does not always use exactly the same numbers in the type-parameter vars it generates
# so I need to substitute them to ensure we can match correctly.
normalizeTypeParamsRegex1 = re.compile(r"type-parameter-\d+-\d+")
normalizeTypeParamsRegex2 = re.compile(r"typename enable_if<.*")
def normalizeTypeParams( line ):
    line = normalizeTypeParamsRegex1.sub("type-parameter-?-?", line)
    return normalizeTypeParamsRegex2.sub("type-parameter-?-?", line)

with io.open("workdir/loplugin.countusersofdefaultparams.log", "rb", buffering=1024*1024) as txt:
    for line in txt:
        tokens = line.strip().split("\t")
        if tokens[0] == "defn:":
            access = tokens[1]
            returnType = tokens[2]
            nameAndParams = tokens[3]
            sourceLocation = tokens[4]
            funcInfo = normalizeTypeParams(returnType) + " " + normalizeTypeParams(nameAndParams)
            definitionToSourceLocationMap[funcInfo] = sourceLocation
            if not funcInfo in callDict:
                callDict[funcInfo] = set()
        elif tokens[0] == "call:":
            returnType = tokens[1]
            nameAndParams = tokens[2]
            sourceLocationOfCall = tokens[3]
            funcInfo = normalizeTypeParams(returnType) + " " + normalizeTypeParams(nameAndParams)
            if not funcInfo in callDict:
                callDict[funcInfo] = set()
            callDict[funcInfo].add(sourceLocationOfCall)
        else:
            print( "unknown line: " + line)

tmp1list = list()
for k,v in callDict.iteritems():
    if len(v) >= 1:
        continue
    # created by macros
    if k.endswith("::RegisterInterface(class SfxModule *)"):
        continue
    if k.endswith("::RegisterChildWindow(_Bool,class SfxModule *,enum SfxChildWindowFlags)"):
        continue
    if k.endswith("::RegisterControl(unsigned short,class SfxModule *)"):
        continue
    if k.endswith("::RegisterFactory(unsigned short)"):
        continue
    # windows-only stuff
    if "ShutdownIcon::OpenURL" in k:
        continue
    # template magic
    if k.startswith("void VclPtr::VclPtr(const VclPtr<type-parameter-?-?> &,typename UpCast<"):
        continue
    if k in definitionToSourceLocationMap:
        tmp1list.append((k, definitionToSourceLocationMap[k]))

# sort the results using a "natural order" so sequences like [item1,item2,item10] sort nicely
def natural_sort_key(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]
# sort by both the source-line and the datatype, so the output file ordering is stable
# when we have multiple items on the same source line
def v_sort_key(v):
    return natural_sort_key(v[1]) + [v[0]]

# sort results by name and line number
tmp1list.sort(key=lambda v: v_sort_key(v))

# print out the results
with open("loplugin.countusersofdefaultparams.report", "wt") as f:
    for t in tmp1list:
        f.write(t[1] + "\n")
        f.write("    " + t[0] + "\n")



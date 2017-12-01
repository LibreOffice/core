#!/usr/bin/python

import sys
import re
import io

definitionToSourceLocationMap = dict() # dict of tuple(parentClass, fieldName) to sourceLocation
fieldAssignDict = dict() # dict of tuple(parentClass, fieldName) to (set of values)

# clang does not always use exactly the same numbers in the type-parameter vars it generates
# so I need to substitute them to ensure we can match correctly.
normalizeTypeParamsRegex = re.compile(r"type-parameter-\d+-\d+")
def normalizeTypeParams( line ):
    return normalizeTypeParamsRegex.sub("type-parameter-?-?", line)

# reading as binary (since we known it is pure ascii) is much faster than reading as unicode
with io.open("loplugin.singlevalfields.log", "rb", buffering=1024*1024) as txt:
    for line in txt:
        tokens = line.strip().split("\t")
        if tokens[0] == "defn:":
            parentClass = normalizeTypeParams(tokens[1])
            fieldName = normalizeTypeParams(tokens[2])
            sourceLocation = tokens[3]
            fieldInfo = (parentClass, fieldName)
            definitionToSourceLocationMap[fieldInfo] = sourceLocation
        elif tokens[0] == "asgn:":
            parentClass = normalizeTypeParams(tokens[1])
            fieldName = normalizeTypeParams(tokens[2])
            if len(tokens) > 3:
                assignValue = tokens[3]
            else:
                assignValue = ""
            fieldInfo = (parentClass, fieldName)
            if not fieldInfo in fieldAssignDict:
                fieldAssignDict[fieldInfo] = set()
            fieldAssignDict[fieldInfo].add(assignValue)

tmp1list = list()
for fieldInfo, assignValues in fieldAssignDict.iteritems():
    v0 = fieldInfo[0] + " " + fieldInfo[1]
    v1 = (",".join(assignValues))
    v2 = ""
    if fieldInfo not in definitionToSourceLocationMap:
        continue
    v2 = definitionToSourceLocationMap[fieldInfo]
    if len(assignValues) != 1:
        continue
    if "?" in assignValues:
        continue
    #if len(assignValues - set(["0", "1", "-1", "nullptr"])) > 0:
    #    continue
    # ignore things which are locally declared but are actually redeclarations of things from 3rd party code
    containingClass = fieldInfo[0]
    if containingClass == "_mwmhints":
        continue
    # ignore things which are representations of on-disk structures
    if containingClass in ["SEPr", "WW8Dop", "BmpInfoHeader", "BmpFileHeader", "Exif::ExifIFD",
            "sw::WW8FFData", "FFDataHeader", "INetURLHistory_Impl::head_entry", "ImplPPTParaPropSet", "SvxSwAutoFormatFlags",
            "T602ImportFilter::T602ImportFilter::format602struct", "DataNode"]:
        continue
    if v2.startswith("hwpfilter/source"):
        continue
    # ignore things which are representations of structures from external code
    if v2.startswith("desktop/unx/source/splashx.c"):
        continue
    # Windows-only
    if containingClass in ["SfxAppData_Impl", "sfx2::ImplDdeItem", "SvFileStream",
            "DdeService", "DdeTopic", "DdeItem", "DdeConnection", "connectivity::sdbcx::OUser", "connectivity::sdbcx::OGroup", "connectivity::sdbcx::OCatalog",
            "cairocanvas::SpriteHelper"]:
        continue
    if v2.startswith("include/svl/svdde.hxx") or v2.startswith("embeddedobj/source/inc/oleembobj.hxx"):
        continue
    # Some of our supported compilers don't do constexpr, which means o3tl::typed_flags can't be 'static const'
    if containingClass in ["WaitWindow_Impl"]:
        continue
    tmp1list.append((v0,v1,v2))

# sort results by filename:lineno
def natural_sort_key(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]
tmp1list.sort(key=lambda v: natural_sort_key(v[2]))

# print out the results
with open("compilerplugins/clang/singlevalfields.results", "wt") as f:
    for v in tmp1list:
        f.write(v[2] + "\n")
        f.write("    " + v[0] + "\n")
        f.write("    " + v[1] + "\n")



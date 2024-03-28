#!/usr/bin/python3

import re
import io

definitionToSourceLocationMap = dict() # dict of tuple(parentClass, fieldName) to sourceLocation
definitionToTypeMap = dict() # dict of tuple(parentClass, fieldName) to field type
fieldAssignDict = dict() # dict of tuple(parentClass, fieldName) to (set of values)

# clang does not always use exactly the same numbers in the type-parameter vars it generates
# so I need to substitute them to ensure we can match correctly.
normalizeTypeParamsRegex = re.compile(r"type-parameter-\d+-\d+")
def normalizeTypeParams( line ):
    return normalizeTypeParamsRegex.sub("type-parameter-?-?", line)

# reading as binary (since we known it is pure ascii) is much faster than reading as unicode
with io.open("workdir/loplugin.singlevalfields.log", "r", buffering=1024*1024) as txt:
    for line in txt:
        tokens = line.strip().split("\t")
        if tokens[0] == "defn:":
            parentClass = normalizeTypeParams(tokens[1])
            fieldName = normalizeTypeParams(tokens[2])
            fieldType = normalizeTypeParams(tokens[3])
            srcLoc = tokens[4]
            # ignore some external stuff that is somehow sneaking through
            if not(srcLoc.startswith("workdir/") or srcLoc.startswith("-3.0/") or srcLoc.startswith("_64-linux-gnu/")):
                fieldInfo = (parentClass, fieldName)
                definitionToSourceLocationMap[fieldInfo] = srcLoc
                definitionToTypeMap[fieldInfo] = fieldType
        elif tokens[0] == "asgn:":
            parentClass = normalizeTypeParams(tokens[1])
            fieldName = normalizeTypeParams(tokens[2])
            if len(tokens) > 3:
                assignValue = tokens[3]
            else:
                assignValue = ""
            fieldInfo = (parentClass, fieldName)
            if fieldInfo not in fieldAssignDict:
                fieldAssignDict[fieldInfo] = set()
            fieldAssignDict[fieldInfo].add(assignValue)
        else:
            print( "unknown line: " + line)

# look for stuff also has a single value
tmp1list = list()
# look for things which have two values - zero and one
tmp2list = list()
for fieldInfo, assignValues in fieldAssignDict.items():
    v0 = fieldInfo[0] + " " + fieldInfo[1]
    v1 = (",".join(assignValues))
    v2 = ""
    if fieldInfo not in definitionToSourceLocationMap:
        continue
    v2 = definitionToSourceLocationMap[fieldInfo]
    if len(assignValues) > 2:
        continue
    if "?" in assignValues:
        continue
    # ignore some random noise, no idea why this does not get filtered out by the normal checks in the C++
    if v2.startswith("ux-gnu/") or v2.startswith(":") or v2.startswith(".h:") or v2.startswith("bxml/"):
        continue
    if v2.startswith("ib.h:") or v2.startswith("freetype/") or v2.startswith("k/") or v2.startswith("n.h"):
        continue
    if v2.startswith("pango/") or v2.startswith("t.h") or v2.startswith("h:"):
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
    if len(assignValues) == 2:
        if "0" in assignValues and "1" in assignValues:
            fieldType = definitionToTypeMap[fieldInfo]
            if "_Bool" not in fieldType and "enum " not in fieldType and "boolean" not in fieldType:
                tmp2list.append((v0,v1,v2,fieldType))
    elif len(assignValues) == 1:
        # ignore timers/idles
        if not("Idle" in v1 or "Timer" in v1):
            tmp1list.append((v0,v1,v2))
    else:
        tmp1list.append((v0,v1,v2))

# sort results by filename:lineno
def natural_sort_key(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]
tmp1list.sort(key=lambda v: natural_sort_key(v[2]))
tmp2list.sort(key=lambda v: natural_sort_key(v[2]))

# print out the results
with open("compilerplugins/clang/singlevalfields.results", "wt") as f:
    for v in tmp1list:
        f.write(v[2] + "\n")
        f.write("    " + v[0] + "\n")
        f.write("    " + v[1] + "\n")
with open("compilerplugins/clang/singlevalfields.could-be-bool.results", "wt") as f:
    for v in tmp2list:
        f.write(v[2] + "\n")
        f.write("    " + v[0] + "\n")
        f.write("    " + v[3] + "\n")



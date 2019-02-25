#!/usr/bin/python

import sys
import re
import io

definitionSet = set()
protectedAndPublicDefinitionSet = set() # set of tuple(type, name)
definitionToSourceLocationMap = dict()
definitionToTypeMap = dict()
touchedFromInsideSet = set()
touchedFromOutsideSet = set()
touchedFromOutsideConstructorSet = set()
readFromSet = set()
writeToSet = set()
sourceLocationSet = set()

# clang does not always use exactly the same numbers in the type-parameter vars it generates
# so I need to substitute them to ensure we can match correctly.
normalizeTypeParamsRegex = re.compile(r"type-parameter-\d+-\d+")
def normalizeTypeParams( line ):
    return normalizeTypeParamsRegex.sub("type-parameter-?-?", line)

def parseFieldInfo( tokens ):
    if len(tokens) == 3:
        return (normalizeTypeParams(tokens[1]), tokens[2])
    else:
        return (normalizeTypeParams(tokens[1]), "")

with io.open("workdir/loplugin.unusedfields.log", "rb", buffering=1024*1024) as txt:
    for line in txt:
        tokens = line.strip().split("\t")
        if tokens[0] == "definition:":
            access = tokens[1]
            fieldInfo = (normalizeTypeParams(tokens[2]), tokens[3])
            srcLoc = tokens[5]
            # ignore external source code
            if (srcLoc.startswith("external/")):
                continue
            # ignore build folder
            if (srcLoc.startswith("workdir/")):
                continue
            definitionSet.add(fieldInfo)
            definitionToTypeMap[fieldInfo] = tokens[4]
            if access == "protected" or access == "public":
                protectedAndPublicDefinitionSet.add(fieldInfo)
            definitionToSourceLocationMap[fieldInfo] = tokens[5]
        elif tokens[0] == "inside:":
            touchedFromInsideSet.add(parseFieldInfo(tokens))
        elif tokens[0] == "outside:":
            touchedFromOutsideSet.add(parseFieldInfo(tokens))
        elif tokens[0] == "outside-constructor:":
            touchedFromOutsideConstructorSet.add(parseFieldInfo(tokens))
        elif tokens[0] == "read:":
            readFromSet.add(parseFieldInfo(tokens))
        elif tokens[0] == "write:":
            writeToSet.add(parseFieldInfo(tokens))
        else:
            print( "unknown line: " + line)

# Calculate untouched
untouchedSet = set()
untouchedSetD = set()
for d in definitionSet:
    if d in touchedFromOutsideSet or d in touchedFromInsideSet:
        continue
    srcLoc = definitionToSourceLocationMap[d];
    # this is all representations of on-disk data structures
    if (srcLoc.startswith("sc/source/filter/inc/scflt.hxx")
        or srcLoc.startswith("sw/source/filter/ww8/")
        or srcLoc.startswith("vcl/source/filter/sgvmain.hxx")
        or srcLoc.startswith("vcl/source/filter/sgfbram.hxx")
        or srcLoc.startswith("vcl/inc/unx/XIM.h")
        or srcLoc.startswith("vcl/inc/unx/gtk/gloactiongroup.h")
        or srcLoc.startswith("include/svl/svdde.hxx")
        or srcLoc.startswith("lotuswordpro/source/filter/lwpsdwdrawheader.hxx")
        or srcLoc.startswith("hwpfilter/")
        or srcLoc.startswith("embeddedobj/source/inc/")
        or srcLoc.startswith("svtools/source/dialogs/insdlg.cxx")
        or srcLoc.startswith("bridges/")):
        continue
    if d[0] in set([ "AtkObjectWrapperClass", "AtkObjectWrapper", "GLOMenu", "GLOAction", "_XRegion", "SalMenuButtonItem", "Vertex",
            "OOoMountOperationClass", "SwCSS1ItemIds", "ScCompiler::AddInMap", "MemoryByteGrabber", "textcat_t", "fp_t", "ngram_t",
            "ImplPPTParaPropSet", "DataNode"]):
        continue
    # unit testing code
    if srcLoc.startswith("cppu/source/uno/check.cxx"):
        continue
    fieldType = definitionToTypeMap[d]
    if "ModuleClient" in fieldType:
        continue
    if "::sfx2::sidebar::ControllerItem" in fieldType:
        continue
    untouchedSet.add((d[0] + " " + d[1] + " " + fieldType, srcLoc))
    untouchedSetD.add(d)

# Calculate only-touched-in-constructor set
onlyUsedInConstructorSet = set()
for d in definitionSet:
    if d in touchedFromOutsideSet or d in touchedFromOutsideConstructorSet:
        continue
    srcLoc = definitionToSourceLocationMap[d];
    # this is all representations of on-disk data structures
    if (srcLoc.startswith("sc/source/filter/inc/scflt.hxx")
        or srcLoc.startswith("sw/source/filter/ww8/")
        or srcLoc.startswith("vcl/source/filter/sgvmain.hxx")
        or srcLoc.startswith("vcl/source/filter/sgfbram.hxx")
        or srcLoc.startswith("vcl/inc/unx/XIM.h")
        or srcLoc.startswith("vcl/inc/unx/gtk/gloactiongroup.h")
        or srcLoc.startswith("include/svl/svdde.hxx")
        or srcLoc.startswith("lotuswordpro/source/filter/lwpsdwdrawheader.hxx")
        or srcLoc.startswith("hwpfilter/")
        or srcLoc.startswith("embeddedobj/source/inc/")
        or srcLoc.startswith("svtools/source/dialogs/insdlg.cxx")
        or srcLoc.startswith("bridges/")):
        continue
    fieldType = definitionToTypeMap[d]
    if "std::unique_ptr" in fieldType:
        continue
    if "std::shared_ptr" in fieldType:
        continue
    if "Reference<" in fieldType:
        continue
    if "VclPtr<" in fieldType:
        continue
    if "osl::Mutex" in fieldType:
        continue
    if "::sfx2::sidebar::ControllerItem" in fieldType:
        continue
    onlyUsedInConstructorSet.add((d[0] + " " + d[1] + " " + fieldType, srcLoc))

writeonlySet = set()
for d in definitionSet:
    parentClazz = d[0];
    if d in readFromSet or d in untouchedSetD:
        continue
    srcLoc = definitionToSourceLocationMap[d];
    # this is all representations of on-disk data structures
    if (srcLoc.startswith("sc/source/filter/inc/scflt.hxx")
        or srcLoc.startswith("sw/source/filter/ww8/")
        or srcLoc.startswith("vcl/source/filter/sgvmain.hxx")
        or srcLoc.startswith("vcl/source/filter/sgfbram.hxx")
        or srcLoc.startswith("vcl/inc/unx/XIM.h")
        or srcLoc.startswith("vcl/inc/unx/gtk/gloactiongroup.h")
        or srcLoc.startswith("include/svl/svdde.hxx")
        or srcLoc.startswith("lotuswordpro/source/filter/lwpsdwdrawheader.hxx")
        or srcLoc.startswith("svtools/source/dialogs/insdlg.cxx")):
        continue
    fieldType = definitionToTypeMap[d]
    if "ModuleClient" in fieldType:
        continue
    if "::sfx2::sidebar::ControllerItem" in fieldType:
        continue
    # ignore reference fields, because writing to them actually writes to another field somewhere else
    if fieldType.endswith("&"):
        continue
    # ignore the import/export data model stuff
    if srcLoc.startswith("sc/source/filter/inc/") and "Model" in fieldType:
        continue
    if srcLoc.startswith("sc/source/filter/inc/") and (parentClazz.startswith("Xcl") or parentClazz.startswith("oox::xls::")):
        continue
    # implement some kind of registration of errors
    if fieldType == "class SfxErrorHandler *":
        continue
    # mutex locking
    if "Guard" in fieldType:
        continue
    # these are just all model classes
    if (srcLoc.startswith("oox/")
        or srcLoc.startswith("lotuswordpro/")
        or srcLoc.startswith("include/oox/")
        or srcLoc.startswith("include/filter/")
        or srcLoc.startswith("hwpfilter/")
        or srcLoc.startswith("filter/")):
        continue

    writeonlySet.add((d[0] + " " + d[1] + " " + definitionToTypeMap[d], srcLoc))


readonlySet = set()
for d in definitionSet:
    parentClazz = d[0];
    if d in writeToSet or d in untouchedSetD:
        continue
    fieldType = definitionToTypeMap[d]
    srcLoc = definitionToSourceLocationMap[d];
    if "ModuleClient" in fieldType:
        continue
    # this is all representations of on-disk data structures
    if (srcLoc.startswith("sc/source/filter/inc/scflt.hxx")
        or srcLoc.startswith("sw/source/filter/ww8/")
        or srcLoc.startswith("vcl/source/filter/sgvmain.hxx")
        or srcLoc.startswith("vcl/source/filter/sgfbram.hxx")
        or srcLoc.startswith("vcl/inc/unx/XIM.h")
        or srcLoc.startswith("vcl/inc/unx/gtk/gloactiongroup.h")
        or srcLoc.startswith("include/svl/svdde.hxx")):
        continue
    # I really don't care about these ancient file formats
    if (srcLoc.startswith("hwpfilter/")
        or srcLoc.startswith("lotuswordpro/")):
        continue
    readonlySet.add((d[0] + " " + d[1] + " " + definitionToTypeMap[d], srcLoc))


canBePrivateSet = set()
for d in protectedAndPublicDefinitionSet:
    clazz = d[0] + " " + d[1]
    if d in touchedFromOutsideSet:
        continue
    srcLoc = definitionToSourceLocationMap[d];

    canBePrivateSet.add((clazz + " " + definitionToTypeMap[d], srcLoc))


# --------------------------------------------------------------------------------------------
# "all fields in class can be made private" analysis
# --------------------------------------------------------------------------------------------

potentialClasses = set()
excludedClasses = set()
potentialClassesSourceLocationMap = dict()
matchClassName = re.compile(r"(\w+)::")
for d in protectedAndPublicDefinitionSet:
    clazz = d[0]
    if d in touchedFromOutsideSet:
        excludedClasses.add(clazz)
    else:
        potentialClasses.add(clazz)
        potentialClassesSourceLocationMap[clazz] = definitionToSourceLocationMap[d]
allFieldsCanBePrivateSet = set()
for d in (potentialClasses - excludedClasses):
    sourceLoc = potentialClassesSourceLocationMap[d]
    # when the class is inside a compile unit, assume that the compiler can figure this out for itself, much less interesting to me
    if not ".cxx" in sourceLoc:
        allFieldsCanBePrivateSet.add((d, sourceLoc))

# sort the results using a "natural order" so sequences like [item1,item2,item10] sort nicely
def natural_sort_key(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]

# sort results by name and line number
tmp1list = sorted(untouchedSet, key=lambda v: natural_sort_key(v[1]))
tmp2list = sorted(writeonlySet, key=lambda v: natural_sort_key(v[1]))
tmp3list = sorted(canBePrivateSet, key=lambda v: natural_sort_key(v[1]))
tmp4list = sorted(readonlySet, key=lambda v: natural_sort_key(v[1]))
tmp5list = sorted(onlyUsedInConstructorSet, key=lambda v: natural_sort_key(v[1]))
tmp6list = sorted(allFieldsCanBePrivateSet, key=lambda v: natural_sort_key(v[1]))

# print out the results
with open("compilerplugins/clang/unusedfields.untouched.results", "wt") as f:
    for t in tmp1list:
        f.write( t[1] + "\n" )
        f.write( "    " + t[0] + "\n" )
with open("compilerplugins/clang/unusedfields.writeonly.results", "wt") as f:
    for t in tmp2list:
        f.write( t[1] + "\n" )
        f.write( "    " + t[0] + "\n" )
# this one is not checked in yet because I haven't actually done anything with it
with open("loplugin.unusedfields.report-can-be-private", "wt") as f:
    for t in tmp3list:
        f.write( t[1] + "\n" )
        f.write( "    " + t[0] + "\n" )
with open("compilerplugins/clang/unusedfields.readonly.results", "wt") as f:
    for t in tmp4list:
        f.write( t[1] + "\n" )
        f.write( "    " + t[0] + "\n" )
with open("compilerplugins/clang/unusedfields.only-used-in-constructor.results", "wt") as f:
    for t in tmp5list:
        f.write( t[1] + "\n" )
        f.write( "    " + t[0] + "\n" )
with open("compilerplugins/clang/unusedfields.report-all-can-be-private", "wt") as f:
    for t in tmp6list:
        f.write( t[1] + "\n" )
        f.write( "    " + t[0] + "\n" )



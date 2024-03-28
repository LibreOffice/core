#!/usr/bin/python3

import re
import io

# --------------------------------------------------------------------------------------------
# globals
# --------------------------------------------------------------------------------------------

definitionSet = set() # set of tuple(return_type, name_and_params)
definitionToSourceLocationMap = dict()

# for the "unused methods" analysis
callSet = set() # set of tuple(return_type, name_and_params)

# for the "method can be private" analysis
publicDefinitionSet = set() # set of tuple(return_type, name_and_params)
protectedDefinitionSet = set() # set of tuple(return_type, name_and_params)
calledFromOutsideSet = set() # set of tuple(return_type, name_and_params)
virtualSet = set() # set of tuple(return_type, name_and_params)

# for the "unused return types" analysis
usedReturnSet = set() # set of tuple(return_type, name_and_params)

# clang does not always use exactly the same numbers in the type-parameter vars it generates
# so I need to substitute them to ensure we can match correctly.
normalizeTypeParamsRegex1 = re.compile(r"type-parameter-\d+-\d+")
# clang sometimes generates a type name as either "class Foo" or "Foo"
# so I need to substitute them to ensure we can match correctly.
normalizeTypeParamsRegex2 = re.compile(r"class ")
def normalizeTypeParams( line ):
    line = normalizeTypeParamsRegex1.sub("type-parameter-?-?", line)
    line = normalizeTypeParamsRegex2.sub("", line)
    return line

# --------------------------------------------------------------------------------------------
# primary input loop
# --------------------------------------------------------------------------------------------

with io.open("workdir/loplugin.unusedmethods.log", "r", buffering=16*1024*1024) as txt:
    for line in txt:
        tokens = line.strip().split("\t")
        if tokens[0] == "definition:":
            access = tokens[1]
            returnType = tokens[2]
            nameAndParams = tokens[3]
            sourceLocation = tokens[4]
            virtual = ""
            if len(tokens)>=6: virtual = tokens[5]
            funcInfo = (normalizeTypeParams(returnType), normalizeTypeParams(nameAndParams))
            definitionSet.add(funcInfo)
            if access == "public":
                publicDefinitionSet.add(funcInfo)
            elif access == "protected":
                protectedDefinitionSet.add(funcInfo)
            definitionToSourceLocationMap[funcInfo] = sourceLocation
            if virtual == "virtual":
                virtualSet.add(funcInfo)
        elif tokens[0] == "call:":
            returnType = tokens[1]
            nameAndParams = tokens[2]
            callSet.add((normalizeTypeParams(returnType), normalizeTypeParams(nameAndParams)))
        elif tokens[0] == "usedReturn:":
            returnType = tokens[1]
            nameAndParams = tokens[2]
            usedReturnSet.add((normalizeTypeParams(returnType), normalizeTypeParams(nameAndParams)))
        elif tokens[0] == "outside:":
            returnType = tokens[1]
            nameAndParams = tokens[2]
            calledFromOutsideSet.add((normalizeTypeParams(returnType), normalizeTypeParams(nameAndParams)))
        else:
            print( "unknown line: " + line)

# Invert the definitionToSourceLocationMap.
sourceLocationToDefinitionMap = {}
for k, v in definitionToSourceLocationMap.items():
    sourceLocationToDefinitionMap[v] = sourceLocationToDefinitionMap.get(v, [])
    sourceLocationToDefinitionMap[v].append(k)

def isOtherConstness( d, callSet ):
    method = d[0] + " " + d[1]
    # if this method is const, and there is a non-const variant of it, and the non-const variant is in use, then leave it alone
    if d[0].startswith("const ") and d[1].endswith(" const"):
        if ((d[0][6:],d[1][:-6]) in callSet):
           return True
    elif method.endswith(" const"):
        method2 = method[:len(method)-6] # strip off " const"
        if ((d[0],method2) in callSet):
           return True
    if method.endswith(" const") and ("::iterator" in method):
        method2 = method[:len(method)-6] # strip off " const"
        method2 = method2.replace("::const_iterator", "::iterator")
        if ((d[0],method2) in callSet):
           return True
    # if this method is non-const, and there is a const variant of it, and the const variant is in use, then leave it alone
    if (not method.endswith(" const")) and ((d[0],"const " + method + " const") in callSet):
           return True
    if (not method.endswith(" const")) and ("::iterator" in method):
        method2 = method.replace("::iterator", "::const_iterator") + " const"
        if ((d[0],method2) in callSet):
           return True
    return False

# sort the results using a "natural order" so sequences like [item1,item2,item10] sort nicely
def natural_sort_key(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]
# sort by both the source-line and the datatype, so the output file ordering is stable
# when we have multiple items on the same source line
def v_sort_key(v):
    return natural_sort_key(v[1]) + [v[0]]
def sort_set_by_natural_key(s):
    return sorted(s, key=lambda v: v_sort_key(v))


# --------------------------------------------------------------------------------------------
#  "unused methods" analysis
# --------------------------------------------------------------------------------------------

tmp1set = set() # set of tuple(method, source_location)
unusedSet = set() # set of tuple(return_type, name_and_params)
for d in definitionSet:
    method = d[0] + " " + d[1]
    if d in callSet:
        continue
    if isOtherConstness(d, callSet):
        continue
    # exclude assignment operators, if we remove them, the compiler creates a default one, which can have odd consequences
    if "::operator=(" in d[1]:
        continue
    # these are only invoked implicitly, so the plugin does not see the calls
    if "::operator new(" in d[1] or "::operator delete(" in d[1]:
        continue
    # just ignore iterators, they normally occur in pairs, and we typically want to leave one constness version alone
    # alone if the other one is in use.
    if d[1] == "begin() const" or d[1] == "begin()" or d[1] == "end()" or d[1] == "end() const":
        continue
    # used by Windows build
    if any(x in d[1] for x in ["DdeTopic::", "DdeData::", "DdeService::", "DdeTransaction::", "DdeConnection::", "DdeLink::", "DdeItem::", "DdeGetPutItem::"]):
       continue
    if method == "class tools::SvRef<class FontCharMap> FontCharMap::GetDefaultMap(_Bool)":
       continue
    # these are loaded by dlopen() from somewhere
    if "get_implementation" in d[1]:
       continue
    if "component_getFactory" in d[1]:
       continue
    if d[0]=="_Bool" and "_supportsService(const class rtl::OUString &)" in d[1]:
       continue
    if (d[0]=="class com::sun::star::uno::Reference<class com::sun::star::uno::XInterface>"
        and "Instance(const class com::sun::star::uno::Reference<class com::sun::star::lang::XMultiServiceFactory> &)" in d[1]):
       continue
    # ignore the Java symbols, loaded from the JavaVM
    if d[1].startswith("Java_"):
       continue
    # ignore the VCL_BUILDER_DECL_FACTORY stuff
    if d[0]=="void" and d[1].startswith("make") and ("(class VclPtr<class vcl::Window> &" in d[1]):
       continue
    # ignore methods used to dump objects to stream - normally used for debugging
    if d[0] == "class std::basic_ostream<char> &" and d[1].startswith("operator<<(class std::basic_ostream<char> &"):
       continue
    if d[0] == "basic_ostream<type-parameter-?-?, type-parameter-?-?> &" and d[1].startswith("operator<<(basic_ostream<type-parameter-?-?"):
       continue
    # ignore lambdas
    if (" ::operator " in method) or (" ::__invoke(" in method) or (" ::operator())" in method): continue
    if ("(lambda at " in method): continue
    # ignore stuff generated by std::function parameters
    if ("(anonymous)::operator " in method) and ("(*)" in method): continue
    # stuff generated by Qt
    if "::tr(" in method or "::trUtf8(" in method: continue

    location = definitionToSourceLocationMap[d]
    # whacky template stuff
    if location.startswith("sc/source/ui/vba/vbaformat.hxx"): continue
    # not sure how this stuff is called
    if location.startswith("include/test"): continue
    # leave the debug/dump alone
    if location.startswith("include/oox/dump"): continue
    # plugin testing stuff
    if location.startswith("compilerplugins/clang/test"): continue
    # leave this alone for now
    if location.startswith("include/LibreOfficeKit"): continue
    # template stuff
    if location.startswith("include/vcl/vclptr.hxx"): continue
    if location.startswith("include/oox/helper/refvector.hxx"): continue
    if location.startswith("include/oox/drawingml/chart/modelbase.hxx"): continue

    unusedSet.add(d) # used by the "unused return types" analysis
    tmp1set.add((method, location))

# print out the results, sorted by name and line number
with open("compilerplugins/clang/unusedmethods.results", "wt") as f:
    for t in sort_set_by_natural_key(tmp1set):
        f.write(t[1] + "\n")
        f.write("    " + t[0] + "\n")

# --------------------------------------------------------------------------------------------
# "unused return types" analysis
# --------------------------------------------------------------------------------------------

tmp2set = set()
for d in definitionSet:
    method = d[0] + " " + d[1]
    if d in usedReturnSet:
        continue
    if d in unusedSet:
        continue
    if isOtherConstness(d, usedReturnSet):
        continue
    # ignore methods with no return type, and constructors
    if d[0] == "void" or d[0] == "":
        continue
    # ignore UNO constructor method entrypoints
    if "_get_implementation" in d[1] or "_getFactory" in d[1]:
        continue
    # the plugin can't see calls to these
    if "::operator new" in d[1]:
        continue
    # unused return type is not a problem here
    if ("operator=(" in d[1] or "operator&=" in d[1] or "operator|=" in d[1] or "operator^=" in d[1]
        or "operator+=" in d[1] or "operator-=" in d[1]
        or "operator<<" in d[1] or "operator>>" in d[1]
        or "operator++" in d[1] or "operator--" in d[1]):
        continue
    # ignore UNO constructor functions
    if (d[0] == "class com::sun::star::uno::Reference<class com::sun::star::uno::XInterface>" and
        d[1].endswith("_createInstance(const class com::sun::star::uno::Reference<class com::sun::star::lang::XMultiServiceFactory> &)")):
        continue
    if (d[0] == "class com::sun::star::uno::Reference<class com::sun::star::uno::XInterface>" and
        d[1].endswith("_CreateInstance(const class com::sun::star::uno::Reference<class com::sun::star::lang::XMultiServiceFactory> &)")):
        continue
    # debug code
    if d[1] == "writerfilter::ooxml::OOXMLPropertySet::toString()":
        continue
    # ignore lambdas
    if "::__invoke(" in d[1]:
        continue
    if "(lambda at " in d[1]:
        continue
    if "::operator " in d[1] and "(*)(" in d[1]:
        continue
    location = definitionToSourceLocationMap[d]
    # windows only
    if location.startswith("include/svl/svdde.hxx"): continue
    # fluent API (return ref to self)
    if location.startswith("include/tools/stream.hxx"): continue
    if location.startswith("include/oox/helper/refvector.hxx"): continue
    if location.startswith("include/oox/drawingml/chart/modelbase.hxx"): continue
    # templates
    if location.startswith("include/vcl/vclptr.hxx"): continue
    # external API
    if location.startswith("include/LibreOfficeKit/LibreOfficeKit.hxx"): continue
    tmp2set.add((method, location))

#Disable this for now, not really using it
# print output, sorted by name and line number
with open("compilerplugins/clang/unusedmethods.unused-returns.results", "wt") as f:
    for t in sort_set_by_natural_key(tmp2set):
        f.write(t[1] + "\n")
        f.write("    " +  t[0] + "\n")


# --------------------------------------------------------------------------------------------
# "method can be private" analysis
# --------------------------------------------------------------------------------------------

tmp3set = set()
for d in publicDefinitionSet:
    method = d[0] + " " + d[1]
    if d in calledFromOutsideSet:
        continue
    if d in virtualSet:
        continue
    # TODO ignore constructors for now, my called-from-outside analysis doesn't work here
    if d[0] == "":
        continue
    if isOtherConstness(d, calledFromOutsideSet):
        continue
    tmp3set.add((method, definitionToSourceLocationMap[d]))

# print output, sorted by name and line number
with open("loplugin.unusedmethods.report-can-be-private", "wt") as f:
    for t in sort_set_by_natural_key(tmp3set):
        f.write(t[1] + "\n")
        f.write("    " + t[0] + "\n")



# --------------------------------------------------------------------------------------------
# "all protected methods in class can be made private" analysis
# --------------------------------------------------------------------------------------------

potentialClasses = set()
excludedClasses = set()
potentialClassesSourceLocationMap = dict()
matchClassName = re.compile(r"(\w+)::")
for d in protectedDefinitionSet:
    m = matchClassName.match(d[1])
    if not m: continue
    clazz = m.group(1)
    if d in calledFromOutsideSet:
        excludedClasses.add(clazz)
    else:
        potentialClasses.add(clazz)
        potentialClassesSourceLocationMap[clazz] = definitionToSourceLocationMap[d]

tmp4set = set()
for d in (potentialClasses - excludedClasses):
    tmp4set.add((d, potentialClassesSourceLocationMap[d]))

# print output, sorted by name and line number
with open("loplugin.unusedmethods.report-all-protected-can-be-private", "wt") as f:
    for t in sort_set_by_natural_key(tmp4set):
        f.write(t[1] + "\n")
        f.write("    " + t[0] + "\n")


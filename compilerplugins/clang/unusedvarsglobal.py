#!/usr/bin/python2

import sys
import re
import io

definitionSet = set()
readFromSet = set()
writeToSet = set()
defToTypeMap = dict()

def parseFieldInfo( tokens ):
    return (tokens[1].strip(), tokens[2].strip())

with io.open("workdir/loplugin.unusedvarsglobal.log", "rb", buffering=1024*1024) as txt:
    for line in txt:
        try:
            tokens = line.strip().split("\t")
            if tokens[0] == "definition:":
                srcLoc = tokens[3]
                # ignore external source code
                if (srcLoc.startswith("external/")):
                    continue
                # ignore build folder
                if (srcLoc.startswith("workdir/")):
                    continue
                varname = tokens[1].strip()
                vartype = tokens[2].strip()
                if vartype.startswith("const "):
                    vartype = vartype[6:]
                if vartype.startswith("class "):
                    vartype = vartype[6:]
                if vartype.startswith("struct "):
                    vartype = vartype[7:]
                if vartype.startswith("::"):
                    vartype = vartype[2:]
                fieldInfo = (srcLoc, varname)
                definitionSet.add(fieldInfo)
                defToTypeMap[fieldInfo] = vartype
            elif tokens[0] == "read:":
                if len(tokens) == 3:
                    readFromSet.add(parseFieldInfo(tokens))
            elif tokens[0] == "write:":
                if len(tokens) == 3:
                    writeToSet.add(parseFieldInfo(tokens))
            else:
                print( "unknown line: " + line)
        except IndexError:
            print "problem with line " + line.strip()
            raise

definitionSet2 = set()
for d in definitionSet:
    varname = d[1]
    vartype = defToTypeMap[d]
    if len(varname) == 0:
        continue
    if varname.startswith("autoRegister"): # auto-generated CPPUNIT stuff
        continue
    if vartype in ["css::uno::ContextLayer", "SolarMutexGuard", "SolarMutexReleaser", "OpenGLZone"]:
        continue
    if vartype in ["PreDefaultWinNoOpenGLZone", "SchedulerGuard", "SkiaZone", "OpenGLVCLContextZone"]:
        continue
    if vartype in ["SwXDispatchProviderInterceptor::DispatchMutexLock_Impl", "SfxObjectShellLock", "OpenCLZone"]:
        continue
    if vartype in ["OpenCLInitialZone", "pyuno::PyThreadDetach", "SortRefUpdateSetter", "oglcanvas::TransformationPreserver"]:
        continue
    if vartype in ["StackHack", "osl::MutexGuard", "accessibility::SolarMethodGuard"]:
        continue
    if vartype in ["osl::ClearableMutexGuard", "comphelper::OExternalLockGuard", "osl::Guard< ::osl::Mutex>"]:
        continue
    if vartype in ["comphelper::OContextEntryGuard", "Guard<class osl::Mutex>", "basic::LibraryContainerMethodGuard"]:
        continue
    if vartype in ["canvas::CanvasBase::MutexType"]:
        continue
    definitionSet2.add(d)

# Calculate untouched
untouchedSet = set()
for d in definitionSet2:
    if d in readFromSet or d in writeToSet:
        continue
    varname = d[1]
    if len(varname) == 0:
        continue
    untouchedSet.add(d)

writeonlySet = set()
for d in definitionSet2:
    if d in readFromSet or d in untouchedSet:
        continue
    varname = d[1]
    vartype = defToTypeMap[d]
    if "Alive" in varname:
        continue
    if "Keep" in varname:
        continue
    if vartype.endswith(" &"):
        continue
    writeonlySet.add(d)

readonlySet = set()
for d in definitionSet2:
    if d in writeToSet or d in untouchedSet:
        continue
    varname = d[1]
    vartype = defToTypeMap[d]
    if "Dummy" in varname:
        continue
    if "Empty" in varname:
        continue
    if varname in ["aOldValue", "aNewValue"]:
        continue
    if "Exception" in vartype and vartype.endswith(" &"):
        continue
    if "exception" in vartype and vartype.endswith(" &"):
        continue
    # TODO for now, focus on the simple stuff
    if not (vartype in ["rtl::OUString", "Bool"]):
        continue
    readonlySet.add(d)

# sort the results using a "natural order" so sequences like [item1,item2,item10] sort nicely
def natural_sort_key(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]

# sort results by name and line number
tmp1list = sorted(untouchedSet, key=lambda v: natural_sort_key(v[0]))
tmp2list = sorted(writeonlySet, key=lambda v: natural_sort_key(v[0]))
tmp3list = sorted(readonlySet, key=lambda v: natural_sort_key(v[0]))

# print out the results
with open("compilerplugins/clang/unusedvarsglobal.untouched.results", "wt") as f:
    for t in tmp1list:
        f.write( t[0] + "\n" )
        f.write( "    " + defToTypeMap[t] + " " + t[1] + "\n" )
with open("compilerplugins/clang/unusedvarsglobal.writeonly.results", "wt") as f:
    for t in tmp2list:
        f.write( t[0] + "\n" )
        f.write( "    " + defToTypeMap[t] + " " + t[1] + "\n" )
with open("compilerplugins/clang/unusedvarsglobal.readonly.results", "wt") as f:
    for t in tmp3list:
        f.write( t[0] + "\n" )
        f.write( "    " + defToTypeMap[t] + " " + t[1] + "\n" )



#!/usr/bin/python3

from collections import defaultdict
import io
import re
import subprocess

# --------------------------------------------------------------------------------------------
# globals
# --------------------------------------------------------------------------------------------

definitionSet = set() # set of method_name
definitionToSourceLocationMap = dict()

# for the "unused methods" analysis
callDict = defaultdict(set) # map of from_method_name -> set(method_name)

# clang does not always use exactly the same numbers in the type-parameter vars it generates
# so I need to substitute them to ensure we can match correctly.
normalizeTypeParamsRegex = re.compile(r"type-parameter-\d+-\d+")
def normalizeTypeParams( line ):
    line = normalizeTypeParamsRegex.sub("type-parameter-?-?", line)
    # make some of the types a little prettier
    line = line.replace("std::__debug", "std::")
    line = line.replace("class ", "")
    line = line.replace("struct ", "")
    line = line.replace("_Bool", "bool")
    return line

# --------------------------------------------------------------------------------------------
# primary input loop
# --------------------------------------------------------------------------------------------

cnt = 0
with io.open("workdir/loplugin.methodcycles.log", "r", buffering=1024*1024) as txt:
    for line in txt:
        tokens = line.strip().split("\t")
        if tokens[0] == "definition:":
            returnType = tokens[1]
            nameAndParams = tokens[2]
            sourceLocation = tokens[3]
            funcInfo = (normalizeTypeParams(returnType) + " " + normalizeTypeParams(nameAndParams)).strip()
            definitionSet.add(funcInfo)
            definitionToSourceLocationMap[funcInfo] = sourceLocation
        elif tokens[0] == "call:":
            returnTypeFrom = tokens[1]
            nameAndParamsFrom = tokens[2]
            returnTypeTo = tokens[3]
            nameAndParamsTo = tokens[4]
            caller = (normalizeTypeParams(returnTypeFrom) + " " + normalizeTypeParams(nameAndParamsFrom)).strip()
            callee = (normalizeTypeParams(returnTypeTo) + " " + normalizeTypeParams(nameAndParamsTo)).strip()
            callDict[caller].add(callee)
        else:
            print( "unknown line: " + line)
        cnt = cnt + 1
        #if cnt > 100000: break

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
#  analysis
# --------------------------------------------------------------------------------------------

# follow caller-callee chains, removing all methods reachable from a root method
def remove_reachable(callDict, startCaller):
    worklist = list()
    worklist.append(startCaller)
    while len(worklist) > 0:
        caller = worklist.pop()
        if caller not in callDict:
            continue
        calleeSet = callDict[caller]
        del callDict[caller]
        if caller in definitionSet:
            definitionSet.remove(caller)
        for c in calleeSet:
            worklist.append(c)

# look for all the external entry points and remove code called from there
to_be_removed = set()
to_be_removed.add("int main(int,char **)")
# random dynload entrypoints that we don't otherwise find
to_be_removed.add("bool TestImportOLE2(SvStream &)")
to_be_removed.add("void SbiRuntime::StepREDIMP()")
to_be_removed.add("_object * (anonymous namespace)::createUnoStructHelper(_object *,_object *,_object *)")
for caller in definitionSet:
    if caller not in definitionToSourceLocationMap:
        to_be_removed.append(caller)
        continue
    location = definitionToSourceLocationMap[caller]
    if "include/com/" in location \
      or "include/cppu/" in location \
      or "include/cppuhelper/" in location \
      or "include/osl/" in location \
      or "include/rtl/" in location \
      or "include/sal/" in location \
      or "include/salhelper/" in location \
      or "include/typelib/" in location \
      or "include/uno/" in location \
      or "workdir/UnpackedTarball/" in location \
      or "workdir/UnoApiHeadersTarget/" in location \
      or "workdir/CustomTarget/officecfg/" in location \
      or "workdir/LexTarget/" in location \
      or "workdir/CustomTarget/i18npool/localedata/" in location \
      or "workdir/SdiTarget/" in location \
      or "/qa/" in location \
      or "include/test/" in location:
        to_be_removed.add(caller)
    # TODO calls to destructors are not mentioned in the AST, so we'll just have to assume they get called,
    # which is not ideal
    if "::~" in caller:
        to_be_removed.add(caller)
    # dyload entry points for VCL builder
    if "(VclPtr<vcl::Window> & rRet, const VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)" in caller:
        to_be_removed.add(caller)
    if "(VclPtr<vcl::Window> &,const VclPtr<vcl::Window> &,std::::map<rtl::OString, rtl::OUString, std::less<rtl::OString>, std::allocator<std::pair<const rtl::OString, rtl::OUString> > > &)" in caller:
        to_be_removed.add(caller)
# find all the UNO load-by-symbol-name entrypoints
uno_constructor_entrypoints = set()
git_grep_process = subprocess.Popen("git grep -h 'constructor=' -- *.component", stdout=subprocess.PIPE, shell=True)
with git_grep_process.stdout as txt:
    for line in txt:
        idx1 = line.find(b"\"")
        idx2 = line.find(b"\"", idx1 + 1)
        func = line[idx1+1 : idx2]
        uno_constructor_entrypoints.add(func.decode('utf-8'))
for caller in callDict:
    if "(com::sun::star::uno::XComponentContext *,const com::sun::star::uno::Sequence<com::sun::star::uno::Any> &)" in caller:
        for func in uno_constructor_entrypoints:
            if func in caller:
                to_be_removed.add(caller)
# remove everything reachable from the found entry points
for caller in to_be_removed:
    remove_reachable(callDict, caller)
for caller in callDict:
    callDict[caller] -= to_be_removed

# create a reverse call graph
inverseCallDict = defaultdict(set) # map of from_method_name -> set(method_name)
for caller in callDict:
    for callee in callDict[caller]:
        inverseCallDict[callee].add(caller)

print_tree_recurse_set = set() # protect against cycles
def print_tree(f, callDict, caller, depth):
    if depth == 0:
        f.write("\n") # add an empty line before each tree
        print_tree_recurse_set.clear()
    # protect against cycles
    if caller in print_tree_recurse_set:
        return
    # when printing out trees, things that are not in the map are things that are reachable,
    # so we're not interested in them
    if caller not in callDict:
        return
    print_tree_recurse_set.add(caller)
    f.write("  " * depth + caller + "\n")
    f.write("  " * depth + definitionToSourceLocationMap[caller] + "\n")
    calleeSet = callDict[caller]
    for c in calleeSet:
        print_tree(f, callDict, c, depth+1)

# find possible roots (ie. entrypoints) by looking for methods that are not called
def dump_possible_roots():
    possibleRootList = list()
    for caller in callDict:
        if caller not in inverseCallDict and caller in definitionToSourceLocationMap:
            possibleRootList.append(caller)
    possibleRootList.sort()

    # print out first 100 trees of caller->callees
    count = 0
    with open("compilerplugins/clang/methodcycles.roots", "wt") as f:
        f.write("callDict size " + str(len(callDict)) + "\n")
        f.write("possibleRootList size " + str(len(possibleRootList)) + "\n")
        f.write("\n")
        for caller in possibleRootList:
            f.write(caller + "\n")
            f.write("  " + definitionToSourceLocationMap[caller] + "\n")
            #print_tree(f, caller, 0)
            count = count + 1
            #if count>1000: break

# Look for cycles in a directed graph
# Adapted from:
# https://codereview.stackexchange.com/questions/86021/check-if-a-directed-graph-contains-a-cycle
def print_cycles():
    with open("compilerplugins/clang/methodcycles.results", "wt") as f:
        path = set()
        visited = set()

        def printPath(path):
            if len(path) < 2:
                return
            # we may have found a cycle, but if the cycle is called from outside the cycle
            # the code is still in use.
            for p in path:
                for caller in inverseCallDict[p]:
                    if caller not in path:
                        return
            f.write("found cycle\n")
            for p in path:
                f.write("    " + p + "\n")
                f.write("    " + definitionToSourceLocationMap[p] + "\n")
                f.write("\n")

        def checkCyclic(vertex):
            if vertex in visited:
                return
            visited.add(vertex)
            path.add(vertex)
            if vertex in callDict:
                for neighbour in callDict[vertex]:
                    if neighbour in path:
                        printPath(path)
                        break
                    else:
                        checkCyclic(neighbour)
            path.remove(vertex)

        for caller in callDict:
            checkCyclic(caller)

print_cycles()

# print partitioned sub-graphs
def print_partitions():
    callDict2 = callDict
    # Remove anything with no callees, and that is itself not called.
    # After this stage, we should only be left with closed sub-graphs ie. partitions
    while True:
        to_be_removed.clear()
        for caller in callDict2:
            if len(callDict2[caller]) == 0 \
                or caller not in inverseCallDict[caller]:
                to_be_removed.add(caller)
        if len(to_be_removed) == 0:
            break
        for caller in to_be_removed:
            remove_reachable(callDict2, caller)
        for caller in callDict2:
            callDict2[caller] -= to_be_removed

    count = 0
    with open("compilerplugins/clang/methodcycles.partition.results", "wt") as f:
        f.write("callDict size " + str(len(callDict2)) + "\n")
        f.write("\n")
        while len(callDict2) > 0:
            print_tree(f, callDict2, next(iter(callDict2)), 0)
            for c in print_tree_recurse_set:
                callDict2.pop(c, None)
            count = count + 1
            if count>1000: break

print_partitions()

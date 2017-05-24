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
with io.open("loplugin.constantparam.log", "rb", buffering=1024*1024) as txt:
    for line in txt:
        try:
            tokens = line.strip().split("\t")
            returnType = normalizeTypeParams(tokens[0])
            nameAndParams = normalizeTypeParams(tokens[1])
            sourceLocation = tokens[2]
            paramName = tokens[3]
            paramType = normalizeTypeParams(tokens[4])
            callValue = tokens[5]
            callInfo = (returnType, nameAndParams, paramName, paramType, sourceLocation)
            if not callInfo in callDict:
                callDict[callInfo] = set()
            callDict[callInfo].add(callValue)
        except IndexError:
            print "problem with line " + line.strip()
            raise

def RepresentsInt(s):
    try:
        int(s)
        return True
    except ValueError:
        return False

constructor_regex = re.compile("^\w+\(\)$")

tmp1list = list()
tmp2list = list()
tmp3list = list()
tmp4list = list()
for callInfo, callValues in callDict.iteritems():
    nameAndParams = callInfo[1]
    if len(callValues) != 1:
        continue
    callValue = next(iter(callValues))
    if "unknown" in callValue:
        continue
    sourceLoc = callInfo[4]
    functionSig = callInfo[0] + " " + callInfo[1]

    # try to ignore setter methods
    if ("," not in nameAndParams) and (("::set" in nameAndParams) or ("::Set" in nameAndParams)):
        continue
    # ignore code that follows a common pattern
    if sourceLoc.startswith("sw/inc/swatrset.hxx"): continue
    if sourceLoc.startswith("sw/inc/format.hxx"): continue
    # template generated code
    if sourceLoc.startswith("include/sax/fshelper.hxx"): continue
    # debug code
    if sourceLoc.startswith("include/oox/dump"): continue
    # part of our binary API
    if sourceLoc.startswith("include/LibreOfficeKit"): continue

    if RepresentsInt(callValue):
        if callValue == "0" or callValue == "1":
            tmp1list.append((sourceLoc, functionSig, callInfo[3] + " " + callInfo[2], callValue))
        else:
            tmp2list.append((sourceLoc, functionSig, callInfo[3] + " " + callInfo[2], callValue))
    # look for places where the callsite is always a constructor invocation
    elif constructor_regex.match(callValue) or callValue == "\"\"":
        if callValue.startswith("Get"): continue
        if callValue.startswith("get"): continue
        if "operator=" in functionSig: continue
        if "&&" in functionSig: continue
        if callInfo[2] == "###0" and callValue == "InitData()": continue
        if callInfo[2] == "###0" and callValue == "InitAggregate()": continue
        if callValue == "shared_from_this()": continue
        tmp3list.append((sourceLoc, functionSig, callInfo[3] + " " + callInfo[2], callValue))
    else:
        tmp4list.append((sourceLoc, functionSig, callInfo[3] + " " + callInfo[2], callValue))


# sort results by filename:lineno
def natural_sort_key(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]
tmp1list.sort(key=lambda v: natural_sort_key(v[0]))
tmp2list.sort(key=lambda v: natural_sort_key(v[0]))
tmp3list.sort(key=lambda v: natural_sort_key(v[0]))
tmp4list.sort(key=lambda v: natural_sort_key(v[0]))

# print out the results
with open("compilerplugins/clang/constantparam.booleans.results", "wt") as f:
    for v in tmp1list:
        f.write(v[0] + "\n")
        f.write("    " + v[1] + "\n")
        f.write("    " + v[2] + "\n")
        f.write("    " + v[3] + "\n")
with open("compilerplugins/clang/constantparam.numbers.results", "wt") as f:
    for v in tmp2list:
        f.write(v[0] + "\n")
        f.write("    " + v[1] + "\n")
        f.write("    " + v[2] + "\n")
        f.write("    " + v[3] + "\n")
with open("compilerplugins/clang/constantparam.constructors.results", "wt") as f:
    for v in tmp3list:
        f.write(v[0] + "\n")
        f.write("    " + v[1] + "\n")
        f.write("    " + v[2] + "\n")
        f.write("    " + v[3] + "\n")
with open("compilerplugins/clang/constantparam.others.results", "wt") as f:
    for v in tmp4list:
        f.write(v[0] + "\n")
        f.write("    " + v[1] + "\n")
        f.write("    " + v[2] + "\n")
        f.write("    " + v[3] + "\n")

# -------------------------------------------------------------
# Now a fun set of heuristics to look for methods that
# take bitmask parameters where one or more of the bits in the
# bitmask is always one or always zero

# integer to hex str
def hex(i):
    return "0x%x" % i
# I can't use python's ~ operator, because that produces negative numbers
def negate(i):
    return (1 << 32) - 1 - i

tmp2list = list()
for callInfo, callValues in callDict.iteritems():
    nameAndParams = callInfo[1]
    if len(callValues) < 2:
        continue
    # we are only interested in enum parameters
    if not "enum" in callInfo[3]: continue
    if not "Flag" in callInfo[3] and not "flag" in callInfo[3] and not "Bit" in callInfo[3] and not "State" in callInfo[3]: continue
    # try to ignore setter methods
    if ("," not in nameAndParams) and (("::set" in nameAndParams) or ("::Set" in nameAndParams)):
        continue

    setBits = 0
    clearBits = 0
    continue_flag = False
    first = True
    for callValue in callValues:
        if "unknown" == callValue or not callValue.isdigit():
            continue_flag = True
            break
        if first:
            setBits = int(callValue)
            clearBits = negate(int(callValue))
            first = False
        else:
            setBits = setBits & int(callValue)
            clearBits = clearBits & negate(int(callValue))

    # estimate allBits by using the highest bit we have seen
    # TODO dump more precise information about the allBits values of enums
    allBits = (1 << setBits.bit_length()) - 1
    clearBits = clearBits & allBits
    if continue_flag or (setBits == 0 and clearBits == 0): continue

    sourceLoc = callInfo[4]
    functionSig = callInfo[0] + " " + callInfo[1]

    v2 = callInfo[3] + " " + callInfo[2]
    if setBits != 0: v2 += " setBits=" + hex(setBits)
    if clearBits != 0: v2 += " clearBits=" + hex(clearBits)
    tmp2list.append((sourceLoc, functionSig, v2))


# sort results by filename:lineno
tmp2list.sort(key=lambda v: natural_sort_key(v[0]))

# print out the results
with open("compilerplugins/clang/constantparam.bitmask.results", "wt") as f:
    for v in tmp2list:
        f.write(v[0] + "\n")
        f.write("    " + v[1] + "\n")
        f.write("    " + v[2] + "\n")

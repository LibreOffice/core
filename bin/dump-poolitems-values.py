#!/usr/bin/python


# Produce a dump of name->constant of the poolitem values, to make interpreting things in the debugger easier
#

import subprocess

macroNameToValue = dict()
macroNameToOriginalLine = dict()


def extractMacroValue(macroValue):
    if isinstance(macroValue, int):
        return macroValue
    elif macroValue.isdigit():
        return int(macroValue)
    elif macroValue[0:2] == "0x":
        return int(macroValue, 16)
    elif macroValue.find("+") != -1:
        tokens = macroValue.split("+")
        tokens1 = tokens[0].strip()
        tokens2 = tokens[1].strip()
        return extractMacroValue(tokens1) + extractMacroValue(tokens2)
    elif macroValue.find("-") != -1:
        tokens = macroValue.split("-")
        tokens1 = tokens[0].strip()
        tokens2 = tokens[1].strip()
        return extractMacroValue(tokens1) - extractMacroValue(tokens2)
    rv = extractMacroValue(macroNameToValue[macroValue])
    macroNameToValue[macroValue] = rv
    return rv


a = subprocess.Popen("cpp -E -dD -Iinclude/ include/editeng/eeitem.hxx", stdout=subprocess.PIPE, shell=True)

with a.stdout as txt:
    for line in txt:
        line = line.strip()
        originalLine = line
        if not line.startswith("#define "):
            continue
        # strip the '#define' off the front
        idx1 = line.find(" ")
        line = line[idx1:len(line)].strip()
        # extract the name
        idx1 = line.find(" ")
        if (idx1 == -1):
            continue
        macroName = line[0:idx1].strip()
        line = line[idx1:len(line)].strip()
        # ignore internal stuff
        if macroName.startswith("_"):
            continue
        # strip any trailing comments
        idx1 = line.find("//")
        if (idx1 != -1):
            line = line[0:idx1].strip()
        idx1 = line.find("/*")
        if (idx1 != -1):
            line = line[0:idx1].strip()
        if len(line) == 0:
            continue
        # strip brackets
        if line[0] == "(":
            line = line[1:]
        if line[len(line)-1] == ")":
            line = line[0:len(line)-1]
        macroValue = line.strip()
        # ignore macros that #define strings, not interested in those
        if (macroValue.find("\"") != -1):
            continue
        # ignore the multiline macros
        if (macroValue.find("\\") != -1):
            continue
        # check for redefinitions
        if macroNameToValue.has_key(macroName):
            print("Redefinition:\n\t",  macroNameToOriginalLine[macroName], "\n\t", originalLine)
        else:
            macroNameToValue[macroName] = macroValue
            macroNameToOriginalLine[macroName] = originalLine

# decode the constants into their numeric values recursively
macroValueToName = dict()
for macroName in macroNameToValue:
    macroValue = macroNameToValue[macroName]
    try:
        macroValue = extractMacroValue(macroName)
        macroValueToName[macroValue] = macroName
    except KeyError:
        print("warning: could not decode macro ", macroName)

for macroValue in sorted(macroValueToName):
    macroName = macroValueToName[macroValue]
    print(repr(macroNameToValue[macroName]).rjust(5), " ", macroName)




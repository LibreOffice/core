#!/usr/bin/python


# Scan .hrc files for conflicting SID constants
#
# This is not as easy as it sounds because some of the constants depend on other
# constants whose names do not start with SID_
#

import subprocess

sidNameToValue = dict()
sidNameToOriginalLine = dict()


def extractSidValue(sidValue):
    if isinstance(sidValue, int):
        return sidValue
    if sidValue.isdigit():
        return int(sidValue)
    if sidValue[0:2] == "0x":
        return int(sidValue, 16)
    if sidValue.find("+") != -1:
        tokens = sidValue.split("+")
        tokens1 = tokens[0].strip()
        tokens2 = tokens[1].strip()
        return extractSidValue(tokens1) + extractSidValue(tokens2)
    rv = extractSidValue(sidNameToValue[sidValue])
    sidNameToValue[sidValue] = rv
    return rv


#a = subprocess.Popen(r"git grep -P '#define\s+(SID_|SC_|DETECTIVE_|DRAWTEXTBAR_|DRAW_BAR_|RID_|OBJBAR_FORMAT_|TAB_POPUP_|DATA_MENU_|EXTRA_MENU_|FORMAT_MENU_|INSERT_MENU_|VIEW_MENU_|EDIT_MENU_|FILE_MENU_|SC_FUNCTION_|RC_)'", stdout=subprocess.PIPE, shell=True)
a = subprocess.Popen(r"git grep -Pn '#define\s+(\S+)' -- *.hrc", stdout=subprocess.PIPE, shell=True)

with a.stdout as txt:
    for line in txt:
        originalLine = line.strip()
        # strip the '#define' off the front
        idx1 = line.find(" ")
        line = line[idx1 : len(line)].strip()
        # extract the name
        idx1 = line.find(" ")
        if (idx1 == -1):
            continue
        sidName = line[0:idx1].strip()
        line = line[idx1:len(line)].strip()
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
        sidTextValue = line.strip()
        # ignore the #define strings
        if (sidTextValue.find("\"") != -1):
            continue
        # ignore the multiline macros
        if (sidTextValue.find("\\") != -1):
            continue
        # check for redefinitions
        if sidName[0:4] == "SID_" and sidNameToValue.has_key(sidName):
            print("Redefinition:\n\t",  sidNameToOriginalLine[sidName], "\n\t" , originalLine)
        else:
            sidNameToValue[sidName] = sidTextValue
        sidNameToOriginalLine[sidName] = originalLine

    # decode the constants into their numeric values recursively
    sidNamesToIgnore = set()
    for sidName in sidNameToValue:
        sidTextValue = sidNameToValue[sidName]
        try:
            sidValueNum = extractSidValue(sidTextValue)
            sidNameToValue[sidName] = sidValueNum
        except KeyError:
            sidNamesToIgnore.add(sidName)

    # check for conflicts
    sidValueToName = dict()
    for sidName in sidNameToValue:
        if sidName in sidNamesToIgnore:
            continue
        if sidName[0:4] != "SID_":
            continue
        sidValue = sidNameToValue[sidName]
        if sidValueToName.has_key(sidValue):
            print("conflict:\n\t", sidNameToOriginalLine[sidName], "\n\t", sidNameToOriginalLine[sidValueToName[sidValue]])
        else:
            sidValueToName[sidValue] = sidName




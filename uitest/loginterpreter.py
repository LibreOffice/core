# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import os
import sys

def usage():
    message = "usage: {program} inputfile outputfile"

    print(message.format(program = os.path.basename(sys.argv[0])))

def parseline(line):
    """
    This function parses a line from log file
    and returns the parsed values as a python dictionary
    """
    if (line == "" or line.startswith("Action on element")):
        return
    dict = {}
    if "{" in line:
        start_index_of_parameters = line.find("{")
        end_index_of_parameters = line.find("}") + 1
        parameters = line[start_index_of_parameters:end_index_of_parameters]
        dict["parameters"] = parameters
        line = line[:start_index_of_parameters-1]
    wordlist = line.split()
    dict["keyword"] = wordlist[0]

    for index in range(1,len(wordlist)):
        key, val = wordlist[index].split(":",1)
        dict[key] = val
    return dict

def parseargs(argv):
    """
    This function parses the command-line arguments
    to get the input and output file details
    """
    if len(argv) != 3:
        usage()
        sys.exit(1)
    else:
        inputaddress = argv[1]
        outputaddress = argv[2]

    return inputaddress, outputaddress

def getlogfile(inputaddress):
    try:
        with open(inputaddress) as f:
            content = f.readlines()
    except IOError as err:
        print("IO error: {0}".format(err))
        usage()
        sys.exit(1)

    content = [x.strip() for x in content]
    return content

def initiatetestgeneration(address):
    try:
        f = open(address,"w")
    except IOError as err:
        print("IO error: {0}".format(err))
        usage()
        sys.exit(1)
    initialtext = \
    "from uitest.framework import UITestCase\n" + \
    "import importlib\n\n" + \
    "class TestClass(UITestCase):\n" + \
    "    def test_function(self):\n"
    f.write(initialtext)
    return f

def getcouplingtype(line1, line2):
    """
    This function checks if two consecutive lines of log file
    refer to the same event
    """
    actiondict1 = parseline(line1)
    actiondict2 = parseline(line2)

    if actiondict1["keyword"] == "CommandSent" and \
        actiondict2["keyword"] == "ModalDialogExecuted":
        return "COMMANDMODALCOUPLE"

    elif actiondict1["keyword"] == "CommandSent" and \
        actiondict2["keyword"] == "ModelessDialogExecuted":
        return "COMMANDMODELESSCOUPLE"

    return "NOTACOUPLE"

def gettestlinefromonelogline(logline):
    actiondict = parseline(logline)
    testline = "        "
    if actiondict["keyword"].endswith("UIObject"):
        parent = actiondict["Parent"]
        if (parent != ""):
            testline += \
            actiondict["Id"] + " = " + parent + ".getChild(\"" + \
            actiondict["Id"] + "\")\n        " + \
            actiondict["Id"] + ".executeAction(\"" + \
            actiondict["Action"] + "\""
            if "parameters" in actiondict:
                testline +=  ", mkPropertyValues(" + \
                actiondict["parameters"] + "))\n"
            else:
                testline += ",tuple())\n"
            return testline

    return ""

def gettestlinefromtwologlines(logline1,logline2):
    couplingtype = getcouplingtype(logline1, logline2)
    actiondict1 = parseline(logline1)
    actiondict2 = parseline(logline2)
    testline = "        "
    if couplingtype == "COMMANDMODALCOUPLE":
        testline += \
        "self.ui_test.execute_dialog_through_command(\"" + \
        actiondict1["Name"] + "\")\n        " + \
        actiondict2["Id"] + " = self.xUITest.getTopFocusWindow()\n"
    elif couplingtype == "COMMANDMODELESSCOUPLE":
        testline += \
        "self.ui_test.execute_modeless_dialog_through_command(\"" + \
        actiondict1["Name"] + "\")\n        " + \
        actiondict2["Id"] + " = self.xUITest.getTopFocusWindow()\n"
    return testline

def main():
    inputaddress, outputaddress = parseargs(sys.argv)
    loglines = getlogfile(inputaddress)
    outputstream = initiatetestgeneration(outputaddress)
    linenumber = 0
    while linenumber < len(loglines):
        if linenumber == len(loglines)-1 or \
            getcouplingtype(loglines[linenumber],loglines[linenumber + 1]) == "NOTACOUPLE":
            testline = gettestlinefromonelogline(loglines[linenumber])
            outputstream.write(testline)
            linenumber += 1
        else:
            testline = gettestlinefromtwologlines(loglines[linenumber],loglines[linenumber + 1])
            outputstream.write(testline)
            linenumber += 2
    outputstream.close()

if __name__ == '__main__':
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
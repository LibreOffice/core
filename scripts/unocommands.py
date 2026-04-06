#!/usr/bin/env python3
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import os
import re
import sys
import polib
from lxml import etree


def usageAndExit():
    message = """usage: {program} [--check|--update|--translate] online_dir [...]

Checks, extracts, or translates .uno: command descriptions from the
LibreOffice XCU files.

Check whether all the commands in the menus have their descriptions in
unocommands.js:

    {program} --check /path/to/online

Update the unocommands.js by fetching the .uno: commands descriptions from the
core.git.  This is what you want to do after you add new .uno: commands or
dialogs to the menus:

    {program} --update /path/to/online /path/to/loffice

Update the translations of unocommands.js before releasing:

    {program} --translate /path/to/online /path/to/translations

"""
    print(message.format(program=sys.argv[0]))
    exit(1)

def commandsFromLine(line):
    """Extract uno commands name from lines like "  'Command1', 'Command2',"""
    commands = []

    inCommand = False
    command = ''
    for c in line:
        if c == "'":
            inCommand = not inCommand
            # command ended, collect it
            if not inCommand and command != '':
                commands += [command]
                command = ''
        elif inCommand:
            command += c

    return commands


# Extract uno commands name from lines like "  {uno: '.uno:Command3',"
def commandFromMenuLine(line):
    m = re.search(r"\b_UNO\('.uno:([^']*)'", line)
    if m:
        return [m.group(1)]

    m = re.search(r"\buno: *'\.uno:([^']*)'", line)
    if m:
        return [m.group(1)]

    m = re.search(r"'command': *'\.uno:([^']*)'", line)
    if m:
        return [m.group(1)]

    return []


# Extract all the uno commands we are using in the Online menu
def extractMenuCommands(path):
    commands = []

    # extract from the menu specifications
    f = open(path + '/browser/src/control/Control.Menubar.ts', 'r', encoding='utf-8')
    for line in f:
        if line.find("uno:") >= 0 and line.find("name:") < 0:
            commands += commandFromMenuLine(line)
        elif line.find("_UNO(") >= 0:
            commands += commandFromMenuLine(line)

    # may the list unique
    return set(commands)


# Extract all the uno commands we are using in the Online context menu
def extractContextCommands(path):
    commandsToIgnore = ["FontDialogForParagraph"]
    commands = []

    # extract from the comments whitelist
    f = open(path + '/browser/src/control/jsdialog/Definitions.MenuCommands.ts', 'r', encoding='utf-8')
    readingCommands = False
    for line in f:
        if line.find('UNOCOMMANDS_EXTRACT_START') >= 0:
            readingCommands = True
        elif line.find('UNOCOMMANDS_EXTRACT_END') >= 0:
            readingCommands = False
        elif readingCommands:
            commands += commandsFromLine(line)

    f = open(path + '/browser/src/control/Control.ColumnHeader.ts', 'r', encoding='utf-8')
    for line in f:
        if line.find("_UNO(") >= 0:
            commands += commandFromMenuLine(line)

    f = open(path + '/browser/src/control/Control.RowHeader.ts', 'r', encoding='utf-8')
    for line in f:
        if line.find("_UNO(") >= 0:
            commands += commandFromMenuLine(line)

    f = open(path + '/browser/src/control/Control.Tabs.js', 'r', encoding='utf-8')
    for line in f:
        if line.find("_UNO(") >= 0:
            commands += commandFromMenuLine(line)

    commands = [command for command in commands
                if command not in commandsToIgnore]
    # may the list unique
    return set(commands)


# Extract all the uno commands we are using in the Online toolbar
def extractToolbarCommands(path):
    commands = []

    # extract from the toolbars
    f = open(path + '/browser/src/control/Control.Toolbar.js', 'r', encoding='utf-8')
    for line in f:
        if line.find("_UNO(") >= 0:
            commands += commandFromMenuLine(line)

    f = open(path + '/browser/src/control/Control.MobileBottomBar.js', 'r', encoding='utf-8')
    for line in f:
        if line.find("_UNO(") >= 0:
            commands += commandFromMenuLine(line)

    f = open(path + '/browser/src/control/Control.MobileTopBar.ts', 'r', encoding='utf-8')
    for line in f:
        if line.find("_UNO(") >= 0:
            commands += commandFromMenuLine(line)

    f = open(path + '/browser/src/control/Control.MobileWizardBuilder.js', 'r', encoding='utf-8')
    for line in f:
        if line.find("_UNO(") >= 0:
            commands += commandFromMenuLine(line)

    f = open(path +
             '/browser/src/control/Control.NotebookbarBuilder.js', 'r', encoding='utf-8')
    for line in f:
        if line.find("_UNO(") >= 0:
            commands += commandFromMenuLine(line)

    f = open(path + '/browser/src/control/Control.Notebookbar.js', 'r', encoding='utf-8')
    for line in f:
        if line.find("_UNO(") >= 0:
            commands += commandFromMenuLine(line)

    f = open(path + '/browser/src/control/Control.NotebookbarWriter.js', 'r', encoding='utf-8')
    for line in f:
        if line.find("_UNO(") >= 0:
            commands += commandFromMenuLine(line)

    f = open(path + '/browser/src/control/Notebookbar.WriterReferencesTab.ts', 'r', encoding='utf-8')
    for line in f:
        if line.find("_UNO(") >= 0:
            commands += commandFromMenuLine(line)

    f = open(path + '/browser/src/control/Control.NotebookbarCalc.js', 'r', encoding='utf-8')
    for line in f:
        if line.find("_UNO(") >= 0:
            commands += commandFromMenuLine(line)

    f = open(path +
             '/browser/src/control/Control.NotebookbarImpress.js', 'r', encoding='utf-8')
    for line in f:
        if line.find("_UNO(") >= 0:
            commands += commandFromMenuLine(line)

    f = open(path + '/browser/src/control/Control.NotebookbarDraw.js', 'r', encoding='utf-8')
    for line in f:
        if line.find("_UNO(") >= 0:
            commands += commandFromMenuLine(line)

    f = open(path + '/browser/src/control/Control.PresentationBar.js', 'r', encoding='utf-8')
    for line in f:
        if line.find("_UNO(") >= 0:
            commands += commandFromMenuLine(line)

    f = open(path + '/browser/src/control/Control.MobileSearchBar.ts', 'r', encoding='utf-8')
    for line in f:
        if line.find("_UNO(") >= 0:
            commands += commandFromMenuLine(line)

    f = open(path + '/browser/src/control/Control.StatusBar.js', 'r', encoding='utf-8')
    for line in f:
        if line.find("_UNO(") >= 0:
            commands += commandFromMenuLine(line)

    f = open(path + '/browser/src/control/Control.TopToolbar.js', 'r', encoding='utf-8')
    for line in f:
        if line.find("_UNO(") >= 0:
            commands += commandFromMenuLine(line)

    f = open(path + '/browser/src/control/Control.AboutDialog.ts', 'r', encoding='utf-8')
    for line in f:
        if line.find("_UNO(") >= 0:
            commands += commandFromMenuLine(line)

    f = open(path + '/browser/src/control/Notebookbar.CalcTableTab.ts', 'r', encoding='utf-8')
    for line in f:
        if line.find("_UNO(") >= 0:
            commands += commandFromMenuLine(line)

    # may the list unique
    return set(commands)


# Extract uno commands from combobox widgets in notebookbar definitions.
# These use 'command': '.uno:...' without a _UNO() call, so extractToolbarCommands misses them.
def extractComboboxCommands(path):
    commands = []
    files = [
        '/browser/src/control/Control.NotebookbarWriter.js',
        '/browser/src/control/Control.NotebookbarCalc.js',
        '/browser/src/control/Control.NotebookbarImpress.js',
        '/browser/src/control/Control.NotebookbarDraw.js',
    ]
    for fname in files:
        f = open(path + fname, 'r', encoding='utf-8')
        inCombobox = False
        for line in f:
            if "'type': 'combobox'" in line:
                inCombobox = True
            elif inCombobox and "'command':" in line:
                commands += commandFromMenuLine(line)
                inCombobox = False
            elif inCombobox and '}' in line:
                inCombobox = False
    return set(commands)


def extractMenubuttonCommands(path):
    commands = []

    f = open(path + '/browser/src/control/jsdialog/Definitions.Menu.ts', 'r', encoding='utf-8')
    for line in f:
        if line.find("_UNO(") >= 0:
            commands += commandFromMenuLine(line)

    # may the list unique
    return set(commands)


# Create mapping between the commands and appropriate strings
def collectCommandsFromXCU(xcu, descriptions, commands, label, type):
    root = etree.parse(xcu)
    nodes = root.xpath("/oor:component-data/node/node/node", namespaces={
        'oor': 'http://openoffice.org/2001/registry',
        })
    for node in nodes:
        # extract the uno command name
        unoCommand = node.get('{http://openoffice.org/2001/registry}name')
        unoCommand = unoCommand[5:]

        if unoCommand in commands:
            # normal labels
            textElement = node.xpath('prop[@oor:name="' + label + '"]/value',
                                     namespaces={'oor':
                                                 'http://open' +
                                                 'office.org/2001/registry', })
            if len(textElement) == 1:
                # extract the uno command's English text
                text = ''.join(textElement[0].itertext())
                command = {}
                if unoCommand in descriptions.keys():
                    command = descriptions[unoCommand]

                if not type in command:
                    command[type] = {}

                menuType = 'menu'
                if label == 'PopupLabel' or label == 'TooltipLabel':
                    menuType = 'context'

                if menuType in command[type]:
                    continue

                command[type][menuType] = text

                descriptions[unoCommand] = command

    return descriptions


# Print commands from all the XCU files, and collect them too
def writeUnocommandsJS(
        onlineDir, lofficeDir, menuCommands, contextCommands, toolbarCommands,
        menubuttonCommands, comboboxCommands):

    descriptions = {}
    dir = lofficeDir + '/officecfg/registry/data/org/openoffice/Office/UI'
    for file in os.listdir(dir):
        if file.endswith('.xcu'):
            type = 'global'
            if file.startswith('Writer'):
                type = 'text'
            elif file.startswith('Calc'):
                type = 'spreadsheet'
            elif file.startswith('DrawImpress'):
                type = 'presentation'

            # main menu
            descriptions = collectCommandsFromXCU(os.path.join(dir, file),
                                                  descriptions, menuCommands,
                                                  'ContextLabel', type)
            descriptions = collectCommandsFromXCU(os.path.join(dir, file),
                                                  descriptions,
                                                  contextCommands,
                                                  'ContextLabel', type)

            # right-click menu
            descriptions = collectCommandsFromXCU(os.path.join(dir, file),
                                                  descriptions,
                                                  contextCommands,
                                                  'PopupLabel', type)

            # toolbar
            descriptions = collectCommandsFromXCU(os.path.join(dir, file),
                                                  descriptions,
                                                  toolbarCommands,
                                                  'PopupLabel', type)
            descriptions = collectCommandsFromXCU(os.path.join(dir, file),
                                                  descriptions,
                                                  toolbarCommands,
                                                  'TooltipLabel', type)

            # fallbacks
            descriptions = collectCommandsFromXCU(os.path.join(dir, file),
                                                  descriptions, menuCommands,
                                                  'Label', type)
            descriptions = collectCommandsFromXCU(os.path.join(dir, file),
                                                  descriptions,
                                                  contextCommands,
                                                  'Label', type)
            descriptions = collectCommandsFromXCU(os.path.join(dir, file),
                                                  descriptions,
                                                  toolbarCommands,
                                                  'Label', type)
            descriptions = collectCommandsFromXCU(os.path.join(dir, file),
                                                  descriptions,
                                                  menubuttonCommands,
                                                  'Label', type)
            descriptions = collectCommandsFromXCU(os.path.join(dir, file),
                                                  descriptions,
                                                  comboboxCommands,
                                                  'Label', type)

    # output the unocommands.js
    f = open(onlineDir + '/browser/src/unocommands.js', 'w',
             encoding='utf-8')
    f.write('''// Don't modify, generated using unocommands.py

var unoCommandsArray = {\n''')

    for key in sorted(descriptions.keys()):
        f.write('\t\'' + key + '\':{')
        for type in sorted(descriptions[key].keys()):
            f.write(type + ':{')
            for menuType in sorted(descriptions[key][type].keys()):
                f.write(menuType + ":_('" + descriptions[key][type][menuType]
                        + "'),")
            f.write('},')
        f.write('},\n')

    f.write('''};

window._UNO = function(string, component, isContext) {
\tvar command = string.substr(5);
\tvar context = 'menu';
\tif (isContext === true) {
\t\tcontext = 'context';
\t}
\tvar entry = unoCommandsArray[command];
\tif (entry === undefined) {
\t\treturn command;
\t}
\tif (component == 'drawing') {
\t\tcomponent = 'presentation';
\t}
\tvar componentEntry = entry[component];
\tif (componentEntry === undefined) {
\t\tcomponentEntry = entry['global'];
\t\tif (componentEntry === undefined) {
\t\t\treturn command;
\t\t}
\t}
\tvar text = componentEntry[context];
\tif (text === undefined) {
\t\ttext = componentEntry['menu'];
\t\tif (text === undefined) {
\t\t\treturn command;
\t\t}
\t}

\treturn this.removeAccessKey(text);
}

window.removeAccessKey = function(text) {
\t// Remove access key markers from translated strings
\t// 1. access key in parenthesis in case of non-latin scripts
\ttext = text.replace(/\\(~[A-Za-z]\\)/, '');
\t// 2. remove normal access key
\ttext = text.replace('~', '');

\treturn text;
}\n''')

    return descriptions


# Read the uno commands present in the unocommands.js for checking
def parseUnocommandsJS(onlineDir):
    strings = {}

    f = open(onlineDir + '/browser/src/unocommands.js', 'r',
             encoding='utf-8')
    for line in f:
        m = re.match(r"\t\'([^:]*)\':.*", line)
        if m:
            command = m.group(1)

            n = re.findall(r"_\('([^']*)'\)", line)
            if n:
                strings[command] = n

    return strings


# Generate translation JSONs for the .uno: commands
def writeTranslations(onlineDir, translationsDir, strings):
    keys = set(strings.keys())

    dir = translationsDir + '/source/'
    for lang in os.listdir(dir):
        poFile = dir + lang + '/officecfg/registry/data/org/openoffice/Office/UI.po'
        if not os.path.isfile(poFile):
            continue

        sys.stderr.write('Generating ' + lang + '...\n')

        po = polib.pofile(poFile, autodetect_encoding=False,
                          encoding="utf-8", wrapwidth=-1)

        translations = {}
        for entry in po.translated_entries():
            m = re.search(r"\.uno:([^\n]*)\n", entry.msgctxt)
            if m:
                command = m.group(1)
                if command in keys:
                    for text in strings[command]:
                        if text == entry.msgid:
                            translations[entry.msgid] = entry.msgstr

        f = open(onlineDir + '/browser/l10n/uno/' +
                 lang + '.json', 'w', encoding='utf-8')
        f.write('{\n')

        writeComma = False

        for key in sorted(translations.keys()):
            if writeComma:
                f.write(',\n')
            else:
                writeComma = True

            value = translations[key]

            # Remove trailing access keys like " (~T)", they are unused
            value = re.sub(r' \(~[A-Z]\)', '', value)

            # Sometimes translation contains ~ when source does not, and it may leak
            if '~' not in key:
                value = value.replace('~', '')

            f.write('"' + key.replace('"', '\\\"') + '":"' +
                    value.replace('"', '\\\"') + '"')

        f.write('\n}\n')


if __name__ == "__main__":
    if len(sys.argv) < 2:
        usageAndExit()

    check = False
    translate = False
    onlineDir = ''
    lofficeDir = ''
    translationsDir = ''
    if (sys.argv[1] == '--check'):
        if len(sys.argv) != 3:
            usageAndExit()

        check = True
        onlineDir = sys.argv[2]
    elif (sys.argv[1] == '--translate'):
        translate = True
        if len(sys.argv) != 4:
            usageAndExit()

        onlineDir = sys.argv[2]
        translationsDir = sys.argv[3]
    elif (sys.argv[1] == "--update"):
        if len(sys.argv) != 4:
            usageAndExit()

        onlineDir = sys.argv[2]
        lofficeDir = os.path.abspath(sys.argv[3])
        if not os.path.isdir(lofficeDir):
            sys.stderr.write("ERROR: invalid loffice parameter '{}' is not a dir\n".format(sys.argv[3]))
            exit(1)
    else:
        usageAndExit()

    onlineDir = os.path.abspath(onlineDir)
    if not os.path.isdir(onlineDir):
        sys.stderr.write("ERROR: invalid online_dir parameter '{}' is not a dir\n".format(sys.argv[2]))
        exit(1)

    menuCommands = extractMenuCommands(onlineDir)
    contextCommands = extractContextCommands(onlineDir)
    toolbarCommands = extractToolbarCommands(onlineDir)
    menubuttonCommands = extractMenubuttonCommands(onlineDir)
    comboboxCommands = extractComboboxCommands(onlineDir)

    processedCommands = set([])
    parsed = {}
    if (check or translate):
        parsed = parseUnocommandsJS(onlineDir)
        processedCommands = set(parsed.keys())
    else:
        written = writeUnocommandsJS(onlineDir, lofficeDir, menuCommands,
                                     contextCommands, toolbarCommands,
                                     menubuttonCommands, comboboxCommands)
        processedCommands = set(written.keys())

    # check that we have translations for everything
    requiredCommands = (menuCommands | contextCommands | toolbarCommands | menubuttonCommands | comboboxCommands)
    dif = requiredCommands - processedCommands

    if len(dif) > 0:
        if check:
            sys.stderr.write("ERROR: The following commands are not covered in unocommands.js, run scripts/unocommands.py --update:\n\n")
        else:
            sys.stderr.write("ERROR: The following commands are referenced in Online but have no description in the core XCU files:\n\n")
        sys.stderr.write(".uno:" + '\n.uno:'.join(sorted(dif)) + "\n\n")
        exit(1)

    if (translate):
        writeTranslations(onlineDir, translationsDir, parsed)

# vim: set shiftwidth=4 softtabstop=4 expandtab:

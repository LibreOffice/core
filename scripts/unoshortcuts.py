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
from lxml import etree

from unocommands import (extractMenuCommands, extractContextCommands,
                         extractToolbarCommands, extractMenubuttonCommands,
                         extractComboboxCommands)


def usageAndExit():
    message = """usage: {program} [--check|--update] online_dir [...]

Extracts .uno: keyboard shortcuts from the LibreOffice Accelerators.xcu
file for all commands used in Online.

Check that unoshortcuts.js exists and is valid:

    {program} --check /path/to/online

Update unoshortcuts.js by reading Accelerators.xcu from core:

    {program} --update /path/to/online /path/to/core

"""
    print(message.format(program=sys.argv[0]))
    exit(1)


# Modifier suffixes appended by lcl_getKeyString in
# framework/source/accelerators/acceleratorconfiguration.cxx.
# Order: _SHIFT, _MOD1, _MOD2, _MOD3
MODIFIER_SUFFIXES = {'SHIFT', 'MOD1', 'MOD2', 'MOD3'}

# Display names for modifier tokens, in the order they appear in the
# display string (Ctrl before Shift before Alt).
MODIFIER_DISPLAY = [
    ('MOD1', 'Ctrl'),
    ('SHIFT', 'Shift'),
    ('MOD2', 'Alt'),
]

# Display names for special base keys.  Derived from the KEY_* identifiers
# in framework/source/accelerators/keymapping.cxx (with the KEY_ prefix
# stripped).  Only keys that need a display name different from the
# identifier are listed here.
SPECIAL_KEY_DISPLAY = {
    'RETURN': 'Return',
    'ESCAPE': 'Escape',
    'TAB': 'Tab',
    'BACKSPACE': 'Backspace',
    'SPACE': 'Space',
    'INSERT': 'Insert',
    'DELETE': 'Delete',
    'PAGEUP': 'PageUp',
    'PAGEDOWN': 'PageDown',
    'HOME': 'Home',
    'END': 'End',
    'UP': 'Up',
    'DOWN': 'Down',
    'LEFT': 'Left',
    'RIGHT': 'Right',
    'ADD': '+',
    'SUBTRACT': '-',
    'MULTIPLY': '*',
    'DIVIDE': '/',
    'POINT': '.',
    'COMMA': ',',
    'LESS': '<',
    'GREATER': '>',
    'EQUAL': '=',
    'NUMBERSIGN': '#',
    'COLON': ':',
    'SEMICOLON': ';',
    'TILDE': '~',
    'QUOTELEFT': '`',
    'QUOTERIGHT': "'",
    'BRACKETLEFT': '[',
    'BRACKETRIGHT': ']',
    'RIGHTCURLYBRACKET': '}',
    'DECIMAL': 'Decimal',
}

# Commands not present in Accelerators.xcu that must be provided manually.
MANUAL_FALLBACKS = {
    '.uno:InsertFootnote': 'Ctrl+Alt+F',
    '.uno:InsertEndnote': 'Ctrl+Alt+D',
    '.uno:KeyboardShortcuts': 'Ctrl+Shift+?',
    '.uno:Strikeout': 'Ctrl+Alt+5',
}

# Groups of UNO commands that share the same shortcut.  When a shortcut is
# found for any member, it is propagated to all members of the group.
EQUIVALENT_GROUPS = [
    ['.uno:LeftPara', '.uno:AlignLeft', '.uno:CommonAlignLeft'],
    ['.uno:CenterPara', '.uno:AlignHorizontalCenter',
     '.uno:CommonAlignHorizontalCenter'],
    ['.uno:RightPara', '.uno:AlignRight', '.uno:CommonAlignRight'],
    ['.uno:JustifyPara', '.uno:AlignBlock', '.uno:CommonAlignJustified'],
]


def parseKeyName(nodeName):
    """Convert an XCU key node name to a display string.

    The node name has the form KEY[_SHIFT][_MOD1][_MOD2][_MOD3] where the
    KEY_ prefix from keymapping.cxx has already been stripped.

    Returns (displayString, modifierCount), or None if the shortcut cannot
    be represented in the browser (e.g. includes MOD3).
    """
    parts = nodeName.split('_')

    # Peel modifier tokens from the right.
    modifiers = set()
    i = len(parts) - 1
    while i >= 0:
        if parts[i] in MODIFIER_SUFFIXES:
            modifiers.add(parts[i])
            i -= 1
        else:
            break
    baseKey = '_'.join(parts[:i + 1])

    # MOD3 has no reliable browser equivalent (was historically Mac Ctrl;
    # unbound on Linux/Windows).  Skip rather than emit a shortcut that
    # silently drops the modifier.
    if 'MOD3' in modifiers:
        return None

    # Convert base key to display form.
    if baseKey in SPECIAL_KEY_DISPLAY:
        displayKey = SPECIAL_KEY_DISPLAY[baseKey]
    elif len(baseKey) == 1:
        # Single letter or digit - keep as-is.
        displayKey = baseKey
    elif baseKey.startswith('F') and baseKey[1:].isdigit():
        # Function keys F1-F26.
        displayKey = baseKey
    else:
        displayKey = baseKey

    # Build display string: modifiers in Ctrl, Shift, Alt order.
    displayParts = []
    for token, label in MODIFIER_DISPLAY:
        if token in modifiers:
            displayParts.append(label)
    displayParts.append(displayKey)

    return ('+'.join(displayParts), len(modifiers))


def extractAllOnlineCommands(onlinePath):
    """Collect all .uno: commands used in Online's menus, toolbars, etc.

    Uses the same extraction functions as unocommands.py.  Returns a set
    of command names with the .uno: prefix.
    """
    commands = set()
    commands |= extractMenuCommands(onlinePath)
    commands |= extractContextCommands(onlinePath)
    commands |= extractToolbarCommands(onlinePath)
    commands |= extractMenubuttonCommands(onlinePath)
    commands |= extractComboboxCommands(onlinePath)

    # Add manual fallback commands so they always appear.
    for cmd in MANUAL_FALLBACKS:
        commands.add(cmd[5:])  # strip .uno: prefix

    # Add all members of equivalent groups.
    for group in EQUIVALENT_GROUPS:
        for cmd in group:
            commands.add(cmd[5:])

    return {'.uno:' + c for c in commands}


def extractShortcutsFromXCU(xcuPath, onlineCommands):
    """Parse Accelerators.xcu and extract command -> shortcut candidates.

    Only extracts shortcuts for commands in onlineCommands.
    Returns a dict mapping command strings to lists of
    (shortcutDisplay, modifierCount) tuples.
    """
    tree = etree.parse(xcuPath)
    root = tree.getroot()
    ns = {'oor': 'http://openoffice.org/2001/registry'}
    installNs = 'http://openoffice.org/2004/installation'

    shortcuts = {}

    def processSection(section):
        for entry in section.xpath('node', namespaces=ns):
            keyName = entry.get('{http://openoffice.org/2001/registry}name')
            if keyName is None:
                continue

            # Find the en-US command value.
            command = None
            for val in entry.xpath('prop[@oor:name="Command"]/value',
                                   namespaces=ns):
                lang = val.get('{http://www.w3.org/XML/1998/namespace}lang',
                               '')
                if lang != 'en-US':
                    continue

                # Skip macOS-only entries.
                installModule = val.get('{%s}module' % installNs, '')
                if installModule == 'macosx':
                    continue

                text = ''.join(val.itertext()).strip()
                if text:
                    command = text
                    break

            if not command:
                continue

            # Strip parameters (e.g., ?InitialFocusReplace:bool=true).
            paramIdx = command.find('?')
            if paramIdx >= 0:
                command = command[:paramIdx]

            if command not in onlineCommands:
                continue

            parsed = parseKeyName(keyName)
            if parsed is None:
                continue
            display, modCount = parsed
            if command not in shortcuts:
                shortcuts[command] = []
            shortcuts[command].append((display, modCount))

    # Process PrimaryKeys/Global first, then each module.
    for section in root.xpath(
            'node[@oor:name="PrimaryKeys"]/node[@oor:name="Global"]',
            namespaces=ns):
        processSection(section)

    for module in root.xpath(
            'node[@oor:name="PrimaryKeys"]/node[@oor:name="Modules"]/node',
            namespaces=ns):
        processSection(module)

    return shortcuts


def pickCanonicalShortcut(candidates):
    """Pick the best shortcut from a list of (display, modCount) tuples.

    Prefers the most frequently occurring shortcut (the one that appears
    across the most modules), then fewest modifiers, then alphabetical.
    """
    # Count occurrences of each display string.
    counts = {}
    for display, modCount in candidates:
        if display not in counts:
            counts[display] = (0, modCount)
        counts[display] = (counts[display][0] + 1, modCount)

    # Sort by: most occurrences (descending), fewest modifiers, alphabetical.
    ranked = sorted(counts.items(),
                    key=lambda item: (-item[1][0], item[1][1], item[0]))
    return ranked[0][0]


def applyEquivalentGroups(shortcuts):
    """Propagate shortcuts within equivalent command groups."""
    for group in EQUIVALENT_GROUPS:
        # Find the shortcut for any member of the group.
        shortcut = None
        for cmd in group:
            if cmd in shortcuts:
                shortcut = shortcuts[cmd]
                break
        if shortcut:
            for cmd in group:
                if cmd not in shortcuts:
                    shortcuts[cmd] = shortcut


def generateShortcuts(onlinePath, corePath):
    """Extract online commands and their shortcuts from core.

    Returns a dict mapping .uno: commands to shortcut display strings.
    """
    xcuPath = os.path.join(
        corePath,
        'officecfg/registry/data/org/openoffice/Office/Accelerators.xcu')
    if not os.path.isfile(xcuPath):
        sys.stderr.write(
            "ERROR: Accelerators.xcu not found at %s\n" % xcuPath)
        exit(1)

    onlineCommands = extractAllOnlineCommands(onlinePath)

    # Extract from XCU.
    candidates = extractShortcutsFromXCU(xcuPath, onlineCommands)

    # Pick canonical shortcut for each command.
    shortcuts = {}
    for cmd, cands in candidates.items():
        shortcuts[cmd] = pickCanonicalShortcut(cands)

    # Apply manual fallbacks for commands not in the XCU.
    for cmd, shortcut in MANUAL_FALLBACKS.items():
        if cmd not in shortcuts:
            shortcuts[cmd] = shortcut

    # Propagate within equivalent groups.
    applyEquivalentGroups(shortcuts)

    return shortcuts


def writeUnoshortcutsJS(onlinePath, shortcuts):
    """Write the generated unoshortcuts.js file."""
    outPath = os.path.join(onlinePath, 'browser/src/unoshortcuts.js')
    with open(outPath, 'w', encoding='utf-8') as f:
        f.write("// Don't modify, generated using scripts/unoshortcuts.py\n")
        f.write("/* eslint-disable no-unused-vars */\n\n")
        f.write("var unoShortcutsMap = {\n")

        for cmd in sorted(shortcuts.keys()):
            f.write("\t'%s': '%s',\n" % (cmd, shortcuts[cmd]))

        f.write("};\n")


def parseUnoshortcutsJS(onlinePath):
    """Read the existing unoshortcuts.js and return command -> shortcut map."""
    jsPath = os.path.join(onlinePath, 'browser/src/unoshortcuts.js')
    shortcuts = {}
    with open(jsPath, 'r', encoding='utf-8') as f:
        for line in f:
            m = re.match(r"\t'([^']+)': '([^']+)',", line)
            if m:
                shortcuts[m.group(1)] = m.group(2)
    return shortcuts


if __name__ == "__main__":
    if len(sys.argv) < 2:
        usageAndExit()

    if sys.argv[1] == '--check':
        if len(sys.argv) != 3:
            usageAndExit()

        onlineDir = os.path.abspath(sys.argv[2])
        existing = parseUnoshortcutsJS(onlineDir)

        if not existing:
            sys.stderr.write(
                "ERROR: unoshortcuts.js is empty or missing, "
                "run scripts/unoshortcuts.py --update\n")
            exit(1)

    elif sys.argv[1] == '--update':
        if len(sys.argv) != 4:
            usageAndExit()

        onlineDir = os.path.abspath(sys.argv[2])
        coreDir = os.path.abspath(sys.argv[3])

        if not os.path.isdir(coreDir):
            sys.stderr.write(
                "ERROR: invalid core parameter '%s' is not a dir\n"
                % sys.argv[3])
            exit(1)

        shortcuts = generateShortcuts(onlineDir, coreDir)

        writeUnoshortcutsJS(onlineDir, shortcuts)
        sys.stderr.write("Updated browser/src/unoshortcuts.js "
                         "(%d shortcuts)\n" % len(shortcuts))

    else:
        usageAndExit()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

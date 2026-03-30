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


# X11 keysyms for modifier keys, used to parse keysymnames.cxx.
MODIFIER_KEYSYMS = {
    'XK_Control_L': 'Ctrl',
    'XK_Control_R': 'Ctrl',
    'XK_Shift_L': 'Shift',
    'XK_Shift_R': 'Shift',
    'XK_Alt_L': 'Alt',
}

# keysymnames.cxx uses deprecated language codes that don't match the
# locale codes used at runtime. Map each deprecated code to the modern
# ISO 639-1 codes the locale may start with.
# Currently only needed for Norwegian, which has two written standards (Bokmaal and Nynorsk).
LANG_ALIASES = {
    'no': ['nb', 'nn'],  # Norwegian: Bokmaal / Nynorsk
}


def extractModifierL10N(corePath):
    """Extract localized modifier key names from keysymnames.cxx.

    Returns a dict mapping language code to a dict of
    {EnglishModifier: LocalizedName} for modifiers that differ from English.
    """
    cxxPath = os.path.join(
        corePath, 'vcl/unx/generic/app/keysymnames.cxx')
    if not os.path.isfile(cxxPath):
        return {}

    with open(cxxPath, 'r', encoding='utf-8') as f:
        content = f.read()

    # Parse the aKeyboards[] array to find language -> array name mapping.
    langArrays = {}
    for m in re.finditer(
            r'\{\s*"(\w+)"\s*,\s*aImplReplacements_(\w+)', content):
        langArrays[m.group(2)] = m.group(1)

    result = {}
    for arrayName, langCode in langArrays.items():
        pattern = (r'aImplReplacements_%s\[\]\s*=\s*\{([^;]+)\};'
                   % re.escape(arrayName))
        m = re.search(pattern, content, re.DOTALL)
        if not m:
            continue

        body = m.group(1)
        replacements = {}
        for entry in re.finditer(
                r'\{\s*(\w+)\s*,\s*"([^"]+)"\s*\}', body):
            keysym = entry.group(1)
            localName = entry.group(2)
            # Decode UTF-8 octal escapes (e.g., \303\272 -> UTF-8 bytes).
            localName = re.sub(
                r'\\(\d{3})',
                lambda m2: chr(int(m2.group(1), 8)),
                localName)
            # Re-encode as proper UTF-8.
            localName = localName.encode('latin-1').decode('utf-8')

            if keysym in MODIFIER_KEYSYMS:
                engName = MODIFIER_KEYSYMS[keysym]
                if localName != engName:
                    replacements[engName] = localName

        if replacements:
            for code in LANG_ALIASES.get(langCode, [langCode]):
                result[code] = replacements

    return result


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

    Returns (shortcuts, modifierL10N) where shortcuts maps .uno: commands
    to shortcut display strings and modifierL10N maps language codes to
    modifier name replacement dicts.
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

    modifierL10N = extractModifierL10N(corePath)

    return shortcuts, modifierL10N


def writeUnoshortcutsJS(onlinePath, shortcuts, modifierL10N):
    """Write the generated unoshortcuts.js file."""
    outPath = os.path.join(onlinePath, 'browser/src/unoshortcuts.js')
    with open(outPath, 'w', encoding='utf-8') as f:
        f.write("// Don't modify, generated using scripts/unoshortcuts.py\n")
        f.write("/* eslint-disable no-unused-vars */\n\n")
        f.write("var unoShortcutsMap = {\n")
        for cmd in sorted(shortcuts.keys()):
            f.write("\t'%s': '%s',\n" % (cmd, shortcuts[cmd]))
        f.write("};\n\n")

        # Localized modifier key names, extracted from
        # vcl/unx/generic/app/keysymnames.cxx in core.
        f.write("var unoShortcutsModifierL10N = {\n")
        for lang in sorted(modifierL10N.keys()):
            parts = []
            for eng in ('Ctrl', 'Shift', 'Alt'):
                if eng in modifierL10N[lang]:
                    parts.append("'%s': '%s'" % (eng, modifierL10N[lang][eng]))
            if parts:
                f.write("\t'%s': {%s},\n" % (lang, ', '.join(parts)))
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

        shortcuts, modifierL10N = generateShortcuts(onlineDir, coreDir)

        writeUnoshortcutsJS(onlineDir, shortcuts, modifierL10N)
        sys.stderr.write("Updated browser/src/unoshortcuts.js "
                         "(%d shortcuts, %d modifier l10n languages)\n"
                         % (len(shortcuts), len(modifierL10N)))

    else:
        usageAndExit()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

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

# Keys that the browser intercepts and that cannot be used as shortcuts in
# Online.  Shortcuts using these base keys are excluded from the generated
# file unless they include Ctrl (which usually prevents the browser
# default).
BROWSER_INTERCEPTED_KEYS = {'F12'}


def parseKeyName(nodeName):
    """Convert an XCU key node name to a display string.

    The node name has the form KEY[_SHIFT][_MOD1][_MOD2][_MOD3] where the
    KEY_ prefix from keymapping.cxx has already been stripped.

    Returns (baseKey, modifiers, displayString, modifierCount) where
    baseKey is the raw key identifier and modifiers is a set of modifier
    tokens.
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

    return (baseKey, modifiers, '+'.join(displayParts), len(modifiers))


def isBrowserIntercepted(baseKey, modifiers):
    """Check if a shortcut would be intercepted by the browser."""
    if baseKey in BROWSER_INTERCEPTED_KEYS and 'MOD1' not in modifiers:
        return True
    # MOD3 has no reliable browser equivalent (was historically Mac Ctrl;
    # unbound on Linux/Windows).  Skip rather than emit a shortcut that
    # silently drops the modifier.
    if 'MOD3' in modifiers:
        return True
    return False


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
    """Parse Accelerators.xcu and extract en-US shortcut candidates.

    Only extracts shortcuts for commands in onlineCommands.
    Online uses en-US accelerator bindings regardless of UI language,
    so only en-US values are extracted.
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

            baseKey, modifiers, display, modCount = parseKeyName(keyName)

            # Skip shortcuts the browser intercepts.
            if isBrowserIntercepted(baseKey, modifiers):
                continue

            # Find the en-US command value.
            command = None
            for val in entry.xpath('prop[@oor:name="Command"]/value',
                                   namespaces=ns):
                lang = val.get(
                    '{http://www.w3.org/XML/1998/namespace}lang', '')
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

# Map Accelerators.xcu module node names to the docType values used by
# ShortcutDescriptor in Map.KeyboardShortcuts.ts.  Bindings from the
# Global section (or from multiple modules) have no docType and apply
# to all document types.
MODULE_DOCTYPE = {
    'com.sun.star.text.TextDocument': 'text',
    'com.sun.star.sheet.SpreadsheetDocument': 'spreadsheet',
    'com.sun.star.presentation.PresentationDocument': 'presentation',
    'com.sun.star.drawing.DrawingDocument': 'drawing',
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


# Modifier bitmask values matching the Mod enum in
# Map.KeyboardShortcuts.ts.
JS_MOD_CTRL = 1
JS_MOD_ALT = 2
JS_MOD_SHIFT = 4

# Mapping from XCU base key identifiers to event.key values used by
# the browser's KeyboardEvent.  For single letters, lowercase is used
# when Shift is not held, uppercase when Shift is held.
SPECIAL_KEY_EVENTKEY = {
    'RETURN': 'Enter',
    'ESCAPE': 'Escape',
    'TAB': 'Tab',
    'BACKSPACE': 'Backspace',
    'SPACE': ' ',
    'INSERT': 'Insert',
    'DELETE': 'Delete',
    'PAGEUP': 'PageUp',
    'PAGEDOWN': 'PageDown',
    'HOME': 'Home',
    'END': 'End',
    'UP': 'ArrowUp',
    'DOWN': 'ArrowDown',
    'LEFT': 'ArrowLeft',
    'RIGHT': 'ArrowRight',
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
    'DECIMAL': '.',
}


def keyNameToEventKey(baseKey, modifiers):
    """Convert a base key + modifiers to the browser event.key value.

    Returns the string that event.key would be for this key combination,
    or None if the key cannot be represented.
    """
    if baseKey in SPECIAL_KEY_EVENTKEY:
        return SPECIAL_KEY_EVENTKEY[baseKey]
    if baseKey.startswith('F') and baseKey[1:].isdigit():
        return baseKey  # F1, F12, etc.
    if len(baseKey) == 1 and baseKey.isalpha():
        # Letters: uppercase when Shift is held, lowercase otherwise.
        if 'SHIFT' in modifiers:
            return baseKey.upper()
        return baseKey.lower()
    if len(baseKey) == 1 and baseKey.isdigit():
        return baseKey
    return None


def modifiersToJsBitmask(modifiers):
    """Convert a set of XCU modifier tokens to a JS Mod bitmask."""
    bitmask = 0
    if 'MOD1' in modifiers:
        bitmask |= JS_MOD_CTRL
    if 'MOD2' in modifiers:
        bitmask |= JS_MOD_ALT
    if 'SHIFT' in modifiers:
        bitmask |= JS_MOD_SHIFT
    return bitmask


def extractAllLanguageBindings(xcuPath):
    """Parse Accelerators.xcu and extract per-language key->command mappings.

    Returns a dict: {lang: {keyNodeName: [(command, sectionName), ...]}}
    for all languages.  Only includes .uno: commands.  Multiple entries
    per key name are kept to support occurrence counting.
    """
    tree = etree.parse(xcuPath)
    root = tree.getroot()
    ns = {'oor': 'http://openoffice.org/2001/registry'}
    installNs = 'http://openoffice.org/2004/installation'

    # {lang: {keyNodeName: [(command, section)]}}
    bindings = {}

    def processSection(section):
        sectionName = section.get(
            '{http://openoffice.org/2001/registry}name', 'Global')
        for entry in section.xpath('node', namespaces=ns):
            keyName = entry.get('{http://openoffice.org/2001/registry}name')
            if keyName is None:
                continue

            for val in entry.xpath('prop[@oor:name="Command"]/value',
                                   namespaces=ns):
                lang = val.get(
                    '{http://www.w3.org/XML/1998/namespace}lang', '')
                if lang == 'x-no-translate' or lang == '':
                    continue

                # Skip macOS-only entries.
                installModule = val.get('{%s}module' % installNs, '')
                if installModule == 'macosx':
                    continue

                text = ''.join(val.itertext()).strip()
                if not text or not text.startswith('.uno:'):
                    continue

                # Strip parameters.
                paramIdx = text.find('?')
                if paramIdx >= 0:
                    text = text[:paramIdx]

                if lang not in bindings:
                    bindings[lang] = {}
                if keyName not in bindings[lang]:
                    bindings[lang][keyName] = []
                bindings[lang][keyName].append((text, sectionName))

    for section in root.xpath(
            'node[@oor:name="PrimaryKeys"]/node[@oor:name="Global"]',
            namespaces=ns):
        processSection(section)

    for module in root.xpath(
            'node[@oor:name="PrimaryKeys"]/node[@oor:name="Modules"]/node',
            namespaces=ns):
        processSection(module)

    return bindings


def buildL10NData(xcuPath, enUSShortcuts):
    """Build per-language tooltip and key binding overrides.

    Returns (l10nShortcuts, l10nKeyBindings) where:
    - l10nShortcuts: {lang: {command: displayString}} for tooltip display
    - l10nKeyBindings: {lang: [(eventKey, modBitmask, command), ...]}
      for keyboard shortcut handling
    """
    allBindings = extractAllLanguageBindings(xcuPath)
    enUSBindings = allBindings.get('en-US', {})

    l10nShortcuts = {}
    l10nKeyBindings = {}

    for lang, langBindings in allBindings.items():
        if lang == 'en-US':
            continue

        tooltipOverrides = {}
        keyBindingOverrides = []
        seenKeyBindings = set()

        for keyName, entries in langBindings.items():
            # Use the most common command for this key in this language.
            command = entries[0][0]

            # Get the most common en-US command for this key.
            enUSEntries = enUSBindings.get(keyName, [])
            enUSCommand = enUSEntries[0][0] if enUSEntries else None

            # Only emit key binding override if this key does something
            # different in this language compared to en-US.
            if enUSCommand and command != enUSCommand:
                baseKey, modifiers, display, modCount = parseKeyName(keyName)

                if isBrowserIntercepted(baseKey, modifiers):
                    continue

                eventKey = keyNameToEventKey(baseKey, modifiers)
                if eventKey is None:
                    continue

                modBitmask = modifiersToJsBitmask(modifiers)

                # Determine docType: only set when the binding comes
                # from exactly one non-Global module.  Bindings from
                # Global or spanning multiple modules apply to all
                # document types.
                sections = {s for (_cmd, s) in entries if _cmd == command}
                docType = None
                if 'Global' not in sections and len(sections) == 1:
                    docType = MODULE_DOCTYPE.get(next(iter(sections)))

                bindKey = (eventKey, modBitmask)
                if bindKey not in seenKeyBindings:
                    keyBindingOverrides.append(
                        (eventKey, modBitmask, command, docType))
                    seenKeyBindings.add(bindKey)

            # Build tooltip: for this language, find the key combo for
            # each command (reverse map).  Count each module occurrence
            # so pickCanonicalShortcut prefers widely-used bindings.
            baseKey, modifiers, display, modCount = parseKeyName(keyName)
            if isBrowserIntercepted(baseKey, modifiers):
                continue
            enUSDisplay = enUSShortcuts.get(command)
            if enUSDisplay and display != enUSDisplay:
                if command not in tooltipOverrides:
                    tooltipOverrides[command] = []
                # Add one candidate per module occurrence so the
                # occurrence count reflects how widely this binding
                # is used across modules.
                for _ in entries:
                    tooltipOverrides[command].append((display, modCount))

        if keyBindingOverrides:
            l10nKeyBindings[lang] = keyBindingOverrides
        if tooltipOverrides:
            resolved = {}
            for cmd, cands in tooltipOverrides.items():
                resolved[cmd] = pickCanonicalShortcut(cands)
            l10nShortcuts[lang] = resolved

    return l10nShortcuts, l10nKeyBindings


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

    Returns (shortcuts, l10nShortcuts, l10nKeyBindings, modifierL10N).
    """
    xcuPath = os.path.join(
        corePath,
        'officecfg/registry/data/org/openoffice/Office/Accelerators.xcu')
    if not os.path.isfile(xcuPath):
        sys.stderr.write(
            "ERROR: Accelerators.xcu not found at %s\n" % xcuPath)
        exit(1)

    onlineCommands = extractAllOnlineCommands(onlinePath)

    # Extract en-US shortcuts for tooltip display.
    candidates = extractShortcutsFromXCU(xcuPath, onlineCommands)

    shortcuts = {}
    for cmd, cands in candidates.items():
        shortcuts[cmd] = pickCanonicalShortcut(cands)

    for cmd, shortcut in MANUAL_FALLBACKS.items():
        if cmd not in shortcuts:
            shortcuts[cmd] = shortcut

    applyEquivalentGroups(shortcuts)

    # Extract per-language overrides for both tooltips and key bindings.
    l10nShortcuts, l10nKeyBindings = buildL10NData(xcuPath, shortcuts)

    modifierL10N = extractModifierL10N(corePath)

    return shortcuts, l10nShortcuts, l10nKeyBindings, modifierL10N


def writeUnoshortcutsJS(onlinePath, shortcuts, l10nShortcuts,
                        l10nKeyBindings, modifierL10N):
    """Write the generated unoshortcuts.js file."""
    outPath = os.path.join(onlinePath, 'browser/src/unoshortcuts.js')
    with open(outPath, 'w', encoding='utf-8') as f:
        f.write("// Don't modify, generated using scripts/unoshortcuts.py\n")
        f.write("/* eslint-disable no-unused-vars */\n\n")

        # en-US command -> display string (for tooltips).
        f.write("var unoShortcutsMap = {\n")
        for cmd in sorted(shortcuts.keys()):
            f.write("\t'%s': '%s',\n" % (cmd, shortcuts[cmd]))
        f.write("};\n\n")

        # Per-language tooltip overrides: command -> display string
        # where the shortcut differs from en-US.
        f.write("var unoShortcutsL10N = {\n")
        for lang in sorted(l10nShortcuts.keys()):
            f.write("\t'%s': {\n" % lang)
            for cmd in sorted(l10nShortcuts[lang].keys()):
                f.write("\t\t'%s': '%s',\n"
                        % (cmd, l10nShortcuts[lang][cmd]))
            f.write("\t},\n")
        f.write("};\n\n")

        # Per-language key bindings: key combo -> command where the
        # command differs from en-US for that key.  Used by
        # Map.KeyboardShortcuts.ts to register ShortcutDescriptors.
        f.write("var unoShortcutsL10NKeyBindings = {\n")
        for lang in sorted(l10nKeyBindings.keys()):
            f.write("\t'%s': [\n" % lang)
            # Sort for stable output.
            entries = sorted(l10nKeyBindings[lang],
                             key=lambda e: (e[2], e[1], e[0]))
            for eventKey, modBitmask, command, docType in entries:
                # Escape single quotes in key names.
                safeKey = eventKey.replace("'", "\\'")
                docTypePart = (", docType: '%s'" % docType) if docType else ''
                f.write("\t\t{key: '%s', modifier: %d, "
                        "unoAction: '%s'%s},\n"
                        % (safeKey, modBitmask, command, docTypePart))
            f.write("\t],\n")
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

        shortcuts, l10nSC, l10nKB, modL10N = generateShortcuts(
            onlineDir, coreDir)

        writeUnoshortcutsJS(onlineDir, shortcuts, l10nSC, l10nKB, modL10N)
        sys.stderr.write(
            "Updated browser/src/unoshortcuts.js "
            "(%d shortcuts, %d l10n tooltip languages, "
            "%d l10n keybinding languages, "
            "%d modifier l10n languages)\n"
            % (len(shortcuts), len(l10nSC), len(l10nKB), len(modL10N)))

    else:
        usageAndExit()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

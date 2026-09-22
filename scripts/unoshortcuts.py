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

# Non-.uno: commands that Online still wants tooltip shortcuts for so
# the JS-side aliases (search, home-search) can resolve through the
# standard locale-aware lookup.
EXTRA_NON_UNO_COMMANDS = {
    'vnd.sun.star.findbar:FocusToFindbar',
}

# Commands not present in Accelerators.xcu that must be provided manually.
MANUAL_FALLBACKS = {
    '.uno:KeyboardShortcuts': 'Ctrl+Shift+?',
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

    result = {'.uno:' + c for c in commands}
    # Non-uno protocol commands that Online aliases (search, home-search,
    # ...) to a shortcut.  Treat them as known so their xcu bindings are
    # picked up like any other command.
    result |= EXTRA_NON_UNO_COMMANDS
    return result


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


def resolveShortcuts(xcuPath, onlineCommands):
    """Work out the shortcut of every command, per language and document type.

    Mirrors what the kit answers a view: the accelerator configuration of
    that view's own language and module, the module's own bindings ahead
    of the ones every document type shares, and within a section the
    first key the configuration lists, which is what core's
    lcl_getPreferredKey picks.

    Returns ({lang: {docType: {command: displayString}}},
    {lang: {docType: {displayString}}}), the second being every key
    combination bound to anything at all there, which is what says
    whether a command may keep a shortcut it would otherwise inherit.
    """
    tree = etree.parse(xcuPath)
    root = tree.getroot()
    ns = {'oor': 'http://openoffice.org/2001/registry'}
    installNs = 'http://openoffice.org/2004/installation'

    # section name -> [(display, {lang: command})], in configuration order.
    sections = {}
    languages = set()

    def readSection(section, name):
        entries = sections.setdefault(name, [])
        for entry in section.xpath('node', namespaces=ns):
            keyName = entry.get('{http://openoffice.org/2001/registry}name')
            if keyName is None:
                continue
            baseKey, modifiers, display, modCount = parseKeyName(keyName)
            if isBrowserIntercepted(baseKey, modifiers):
                continue

            commands = {}
            for val in entry.xpath('prop[@oor:name="Command"]/value',
                                   namespaces=ns):
                lang = val.get(
                    '{http://www.w3.org/XML/1998/namespace}lang', '')
                if lang == 'x-no-translate' or not lang:
                    continue
                if val.get('{%s}module' % installNs, '') == 'macosx':
                    continue
                text = ''.join(val.itertext()).strip()
                paramIdx = text.find('?')
                if paramIdx >= 0:
                    text = text[:paramIdx]
                commands[lang] = text
                languages.add(lang)

            if commands:
                entries.append((display, commands))

    for section in root.xpath(
            'node[@oor:name="PrimaryKeys"]/node[@oor:name="Global"]',
            namespaces=ns):
        readSection(section, 'Global')
    for module in root.xpath(
            'node[@oor:name="PrimaryKeys"]/node[@oor:name="Modules"]/node',
            namespaces=ns):
        readSection(module, module.get(
            '{http://openoffice.org/2001/registry}name'))

    moduleOf = {docType: name for name, docType in MODULE_DOCTYPE.items()}
    resolved = {}
    bound = {}
    for lang in sorted(languages):
        perDocType = {}
        perDocTypeBound = {}
        for docType in sorted(MODULE_DOCTYPE.values()):
            labels = {}
            taken = set()
            # A key this language names itself comes first, so a French
            # menu offers Ctrl+G for bold rather than the Ctrl+B it also
            # inherits.  Then the module's own keys ahead of the shared
            # ones, and within a section the order the configuration
            # lists them.
            for ownLanguage in (True, False):
                for name in (moduleOf[docType], 'Global'):
                    for display, commands in sections.get(name, []):
                        if ownLanguage and lang not in commands:
                            continue
                        command = commands.get(lang, commands.get('en-US'))
                        if not command:
                            continue
                        taken.add(display)
                        if (not command.startswith('.uno:')
                                and command not in EXTRA_NON_UNO_COMMANDS):
                            continue
                        if command not in onlineCommands:
                            continue
                        labels.setdefault(command, display)
            applyEquivalentGroups(labels)
            perDocType[docType] = labels
            perDocTypeBound[docType] = taken
        resolved[lang] = perDocType
        bound[lang] = perDocTypeBound
    return resolved, bound


def splitBaseAndOverrides(resolved, bound):
    """Layer the resolved labels so each difference is stored once.

    Returns (base, byDocType, l10n):
    - base: what en-US says wherever the document types agree.
    - byDocType: where en-US differs for one document type.
    - l10n: {lang: {docType: ...}}, where a language differs from the
      en-US answer for that same document type.

    A command that this context does not bind, while the label it would
    otherwise inherit belongs to another command here, is stored as an
    empty string, so the menu says nothing rather than a key combination
    that does something else.
    """
    docTypes = sorted(MODULE_DOCTYPE.values())
    enUS = resolved.get('en-US', {})

    base = {}
    for command in {c for labels in enUS.values() for c in labels}:
        seen = [enUS[d][command] for d in docTypes if command in enUS[d]]
        base[command] = sorted(set(seen),
                               key=lambda v: (-seen.count(v), len(v), v))[0]

    def diff(labels, inherited, takenHere):
        """What this context has to say beyond what it inherits."""
        out = {}
        for command, display in labels.items():
            if inherited.get(command) != display:
                out[command] = display
        for command, display in inherited.items():
            if command not in labels and display in takenHere:
                out[command] = ''
        return out

    byDocType = {}
    enUSFor = {}
    for docType in docTypes:
        labels = enUS.get(docType, {})
        enUSFor[docType] = dict(base)
        d = diff(labels, base, bound['en-US'][docType])
        if d:
            byDocType[docType] = d
        enUSFor[docType].update(d)
        enUSFor[docType] = {c: l for c, l in enUSFor[docType].items() if l}

    l10n = {}
    for lang, perDocType in resolved.items():
        if lang == 'en-US':
            continue
        byDoc = {}
        for docType in docTypes:
            d = diff(perDocType.get(docType, {}), enUSFor[docType],
                     bound[lang][docType])
            if d:
                byDoc[docType] = d
        if byDoc:
            l10n[lang] = byDoc
    return base, byDocType, l10n


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

    Returns (shortcuts, byDocType, l10nShortcuts, modifierL10N).
    """
    xcuPath = os.path.join(
        corePath,
        'officecfg/registry/data/org/openoffice/Office/Accelerators.xcu')
    if not os.path.isfile(xcuPath):
        sys.stderr.write(
            "ERROR: Accelerators.xcu not found at %s\n" % xcuPath)
        exit(1)

    onlineCommands = extractAllOnlineCommands(onlinePath)

    resolved, bound = resolveShortcuts(xcuPath, onlineCommands)
    shortcuts, byDocType, l10nShortcuts = splitBaseAndOverrides(resolved, bound)

    for cmd, shortcut in MANUAL_FALLBACKS.items():
        if cmd not in shortcuts:
            shortcuts[cmd] = shortcut

    modifierL10N = extractModifierL10N(corePath)

    return shortcuts, byDocType, l10nShortcuts, modifierL10N


_IDENT_RE = re.compile(r'^[A-Za-z_$][A-Za-z0-9_$]*$')


def _jsKey(key):
    """Render a JS object key prettier-style (quoteProps: as-needed).

    Identifier-like keys come out unquoted; everything else stays
    single-quoted.  This matches what `make prettier-write` produces
    for unoshortcuts.js.
    """
    if _IDENT_RE.match(key):
        return key
    return "'%s'" % key.replace("'", "\\'")


# Prettier's default print width and tab width.
_PRINT_WIDTH = 80
_TAB_WIDTH = 2


def _renderObject(props, indent, trailing=','):
    """Render a list of (key, value) pairs as a JS object literal.

    Tries a single-line form `{ k: v, k: v }`; falls back to a
    multi-line form when the single-line version (including trailing
    punctuation) exceeds prettier's print width.  Tabs in `indent` are
    counted as `_TAB_WIDTH` columns to match prettier's measurement.
    """
    inline = "{ %s }" % ', '.join(
        '%s: %s' % (k, v) for k, v in props)
    width = len(indent.expandtabs(_TAB_WIDTH)) + len(inline) + len(trailing)
    if width <= _PRINT_WIDTH:
        return inline
    inner = indent + '\t'
    body = ''.join('%s%s: %s,\n' % (inner, k, v) for k, v in props)
    return '{\n%s%s}' % (body, indent)


def _writeDocTypeMap(f, byDocType, indent):
    """Write a {docType: {command: display}} object."""
    for docType in sorted(byDocType.keys()):
        f.write("%s%s: {\n" % (indent, _jsKey(docType)))
        for cmd in sorted(byDocType[docType].keys()):
            f.write("%s\t%s: '%s',\n"
                    % (indent, _jsKey(cmd), byDocType[docType][cmd]))
        f.write("%s},\n" % indent)


def writeUnoshortcutsJS(onlinePath, shortcuts, byDocType, l10nShortcuts,
                        modifierL10N):
    """Write the generated unoshortcuts.ts file."""
    outPath = os.path.join(onlinePath, 'browser/src/unoshortcuts.ts')
    with open(outPath, 'w', encoding='utf-8') as f:
        f.write("// Don't modify, generated using scripts/unoshortcuts.py\n")
        f.write("/* eslint-disable no-unused-vars */\n\n")

        # en-US command -> display string, where every document type
        # agrees.  An empty string means the command has no shortcut in
        # that context, so the menu says nothing rather than a key
        # combination that does something else there.
        f.write("var unoShortcutsMap: Record<string, string> = {\n")
        for cmd in sorted(shortcuts.keys()):
            f.write("\t%s: '%s',\n" % (_jsKey(cmd), shortcuts[cmd]))
        f.write("};\n\n")

        # Where en-US differs for one document type.
        f.write("var unoShortcutsDocType: "
                "Record<string, Record<string, string>> = {\n")
        _writeDocTypeMap(f, byDocType, "\t")
        f.write("};\n\n")

        # Where a language differs from the en-US answer for the same
        # document type.
        f.write("var unoShortcutsL10N: "
                "Record<string, Record<string, Record<string, string>>> = {\n")
        for lang in sorted(l10nShortcuts.keys()):
            f.write("\t%s: {\n" % _jsKey(lang))
            _writeDocTypeMap(f, l10nShortcuts[lang], "\t\t")
            f.write("\t},\n")
        f.write("};\n\n")

        # Localized modifier key names, extracted from
        # vcl/unx/generic/app/keysymnames.cxx in core.
        f.write("var unoShortcutsModifierL10N: Record<string, Record<string, string>> = {\n")
        for lang in sorted(modifierL10N.keys()):
            props = []
            for eng in ('Ctrl', 'Shift', 'Alt'):
                if eng in modifierL10N[lang]:
                    props.append((eng, "'%s'" % modifierL10N[lang][eng]))
            if props:
                f.write("\t%s: %s,\n"
                        % (_jsKey(lang), _renderObject(props, '\t')))
        f.write("};\n")


def parseUnoshortcutsJS(onlinePath):
    """Read the existing unoshortcuts.ts and return command -> shortcut map."""
    jsPath = os.path.join(onlinePath, 'browser/src/unoshortcuts.ts')
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

        shortcuts, byDocType, l10nSC, modL10N = generateShortcuts(
            onlineDir, coreDir)

        writeUnoshortcutsJS(onlineDir, shortcuts, byDocType, l10nSC, modL10N)

    else:
        usageAndExit()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

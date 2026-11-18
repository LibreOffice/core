#!/usr/bin/env python3
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import os
import polib
import sys
import itertools
import re


def usageAndExit():
    message = """usage: {program} online_dir lo_translations_dir lang

Prints en-US strings that do not have translations in the specified language.

"""
    print(message.format(program=os.path.basename(sys.argv[0])))
    exit(1)

# extract translations from po files


def extractFromPo(poFile, stringIds, untranslated):
    if not os.path.isfile(poFile):
        return

    po = polib.pofile(poFile,
                      autodetect_encoding=False,
                      encoding="utf-8",
                      wrapwidth=-1)

    for entry in itertools.chain(po.untranslated_entries(),
                                 po.fuzzy_entries()):
        for stringId in stringIds:
            if stringId in entry.msgctxt:
                untranslated.append(entry.msgid)


# Read the uno commands present in the unocommands.js for checking
def parseUnocommandsJS(onlineDir):
    strings = {}

    with open(onlineDir + '/browser/src/unocommands.js', 'r', encoding='utf-8') as f:
        for line in f:
            m = re.match(r"\t([^:]*):.*", line)
            if m:
                command = m.group(1)
                n = re.findall(r"_\('([^']*)'\)", line)
                if n:
                    strings[command] = n

    return strings


# Remove duplicates from list
def uniq(seq):
    seen = set()
    return [x for x in seq if not (x in seen or seen.add(x))]


if __name__ == "__main__":
    if len(sys.argv) != 4:
        usageAndExit()

    onlineDir = sys.argv[1]
    translationsDir = sys.argv[2]
    lang = sys.argv[3]

    dir = translationsDir + '/source/'

    untranslated = []

# LO Core strings

    # extract 'Clear formatting' and some status bar strings
    poFile = dir + lang + '/svx/messages.po'
    extractFromPo(poFile,
                  ["RID_SVXSTR_CLEARFORM",
                   "RID_SVXSTR_OVERWRITE_TEXT",
                   "selectionmenu|"],
                  untranslated)

    # extract Writer style names and status bar strings
    poFile = dir + lang + '/sw/messages.po'
    extractFromPo(poFile,
                  ["STR_POOL",
                   "STR_PAGE_COUNT",
                   "STR_STATUSBAR_WORDCOUNT_NO_SELECTION",
                   "STR_LANGSTATUS_NONE"],
                  untranslated)

    # extract Impress/Draw style names, layout names and 'Slide %1 of %2'
    poFile = dir + lang + '/sd/messages.po'
    extractFromPo(poFile,
                  ["STR_STANDARD_STYLESHEET_NAME",
                   "STR_POOL",
                   "STR_PSEUDOSHEET",
                   "STR_AUTOLAYOUT",
                   "STR_AL_",
                   "STR_SD_PAGE_COUNT"],
                  untranslated)

    # extract Calc style names and strings for status bar
    poFile = dir + lang + '/sc/messages.po'
    extractFromPo(poFile,
                  ["STR_STYLENAME_",
                   "STR_FILTER_SELCOUNT",
                   "STR_ROWCOL_SELCOUNT",
                   "STR_FUN_TEXT_",
                   "STR_UNDO_INSERTCELLS",
                   "STR_TABLE_COUNT"],
                  untranslated)

    # extract language names
    poFile = dir + lang + '/svtools/messages.po'
    extractFromPo(poFile, ["STR_ARR_SVT_LANGUAGE_TABLE"], untranslated)

# UNO command strings

    parsed = parseUnocommandsJS(onlineDir)
    keys = set(parsed.keys())

    poFile = dir + lang + '/officecfg/registry/data/org/openoffice/Office/UI.po'
    po = polib.pofile(poFile,
                      autodetect_encoding=False,
                      encoding="utf-8",
                      wrapwidth=-1)

    for entry in itertools.chain(po.untranslated_entries(),
                                 po.fuzzy_entries()):
        m = re.search(r"\.uno:([^\n]*)\n", entry.msgctxt)
        if m:
            command = m.group(1)
            if command in keys:
                for text in parsed[command]:
                    if text == entry.msgid:
                        untranslated.append(entry.msgid.replace("~", ""))

# Online UI

    poFile = onlineDir + '/browser/po/ui-' + lang.replace("-", "_") + '.po'
    po = polib.pofile(poFile,
                      autodetect_encoding=False,
                      encoding="utf-8",
                      wrapwidth=-1)

    for entry in itertools.chain(po.untranslated_entries(),
                                 po.fuzzy_entries()):
        untranslated.append(entry.msgid)

# Online help (keyboard shortcuts)

    poFile = onlineDir + '/browser/po/help-' + lang.replace("-", "_") + '.po'
    po = polib.pofile(poFile,
                      autodetect_encoding=False,
                      encoding="utf-8",
                      wrapwidth=-1)

    for entry in itertools.chain(po.untranslated_entries(),
                                 po.fuzzy_entries()):
        untranslated.append(entry.msgid)

# Print the results

    for elem in uniq(untranslated):
        print(elem)

# vim: set shiftwidth=4 softtabstop=4 expandtab:

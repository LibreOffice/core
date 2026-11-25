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

# Converts a .po file back to a localized .strings file:
#
#   po_to_strings.py [--table storyboard|code] BASE.strings LANG.po OUT.strings
#
# The storyboard table (Main.strings) is looked up by the Interface Builder
# object id in msgctxt; the code table (Localizable.strings) by msgid alone.
# A string without a translation keeps its English text.

import argparse
import json
import re
import subprocess
import sys
import polib

def load_strings_via_plutil(path):
    proc = subprocess.run(
        ["plutil", "-convert", "json", "-o", "-", path],
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    return json.loads(proc.stdout.decode("utf-8"))

def escape_strings_value(s):
    # Escape for .strings file
    return (
        s.replace("\\", "\\\\")
         .replace("\"", "\\\"")
         .replace("\n", "\\n")
         .replace("\r", "\\r")
    )

# A format specifier of String(format:), such as %@, %d, %1$@ or %.2f
SPECIFIER = re.compile(r"%(?:\d+\$)?[-+ #0]*\d*(?:\.\d+)?(?:hh|h|ll|l|q|L|z|t|j)?[a-zA-Z@]")

def specifiers(s):
    return sorted(SPECIFIER.findall(s.replace("%%", "")))

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--table", choices=["storyboard", "code"], default="storyboard")
    parser.add_argument("base", help="the English .strings file")
    parser.add_argument("po", help="the translation")
    parser.add_argument("out", help="the localized .strings file to write")
    args = parser.parse_args()

    base_map = load_strings_via_plutil(args.base)  # key -> English
    keys = sorted(base_map.keys())

    po = polib.pofile(args.po)
    if args.table == "storyboard":
        lookup = {e.msgctxt: e for e in po if e.msgctxt}
    else:
        lookup = {e.msgid: e for e in po if not e.msgctxt}

    lines = []
    for key in keys:
        english = str(base_map[key]) if base_map[key] is not None else ""
        translated = english

        entry = lookup.get(key)
        if entry and entry.msgstr and not entry.fuzzy:
            translated = entry.msgstr
            # A translation with other format specifiers than the English
            # would crash String(format:), so keep the English for it.
            if args.table == "code" and specifiers(translated) != specifiers(english):
                print("%s: the format specifiers of the translation of \"%s\" differ, keeping English"
                      % (args.po, english), file=sys.stderr)
                translated = english

        lines.append("\"%s\" = \"%s\";\n" %
                     (escape_strings_value(key),
                      escape_strings_value(translated)))

    with open(args.out, "w", encoding="utf-8") as out:
        out.writelines(lines)

if __name__ == "__main__":
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

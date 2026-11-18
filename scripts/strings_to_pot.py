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

# Converts a storyboard .strings file to .pot

import sys
import json
import subprocess
import time

HEADER = """msgid ""
msgstr ""
"Project-Id-Version: PACKAGE VERSION\\n"
"Report-Msgid-Bugs-To: \\n"
"POT-Creation-Date: {date}\\n"
"PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\\n"
"Last-Translator: FULL NAME <EMAIL@ADDRESS>\\n"
"Language-Team: LANGUAGE <LL@li.org>\\n"
"MIME-Version: 1.0\\n"
"Content-Type: text/plain; charset=UTF-8\\n"
"Content-Transfer-Encoding: 8bit\\n"

"""

PLACEHOLDER = "ProductName"
PLACEHOLDER_COMMENT = "Do not change the placeholder ProductName!"

def load_strings_via_plutil(path):
    # plutil parses .strings correctly (escapes, etc.)
    proc = subprocess.run(
        ["plutil", "-convert", "json", "-o", "-", path],
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    return json.loads(proc.stdout.decode("utf-8"))

def escape_po(s):
    return s.replace("\\", "\\\\").replace("\"", "\\\"")

def main():
    if len(sys.argv) != 3:
        print("Usage: strings_to_pot.py Main.strings Main.pot", file=sys.stderr)
        sys.exit(1)

    base_path = sys.argv[1]
    pot_path = sys.argv[2]

    mapping = load_strings_via_plutil(base_path)  # key -> English
    keys = sorted(mapping.keys())

    now = time.strftime("%Y-%m-%d %H:%M%z")
    with open(pot_path, "w", encoding="utf-8") as out:
        out.write(HEADER.format(date=now))
        for key in keys:
            english = mapping[key]
            if english is None:
                continue

            english = str(english)

            if PLACEHOLDER in english:
                out.write("#. %s\n" % PLACEHOLDER_COMMENT)

            out.write('msgctxt "%s"\n' % escape_po(key))
            out.write('msgid "%s"\n' % escape_po(english))
            out.write('msgstr ""\n\n')

if __name__ == "__main__":
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

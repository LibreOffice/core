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

# Converts .po file back to a localized storyboard .strings file

import sys
import json
import subprocess
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

def main():
    if len(sys.argv) != 4:
        print("Usage: po_to_strings.py Main.strings lang.po out.strings", file=sys.stderr)
        sys.exit(1)

    base_strings_path = sys.argv[1]
    po_path = sys.argv[2]
    out_strings_path = sys.argv[3]

    base_map = load_strings_via_plutil(base_strings_path)  # key -> English
    keys = sorted(base_map.keys())

    po = polib.pofile(po_path)
    by_ctx = {e.msgctxt: e for e in po if e.msgctxt}

    lines = []
    for key in keys:
        english = str(base_map[key]) if base_map[key] is not None else ""
        translated = english

        entry = by_ctx.get(key)
        if entry and entry.msgstr and not entry.fuzzy:
            translated = entry.msgstr

        lines.append("\"%s\" = \"%s\";\n" %
                     (escape_strings_value(key),
                      escape_strings_value(translated)))

    with open(out_strings_path, "w", encoding="utf-8") as out:
        out.writelines(lines)

if __name__ == "__main__":
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

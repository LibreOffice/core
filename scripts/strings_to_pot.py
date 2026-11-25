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

# Converts the macOS app's .strings files to one .pot:
#
#   strings_to_pot.py -o Main.pot --storyboard Main.strings [--code Localizable.strings]
#
# A storyboard string is identified by its Interface Builder object id, which
# becomes the msgctxt. A code string (an NSLocalizedString() key, extracted by
# genstrings_utf8.sh) is identified by its English text alone, so it has no
# msgctxt; the comment of the call becomes the translator note.

import argparse
import json
import re
import subprocess
import sys
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

STRINGS_ESCAPES = {"n": "\n", "r": "\r", "t": "\t", '"': '"', "\\": "\\"}

def unescape_strings_key(raw):
    def repl(m):
        esc = m.group(1)
        if esc.startswith("U"):
            return chr(int(esc[1:], 16))
        return STRINGS_ESCAPES.get(esc, esc)
    return re.sub(r"\\(U[0-9A-Fa-f]{4}|.)", repl, raw)

def load_comments(path):
    # The comments of a genstrings file, by key. plutil drops them.
    with open(path, encoding="utf-8") as f:
        text = f.read()
    comments = {}
    for m in re.finditer(r'/\*\s*(.*?)\s*\*/\s*\n"((?:[^"\\]|\\.)*)"\s*=', text, re.S):
        key = unescape_strings_key(m.group(2))
        lines = [line.strip() for line in m.group(1).split("\n")]
        comments[key] = [line for line in lines if line]
    return comments

def escape_po(s):
    return s.replace("\\", "\\\\").replace("\"", "\\\"").replace("\n", "\\n")

def write_entry(out, key, english, comments, context):
    for comment in comments:
        out.write("#. %s\n" % comment)
    if PLACEHOLDER in english:
        out.write("#. %s\n" % PLACEHOLDER_COMMENT)
    if context is None and "%" in english:
        out.write("#, objc-format\n")
    if context is not None:
        out.write('msgctxt "%s"\n' % escape_po(context))
    out.write('msgid "%s"\n' % escape_po(english))
    out.write('msgstr ""\n\n')

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", "--output", required=True, help="the .pot to write")
    parser.add_argument("--storyboard", required=True,
                        help="Main.strings generated from the storyboard by ibtool")
    parser.add_argument("--code",
                        help="Localizable.strings extracted from the sources by genstrings")
    args = parser.parse_args()

    storyboard = load_strings_via_plutil(args.storyboard)  # object id -> English
    code = load_strings_via_plutil(args.code) if args.code else {}  # English -> English
    code_comments = load_comments(args.code) if args.code else {}

    now = time.strftime("%Y-%m-%d %H:%M%z")
    with open(args.output, "w", encoding="utf-8") as out:
        out.write(HEADER.format(date=now))
        for key in sorted(storyboard.keys()):
            english = storyboard[key]
            if english is None:
                continue
            write_entry(out, key, str(english), [], key)
        for key in sorted(code.keys()):
            write_entry(out, key, key, code_comments.get(key, []), None)

if __name__ == "__main__":
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

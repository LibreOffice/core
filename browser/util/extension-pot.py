#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Extract the translatable strings of a COOL JS extension into po/<id>.pot.
#
#     python3 util/extension-pot.py extensions/com.example.myextension
#
# Strings come from four places, see extensions/README.md:
#   - manifest.json: "name"
#   - the contributes file (ui.json): command "title", notebookbar "tab",
#     group "label", dropdown menu "title"
#   - HTML pages: data-l10n, data-l10n-title, data-l10n-placeholder and
#     data-l10n-aria-label attribute values
#   - JS files: cool._('...') and cool._n('one', 'many', n) calls
# Translators then produce po/<lang>.po from the template (Weblate); the build
# turns those into l10n/<lang>.json with po2json.py.

import datetime
import glob
import html
import json
import os
import re
import sys

import polib

if len(sys.argv) != 2:
    sys.stderr.write("usage: %s <extension directory>\n" % sys.argv[0])
    sys.exit(1)

ext_dir = sys.argv[1].rstrip("/")
ext_id = os.path.basename(ext_dir)
manifest_path = os.path.join(ext_dir, "manifest.json")
if not os.path.isfile(manifest_path):
    sys.stderr.write("%s: no manifest.json\n" % ext_dir)
    sys.exit(1)

# msgid -> (msgid_plural or None, [references])
entries = {}


def add(msgid, ref, plural=None):
    if not msgid:
        return
    key = (msgid, plural)
    entries.setdefault(key, [])
    if ref not in entries[key]:
        entries[key].append(ref)


def rel(path):
    return os.path.relpath(path, ext_dir)


with open(manifest_path, encoding="utf-8") as f:
    manifest = json.load(f)
add(manifest.get("name"), rel(manifest_path) + ":name")

contributes = manifest.get("contributes")
if isinstance(contributes, str):
    ui_path = os.path.join(ext_dir, contributes)
    if os.path.isfile(ui_path):
        with open(ui_path, encoding="utf-8") as f:
            ui = json.load(f)
        for cmd in ui.get("commands", []):
            add(cmd.get("title"), rel(ui_path) + ":commands." + str(cmd.get("id")))
        for tab in ui.get("notebookbar", []):
            add(tab.get("tab"), rel(ui_path) + ":notebookbar.tab")
            for group in tab.get("groups", []):
                add(group.get("label"), rel(ui_path) + ":notebookbar.group." + str(group.get("id")))
                for item in group.get("items", []):
                    if item.get("type") == "menu":
                        add(item.get("title"), rel(ui_path) + ":notebookbar.menu")

HTML_ATTRS = re.compile(r'data-l10n(?:-title|-placeholder|-aria-label)?\s*=\s*("([^"]*)"|\'([^\']*)\')')
# cool._('...') and cool._("...") with the usual JS escapes; also bare _(...) inside
# extension scripts is deliberately not matched: extensions use the cool.* namespace.
JS_ONE = re.compile(r"""cool\._\(\s*(?:'((?:[^'\\]|\\.)*)'|"((?:[^"\\]|\\.)*)")\s*\)""")
JS_MANY = re.compile(r"""cool\._n\(\s*(?:'((?:[^'\\]|\\.)*)'|"((?:[^"\\]|\\.)*)")\s*,\s*(?:'((?:[^'\\]|\\.)*)'|"((?:[^"\\]|\\.)*)")""")


def unescape(s):
    return bytes(s, "utf-8").decode("unicode_escape") if "\\" in s else s


def line_of(text, pos):
    return text.count("\n", 0, pos) + 1


for path in sorted(glob.glob(os.path.join(ext_dir, "**", "*"), recursive=True)):
    if not os.path.isfile(path):
        continue
    parts = rel(path).split(os.sep)
    if parts[0] in ("po", "l10n", "node_modules"):
        continue
    is_html = path.endswith(".html")
    if not is_html and not path.endswith(".js") and not path.endswith(".mjs"):
        continue
    with open(path, encoding="utf-8") as f:
        text = f.read()
    if is_html:
        for m in HTML_ATTRS.finditer(text):
            # The DOM hands cool.js the decoded attribute value, so the
            # msgid is "Generate & insert", not "Generate &amp; insert".
            value = html.unescape(m.group(2) if m.group(2) is not None else m.group(3))
            add(value, "%s:%d" % (rel(path), line_of(text, m.start())))
    # cool._() calls: in JS files and in the inline scripts of HTML pages
    for m in JS_MANY.finditer(text):
        one = m.group(1) if m.group(1) is not None else m.group(2)
        many = m.group(3) if m.group(3) is not None else m.group(4)
        add(unescape(one), "%s:%d" % (rel(path), line_of(text, m.start())), unescape(many))
    for m in JS_ONE.finditer(text):
        add(unescape(m.group(1) if m.group(1) is not None else m.group(2)), "%s:%d" % (rel(path), line_of(text, m.start())))

pot = polib.POFile()
pot.metadata = {
    "Project-Id-Version": ext_id,
    "Report-Msgid-Bugs-To": "",
    "POT-Creation-Date": datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%d %H:%M+0000"),
    "PO-Revision-Date": "YEAR-MO-DA HO:MI+ZONE",
    "Last-Translator": "FULL NAME <EMAIL@ADDRESS>",
    "Language-Team": "LANGUAGE <LL@li.org>",
    "MIME-Version": "1.0",
    "Content-Type": "text/plain; charset=UTF-8",
    "Content-Transfer-Encoding": "8bit",
}
for (msgid, plural), refs in sorted(entries.items(), key=lambda kv: (kv[0][0], kv[0][1] or "")):
    entry = polib.POEntry(msgid=msgid, occurrences=[(r, "") for r in refs])
    if plural:
        entry.msgid_plural = plural
        entry.msgstr_plural = {0: "", 1: ""}
    pot.append(entry)

out_dir = os.path.join(ext_dir, "po")
os.makedirs(out_dir, exist_ok=True)
out = os.path.join(out_dir, ext_id + ".pot")
pot.save(out)
print("%s: %d strings -> %s" % (ext_id, len(pot), out))

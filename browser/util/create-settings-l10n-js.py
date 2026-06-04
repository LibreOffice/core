#!/usr/bin/env python3
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Emit a small JS file that sets window.LOCALIZATIONS for the Options
# (integrator settings) dialog only.
#
# The desktop and mobile apps load the settings UI in a file:// iframe where
# the <link rel="localizations"> loader never worked. The main app solves this
# with l10n-all.js (a window.LANG -> window.LOCALIZATIONS if/else), but that
# file carries the whole UI for every language (~8 MB). The settings dialog
# needs ~120 strings, so this generator reuses the same selection mechanism but
# emits only those strings - one file, all languages, ~200 KB.
#
# The string set is read straight from the integrator sources (the _('...')
# literals), so no xgettext run is needed at build time; translations come from
# the committed po/ui-*.po files.

import argparse
import json
import os
import re

# The files that make up the Options dialog UI. Keep in sync with the
# integrator entries of the `pot` target in browser/Makefile.am.
SETTINGS_SOURCES = [
    "admin/src/integrator/AdminIntegratorSettings.ts",
    "admin/src/integrator/WordBook.ts",
    "admin/src/integrator/Xcu.ts",
]

# Languages that match directly on the base language (onlylang).
SIMPLE = [
    "ar", "ca", "cs", "cy", "da", "de", "el", "es", "eu", "fi",
    "fr", "ga", "gl", "he", "hr", "hu", "hy", "id", "is", "it",
    "ja", "kk", "ko", "nl", "pl", "pt", "ro", "ru", "sk", "sl",
    "sq", "sv", "tr", "uk",
]

# Languages that need explicit alias checks on the full window.LANG.
ALIASES = {
    "en_GB": ["en-GB", "en_GB"],
    "pt_BR": ["pt-BR", "pt_BR"],
    "zh_CN": ["zh-CN", "zh-Hans-CN", "zh_CN", "zh_Hans_CN"],
    "zh_TW": ["zh-TW", "zh-Hant-TW", "zh_TW", "zh_Hant_TW"],
}

_ESCAPES = {
    "n": "\n", "t": "\t", "r": "\r", "a": "\a", "b": "\b",
    "f": "\f", "v": "\v", '"': '"', "'": "'", "\\": "\\",
}


def unescape(s):
    """Resolve C/JS backslash escapes without mangling UTF-8 bytes."""
    out = []
    i = 0
    while i < len(s):
        c = s[i]
        if c == "\\" and i + 1 < len(s):
            nxt = s[i + 1]
            out.append(_ESCAPES.get(nxt, nxt))
            i += 2
        else:
            out.append(c)
            i += 1
    return "".join(out)


# Matches _('...') / _("...") with escaped quotes inside.
_CALL_RE = re.compile(r"""_\(\s*(['"])((?:\\.|(?!\1).)*?)\1""", re.DOTALL)


def collect_msgids(base):
    """The set of source strings the Options dialog passes to _()."""
    ids = set()
    for rel in SETTINGS_SOURCES:
        with open(os.path.join(base, rel), encoding="utf-8") as f:
            text = f.read()
        for _quote, raw in _CALL_RE.findall(text):
            ids.add(unescape(raw))
    return ids


def parse_po(path):
    """Minimal PO reader: msgid -> msgstr, ignoring plurals and contexts."""
    table = {}
    msgid = ""
    msgstr = ""
    state = None

    def piece(line, keyword):
        return unescape(line[len(keyword):].strip().strip('"'))

    with open(path, encoding="utf-8") as f:
        for line in f:
            line = line.rstrip("\n")
            if line.startswith("msgid "):
                if state and msgid:
                    table[msgid] = msgstr
                msgid = piece(line, "msgid ")
                msgstr = ""
                state = "id"
            elif line.startswith("msgstr "):
                msgstr = piece(line, "msgstr ")
                state = "str"
            elif line.startswith('"'):
                if state == "id":
                    msgid += unescape(line.strip().strip('"'))
                elif state == "str":
                    msgstr += unescape(line.strip().strip('"'))
            elif line.strip() == "":
                if state and msgid:
                    table[msgid] = msgstr
                msgid = ""
                msgstr = ""
                state = None
    if state and msgid:
        table[msgid] = msgstr
    return table


def localizations(base, lang, allow):
    """{source: translation} for the allowed strings translated in this lang."""
    path = os.path.join(base, "po", "ui-%s.po" % lang)
    if not os.path.exists(path):
        return {}
    table = parse_po(path)
    return {k: table[k] for k in allow if table.get(k)}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--srcdir",
        default=os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
        help="browser/ source directory (default: two levels up from script)",
    )
    args = parser.parse_args()
    base = args.srcdir

    allow = collect_msgids(base)

    print("""var onlylang = window.LANG;
var hyphen = onlylang.indexOf('-');
if (hyphen > 0) {
    onlylang = onlylang.substring(0, hyphen);
}
var underscore = onlylang.indexOf('_');
if (underscore > 0) {
    onlylang = onlylang.substring(0, underscore);
}

if (false) {
    ;
}
""")

    def emit(condition, lang):
        data = localizations(base, lang, allow)
        dump = json.dumps(data, ensure_ascii=False, indent=4, sort_keys=True)
        print("else if (%s) {" % condition)
        print("    window.LOCALIZATIONS = " + dump + ";")
        print("}")

    for lang in SIMPLE:
        emit("onlylang == '%s'" % lang, lang)

    for lang in sorted(ALIASES.keys()):
        condition = " || ".join(
            "window.LANG == '%s'" % alias for alias in ALIASES[lang]
        )
        emit(condition, lang)

    print("""
else {
    window.LOCALIZATIONS = {};
}""")


if __name__ == "__main__":
    main()

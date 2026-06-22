#!/usr/bin/env python3
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import json
import os
import sys

# Base path: two levels up from this script 
BASE_PATH = os.path.dirname(os.path.dirname(__file__)) + "/"


def readwhole(file_path):
    """Read entire file content as UTF-8."""
    with open(file_path, "r", encoding="utf-8") as f:
        return f.read()


def _short(s, max_len):
    """
    Convert multi-line JSON into a single-line string
    for warnings and truncate if too long.
    """
    s = "\\n".join(s.splitlines())
    return s[:max_len] + "…" if len(s) > max_len else s


def insert(*relfiles):
    """
    Merge one or more JSON translation files into a single object.

    - Later files override earlier ones.
    - Warn only if a key is overwritten with a different value.
    - Output is sorted and pretty-printed for stable diffs.
    """
    if not relfiles:
        raise RuntimeError("insert(): no files specified")

    merged = {}
    seen_in = {}  # Tracks where a key was first introduced

    for rel in relfiles:
        full_path = os.path.join(BASE_PATH, rel)

        raw = readwhole(full_path)
        obj = json.loads(raw)

        if not isinstance(obj, dict):
            raise RuntimeError(f"insert(): {rel} is not a JSON object")

        for k, v in obj.items():
            if k in merged:
                prev = seen_in.get(k, "(unknown)")

                # Compare serialized JSON to detect actual value changes
                old_json = json.dumps(
                    merged[k], ensure_ascii=False, indent=4, sort_keys=True
                )
                new_json = json.dumps(
                    v, ensure_ascii=False, indent=4, sort_keys=True
                )

                if old_json != new_json:
                    old_s = _short(old_json, 140)
                    new_s = _short(new_json, 140)
                    sys.stderr.write(
                        f"insert(): key '{k}' overwritten "
                        f"({prev} -> {rel}): {old_s} -> {new_s}\n"
                    )
            else:
                seen_in[k] = rel

            merged[k] = v

    # Stable output 
    return json.dumps(merged, ensure_ascii=False, indent=4, sort_keys=True)


# JavaScript output

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

# Languages that match directly on onlylang
simple = [
   "ar", "ca", "cs", "cy", "da", "de", "el", "es", "eu", "fi",
   "fr", "ga", "gl", "he", "hr", "hu", "hy", "id", "is", "it",
   "ja", "kk", "ko", "nl", "pl", "pt", "ro", "ru", "sk", "sl",
   "sq", "sv", "tr", "uk"
]

# Languages that need explicit alias checks on window.LANG
aliases = {
    "en_GB": ["en-GB", "en_GB"],
    "pt_BR": ["pt-BR", "pt_BR"],
    "zh_CN": ["zh-CN", "zh-Hans-CN", "zh_CN", "zh_Hans_CN"],
    "zh_TW": ["zh-TW", "zh-Hant-TW", "zh_TW", "zh_Hant_TW"],
}

for lang in simple:
    lang_hyphen = lang.replace("_", "-")

    print(f"else if (onlylang == '{lang}') {{")
    print(
        "    window.LOCALIZATIONS = "
        + insert(
            f"po/ui-{lang}.po.json",
            f"l10n/uno/{lang_hyphen}.json",
        )
        + ";"
    )
    print(
        "    window.LOCALIZATIONS_HELP = "
        + insert(f"po/help-{lang}.po.json")
        + ";"
    )
    print("}")

for lang in sorted(aliases.keys()):
    cond = " || ".join(
        [f"window.LANG == '{alias}'" for alias in aliases[lang]]
    )
    lang_hyphen = lang.replace("_", "-")

    print(f"else if ({cond}) {{")
    print(
        "    window.LOCALIZATIONS = "
        + insert(
            f"po/ui-{lang}.po.json",
            f"l10n/uno/{lang_hyphen}.json",
        )
        + ";"
    )
    print(
        "    window.LOCALIZATIONS_HELP = "
        + insert(f"po/help-{lang}.po.json")
        + ";"
    )
    print("}")

print("""
else {
    window.LOCALIZATIONS = {};
}""")

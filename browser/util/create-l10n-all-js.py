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

# The shipped languages come from engine/distro-configs/Langs.conf (the single
# source of truth), or from the generated localizations.json map when the engine
# is absent.
def read_langs():
    conf = os.path.join(BASE_PATH, "..", "engine", "distro-configs", "Langs.conf")
    if os.path.isfile(conf):
        for line in readwhole(conf).splitlines():
            if line.startswith("--with-lang="):
                return line[len("--with-lang="):].split()
    mapping = json.loads(readwhole(os.path.join(BASE_PATH, "l10n", "localizations.json")))
    langs = set()
    for value in mapping.values():
        if isinstance(value, str):  # ".../ui-en_GB.json" -> "en-GB"
            base = os.path.basename(value)[:-len(".json")]
            langs.add(base[len("ui-"):].replace("_", "-"))
    return sorted(langs)


all_langs = [lang for lang in read_langs() if lang != "en-US"]

# Languages without a region subtag match directly on the stripped onlylang.
simple = sorted(lang for lang in all_langs if "-" not in lang)

# Region languages need explicit window.LANG checks, keyed by the underscore
# file-name form. Chinese also accepts the script-qualified forms.
script_variants = {
    "zh-CN": ["zh-Hans-CN", "zh_Hans_CN"],
    "zh-TW": ["zh-Hant-TW", "zh_Hant_TW"],
}
aliases = {}
for lang in sorted(lang for lang in all_langs if "-" in lang):
    key = lang.replace("-", "_")
    aliases[key] = [lang, key] + script_variants.get(lang, [])

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

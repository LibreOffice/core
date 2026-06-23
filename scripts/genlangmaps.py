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

"""Generate the localization map files under browser/l10n/ from the shared
language list engine/distro-configs/Langs.conf.

That list (curated from Weblate statistics) is the single source of truth for
which languages we ship, so we don't ship languages that the engine does not
translate or that are poorly translated. The generated maps are:

    localizations.json       (UI strings,   l10n/ui-<lang>.json)
    help-localizations.json  (help strings, l10n/help-<lang>.json)
    uno-localizations.json   (UNO commands, l10n/uno/<lang>.json)

Usage: genlangmaps.py <online_dir> <engine_dir>
"""

import os
import sys

PREFIX = '%SERVICE_ROOT%/browser/%VERSION%/l10n/'

# Curated aliases: a requested language tag -> the language whose file it should
# reuse. Only emitted when the target language is in the list. Script-tag
# aliases (zh-Hans/zh-Hant) are used verbatim; region-tag aliases (zh-HK) also
# get a lowercased form, like the base languages do.
ALIASES = {
    'zh-Hans': 'zh-CN',
    'zh-Hant': 'zh-TW',
    'zh-HK': 'zh-TW',
}


def readLangs(engineDir):
    conf = os.path.join(engineDir, 'distro-configs', 'Langs.conf')
    with open(conf, 'r', encoding='utf-8') as f:
        for line in f:
            if line.startswith('--with-lang='):
                return line[len('--with-lang='):].split()
    sys.exit('genlangmaps: no --with-lang= line in ' + conf)


def isRegionTag(alias):
    # e.g. zh-HK -> region "HK" (two uppercase letters); zh-Hans -> script "Hans"
    parts = alias.split('-')
    return len(parts) == 2 and len(parts[1]) == 2 and parts[1].isupper()


def buildMap(langs, onlineDir, kind):
    """kind is 'ui', 'help' or 'uno'."""
    def fileUrl(lang):
        if kind == 'uno':
            return PREFIX + 'uno/' + lang + '.json'
        return PREFIX + kind + '-' + lang.replace('-', '_') + '.json'

    def shipped(lang):
        # uno/<lang>.json is generated for every mapped language (empty when the
        # engine has no translation), so no .po check there. ui/help only ship a
        # language when its .po exists.
        if kind == 'uno':
            return True
        poName = kind + '-' + lang.replace('-', '_') + '.po'
        return os.path.isfile(os.path.join(onlineDir, 'browser', 'po', poName))

    mapping = {}
    present = set()
    for lang in langs:
        if lang == 'en-US':
            mapping['en'] = False
            continue
        if not shipped(lang):
            continue
        present.add(lang)
        url = fileUrl(lang)
        mapping[lang] = url
        if lang != lang.lower():
            mapping[lang.lower()] = url

    for alias, target in ALIASES.items():
        if target in present:
            url = fileUrl(target)
            mapping[alias] = url
            if isRegionTag(alias):
                mapping[alias.lower()] = url

    return mapping


def writeMap(path, mapping):
    keys = sorted(k for k in mapping if k != 'en')
    lines = []
    if 'en' in mapping:
        lines.append('    "en": false')
    for k in keys:
        lines.append('    "%s": "%s"' % (k, mapping[k]))
    with open(path, 'w', encoding='utf-8') as f:
        f.write('{\n')
        f.write(',\n'.join(lines))
        f.write('\n}\n')


if __name__ == '__main__':
    if len(sys.argv) != 3:
        sys.exit('usage: genlangmaps.py <online_dir> <engine_dir>')

    onlineDir = sys.argv[1]
    engineDir = sys.argv[2]
    langs = readLangs(engineDir)

    l10nDir = os.path.join(onlineDir, 'browser', 'l10n')
    os.makedirs(l10nDir, exist_ok=True)

    for kind, name in (('ui', 'localizations.json'),
                       ('help', 'help-localizations.json'),
                       ('uno', 'uno-localizations.json')):
        writeMap(os.path.join(l10nDir, name), buildMap(langs, onlineDir, kind))

# vim: set shiftwidth=4 softtabstop=4 expandtab:

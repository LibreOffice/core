#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# This file is part of the Collabora Online project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Regenerate l10n/logo-l10n.json from the keyword and message translations that
# engine/librelogo/source/LibreLogo/LibreLogo.py embeds as its __lng_fallback__
# dictionary. Usage, from the repository root:
#
#     python3 browser/extensions/com.collaboraoffice.librelogo/tools/gen-l10n.py
#
# The JSON has the same shape as the Python dictionary: for every key (FORWARD,
# ERR_ZERODIVISION, ...) a map from language code to the localized value, where
# a value is a "|"-separated list of alternatives and en_US is always present.
# Languages that use the English value for a key are simply absent for that key,
# so the interpreter falls back to en_US when a lookup misses.

import json
import os
import sys

here = os.path.dirname(os.path.abspath(__file__))
root = os.path.abspath(os.path.join(here, '..', '..', '..', '..'))
source = os.path.join(root, 'engine', 'librelogo', 'source', 'LibreLogo', 'LibreLogo.py')
target = os.path.join(here, '..', 'l10n', 'logo-l10n.json')

with open(source, encoding='utf-8') as f:
    text = f.read()

start = text.rindex('\n__lng_fallback__ = {')
namespace = {}
exec(text[start:], namespace)
fallback = namespace['__lng_fallback__']

os.makedirs(os.path.dirname(target), exist_ok=True)
with open(target, 'w', encoding='utf-8') as f:
    json.dump(fallback, f, ensure_ascii=False, separators=(',', ':'), sort_keys=True)
    f.write('\n')

languages = set()
for values in fallback.values():
    languages.update(values.keys())
print('%d keys, %d languages -> %s' % (len(fallback), len(languages), os.path.relpath(target, root)))

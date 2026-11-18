#!/bin/env python
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
#coding: utf8
import json
import sys

# This script is used in the forum for weblate montly stats

if len(sys.argv) > 1:
    json_filename = sys.argv[1]
else:
    print('''
    JSON file missing
    Please run this script with the translation file path as a parameter\n
    Generate a json file from "Contributor Stats"
    https://hosted.weblate.org/projects/collabora-online/#reports\n
    Then run the script, example:
    python3 weblate-translation-count-parser.py april-translations-count.json
    ''')
    sys.exit(0)

if len(sys.argv) == 3:
    use_markdown = bool(sys.argv[2])
else:
    use_markdown = False

with open(sys.argv[1], 'r') as json_data:
    data = json.load(json_data)

# Or from string
# data = json.loads('[{"name" : "1", "two" : "2", "three" : "3"}]')

def print_parsed(msg, markdowned):
    if markdowned is False:
        print(msg)
        l_r_separator = ''
        m_separator = ': '
    else:
        print(msg + '\n| Translator | Number of strings  |\n| --- | --- |')
        l_r_separator = '|'
        m_separator = '|'
    medal_it = 1
    for weblate_user in data:
        if medal_it is 1:
            medal_str = ':1st_place_medal: '
        elif medal_it is 2:
            medal_str = ':2nd_place_medal: '
        elif medal_it is 3:
            medal_str = ':3rd_place_medal: '
        else:
            medal_str = ''
        weblate_row = l_r_separator + medal_str + weblate_user['name'] + m_separator + str(weblate_user['count']) + l_r_separator
        print (weblate_row)
        medal_it +=1

# print_parsed('\nOriginal:\n')
data.sort(key=lambda x: x['count'], reverse=True)
print_parsed('\nParsed from ' + json_filename  + ':\n', use_markdown)

# vim: set shiftwidth=4 softtabstop=4 expandtab:

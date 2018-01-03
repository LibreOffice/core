#!/usr/bin/env python3

# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import os
import datetime

def analyze_file(filename):
    class_name = ""
    method_list = []
    with open(filename, encoding='utf-8') as fh:
        for line in fh:
            if line.lstrip().startswith('class '):
                class_name = line.lstrip().split(" ")[1].split("(")[0]
            elif line.lstrip().startswith('def test_'):
                method_list.append(
                        line.lstrip().split("test_")[1].split("(")[0])
            else:
                continue
    return class_name, method_list

def get_files_list(directory, extension):
    array_items = []

    dh = os.scandir(directory)
    for entry in dh:
        if entry.is_dir():
            array_items += get_files_list(entry.path, extension)
        elif entry.is_file():
            if entry.name.endswith(extension):
                array_items.append(entry.path)

    return array_items

def linkFormat(name):
    if name.startswith('tdf'):
        return "[https://bugs.documentfoundation.org/show_bug.cgi?id={} {}]"\
                .format(name.split('tdf')[1], name)
    else:
        return name


def main():
    uitest_ext = '.py'
    uitest_dirs = {
            'Writer' : ['../uitest/writer_tests/', '../writerperfect/qa/uitest/', '../sw/qa/uitest/writer_tests/'],
            'Calc' : ['../uitest/calc_tests', '../sc/qa/uitest/'],
            'Impress' : ['../uitest/impress_tests/'],
            'Math': ['../uitest/math_tests/'],
            'Draw': [''],
            'Manual_tests': ['../uitest/manual_tests/']}

    print('{{TopMenu}}')
    print('{{Menu}}')
    print('{{Menu.Development}}')
    print('{{OrigLang|}}')
    print()
    print('Generated on ' + str(datetime.datetime.now()))
    for k,v in uitest_dirs.items():
        print('\n=== ' + k + ' ===')
        for uitest_dir in v:
            if uitest_dir:
                uitest_files = get_files_list(uitest_dir, uitest_ext)
                for uitest_file in uitest_files:
                    class_name, method_names = analyze_file(uitest_file)
                    if class_name:
                        print("* {} ({})".format(
                            linkFormat(class_name),uitest_file[3:]))
                        for m in method_names:
                            print('**' + linkFormat(m))
    print()
    print('[[Category:QA]][[Category:Development]]')

if __name__ == '__main__':
    main()

#!/usr/bin/env python3

# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

"""
Script to generate https://wiki.documentfoundation.org/Development/DispatchCommands
"""

import argparse
import os
import sys


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


def analyze_file(filename, all_slots):
    with open(filename) as fh:
        for line in fh:
            if not line.startswith('// Slot Nr. '):
                continue

            tmp = line.split(':')
            slot_id = tmp[1].strip()

            line = next(fh)
            tmp = line.split(',')
            slot_rid = tmp[1]

            next(fh)
            next(fh)
            line = next(fh)
            mode = 'C' if 'CACHABLE' in line else ' '
            mode += 'U' if 'AUTOUPDATE' in line else ' '
            mode += 'M' if 'MENUCONFIG' in line else ' '
            mode += 'T' if 'TOOLBOXCONFIG' in line else ' '
            mode += 'A' if 'ACCELCONFIG' in line else ' '

            next(fh)
            next(fh)
            line = next(fh)
            if '"' not in line:
                line = next(fh)
            tmp = line.split('"')
            try:
                slot_name = '.uno:' + tmp[1]
            except IndexError:
                print("Warning: expected \" in line '%s' from file %s" % (line.strip(), filename),
                      file=sys.stderr)
                slot_name = '.uno:'

            if slot_name not in all_slots:
                all_slots[slot_name] = {'slot_id': slot_id,
                                        'slot_rid': slot_rid,
                                        'mode': mode,
                                        'slot_description': ''}


def analyze_xcu(filename, all_slots):
    with open(filename) as fh:
        for line in fh:
            if '<node oor:name=".uno:' not in line:
                continue

            tmp = line.split('"')
            slot_name = tmp[1]

            while '<value xml:lang="en-US">' not in line:
                try:
                    line = next(fh)
                except StopIteration:
                    print("Warning: couldn't find '<value xml:lang=\"en-US\">' line in %s" % filename,
                          file=sys.stderr)
                    break

            line = line.replace('<value xml:lang="en-US">', '')
            line = line.replace('</value>', '').strip()

            if slot_name in all_slots:
                all_slots[slot_name]['slot_description'] = line.replace('~', '')


def main():
    modules = ['basslots', 'scslots', 'sdgslots', 'sdslots', 'sfxslots', 'smslots', 'svxslots', 'swslots']
    sdi_dir = './workdir/SdiTarget'
    sdi_ext = '.hxx'
    xcu_dir = 'officecfg/registry/data/org/openoffice/Office/UI'
    xcu_ext = '.xcu'
    all_slots = {}

    parser = argparse.ArgumentParser()
    parser.add_argument('module', choices=modules)
    args = parser.parse_args()

    module_filename = args.module + sdi_ext

    sdi_files = get_files_list(sdi_dir, sdi_ext)
    for sdi_file in sdi_files:
        sdi_file_basename = os.path.basename(sdi_file)
        if sdi_file_basename == module_filename:
            analyze_file(sdi_file, all_slots)

    xcu_files = get_files_list(xcu_dir, xcu_ext)
    for xcu_file in xcu_files:
        analyze_xcu(xcu_file, all_slots)

    for name in sorted(all_slots.keys()):
        props = all_slots[name]
        print('|-\n| %s' % name)
        print('| %(slot_rid)s\n| %(slot_id)s\n| %(mode)s\n| %(slot_description)s' % props)

    print("|-")

if __name__ == '__main__':
    main()

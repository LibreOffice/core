# -*- Mode: Python; tab-width: 4; indent-tabs-mode: nil -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

import sys, os, re
import argparse

global_list = []
global_hash = {}
args = None

def read_icons(fname):
    global args
    images = []
    full_path = os.path.join(args.base_path, fname)
    if not os.path.exists(full_path):
        if not args.quiet:
            print("Skipping non-existent {}\n".format(full_path), file=sys.stderr)
        return images
    with open(full_path) as fp:
        for line in fp:
            m = re.search(r'xlink:href="\.uno:(\S+)"\s+', line)
            if m:
                images.append(m.group(1).lower())
    return images

# filter out already seen icons & do prefixing
def read_new_icons(fname, prefix):
    images = read_icons(fname)
    new_icons_arr = []
    new_icons_d = {}
    for icon in images:
        iname = "cmd/" + prefix + icon + ".png"
        if iname not in global_hash and \
            iname not in new_icons_d:
            new_icons_arr.append(iname)
            new_icons_d[iname] = 1
    return new_icons_arr

def process_group(prefix, uiconfigs):
    global global_list, global_hash
    group = {}
    cur_max = 1.0

    # a very noddy sorting algorithm
    for uiconfig in uiconfigs:
        images = read_new_icons(uiconfig, prefix)
        prev = ''
        for icon in images:
            if icon not in group:
                if prev not in group:
                    group[icon] = cur_max
                    cur_max += 1.0
                else:
                    group[icon] = group[prev] + (1.0 - 0.5 / cur_max)
    def intvalue(i):
        return group[i]
    for icon in sorted(group.keys(), key=intvalue):
        global_list.append(icon)
        global_hash[icon] = 1

def process_file(fname, prefix):
    global global_list, global_hash
    images = read_new_icons(fname, prefix)

    for icon in images:
        global_list.append(icon)
        global_hash[icon] = 1

def chew_controlfile(ifile):
    global global_list, global_hash
    filelist = []
    for line in ifile:
        line = line.strip()
        if line.startswith('#'):
            continue
        if not line:
            continue

        m = re.match(r'-- (\S+)\s*', line)
        if m:
            # control code
            code = m.group(1)
            small = line.lower().endswith(' small')
            if code.lower() == 'group':
                if not small:
                    process_group("lc_", filelist)
                process_group ("sc_", filelist)
            elif code.lower() == 'ordered':
                if not small:
                    for f in filelist:
                        process_file(f, "lc_")
                for f in filelist:
                    process_file(f, "sc_")
            elif code.lower() == 'literal':
                for f in filelist:
                    if f not in global_hash:
                        global_list.append(f)
                        global_hash[f] = 1
            else:
                sys.exit("Unknown code '{}'".format(code))
            filelist = []
        else:
            filelist.append(line)

parser = argparse.ArgumentParser()
# where the control file lives
parser.add_argument('control_file', metavar='image-sort.lst', type=open,
                    help='the sort control file')
# where the uiconfigs live
parser.add_argument('base_path', metavar='directory',
                    help='path to the UIConfigs directory')
parser.add_argument('output', metavar='output file', type=argparse.FileType('w'),
                    nargs='?', default=None, help='optionally write to this output file')
parser.add_argument("-q", "--quiet", action="store_true",
                    help="don't print status messages to stdout")

args = parser.parse_args()

if args.output is not None:
    close_output = True
else:
    args.output = sys.stdout
    close_output = False

chew_controlfile(args.control_file)

for icon in global_list:
    if not icon.startswith('sc_'):
        args.output.write(icon + "\n")

for icon in global_list:
    if icon.startswith('sc_'):
        args.output.write(icon + "\n")

if close_output:
    args.output.close()

# dnl vim:set shiftwidth=4 softtabstop=4 expandtab:

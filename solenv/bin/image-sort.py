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

global_list = []
global_hash = {}
base_path = None

def read_icons(fname):
    global base_path
    images = []
    full_path = os.path.join(base_path, fname)
    if not os.path.exists(full_path):
        print("Skipping non-existent {}\n".format(full_path))
        return images
    for line in open(full_path):
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

def chew_controlfile(fname):
    global global_list, global_hash
    filelist = []
    for line in open(fname):
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

if len(sys.argv) == 1:
    print("image-sort <image-sort.lst> /path/to/OOOo/source/root\n")
    sys.exit(1)

# where the control file lives
control = sys.argv[1]
# where the uiconfigs live
base_path = sys.argv[2]
# output
if len(sys.argv) > 3:
    output = open(sys.argv[3], 'w')
else:
    output = sys.stdout

chew_controlfile(control)

for icon in global_list:
    if not icon.startswith('sc_'):
        output.write(icon + "\n")

for icon in global_list:
    if icon.startswith('sc_'):
        output.write(icon + "\n")

# dnl vim:set shiftwidth=4 softtabstop=4 expandtab:

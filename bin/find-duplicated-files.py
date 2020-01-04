#!/usr/bin/env python3
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import os
import sys

from filecmp import dircmp

"""
This script compares two directories and lists the files which are the same in both directories.
Intended to find duplicate icons among icon themes.

Adopted from the example at https://docs.python.org/3.5/library/filecmp.html

Usage: ./bin/findduplicatefiles dir1 dir2
"""

def print_diff_files(dcmp):
    for name in dcmp.same_files:
        print("%s found in %s and %s" % (name, dcmp.left, dcmp.right))
    for sub_dcmp in dcmp.subdirs.values():
        print_diff_files(sub_dcmp)

if len(sys.argv) != 3:
    print("Usage: %s dir1 dir2" % sys.argv[0])
    exit()

dir1 = sys.argv[1]
dir2 = sys.argv[2]

if not os.path.isdir(dir1) or not os.path.isdir(dir2):
    print("Arguments must be directories!")
    exit()

dcmp = dircmp(dir1, dir2)
print_diff_files(dcmp)


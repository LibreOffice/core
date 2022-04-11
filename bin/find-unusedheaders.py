#!/usr/bin/env python3

# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

"""
Find dirs in:
workdir/Dep/CObject
workdir/Dep/CxxObject

Concat these files and compare them with the output of
`git ls-tree HEAD -r --name-only` and report files in the git ls-tree that aren't in the first.
"""

import os
import subprocess


def get_files_dict_recursively(directory):
    data = {}
    for root, _, files in os.walk(directory, topdown=False):
        for f in files:
            basename = os.path.splitext(f)[0]
            data[basename] = os.path.join(root, f)
    return data


def main():
    data = {}
    for d in ('workdir/Dep/CObject', 'workdir/Dep/CxxObject'):
        tmp = get_files_dict_recursively(d)
        data.update(tmp)

    gitfiles = subprocess.check_output(['git', 'ls-tree', 'HEAD', '-r', '--name-only']).decode('utf-8').split('\n')

    for f in gitfiles:
        ext = os.path.splitext(f)[1]
        if ext[1:] in ('c', 'cxx', 'h', 'hxx'):
            tmp = os.path.basename(f)
            tmp = os.path.splitext(tmp)[0]
            if tmp not in data:
                print(f)

if __name__ == '__main__':
    main()

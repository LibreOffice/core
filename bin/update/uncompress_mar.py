#!/usr/bin/env python3
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Extract a mar file and uncompress the content

import os
import sys
import subprocess

def uncompress_content(file_path):
    bzip2 = os.environ.get('BZIP2', 'bzip2')
    file_path_compressed = file_path + ".bz2"
    os.rename(file_path, file_path_compressed)
    subprocess.check_call(["bzip2", "-d", file_path_compressed])

def extract_mar(mar_file, target_dir):
    mar = os.environ.get('MAR', 'mar')
    subprocess.check_call([mar, "-C", target_dir, "-x", mar_file])
    file_info = subprocess.check_output([mar, "-t", mar_file])
    lines = file_info.splitlines()
    for line in lines:
        info = line.split()
        # ignore header line
        if info[2] == b'NAME':
            continue

        uncompress_content(os.path.join(target_dir, info[2].decode("utf-8")))

def main():
    if len(sys.argv) != 3:
        print("Help: This program takes exactly two arguments pointing to a mar file and a target location")
        sys.exit(1)

    mar_file = sys.argv[1]
    target_dir = sys.argv[2]
    extract_mar(mar_file, target_dir)

if __name__ == "__main__":
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

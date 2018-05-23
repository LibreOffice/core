# -*- coding: utf-8 -*-
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
""" Prepare and add SVG variants to sourceimagelist. """

from __future__ import with_statement
import argparse
import os
import sys

if __name__ == '__main__':
    parser = argparse.ArgumentParser("Prepare and add SVG variants to sourceimagelist")
    parser.add_argument('-l', '--imagelist-file', dest='imagelist_file',
                        action='store', required=True,
                        help='file containing list of image list file')

    ARGS = parser.parse_args()

    # Sanity checks
    if not os.path.exists(ARGS.imagelist_file):
        sys.exit(2)

    stored_lines = []

    with open(ARGS.imagelist_file, 'r') as source_file:
        for line in source_file:
            stored_lines.append(line)

    with open(ARGS.imagelist_file, 'w') as target_file:
        for line in stored_lines:
            target_file.write(line)
            target_file.write(line.replace(".png", ".svg"))

    sys.exit(0)

# vim: set shiftwidth=4 softtabstop=4 expandtab:

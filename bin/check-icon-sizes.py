#!/usr/bin/env python3
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import os

from PIL import Image

"""
This script walks through all icon files and checks whether the sc_* and lc_* files have the correct size.
"""

icons_folder = os.path.abspath(os.path.join(__file__, '..', '..', 'icon-themes'))

def check_size(filename, size):
    image = Image.open(filename)
    width, height = image.size
    if width != size or height != size:
        print("%s has size %dx%d but should have %dx%d" % (filename, width, height, size, size))

for root, dirs, files in os.walk(icons_folder):
    for filename in files:
        if not filename.endswith('png'):
            continue
        if filename.startswith('lc_'):
            check_size(os.path.join(root, filename), 24)
        elif filename.startswith('sc_'):
            check_size(os.path.join(root, filename), 16)


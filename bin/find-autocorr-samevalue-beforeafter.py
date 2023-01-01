#!/usr/bin/env python3

# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Use this script to find the lines in extras/source/autocorr/lang/<language>/DocumentList.xml
# which contain the same value for abbreviated-name and name
# Usage sample: ./find-autocorr-samevalue-beforeafter.py

import os
import xml.etree.ElementTree as ET

root_dir = '../extras/source/autocorr/lang'

bAllFilesOk = True

for root, dirs, files in os.walk(root_dir):
  for file in files:

    # just deal with DocumentList.xml, ignore the other files
    if (file != "DocumentList.xml"):
      continue
    complete_file = os.path.join(str(root), file)
    # parse the XML file
    tree = ET.parse(complete_file)
    root = tree.getroot()

    # find all elements X
    elements_x = root.findall('.//block-list:block', namespaces={'block-list': "http://openoffice.org/2001/block-list"})
    for element in elements_x:
      # get the value of the attribute "abbreviated-name"
      value_a = element.get('{http://openoffice.org/2001/block-list}abbreviated-name')
      # get the value of the attribute "name"
      value_b = element.get('{http://openoffice.org/2001/block-list}name')
      # check if the values are equal
      if value_a == value_b:
        print('In ' + complete_file + ' same value: ' + value_a)
        bAllFilesOk = False

if bAllFilesOk == True:
  exit(0)
exit(1)

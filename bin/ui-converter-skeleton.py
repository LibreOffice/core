#!/bin/python
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# this should parse a .ui and overwrite it with the same content
# for a in `git ls-files "*.ui"`; do bin/ui-converter-skeleton.py $a; done

import lxml.etree as etree
import sys

with open(sys.argv[1], encoding="utf-8") as f:
  header = f.readline()
  firstline = f.readline()
  # the comment after the xml declaration goes missing unless we provide a
  # custom doctype with tostring that contains the comment as a line after
  # the true doctype
  if firstline.startswith("<!--"):
    header = header + firstline
  f.seek(0)
  # remove_blank_text so pretty-printed input doesn't disrupt pretty-printed
  # output if nodes are added or removed
  parser = etree.XMLParser(remove_blank_text=True)
  tree = etree.parse(f, parser)
  # make sure <property name="label" translatable="no"></property> stays like that
  # and doesn't change to <property name="label" translatable="no"/>
  for status_elem in tree.xpath("//property[@name='label' and string() = '']"):
    status_elem.text = ""
  root = tree.getroot()

# do some targeted conversion here

with open(sys.argv[1], 'wb') as o:
  # without encoding='unicode' (and the matching encode("utf8")) we get &#XXXX replacements for non-ascii characters
  # which we don't want to see changed in the output
  o.write(etree.tostring(root, pretty_print=True, method='xml', encoding='unicode', doctype=header[0:-1]).encode("utf8"))

# vim: set shiftwidth=4 softtabstop=4 expandtab:

# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import gdb
import six

from libreoffice.util import printing

class ItemSetPrinter(object):
    '''Prints SfxItemSets'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        whichranges = self.which_ranges()
        return "SfxItemSet of pool %s with parent %s and Which ranges: %s" \
                % (self.value['m_pPool'], self.value['m_pParent'], whichranges)

    def which_ranges(self):
        whichranges = self.value['m_pWhichRanges']
        index = 0
        whiches = []
        while (whichranges[index]):
            whiches.append((int(whichranges[index]), int(whichranges[index+1])))
            index = index + 2
        return whiches

    def children(self):
        children = [ ( 'items', self.value['m_aItems'] ) ]
        return children.__iter__()

printer = None

def build_pretty_printers():
    global printer

    printer = printing.Printer("libreoffice/svl")

    printer.add('SfxItemSet', ItemSetPrinter)

def register_pretty_printers(obj):
    printing.register_pretty_printer(printer, obj)

build_pretty_printers()

# vim:set shiftwidth=4 softtabstop=4 expandtab:

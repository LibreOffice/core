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
        whichranges = self.value['m_aWhichRanges']['m_pairs']
        whichranges_cnt = self.value['m_aWhichRanges']['m_size']
        whiches = []
        for index in range(whichranges_cnt):
            whiches.append((int(whichranges[index]['first']), int(whichranges[index]['second'])))
        return whiches

    def children(self):
        return self._iterator(self.value['m_aPoolItemMap'])

    class _iterator(six.Iterator):

        def __init__(self, data):
            self.iter = gdb.default_visualizer(data).children()

        def __iter__(self):
            return self

        def __next__(self):
            # unordered_map iter is rather weird?
            (_, key) = self.iter.__next__()
            (_, elem) = self.iter.__next__()

            if (elem == -1):
                elem = "(Invalid)"
            elif (elem != 0):
                # let's try how well that works...
                elem = elem.cast(elem.dynamic_type).dereference()
            return ("[" + str(key) + "]", elem)

printer = None

def build_pretty_printers():
    global printer

    printer = printing.Printer("libreoffice/svl")

    printer.add('SfxItemSet', ItemSetPrinter)

def register_pretty_printers(obj):
    printing.register_pretty_printer(printer, obj)

build_pretty_printers()

# vim:set shiftwidth=4 softtabstop=4 expandtab:

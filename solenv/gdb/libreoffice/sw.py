# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       David Tardon, Red Hat Inc. <dtardon@redhat.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

from libreoffice.util import printing

class SwPositionPrinter(object):
    '''Prints SwPosition.'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        node = self.value['nNode']['pNd'].dereference();
        offset = self.value['nContent']['nIndex']
        return "%s (node %d, offset %d)" % (self.typename, node['nOffset'], offset)

class SwPaMPrinter(object):
    '''Prints SwPaM.'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        return "%s" % (self.typename)
        
    def children(self):
        point = self.value['m_pPoint'].dereference()
        mark = self.value['m_pMark'].dereference()
        children = [ ( 'point', point), ( 'mark', mark ) ]
        return children.__iter__()


class BigPtrArrayPrinter(object):
    '''Prints BigPtrArray.'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        length = self.value['nSize']
        if length > 0:
            return "%s of length %d" % (self.typename, length)
        else:
            return "empty %s" % self.typename

    def children(self):
        return self._iterator(self.value)

    def display_hint(self):
        return 'array'


    class _iterator(object):

        def __init__(self, array):
            self.blocks = array['ppInf']
            self.count = array['nSize']
            self.pos = 0
            self.block_count = array['nBlock']
            self.block_pos = 0
            self.block = None
            self._next_block(False)
            self._check_invariant()

        def __iter__(self):
            return self

        def next(self):
            if self.pos == self.count:
                raise StopIteration()

            name = str(self.pos)
            value = self.block['pData'][self.pos - self.block['nStart']]
            if self.pos == self.block['nEnd']:
                self._next_block()
            self.pos += 1

            self._check_invariant()
            return (name, value)

        def _next_block(self, advance = True):
            if advance:
                self.block_pos += 1

            if self.block_pos == self.block_count:
                return

            pblock = self.blocks[self.block_pos]
            assert pblock
            block = pblock.dereference()
            start = block['nStart']
            end = block['nEnd']
            assert end - start + 1 == block['nElem']
            if self.block:
                assert start == self.block['nEnd'] + 1
                assert end <= self.count
            else:
                assert start == 0
            self.block = block

        def _check_invariant(self):
            assert self.pos <= self.count
            assert self.block_pos <= self.block_count
            if self.pos == 0 and self.pos < self.count:
                assert self.block != None

printer = None

def build_pretty_printers():
    global printer

    printer = printing.Printer("libreoffice/sw")
    printer.add('BigPtrArray', BigPtrArrayPrinter)
    printer.add('SwPosition', SwPositionPrinter)
    printer.add('SwPaM', SwPaMPrinter)

def register_pretty_printers(obj):
    printing.register_pretty_printer(printer, obj)

build_pretty_printers()

# vim:set shiftwidth=4 softtabstop=4 expandtab:

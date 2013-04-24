# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import gdb

from libreoffice.util import printing

class SvArrayPrinter(object):
    '''Prints macro-declared arrays from svl module'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        if int(self.value['nA']):
            return "%s of length %d" % (self.typename, self.value['nA'])
        else:
            return "empty " + self.typename

    def children(self):
        return self._iterator(self.value['pData'], self.value['nA'])

    def display_hint(self):
        return 'array'

    class _iterator(object):

        def __init__(self, data, count):
            self.data = data
            self.count = count
            self.pos = 0
            self._check_invariant()

        def __iter__(self):
            return self

        def next(self):
            if self.pos == self.count:
                raise StopIteration()

            pos = self.pos
            elem = self.data[pos]
            self.pos = self.pos + 1

            self._check_invariant()
            return (str(pos), elem)

        def _check_invariant(self):
            assert self.count >= 0
            if self.count > 0:
                assert self.data
            assert self.pos >= 0
            assert self.pos <= self.count

    @staticmethod
    def query(type):
        if type.code == gdb.TYPE_CODE_REF:
            type = type.target()
        type = type.unqualified().strip_typedefs()

        if not type.tag:
            return False

        ushort = gdb.lookup_type('sal_uInt16')
        conforming = True
        for field in type.fields():
            if field.name == 'pData':
                conforming = field.type.code == gdb.TYPE_CODE_PTR
            elif field.name == 'nFree':
                conforming = field.type == ushort
            elif field.name == 'nA':
                conforming = field.type == ushort
            else:
                conforming = False
            if not conforming:
                return False

        try:
            gdb.lookup_type('FnForEach_' + type.tag)
        except RuntimeError:
            return False

        return True

printer = None

def build_pretty_printers():
    global printer

    printer = printing.Printer("libreoffice/svl")

    # macro-based arrays from svl module
    printer.add('SvArray', SvArrayPrinter, SvArrayPrinter.query)

def register_pretty_printers(obj):
    printing.register_pretty_printer(printer, obj)

build_pretty_printers()

# vim:set shiftwidth=4 softtabstop=4 expandtab:

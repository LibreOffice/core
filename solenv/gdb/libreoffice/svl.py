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

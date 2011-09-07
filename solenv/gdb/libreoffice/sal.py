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
from libreoffice.util.string import StringPrinterHelper

class RtlStringPrinter(StringPrinterHelper):
    '''Prints rtl_String or rtl_uString'''

    def __init__(self, typename, val, encoding = None):
        super(RtlStringPrinter, self).__init__(typename, val, encoding)

    def data(self):
        return self.val['buffer']

    def length(self):
        return self.val['length']

class StringPrinter(StringPrinterHelper):
    '''Prints rtl:: strings and string buffers'''

    def __init__(self, typename, val, encoding = None):
        super(StringPrinter, self).__init__(typename, val, encoding)

    def valid(self):
        return self.val['pData']

    def data(self):
        assert self.val['pData']
        return self.val['pData'].dereference()['buffer']

    def length(self):
        assert self.val['pData']
        return self.val['pData'].dereference()['length']

class SalUnicodePrinter(StringPrinterHelper):
    '''Prints a sal_Unicode*'''

    def __init__(self, typename, val):
        super(SalUnicodePrinter, self).__init__(typename, val, 'utf-16')

    def data(self):
        return self.val

    @staticmethod
    def query(type):
        type = type.unqualified()
        if type.code != gdb.TYPE_CODE_PTR:
            return False
        return str(type.target()) == 'sal_Unicode'

class RtlReferencePrinter(object):
    '''Prints rtl::Reference'''

    def __init__(self, typename, val):
        self.typename = typename
        self.val = val

    def to_string(self):
        print("RtlReferencePrinter:to_string")
        pointee = self.val['m_pBody']
        if pointee:
            val = pointee.dereference()
            return '%s to %s' % (self.typename, str(val))
        else:
            return "empty %s" % self.typename

printer = None

def build_pretty_printers():
    global printer

    printer = printing.Printer("libreoffice/sal")

    # strings and string buffers
    printer.add('_rtl_String', RtlStringPrinter)
    printer.add('_rtl_uString', lambda name, val: RtlStringPrinter(name,
        val, 'utf-16le'))
    printer.add('rtl::OString', StringPrinter)
    printer.add('rtl::OUString', lambda name, val: StringPrinter(name, val, 'utf-16'))
    printer.add('rtl::OStringBuffer', StringPrinter)
    printer.add('rtl::OUStringBuffer', lambda name, val: StringPrinter(name, val, 'utf-16'))
    printer.add('sal_Unicode', SalUnicodePrinter, SalUnicodePrinter.query)

    # other stuff
    printer.add('rtl::Reference', RtlReferencePrinter)

    return printer

def register_pretty_printers(obj):
    printing.register_pretty_printer(printer, obj)

build_pretty_printers()

# vim:set shiftwidth=4 softtabstop=4 expandtab:

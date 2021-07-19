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

class TranslateIdPrinter(object):
    '''Prints a TranslateId.'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        return self.value['mpContext'].format_string(format='s') + " " + self.value['mpId'].format_string(format='s')

printer = None

def build_pretty_printers():
    global printer

    printer = printing.Printer('libreoffice/utl')

    # various types
    printer.add('TranslateId', TranslateIdPrinter)

def register_pretty_printers(obj):
    printing.register_pretty_printer(printer, obj)

build_pretty_printers()

# vim:set shiftwidth=4 softtabstop=4 expandtab:

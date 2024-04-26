# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.util import printing

class OOXMLPropertySetPrinter(object):
    '''Prints writerfilter::ooxml::OOXMLPropertySet'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        return "%s" % (self.typename)

    def children(self):
        children = [ ( 'properties', self.value['mProperties'] ) ]
        return children.__iter__()

class OOXMLPropertyPrinter(object):
    '''Prints writerfilter::ooxml::OOXMLProperty'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        return "%s" % (self.typename)

    def children(self):
        children = [ ( 'id', self.value['mId'] ),
                     ( 'type', self.value['meType'] ),
                     ( 'value', self.value['mpValue'] ) ]
        return children.__iter__()

class OOXMLPropertySetValuePrinter(object):
    '''Prints writerfilter::ooxml::OOXMLPropertySetValue'''

    def __init__(self, typename, value):
        self.typename = typename
        self.value = value

    def to_string(self):
        return "%s" % (self.typename)

class OOXMLStringValuePrinter(object):
    '''Prints writerfilter::ooxml::OOXMLStringValue'''

    def __init__(self, typename, value):
        self.value = value

    def to_string(self):
        return "%s" % (self.value['mStr'])

class OOXMLIntegerValuePrinter(object):
    '''Prints writerfilter::ooxml::OOXMLIntegerValue'''

    def __init__(self, typename, value):
        self.value = value

    def to_string(self):
        return "%d" % (self.value['mnValue'])

printer = None

def build_pretty_printers():
    global printer

    printer = printing.Printer("libreoffice/sw_writerfilter")
    printer.add('writerfilter::ooxml::OOXMLProperty', OOXMLPropertyPrinter)
    printer.add('writerfilter::ooxml::OOXMLPropertySet', OOXMLPropertySetPrinter)
    printer.add('writerfilter::ooxml::OOXMLPropertySetValue', OOXMLPropertySetValuePrinter)
    printer.add('writerfilter::ooxml::OOXMLStringValue', OOXMLStringValuePrinter)
    printer.add('writerfilter::ooxml::OOXMLIntegerValue', OOXMLIntegerValuePrinter)
    printer.add('writerfilter::ooxml::OOXMLHexValue', OOXMLIntegerValuePrinter)

def register_pretty_printers(obj):
    printing.register_pretty_printer(printer, obj)

build_pretty_printers()

# vim:set shiftwidth=4 softtabstop=4 expandtab:

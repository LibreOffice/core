# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import gdb
import gdb.types
import six

from libreoffice.util import printing
from libreoffice.util.string import StringPrinterHelper

class RtlStringPrinter(StringPrinterHelper):
    '''Prints rtl_String or rtl_uString'''

    def __init__(self, typename, val, encoding = None):
        super(RtlStringPrinter, self).__init__(typename, val, encoding)

    def data(self):
        return self.val['buffer'].address

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
        return self.val['pData'].dereference()['buffer'].address

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
        pointee = self.val['m_pBody']
        if pointee:
            return '%s to %s' % (self.typename, str(pointee))
        else:
            return "empty %s" % self.typename

class OslFileStatusPrinter(object):
    '''Prints oslFileStatus'''

    def __init__(self, typename, val):
        self.val = val

    def to_string(self):
        osl_file_type = gdb.lookup_type('oslFileType').strip_typedefs()
        fields_to_enum_val = gdb.types.make_enum_dict(osl_file_type)

        etype = self.field_val_if_valid('eType')
        if etype is not None:
            pretty_etype = '<unknown type>' # in case it's not one of the fields

            for field_name, field_val in six.iteritems(fields_to_enum_val):
                if etype == field_val:
                    pretty_etype = self.pretty_file_type(field_name)
        else:
            pretty_etype = '<invalid type>'

        file_url = self.field_val_if_valid('ustrFileURL')
        if file_url is not None:
            pretty_file_url = str(file_url.dereference())
        else:
            pretty_file_url = '<invalid file url>'

        pretty_file_status = pretty_etype + ': ' + pretty_file_url

        # for links append the link target if valid
        if etype == fields_to_enum_val['osl_File_Type_Link']:
            link_target = self.field_val_if_valid('ustrLinkTargetURL')
            if link_target is None:
                pretty_link_target = '<invalid link target>'
            else:
                pretty_link_target = str(link_target.dereference())

            pretty_file_status += ' -> ' + pretty_link_target

        return pretty_file_status

    def pretty_file_type(self, file_type_name):
        if file_type_name != 'osl_File_Type_Regular':
            return file_type_name.replace('osl_File_Type_', '').lower()
        else:
            return 'file' # regular is not very descriptive, file is better

    def field_val_if_valid(self, field):
        mask_for_field = {'eType': 0x00000001,
                          'uAttributes': 0x00000002,
                          'aCreationTime': 0x00000010,
                          'aAccessTime': 0x00000020,
                          'aModifyTime': 0x00000040,
                          'uFileSize': 0x00000080,
                          'ustrFileName': 0x00000100,
                          'ustrFileURL': 0x00000200,
                          'ustrLinkTargetURL': 0x00000400}

        valid_fields = self.val['uValidFields']
        if valid_fields & mask_for_field[field]:
            return self.val[field]
        else:
            return None

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
    printer.add('_oslFileStatus', OslFileStatusPrinter)

    return printer

def register_pretty_printers(obj):
    printing.register_pretty_printer(printer, obj)

build_pretty_printers()

# vim:set shiftwidth=4 softtabstop=4 expandtab:

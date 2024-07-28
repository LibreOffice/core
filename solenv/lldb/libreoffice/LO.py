# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# To use, add something like this to your ~/.lldbinit:
# command script import '~/lo/sim/solenv/lldb/libreoffice/LO.py'

import lldb

def rtl_String_summary(valobj, dict):
    if valobj.TypeIsPointerType():
        return rtl_String_summary(valobj.Dereference(), dict)

    length = valobj.GetChildMemberWithName('length').GetValueAsUnsigned(0)
    buffer = valobj.GetChildMemberWithName('buffer')
    buffer_ptr = buffer.AddressOf()

#    return '"' + buffer_ptr.GetPointeeData(0, length).GetString(lldb.SBError(), 0) + '"'
    return sal_ascii_string(buffer_ptr, length)

def rtl_OString_summary(valobj, dict):
    return rtl_String_summary(valobj.GetChildMemberWithName('pData'), dict)

def sal_ascii_string(buffer_ptr, length):
    e = lldb.SBError()

    s = '"'
    i = 0
    while i < length:
        c = buffer_ptr.GetPointeeData(i, 1).GetUnsignedInt8(e, 0)
        if c == ord('"'):
            s = s + '\\"'
        elif c == ord('\\'):
            s = s + '\\\\'
        elif c == ord('\n'):
            s = s + '\\n'
        elif c == ord('\r'):
            s = s + '\\r'
        elif c == ord('\t'):
            s = s + '\\t'
        elif c < ord(' '):
            s = s + '\\{:03o}'.format(c)
        elif c < 127:
            s = s + chr(c)
        else:
            s = s + '\\u{:04x}'.format(c)
        i = i + 1
    s = s + '"'

    return s

def rtl_uString_summary(valobj, dict):
    # print "valobj = " + str(valobj) + ", valobj.GetData() = " + str(valobj.GetData()) + ", valobj.GetTypeName() = " + str(valobj.GetTypeName())

    # As we don't use --skip-pointers when doing the "type summary add" for this function,
    # the value to be printed might actually be a pointer to a rtl_uString. Weird, huh?
    if valobj.TypeIsPointerType():
        return rtl_uString_summary(valobj.Dereference(), dict)

    length = valobj.GetChildMemberWithName('length').GetValueAsUnsigned(0)
    buffer = valobj.GetChildMemberWithName('buffer')

    buffer_ptr = buffer.AddressOf()

    return sal_unicode_string(buffer_ptr, length)

def rtl_OUString_summary(valobj, dict):
    return rtl_uString_summary(valobj.GetChildMemberWithName('pData'), dict)

def sal_unicode_string(buffer_ptr, length):
    e = lldb.SBError()

    s = '"'
    i = 0
    while i < length:
        c = buffer_ptr.GetPointeeData(i, 1).GetUnsignedInt16(e, 0)
        if c == ord('"'):
            s = s + '\\"'
        elif c == ord('\\'):
            s = s + '\\\\'
        elif c == ord('\n'):
            s = s + '\\n'
        elif c == ord('\r'):
            s = s + '\\r'
        elif c == ord('\t'):
            s = s + '\\t'
        elif c < ord(' '):
            s = s + '\\{:03o}'.format(c)
        elif c < 127:
            s = s + chr(c)
        else:
            s = s + '\\u{:04x}'.format(c)
        i = i + 1
    s = s + '"'

    return s

# Automatically install the above summary functions when this is loaded
def __lldb_init_module(debugger, dict):
    debugger.HandleCommand("type summary add --skip-references --python-function LO.rtl_String_summary rtl_String")
    debugger.HandleCommand("type summary add --skip-pointers --skip-references --python-function LO.rtl_OString_summary rtl::OString")
    debugger.HandleCommand("type summary add --skip-references --python-function LO.rtl_uString_summary rtl_uString")
    debugger.HandleCommand("type summary add --skip-pointers --skip-references --python-function LO.rtl_OUString_summary rtl::OUString")

# vim:set shiftwidth=4 softtabstop=4 expandtab:

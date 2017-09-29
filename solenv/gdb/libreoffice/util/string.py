# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import gdb

from libreoffice.util.compatibility import use_lazy_string

class StringPrinterHelper(object):
    '''Base for all string pretty printers'''

    class MustBeImplemented(Exception):
        pass

    def __init__(self, typename, val, encoding = None):
        self.typename = typename
        self.val = val
        self.encoding = encoding

    def to_string(self):
        if self.valid():
            data = self.data()
            len = self.length()
            return self.make_string(data, self.encoding, len)
        else:
            return "uninitialized %s" % self.typename

    def display_hint(self):
        if self.valid():
            return 'string'
        else:
            return None

    def valid(self):
        return True

    def data(self):
        raise self.MustBeImplemented()

    def length(self):
        return -1

    @staticmethod
    def make_string(data, encoding = None, length = -1):
        '''Creates a new string from memory'''

        if not encoding:
            encoding = ''

        # we need to determine length, if not given (for sal_Unicode*)
        if length < 0:
            length = 0
            while data[length] != 0 and length <= 512: # arbitrary limit
                length += 1

        if use_lazy_string:
            return data.lazy_string(encoding, length)

        # The gdb.Value.string() conversion works on array of bytes, but
        # the length we have is the length of the string. So we must
        # multiply it by width of character if the string is Unicode.
        width = data[0].type.sizeof
        if width > 1:
            length = length * width

        char = gdb.lookup_type('char')
        bytes = data.cast(char.pointer())
        return bytes.string(encoding, length = length)

# vim:set shiftwidth=4 softtabstop=4 expandtab:

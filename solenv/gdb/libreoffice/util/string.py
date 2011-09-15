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
        data = self.data()
        len = self.length()
        if self.valid():
            return self.make_string(data, self.encoding, len)
        else:
            return "unintialized %s" % self.typename

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

        if use_lazy_string:
            return data.lazy_string(encoding, length)

        # we need to determine length, if not given (for sal_Unicode*)
        if length < 0:
            length = 0
            while data[length] != 0 and length <= 512: # arbitrary limit
                length += 1

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

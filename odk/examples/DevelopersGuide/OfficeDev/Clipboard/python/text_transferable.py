# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import uno
import unohelper

from com.sun.star.datatransfer import DataFlavor
from com.sun.star.datatransfer import UnsupportedFlavorException
from com.sun.star.datatransfer import XTransferable

# A simple transferable for the clipboard.py example
# contains only one format, that is, unicode text

class TextTransferable(unohelper.Base, XTransferable):
    UNICODE_CONTENT_TYPE = "text/plain;charset=utf-16"

    def __init__(self, text):
        self.text = text

    ''' XTransferable methods '''
    def getTransferData(self, flavor):
        # str.casefold() allows reliable case-insensitive comparision
        if flavor.MimeType.casefold() != __class__.UNICODE_CONTENT_TYPE.casefold():
            raise UnsupportedFlavorException()
        return self.text

    def getTransferDataFlavors(self):
        unicode_flavor = DataFlavor(__class__.UNICODE_CONTENT_TYPE, "Unicode Text", uno.getTypeByName("string"))
        return [unicode_flavor]

    def isDataFlavorSupported(self, flavor):
        # str.casefold() allows reliable case-insensitive comparision
        return flavor.MimeType.casefold() == __class__.UNICODE_CONTENT_TYPE.casefold()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import unohelper

from com.sun.star.datatransfer.clipboard import XClipboardOwner

# A simple clipboard owner for the clipboard.py example

class ClipboardOwner(unohelper.Base, XClipboardOwner):
    def __init__(self):
        self.is_owner = True

    def isClipboardOwner(self):
        return self.is_owner

    ''' XClipboardOwner methods '''
    def lostOwnership(self, x_clipboard, x_transferable):
        print("Lost clipboard ownership...")
        self.is_owner = False

# vim: set shiftwidth=4 softtabstop=4 expandtab:
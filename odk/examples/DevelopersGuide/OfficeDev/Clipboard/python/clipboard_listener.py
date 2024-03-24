# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import unohelper

from com.sun.star.datatransfer.clipboard import XClipboardListener

# A simple clipboard listener for the clipboard.py example

class ClipboardListener(unohelper.Base, XClipboardListener):

    ''' XClipboardListener methods '''
    def disposing(self, event):
        pass

    def changedContents(self, event):
        print("Clipboard content has changed!")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
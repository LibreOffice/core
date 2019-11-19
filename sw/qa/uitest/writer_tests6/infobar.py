# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.debug import sleep

from com.sun.star.beans import StringPair
from com.sun.star.frame import XInfobarProvider
from com.sun.star.frame import InfobarType

import org.libreoffice.unotest


# Test for Infobar API

class tdf126627(UITestCase):
    def test_infobar_add(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        buttons = [StringPair("eins", "zwei")]
        document.getCurrentController().appendInfobar("my", "Hello world", "The quick, brown fox jumps over a lazy dog.", InfobarType.INFO, buttons, True)
        sleep(10)
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

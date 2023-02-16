# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import get_url_for_data_file

class Tdf128610(UITestCase):

    def test_tdf128610(self):
        with self.ui_test.load_file(get_url_for_data_file('tdf128610.fodt')):
            self.xUITest.executeCommand('.uno:SelectAll')
            self.xUITest.executeCommand('.uno:Copy')

        with self.ui_test.load_empty_file("math"):

            self.xUITest.executeCommand('.uno:ImportMathMLClipboard')

            xMathDoc = self.xUITest.getTopFocusWindow()
            xEditView = xMathDoc.getChild("editview")

            # Without the fix in place, this test would have failed with
            # AssertionError: '{ f _ c = frac { 1 } { K _ m } }' != ''
            self.assertEqual("{ f _ c = { frac { 1 } { K _ m } } }", get_state_as_dict(xEditView)["Text"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:

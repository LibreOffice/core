# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_url_for_data_file

class tdf54768(UITestCase):
    def test_tdf54768(self):
        # This document contains an image with an original size of 7.99cm x 5.74cm.
        # The image has been cropped 2.73cm at the top.
        # Also, it has been resized to a width of 4.04cm.
        with self.ui_test.load_file(get_url_for_data_file("tdf54768.ods")) as document:
            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")

            # select the image
            xGridWin.executeAction("SELECT", mkPropertyValues({"OBJECT":"Image"}))

            # click "Original Size"
            self.xUITest.executeCommand(".uno:OriginalSize")

            # tdf#155863: Without the fix in place, the image stays cropped,
            #             but stretches to the size of original image
            self.assertEqual(7988, document.DrawPages[0][0].Size.Width)
            self.assertEqual(3005, document.DrawPages[0][0].Size.Height)

# vim: set shiftwidth=4 softtabstop=4 expandtab:

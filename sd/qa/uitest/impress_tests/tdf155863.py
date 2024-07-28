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

class tdf155863(UITestCase):
    def test_tdf155863(self):
        # This document contains an image with an original size of 7.99cm x 5.74cm.
        # The image has been cropped 0.8cm on the left, 3.59cm on the right, 0.99cm at the top,
        # and 0.75cm at the bottom.
        # Also, it has been resized to a width of 2.1cm and a height of 2.33cm.
        with self.ui_test.load_file(get_url_for_data_file("tdf155863.odp")) as document:
            xImpressDoc = self.xUITest.getTopFocusWindow()
            xEditWin = xImpressDoc.getChild("impress_win")

            # select the image on the first slide
            xEditWin.executeAction("SELECT", mkPropertyValues({"OBJECT":"Image"}))

            # click "Original Size"
            self.xUITest.executeCommand(".uno:OriginalSize")

            # tdf#155863: Without the fix in place, the image will be distorted due to the wrong size.
            self.assertEqual(3597, document.DrawPages[0][0].Size.Width)
            self.assertEqual(3998, document.DrawPages[0][0].Size.Height)

# vim: set shiftwidth=4 softtabstop=4 expandtab:

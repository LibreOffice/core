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

class tdf146019(UITestCase):

    def test_tdf146019(self):

        with self.ui_test.create_doc_in_start_center("impress") as document:

            xTemplateDlg = self.xUITest.getTopFocusWindow()
            xCancelBtn = xTemplateDlg.getChild("close")
            self.ui_test.close_dialog_through_button(xCancelBtn)

            with self.ui_test.execute_dialog_through_command(".uno:InsertGraphic", close_button="open") as xOpenDialog:

                xFileName = xOpenDialog.getChild("file_name")
                xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("tdf146019.jpg")}))

            # Before the fix, a dialog was displayed at this point

            # Check the shape is rotated, height > width
            drawPage = document.getDrawPages().getByIndex(0)
            shape = drawPage.getByIndex(2)
            self.assertEqual(8996, shape.getSize().Width)
            self.assertEqual(11745, shape.getSize().Height)

# vim: set shiftwidth=4 softtabstop=4 expandtab:

# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf114710(UITestCase):

    def test_tdf114710(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf114710.ods")):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:O7"}))
            self.xUITest.executeCommand(".uno:Copy")

        with self.ui_test.load_empty_file("writer") as writer_document:

            self.xUITest.getTopFocusWindow()

            with self.ui_test.execute_dialog_through_command(".uno:PasteSpecial") as xDialog:

                xList = xDialog.getChild('list')

                for childName in xList.getChildren():
                    xChild = xList.getChild(childName)
                    if get_state_as_dict(xChild)['Text'] == "Graphics Device Interface metafile (GDI)":
                        break

                xChild.executeAction("SELECT", tuple())
                self.assertEqual(
                        get_state_as_dict(xList)['SelectEntryText'], "Graphics Device Interface metafile (GDI)")


            # Without the fix in place, this test would have crashed here
            self.assertEqual(1, writer_document.GraphicObjects.getCount())

# vim: set shiftwidth=4 softtabstop=4 expandtab:

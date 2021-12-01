# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf140506(UITestCase):

    def test_tdf140506(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf140506.odt")) as writer_doc:

            xMainWindow = self.xUITest.getTopFocusWindow()
            xwriter_edit = xMainWindow.getChild("writer_edit")

            tables = writer_doc.getTextTables()
            self.assertEqual(len(tables[0].getRows()), 20)
            self.assertEqual(len(tables[0].getColumns()), 2)

            self.assertEqual(2, writer_doc.CurrentController.PageCount)

            with self.ui_test.execute_dialog_through_command(".uno:TableDialog") as xDialog:
                tabcontrol = xDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "1")

                headline = xDialog.getChild("headline")
                repeatheader = xDialog.getChild("repeatheadernf")
                self.assertEqual('false', get_state_as_dict(headline)["Selected"])
                headline.executeAction("CLICK", tuple())
                self.assertEqual('true', get_state_as_dict(headline)["Selected"])
                self.assertEqual('1', get_state_as_dict(repeatheader)["Value"])


            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.processEventsToIdle()

            self.assertEqual(len(tables[0].getRows()), 20)
            self.assertEqual(len(tables[0].getColumns()), 2)

            # Without the fix in place, this test would have failed with
            # AssertionError: 20 != 2
            self.assertEqual(20, writer_doc.CurrentController.PageCount)


# vim: set shiftwidth=4 softtabstop=4 expandtab:

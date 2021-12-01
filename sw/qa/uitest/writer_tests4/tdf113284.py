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
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class tdf113284(UITestCase):

   def test_tdf113284(self):
    with self.ui_test.load_file(get_url_for_data_file("tdf113284.odt")) as writer_doc:
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
        xToolkit.processEventsToIdle()

        xPageCount = writer_doc.CurrentController.PageCount
        with self.ui_test.execute_dialog_through_command(".uno:GotoPage") as xDialog:
            xPageText = xDialog.getChild("page")
            xPageText.executeAction("TYPE", mkPropertyValues({"TEXT":str(xPageCount)})) # goto last page

        xToolkit.processEventsToIdle()

        self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], str(xPageCount))
        with self.ui_test.execute_dialog_through_command(".uno:EditCurIndex", close_button="cancel"):
            pass

        #page count  is not constant
        #self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "66")    #page 66 start of the Index
        #pagecount unchanged
        self.assertEqual(writer_doc.CurrentController.PageCount, xPageCount)

# vim: set shiftwidth=4 softtabstop=4 expandtab:

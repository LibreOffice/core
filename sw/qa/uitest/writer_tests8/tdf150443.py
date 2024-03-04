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

class tdf150443(UITestCase):

    def test_tdf150443(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf150443.docx")):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')

            # search term "Jump here!"
            with self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog", close_button="close") as xDialog:
                searchterm = xDialog.getChild("searchterm")
                searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"Jump here!"}))
                xsearch = xDialog.getChild("search")
                xsearch.executeAction("CLICK", tuple())  #first search
                xToolkit.processEventsToIdle()
                page = get_state_as_dict(xWriterEdit)["CurrentPage"]
                # page may depend on font substitution, just check it moved
                self.assertTrue(page == "4" or page == "5")

            # reject the tracked table row in Manage Changes dialog window
            with self.ui_test.execute_modeless_dialog_through_command(".uno:AcceptTrackedChanges", close_button="close") as xTrackDlg:
                changesList = xTrackDlg.getChild("writerchanges")

                items = len(changesList.getChildren())

                # select tree parent of the actual tracked row deletion in tree list
                changesList.executeAction("TYPE", mkPropertyValues({"KEYCODE": "LEFT"}))
                xToolkit.processEventsToIdle()

                # Without the fix in place, it would have crashed here
                xAccBtn = xTrackDlg.getChild("reject")
                xAccBtn.executeAction("CLICK", tuple())
                self.assertEqual(items - 1, len(changesList.getChildren()))

# vim: set shiftwidth=4 softtabstop=4 expandtab:

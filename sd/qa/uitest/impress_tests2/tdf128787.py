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
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file, select_by_text

class tdf128787(UITestCase):

  def test_tdf128787(self):
    with self.ui_test.load_file(get_url_for_data_file("tdf128787.odp")):
        xImpressDoc = self.xUITest.getTopFocusWindow()
        xEditWin = xImpressDoc.getChild("impress_win")

        # Without opening the StyleListPanel, executing_dialog_through_command
        # .uno:StyleNewByExample doesn't work as expected in the test environment.
        # Perhaps this is required so the styles are loaded. The sidebar can be closed after
        # this and .uno:StyleNewByExample will work but for this test this is not wanted.
        xEditWin.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "StyleListPanel"}))

        # wait until the template panel is available
        xTemplatePanel = self.ui_test.wait_until_child_is_available('TemplatePanel')

        # select the image
        xEditWin.executeAction("SELECT", mkPropertyValues({"OBJECT":"Image 1"}))

        # New Style from Selection [uno:StyleNewByExample]
        with self.ui_test.execute_dialog_through_command(".uno:StyleNewByExample") as xDialog:
            # Enter a name in the Create Style dialog and press OK
            stylename = xDialog.getChild("stylename")
            stylename.executeAction("TYPE", mkPropertyValues({"TEXT": "New Style"}))

        # make sure filter is set "Hierarchical"' so the 'treeview' tree is used
        xFilter = xTemplatePanel.getChild('filter')
        select_by_text(xFilter, "Hierarchical")

        xTreeView = xTemplatePanel.getChild('treeview')
        # "New Style" should be the first child of the first child in the tree
        xItem = xTreeView.getChild(0).getChild(0)
        self.assertEqual("New Style", get_state_as_dict(xItem)['Text'])

# vim: set shiftwidth=4 softtabstop=4 expandtab:

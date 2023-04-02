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
import time

class tdf154212(UITestCase):

    def test_tdf154212(self):

        with self.ui_test.load_file(get_url_for_data_file('tdf154212.odt')):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild('writer_edit')

            self.xUITest.executeCommand('.uno:Sidebar')
            xWriterEdit.executeAction('SIDEBAR', mkPropertyValues({'PANEL': 'SwNavigatorPanel'}))

            # wait until the navigator panel is available
            xNavigatorPanel = self.ui_test.wait_until_child_is_available('NavigatorPanel')

            # HACK, see the `m_aUpdTimer.SetTimeout(1000)` in the SwContentTree ctor in
            # sw/source/uibase/utlui/content.cxx, where that m_aUpdTimer is started by
            # SwContentTree::ShowTree triggered from the SIDEBAR action above, and which can
            # invalidate the TreeListEntryUIObjects used by the below code (see
            # 2798430c8a711861fdcdfbf9ac00a0527abd3bfc "Mark the uses of TreeListEntryUIObject as
            # dubious"); lets double that 1000 ms timeout value here to hopefully be on the safe
            # side:
            time.sleep(2)

            xNavigatorPanelContentTree = xNavigatorPanel.getChild("contenttree")

            xNavigatorPanelContentTreeHeadings = xNavigatorPanelContentTree.getChild('0')
            xNavigatorPanelContentTreeHeadings.executeAction("EXPAND", tuple())

            xHeadingsChild0 = xNavigatorPanelContentTreeHeadings.getChild('0')
            xHeadingsChild0.executeAction("EXPAND", tuple())

            xHeadingsChild0Child2 = xHeadingsChild0.getChild('2')
            self.assertEqual(get_state_as_dict(xHeadingsChild0Child2)["Text"], "MOVE THIS Heading level 2")

            # double click on the entry to select and set focus
            xHeadingsChild0Child2.executeAction("DOUBLECLICK", tuple())

            # click on the 'move chapter down' button in the Navigator tool box
            xNavigatorPanel = xWriterEdit.getChild("NavigatorPanel")
            xToolBarContent6 = xNavigatorPanel.getChild("content6")
            xToolBarContent6.executeAction("CLICK", mkPropertyValues({"POS": "5"}))

            self.ui_test.wait_until_property_is_updated(xNavigatorPanelContentTree, "SelectEntryText", "MOVE THIS Heading level 2")

            xNavigatorPanelContentTreeHeadings = xNavigatorPanelContentTree.getChild('0')

            xHeadingsChild0 = xNavigatorPanelContentTreeHeadings.getChild('0')

            xHeadingsChild0Child4 = xHeadingsChild0.getChild('4')
            self.assertEqual(get_state_as_dict(xHeadingsChild0Child4)["Text"], "MOVE THIS Heading level 2")

            # click on the 'move chapter up' button in the Navigator tool box
            xNavigatorPanel = xWriterEdit.getChild("NavigatorPanel")
            xToolBarContent6 = xNavigatorPanel.getChild("content6")
            xToolBarContent6.executeAction("CLICK", mkPropertyValues({"POS": "4"}))

            self.ui_test.wait_until_property_is_updated(xNavigatorPanelContentTree, "SelectEntryText", "MOVE THIS Heading level 2")

            xNavigatorPanelContentTreeHeadings = xNavigatorPanelContentTree.getChild('0')

            xHeadingsChild0 = xNavigatorPanelContentTreeHeadings.getChild('0')

            # Without the fix in place, this test would have failed with
            # AssertionError: 'MOVE THIS Heading level 2' != 'Heading level 1'
            self.assertEqual(get_state_as_dict(xHeadingsChild0)["Text"], "Heading level 1")

            xHeadingsChild0Child2 = xHeadingsChild0.getChild('2')
            self.assertEqual(get_state_as_dict(xHeadingsChild0Child2)["Text"], "MOVE THIS Heading level 2")

            self.xUITest.executeCommand('.uno:Sidebar')

# vim: set shiftwidth=4 softtabstop=4 expandtab:

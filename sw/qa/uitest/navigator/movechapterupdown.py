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

class movechapterupdown(UITestCase):

    def test_movechapterupdown(self):

        with self.ui_test.load_file(get_url_for_data_file('MoveChapterUpDown.odt')):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild('writer_edit')

            self.xUITest.executeCommand('.uno:Sidebar')
            xWriterEdit.executeAction('SIDEBAR', mkPropertyValues({'PANEL': 'SwNavigatorPanel'}))

            # wait until the navigator panel is available
            xNavigatorPanel = self.ui_test.wait_until_child_is_available('NavigatorPanel')

            # See the `m_aUpdateTimer.SetTimeout(200)` (to "avoid flickering of buttons")
            # in the SwChildWinWrapper ctor in sw/source/uibase/fldui/fldwrap.cxx, where that
            # m_aUpdateTimer is started by SwChildWinWrapper::ReInitDlg triggered from the
            # xInsert click above.
            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.waitUntilAllIdlesDispatched()

            # Given the document chapter structure:
                # 1. One H1
                #   1.1. one_A (H2)
                #   1.2. one_B (H2)
                # 2. Two (H1)
                #     A heading of level 3
                #   2.1. Two_A (H2)
                #   2.1. Two_B (H2)
                # 3. Three (H1)
                #   3.1. Three_A (H2)
                #   3.2. Three_B (H2)

            xNavigatorPanelContentTree = xNavigatorPanel.getChild("contenttree")

            xNavigatorPanelContentTreeHeadings = xNavigatorPanelContentTree.getChild('0')
            xNavigatorPanelContentTreeHeadings.executeAction("EXPAND", tuple())

            #
            # test a level 1 chapter move up then move down
            #

            # Double click on the "2. Two (H1)" entry to select and set focus
            xHeadingsChild1 = xNavigatorPanelContentTreeHeadings.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild1)["Text"], "2. Two (H1)")
            xHeadingsChild1.executeAction("DOUBLECLICK", tuple())

            self.ui_test.wait_until_property_is_updated(xNavigatorPanelContentTree, "SelectEntryText", "2. Two (H1)")

            # Click on the 'Move chapter up' button in the Navigator tool box
            xNavigatorPanel = xWriterEdit.getChild("NavigatorPanel")
            xToolBar = xNavigatorPanel.getChild("HeadingsContentFunctionButtonsToolbar")
            xToolBar.executeAction("CLICK", mkPropertyValues({"POS": "2"}))

            # Expected chapter order:
                # 2. Two (H1)
                #     A heading of level 3
                #   2.1. Two_A (H2)
                #   2.1. Two_B (H2)
                # 1. One (H1)
                #   1.1. One_A (H2)
                #   1.2. One_B (H2)
                # 3. Three (H1)
                #   3.1. Three_A (H2)
                #   3.2. Three_B (H2)

            xNavigatorPanelContentTreeHeadings = xNavigatorPanelContentTree.getChild('0')

            xHeadingsChild0 = xNavigatorPanelContentTreeHeadings.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild0)["Text"], "2. Two (H1)")
            xHeadingsChild0Child0 = xHeadingsChild0.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild0Child0)["Text"], "A heading of level 3")
            xHeadingsChild0Child1 = xHeadingsChild0.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild0Child1)["Text"], "2.1. Two_A (H2)")
            xHeadingsChild0Child2 = xHeadingsChild0.getChild('2')
            self.assertEqual(get_state_as_dict(xHeadingsChild0Child2)["Text"], "2.1. Two_B (H2)")

            xHeadingsChild1 = xNavigatorPanelContentTreeHeadings.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild1)["Text"], "1. One (H1)")
            xHeadingsChild1Child0 = xHeadingsChild1.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child0)["Text"], "1.1. One_A (H2)")
            xHeadingsChild1Child1 = xHeadingsChild1.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child1)["Text"], "1.2. One_B (H2)")

            xHeadingsChild2 = xNavigatorPanelContentTreeHeadings.getChild('2')
            self.assertEqual(get_state_as_dict(xHeadingsChild2)["Text"], "3. Three (H1)")
            xHeadingsChild2Child0 = xHeadingsChild2.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild2Child0)["Text"], "3.1. Three_A (H2)")
            xHeadingsChild2Child1 = xHeadingsChild2.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild2Child1)["Text"], "3.2. Three_B (H2)")

            # Click on the 'Move chapter down' button in the Navigator tool box
            xNavigatorPanel = xWriterEdit.getChild("NavigatorPanel")
            xToolBar = xNavigatorPanel.getChild("HeadingsContentFunctionButtonsToolbar")
            xToolBar.executeAction("CLICK", mkPropertyValues({"POS": "3"}))

            # Expected chapter order is the original order
            xNavigatorPanelContentTreeHeadings = xNavigatorPanelContentTree.getChild('0')

            xHeadingsChild0 = xNavigatorPanelContentTreeHeadings.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild0)["Text"], "1. One (H1)")
            xHeadingsChild0Child0 = xHeadingsChild0.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild0Child0)["Text"], "1.1. One_A (H2)")
            xHeadingsChild0Child1 = xHeadingsChild0.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild0Child1)["Text"], "1.2. One_B (H2)")

            xHeadingsChild1 = xNavigatorPanelContentTreeHeadings.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild1)["Text"], "2. Two (H1)")
            xHeadingsChild1Child0 = xHeadingsChild1.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child0)["Text"], "A heading of level 3")
            xHeadingsChild1Child1 = xHeadingsChild1.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child1)["Text"], "2.1. Two_A (H2)")
            xHeadingsChild1Child2 = xHeadingsChild1.getChild('2')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child2)["Text"], "2.1. Two_B (H2)")

            xHeadingsChild2 = xNavigatorPanelContentTreeHeadings.getChild('2')
            self.assertEqual(get_state_as_dict(xHeadingsChild2)["Text"], "3. Three (H1)")
            xHeadingsChild2Child0 = xHeadingsChild2.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild2Child0)["Text"], "3.1. Three_A (H2)")
            xHeadingsChild2Child1 = xHeadingsChild2.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild2Child1)["Text"], "3.2. Three_B (H2)")

            #
            # test a level 1 chapter move down then move up
            #

            # Double click on the "2. Two (H1)" entry to select and set focus
            xHeadingsChild1 = xNavigatorPanelContentTreeHeadings.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild1)["Text"], "2. Two (H1)")
            xHeadingsChild1.executeAction("DOUBLECLICK", tuple())

            self.ui_test.wait_until_property_is_updated(xNavigatorPanelContentTree, "SelectEntryText", "2. Two (H1)")

            # Click on the 'Move chapter down' button in the Navigator tool box
            xNavigatorPanel = xWriterEdit.getChild("NavigatorPanel")
            xToolBar = xNavigatorPanel.getChild("HeadingsContentFunctionButtonsToolbar")
            xToolBar.executeAction("CLICK", mkPropertyValues({"POS": "3"}))

            # Expected chapter order:
                # 1. One (H1)
                #   1.1. One_A (H2)
                #   1.2. One_B (H2)
                # 3. Three (H1)
                #   3.1. Three_A (H2)
                #   3.2. Three_B (H2)
                # 2. Two (H1)
                #     A heading of level 3
                #   2.1. Two_A (H2)
                #   2.1. Two_B (H2)

            xNavigatorPanelContentTreeHeadings = xNavigatorPanelContentTree.getChild('0')

            xHeadingsChild0 = xNavigatorPanelContentTreeHeadings.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild0)["Text"], "1. One (H1)")
            xHeadingsChild0Child0 = xHeadingsChild0.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild0Child0)["Text"], "1.1. One_A (H2)")
            xHeadingsChild0Child1 = xHeadingsChild0.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild0Child1)["Text"], "1.2. One_B (H2)")

            xHeadingsChild1 = xNavigatorPanelContentTreeHeadings.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild1)["Text"], "3. Three (H1)")
            xHeadingsChild1Child0 = xHeadingsChild1.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child0)["Text"], "3.1. Three_A (H2)")
            xHeadingsChild1Child1 = xHeadingsChild1.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child1)["Text"], "3.2. Three_B (H2)")

            xHeadingsChild2 = xNavigatorPanelContentTreeHeadings.getChild('2')
            self.assertEqual(get_state_as_dict(xHeadingsChild2)["Text"], "2. Two (H1)")
            xHeadingsChild2Child0 = xHeadingsChild2.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild2Child0)["Text"], "A heading of level 3")
            xHeadingsChild2Child1 = xHeadingsChild2.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild2Child1)["Text"], "2.1. Two_A (H2)")
            xHeadingsChild2Child2 = xHeadingsChild2.getChild('2')
            self.assertEqual(get_state_as_dict(xHeadingsChild2Child2)["Text"], "2.1. Two_B (H2)")

            # Click on the 'Move chapter up' button in the Navigator tool box
            xNavigatorPanel = xWriterEdit.getChild("NavigatorPanel")
            xToolBar = xNavigatorPanel.getChild("HeadingsContentFunctionButtonsToolbar")
            xToolBar.executeAction("CLICK", mkPropertyValues({"POS": "2"}))

            # Expected chapter order is the original order
            xNavigatorPanelContentTreeHeadings = xNavigatorPanelContentTree.getChild('0')

            xHeadingsChild0 = xNavigatorPanelContentTreeHeadings.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild0)["Text"], "1. One (H1)")
            xHeadingsChild0Child0 = xHeadingsChild0.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild0Child0)["Text"], "1.1. One_A (H2)")
            xHeadingsChild0Child1 = xHeadingsChild0.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild0Child1)["Text"], "1.2. One_B (H2)")

            xHeadingsChild1 = xNavigatorPanelContentTreeHeadings.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild1)["Text"], "2. Two (H1)")
            xHeadingsChild1Child0 = xHeadingsChild1.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child0)["Text"], "A heading of level 3")
            xHeadingsChild1Child1 = xHeadingsChild1.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child1)["Text"], "2.1. Two_A (H2)")
            xHeadingsChild1Child2 = xHeadingsChild1.getChild('2')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child2)["Text"], "2.1. Two_B (H2)")

            xHeadingsChild2 = xNavigatorPanelContentTreeHeadings.getChild('2')
            self.assertEqual(get_state_as_dict(xHeadingsChild2)["Text"], "3. Three (H1)")
            xHeadingsChild2Child0 = xHeadingsChild2.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild2Child0)["Text"], "3.1. Three_A (H2)")
            xHeadingsChild2Child1 = xHeadingsChild2.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild2Child1)["Text"], "3.2. Three_B (H2)")

            #
            # test a sub chapter move chapter up then move down
            #

            # Double click on the "2.1. Two_A (H2)" entry to select and set focus
            xHeadingsChild1 = xNavigatorPanelContentTreeHeadings.getChild('1')
            xHeadingsChild1Child1 = xHeadingsChild1.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child1)["Text"], "2.1. Two_A (H2)")
            xHeadingsChild1Child1.executeAction("DOUBLECLICK", tuple())

            self.ui_test.wait_until_property_is_updated(xNavigatorPanelContentTree, "SelectEntryText", "2.1. Two_A (H2)")

            # Click on the 'Move chapter up' button in the Navigator tool box
            xNavigatorPanel = xWriterEdit.getChild("NavigatorPanel")
            xToolBar = xNavigatorPanel.getChild("HeadingsContentFunctionButtonsToolbar")
            xToolBar.executeAction("CLICK", mkPropertyValues({"POS": "2"}))

            # Expected chapter order:
                # 1. One H1
                #   1.1. one_A (H2)
                #   1.2. one_B (H2)
                # 2. Two (H1)
                #   2.1. Two_A (H2)
                #     A heading of level 3
                #   2.1. Two_B (H2)
                # 3. Three (H1)
                #   3.1. Three_A (H2)
                #   3.2. Three_B (H2)

            xNavigatorPanelContentTreeHeadings = xNavigatorPanelContentTree.getChild('0')

            xHeadingsChild0 = xNavigatorPanelContentTreeHeadings.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild0)["Text"], "1. One (H1)")
            xHeadingsChild0Child0 = xHeadingsChild0.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild0Child0)["Text"], "1.1. One_A (H2)")
            xHeadingsChild0Child1 = xHeadingsChild0.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild0Child1)["Text"], "1.2. One_B (H2)")

            xHeadingsChild1 = xNavigatorPanelContentTreeHeadings.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild1)["Text"], "2. Two (H1)")
            xHeadingsChild1Child0 = xHeadingsChild1.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child0)["Text"], "2.1. Two_A (H2)")
            xHeadingsChild1Child0Child0 = xHeadingsChild1Child0.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child0Child0)["Text"], "A heading of level 3")
            xHeadingsChild1Child1 = xHeadingsChild1.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child1)["Text"], "2.1. Two_B (H2)")

            xHeadingsChild2 = xNavigatorPanelContentTreeHeadings.getChild('2')
            self.assertEqual(get_state_as_dict(xHeadingsChild2)["Text"], "3. Three (H1)")
            xHeadingsChild2Child0 = xHeadingsChild2.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild2Child0)["Text"], "3.1. Three_A (H2)")
            xHeadingsChild2Child1 = xHeadingsChild2.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild2Child1)["Text"], "3.2. Three_B (H2)")

            self.ui_test.wait_until_property_is_updated(xNavigatorPanelContentTree, "SelectEntryText", "2.1. Two_A (H2)")

            # Click on the 'Move chapter down' button in the Navigator tool box
            xNavigatorPanel = xWriterEdit.getChild("NavigatorPanel")
            xToolBar = xNavigatorPanel.getChild("HeadingsContentFunctionButtonsToolbar")
            xToolBar.executeAction("CLICK", mkPropertyValues({"POS": "3"}))

            # Expected chapter order:
                # 1. One H1
                #   1.1. one_A (H2)
                #   1.2. one_B (H2)
                # 2. Two (H1)
                #   2.1. Two_B (H2)
                #   2.1. Two_A (H2)
                #     A heading of level 3
                # 3. Three (H1)
                #   3.1. Three_A (H2)
                #   3.2. Three_B (H2)

            xNavigatorPanelContentTreeHeadings = xNavigatorPanelContentTree.getChild('0')

            xHeadingsChild0 = xNavigatorPanelContentTreeHeadings.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild0)["Text"], "1. One (H1)")
            xHeadingsChild0Child0 = xHeadingsChild0.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild0Child0)["Text"], "1.1. One_A (H2)")
            xHeadingsChild0Child1 = xHeadingsChild0.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild0Child1)["Text"], "1.2. One_B (H2)")

            xHeadingsChild1 = xNavigatorPanelContentTreeHeadings.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild1)["Text"], "2. Two (H1)")
            xHeadingsChild1Child0 = xHeadingsChild1.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child0)["Text"], "2.1. Two_B (H2)")
            xHeadingsChild1Child1 = xHeadingsChild1.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child1)["Text"], "2.1. Two_A (H2)")
            xHeadingsChild1Child1Child0 = xHeadingsChild1Child1.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child1Child0)["Text"], "A heading of level 3")

            xHeadingsChild2 = xNavigatorPanelContentTreeHeadings.getChild('2')
            self.assertEqual(get_state_as_dict(xHeadingsChild2)["Text"], "3. Three (H1)")
            xHeadingsChild2Child0 = xHeadingsChild2.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild2Child0)["Text"], "3.1. Three_A (H2)")
            xHeadingsChild2Child1 = xHeadingsChild2.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild2Child1)["Text"], "3.2. Three_B (H2)")

            # Move "A heading of level 3" to its original position

            # Double click on the "A heading of level 3" entry to select and set focus
            xHeadingsChild1 = xNavigatorPanelContentTreeHeadings.getChild('1')
            xHeadingsChild1Child1Child0 = xHeadingsChild1Child1.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child1Child0)["Text"], "A heading of level 3")
            xHeadingsChild1Child1Child0.executeAction("DOUBLECLICK", tuple())

            self.ui_test.wait_until_property_is_updated(xNavigatorPanelContentTree, "SelectEntryText", "A heading of level 3")

            # Click on the 'Move chapter up' button in the Navigator tool box
            xNavigatorPanel = xWriterEdit.getChild("NavigatorPanel")
            xToolBar = xNavigatorPanel.getChild("HeadingsContentFunctionButtonsToolbar")
            xToolBar.executeAction("CLICK", mkPropertyValues({"POS": "2"}))

            # Click on the 'Move chapter up' button in the Navigator tool box
            xNavigatorPanel = xWriterEdit.getChild("NavigatorPanel")
            xToolBar = xNavigatorPanel.getChild("HeadingsContentFunctionButtonsToolbar")
            xToolBar.executeAction("CLICK", mkPropertyValues({"POS": "2"}))

            # Expected chapter order:
                # 1. One H1
                #   1.1. one_A (H2)
                #   1.2. one_B (H2)
                # 2. Two (H1)
                #     A heading of level 3
                #   2.1. Two_B (H2)
                #   2.1. Two_A (H2)
                # 3. Three (H1)
                #   3.1. Three_A (H2)
                #   3.2. Three_B (H2)

            xNavigatorPanelContentTreeHeadings = xNavigatorPanelContentTree.getChild('0')

            xHeadingsChild0 = xNavigatorPanelContentTreeHeadings.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild0)["Text"], "1. One (H1)")
            xHeadingsChild0Child0 = xHeadingsChild0.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild0Child0)["Text"], "1.1. One_A (H2)")
            xHeadingsChild0Child1 = xHeadingsChild0.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild0Child1)["Text"], "1.2. One_B (H2)")

            xHeadingsChild1 = xNavigatorPanelContentTreeHeadings.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild1)["Text"], "2. Two (H1)")
            xHeadingsChild1Child0 = xHeadingsChild1.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child0)["Text"], "A heading of level 3")
            xHeadingsChild1Child1 = xHeadingsChild1.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child1)["Text"], "2.1. Two_B (H2)")
            xHeadingsChild1Child2 = xHeadingsChild1.getChild('2')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child2)["Text"], "2.1. Two_A (H2)")

            xHeadingsChild2 = xNavigatorPanelContentTreeHeadings.getChild('2')
            self.assertEqual(get_state_as_dict(xHeadingsChild2)["Text"], "3. Three (H1)")
            xHeadingsChild2Child0 = xHeadingsChild2.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild2Child0)["Text"], "3.1. Three_A (H2)")
            xHeadingsChild2Child1 = xHeadingsChild2.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild2Child1)["Text"], "3.2. Three_B (H2)")

            # Move "2.1. Two_B (H2)" to its original position

            # Double click on the "2.1. Two_B (H2)" entry to select and set focus
            xHeadingsChild1 = xNavigatorPanelContentTreeHeadings.getChild('1')
            xHeadingsChild1Child1 = xHeadingsChild1.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child1)["Text"], "2.1. Two_B (H2)")
            xHeadingsChild1Child1.executeAction("DOUBLECLICK", tuple())

            self.ui_test.wait_until_property_is_updated(xNavigatorPanelContentTree, "SelectEntryText", "2.1. Two_B (H2)")

            # Click on the 'Move chapter down' button in the Navigator tool box
            xNavigatorPanel = xWriterEdit.getChild("NavigatorPanel")
            xToolBar = xNavigatorPanel.getChild("HeadingsContentFunctionButtonsToolbar")
            xToolBar.executeAction("CLICK", mkPropertyValues({"POS": "3"}))

            # Expected chapter order is the original order
            xNavigatorPanelContentTreeHeadings = xNavigatorPanelContentTree.getChild('0')

            xHeadingsChild0 = xNavigatorPanelContentTreeHeadings.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild0)["Text"], "1. One (H1)")
            xHeadingsChild0Child0 = xHeadingsChild0.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild0Child0)["Text"], "1.1. One_A (H2)")
            xHeadingsChild0Child1 = xHeadingsChild0.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild0Child1)["Text"], "1.2. One_B (H2)")

            xHeadingsChild1 = xNavigatorPanelContentTreeHeadings.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild1)["Text"], "2. Two (H1)")
            xHeadingsChild1Child0 = xHeadingsChild1.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child0)["Text"], "A heading of level 3")
            xHeadingsChild1Child1 = xHeadingsChild1.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child1)["Text"], "2.1. Two_A (H2)")
            xHeadingsChild1Child2 = xHeadingsChild1.getChild('2')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child2)["Text"], "2.1. Two_B (H2)")

            xHeadingsChild2 = xNavigatorPanelContentTreeHeadings.getChild('2')
            self.assertEqual(get_state_as_dict(xHeadingsChild2)["Text"], "3. Three (H1)")
            xHeadingsChild2Child0 = xHeadingsChild2.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild2Child0)["Text"], "3.1. Three_A (H2)")
            xHeadingsChild2Child1 = xHeadingsChild2.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild2Child1)["Text"], "3.2. Three_B (H2)")

            #
            # test moving a sub chapter out of and then back into its parent
            #

            # Double click on the "1.1. One_A (H2)" entry to select and set focus
            xHeadingsChild0 = xNavigatorPanelContentTreeHeadings.getChild('0')
            xHeadingsChild0Child0 = xHeadingsChild0.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild0Child0)["Text"], "1.1. One_A (H2)")
            xHeadingsChild0Child0.executeAction("DOUBLECLICK", tuple())

            self.ui_test.wait_until_property_is_updated(xNavigatorPanelContentTree, "SelectEntryText", "1.1. One_A (H2)")

            # Click on the 'Move chapter up' button in the Navigator tool box
            xNavigatorPanel = xWriterEdit.getChild("NavigatorPanel")
            xToolBar = xNavigatorPanel.getChild("HeadingsContentFunctionButtonsToolbar")
            xToolBar.executeAction("CLICK", mkPropertyValues({"POS": "2"}))

            # Expected chapter order:
                #   1.1. One_A (H2)
                # 1. One H1
                #   1.2. One_B (H2)
                # 2. Two (H1)
                #     A heading of level 3
                #   2.1. Two_A (H2)
                #   2.1. Two_B (H2)
                # 3. Three (H1)
                #   3.1. Three_A (H2)
                #   3.2. Three_B (H2)

            xNavigatorPanelContentTreeHeadings = xNavigatorPanelContentTree.getChild('0')

            xHeadingsChild0 = xNavigatorPanelContentTreeHeadings.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild0)["Text"], "1.1. One_A (H2)")

            xHeadingsChild1 = xNavigatorPanelContentTreeHeadings.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild1)["Text"], "1. One (H1)")
            xHeadingsChild1Child0 = xHeadingsChild1.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child0)["Text"], "1.2. One_B (H2)")

            xHeadingsChild2 = xNavigatorPanelContentTreeHeadings.getChild('2')
            self.assertEqual(get_state_as_dict(xHeadingsChild2)["Text"], "2. Two (H1)")
            xHeadingsChild2Child0 = xHeadingsChild2.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild2Child0)["Text"], "A heading of level 3")
            xHeadingsChild2Child1 = xHeadingsChild2.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild2Child1)["Text"], "2.1. Two_A (H2)")
            xHeadingsChild2Child2 = xHeadingsChild2.getChild('2')
            self.assertEqual(get_state_as_dict(xHeadingsChild2Child2)["Text"], "2.1. Two_B (H2)")

            xHeadingsChild3 = xNavigatorPanelContentTreeHeadings.getChild('3')
            self.assertEqual(get_state_as_dict(xHeadingsChild3)["Text"], "3. Three (H1)")
            xHeadingsChild3Child0 = xHeadingsChild3.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild3Child0)["Text"], "3.1. Three_A (H2)")
            xHeadingsChild3Child1 = xHeadingsChild3.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild3Child1)["Text"], "3.2. Three_B (H2)")

            # Click on the 'Move chapter down' button in the Navigator tool box
            xNavigatorPanel = xWriterEdit.getChild("NavigatorPanel")
            xToolBar = xNavigatorPanel.getChild("HeadingsContentFunctionButtonsToolbar")
            xToolBar.executeAction("CLICK", mkPropertyValues({"POS": "3"}))

            # Expected chapter order is the original order
            xNavigatorPanelContentTreeHeadings = xNavigatorPanelContentTree.getChild('0')

            xHeadingsChild0 = xNavigatorPanelContentTreeHeadings.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild0)["Text"], "1. One (H1)")
            xHeadingsChild0Child0 = xHeadingsChild0.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild0Child0)["Text"], "1.1. One_A (H2)")
            xHeadingsChild0Child1 = xHeadingsChild0.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild0Child1)["Text"], "1.2. One_B (H2)")

            xHeadingsChild1 = xNavigatorPanelContentTreeHeadings.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild1)["Text"], "2. Two (H1)")
            xHeadingsChild1Child0 = xHeadingsChild1.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child0)["Text"], "A heading of level 3")
            xHeadingsChild1Child1 = xHeadingsChild1.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child1)["Text"], "2.1. Two_A (H2)")
            xHeadingsChild1Child2 = xHeadingsChild1.getChild('2')
            self.assertEqual(get_state_as_dict(xHeadingsChild1Child2)["Text"], "2.1. Two_B (H2)")

            xHeadingsChild2 = xNavigatorPanelContentTreeHeadings.getChild('2')
            self.assertEqual(get_state_as_dict(xHeadingsChild2)["Text"], "3. Three (H1)")
            xHeadingsChild2Child0 = xHeadingsChild2.getChild('0')
            self.assertEqual(get_state_as_dict(xHeadingsChild2Child0)["Text"], "3.1. Three_A (H2)")
            xHeadingsChild2Child1 = xHeadingsChild2.getChild('1')
            self.assertEqual(get_state_as_dict(xHeadingsChild2Child1)["Text"], "3.2. Three_B (H2)")

            self.xUITest.executeCommand('.uno:Sidebar')

# vim: set shiftwidth=4 softtabstop=4 expandtab:

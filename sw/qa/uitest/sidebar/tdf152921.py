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
from uitest.uihelper.common import get_state_as_dict

class tdf152921(UITestCase):

    def test_tdf152921(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            xWriterEdit = self.xUITest.getTopFocusWindow().getChild("writer_edit")

            # open the sidebar if need, it should already be open on a clean run
            if 'TabBar' not in xWriterEdit.Children:
                self.xUITest.executeCommand(".uno:Sidebar")

            xTabBar = xWriterEdit.getChild('TabBar')

            # make sure only the tabbar is visible, no deck
            xDeckTitleToolBar = xWriterEdit.getChild('toolbar')
            xDeckTitleToolBar.executeAction("CLICK", mkPropertyValues({"POS": "1"}))
            # tabbar is visible, deck is not
            # without the patch this assert would fail, a tab would be highlighted
            self.assertEqual(0, len(get_state_as_dict(xTabBar)['HighlightedTabsIds']))

            # open a panel and assert that a tab is highlighted, also assert that only one tab is
            # highlighted, only a single tab should ever be highlighted
            xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "TextPropertyPanel"}))
            self.assertEqual(12, len(get_state_as_dict(xTabBar)['HighlightedTabsIds']))
            self.assertEqual(len(get_state_as_dict(xTabBar)['HighlightedTabsIds'].split(",")), 1)

            # click on the 'Close Sidebar Deck' button in the deck title tool bar
            xDeckTitleToolBar.executeAction("CLICK", mkPropertyValues({"POS": "1"}))
            # without the patch this assert would fail, a tab would be highlighted
            self.assertEqual(0, len(get_state_as_dict(xTabBar)['HighlightedTabsIds']))

            # open a deck by simulating a click on a tab in the tabbar
            xTabBar.executeAction("CLICK", mkPropertyValues({"POS": "4"}))
            self.assertEqual(14, len(get_state_as_dict(xTabBar)['HighlightedTabsIds']))
            self.assertEqual(len(get_state_as_dict(xTabBar)['HighlightedTabsIds'].split(",")), 1)

            # close it by clicking on the same tab
            xTabBar.executeAction("CLICK", mkPropertyValues({"POS": "4"}))
            # tabbar is visible, deck is not
            # without the patch this assert would fail, a tab would be highlighted
            self.assertEqual(0, len(get_state_as_dict(xTabBar)['HighlightedTabsIds']))

            # open a deck
            xTabBar.executeAction("CLICK", mkPropertyValues({"POS": "3"}))
            self.assertEqual(13, len(get_state_as_dict(xTabBar)['HighlightedTabsIds']))
            self.assertEqual(len(get_state_as_dict(xTabBar)['HighlightedTabsIds'].split(",")), 1)

            # open a different deck
            xTabBar.executeAction("CLICK", mkPropertyValues({"POS": "1"}))
            self.assertEqual(13, len(get_state_as_dict(xTabBar)['HighlightedTabsIds']))
            self.assertEqual(len(get_state_as_dict(xTabBar)['HighlightedTabsIds'].split(",")), 1)

            # click on the 'Close Sidebar Deck' button
            xDeckTitleToolBar.executeAction("CLICK", mkPropertyValues({"POS": "1"}))
            # without the patch this assert would fail, a tab would be highlighted
            self.assertEqual(0, len(get_state_as_dict(xTabBar)['HighlightedTabsIds']))

# vim: set shiftwidth=4 softtabstop=4 expandtab:

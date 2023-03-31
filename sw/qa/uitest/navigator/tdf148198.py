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

class tdf148198(UITestCase):

    def get_item(self, xTree, name):
        for i in xTree.getChildren():
            xItem = xTree.getChild(i)
            if name == get_state_as_dict(xItem)['Text']:
                return xItem

    def test_tdf148198(self):
        sHyperlink0 = 'a hyperlink with an anchor, different character styles, a footnote1, a field This is a bookmark, and a rsid'
        sHyperlink1 = 'another hyperlink'

        with self.ui_test.load_file(get_url_for_data_file('hyperlink_multiple_spans.fodt')):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild('writer_edit')

            self.xUITest.executeCommand('.uno:Sidebar')
            xWriterEdit.executeAction('SIDEBAR', mkPropertyValues({'PANEL': 'SwNavigatorPanel'}))

            # wait until the navigator panel is available
            xNavigatorPanel = self.ui_test.wait_until_child_is_available('NavigatorPanel')

            xContentTree = xNavigatorPanel.getChild('contenttree')
            xHyperlinks = self.get_item(xContentTree, 'Hyperlinks')
            self.assertEqual('Hyperlinks', get_state_as_dict(xHyperlinks)['Text'])

            xHyperlinks.executeAction('EXPAND', tuple())

            self.assertEqual('2', get_state_as_dict(xHyperlinks)['Children'])
            self.assertEqual(sHyperlink0, get_state_as_dict(xHyperlinks.getChild('0'))['Text'])
            self.assertEqual(sHyperlink1, get_state_as_dict(xHyperlinks.getChild('1'))['Text'])

            # save and reload: the first hyperlink must be kept in one piece
            self.xUITest.executeCommand('.uno:Save')
            self.xUITest.executeCommand('.uno:Reload')

            # wait until the navigator panel is available
            xNavigatorPanel = self.ui_test.wait_until_child_is_available('NavigatorPanel')

            xContentTree = xNavigatorPanel.getChild('contenttree')
            xHyperlinks = self.get_item(xContentTree, 'Hyperlinks')
            self.assertEqual('Hyperlinks', get_state_as_dict(xHyperlinks)['Text'])

            xHyperlinks.executeAction('EXPAND', tuple())

            # without the fix in place, this would fail:
            #   AssertionError: '2' != '11'
            self.assertEqual('2', get_state_as_dict(xHyperlinks)['Children'])
            self.assertEqual(sHyperlink0, get_state_as_dict(xHyperlinks.getChild('0'))['Text'])
            self.assertEqual(sHyperlink1, get_state_as_dict(xHyperlinks.getChild('1'))['Text'])

            self.xUITest.executeCommand('.uno:Sidebar')

# vim: set shiftwidth=4 softtabstop=4 expandtab:

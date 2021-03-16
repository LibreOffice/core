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
from uitest.uihelper.common import select_pos

class tdf81457(UITestCase):

#tdf 81457
   def test_open_documentProperties_tdf81457(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf81457.odt"))
        self.ui_test.execute_dialog_through_command(".uno:SetDocumentProperties")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "2")     #tab Custom properties

        aExpectedNames = ['BookMarkCount', 'BookMarkInfo1', 'BookMarkInfo10', 'BookMarkInfo11',
                'BookMarkInfo12', 'BookMarkInfo13']
        aExpectedValues = ['78', '00FF0000FF010', '00FF0000FF1E0', '00FF0000FF1E0',
                '00FF0000FF210', '00FF0000FF230']

        for i in range(6):
            xNameBox = xDialog.getChild("namebox" + str(i + 1))
            xTypeBox = xDialog.getChild("typebox" + str(i + 1))
            xValueEdit = xDialog.getChild("valueedit" + str(i + 1))
            self.assertEqual(aExpectedNames[i], get_state_as_dict(xNameBox)['Text'])
            self.assertEqual('Text', get_state_as_dict(xTypeBox)['DisplayText'])
            self.assertEqual(aExpectedValues[i], get_state_as_dict(xValueEdit)['Text'][:13])


        xOkBtn = xDialog.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

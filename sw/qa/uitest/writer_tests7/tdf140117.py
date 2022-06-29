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
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import type_text
from uitest.uihelper.common import get_url_for_data_file

class tdf140117(UITestCase):
    def test_tdf140117(self):
        # load the sample file
        with self.ui_test.load_file(get_url_for_data_file("tdf140117.fodt")) as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            for i in range(3):
                    xWriterEdit.executeAction("GOTO", mkPropertyValues({"PAGE": "2"}))

                    self.xUITest.executeCommand(".uno:JumpToHeader")

                    xPageSytle = document.getStyleFamilies().getByIndex(2)
                    xHeaderText = xPageSytle.getByIndex(0).HeaderText.String
                    xHeaderLeftText = xPageSytle.getByIndex(0).HeaderTextLeft.String
                    xHeaderRightText = xPageSytle.getByIndex(0).HeaderTextRight.String

                    # Option "same content on left and right pages" is false,
                    # insert text "XXXX" before actual header text "left" on page 2
                    if i == 0:
                            type_text(xWriterEdit, "XXXX")

                    # Option "same content on left and right pages" is true,
                    # header of page 2 contains the same text as page 1
                    elif i == 1:
                            self.assertEqual("right", xHeaderText)
                            self.assertEqual("right", xHeaderRightText)

                    # Option "same content on left and right pages" is false again.
                    # This was "right" instead of keeping the header content disabled
                    # temporarily for the second interaction of the loop.
                    elif i == 2:
                            self.assertEqual("XXXXleft", xHeaderLeftText)

                    with self.ui_test.execute_dialog_through_command(".uno:PageDialog") as PageDialog:

                        xTabs = PageDialog.getChild("tabcontrol")
                        select_pos(xTabs, "4")

                        # Change option "same content on left and right pages" for the next iteration
                        Button = xTabs.getChild('checkSameLR')
                        Button.executeAction("CLICK", tuple())

# vim: set shiftwidth=4 softtabstop=4 expandtab:

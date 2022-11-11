# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.framework import UITestCase


class Tdf148810(UITestCase):

    def test_Tdf148810(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf148810_PARA_OUTLLEVEL.pptx")):
            document = self.ui_test.get_component()

            xDoc = self.xUITest.getTopFocusWindow()
            xEditWin = xDoc.getChild("impress_win")

            xEditWin.executeAction("SELECT", mkPropertyValues({"OBJECT":"TextShape 2"}))

            # type something to get into text editing mode (instead of shape selection).
            xEditWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
            #time.sleep(2)

            # get to the front of the text (behind the bullet point)
            xEditWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "HOME"}))
            # remove the numbering bullet point
            xEditWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))

            #xShape = xEditWin.getChild("TextShape 2")
            #print(xShape)
            #print(xShape.getChildren())
            #print(get_state_as_dict(xShape))
            #print(dir(xShape)

            xText = document.DrawPages[0].getByIndex(1).createEnumeration().nextElement()
            #print(xText)
            # this is the first numbering level (as opposed to either -1 or None for no numbering)
            self.assertEqual(0, xText.NumberingLevel)
            #time.sleep(2)

            self.xUITest.executeCommand(".uno:Undo")
            #time.sleep(2)

            xText = document.DrawPages[0].getByIndex(1).createEnumeration().nextElement()
            # This was failing with "None"
            self.assertEqual(0, xText.NumberingLevel)

# vim: set shiftwidth=4 softtabstop=4 expandtab:

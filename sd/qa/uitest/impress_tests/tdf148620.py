# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.framework import UITestCase

class Tdf148620(UITestCase):

    def test_Tdf148620(self):
        with self.ui_test.create_doc_in_start_center("impress") as document:

            xTemplateDlg = self.xUITest.getTopFocusWindow()
            xCancelBtn = xTemplateDlg.getChild("close")
            self.ui_test.close_dialog_through_button(xCancelBtn)

            xDoc = self.xUITest.getTopFocusWindow()
            xEditWin = xDoc.getChild("impress_win")

            xEditWin.executeAction("SELECT", mkPropertyValues({"OBJECT":"Unnamed Drawinglayer object 1"}))
            self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())

            xEditWin.executeAction("TYPE", mkPropertyValues({"TEXT":"one"}))
            xEditWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
            xEditWin.executeAction("TYPE", mkPropertyValues({"TEXT":"two"}))
            xEditWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
            xEditWin.executeAction("TYPE", mkPropertyValues({"TEXT":"three"}))
            xEditWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
            xEditWin.executeAction("TYPE", mkPropertyValues({"TEXT":"four"}))
            xEditWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
            xEditWin.executeAction("TYPE", mkPropertyValues({"TEXT":"five"}))
            xEditWin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
            xEditWin.executeAction("TYPE", mkPropertyValues({"TEXT":"six"}))

            self.assertEqual("One\nTwo\nThree\nFour\nFive\nsix", document.DrawPages[0].getByIndex(1).String)

            xArgs = mkPropertyValues({"KeyModifier": 0})
            self.xUITest.executeCommandWithParameters(".uno:OutlineUp", xArgs)
            self.assertEqual("One\nTwo\nThree\nFour\nsix\nFive", document.DrawPages[0].getByIndex(1).String)

            self.xUITest.executeCommandWithParameters(".uno:OutlineUp", xArgs)
            self.assertEqual("One\nTwo\nThree\nsix\nFour\nFive", document.DrawPages[0].getByIndex(1).String)

            self.xUITest.executeCommandWithParameters(".uno:OutlineUp", xArgs)
            self.assertEqual("One\nTwo\nsix\nThree\nFour\nFive", document.DrawPages[0].getByIndex(1).String)

            self.xUITest.executeCommandWithParameters(".uno:OutlineUp", xArgs)
            self.assertEqual("One\nsix\nTwo\nThree\nFour\nFive", document.DrawPages[0].getByIndex(1).String)

            self.xUITest.executeCommandWithParameters(".uno:OutlineUp", xArgs)
            self.assertEqual("six\nOne\nTwo\nThree\nFour\nFive", document.DrawPages[0].getByIndex(1).String)

            self.xUITest.executeCommandWithParameters(".uno:OutlineDown", xArgs)

            # Without the fix in place, this test would have failed with
            # AssertionError: 'One\nsix\nTwo\nThree\nFour\nFive' != 'One\nTwo\nsix\nThree\nFour\nFive'
            self.assertEqual("One\nsix\nTwo\nThree\nFour\nFive", document.DrawPages[0].getByIndex(1).String)

            self.xUITest.executeCommandWithParameters(".uno:OutlineDown", xArgs)
            self.assertEqual("One\nTwo\nsix\nThree\nFour\nFive", document.DrawPages[0].getByIndex(1).String)

            self.xUITest.executeCommandWithParameters(".uno:OutlineDown", xArgs)
            self.assertEqual("One\nTwo\nThree\nsix\nFour\nFive", document.DrawPages[0].getByIndex(1).String)

            self.xUITest.executeCommandWithParameters(".uno:OutlineDown", xArgs)
            self.assertEqual("One\nTwo\nThree\nFour\nsix\nFive", document.DrawPages[0].getByIndex(1).String)

            self.xUITest.executeCommandWithParameters(".uno:OutlineDown", xArgs)
            self.assertEqual("One\nTwo\nThree\nFour\nFive\nsix", document.DrawPages[0].getByIndex(1).String)

# vim: set shiftwidth=4 softtabstop=4 expandtab:

# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict

class Tdf137637(UITestCase):

   def test_tdf137637(self):
        with self.ui_test.create_doc_in_start_center("impress") as document:

            xTemplateDlg = self.xUITest.getTopFocusWindow()
            xCancelBtn = xTemplateDlg.getChild("close")
            self.ui_test.close_dialog_through_button(xCancelBtn)

            # Insert shape with Ctrl key
            xArgs = mkPropertyValues({"KeyModifier": 8192})
            self.xUITest.executeCommandWithParameters(".uno:BasicShapes.rectangle", xArgs)

            self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())

            xImpressDoc = self.xUITest.getTopFocusWindow()

            xEditWin = xImpressDoc.getChild("impress_win")

            xEditWin.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "SdCustomAnimationPanel"}))

            # Without the fix in place, this test would have failed with
            # AttributeError: 'NoneType' object has no attribute 'getImplementationName'
            self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())

            xAnimationList = xImpressDoc.getChild("custom_animation_list")
            self.assertEqual('0', get_state_as_dict(xAnimationList)['Children'])

            xAddBtn = xImpressDoc.getChild("add_effect")
            xAddBtn.executeAction("CLICK", tuple())

            self.assertEqual('1', get_state_as_dict(xAnimationList)['Children'])

            self.xUITest.executeCommand(".uno:Undo")

            # tdf#135033: Without the fix in place, this test would have failed with
            # AssertionError: '0' != '1'
            self.assertEqual('0', get_state_as_dict(xAnimationList)['Children'])


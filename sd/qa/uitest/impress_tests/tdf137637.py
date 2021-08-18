# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues

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


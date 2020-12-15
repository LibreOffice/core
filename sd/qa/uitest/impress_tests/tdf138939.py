# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf138939(UITestCase):

   def test_run(self):
        self.ui_test.create_doc_in_start_center("impress")
        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        xDoc = self.xUITest.getTopFocusWindow()
        xEdit = xDoc.getChild("impress_win")

        # Insert shape
        self.xUITest.executeCommandWithParameters(".uno:BasicShapes.rectangle", mkPropertyValues({"KeyModifier": 8192}))

        document = self.ui_test.get_component()
        self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())

        xEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "SdCustomAnimationPanel"}))

        # Without the fix in place, this test would have failed with
        # AttributeError: 'NoneType' object has no attribute 'getImplementationName'
        self.assertEqual("com.sun.star.drawing.SvxShapeCollection", document.CurrentSelection.getImplementationName())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

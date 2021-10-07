# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf129346(UITestCase):

   def test_run(self):
        with self.ui_test.create_doc_in_start_center("impress") as document:
            xTemplateDlg = self.xUITest.getTopFocusWindow()
            xCancelBtn = xTemplateDlg.getChild("close")
            self.ui_test.close_dialog_through_button(xCancelBtn)

            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')

            self.assertEqual(document.CurrentController.getCurrentPage().Number, 1)
            self.xUITest.executeCommand(".uno:DuplicatePage")
            xToolkit.processEventsToIdle()
            self.assertEqual(document.CurrentController.getCurrentPage().Number, 2)

            xDoc = self.xUITest.getTopFocusWindow()
            xEdit = xDoc.getChild("impress_win")
            # Type "test" into the text box
            xEdit.executeAction("TYPE", mkPropertyValues({"TEXT":"test"}))
            # Go to Page 1, which also forces to end edit box
            xEdit.executeAction("GOTO", mkPropertyValues({"PAGE": "1"}))
            xToolkit.processEventsToIdle()

            # We should be at Page 1
            self.assertEqual(document.CurrentController.getCurrentPage().Number, 1)

            # Undo sends us to Page 2 and undo-es the text edit
            self.xUITest.executeCommand(".uno:Undo")
            xToolkit.processEventsToIdle()
            self.assertEqual(document.CurrentController.getCurrentPage().Number, 2)

            # Undo sends us to page 1 and undo-es command ".uno:DuplicatePage"
            self.xUITest.executeCommand(".uno:Undo")
            xToolkit.processEventsToIdle()
            self.assertEqual(document.CurrentController.getCurrentPage().Number, 1)

            # Redo ".uno:DuplicatePage" - we go to Page 2
            self.xUITest.executeCommand(".uno:Redo")
            xToolkit.processEventsToIdle()
            self.assertEqual(document.CurrentController.getCurrentPage().Number, 2)

            # Redo text edit
            self.xUITest.executeCommand(".uno:Redo")

            xDoc = self.xUITest.getTopFocusWindow()
            xEdit = xDoc.getChild("impress_win")
            xEdit.executeAction("TYPE", mkPropertyValues({"TEXT":"test"}))

            xToolkit.processEventsToIdle()
            #Without the accompanying fix in place, it would fail with AssertionError: 2 != 1
            self.assertEqual(document.CurrentController.getCurrentPage().Number, 2)

# vim: set shiftwidth=4 softtabstop=4 expandtab:

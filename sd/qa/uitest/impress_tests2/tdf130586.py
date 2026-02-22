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

class tdf130586(UITestCase):

    def test_run(self):
        with self.ui_test.create_doc_in_start_center("impress") as document:
            xTemplateDlg = self.xUITest.getTopFocusWindow()
            xCancelBtn = xTemplateDlg.getChild("close")
            self.ui_test.close_dialog_through_button(xCancelBtn)

            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')

            self.assertEqual(document.CurrentController.getCurrentPage().Number, 1)

            # Insert a new slide
            self.xUITest.executeCommand(".uno:InsertPage")
            xToolkit.processEventsToIdle()
            self.assertEqual(document.CurrentController.getCurrentPage().Number, 2)

            # Change layout on slide 2
            self.xUITest.executeCommand(".uno:AssignLayout?WhatLayout:long=1")
            xToolkit.processEventsToIdle()
            self.assertEqual(document.CurrentController.getCurrentPage().Number, 2)

            # Undo x2 (undo layout change, then undo insert slide)
            self.xUITest.executeCommand(".uno:Undo")
            xToolkit.processEventsToIdle()
            self.assertEqual(document.CurrentController.getCurrentPage().Number, 2)

            self.xUITest.executeCommand(".uno:Undo")
            xToolkit.processEventsToIdle()
            self.assertEqual(document.CurrentController.getCurrentPage().Number, 1)

            # Redo x2 (redo insert slide, then redo layout change)
            self.xUITest.executeCommand(".uno:Redo")
            xToolkit.processEventsToIdle()
            self.assertEqual(document.CurrentController.getCurrentPage().Number, 2)

            self.xUITest.executeCommand(".uno:Redo")
            xToolkit.processEventsToIdle()
            # Without the fix, this would fail with AssertionError: 2 != 1
            self.assertEqual(document.CurrentController.getCurrentPage().Number, 2)

# vim: set shiftwidth=4 softtabstop=4 expandtab:

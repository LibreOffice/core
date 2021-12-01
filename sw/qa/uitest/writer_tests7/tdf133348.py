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

class tdf133348(UITestCase):
    def test_tdf133348(self):

        with self.ui_test.create_doc_in_start_center("writer") as document:

            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")


            selection = self.xUITest.executeCommand(".uno:SelectAll")
            xArgs = mkPropertyValues({"Text": "C1"})
            self.xUITest.executeCommandWithParameters(".uno:InsertAnnotation", xArgs)

            with self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog") as xDialogOpt:
                xPages = xDialogOpt.getChild("pages")
                xEntry = xPages.getChild('0')
                xEntry.executeAction("EXPAND", tuple())
                xGeneralEntry = xEntry.getChild('0')
                xGeneralEntry.executeAction("SELECT", tuple())
                xFirstName = xDialogOpt.getChild("firstname")
                props = {"TEXT": "Known Author"}
                actionProps = mkPropertyValues(props)
                xFirstName.executeAction("TYPE", actionProps)

            xWriterDoc = self.xUITest.getTopFocusWindow()
            xArgs = mkPropertyValues({"Text": "C2"})
            self.xUITest.executeCommandWithParameters(".uno:ReplyComment", xArgs)

            # Wait for async events to be processed
            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.processEventsToIdle()

            xEnum = document.TextFields.createEnumeration()
            self.assertEqual(xEnum.nextElement().Author.strip(), 'Unknown Author')
            self.assertEqual(xEnum.nextElement().Author.strip(), 'Known Author')

            self.xUITest.executeCommand(".uno:Undo")
            self.xUITest.executeCommand(".uno:Undo")

            # Without the fix in place, it would have crashed here
            self.xUITest.executeCommand(".uno:Undo")
            self.xUITest.executeCommand(".uno:Undo")

            # all comments have been deleted
            self.assertFalse(document.TextFields.createEnumeration().hasMoreElements())

# vim: set shiftwidth=4 softtabstop=4 expandtab:

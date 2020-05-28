# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf133348(UITestCase):
    def test_tdf133348(self):

        self.ui_test.create_doc_in_start_center("writer")

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        document = self.ui_test.get_component()

        selection = self.xUITest.executeCommand(".uno:SelectAll")
        xArgs = mkPropertyValues({"Text": "C1"})
        self.xUITest.executeCommandWithParameters(".uno:InsertAnnotation", xArgs)

        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")
        xDialogOpt = self.xUITest.getTopFocusWindow()
        xPages = xDialogOpt.getChild("pages")
        xEntry = xPages.getChild('0')
        xEntry.executeAction("EXPAND", tuple())
        xGeneralEntry = xEntry.getChild('0')
        xGeneralEntry.executeAction("SELECT", tuple())
        xFirstName = xDialogOpt.getChild("firstname")
        props = {"TEXT": "Known Author"}
        actionProps = mkPropertyValues(props)
        xFirstName.executeAction("TYPE", actionProps)
        xOKBtn = xDialogOpt.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xArgs = mkPropertyValues({"Text": "C2"})
        self.xUITest.executeCommandWithParameters(".uno:ReplyComment", xArgs)

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

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:


# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues

class insertFootEndnote(UITestCase):

   def test_insert_foot_endnote(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()

#Automatic - Footnote
        with self.ui_test.execute_dialog_through_command_guarded(".uno:InsertFootnoteDialog"):
            pass

        self.assertEqual(document.Footnotes.getCount(), 1)
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(document.Footnotes.getCount(), 0)
#Automatic - Endnote
        with self.ui_test.execute_dialog_through_command_guarded(".uno:InsertFootnoteDialog") as xDialog:
            xEndnote = xDialog.getChild("endnote")
            xEndnote.executeAction("CLICK", tuple())

        self.assertEqual(document.Endnotes.getCount(), 1)
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(document.Endnotes.getCount(), 0)
#Character - Footnote
        with self.ui_test.execute_dialog_through_command_guarded(".uno:InsertFootnoteDialog") as xDialog:
            xChar = xDialog.getChild("character")
            xChar.executeAction("CLICK", tuple())
            xCharentry = xDialog.getChild("characterentry")
            xCharentry.executeAction("TYPE", mkPropertyValues({"TEXT":"A"}))

        self.assertEqual(document.Footnotes.getCount(), 1)
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(document.Footnotes.getCount(), 0)

#Character - Endnote
        with self.ui_test.execute_dialog_through_command_guarded(".uno:InsertFootnoteDialog") as xDialog:
            xChar = xDialog.getChild("character")
            xChar.executeAction("CLICK", tuple())
            xCharentry = xDialog.getChild("characterentry")
            xCharentry.executeAction("TYPE", mkPropertyValues({"TEXT":"A"}))

            xEndnote = xDialog.getChild("endnote")
            xEndnote.executeAction("CLICK", tuple())

        self.assertEqual(document.Endnotes.getCount(), 1)
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(document.Endnotes.getCount(), 0)

#Cancel button
        with self.ui_test.execute_dialog_through_command_guarded(".uno:InsertFootnoteDialog", close_button="cancel"):
            pass

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:

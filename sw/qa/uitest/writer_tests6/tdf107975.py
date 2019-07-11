# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text
from uitest.debug import sleep
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

#Bug 107975 - CRASH: Crash after undo operation (BigPtrArray::Index2Block(unsigned long)

class tdf107975(UITestCase):
   def test_tdf107975_crash_after_undo(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf107975.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.assertEqual(writer_doc.getGraphicObjects().getCount(), 1)

        #Press CTRL+A and + CTRL+C
        self.xUITest.executeCommand(".uno:SelectAll")
        self.xUITest.executeCommand(".uno:Copy")
        #Position the mouse cursor (caret) after "ABC" below the blue image
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RIGHT"}))
        #Paste CTRL+V
        self.xUITest.executeCommand(".uno:Paste")
        self.assertEqual(writer_doc.getGraphicObjects().getCount(), 2)
        #Undo paste CTRL+Z -> Crash
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(document.Text.String[0:3], "ABC")
        self.assertEqual(writer_doc.getGraphicObjects().getCount(), 1)
        self.xUITest.executeCommand(".uno:Redo")
        self.assertEqual(writer_doc.getGraphicObjects().getCount(), 2)
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(writer_doc.getGraphicObjects().getCount(), 1)
        self.xUITest.executeCommand(".uno:Redo")
        self.assertEqual(writer_doc.getGraphicObjects().getCount(), 2)
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(writer_doc.getGraphicObjects().getCount(), 1)

        # try again with anchor at start of doc which is another special case
        xShape = writer_doc.getGraphicObjects().getByIndex(0)
        xStart = writer_doc.getText().getStart()
        xShape.attach(xStart)

        #Press CTRL+A and + CTRL+C
        self.xUITest.executeCommand(".uno:SelectAll")
        self.xUITest.executeCommand(".uno:Copy")
        #Position the mouse cursor (caret) after "ABC" below the blue image
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RIGHT"}))
        #Paste CTRL+V
        self.xUITest.executeCommand(".uno:Paste")
        self.assertEqual(writer_doc.getGraphicObjects().getCount(), 2)
        #Undo paste CTRL+Z -> Crash
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(writer_doc.getGraphicObjects().getCount(), 1)
        self.assertEqual(document.Text.String[0:3], "ABC")
        self.xUITest.executeCommand(".uno:Redo")
        self.assertEqual(writer_doc.getGraphicObjects().getCount(), 2)
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(writer_doc.getGraphicObjects().getCount(), 1)
        self.xUITest.executeCommand(".uno:Redo")
        self.assertEqual(writer_doc.getGraphicObjects().getCount(), 2)
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(writer_doc.getGraphicObjects().getCount(), 1)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

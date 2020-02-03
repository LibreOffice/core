# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.path import get_srcdir_url

def get_url_for_data_file(file_name):
    return get_srcdir_url() + "/sw/qa/uitest/writer_tests/data/" + file_name

class tdf128782(UITestCase):

   def test_tdf128782_move_textbox_undo(self):
        self.ui_test.load_file(get_url_for_data_file("tdf128782.odt"))
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        document = self.ui_test.get_component()

        oldX1 = document.getDrawPage()[0].Position.X
        oldY1 = document.getDrawPage()[0].Position.Y
        oldX2 = document.getDrawPage()[1].Position.X
        oldY2 = document.getDrawPage()[1].Position.Y

        #select shape 2 and move it down
        document.getCurrentController().select(document.getDrawPage()[1])
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))

        #Only the Y value of shape 2 changes
        self.assertEqual(oldX1, document.getDrawPage()[0].Position.X)
        self.assertEqual(oldY1, document.getDrawPage()[0].Position.Y)
        self.assertEqual(oldX2, document.getDrawPage()[1].Position.X)
        self.assertNotEqual(oldY2, document.getDrawPage()[1].Position.Y)

        self.xUITest.executeCommand(".uno:Undo")

        #Both shapes should have the same position as before
        self.assertEqual(oldX1, document.getDrawPage()[0].Position.X)
        self.assertEqual(oldY1, document.getDrawPage()[0].Position.Y)
        self.assertEqual(oldX2, document.getDrawPage()[1].Position.X)
        self.assertEqual(oldY2, document.getDrawPage()[1].Position.Y)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

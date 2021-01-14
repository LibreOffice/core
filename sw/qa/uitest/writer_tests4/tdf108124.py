# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class tdf108124(UITestCase):

   def test_tdf108124(self):
    writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf108124.odt"))
    xWriterDoc = self.xUITest.getTopFocusWindow()
    xWriterEdit = xWriterDoc.getChild("writer_edit")
    document = self.ui_test.get_component()

    self.assertEqual(document.GraphicObjects.getCount(), 2)  #nr. of images

    xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+a"})) # select all
    xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+c"})) # copy
    xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+v"})) # paste
    self.assertEqual(document.GraphicObjects.getCount(), 2)
    xObj1Old = document.GraphicObjects[0]
    xObj2Old = document.GraphicObjects[1]
    self.xUITest.executeCommand(".uno:Undo")  #Undo
    self.assertEqual(document.GraphicObjects.getCount(), 2)
    xObj1New = document.GraphicObjects[0]
    xObj2New = document.GraphicObjects[1]
    # there should be 2 different objects now but they have the same names,
    # so rely on the object identity for testing...
    self.assertNotEqual(xObj1Old, xObj1New)
    self.assertNotEqual(xObj1Old, xObj2New)
    self.assertNotEqual(xObj2Old, xObj1New)
    self.assertNotEqual(xObj2Old, xObj2New)
    self.xUITest.executeCommand(".uno:Redo")  #Redo
    self.assertEqual(document.GraphicObjects.getCount(), 2)

    self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

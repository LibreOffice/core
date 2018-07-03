# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
import org.libreoffice.unotest
import pathlib
import time
from uitest.debug import sleep

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

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
    self.assertEqual(document.GraphicObjects.getCount(), 4)
    self.xUITest.executeCommand(".uno:Undo")  #Undo
    self.assertEqual(document.GraphicObjects.getCount(), 2)
    self.xUITest.executeCommand(".uno:Redo")  #Redo
    self.assertEqual(document.GraphicObjects.getCount(), 4)

    self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:

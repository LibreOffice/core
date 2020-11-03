# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

class tdf137964(UITestCase):

    def test_tdf137964(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf137964.odt"))
        document = self.ui_test.get_component()

        nPosXBefore = document.DrawPages[0].getByIndex(0).Position.X
        nPosYBefore = document.DrawPages[0].getByIndex(0).Position.Y

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        xWriterEdit.executeAction("SELECT", mkPropertyValues({"OBJECT": "Shape2"}))
        xWriterEdit.executeAction("ACTIVATE", tuple())

        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "SHIFT+UP"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "SHIFT+LEFT"}))

        nPosXAfter = document.DrawPages[0].getByIndex(0).Position.X
        nPosYAfter = document.DrawPages[0].getByIndex(0).Position.Y

        self.assertTrue(nPosXAfter < nPosXBefore)
        self.assertTrue(nPosYAfter < nPosYBefore)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:


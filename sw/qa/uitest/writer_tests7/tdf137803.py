# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
import importlib
import time
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

class tdf137803(UITestCase):
    def test_tdf137803(self):
        # load the sample file
        self.ui_test.load_file(get_url_for_data_file("tdf137803.odt"))
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        document = self.ui_test.get_component()

        # select the shape
        self.xUITest.executeCommand(".uno:JumpToNextFrame")
        self.ui_test.wait_until_child_is_available(xWriterEdit, 'metricfield')

        # open textattrs dialog
        self.ui_test.execute_dialog_through_command(".uno:TextAttributes")
        TextDialog = self.xUITest.getTopFocusWindow();

        # check autosize on
        TSB_AUTOGROW_SIZE = TextDialog.getChild('TSB_AUTOGROW_SIZE')
        TSB_AUTOGROW_SIZE.executeAction("CLICK",tuple())
        ok = TextDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(ok)

        # get the shape
        drawPage = document.getDrawPages().getByIndex(0)
        shape = drawPage.getByIndex(0)

        # and the textbox
        frame = shape.getText()

        # get the positions
        shapeYPos = shape.getPropertyValue("VertOrientPosition")
        frameYPos = frame.getPropertyValue("VertOrientPosition")
        shpsize = shape.getSize().Height

        xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
        xToolkit.processEventsToIdle()

        # without the fix, at this point the textbox fall apart so this wont be passed
        self.assertLess(frameYPos, shapeYPos + shpsize)

        # close the doc
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

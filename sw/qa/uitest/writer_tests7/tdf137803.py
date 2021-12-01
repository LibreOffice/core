# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file


class tdf137803(UITestCase):
    def test_tdf137803(self):
        # load the sample file
        with self.ui_test.load_file(get_url_for_data_file("tdf137803.odt")) as document:

            # select the shape
            self.xUITest.executeCommand(".uno:JumpToNextFrame")
            self.ui_test.wait_until_child_is_available('metricfield')

            # open textattrs dialog
            with self.ui_test.execute_dialog_through_command(".uno:TextAttributes") as TextDialog:

                # check autosize on
                TSB_AUTOGROW_SIZE = TextDialog.getChild('TSB_AUTOGROW_SIZE')
                TSB_AUTOGROW_SIZE.executeAction("CLICK",tuple())

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

            # without the fix, at this point the textbox falls apart so this won't be passed
            self.assertLess(frameYPos, shapeYPos + shpsize)

            # close the doc

# vim: set shiftwidth=4 softtabstop=4 expandtab:

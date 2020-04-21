# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import time
from uitest.framework import UITestCase
from com.sun.star.frame import InfobarType
from com.sun.star.container import NoSuchElementException
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
   return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

#Bug 117039 - Print Preview crashes on signed document

class tdf117039(UITestCase):
   def test_tdf117039_preview_signed_document(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf117039.odt"))
        controller = self.ui_test.get_component().getCurrentController()

        #wait until the infobar is available
        timeout = time.time() + 1
        while time.time() < timeout:
            try:
                controller.updateInfobar("signature", "", "", InfobarType.WARNING)
                break
            except NoSuchElementException:
                time.sleep(0.1)

        #if the infobar is not available it would through a NoSuchElementException exception
        controller.removeInfobar("signature")

        self.xUITest.executeCommand(".uno:PrintPreview")  #open print preview
        self.xUITest.executeCommand(".uno:ClosePreview")  # close print preview

        self.xUITest.getTopFocusWindow() #Get focus after closing preview

        controller = self.ui_test.get_component().getCurrentController()
        #wait until the infobar is available
        timeout = time.time() + 1
        while time.time() < timeout:
            try:
                controller.updateInfobar("signature", "", "", InfobarType.WARNING)
                break
            except NoSuchElementException:
                time.sleep(0.1)

        #if the infobar is not available it would through a NoSuchElementException exception
        controller.removeInfobar("signature")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

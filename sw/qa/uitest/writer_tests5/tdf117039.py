# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

#Bug 117039 - Print Preview crashes on signed document

class tdf117039(UITestCase):
   def test_tdf117039_preview_signed_document(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf117039.odt"))
        document = self.ui_test.get_component()
        self.xUITest.executeCommand(".uno:PrintPreview")  #open print preview
        self.xUITest.executeCommand(".uno:ClosePreview")  # close print preview

        self.xUITest.getTopFocusWindow() #Get focus after closing preview

        #verify
        self.assertEqual(document.Text.String[0:22], "Test digital signature")
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

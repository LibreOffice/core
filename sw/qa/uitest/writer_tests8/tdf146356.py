# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.linguistic.linguservice import get_lingu_service_manager
from com.sun.star.lang import Locale
import time

# insert paragraph in front of a protected table of contents
# and insert text into this new paragraph
class tdf146356(UITestCase):
    def test_tdf146356_insert_para_before_TOX(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf146356.odt")) as writer_doc:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ALT+RETURN"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "Some text"}))

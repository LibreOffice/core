# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import org.libreoffice.unotest
from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
import time

class tdf108910(UITestCase):

    def test_tdf108910_alphabetical_index_utf8(self):
        # Copy concordance file containing an utf8 index entry
        org.libreoffice.unotest.makeCopyFromTDOC("tdf108910.sdi")
        with self.ui_test.load_file(get_url_for_data_file("tdf108910.odt")) as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()

            with self.ui_test.execute_dialog_through_command(".uno:InsertMultiIndex", close_button="cancel") as xDialog:
                # Open the index entries of the concordance file
                xFromFile = xDialog.getChild("fromfile")
                self.assertEqual(get_state_as_dict(xFromFile)["Selected"], "true")
                xFile = xDialog.getChild("file")
                with self.ui_test.execute_blocking_action(xFile.executeAction, args=('OPENFROMLIST', mkPropertyValues({"POS": "2"}))) as xAutoMarkDialog:
                    xIndexEntry = xAutoMarkDialog.getChild("entry")
                    # Without the fix in place the index does not contain the ut8 index entry
                    self.assertEqual("Αὐτὸς ἔφα", get_state_as_dict(xIndexEntry)['Text'])

# vim: set shiftwidth=4 softtabstop=4 expandtab:

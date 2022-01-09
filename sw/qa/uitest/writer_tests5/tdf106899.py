# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import type_text
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_url_for_data_file

class tdf106899(UITestCase):

    def test_tdf106899_alphabetical_index_utf8(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf106899.odt")) as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()

            # Update the alphabetical index and check if it contains the utf8 index entry
            xDocumentIndexes = document.DocumentIndexes
            self.assertEqual(xDocumentIndexes.getCount(), 1)
            self.assertEqual(xDocumentIndexes.hasByName("Alphabetical Index1"), True)
            xIndexAnchor = xDocumentIndex.getAnchor()
            xDocumentIndex = xDocumentIndexes.getByName("Alphabetical Index1")
            self.assertEqual("Nguyễn Khánh" not in xIndexAnchor.getString(), True)
            xDocumentIndex.update()
            xIndexAnchor = xDocumentIndex.getAnchor()
            self.assertEqual("Nguyễn Khánh" in xIndexAnchor.getString(), True)

# vim: set shiftwidth=4 softtabstop=4 expandtab:

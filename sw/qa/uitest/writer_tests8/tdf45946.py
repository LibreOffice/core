# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase

class tdf45946(UITestCase):

    def test_tdf45946_convert_selected_field(self):
        with self.ui_test.create_doc_in_start_center("writer") as writer_doc:
            self.xUITest.executeCommand(".uno:InsertDateField")
            self.xUITest.executeCommand(".uno:GoToStartOfLine")
            self.assertTrue(writer_doc.getTextFields().createEnumeration().hasMoreElements())
            self.xUITest.executeCommand(".uno:ConvertSelectedField")
            self.assertFalse(writer_doc.getTextFields().createEnumeration().hasMoreElements())

# vim: set shiftwidth=4 softtabstop=4 expandtab:

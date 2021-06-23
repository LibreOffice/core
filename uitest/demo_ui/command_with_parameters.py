# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import type_text
from libreoffice.uno.propertyvalue import mkPropertyValues

class CommandWithParametersTest(UITestCase):

    def test_text_color_change(self):

        self.ui_test.create_doc_in_start_center("writer")

        self.xUITest.executeCommandWithParameters(".uno:Color",
            mkPropertyValues({"Color": 16776960}))
        xWriterEdit = self.xUITest.getTopFocusWindow().getChild("writer_edit")
        type_text(xWriterEdit, "LibreOffice")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

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
from uitest.uihelper.common import select_pos

import unittest

class tdf105466(UITestCase):

    @unittest.skip("issue with floating windows")
    def test_changing_conditional_format(self):

        with self.ui_test.create_doc_in_start_center("calc"):

            with self.ui_test.execute_modeless_dialog_through_command(".uno:ConditionalFormatDialog") as xCondFormatDlg:

                for i in range(0,4):
                    with self.subTest(i = i):
                        xTypeLstBox = xCondFormatDlg.getChild("type")
                        select_pos(xTypeLstBox, str(i))

# vim: set shiftwidth=4 softtabstop=4 expandtab:

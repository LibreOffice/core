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
from uitest.uihelper.common import select_pos

class tdf142785(UITestCase):
    def test_tdf142785(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf142785.docx")) as document:

            sHeader = "ODD   ODD   ODD   ODD   ODD   header"
            sFooter = "ODD   ODD   ODD   ODD   ODD   footer"

            xStandardStyle = document.StyleFamilies.PageStyles.Standard
            self.assertEqual(sHeader, xStandardStyle.HeaderText.String)
            self.assertEqual(sHeader, xStandardStyle.HeaderTextFirst.String)
            self.assertEqual(sHeader, xStandardStyle.HeaderTextLeft.String)
            self.assertEqual(sHeader, xStandardStyle.HeaderTextRight.String)

            self.assertEqual(sFooter, xStandardStyle.FooterText.String)
            self.assertEqual(sFooter, xStandardStyle.FooterTextFirst.String)
            self.assertEqual(sFooter, xStandardStyle.FooterTextLeft.String)
            self.assertEqual(sFooter, xStandardStyle.FooterTextRight.String)

            with self.ui_test.execute_dialog_through_command(".uno:PageStyleName") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "4")
                checkSameLR = xDialog.getChild("checkSameLR-header")
                checkSameLR.executeAction("CLICK",tuple())

            self.assertEqual(sHeader, xStandardStyle.HeaderText.String)
            self.assertEqual(sHeader, xStandardStyle.HeaderTextFirst.String)
            self.assertEqual("EVEN   EVEN   EVEN   EVEN   EVEN   header", xStandardStyle.HeaderTextLeft.String)
            self.assertEqual(sHeader, xStandardStyle.HeaderTextRight.String)

            self.assertEqual(sFooter, xStandardStyle.FooterText.String)
            self.assertEqual(sFooter, xStandardStyle.FooterTextFirst.String)
            self.assertEqual(sFooter, xStandardStyle.FooterTextLeft.String)
            self.assertEqual(sFooter, xStandardStyle.FooterTextRight.String)

            with self.ui_test.execute_dialog_through_command(".uno:PageStyleName") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "5")
                checkSameLR = xDialog.getChild("checkSameLR-footer")
                checkSameLR.executeAction("CLICK",tuple())

            self.assertEqual(sHeader, xStandardStyle.HeaderText.String)
            self.assertEqual(sHeader, xStandardStyle.HeaderTextFirst.String)
            self.assertEqual("EVEN   EVEN   EVEN   EVEN   EVEN   header", xStandardStyle.HeaderTextLeft.String)
            self.assertEqual(sHeader, xStandardStyle.HeaderTextRight.String)

            self.assertEqual(sFooter, xStandardStyle.FooterText.String)
            self.assertEqual(sFooter, xStandardStyle.FooterTextFirst.String)

            # Without the fix in place, this test would have failed with
            # AssertionError: 'EVEN   EVEN   EVEN   EVEN   EVEN   footer' != 'ODD   ODD   ODD   ODD   ODD   footer'
            self.assertEqual("EVEN   EVEN   EVEN   EVEN   EVEN   footer", xStandardStyle.FooterTextLeft.String)
            self.assertEqual(sFooter, xStandardStyle.FooterTextRight.String)

# vim: set shiftwidth=4 softtabstop=4 expandtab:

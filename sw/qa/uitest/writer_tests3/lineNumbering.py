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
from uitest.uihelper.common import select_by_text
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import change_measurement_unit

class WriterLineNumbering(UITestCase):

    def test_line_numbering_dialog(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            change_measurement_unit(self, "Centimeter")

            with self.ui_test.execute_dialog_through_command(".uno:LineNumberingDialog") as xDialog:
                xshownumbering = xDialog.getChild("shownumbering")
                xstyledropdown = xDialog.getChild("styledropdown")
                xformatdropdown = xDialog.getChild("formatdropdown")
                xpositiondropdown = xDialog.getChild("positiondropdown")
                xspacingspin = xDialog.getChild("spacingspin")
                xintervalspin = xDialog.getChild("intervalspin")
                xtextentry = xDialog.getChild("textentry")
                xlinesspin = xDialog.getChild("linesspin")
                xblanklines = xDialog.getChild("blanklines")
                xlinesintextframes = xDialog.getChild("linesintextframes")
                xshowfooterheadernumbering = xDialog.getChild("showfooterheadernumbering")
                xrestarteverynewpage = xDialog.getChild("restarteverynewpage")

                xshownumbering.executeAction("CLICK", tuple())
                select_by_text(xstyledropdown, "Bullets")
                select_by_text(xformatdropdown, "A, B, C, ...")
                select_by_text(xpositiondropdown, "Right")
                xspacingspin.executeAction("UP", tuple())
                xintervalspin.executeAction("UP", tuple())
                xtextentry.executeAction("TYPE", mkPropertyValues({"TEXT":";"}))
                xlinesspin.executeAction("UP", tuple())
                xblanklines.executeAction("CLICK", tuple())
                xlinesintextframes.executeAction("CLICK", tuple())
                xshowfooterheadernumbering.executeAction("CLICK", tuple())
                xrestarteverynewpage.executeAction("CLICK", tuple())

            with self.ui_test.execute_dialog_through_command(".uno:LineNumberingDialog", close_button="cancel") as xDialog:
                xshownumbering = xDialog.getChild("shownumbering")
                xstyledropdown = xDialog.getChild("styledropdown")
                xformatdropdown = xDialog.getChild("formatdropdown")
                xpositiondropdown = xDialog.getChild("positiondropdown")
                xspacingspin = xDialog.getChild("spacingspin")
                xintervalspin = xDialog.getChild("intervalspin")
                xtextentry = xDialog.getChild("textentry")
                xlinesspin = xDialog.getChild("linesspin")
                xblanklines = xDialog.getChild("blanklines")
                xlinesintextframes = xDialog.getChild("linesintextframes")
                xshowfooterheadernumbering = xDialog.getChild("showfooterheadernumbering")
                xrestarteverynewpage = xDialog.getChild("restarteverynewpage")

                self.assertEqual(get_state_as_dict(xshownumbering)["Selected"], "true")
                self.assertEqual(get_state_as_dict(xstyledropdown)["SelectEntryText"], "Bullets")
                self.assertEqual(get_state_as_dict(xformatdropdown)["SelectEntryText"], "A, B, C, ...")
                self.assertEqual(get_state_as_dict(xpositiondropdown)["SelectEntryText"], "Right")
                self.assertEqual(get_state_as_dict(xspacingspin)["Text"], "0.60 cm")
                self.assertEqual(get_state_as_dict(xintervalspin)["Text"], "6")
                self.assertEqual(get_state_as_dict(xtextentry)["Text"], ";")
                self.assertEqual(get_state_as_dict(xlinesspin)["Text"], "4")
                self.assertEqual(get_state_as_dict(xblanklines)["Selected"], "false")
                self.assertEqual(get_state_as_dict(xlinesintextframes)["Selected"], "true")
                self.assertEqual(get_state_as_dict(xshowfooterheadernumbering)["Selected"], "true")
                self.assertEqual(get_state_as_dict(xrestarteverynewpage)["Selected"], "true")

    def test_tdf86185(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:LineNumberingDialog", close_button="cancel") as xDialog:
                xshownumbering = xDialog.getChild("shownumbering")
                xformatdropdown = xDialog.getChild("formatdropdown")

                xshownumbering.executeAction("CLICK", tuple())
                itemFormat = ["1, 2, 3, ...", "A, B, C, ...", "a, b, c, ...", "I, II, III, ...", "i, ii, iii, ...", "A, .., AA, .., AAA, ..."]
                for i in range(6):
                    select_by_text(xformatdropdown, itemFormat[i])
                    self.assertEqual(get_state_as_dict(xformatdropdown)["SelectEntryText"], itemFormat[i])
# vim: set shiftwidth=4 softtabstop=4 expandtab:

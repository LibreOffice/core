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
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import select_by_text
from uitest.uihelper.common import change_measurement_unit

#Outline Numbering is now Chapter Numbering

class WriterChapterNumbering(UITestCase):

    def test_chapter_numbering_dialog(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            change_measurement_unit(self, "Millimeter")

            with self.ui_test.execute_dialog_through_command(".uno:ChapterNumberingDialog") as xDialog:
                xstyle = xDialog.getChild("style")
                xnumbering = xDialog.getChild("numbering")
                xcharstyle = xDialog.getChild("charstyle")
                xprefix = xDialog.getChild("prefix")
                xsuffix = xDialog.getChild("suffix")
                xstartat = xDialog.getChild("startat")
                xtab = xDialog.getChild("tabcontrol")
                #second tab
                xalignedatmf = xDialog.getChild("alignedatmf")
                xnum2alignlb = xDialog.getChild("num2alignlb")
                xnumfollowedbylb = xDialog.getChild("numfollowedbylb")
                xatmf = xDialog.getChild("atmf")
                xindentatmf = xDialog.getChild("indentatmf")
                select_pos(xtab, "0") #first tab
                select_by_text(xstyle, "Heading")
                select_by_text(xnumbering, "1, 2, 3, ...")
                select_by_text(xcharstyle, "Bullets")
                xprefix.executeAction("TYPE", mkPropertyValues({"TEXT":"A"}))
                xsuffix.executeAction("TYPE", mkPropertyValues({"TEXT":"B"}))
                xstartat.executeAction("UP", tuple())

                select_pos(xtab, "1") #second tab Position
                xalignedatmf.executeAction("UP", tuple())
                select_by_text(xnum2alignlb, "Centered")
                select_by_text(xnumfollowedbylb, "Tab stop")
                xatmf.executeAction("UP", tuple())
                xindentatmf.executeAction("UP", tuple())


            with self.ui_test.execute_dialog_through_command(".uno:ChapterNumberingDialog", close_button="cancel") as xDialog:

                xstyle = xDialog.getChild("style")
                xnumbering = xDialog.getChild("numbering")
                xcharstyle = xDialog.getChild("charstyle")
                xprefix = xDialog.getChild("prefix")
                xsuffix = xDialog.getChild("suffix")
                xstartat = xDialog.getChild("startat")
                xtab = xDialog.getChild("tabcontrol")
                #second tab
                xalignedatmf = xDialog.getChild("alignedatmf")
                xnum2alignlb = xDialog.getChild("num2alignlb")
                xnumfollowedbylb = xDialog.getChild("numfollowedbylb")
                xatmf = xDialog.getChild("atmf")
                xindentatmf = xDialog.getChild("indentatmf")
                select_pos(xtab, "0")
                self.assertEqual(get_state_as_dict(xstyle)["SelectEntryText"], "Heading")
                self.assertEqual(get_state_as_dict(xnumbering)["SelectEntryText"], "1, 2, 3, ...")
                self.assertEqual(get_state_as_dict(xcharstyle)["SelectEntryText"], "Bullets")
                self.assertEqual(get_state_as_dict(xprefix)["Text"], "A")
                self.assertEqual(get_state_as_dict(xsuffix)["Text"], "B")
                self.assertEqual(get_state_as_dict(xstartat)["Text"], "2")
                select_pos(xtab, "1") #second tab Position
                self.assertEqual(get_state_as_dict(xalignedatmf)["Text"], "0.1 mm")
                self.assertEqual(get_state_as_dict(xnum2alignlb)["SelectEntryText"], "Centered")
                self.assertEqual(get_state_as_dict(xnumfollowedbylb)["SelectEntryText"], "Tab stop")
                self.assertEqual(get_state_as_dict(xatmf)["Text"], "0.1 mm")
                self.assertEqual(get_state_as_dict(xindentatmf)["Text"], "0.1 mm")

# vim: set shiftwidth=4 softtabstop=4 expandtab:

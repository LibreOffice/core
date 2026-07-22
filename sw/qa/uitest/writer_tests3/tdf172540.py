# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos, select_by_text
from com.sun.star.style.NumberingType import ARABIC, ROMAN_LOWER

class tdf172540(UITestCase):

    def test_tdf172540_footnote_endnote_settings_html(self):
        # Tools > Footnotes/Endnotes Settings used to crash LibreOffice when
        # the current document was an HTML (Writer/Web) document.
        # SwEndNoteOptionPage::Reset() looked up a widget named "allstyles"
        # that no longer exists in footnotepage.ui/endnotepage.ui
        # For a .odt document the null dereference (crash root cause) is never reached
        with self.ui_test.load_empty_file("writer/web") as document:

            endnoteSettings = document.getEndnoteSettings()
            self.assertEqual(ROMAN_LOWER, endnoteSettings.NumberingType)

            # Change the endnote numbering format, matching the exact
            # scenario from the bug report (changing "i, ii, iii..." to
            # "1, 2, 3..."), to confirm the dialog is fully functional.
            with self.ui_test.execute_dialog_through_command(".uno:FootnoteDialog") as xDialog:
                # Switch to the Endnotes tab: the widget needs to actually be
                # visible for the SELECT action to take effect.
                select_pos(xDialog.getChild("tabcontrol"), "1")

                # The "numberinglb" id is used by both the Footnotes and the
                # Endnotes tab page, so look up there
                # to reach the Endnotes tab's control
                endnotePage = xDialog.getChild("EndnotePage")
                numberingBox = endnotePage.getChild("numberinglb")
                select_by_text(numberingBox, "1, 2, 3, ...")

            endnoteSettings = document.getEndnoteSettings()
            self.assertEqual(ARABIC, endnoteSettings.NumberingType)

# vim: set shiftwidth=4 softtabstop=4 expandtab:

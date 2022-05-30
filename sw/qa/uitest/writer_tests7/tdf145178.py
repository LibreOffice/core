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
from uitest.uihelper.common import get_state_as_dict, select_pos
from uitest.uihelper.common import get_url_for_data_file

class tdf145178(UITestCase):

    def test_tdf145178(self):

        with self.ui_test.load_file(get_url_for_data_file('tdf145178.fodt')):

            with self.ui_test.execute_dialog_through_command(".uno:EditRegion") as xDialog:
                xTree = xDialog.getChild("tree")
                self.assertEqual("1", get_state_as_dict(xTree)['Children'])
                self.assertEqual("Section1", get_state_as_dict(xTree.getChild('0'))['Text'])
                self.assertEqual("Section1", get_state_as_dict(xDialog.getChild('curname'))['Text'])

                xOptions = xDialog.getChild("options")
                with self.ui_test.execute_blocking_action(
                        xOptions.executeAction, args=('CLICK', ())) as xOptDialog:
                    xTabs = xOptDialog.getChild("tabcontrol")
                    select_pos(xTabs, "3")

                    xFtnExt = xOptDialog.getChild("ftnntattextend")
                    xFtnNum = xOptDialog.getChild("ftnntnum")
                    xFtnFrm = xOptDialog.getChild("ftnntnumfmt")
                    xFtnPre = xOptDialog.getChild("ftnprefix")
                    xFtnSuf = xOptDialog.getChild("ftnsuffix")
                    xFtnView = xOptDialog.getChild("ftnnumviewbox")
                    xFtnOffset = xOptDialog.getChild("ftnoffset")

                    self.assertEqual("true", get_state_as_dict(xFtnExt)["Enabled"])
                    self.assertEqual("true", get_state_as_dict(xFtnNum)["Enabled"])
                    self.assertEqual("true", get_state_as_dict(xFtnFrm)["Enabled"])
                    self.assertEqual("true", get_state_as_dict(xFtnPre)["Enabled"])
                    self.assertEqual("true", get_state_as_dict(xFtnSuf)["Enabled"])
                    self.assertEqual("true", get_state_as_dict(xFtnView)["Enabled"])
                    self.assertEqual("true", get_state_as_dict(xFtnOffset)["Enabled"])

                    self.assertEqual("1, 2, 3, ...", get_state_as_dict(xFtnView)["DisplayText"])
                    self.assertEqual("(", get_state_as_dict(xFtnPre)["Text"])

                    # Without the fix in place, this test would have failed with
                    # AssertionError: ')' != ''
                    self.assertEqual(")", get_state_as_dict(xFtnSuf)["Text"])

                    self.assertEqual("3", get_state_as_dict(xFtnOffset)["Text"])

                    xEndExt = xOptDialog.getChild("endntattextend")
                    xEndNum = xOptDialog.getChild("endntnum")
                    xEndFrm = xOptDialog.getChild("endntnumfmt")
                    xEndPre = xOptDialog.getChild("endprefix")
                    xEndSuf = xOptDialog.getChild("endsuffix")
                    xEndView = xOptDialog.getChild("endnumviewbox")
                    xEndOffset = xOptDialog.getChild("endoffset")

                    self.assertEqual("true", get_state_as_dict(xEndExt)["Enabled"])
                    self.assertEqual("true", get_state_as_dict(xEndNum)["Enabled"])
                    self.assertEqual("true", get_state_as_dict(xEndFrm)["Enabled"])
                    self.assertEqual("true", get_state_as_dict(xEndPre)["Enabled"])
                    self.assertEqual("true", get_state_as_dict(xEndSuf)["Enabled"])
                    self.assertEqual("true", get_state_as_dict(xEndView)["Enabled"])
                    self.assertEqual("true", get_state_as_dict(xEndOffset)["Enabled"])

                    self.assertEqual("i, ii, iii, ...", get_state_as_dict(xEndView)["DisplayText"])
                    self.assertEqual("[", get_state_as_dict(xEndPre)["Text"])
                    self.assertEqual("]", get_state_as_dict(xEndSuf)["Text"])

                    self.assertEqual("2", get_state_as_dict(xEndOffset)["Text"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:

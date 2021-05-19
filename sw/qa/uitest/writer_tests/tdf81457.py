# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_pos

class tdf81457(UITestCase):

#tdf 81457
   def test_open_documentProperties_tdf81457(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf81457.odt")) as writer_doc:
            with self.ui_test.execute_dialog_through_command(".uno:SetDocumentProperties") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "2")     #tab Custom properties

                # tdf#123919 - custom document properties are sorted now
                aExpectedDocProp = {
                    2: {'aAndra': 'Ja'},
                    4: {'BookMarkCount': '78'},
                    5: {'BookMarkInfo1': '00FF0000FF010'},
                    6: {'BookMarkInfo2': '00FF0000FF030'}}

                for pos, aDocProp in aExpectedDocProp.items():
                    xNameBox = xDialog.getChild("namebox" + str(pos))
                    xTypeBox = xDialog.getChild("typebox" + str(pos))
                    xValueEdit = xDialog.getChild("valueedit" + str(pos))
                    name, value = aDocProp.popitem()
                    self.assertEqual(name, get_state_as_dict(xNameBox)['Text'])
                    self.assertEqual('Text', get_state_as_dict(xTypeBox)['DisplayText'])
                    self.assertEqual(value, get_state_as_dict(xValueEdit)['Text'][:13])



# vim: set shiftwidth=4 softtabstop=4 expandtab:

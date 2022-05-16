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

class Tdf148395(UITestCase):

    def test_tdf148395(self):

        with self.ui_test.create_doc_in_start_center("writer") as document:

            with self.ui_test.execute_dialog_through_command(".uno:InsertObject") as xDialog:
                xTypes = xDialog.getChild("types")
                xCreateNew = xDialog.getChild("createnew")

                self.assertEqual('true', get_state_as_dict(xCreateNew)['Checked'])

                xSelectedEntry = get_state_as_dict(xTypes)['SelectEntryText']
                self.assertTrue('Spreadsheet' in xSelectedEntry )
                # On Windows, there's also 6th item: "Further objects"
                self.assertLessEqual('5', get_state_as_dict(xTypes)['Children'])

                for i in range(5):
                    xTypes.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
                    xSelectedEntry = get_state_as_dict(xTypes)['SelectEntryText']
                    if 'Chart' in xSelectedEntry:
                        break

            self.assertEqual(1, document.EmbeddedObjects.Count)
            self.assertEqual("SwXTextEmbeddedObject", document.CurrentSelection.getImplementationName())

            xChartMainTop = self.xUITest.getTopFocusWindow()
            xChartMain = xChartMainTop.getChild("chart_window")
            xSeriesObj =  xChartMain.getChild("CID/Page=")

            # Without the fix in place, this test would have crashed here
            with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramArea"})) as xDialog:
                pass

# vim: set shiftwidth=4 softtabstop=4 expandtab:

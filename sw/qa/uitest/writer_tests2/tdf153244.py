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
from uitest.uihelper.common import get_url_for_data_file, get_state_as_dict, select_by_text

class tdf153244(UITestCase):

   def test_tdf153244(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            text = document.getText()
            cursor = text.createTextCursor()
            textGraphic = document.createInstance('com.sun.star.text.TextGraphicObject')
            provider = self.xContext.ServiceManager.createInstance('com.sun.star.graphic.GraphicProvider')
            graphic = provider.queryGraphic( mkPropertyValues({"URL": get_url_for_data_file("LibreOffice.jpg")}))
            textGraphic.Graphic = graphic
            text.insertTextContent(cursor, textGraphic, False)
            #select image
            document.getCurrentController().select(document.getDrawPage()[0])

            with self.ui_test.execute_dialog_through_command(".uno:InsertCaptionDialog") as xCaptionDialog:

                xOptionsBtn = xCaptionDialog.getChild("options")
                with self.ui_test.execute_blocking_action(xOptionsBtn.executeAction, args=('CLICK', ())) as xOptionsDialog:
                    xSeparator = xOptionsDialog.getChild("separator")
                    xLevel = xOptionsDialog.getChild("level")
                    self.assertEqual("[None]", get_state_as_dict(xLevel)["DisplayText"])
                    self.assertEqual(".", get_state_as_dict(xSeparator)["Text"])

                    select_by_text(xLevel, "1")
                    xSeparator.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                    xSeparator.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                    xSeparator.executeAction("TYPE", mkPropertyValues({"TEXT":"x"}))

                with self.ui_test.execute_blocking_action(xOptionsBtn.executeAction, args=('CLICK', ())) as xOptionsDialog:
                    xSeparator = xOptionsDialog.getChild("separator")
                    xLevel = xOptionsDialog.getChild("level")

                    # Without the fix in place, this test would have failed with
                    # AssertionError: '1' != '[None]'
                    self.assertEqual("1", get_state_as_dict(xLevel)["DisplayText"])
                    self.assertEqual("x", get_state_as_dict(xSeparator)["Text"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:

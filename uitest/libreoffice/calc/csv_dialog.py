# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from contextlib import contextmanager

@contextmanager
def load_csv_file(UITestCase, fileName, bUseDefaultOptions):
    with UITestCase.ui_test.execute_dialog_through_command(".uno:Open", close_button="open") as xOpenDialog:

        xFileName = xOpenDialog.getChild("file_name")
        xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file(fileName)}))

    xDialog = UITestCase.ui_test.wait_for_top_focus_window('TextImportCsvDialog')

    try:
        if bUseDefaultOptions:
            xSeparatedBy = xDialog.getChild("toseparatedby")
            xSeparatedBy.executeAction("CLICK", tuple())

            xTextDelimiter = xDialog.getChild("textdelimiter")
            xTextDelimiter.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
            xTextDelimiter.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
            xTextDelimiter.executeAction("TYPE", mkPropertyValues({"TEXT": "\""}))

            setToTrue = ['tab', 'comma', 'semicolon']
            for childName in setToTrue:
                xChild = xDialog.getChild(childName)
                if get_state_as_dict(xChild)['Selected'] == 'false':
                    xChild.executeAction("CLICK", tuple())
                UITestCase.assertEqual('true', get_state_as_dict(xChild)['Selected'])

            setToFalse = ['space', 'other', 'removespace', 'mergedelimiters',
                    'evaluateformulas', 'quotedfieldastext', 'detectspecialnumbers']
            for childName in setToFalse:
                xChild = xDialog.getChild(childName)
                if get_state_as_dict(xChild)['Selected'] == 'true':
                    xChild.executeAction("CLICK", tuple())
                UITestCase.assertEqual('false', get_state_as_dict(xChild)['Selected'])

            UITestCase.assertEqual('1', get_state_as_dict(xDialog.getChild("fromrow"))['Text'])

        yield xDialog
    finally:
        xOK = xDialog.getChild('ok')
        with UITestCase.ui_test.wait_until_component_loaded():
            UITestCase.ui_test.close_dialog_through_button(xOK)

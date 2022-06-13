# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.uihelper.common import get_state_as_dict

def reset_default_values(UITestCase, xDialog):
        setToFalse = ['paste_all', 'formats', 'comments', 'objects', 'formulas',
                'link', 'transpose', 'skip_empty']
        for childName in setToFalse:
            xChild = xDialog.getChild(childName)
            if get_state_as_dict(xChild)['Selected'] == 'true':
                xChild.executeAction("CLICK", tuple())
            UITestCase.assertEqual('false', get_state_as_dict(xChild)['Selected'])

        setToTrue = ['text', 'numbers', 'datetime', 'cbImmediately']
        for childName in setToTrue:
            xChild = xDialog.getChild(childName)
            if get_state_as_dict(xChild)['Selected'] == 'false':
                xChild.executeAction("CLICK", tuple())
            UITestCase.assertEqual('true', get_state_as_dict(xChild)['Selected'])

        setToCheck = ['none', 'no_shift']
        for childName in setToCheck:
            xChild = xDialog.getChild(childName)
            xChild.executeAction("CLICK", tuple())
            UITestCase.assertEqual('true', get_state_as_dict(xChild)['Checked'])

# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.framework import UITestCase
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

class TestTdf153161(UITestCase):

    def testTdf153161(self):

        document = self.ui_test.load_file(get_url_for_data_file('tdf153161_FlushToSave.odp'))
        oldText = document.DrawPages[0].getByIndex(1).String
        self.assertTrue(oldText.startswith('在没有版本控制系统的时期'))

        xImpressDoc = self.xUITest.getTopFocusWindow()
        xEditWin = xImpressDoc.getChild('impress_win')
        xEditWin.executeAction('SELECT', mkPropertyValues({'OBJECT':'Unnamed Drawinglayer object 1'}))

        # Type something, getting into text editing mode (appending) automatically
        xEditWin.executeAction('TYPE', mkPropertyValues({'TEXT': 'Foo Bar'}))
        xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
        xToolkit.processEventsToIdle()
        self.xUITest.executeCommand('.uno:Save')
        self.xUITest.executeCommand('.uno:Reload')

        # Reload and check that the edit was saved
        document = self.ui_test.get_component()
        self.assertEqual(oldText + 'Foo Bar', document.DrawPages[0].getByIndex(1).String)

# vim: set shiftwidth=4 softtabstop=4 expandtab:

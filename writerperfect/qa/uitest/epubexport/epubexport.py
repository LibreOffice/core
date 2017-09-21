#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.framework import UITestCase


def handleDialog(dialog):
    # Select the second entry to request EPUB2, not EPUB3.
    dialog.getChild("versionlb").executeAction("SELECT", mkPropertyValues({"POS": "1"}))
    dialog.getChild("ok").executeAction("CLICK", tuple())


# Test for EPUBExportDialog and EPUBExportUIComponent.
class EPUBExportTest(UITestCase):

    def testUIComponent(self):

        uiComponent = self.ui_test._xContext.ServiceManager.createInstanceWithContext("com.sun.star.comp.Writer.EPUBExportUIComponent", self.ui_test._xContext)
        self.ui_test.execute_blocking_action(action=uiComponent.execute, dialog_handler=handleDialog)
        propertyValues = uiComponent.getPropertyValues()
        filterData = [i.Value for i in propertyValues if i.Name == "FilterData"][0]
        epubVersion = [i.Value for i in filterData if i.Name == "EPUBVersion"][0]
        # This was 30, EPUBExportDialog::VersionSelectHdl() did not set the version.
        self.assertEqual(20, epubVersion)

# vim: set shiftwidth=4 softtabstop=4 expandtab:

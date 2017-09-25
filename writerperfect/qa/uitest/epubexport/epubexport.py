#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from com.sun.star.beans import PropertyValue
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.framework import UITestCase
import uno


def handleDialog(dialog):
    # Select the second entry to request EPUB2, not EPUB3.
    dialog.getChild("versionlb").executeAction("SELECT", mkPropertyValues({"POS": "1"}))
    dialog.getChild("ok").executeAction("CLICK", tuple())


# Test for EPUBExportDialog and EPUBExportUIComponent.
class EPUBExportTest(UITestCase):

    def testUIComponent(self):

        uiComponent = self.ui_test._xContext.ServiceManager.createInstanceWithContext("com.sun.star.comp.Writer.EPUBExportUIComponent", self.ui_test._xContext)

        # Make sure we get what we asked for.
        self.assertEqual("com.sun.star.comp.Writer.EPUBExportUIComponent", uiComponent.getImplementationName())
        self.assertEqual(True, uiComponent.supportsService("com.sun.star.ui.dialogs.FilterOptionsDialog"))
        # Just make sure this doesn't fail.
        uiComponent.setTitle(str())

        self.ui_test.execute_blocking_action(action=uiComponent.execute, dialog_handler=handleDialog)
        propertyValues = uiComponent.getPropertyValues()
        filterData = [i.Value for i in propertyValues if i.Name == "FilterData"][0]
        epubVersion = [i.Value for i in filterData if i.Name == "EPUBVersion"][0]
        # This was 30, EPUBExportDialog::VersionSelectHdl() did not set the version.
        self.assertEqual(20, epubVersion)

    def testCustomProperties(self):
        uiComponent = self.ui_test._xContext.ServiceManager.createInstanceWithContext("com.sun.star.comp.Writer.EPUBExportUIComponent", self.ui_test._xContext)

        # Make sure that by default the version is not altered.
        propertyValues = uiComponent.getPropertyValues()
        filterData = [i.Value for i in propertyValues if i.Name == "FilterData"][0]
        self.assertEqual(0, len([i.Value for i in filterData if i.Name == "EPUBVersion"]))

        # But if we set it explicitly, then it's retained, even without interacting with the UI.
        filterData = (PropertyValue(Name="EPUBVersion", Value=30),)
        propertyValues = (PropertyValue(Name="FilterData", Value=uno.Any("[]com.sun.star.beans.PropertyValue", filterData)),)
        uiComponent.setPropertyValues(propertyValues)
        propertyValues = uiComponent.getPropertyValues()
        filterData = [i.Value for i in propertyValues if i.Name == "FilterData"][0]
        epubVersion = [i.Value for i in filterData if i.Name == "EPUBVersion"][0]
        self.assertEqual(30, epubVersion)

# vim: set shiftwidth=4 softtabstop=4 expandtab:

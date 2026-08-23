# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf133348(UITestCase):

    # The comment author comes from the user name, see SvtUserOptions
    GIVEN_NAME = '/org.openoffice.UserProfile/Data/givenname'

    def test_tdf133348(self):

        with self.ui_test.create_doc_in_start_center("writer") as document:

            self.xUITest.executeCommand(".uno:SelectAll")
            xArgs = mkPropertyValues({"Text": "C1"})
            self.xUITest.executeCommandWithParameters(".uno:InsertAnnotation", xArgs)

            with self.ui_test.set_config(self.GIVEN_NAME, "Known Author"):
                xArgs = mkPropertyValues({"Text": "C2"})
                self.xUITest.executeCommandWithParameters(".uno:ReplyComment", xArgs)

                # Wait for async events to be processed
                xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
                xToolkit.processEventsToIdle()

                xEnum = document.TextFields.createEnumeration()
                self.assertEqual(xEnum.nextElement().Author.strip(), 'Unknown Author')
                self.assertEqual(xEnum.nextElement().Author.strip(), 'Known Author')

                self.xUITest.executeCommand(".uno:Undo")
                self.xUITest.executeCommand(".uno:Undo")

                # Without the fix in place, it would have crashed here
                self.xUITest.executeCommand(".uno:Undo")
                self.xUITest.executeCommand(".uno:Undo")

                # all comments have been deleted
                self.assertFalse(document.TextFields.createEnumeration().hasMoreElements())

# vim: set shiftwidth=4 softtabstop=4 expandtab:

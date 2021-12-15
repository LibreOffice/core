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
from uitest.uihelper.common import get_state_as_dict, type_text, select_pos

class slideShowSettings(UITestCase):
    def test_slideShowSettings(self):
        with self.ui_test.create_doc_in_start_center("impress"):
            MainWindow = self.xUITest.getTopFocusWindow()
            TemplateDialog = self.xUITest.getTopFocusWindow()
            cancel = TemplateDialog.getChild("close")
            self.ui_test.close_dialog_through_button(cancel)
            self.xUITest.executeCommand(".uno:InsertPage")
            with self.ui_test.execute_dialog_through_command(".uno:PresentationDialog") as PresentationDialog:

                xfrom = PresentationDialog.getChild("from")
                xfrom.executeAction("CLICK",tuple())
                from_cb = PresentationDialog.getChild("from_cb")
                select_pos(from_cb, "0")
                window = PresentationDialog.getChild("window")
                window.executeAction("CLICK",tuple())
                manualslides = PresentationDialog.getChild("manualslides")
                manualslides.executeAction("CLICK",tuple())
                pointervisible = PresentationDialog.getChild("pointervisible")
                pointervisible.executeAction("CLICK",tuple())
                pointeraspen = PresentationDialog.getChild("pointeraspen")
                pointeraspen.executeAction("CLICK",tuple())
                animationsallowed = PresentationDialog.getChild("animationsallowed")
                animationsallowed.executeAction("CLICK",tuple())
                changeslidesbyclick = PresentationDialog.getChild("changeslidesbyclick")
                changeslidesbyclick.executeAction("CLICK",tuple())

            #verify
            with self.ui_test.execute_dialog_through_command(".uno:PresentationDialog", close_button="cancel") as PresentationDialog:
                xfrom = PresentationDialog.getChild("from")
                self.assertEqual(get_state_as_dict(xfrom)["Checked"], "true")
                from_cb = PresentationDialog.getChild("from_cb")
                self.assertEqual(get_state_as_dict(from_cb)["SelectEntryText"], "Slide 1")
                manualslides = PresentationDialog.getChild("manualslides")
                self.assertEqual(get_state_as_dict(manualslides)["Selected"], "true")
                pointervisible = PresentationDialog.getChild("pointervisible")
                self.assertEqual(get_state_as_dict(pointervisible)["Selected"], "true")
                pointeraspen = PresentationDialog.getChild("pointeraspen")
                self.assertEqual(get_state_as_dict(pointeraspen)["Selected"], "true")
                animationsallowed = PresentationDialog.getChild("animationsallowed")
                self.assertEqual(get_state_as_dict(animationsallowed)["Selected"], "false")
                changeslidesbyclick = PresentationDialog.getChild("changeslidesbyclick")
                self.assertEqual(get_state_as_dict(changeslidesbyclick)["Selected"], "false")



# vim: set shiftwidth=4 softtabstop=4 expandtab:

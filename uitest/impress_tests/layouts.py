#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues

class ImpressLayouts(UITestCase):

    def test_impress_layouts(self):

        self.ui_test.create_doc_in_start_center("impress")

        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("close")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        layouts= (".uno:AssignLayout?WhatLayout:long=20", ".uno:AssignLayout?WhatLayout:long=19",
                    ".uno:AssignLayout?WhatLayout:long=0", ".uno:AssignLayout?WhatLayout:long=1",
                    ".uno:AssignLayout?WhatLayout:long=32", ".uno:AssignLayout?WhatLayout:long=3",
                    ".uno:AssignLayout?WhatLayout:long=12", ".uno:AssignLayout?WhatLayout:long=15",
                    ".uno:AssignLayout?WhatLayout:long=14", ".uno:AssignLayout?WhatLayout:long=16",
                    ".uno:AssignLayout?WhatLayout:long=18", ".uno:AssignLayout?WhatLayout:long=34",
                    ".uno:AssignLayout?WhatLayout:long=28", ".uno:AssignLayout?WhatLayout:long=27",
                    ".uno:AssignLayout?WhatLayout:long=29", ".uno:AssignLayout?WhatLayout:long=30")

        for i in layouts:
            self.xUITest.executeCommand(i)

            xImpressDoc = self.xUITest.getTopFocusWindow()

            xEditWin = xImpressDoc.getChild("impress_win")

            # There's a layout with 7 objects
            for j in range(0,6):
                xEditWin.executeAction("SELECT", mkPropertyValues({"OBJECT":"Unnamed Drawinglayer object " + str(j)}))
                xEditWin.executeAction("DESELECT", tuple())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

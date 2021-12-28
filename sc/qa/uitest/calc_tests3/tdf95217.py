# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.httpserver import launchHTTPServer
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_url_for_data_file
from libreoffice.calc.document import get_cell_by_position

class tdf95217(UITestCase):

    def test_tdf95217(self):

        with launchHTTPServer() as httpd:
            xAddress = "http://%s:%d/tdf95217.html" % (httpd.server_name, httpd.server_port)
            with self.ui_test.execute_dialog_through_command(".uno:Open", close_button="open") as xOpenDialog:

                xFileName = xOpenDialog.getChild("file_name")
                xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": xAddress}))

            document = self.ui_test.get_component()
            self.assertEqual(document.TextTables.getCount(), 1)

            self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, select_pos, get_url_for_data_file


class Tdf166756(UITestCase):

    def test_tdf166756_edit_header_and_footer(self):
        # Test that the filename is correctly displayed in the edit header and footer dialog
        # even if the filename contains special chars that are encoded in a URL
        # This test does not cover actual printing or print preview
        fileName="tdf166756_[1].ods"
        fileURL = get_url_for_data_file(fileName)
        urlFileBaseName = fileURL.split("/")[-1];

        # check that the file name URL encoding is different from fileName
        # this is the purpose of the test
        self.assertEqual("tdf166756_%5B1%5D.ods", urlFileBaseName)

        with self.ui_test.load_file(fileURL):

            # Check that the file name fields in header and footer preview are not URL encoded
            with self.ui_test.execute_dialog_through_command(".uno:PageFormatDialog") as xPageFormatDialog:
                xTabControl = xPageFormatDialog.getChild("tabcontrol")
                headerTab = "4"
                footerTab = "5"
                for tab in [headerTab, footerTab]:
                    select_pos(xTabControl, tab)
                    with self.ui_test.execute_dialog_through_command(".uno:EditHeaderAndFooter") as xHeaderDialog:
                        xLeftTextWindow = xHeaderDialog.getChild("textviewWND_LEFT")
                        content = get_state_as_dict(xLeftTextWindow)["Text"].split("\n")

                        self.assertEqual("Title: tdf166756_[1]", content[0])
                        # check file name is not url encoded
                        self.assertEqual("FileName: tdf166756_[1].ods", content[1])
                        # check that path is not url encoded
                        self.assertEqual("tdf166756_[1].ods", content[2].split("/")[-1])


# vim: set shiftwidth=4 softtabstop=4 expandtab:

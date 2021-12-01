# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase

class tdf92611(UITestCase):

    def test_launch_and_close_bibliography(self):

        with self.ui_test.create_doc_in_start_center("writer"):

            self.xUITest.executeCommand(".uno:BibliographyComponent")

            self.xUITest.executeCommand(".uno:CloseWin")


# vim: set shiftwidth=4 softtabstop=4 expandtab:

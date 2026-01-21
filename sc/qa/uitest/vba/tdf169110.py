# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from com.sun.star.uno import Exception as UnoException
from uitest.uihelper.common import get_url_for_data_file

class TDF169110(UITestCase):

    def test_vba_cutcopymode_corruption_with_ide(self):
        # This test reproduces a regression where opening the Basic IDE
        # triggered full VBA initialization during code indexing,
        # corrupting the runtime state (CutCopyMode) of active scripts.

        with self.ui_test.load_file(get_url_for_data_file("tdf169110.ods")) as document:
            self.xUITest.executeCommand(".uno:BasicIDEAppear")

            xScriptProvider = document.getScriptProvider()
            sScriptURL = (
                "vnd.sun.star.script:"
                "VBAProject.Module1.countifs_benchmark"
                "?language=Basic&location=document"
            )
            xScript = xScriptProvider.getScript(sScriptURL)

            try:
                xScript.invoke((), (), ())
            except UnoException as e:
                self.fail(
                    "Regression tdf#169110 reproduced: macro failed after "
                    f"Basic IDE/Object Browser activation: {e}"
                )

# vim: set shiftwidth=4 softtabstop=4 expandtab:

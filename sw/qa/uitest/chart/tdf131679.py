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
from uitest.uihelper.common import get_state_as_dict
from uitest.debug import sleep

class tdf131679( UITestCase ):

    def test_Testtdf131679( self ):
        with self.ui_test.load_file(get_url_for_data_file("tdf95554.ods")):

            #Insert Chart
            self.xUITest.executeCommand( ".uno:InsertObjectChart" )
            xChartMainTop = self.xUITest.getTopFocusWindow()
            xChartMain = xChartMainTop.getChild( "chart_window" )
            xChart = xChartMain.getChild( "CID/Page=" )
            xChart.executeAction( "SELECT", tuple())

            # test should fail here without the fix
            self.xUITest.executeCommand(".uno:Cut")

            self.ui_test.close_doc()

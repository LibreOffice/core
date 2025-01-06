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

class tdf138556( UITestCase ):

    def test_stock_chart13_insert_series( self ):
        #Start LibreOffice Writer
        with self.ui_test.create_doc_in_start_center( "writer" ):

            #Insert Chart
            self.xUITest.executeCommand( ".uno:InsertObjectChart" )
            xChartMainTop = self.xUITest.getTopFocusWindow()
            xChartMain = xChartMainTop.getChild( "chart_window" )
            xChart = xChartMain.getChild( "CID/Page=" )

            #Change Chart Type to Stock 1
            #TODO: test other subtypes
            with self.ui_test.execute_dialog_through_action( xChart, "COMMAND", mkPropertyValues({ "COMMAND" : "DiagramType" })) as xDialog:
                xChartType = xDialog.getChild( "charttype" )
                xStockType = xChartType.getChild( "9" )
                xStockType.executeAction( "SELECT", tuple())

            #Insert Data Series
            with self.ui_test.execute_dialog_through_action( xChart, "COMMAND", mkPropertyValues({ "COMMAND" : "DiagramData" }), close_button="close") as xDialog:
                xToolbar = xDialog.getChild( "toolbar" )
                xToolbar.executeAction( "CLICK", mkPropertyValues({ "POS" : "1" }))

            #Check Number of Sequences
            xDocument = self.ui_test.get_component()
            nSequences = len( xDocument.FirstDiagram.
                CoordinateSystems[0].ChartTypes[0].DataSeries[0].DataSequences )
            self.assertEqual( nSequences, 3 )

# vim: set shiftwidth=4 softtabstop=4 expandtab:

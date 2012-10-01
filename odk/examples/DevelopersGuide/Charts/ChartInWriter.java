/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

// __________ Imports __________

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.*;

// property access
import com.sun.star.beans.*;

// application specific classes
import com.sun.star.chart.*;
import com.sun.star.drawing.*;
import com.sun.star.frame.XModel;
// base graphics things
import com.sun.star.awt.Point;
import com.sun.star.awt.Size;
// Exceptions
import com.sun.star.uno.Exception;
import com.sun.star.uno.RuntimeException;
import com.sun.star.beans.UnknownPropertyException;


// __________ Implementation __________

/** Test to create a writer document and insert an OLE Chart.

    Be careful!  This does not really work.  The Writer currently has no
    interface for dealing with OLE objects.  You can add an OLE shape to the
    Writer's drawing layer, but it is not treated correctly as OLE object.
    Thus, you can not activate the chart by double-clicking.  The office may
    also crash when the document is closed!

 */
public class ChartInWriter
{
    // ____________________

    public static void main( String args[] )
    {
        Helper aHelper = new Helper( args );

        ChartHelper aChartHelper = new ChartHelper(
            (XModel) UnoRuntime.queryInterface( XModel.class,
                                                aHelper.createTextDocument()));

        // the unit for measures is 1/100th of a millimeter
        // position at (1cm, 1cm)
        Point aPos    = new Point( 1000, 1000 );

        // size of the chart is 15cm x 12cm
        Size  aExtent = new Size( 15000, 13000 );

        // insert a new chart into the "Chart" sheet of the
        // spreadsheet document
        XChartDocument aChartDoc = aChartHelper.insertOLEChartInWriter(
            "BarChart",
            aPos,
            aExtent,
            "com.sun.star.chart.AreaDiagram" );

        // instantiate test class with newly created chart
        ChartInWriter aTest   = new ChartInWriter( aChartDoc );

        try
        {
             aTest.lockControllers();

            // do tests here
             aTest.testWall();

             aTest.unlockControllers();
        }
        catch( Exception ex )
        {
            System.out.println( "UNO Exception caught: " + ex );
            System.out.println( "Message: " + ex.getMessage() );
        }

        System.exit( 0 );
    }


    // ________________________________________

    public ChartInWriter( XChartDocument aChartDoc )
    {
        maChartDocument = aChartDoc;
        maDiagram       = maChartDocument.getDiagram();
    }

    // ____________________

    public void lockControllers()
        throws RuntimeException
    {
        ((XModel) UnoRuntime.queryInterface( XModel.class, maChartDocument )).lockControllers();
    }

    // ____________________

    public void unlockControllers()
        throws RuntimeException
    {
        ((XModel) UnoRuntime.queryInterface( XModel.class, maChartDocument )).unlockControllers();
    }

    // ____________________

    public void testWall()
        throws RuntimeException, UnknownPropertyException, PropertyVetoException,
               com.sun.star.lang.IllegalArgumentException, WrappedTargetException
    {
        XPropertySet aWall = ((X3DDisplay) UnoRuntime.queryInterface(
                                  X3DDisplay.class, maDiagram )).getWall();

        // change background color of area
        aWall.setPropertyValue( "FillColor", new Integer( 0xeecc99 ));
        aWall.setPropertyValue( "FillStyle",  FillStyle.SOLID );
    }

    // ______________________________
    //
    // private members
    // ______________________________

    private XChartDocument maChartDocument;
    private XDiagram       maDiagram;
}

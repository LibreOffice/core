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

// Create a spreadsheet add some data and add a chart

public class ChartInDraw
{
    // ____________________

    public static void main( String args[] )
    {
        Helper aHelper = new Helper( args );

        ChartHelper aChartHelper = new ChartHelper( aHelper.createDrawingDocument());

        // the unit for measures is 1/100th of a millimeter
        // position at (1cm, 1cm)
        Point aPos    = new Point( 1000, 1000 );

        // size of the chart is 15cm x 12cm
        Size  aExtent = new Size( 15000, 13000 );

        // insert a new chart into the "Chart" sheet of the
        // spreadsheet document
        XChartDocument aChartDoc = aChartHelper.insertOLEChartInDraw(
            "BarChart",
            aPos,
            aExtent,
            "com.sun.star.chart.BarDiagram" );

        // instantiate test class with newly created chart
        ChartInDraw aTest   = new ChartInDraw( aChartDoc );

        try
        {
            aTest.lockControllers();

            aTest.testArea();
            aTest.testWall();
            aTest.testTitle();
            aTest.testLegend();
            aTest.testThreeD();

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

    public ChartInDraw( XChartDocument aChartDoc )
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

    public void testArea()
        throws RuntimeException, UnknownPropertyException, PropertyVetoException,
               com.sun.star.lang.IllegalArgumentException, WrappedTargetException
    {
        XPropertySet   aArea = maChartDocument.getArea();

        if( aArea != null )
        {
            // change background color of entire chart
            aArea.setPropertyValue( "FillStyle", FillStyle.SOLID );
            aArea.setPropertyValue( "FillColor", new Integer( 0xeeeeee ));
        }
    }

    // ____________________

    public void testWall()
        throws RuntimeException, UnknownPropertyException, PropertyVetoException,
               com.sun.star.lang.IllegalArgumentException, WrappedTargetException
    {
        XPropertySet aWall = ((X3DDisplay) UnoRuntime.queryInterface(
                                  X3DDisplay.class, maDiagram )).getWall();

        // change background color of area
        aWall.setPropertyValue( "FillColor", new Integer( 0xcccccc ));
        aWall.setPropertyValue( "FillStyle",  FillStyle.SOLID );
    }

    // ____________________

    public void testTitle()
        throws RuntimeException, UnknownPropertyException, PropertyVetoException,
               com.sun.star.lang.IllegalArgumentException, WrappedTargetException
    {
        // change main title
        XPropertySet aDocProp = (XPropertySet) UnoRuntime.queryInterface(
            XPropertySet.class, maChartDocument );
        aDocProp.setPropertyValue( "HasMainTitle", new Boolean( true ));

        XShape aTitle = maChartDocument.getTitle();
        XPropertySet aTitleProp = (XPropertySet) UnoRuntime.queryInterface( XPropertySet.class, aTitle );

        // set new text
        if( aTitleProp != null )
        {
            aTitleProp.setPropertyValue( "String", "Bar Chart in a Draw Document" );
        }
    }

    // ____________________

    public void testLegend()
        throws RuntimeException, UnknownPropertyException, PropertyVetoException,
               com.sun.star.lang.IllegalArgumentException, WrappedTargetException
    {
        XShape aLegend = maChartDocument.getLegend();
        XPropertySet aLegendProp = (XPropertySet) UnoRuntime.queryInterface( XPropertySet.class, aLegend );

        aLegendProp.setPropertyValue( "Alignment", ChartLegendPosition.LEFT );
        aLegendProp.setPropertyValue( "FillStyle", FillStyle.SOLID );
        aLegendProp.setPropertyValue( "FillColor", new Integer( 0xeeddee ));
    }

    // ____________________

    public void testThreeD()
        throws RuntimeException, UnknownPropertyException, PropertyVetoException,
        com.sun.star.lang.IllegalArgumentException, WrappedTargetException,
        com.sun.star.lang.IndexOutOfBoundsException
    {
        XPropertySet aDiaProp = (XPropertySet) UnoRuntime.queryInterface( XPropertySet.class, maDiagram );
        Boolean aTrue = new Boolean( true );

        aDiaProp.setPropertyValue( "Dim3D", aTrue );
        aDiaProp.setPropertyValue( "Deep", aTrue );
        // from Chart3DBarProperties:
        aDiaProp.setPropertyValue( "SolidType", new Integer( ChartSolidType.CYLINDER ));

        // change floor color to Magenta6
        XPropertySet aFloor = ((X3DDisplay) UnoRuntime.queryInterface(
                                   X3DDisplay.class, maDiagram )).getFloor();
        aFloor.setPropertyValue( "FillColor", new Integer( 0x6b2394 ));

        // apply changes to get a 3d scene
        unlockControllers();
        lockControllers();


        // rotate scene to a different angle
        HomogenMatrix aMatrix = new HomogenMatrix();
        HomogenMatrixLine aLines[] = new HomogenMatrixLine[]
            {
                new HomogenMatrixLine( 1.0, 0.0, 0.0, 0.0 ),
                new HomogenMatrixLine( 0.0, 1.0, 0.0, 0.0 ),
                new HomogenMatrixLine( 0.0, 0.0, 1.0, 0.0 ),
                new HomogenMatrixLine( 0.0, 0.0, 0.0, 1.0 )
            };

        aMatrix.Line1 = aLines[ 0 ];
        aMatrix.Line2 = aLines[ 1 ];
        aMatrix.Line3 = aLines[ 2 ];
        aMatrix.Line4 = aLines[ 3 ];

        // rotate 10 degrees along the x axis
        double fAngle = 10.0;
        double fCosX = java.lang.Math.cos( java.lang.Math.PI / 180.0 * fAngle );
        double fSinX = java.lang.Math.sin( java.lang.Math.PI / 180.0 * fAngle );

        // rotate -20 degrees along the y axis
        fAngle = -20.0;
        double fCosY = java.lang.Math.cos( java.lang.Math.PI / 180.0 * fAngle );
        double fSinY = java.lang.Math.sin( java.lang.Math.PI / 180.0 * fAngle );

        // rotate -5 degrees along the z axis
        fAngle = -5.0;
        double fCosZ = java.lang.Math.cos( java.lang.Math.PI / 180.0 * fAngle );
        double fSinZ = java.lang.Math.sin( java.lang.Math.PI / 180.0 * fAngle );

        aMatrix.Line1.Column1 =  fCosY *  fCosZ;
        aMatrix.Line1.Column2 =  fCosY * -fSinZ;
        aMatrix.Line1.Column3 =  fSinY;

        aMatrix.Line2.Column1 =  fSinX *  fSinY *  fCosZ +  fCosX *  fSinZ;
        aMatrix.Line2.Column2 = -fSinX *  fSinY *  fSinZ +  fCosX *  fCosZ;
        aMatrix.Line2.Column3 = -fSinX *  fCosY;

        aMatrix.Line3.Column1 = -fCosX *  fSinY *  fCosZ +  fSinX *  fSinZ;
        aMatrix.Line3.Column2 =  fCosX *  fSinY *  fSinZ +  fSinX *  fCosZ;
        aMatrix.Line3.Column3 =  fCosX *  fCosY;

        aDiaProp.setPropertyValue( "D3DTransformMatrix", aMatrix );

        // add a red light source

        // in a chart by default only the second (non-specular) light source is switched on
        // light source 1 is a specular light source
        aDiaProp.setPropertyValue( "D3DSceneLightColor1", new Integer( 0xff3333 ));

        // set direction
        com.sun.star.drawing.Direction3D aDirection = new com.sun.star.drawing.Direction3D();

        aDirection.DirectionX = -0.75;
        aDirection.DirectionY =  0.5;
        aDirection.DirectionZ =  0.5;

        aDiaProp.setPropertyValue( "D3DSceneLightDirection1", aDirection );
        aDiaProp.setPropertyValue( "D3DSceneLightOn1", new Boolean( true ));
    }

    // ______________________________
    //
    // private members
    // ______________________________

    private XChartDocument maChartDocument;
    private XDiagram       maDiagram;
}

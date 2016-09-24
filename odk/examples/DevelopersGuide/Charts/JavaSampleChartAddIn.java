/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

import com.sun.star.uno.UnoRuntime;
import com.sun.star.container.XIndexAccess;
import com.sun.star.lib.uno.helper.WeakBase;

// factories
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;

// graphics stuff
import com.sun.star.drawing.*;
import com.sun.star.awt.Point;
import com.sun.star.awt.Size;

// chart stuff
import com.sun.star.chart.*;

// property access
import com.sun.star.beans.*;

// Add-In stuff
import com.sun.star.lang.XInitialization;
import com.sun.star.util.XRefreshable;
import com.sun.star.lang.XServiceName;
import com.sun.star.lang.XServiceInfo;
// Exceptions
import com.sun.star.uno.Exception;
import com.sun.star.uno.RuntimeException;

import javax.swing.JOptionPane;

public class JavaSampleChartAddIn extends WeakBase implements
    XInitialization,
    XRefreshable,
    XDiagram,
    XServiceName,
    XServiceInfo
{

    // __________ interface methods __________

    // XInitialization
    public void initialize( Object[] aArguments )
        throws Exception, RuntimeException
    {
        if( aArguments.length > 0 )
        {
            maChartDocument = UnoRuntime.queryInterface(
                XChartDocument.class, aArguments[ 0 ]);

            XPropertySet aDocProp = UnoRuntime.queryInterface(
                XPropertySet.class, maChartDocument );
            if( aDocProp != null )
            {
                // set base diagram which will be extended in refresh()
                aDocProp.setPropertyValue( "BaseDiagram", "com.sun.star.chart.XYDiagram" );
            }

            // get the draw page
            XDrawPageSupplier aPageSupp = UnoRuntime.queryInterface(
                XDrawPageSupplier.class, maChartDocument );
            if( aPageSupp != null )
                maDrawPage = UnoRuntime.queryInterface(
                    XDrawPage.class, aPageSupp.getDrawPage() );

            // get a factory for creating shapes
            maShapeFactory = UnoRuntime.queryInterface(
                XMultiServiceFactory.class, maChartDocument );
        }
    }

    // XRefreshable
    public void refresh() throws RuntimeException
    {
        // recycle shapes in first call, if document was loaded
        if( maBottomLine == null ||
            maTopLine == null )
        {
            // try to recycle loaded shapes
            XPropertySet aDocProp = UnoRuntime.queryInterface(
                XPropertySet.class, maChartDocument );
            if( aDocProp != null )
            {
                try
                {
                    XIndexAccess aShapesIA = UnoRuntime.queryInterface(
                        XIndexAccess.class, aDocProp.getPropertyValue( "AdditionalShapes" ));
                    if( aShapesIA != null &&
                        aShapesIA.getCount() > 0 )
                    {
                        XShape aShape;
                        String aName;
                        for( int i = aShapesIA.getCount() - 1; i >= 0; --i )
                        {
                            aShape = UnoRuntime.queryInterface(
                                XShape.class, aShapesIA.getByIndex( i ));
                            if( aShape != null )
                            {
                                XPropertySet aProp = UnoRuntime.queryInterface(
                                    XPropertySet.class, aShape );
                                aName = (String) aProp.getPropertyValue( "Name" );

                                if( aName.equals( "top" ))
                                {
                                    maTopLine = aShape;
                                }
                                else if( aName.equals( "bottom" ))
                                {
                                    maBottomLine = aShape;
                                }
                            }
                        }
                    }
                }
                catch( Exception ex )
                {
                    JOptionPane.showMessageDialog( null, ex, "Exception caught", JOptionPane.WARNING_MESSAGE );
                }
            }
        }

        // create top line if it does not yet exist
        try
        {
            if( maTopLine == null )
            {
                maTopLine = UnoRuntime.queryInterface(
                    XShape.class, maShapeFactory.createInstance( "com.sun.star.drawing.LineShape" ));
                maDrawPage.add( maTopLine );

                // make line red and thicker
                XPropertySet aShapeProp = UnoRuntime.queryInterface(
                    XPropertySet.class, maTopLine );

                aShapeProp.setPropertyValue( "LineColor", Integer.valueOf( 0xe01010 ));
                aShapeProp.setPropertyValue( "LineWidth", Integer.valueOf( 50 ));
                aShapeProp.setPropertyValue( "Name", "top" );
            }
        }
        catch( Exception ex )
        {
            JOptionPane.showMessageDialog( null, ex, "Exception caught", JOptionPane.WARNING_MESSAGE );
        }

        // create bottom line if it does not yet exist
        try
        {
            if( maBottomLine == null )
            {
                maBottomLine = UnoRuntime.queryInterface(
                    XShape.class, maShapeFactory.createInstance( "com.sun.star.drawing.LineShape" ));
                maDrawPage.add( maBottomLine );

                // make line green and thicker
                XPropertySet aShapeProp = UnoRuntime.queryInterface(
                    XPropertySet.class, maBottomLine );

                aShapeProp.setPropertyValue( "LineColor", Integer.valueOf( 0x10e010 ));
                aShapeProp.setPropertyValue( "LineWidth", Integer.valueOf( 50 ));
                aShapeProp.setPropertyValue( "Name", "bottom" );
            }
        }
        catch( Exception ex )
        {
            JOptionPane.showMessageDialog( null, ex, "Exception caught", JOptionPane.WARNING_MESSAGE );
        }

        if( maTopLine == null ||
            maBottomLine == null )
        {
            JOptionPane.showMessageDialog( null, "One of the lines is still null", "Assertion", JOptionPane.WARNING_MESSAGE );
            return;
        }

        // position lines


        // get data
        XChartDataArray aDataArray = UnoRuntime.queryInterface(
            XChartDataArray.class, maChartDocument.getData());
        double aData[][] = aDataArray.getData();

        // get axes
        XDiagram aDiagram = maChartDocument.getDiagram();
        XShape aXAxis = UnoRuntime.queryInterface(
            XShape.class, UnoRuntime.queryInterface(
                               XAxisXSupplier.class, aDiagram ).getXAxis() );
        XShape aYAxis = UnoRuntime.queryInterface(
            XShape.class, UnoRuntime.queryInterface(
                               XAxisYSupplier.class, aDiagram ).getYAxis() );

        // calculate points for hull
        final int nLength = aData.length;
        int i, j;
        double fMax, fMin;

        Point aMaxPtSeq[][] = new Point[ 1 ][];
        aMaxPtSeq[ 0 ] = new Point[ nLength ];
        Point aMinPtSeq[][] = new Point[ 1 ][];
        aMinPtSeq[ 0 ] = new Point[ nLength ];

        for( i = 0; i < nLength; i++ )
        {
            fMin = fMax = aData[ i ][ 1 ];
            for( j = 1; j < aData[ i ].length; j++ )
            {
                if( aData[ i ][ j ] > fMax )
                    fMax = aData[ i ][ j ];
                else if( aData[ i ][ j ] < fMin )
                    fMin = aData[ i ][ j ];
            }
            aMaxPtSeq[ 0 ][ i ] = new Point( getAxisPosition( aXAxis, aData[ i ][ 0 ], false ),
                                             getAxisPosition( aYAxis, fMax, true ));
            aMinPtSeq[ 0 ][ i ] = new Point( getAxisPosition( aXAxis, aData[ i ][ 0 ], false ),
                                             getAxisPosition( aYAxis, fMin, true ));
        }

        // apply point sequences to lines
        try
        {
            XPropertySet aShapeProp = UnoRuntime.queryInterface(
                XPropertySet.class, maTopLine );
            aShapeProp.setPropertyValue( "PolyPolygon", aMaxPtSeq );

            aShapeProp = UnoRuntime.queryInterface(
                XPropertySet.class, maBottomLine );
            aShapeProp.setPropertyValue( "PolyPolygon", aMinPtSeq );
        }
        catch( Exception ex )
        {
            JOptionPane.showMessageDialog( null, ex, "Exception caught", JOptionPane.WARNING_MESSAGE );
        }
    }

    public void addRefreshListener( com.sun.star.util.XRefreshListener aListener )
        throws RuntimeException
    {
        // we don't want this but we have to implement the interface
    }

    public void removeRefreshListener( com.sun.star.util.XRefreshListener aListener )
        throws RuntimeException
    {
        // we don't want this but we have to implement the interface
    }


    // XServiceName
    public String getServiceName() throws RuntimeException
    {
        return smServiceName;
    }

    // XServiceInfo
    public boolean supportsService( String aServiceName )
    {
        String[] aServices = getSupportedServiceNames_Static();
        int i, nLength = aServices.length;
        boolean bResult = false;

        for( i = 0; !bResult && i < nLength; ++i )
            bResult = aServiceName.equals( aServices[ i ] );

        return bResult;
    }

    public String getImplementationName()
    {
        return( JavaSampleChartAddIn.class.getName() );
    }

    public String[] getSupportedServiceNames()
    {
        return getSupportedServiceNames_Static();
    }

    // XDiagram
    public String getDiagramType() throws RuntimeException
    {
        return smServiceName;
    }

    public XPropertySet getDataRowProperties( int nRow )
        throws com.sun.star.lang.IndexOutOfBoundsException, RuntimeException
    {
        return maChartDocument.getDiagram().getDataRowProperties( nRow );
    }

    public XPropertySet getDataPointProperties( int nCol, int nRow )
        throws com.sun.star.lang.IndexOutOfBoundsException, RuntimeException
    {
        return maChartDocument.getDiagram().getDataPointProperties( nCol, nRow );
    }

    // XShape : XDiagram
    public Size getSize() throws RuntimeException
    {
        return UnoRuntime.queryInterface( XShape.class, maChartDocument.getDiagram()).getSize();
    }
    public void setSize( Size aSize ) throws RuntimeException, PropertyVetoException
    {
        UnoRuntime.queryInterface( XShape.class, maChartDocument.getDiagram()).setSize( aSize );
    }

    public Point getPosition() throws RuntimeException
    {
        return UnoRuntime.queryInterface( XShape.class, maChartDocument.getDiagram()).getPosition();
    }
    public void setPosition( Point aPos ) throws RuntimeException
    {
        UnoRuntime.queryInterface( XShape.class, maChartDocument.getDiagram()).setPosition( aPos );
    }

    // XShapeDescriptor : XShape : XDiagram
    public String getShapeType() throws RuntimeException
    {
        return "com.sun.star.comp.Chart.JavaSampleDiagramShape";
    }


    // __________ private members __________
    private com.sun.star.chart.XChartDocument       maChartDocument;
    private com.sun.star.drawing.XDrawPage          maDrawPage;
    private com.sun.star.lang.XMultiServiceFactory  maShapeFactory;

    // shapes added by add-in
    private com.sun.star.drawing.XShape             maTopLine;
    private com.sun.star.drawing.XShape             maBottomLine;

    // __________ private methods __________

    private int getAxisPosition( XShape aAxis, double fValue, boolean bVertical )
    {
        int nResult = 0;

        if( aAxis != null )
        {
            XPropertySet aAxisProp = UnoRuntime.queryInterface(
                XPropertySet.class, aAxis );

            try
            {
                double fMin, fMax;
                fMin = ((Double) aAxisProp.getPropertyValue( "Min" )).doubleValue();
                fMax = ((Double) aAxisProp.getPropertyValue( "Max" )).doubleValue();
                double fRange = fMax - fMin;

                if( fMin <= fValue && fValue <= fMax &&
                    fRange != 0 )
                {
                    if( bVertical )
                    {
                        nResult = aAxis.getPosition().Y +
                            (int)((aAxis.getSize().Height) *
                                  (1.0 - (( fValue - fMin ) / fRange )));
                    }
                    else
                    {
                        nResult = aAxis.getPosition().X +
                            (int)((aAxis.getSize().Width) *
                                  (( fValue - fMin ) / fRange ));
                    }
                }
            }
            catch( Exception ex )
            {
                JOptionPane.showMessageDialog( null, ex, "Exception caught", JOptionPane.WARNING_MESSAGE );
            }
        }
        return nResult;
    }

    // __________ static things __________

    private static final String smServiceName  = "com.sun.star.comp.Chart.JavaSampleChartAddIn";

    public static String[] getSupportedServiceNames_Static()
    {
        String[] aResult = { smServiceName,
                             "com.sun.star.chart.Diagram",
                             "com.sun.star.chart.ChartAxisYSupplier" };
        return aResult;
    }


    /**
     * Returns a factory for creating the service.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns a <code>XSingleServiceFactory</code> for creating the component
     * @param   implName     the name of the implementation for which a service is desired
     * @param   multiFactory the service manager to be used if needed
     * @param   regKey       the registryKey
     * @see                  com.sun.star.comp.loader.JavaLoader
     */
    public static XSingleServiceFactory __getServiceFactory(
        String implName,
        XMultiServiceFactory multiFactory,
        com.sun.star.registry.XRegistryKey regKey )
    {
        XSingleServiceFactory xSingleServiceFactory = null;

        if( implName.equals( JavaSampleChartAddIn.class.getName()) )
        {
            xSingleServiceFactory = com.sun.star.comp.loader.FactoryHelper.getServiceFactory(
                JavaSampleChartAddIn.class, smServiceName,
                multiFactory,  regKey );
        }

        return xSingleServiceFactory;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

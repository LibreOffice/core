// package name: as default, start with complex
package qa;

// imports
import complexlib.ComplexTestCase;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Type;
import com.sun.star.uno.XComponentContext;

import java.io.PrintWriter;
import java.util.Hashtable;

import com.sun.star.lang.*;
import com.sun.star.beans.*;
import com.sun.star.frame.*;
import com.sun.star.chart.*;
import com.sun.star.drawing.*;
import com.sun.star.util.XCloseable;
import com.sun.star.util.CloseVetoException;

import drafts.com.sun.star.chart2.XTitled;
import drafts.com.sun.star.chart2.XTitle;
import drafts.com.sun.star.chart2.XDataProvider;
import drafts.com.sun.star.chart2.XFormattedString;

import com.sun.star.uno.AnyConverter;
import com.sun.star.comp.helper.ComponentContext;

/**
 * The following Complex Test will test the
 * com.sun.star.document.IndexedPropertyValues
 * service
 */

public class TestCaseOldAPI extends ComplexTestCase {

    // The name of the tested service
    private final String testedServiceName =
          "com.sun.star.chart.ChartDocument";

    // The first of the mandatory functions:
    /**
     * Return the name of the test.
     * In this case it is the actual name of the service.
     * @return The tested service.
     */
    public String getTestObjectName() {
        return testedServiceName;
    }

    // The second of the mandatory functions: return all test methods as an
    // array. There is only one test function in this example.
    /**
     * Return all test methods.
     * @return The test methods.
     */
    public String[] getTestMethodNames() {
        return new String[] {
            "testTitle",
            "testSubTitle",
            "testDiagram",
            "testAxis",
            "testLegend",
            "testArea",
            "testChartType"
        };
    }

    // ____________

    public void before()
    {
        boolean bCreateView = false;

        if( bCreateView )
            mxChartModel = createDocument( "chart" );
        else
            mxChartModel = createChartModel();

        createFileDataSource( mxChartModel );
        mxOldDoc = (XChartDocument) UnoRuntime.queryInterface(
            XChartDocument.class, mxChartModel );
    }

    // ____________

    public void after()
    {
        XCloseable xCloseable = (XCloseable) UnoRuntime.queryInterface(
            XCloseable.class, mxChartModel );
        assure( "document is no XCloseable", xCloseable != null );

        try
        {
            xCloseable.close( true );
        }
        catch( CloseVetoException ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }

    // ____________

    public void testTitle()
    {
        try
        {
            // set title at new chart
            XTitled xTitled = (XTitled) UnoRuntime.queryInterface(
                XTitled.class, mxChartModel );

            // set title via new API
            XTitle xTitle = setTitle( xTitled, "Sample", "@main-title" );

//             printInterfacesAndServices( xTitle );

            // get title via old API
            XShape xTitleShape = mxOldDoc.getTitle();
            XPropertySet xTitleProp = (XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class, xTitleShape );

            // set property via old API
            if( xTitleProp != null )
            {
                String aTitle = " Overwritten by Old API ";
                float fHeight = (float)17.0;

                xTitleProp.setPropertyValue( "String", aTitle );
                xTitleProp.setPropertyValue( "CharHeight", new Float( fHeight ) );

                float fNewHeight = AnyConverter.toFloat( xTitleProp.getPropertyValue( "CharHeight" ) );
                assure( "Changing CharHeight via old API failed", fNewHeight == fHeight );

                String aNewTitle = AnyConverter.toString( xTitleProp.getPropertyValue( "String" ) );
                assure( "Property \"String\" failed", aNewTitle.equals( aTitle ));
            }
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }

    // ____________

    public void testSubTitle()
    {
        try
        {
            // trying back-querying (from old Wrapper to new Model)
            drafts.com.sun.star.chart2.XChartDocument xNewDoc =
                (drafts.com.sun.star.chart2.XChartDocument) UnoRuntime.queryInterface(
                    drafts.com.sun.star.chart2.XChartDocument.class,
                    mxOldDoc );

            // set title at new chart
            XTitled xTitled = (XTitled) UnoRuntime.queryInterface(
                XTitled.class, xNewDoc.getDiagram() );

            // set title via new API
            setTitle( xTitled, "Sub", "@sub-title" );

            // get Property via old API
            XShape xTitleShape = mxOldDoc.getSubTitle();
            XPropertySet xTitleProp = (XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class, xTitleShape );

            // set Property via old API
            if( xTitleProp != null )
            {
                int nColor = 0x009acd; // DeepSkyBlue3
                float fWeight = com.sun.star.awt.FontWeight.BOLD;

                xTitleProp.setPropertyValue( "CharColor", new Integer( nColor ) );
                xTitleProp.setPropertyValue( "CharWeight", new Float( fWeight ));

                int nNewColor = AnyConverter.toInt( xTitleProp.getPropertyValue( "CharColor" ) );
                assure( "Changing CharColor via old API failed", nNewColor == nColor );

                float fNewWeight = AnyConverter.toFloat( xTitleProp.getPropertyValue( "CharWeight" ) );
                assure( "Changing CharWeight via old API failed", fNewWeight == fWeight );
            }
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }

    // ------------

    public void testDiagram()
    {
        try
        {
            // testing wall
            XDiagram xDia = mxOldDoc.getDiagram();
            if( xDia != null )
            {
                X3DDisplay xDisp = (X3DDisplay) UnoRuntime.queryInterface(
                    X3DDisplay.class, xDia );
                assure( "X3DDisplay not supported", xDisp != null );

                XPropertySet xProp = xDisp.getWall();
                if( xProp != null )
                {
                    log.println( "Testing wall" );

                    int nColor = 0xffe1ff; // thistle1

                    xProp.setPropertyValue( "FillColor", new Integer( nColor ) );

                    int nNewColor = AnyConverter.toInt( xProp.getPropertyValue( "FillColor" ) );
                    assure( "Changing FillColor via old API failed", nNewColor == nColor );
                }

                assure( "Wrong Diagram Type", xDia.getDiagramType().equals(
                            "com.sun.star.chart.BarDiagram" ));
            }
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }

    // ------------

    public void testAxis()
    {
        try
        {
            XAxisYSupplier xYAxisSuppl = (XAxisYSupplier) UnoRuntime.queryInterface(
                XAxisYSupplier.class, mxOldDoc.getDiagram() );
            assure( "Diagram is no y-axis supplier", xYAxisSuppl != null );

            XPropertySet xProp = xYAxisSuppl.getYAxis();
            assure( "No y-axis found", xProp != null );

            double nNewMax = 12.3;
            double nNewOrigin = 2.7;

            xProp.setPropertyValue( "Max", new Double( nNewMax ));
            assure( "AutoMax is on", ! AnyConverter.toBoolean( xProp.getPropertyValue( "AutoMax" )) );

            assure( "Maximum value invalid",
                    approxEqual(
                        AnyConverter.toDouble( xProp.getPropertyValue( "Max" )),
                        nNewMax ));

            xProp.setPropertyValue( "AutoMin", new Boolean( true ));
            assure( "AutoMin is off", AnyConverter.toBoolean( xProp.getPropertyValue( "AutoMin" )) );

            // missing: Min/Max, etc. is not always returning a double
//             Object oMin = xProp.getPropertyValue( "Min" );
//             assure( "No Minimum set", AnyConverter.isDouble( oMin ));
//             log.println( "Minimum retrieved: " + AnyConverter.toDouble( oMin ));

            xProp.setPropertyValue( "Origin", new Double( nNewOrigin ));
            assure( "Origin invalid",
                    approxEqual(
                        AnyConverter.toDouble( xProp.getPropertyValue( "Origin" )),
                        nNewOrigin ));
            xProp.setPropertyValue( "AutoOrigin", new Boolean( true ));
            assure( "AutoOrigin is off", AnyConverter.toBoolean( xProp.getPropertyValue( "AutoOrigin" )) );

            xProp.setPropertyValue( "Logarithmic", new Boolean( true ));
            assure( "Scaling is not logarithmic",
                    AnyConverter.toBoolean( xProp.getPropertyValue( "Logarithmic" )) );
            xProp.setPropertyValue( "Logarithmic", new Boolean( false ));
            assure( "Scaling is not logarithmic",
                    ! AnyConverter.toBoolean( xProp.getPropertyValue( "Logarithmic" )) );

            int nNewColor =  0xcd853f; // peru
            xProp.setPropertyValue( "LineColor", new Integer( nNewColor ));
            assure( "Property LineColor",
                    AnyConverter.toInt( xProp.getPropertyValue( "LineColor" )) == nNewColor );
            float fNewCharHeight = (float)(16.0);
            xProp.setPropertyValue( "CharHeight", new Float( fNewCharHeight ));
            assure( "Property CharHeight",
                    AnyConverter.toFloat( xProp.getPropertyValue( "CharHeight" )) == fNewCharHeight );

            int nNewTextRotation = 700; // in 1/100 degrees
            xProp.setPropertyValue( "TextRotation", new Integer( nNewTextRotation ));
            assure( "Property TextRotation",
                    AnyConverter.toInt( xProp.getPropertyValue( "TextRotation" )) == nNewTextRotation );
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }

    // ------------

    public void testLegend()
    {
        XShape xLegend = mxOldDoc.getLegend();
        assure( "No Legend returned", xLegend != null );

        XPropertySet xLegendProp = (XPropertySet) UnoRuntime.queryInterface(
            XPropertySet.class, xLegend );
        assure( "Legend is no property set", xLegendProp != null );

        try
        {
            ChartLegendPosition eNewPos = ChartLegendPosition.BOTTOM;
            xLegendProp.setPropertyValue( "Alignment", eNewPos );
            assure( "Property Alignment",
                    AnyConverter.toObject(
                        new Type( ChartLegendPosition.class ),
                        xLegendProp.getPropertyValue( "Alignment" )) == eNewPos );
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }

    // ------------

    public void testArea()
    {
        XPropertySet xArea = mxOldDoc.getArea();
        assure( "No Area", xArea != null );

        try
        {
            int nColor = 0xf5fffa; // mint cream
            xArea.setPropertyValue( "FillColor", new Integer( nColor ) );
            xArea.setPropertyValue( "FillStyle", FillStyle.SOLID );

            int nNewColor = AnyConverter.toInt( xArea.getPropertyValue( "FillColor" ) );
            assure( "Changing FillColor of Area failed", nNewColor == nColor );
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }

    // ------------

    public void testChartType()
    {
        XMultiServiceFactory xFact = (XMultiServiceFactory) UnoRuntime.queryInterface(
            XMultiServiceFactory.class, mxOldDoc );
        assure( "document is no factory", xFact != null );

        try
        {
            String aMyServiceName = new String( "com.sun.star.chart.AreaDiagram" );
            String aServices[] = xFact.getAvailableServiceNames();
            boolean bServiceFound = false;
            for( int i = 0; i < aServices.length; ++i )
            {
                if( aServices[ i ].equals( aMyServiceName ))
                {
                    bServiceFound = true;
                    break;
                }
            }
            assure( "getAvailableServiceNames did not return " + aMyServiceName, bServiceFound );

            if( bServiceFound )
            {
                XDiagram xDia = (XDiagram) UnoRuntime.queryInterface(
                    XDiagram.class, xFact.createInstance( aMyServiceName ));
                assure( "AreaDiagram could not be created", xDia != null );

                mxOldDoc.setDiagram( xDia );
            }
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }

    // ================================================================================

    private XModel                    mxChartModel;
    private XChartDocument            mxOldDoc;

    // --------------------------------------------------------------------------------

    private void createFileDataSource( XModel xModel )
    {
        XMultiServiceFactory xFactory = (XMultiServiceFactory) param.getMSF();

        try
        {
            XDataProvider xDataProv = (XDataProvider) UnoRuntime.queryInterface(
                XDataProvider.class,
                xFactory.createInstance( "com.sun.star.comp.chart.FileDataProvider" ));

            drafts.com.sun.star.chart2.XChartDocument xDoc =
                (drafts.com.sun.star.chart2.XChartDocument) UnoRuntime.queryInterface(
                    drafts.com.sun.star.chart2.XChartDocument.class,
                    xModel );

            if( xDataProv != null &&
                xDoc != null )
            {
                xDoc.attachDataProvider( xDataProv );
                String aURL = "file://" + System.getProperty( "user.dir" ) +
                    System.getProperty( "file.separator" ) + "data.chd";
                log.println( aURL );
                xDoc.setRangeRepresentation( aURL );
            }
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }
    }

    // ------------

    private XModel createDocument( String sDocType )
    {
        XModel aResult = null;
        try
        {
            XComponentLoader aLoader = (XComponentLoader) UnoRuntime.queryInterface(
                XComponentLoader.class,
                ((XMultiServiceFactory)param.getMSF()).createInstance( "com.sun.star.frame.Desktop" ) );

            aResult = (XModel) UnoRuntime.queryInterface(
                XModel.class,
                aLoader.loadComponentFromURL( "private:factory/" + sDocType,
                                              "_blank",
                                              0,
                                              new PropertyValue[ 0 ] ) );
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }

        return aResult;
    }

    // ------------

    public XModel createChartModel()
    {
        XModel aResult = null;
        try
        {
            aResult = (XModel) UnoRuntime.queryInterface(
                XModel.class,
                ((XMultiServiceFactory)param.getMSF()).createInstance( "com.sun.star.comp.chart2.ChartModel" ) );
        }
        catch( Exception ex )
        {
            failed( ex.getMessage() );
            ex.printStackTrace( (PrintWriter)log );
        }

        return aResult;
    }

    // ------------

    private XTitle setTitle( XTitled xTitleParent, String aTitle, String aId )
    {
        XTitle xTitle = null;

        if( xTitleParent != null )
            try
            {
                XMultiServiceFactory xFact = (XMultiServiceFactory)param.getMSF();
                XMultiComponentFactory xCompFact = (XMultiComponentFactory) UnoRuntime.queryInterface(
                    XMultiComponentFactory.class, xFact );

                XFormattedString[] aStrings = new XFormattedString[ 2 ];
                aStrings[0] = (XFormattedString) UnoRuntime.queryInterface(
                    XFormattedString.class,
                    xFact.createInstance(
                        "drafts.com.sun.star.chart2.FormattedString" ));
                aStrings[1] = (XFormattedString) UnoRuntime.queryInterface(
                    XFormattedString.class,
                    xFact.createInstance(
                        "drafts.com.sun.star.chart2.FormattedString" ));
                aStrings[0].setString( aTitle );
                aStrings[1].setString( " Title" );

                Hashtable aParams = new Hashtable();
                aParams.put( "Identifier", aId );

                xTitle = (XTitle) UnoRuntime.queryInterface(
                    XTitle.class,
                    xCompFact.createInstanceWithContext(
                        "drafts.com.sun.star.chart2.Title",
                        new ComponentContext( aParams, getComponentContext( xFact ) )));

                xTitle.setText( aStrings );
                XPropertySet xTitleProp = (XPropertySet) UnoRuntime.queryInterface(
                    XPropertySet.class, xTitle );
                xTitleProp.setPropertyValue( "FillColor", new Integer( 0xfff8dc )); // cornsilk1
                xTitleParent.setTitle( xTitle );
            }
            catch( Exception ex )
            {
                failed( ex.getMessage() );
                ex.printStackTrace( (PrintWriter)log );
            }

        return xTitle;
    }

    // ------------

    private XComponentContext getComponentContext( XMultiServiceFactory xFact )
    {
        XComponentContext xResult = null;

        XPropertySet xProp = (XPropertySet) UnoRuntime.queryInterface(
            XPropertySet.class, xFact );
        if( xProp != null )
            try
            {
                xResult = (XComponentContext)
                    AnyConverter.toObject(
                        new Type( XComponentContext.class ),
                        xProp.getPropertyValue( "DefaultContext" ) );
            }
            catch( Exception ex )
            {
                failed( ex.getMessage() );
                ex.printStackTrace( (PrintWriter)log );
            }

        return xResult;
    }

    // ------------

    private void printInterfacesAndServices( Object oObj )
    {
        log.println( "Services:" );
        util.dbg.getSuppServices( oObj );
        log.println( "Interfaces:" );
        util.dbg.printInterfaces( (XInterface)oObj, true );
    }

    // ------------

    /// see rtl/math.hxx
    private boolean approxEqual( double a, double b )
    {
        if( a == b )
            return true;
        double x = a - b;
        return (x < 0.0 ? -x : x)
            < ((a < 0.0 ? -a : a) * (1.0 / (16777216.0 * 16777216.0)));
    }
}

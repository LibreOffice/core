// __________ Imports __________


// base classes
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.*;

// factory for creating components
import com.sun.star.lang.XMultiServiceFactory;

// property access
import com.sun.star.beans.*;

// container access
import com.sun.star.container.*;

// application specific classes
import com.sun.star.chart.XChartDocument;
import com.sun.star.chart.XDiagram;
import com.sun.star.drawing.*;
import com.sun.star.frame.XModel;

// base graphics things
import com.sun.star.awt.Point;
import com.sun.star.awt.Size;

// Exceptions
import com.sun.star.uno.RuntimeException;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.lang.IndexOutOfBoundsException;

// __________ Implementation __________

/** Helper for creating an OLE chart
    @author Bj&ouml;rn Milcke
 */
public class ChartHelper
{
    public ChartHelper( XModel aContainerDoc )
    {
        maContainerDocument = aContainerDoc;
    }

    // ____________________

    public XChartDocument insertOLEChart(
        String sChartName,
        Point  aUpperLeft,
        Size   aExtent,
        String sChartServiceName )
    {
        XChartDocument aResult = null;

        XShapes aPage = null;

        // try interface for multiple pages in a document
        XDrawPagesSupplier aSupplier = (XDrawPagesSupplier) UnoRuntime.queryInterface(
            XDrawPagesSupplier.class, maContainerDocument );

        if( aSupplier != null )
        {
            try
            {
                // get first page
                aPage = (XShapes) UnoRuntime.queryInterface(
                    XShapes.class, aSupplier.getDrawPages().getByIndex( 0 ) );
            }
            catch( Exception ex )
            {
                System.out.println( "First page not found in shape collection: " + ex );
            }
        }
        else
        {
            // try interface for single draw page (e.g. spreadsheet)
            XDrawPageSupplier aOnePageSupplier = (XDrawPageSupplier) UnoRuntime.queryInterface(
                XDrawPageSupplier.class, maContainerDocument );

            if( aOnePageSupplier != null )
            {
                aPage = (XShapes) UnoRuntime.queryInterface(
                    XShapes.class, aOnePageSupplier.getDrawPage());
            }
        }

        if( aPage != null )
        {
            XMultiServiceFactory aFact = (XMultiServiceFactory) UnoRuntime.queryInterface(
                XMultiServiceFactory.class, maContainerDocument );

            if( aFact != null )
            {
                try
                {
                    // create an OLE shape
                    XShape aShape = (XShape) UnoRuntime.queryInterface(
                        XShape.class,
                        aFact.createInstance( "com.sun.star.drawing.OLE2Shape" ));

                    // insert the shape into the page
                    aPage.add( aShape );
                    aShape.setPosition( aUpperLeft );
                    aShape.setSize( aExtent );

                    // make the OLE shape a chart
                    XPropertySet aShapeProp = (XPropertySet) UnoRuntime.queryInterface(
                        XPropertySet.class, aShape );
                    if( aShapeProp != null )
                    {
                        // set the class id for charts
                        aShapeProp.setPropertyValue( "CLSID", msChartClassID );

                        // retrieve the chart document as model of the OLE shape
                        aResult = (XChartDocument) UnoRuntime.queryInterface(
                            XChartDocument.class,
                            aShapeProp.getPropertyValue( "Model" ));

                        // create a diagram via the factory and set this as new diagram
                        aResult.setDiagram(
                            (XDiagram) UnoRuntime.queryInterface(
                                XDiagram.class,
                                ((XMultiServiceFactory) UnoRuntime.queryInterface(
                                    XMultiServiceFactory.class,
                                    aResult )).createInstance( sChartServiceName )));
                    }
                }
                catch( Exception ex )
                {
                    System.out.println( "Couldn't change the OLE shape into a chart: " + ex );
                }
            }
        }

        return aResult;
    }


    // __________ private members and methods __________

    private final String  msChartClassID   = "12dcae26-281f-416f-a234-c3086127382e";

    private XModel maContainerDocument;
}

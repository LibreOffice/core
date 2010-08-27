package complex.checkColor;

import com.sun.star.awt.Rectangle;
import com.sun.star.awt.Size;
import com.sun.star.awt.WindowDescriptor;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.drawing.XControlShape;
import com.sun.star.drawing.XShape;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XController;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.view.XControlAccess;
import complexlib.ComplexTestCase;
import java.io.PrintWriter;
import util.FormTools;
import util.SOfficeFactory;
import util.WriterTools;

/**
 * Created because of complaint on dev@openoffice.org: check the changing of
 * BackColor and IsLandscape properties on the PageStyle service.
 */
public class CheckChangeColor extends ComplexTestCase {

    private XToolkit xToolkit = null;
    private XWindowPeer xWinPeer = null;

    /**
     * Get all test method names.
     * @return The test methods.
     */
    public String[] getTestMethodNames() {
        return new String[] {"checkChangeColor"};
    }


    /**
     * Check BackColor and IsLandscape properties, wait for an exception: test
     * is ok if no exception happened.
     */
    public void checkChangeColor() {
        try {
            XMultiServiceFactory m_xMSF_ =  (XMultiServiceFactory)param.getMSF();
            XComponentLoader aLoader = (XComponentLoader)UnoRuntime.queryInterface(XComponentLoader.class,m_xMSF_.createInstance( "com.sun.star.frame.Desktop" ));
            XComponent xDocument = (XComponent)UnoRuntime.queryInterface(XComponent.class, aLoader.loadComponentFromURL( "private:factory/swriter", "_blank", 0, new PropertyValue[ 0 ] ) );
    //        xDocument.addEventListener( this );

            XTextDocument oDoc = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, xDocument);
            XMultiServiceFactory oDocMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, oDoc );

            // XInterface xInterface = (XInterface) oDocMSF.createInstance( "com.sun.star.style.PageStyle" );

            // create a supplier to get the Style family collection
            XStyleFamiliesSupplier xSupplier = ( XStyleFamiliesSupplier ) UnoRuntime.queryInterface(XStyleFamiliesSupplier.class, oDoc );

            // get the NameAccess interface from the Style family collection
            XNameAccess xNameAccess = xSupplier.getStyleFamilies();

            XNameContainer xPageStyleCollection = (XNameContainer) UnoRuntime.queryInterface(XNameContainer.class, xNameAccess.getByName( "PageStyles" ));

            // create a PropertySet to set the properties for the new Pagestyle
            XPropertySet xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xPageStyleCollection.getByName("Standard") );

            log.println("BackColor @ "+xPropertySet.getPropertyValue("BackColor").toString());
            log.println("IsLandscape @ "+xPropertySet.getPropertyValue("IsLandscape").toString());
            log.println("Size @ H:"+((Size)xPropertySet.getPropertyValue("Size")).Height+" W:"+((Size)xPropertySet.getPropertyValue("Size")).Width);

            log.println("Set Landscape");
            xPropertySet.setPropertyValue("IsLandscape",new Boolean(true) );
            log.println("Set BackColor");
            xPropertySet.setPropertyValue("BackColor",new Integer((int)255000000) );
        }
        catch(Exception e) {
            e.printStackTrace();
            failed("Exception.");
        }
    }
}

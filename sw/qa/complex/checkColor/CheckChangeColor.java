package complex.checkColor;

import com.sun.star.awt.Size;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.Any;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import util.DesktopTools;
import util.SOfficeFactory;
import static org.junit.Assert.*;

/**
 * Created because of complaint on dev@openoffice.org: check the changing of
 * BackColor and IsLandscape properties on the PageStyle service.
 */
public class CheckChangeColor {
    /**
     * Check BackColor and IsLandscape properties, wait for an exception: test
     * is ok if no exception happened.
     */
    @Test public void checkChangeColor() throws Exception {
        // create a supplier to get the Style family collection
        XStyleFamiliesSupplier xSupplier = ( XStyleFamiliesSupplier ) UnoRuntime.queryInterface(XStyleFamiliesSupplier.class, document);

        // get the NameAccess interface from the Style family collection
        XNameAccess xNameAccess = xSupplier.getStyleFamilies();

        XNameContainer xPageStyleCollection = (XNameContainer) UnoRuntime.queryInterface(XNameContainer.class, xNameAccess.getByName( "PageStyles" ));

        // create a PropertySet to set the properties for the new Pagestyle
        XPropertySet xPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xPageStyleCollection.getByName("Standard") );

        assertEquals(
            "BackColor", new Any(Type.LONG, 0xFFFFFFFF),
            Any.complete(xPropertySet.getPropertyValue("BackColor")));
        assertEquals(
            "IsLandscape", new Any(Type.BOOLEAN, false),
            Any.complete(xPropertySet.getPropertyValue("IsLandscape")));
        assertEquals(
            "Size", new Type(Size.class),
            Any.complete(xPropertySet.getPropertyValue("Size")).getType());

        xPropertySet.setPropertyValue("BackColor", 0xFF000000);
        xPropertySet.setPropertyValue("IsLandscape", true);
        assertEquals(
            "BackColor", new Any(Type.LONG, 0xFF000000),
            Any.complete(xPropertySet.getPropertyValue("BackColor")));
        assertEquals(
            "IsLandscape", new Any(Type.BOOLEAN, true),
            Any.complete(xPropertySet.getPropertyValue("IsLandscape")));
    }

    @Before public void setUpDocument() throws com.sun.star.uno.Exception {
        document = SOfficeFactory.getFactory(
            UnoRuntime.queryInterface(
                XMultiServiceFactory.class,
                connection.getComponentContext().getServiceManager())).
            createTextDoc(null);
    }

    @After public void tearDownDocument() {
        DesktopTools.closeDoc(document);
    }

    private XTextDocument document = null;

    @BeforeClass public static void setUpConnection() throws Exception {
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();
}

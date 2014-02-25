package complex.sfx2;

import org.openoffice.test.OfficeConnection;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XComponentContext;
import org.junit.AfterClass;
import org.junit.BeforeClass;

public class JUnitBasedTest
{

    protected XComponentContext getContext()
    {
        return m_connection.getComponentContext();
    }


    protected XMultiServiceFactory getORB()
    {
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface(
            XMultiServiceFactory.class, getContext().getServiceManager() );
        return xMSF1;
    }


    @BeforeClass
    public static void setUpConnection() throws Exception
    {
        System.out.println( "--------------------------------------------------------------------------------" );
        System.out.println( "connecting ..." );
        m_connection.setUp();
    }


    @AfterClass
    public static void tearDownConnection() throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println();
        System.out.println( "tearing down connection" );
        m_connection.tearDown();
        System.out.println( "--------------------------------------------------------------------------------" );
    }

    private static final OfficeConnection   m_connection = new OfficeConnection();
}

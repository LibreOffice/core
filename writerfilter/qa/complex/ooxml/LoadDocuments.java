
package complex.ooxml;

import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import complexlib.ComplexTestCase;
import java.io.File;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.Argument;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author hb137859
 */
public class LoadDocuments {
    @Test public void test1() {
        String testDocumentsPath = Argument.get("tdoc");
        System.out.println("Test documents in:" + testDocumentsPath);

        File dir = new File(testDocumentsPath);
        String [] files = dir.list();

        try {
            if (files != null) {
                for (int i = 0; i < files.length; ++i) {
                    System.out.println(files[i]);
                    String url = TestDocument.getUrl(files[i]);
                    System.out.println(url);

                    XComponent xDoc = util.DesktopTools.loadDoc(getMSF(), url, null);
                    System.out.println("loaded.");
                    util.DesktopTools.closeDoc(xDoc);
                    System.out.println("done.");
                }
            } else {
                fail("Files not found");
            }
        }
        catch (Exception e) {
            System.out.println(e);
            fail("failed");
        }
    }

    private XMultiServiceFactory getMSF()
    {
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
        return xMSF1;
    }

    // setup and close connections
    @BeforeClass public static void setUpConnection() throws Exception
    {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
            throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }
    private static final OfficeConnection connection = new OfficeConnection();
}

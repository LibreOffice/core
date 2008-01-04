package complex.tempfile;

import complexlib.ComplexTestCase;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.io.*;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.UnoRuntime;
import java.util.Random;
import share.LogWriter;

public class Test02 implements TempFileTest {

    XMultiServiceFactory m_xMSF;
    XSimpleFileAccess m_xSFA;
    TestHelper m_aTestHelper;

    public Test02(XMultiServiceFactory xMSF, XSimpleFileAccess xSFA, LogWriter aLogWriter) {
        m_xMSF = xMSF;
        m_xSFA = xSFA;
        m_aTestHelper = new TestHelper(aLogWriter, "Test02: ");
    }

    public boolean test() {
        Object oTempFile = null;
        XTempFile xTempFile = null;
        XTruncate xTruncate = null;
        String sFileURL = null;
        String sFileName = null;
        //create a temporary file.
        try {
            oTempFile = m_xMSF.createInstance( "com.sun.star.io.TempFile" );
            xTempFile = (XTempFile) UnoRuntime.queryInterface( XTempFile.class,
                    oTempFile );
            m_aTestHelper.Message( "Tempfile created." );
            xTruncate = (XTruncate)UnoRuntime.queryInterface( XTruncate.class,
                    oTempFile );
        } catch(Exception e) {
            m_aTestHelper.Error( "Cannot create TempFile. exception: " + e );
            return false;
        }
        try {
            //write something.
            byte pBytesIn[] = new byte[9];
            byte pBytesOut[][] = new byte[1][9];
            Random oRandom = new Random();
            oRandom.nextBytes( pBytesIn );
            m_aTestHelper.WriteBytesWithStream( pBytesIn, xTempFile );

            //get the URL.
            sFileURL = m_aTestHelper.GetTempFileURL( xTempFile );

            //let the service not to remove the URL.
            m_aTestHelper.SetTempFileRemove( xTempFile, false );

            //close the tempfile by closing input and output.
            m_aTestHelper.CloseTempFile( xTempFile );

            //check that the file is still available.
            //xTempFile.seek(0);
            m_aTestHelper.ReadDirectlyFromTempFile( pBytesOut, pBytesIn.length + 1, m_xSFA, sFileURL );
            for ( int i = 0; i < pBytesIn.length; i++ ) {
                if ( pBytesOut[0][i] != pBytesIn[i] ) {
                    m_aTestHelper.Error( "Tempfile contains false data!" );
                }
            }
        } catch ( Exception e) {
            m_aTestHelper.Error("Exception: " + e);
            return false;
        }
        return true;
    }
}
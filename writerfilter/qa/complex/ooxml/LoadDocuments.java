
package complex.ooxml;
import com.sun.star.lang.XMultiServiceFactory;
import complexlib.ComplexTestCase;
import java.io.File;
import com.sun.star.text.XTextDocument;

/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author hb137859
 */
public class LoadDocuments extends ComplexTestCase {
    private XMultiServiceFactory m_xMSF;

    @Override
    public String[] getTestMethodNames() {
        return new String [] {
            "test1"
        };
    }

    public void before() throws Exception {
        m_xMSF = (XMultiServiceFactory) param.getMSF();
    }

    public void test1() {
        String testDocumentsPath = util.utils.getFullTestDocName("");
        log.println("Test documents in:" + testDocumentsPath);

        File dir = new File(testDocumentsPath);
        String [] files = dir.list();

        if (files != null) {
            for (int i = 0; i < files.length; ++i) {
                log.println(files[i]);
                String url = util.utils.getFullTestURL(files[i]);
                log.println(url);

                XTextDocument xDoc = util.WriterTools.loadTextDoc(m_xMSF, url);
                util.DesktopTools.closeDoc(xDoc);
            }
        } else {
            failed();
        }
    }
}

package complex.sfx2.undo;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Exception;
import org.openoffice.test.tools.DocumentType;
import org.openoffice.test.tools.OfficeDocument;

/**
 * @author frank.schoenheit@oracle.com
 */
abstract class DocumentTestBase implements DocumentTest
{
    DocumentTestBase( final XMultiServiceFactory i_orb, final DocumentType i_docType ) throws Exception
    {
        m_document = OfficeDocument.blankDocument( i_orb, i_docType );
    }

    public OfficeDocument getDocument()
    {
        return m_document;
    }

    public void closeDocument()
    {
        m_document.close();
    }

    protected final OfficeDocument  m_document;
}

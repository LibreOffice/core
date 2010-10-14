package complex.sfx2.undo;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Exception;
import org.openoffice.test.tools.DocumentType;
import org.openoffice.test.tools.OfficeDocument;

/**
 * @author frank.schoenheit@oracle.com
 */
class DocumentTestBase
{
    DocumentTestBase( final XMultiServiceFactory i_orb, final DocumentType i_docType ) throws Exception
    {
        m_document = OfficeDocument.blankDocument( i_orb, i_docType );
    }

    protected final OfficeDocument  m_document;
}

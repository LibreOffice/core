package complex.sfx2.undo;

import org.openoffice.test.tools.OfficeDocument;

/**
 * wrapper around an OfficeDocument, for running a standardized test procedure (related do Undo functionality)
 * on the document.
 *
 * @author frank.schoenheit@oracle.com
 */
public interface DocumentTest
{
    /**
     * returns a human-readable description for the document/type which the tests operates on
     */
    public String getDocumentDescription();

    /**
     * initializes the document to a state where the subsequent tests can be ran
     */
    public void initializeDocument() throws com.sun.star.uno.Exception;

    /**
     * closes the document which the test is based on
     */
    public void closeDocument();

    /**
     * does a simple modification to the document, which results in one Undo action being auto-generated
     * by the OOo implementation
     */
    public void doSingleModification() throws com.sun.star.uno.Exception;

    /**
     * verifies the document is in the same state as after {@link #initializeDocument}
     */
    public void verifyInitialDocumentState() throws com.sun.star.uno.Exception;

    /**
     * verifies the document is in the state as expected after {@link #doSingleModification}
     * @throws com.sun.star.uno.Exception
     */
    public void verifySingleModificationDocumentState() throws com.sun.star.uno.Exception;

    /**
     * does multiple modifications do the document, which would normally result in multiple Undo actions.
     *
     * The test framework will encapsulate the call into an {@link XUndoManager.enterUndoContext()} and
     * {@link XUndoManager.leaveUndoContext()} call.
     *
     * @return
     *  the number of modifications done to the document. The caller assumes (and asserts) that the number
     *  of actions on the Undo stack equals this number.
     */
    public int doMultipleModifications() throws com.sun.star.uno.Exception;

    /**
     * returns the document which the test operates on
     */
    public OfficeDocument   getDocument();
}

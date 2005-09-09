/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwXFootnoteProperties.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:47:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package mod._sw;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XFootnote;
import com.sun.star.text.XFootnotesSupplier;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;


/**
 * Test for the object, which is represented as set of footnote properties
 * (instance of <code>com.sun.star.text.FootnoteSettings</code> service).
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::text::FootnoteSettings</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 * @see com.sun.star.text.FootnoteSettings
 * @see ifc.text._FootnoteSettings
 */
public class SwXFootnoteProperties extends TestCase {
    XTextDocument xTextDoc;

    /**
    * Creates text document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );
        try {
            log.println( "creating a textdocument" );
            xTextDoc = SOF.createTextDoc( null );
        } catch ( com.sun.star.uno.Exception e ) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
    * Disposes text document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        util.DesktopTools.closeDoc(xTextDoc);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.text.Footnote</code>. Then inserts created Footnote
    * to the text document, and finally gets footnote settings from text
    * document through <code>XFootnotesSupplier</code> interface.<br>
    */
    public synchronized TestEnvironment createTestEnvironment(
            TestParameters Param, PrintWriter log ) throws StatusException {
        XFootnotesSupplier oInterface = null;
        XInterface oObj = null;
        XFootnote oFootnote;

        log.println( "Creating a test environment" );
        // get a soffice factory object
        XMultiServiceFactory msf = (XMultiServiceFactory)
            UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDoc);
        log.println("creating a footnote");

        try {
            oFootnote = (XFootnote) UnoRuntime.queryInterface(XFootnote.class,
                    msf.createInstance("com.sun.star.text.Footnote"));
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create footnote", e);
        }

        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();

        log.println("inserting the footnote into text document");
        try {
            oText.insertTextContent(oCursor, oFootnote, false);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't insert the footnote", e);
        }

        oInterface = (XFootnotesSupplier)
            UnoRuntime.queryInterface(XFootnotesSupplier.class, xTextDoc);
        oObj = oInterface.getFootnoteSettings();

        TestEnvironment tEnv = new TestEnvironment(oObj);
        return tEnv;
    }

}


/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DocumentProperties.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:18:17 $
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

package mod._fwk;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.document.XDocumentInfoSupplier;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.document.DocumentInfo</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li><code>com::sun::star::beans::XFastPropertySet</code></li>
*  <li><code>com::sun::star::beans::XPropertySet</code></li>
*  <li><code>com::sun::star::document::XDocumentInfo</code></li>
*  <li><code>com::sun::star::document::XStandaloneDocumentInfo</code></li>
*  <li><code>com::sun::star::lang::XComponent</code></li>
* </ul><p>
* @see com.sun.star.beans.XFastPropertySet
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.document.XDocumentInfo
* @see com.sun.star.document.XStandaloneDocumentInfo
* @see com.sun.star.lang.XComponent
* @see ifc.beans._XFastPropertySet
* @see ifc.beans._XPropertySet
* @see ifc.document._XDocumentInfo
* @see ifc.document._XStandaloneDocumentInfo
* @see ifc.lang._XComponent
*/
public class DocumentProperties extends TestCase {

    XTextDocument xTextDoc;

    /**
     * Disposes the document, if exists, created in
     * <code>createTestEnvironment</code> method.
     */
    protected void cleanup( TestParameters Param, PrintWriter log) {

        log.println("disposing xTextDoc");

        if (xTextDoc != null) {
            xTextDoc.dispose();
        }
    }

    /**
    * Creates a text document.
    * Obtains the property <code>'DocumentInfo'</code> of the created document.
    */
    public TestEnvironment createTestEnvironment( TestParameters Param,
        PrintWriter log ) throws StatusException {

        XInterface oObj = null;

        log.println( "creating a test environment" );

        if (xTextDoc != null) xTextDoc.dispose();

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF());

        try {
            log.println( "creating a text document" );
            xTextDoc = SOF.createTextDoc(null);
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }

        shortWait();

        XDocumentInfoSupplier xdis = (XDocumentInfoSupplier)
                UnoRuntime.queryInterface(XDocumentInfoSupplier.class, xTextDoc);
        //oObj = (XInterface)UnoRuntime.queryInterface(XInterface.class, docInfo);
        oObj = xdis.getDocumentInfo();
        TestEnvironment tEnv = new TestEnvironment( oObj );

        return tEnv;
    } // finish method getTestEnvironment

    /**
    * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(500) ;
        } catch (InterruptedException e) {
            log.println("While waiting :" + e) ;
        }
    }
}

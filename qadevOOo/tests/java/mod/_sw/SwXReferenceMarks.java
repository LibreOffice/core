/*************************************************************************
 *
 *  $RCSfile: SwXReferenceMarks.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 12:48:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package mod._sw;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.container.XNamed;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XReferenceMarksSupplier;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.text.ReferenceMarks</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::container::XNameAccess</code></li>
 *  <li> <code>com::sun::star::container::XIndexAccess</code></li>
 *  <li> <code>com::sun::star::container::XElementAccess</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 * @see com.sun.star.container.XNameAccess
 * @see com.sun.star.container.XIndexAccess
 * @see com.sun.star.container.XElementAccess
 * @see com.sun.star.text.ReferenceMarks
 * @see ifc.container._XNameAccess
 * @see ifc.container._XIndexAccess
 * @see ifc.container._XElementAccess
 */
public class SwXReferenceMarks extends TestCase {
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
            throw new StatusException( "Couldn³t create document", e );
        }
    }

    /**
    * Disposes text document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        xTextDoc.dispose();
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instances of the service
    * <code>com.sun.star.text.ReferenceMark</code>, then sets new names to
    * created ReferenceMark's using <code>XNamed</code> interface. Then renamed
    * ReferenceMark's are inserted to a major text of text document. Finally,
    * ReferenceMarks are gotten from text document using
    * <code>XReferenceMarksSupplier</code> interface.
    */
    public synchronized TestEnvironment createTestEnvironment(
            TestParameters tParam, PrintWriter log ) throws StatusException {
        XInterface oObj = null;
        XText oText = null;
        String Name = "SwXReferenceMark01";
        String Name2 = "SwXReferenceMark02";

        log.println( "creating a test environment" );
        oText = xTextDoc.getText();

        XMultiServiceFactory oDocMSF = (XMultiServiceFactory)
            UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDoc);

        // Creation and insertion of ReferenceMark01
        try {
            oObj = (XInterface)
                oDocMSF.createInstance( "com.sun.star.text.ReferenceMark" );
        } catch ( com.sun.star.uno.Exception e ) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn´t get ReferenceMark", e);
        }
        XNamed oObjN = (XNamed) UnoRuntime.queryInterface(XNamed.class, oObj);
        oObjN.setName(Name);
        XTextContent oObjTC = (XTextContent)
            UnoRuntime.queryInterface(XTextContent.class, oObj);
        XTextCursor oCursor = oText.createTextCursor();
        try {
            oText.insertTextContent(oCursor, oObjTC, false);
        } catch ( com.sun.star.lang.IllegalArgumentException e ){
            e.printStackTrace( log );
            throw new StatusException(" Couldn't insert ReferenceMark01", e);
        }

        // Creation and insertion of ReferenceMark02
        try {
            oObj = (XInterface)
                oDocMSF.createInstance( "com.sun.star.text.ReferenceMark" );
        } catch ( com.sun.star.uno.Exception e ) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn´t get ReferenceMark", e);
        }
        XNamed oObjN2 = (XNamed) UnoRuntime.queryInterface(XNamed.class, oObj);
        oObjN2.setName(Name2);

        XTextContent oObjTC2 = (XTextContent)
            UnoRuntime.queryInterface(XTextContent.class, oObj);
        try {
            oText.insertTextContent(oCursor, oObjTC2, false);
        } catch ( com.sun.star.lang.IllegalArgumentException e ){
            e.printStackTrace( log );
            throw new StatusException(" Couldn't insert ReferenceMark02", e);
        }

        // getting ReferenceMarks from text document
        XReferenceMarksSupplier oRefSupp = (XReferenceMarksSupplier)
            UnoRuntime.queryInterface(XReferenceMarksSupplier.class, xTextDoc);
        oObj = oRefSupp.getReferenceMarks();

        TestEnvironment tEnv = new TestEnvironment( oObj );
        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SwXReferenceMarks


/*************************************************************************
 *
 *  $RCSfile: SwXFieldEnumeration.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 12:45:14 $
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

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XDependentTextField;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextFieldsSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;


/**
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::container::XEnumeration</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 * @see com.sun.star.container.XEnumeration
 * @see ifc.container._XEnumeration
 */
public class SwXFieldEnumeration extends TestCase {
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
    * Creating a Testenvironment for the interfaces to be tested. FieldMaster
    * object is created and attached to Bibliography database. After setting
    * properties of created FieldMaster object, it is attached to the TextField
    * object. Then TextField is inserted as TextContent to the text document.
    * At the end, TextFields of text document are obtained and
    * enumeration of these fields is created.<br>
    * Creates instances of the services:
    * <code>com.sun.star.text.FieldMaster.Database</code>,
    * <code>com.sun.star.text.TextField.Database</code>.<br>
    *     Object relations created :
    * <ul>
    *  <li> <code>'ENUM'</code> for
    *     {@link ifc.container._XEnumeration} : text fields</li>
    * </ul>
    */
    public TestEnvironment createTestEnvironment(
            TestParameters tParam, PrintWriter log ) throws StatusException {
        XInterface oObj = null;
        Object FieldMaster = null;
        XPropertySet PFieldMaster = null;
        XDependentTextField xTF = null;
        XEnumerationAccess xFEA = null;
        XText the_Text;
        XTextCursor the_Cursor;
        XTextContent the_Field;

        log.println( "creating a test environment" );
        XMultiServiceFactory oDocMSF = (XMultiServiceFactory)
            UnoRuntime.queryInterface( XMultiServiceFactory.class, xTextDoc );

        try {
            FieldMaster = oDocMSF.createInstance
                ( "com.sun.star.text.FieldMaster.Database" );
            PFieldMaster = (XPropertySet) UnoRuntime.queryInterface
                (XPropertySet.class,(XInterface) FieldMaster);
            oObj = (XInterface)
                oDocMSF.createInstance("com.sun.star.text.TextField.Database");
            xTF = (XDependentTextField)
                UnoRuntime.queryInterface(XDependentTextField.class,oObj);
        } catch ( com.sun.star.uno.Exception e ) {
            e.printStackTrace(log);
        }

        try {
            PFieldMaster.setPropertyValue("DataBaseName","Bibliography");
            PFieldMaster.setPropertyValue("DataTableName","biblio");
            PFieldMaster.setPropertyValue("DataColumnName","Address");
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            log.println("Error: can't set PropertyValue to a FieldMaster");
            e.printStackTrace(log);
        } catch ( com.sun.star.lang.IllegalArgumentException e ) {
            log.println("Error: can't set PropertyValue to a FieldMaster");
            e.printStackTrace(log);
        } catch ( com.sun.star.beans.UnknownPropertyException e ) {
            log.println("Error: can't set PropertyValue to a FieldMaster");
            e.printStackTrace(log);
        } catch ( com.sun.star.beans.PropertyVetoException e ) {
            log.println("Error: can't set PropertyValue to a FieldMaster");
            e.printStackTrace(log);
        }

        the_Text = xTextDoc.getText();
        the_Cursor = the_Text.createTextCursor();
        the_Field = (XTextContent)
            UnoRuntime.queryInterface(XTextContent.class, oObj);

        try {
            xTF.attachTextFieldMaster(PFieldMaster);
            the_Text.insertTextContent(the_Cursor,the_Field,false);
        } catch ( com.sun.star.lang.IllegalArgumentException e ) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't create TextField", e );
        }

        // create testobject here
        XTextFieldsSupplier oTFS = (XTextFieldsSupplier)
            UnoRuntime.queryInterface( XTextFieldsSupplier.class, xTextDoc );
        xFEA = oTFS.getTextFields();
        oObj = oTFS.getTextFields().createEnumeration();

        log.println( "creating a new environment for FieldEnumeration object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("ENUM", xFEA);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SwXFieldEnumeration


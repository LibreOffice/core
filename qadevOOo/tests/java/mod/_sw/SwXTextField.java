/*************************************************************************
 *
 *  $RCSfile: SwXTextField.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 12:51:16 $
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
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XDependentTextField;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 *
 * initial description
 * @see com.sun.star.lang.XComponent
 * @see com.sun.star.text.TextContent
 * @see com.sun.star.text.XTextContent
 * @see com.sun.star.text.XTextField
 *
 */
public class SwXTextField extends TestCase {

    XTextDocument xTextDoc;

    /**
     * in general this method creates a testdocument
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *
     *  @see TestParameters
     *    @see PrintWriter
     *
     */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            log.println( "creating a textdocument" );
            xTextDoc = SOF.createTextDoc( null );
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn³t create document", e );
        }
    }

    /**
     * in general this method disposes the testenvironment and document
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *
     *  @see TestParameters
     *    @see PrintWriter
     *
     */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        xTextDoc.dispose();
    }


    /**
     *    creating a Testenvironment for the interfaces to be tested
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *  @return    Status class
     *
     *  @see TestParameters
     *    @see PrintWriter
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {

        XInterface oObj = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        Object instance = null;

          // create testobject here
        try {
            XMultiServiceFactory oDocMSF = (XMultiServiceFactory)
                UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDoc);

            Object FieldMaster = oDocMSF.createInstance
                ( "com.sun.star.text.FieldMaster.Database" );
            XPropertySet PFieldMaster = (XPropertySet) UnoRuntime.queryInterface
                (XPropertySet.class,(XInterface) FieldMaster);
            oObj = (XInterface) oDocMSF.createInstance
                ( "com.sun.star.text.TextField.Database" );

            instance = (XInterface) oDocMSF.createInstance
                ( "com.sun.star.text.TextField.DateTime" );

            XDependentTextField xTF = (XDependentTextField)
                UnoRuntime.queryInterface(XDependentTextField.class,oObj);

            PFieldMaster.setPropertyValue("DataBaseName","Address Book File");
            PFieldMaster.setPropertyValue("DataTableName","address");
            PFieldMaster.setPropertyValue("DataColumnName","FIRSTNAME");
            XText the_Text = xTextDoc.getText();
            XTextCursor the_Cursor = the_Text.createTextCursor();
            XTextContent the_Field = (XTextContent)
                             UnoRuntime.queryInterface(XTextContent.class,oObj);

            xTF.attachTextFieldMaster(PFieldMaster);
            the_Text.insertTextContent(the_Cursor,the_Field,false);
        }
        catch (Exception ex) {
            log.println("Couldn't create instance");
            ex.printStackTrace(log);
        }

        log.println( "creating a new environment for FieldMaster object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );


        tEnv.addObjRelation("CONTENT", (XTextContent)
                        UnoRuntime.queryInterface(XTextContent.class,instance));
        tEnv.addObjRelation("RANGE", xTextDoc.getText().createTextCursor());

        return tEnv;
    } // finish method getTestEnvironment
}    // finish class SwXTextField


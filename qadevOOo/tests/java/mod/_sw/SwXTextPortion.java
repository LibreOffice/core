/*************************************************************************
 *
 *  $RCSfile: SwXTextPortion.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 12:52:44 $
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
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 *
 * initial description
 * @see com.sun.star.text.TextPortion
 *
 */
public class SwXTextPortion extends TestCase {
    XTextDocument xTextDoc;

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

    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        xTextDoc.dispose();
    }

    /**
     *  *    creating a Testenvironment for the interfaces to be tested
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *  @return    Status class
     *
     *  @see TestParameters
     *  *    @see PrintWriter
     */
    protected synchronized TestEnvironment createTestEnvironment
            (TestParameters tParam, PrintWriter log) {

        XInterface oObj = null;
        XInterface param = null;
        XPropertySet paraP = null;
        XPropertySet portP = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        // create testobject here

        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();

        log.println( "inserting Strings" );
        log.println( "inserting ControlCharacter" );

        try{
            for (int i =0; i < 5; i++){
                oText.insertString( oCursor,"Paragraph Number: " + i, false);
                oText.insertControlCharacter(
                    oCursor, ControlCharacter.LINE_BREAK, false );
                oText.insertString( oCursor,
                    "The quick brown fox jumps over the lazy Dog: SwXParagraph\n",
                    false);
                oText.insertControlCharacter(
                    oCursor, ControlCharacter.LINE_BREAK, false );
                oText.insertString( oCursor,
                    "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG: SwXParagraph",
                    false);
                oText.insertControlCharacter(
                    oCursor, ControlCharacter.PARAGRAPH_BREAK, false );
            }
        }catch(Exception e){
            log.println("Couldn't insert Text");
            e.printStackTrace(log);
            throw new StatusException( "Couldn't insert Text", e );
        }

        // Enumeration
        XEnumerationAccess oEnumA = (XEnumerationAccess)
            UnoRuntime.queryInterface( XEnumerationAccess.class, oText );
        XEnumeration oEnum = oEnumA.createEnumeration();

        int n = 0;
        while ( (oEnum.hasMoreElements()) ) {
            try {
                param = (XInterface) AnyConverter.toObject(
                        new Type(XInterface.class),oEnum.nextElement());
                log.println("Element Nr.: " + n );
            } catch ( Exception e) {
                log.println("Couldn't get Paragraph");
                e.printStackTrace(log);
                throw new StatusException( "Couldn't get Paragraph", e );
            }
            n++;
        }

        XEnumerationAccess oEnumP = (XEnumerationAccess)
            UnoRuntime.queryInterface( XEnumerationAccess.class, param );
        XEnumeration oEnum2 = oEnumP.createEnumeration();
        try {
            oObj = (XInterface)AnyConverter.toObject(
                        new Type(XInterface.class),oEnum2.nextElement());
        } catch ( Exception e) {
            log.println("Couldn't get TextPortion");
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get TextPortion", e );
        }

        portP = (XPropertySet)
                UnoRuntime.queryInterface(XPropertySet.class, oObj);
        paraP = (XPropertySet)
                UnoRuntime.queryInterface(XPropertySet.class, param);


        log.println( "creating a new environment for Paragraph object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        log.println("adding ObjRelation TRO for TextContent");
        tEnv.addObjRelation("TRO", new Boolean(true));

        log.println("adding ObjectRelation 'PARA' for CharacterProperties");
        tEnv.addObjRelation("PARA", paraP);

        log.println("adding ObjectRelation 'PORTION' for CharacterProperties");
        tEnv.addObjRelation("PORTION", portP);

        tEnv.addObjRelation("XTEXT", oText);

        return tEnv;
    } // finish method getTestEnvironment


}    // finish class SwXTextPortion


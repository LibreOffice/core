/*************************************************************************
 *
 *  $RCSfile: SwXTextDocument.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-05-27 13:50:13 $
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

import com.sun.star.container.XNamed;
import com.sun.star.frame.XController;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextTable;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.view.XSelectionSupplier;
import java.io.PrintWriter;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;
import util.WriterTools;

public class SwXTextDocument extends TestCase {
    XTextDocument xTextDoc;
    XTextDocument xSecondTextDoc;

    protected void initialize( TestParameters tParam, PrintWriter log ) {
    }

    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        xTextDoc.dispose();
        xSecondTextDoc.dispose();
    }

    /**
     *    creating a Testenvironment for the interfaces to be tested
     */

    public synchronized TestEnvironment createTestEnvironment
            ( TestParameters Param, PrintWriter log )
            throws StatusException {

        XInterface oObj = null;
        TestEnvironment tEnv = null;
        XTextTable the_table = null;

        if (xTextDoc != null) xTextDoc.dispose();
        if (xSecondTextDoc != null) xSecondTextDoc.dispose();

        try {
            log.println( "creating a textdocument" );
            xTextDoc = WriterTools.createTextDoc( (XMultiServiceFactory)Param.getMSF() );

            log.println( "    adding TextTable" );
            the_table = SOfficeFactory.createTextTable(xTextDoc,6,4);
            XNamed the_name = (XNamed) UnoRuntime.queryInterface
                (XNamed.class,the_table);
            the_name.setName("SwXTextDocument");
            SOfficeFactory.insertTextContent(xTextDoc,(XTextContent) the_table);

            log.println( "    adding ReferenceMark" );
            XMultiServiceFactory oDocMSF = (XMultiServiceFactory)
                UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDoc);
            XInterface aMark = (XInterface) oDocMSF.createInstance(
                                            "com.sun.star.text.ReferenceMark");
            the_name = (XNamed) UnoRuntime.queryInterface(XNamed.class,aMark);
            the_name.setName("SwXTextDocument");
            XTextContent oTC = (XTextContent)
                UnoRuntime.queryInterface(XTextContent.class, aMark);
            SOfficeFactory.insertTextContent(xTextDoc,(XTextContent) oTC);

            log.println( "    adding TextGraphic" );
            WriterTools.insertTextGraphic(xTextDoc,oDocMSF,5200,4200,4400,
                4000,"space-metal.jpg","SwXTextDocument");

            log.println( "    adding EndNote" );
            XInterface aEndNote = (XInterface) oDocMSF.createInstance(
                                            "com.sun.star.text.Endnote");
            oTC = (XTextContent) UnoRuntime.queryInterface
                (XTextContent.class, aEndNote);
            SOfficeFactory.insertTextContent(xTextDoc,(XTextContent) oTC);

            log.println( "creating a second textdocument" );
            xSecondTextDoc = WriterTools.createTextDoc( (XMultiServiceFactory)Param.getMSF() );

        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn³t create document", e );
        }
        if (xTextDoc != null) {
            log.println("Creating instance...");
            XText oText = xTextDoc.getText();
            XTextCursor oTextCursor = oText.createTextCursor();

            for (int i=0;i<11;i++) {
                oText.insertString(oTextCursor, "xTextDoc ", false);
            }

            tEnv = new TestEnvironment(xTextDoc);
        } else  {
            log.println("Failed to create instance.");
            return tEnv;
        }

        XModel model1 = (XModel)
                    UnoRuntime.queryInterface(XModel.class, xTextDoc);
        XModel model2 = (XModel)
                    UnoRuntime.queryInterface(XModel.class, xSecondTextDoc);

        XController cont1 = model1.getCurrentController();
        XController cont2 = model2.getCurrentController();

        XSelectionSupplier sel = (XSelectionSupplier)
                    UnoRuntime.queryInterface(XSelectionSupplier.class,cont1);

        log.println( "Adding SelectionSupplier and Shape to select for XModel");
        tEnv.addObjRelation("SELSUPP",sel);
        tEnv.addObjRelation("TOSELECT",the_table);

        log.println( "adding Controller as ObjRelation for XModel");
        tEnv.addObjRelation("CONT1",cont1);
        tEnv.addObjRelation("CONT2",cont2);

        return tEnv;
    }


}    // finish class SwXTextDocument


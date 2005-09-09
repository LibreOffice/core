/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwXTextSections.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 04:01:39 $
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

import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextSectionsSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 *
 * initial description
 * @see com.sun.star.text.XText
 *
 */
public class SwXTextSections extends TestCase {
    XTextDocument xTextDoc;

    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            log.println( "creating a textdocument" );
            xTextDoc = SOF.createTextDoc( null );
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        util.DesktopTools.closeDoc(xTextDoc);
    }

    /**
     *    creating a Testenvironment for the interfaces to be tested
     */
    public synchronized TestEnvironment createTestEnvironment
        (TestParameters Param, PrintWriter log ) throws StatusException {

        XInterface oObj = null;
        XInterface oTS = null;
        XNameAccess oTSSuppName = null;

        XText oText = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );


        oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();


        log.println( "inserting TextSections" );

        XMultiServiceFactory oDocMSF = (XMultiServiceFactory)
            UnoRuntime.queryInterface(XMultiServiceFactory.class, xTextDoc);

        // First TextSection
        try {
            oTS = (XInterface) oDocMSF.createInstance
                ("com.sun.star.text.TextSection");
            XTextContent oTSC = (XTextContent)
                UnoRuntime.queryInterface(XTextContent.class, oTS);
            oText.insertTextContent(oCursor, oTSC, false);
        }
        catch(Exception e){
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }

        // Second TextSection
        try {
            oTS = (XInterface) oDocMSF.createInstance
                ("com.sun.star.text.TextSection");
            XTextContent oTSC = (XTextContent)
                UnoRuntime.queryInterface(XTextContent.class, oTS);
            oText.insertTextContent(oCursor, oTSC, false);
        }
        catch(Exception e){
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }


        log.println( "try to get a TextSection with the XTextSectionSupplier()" );

        try{
            XTextSectionsSupplier oTSSupp = (XTextSectionsSupplier)
                UnoRuntime.queryInterface( XTextSectionsSupplier.class,
                xTextDoc );
            oTSSuppName = oTSSupp.getTextSections();
        }
        catch(Exception e){
            System.out.println("Couldn't get Textsection " + e);
        }


        oObj = oTSSuppName;

        log.println( "creating a new environment for TextSections object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        log.println( "adding TextDocument as mod relation to environment" );
        tEnv.addObjRelation("TEXTDOC", xTextDoc);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SwXTextSection

/*************************************************************************
 *
 *  $RCSfile: Toolkit.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 13:03:16 $
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

package mod._toolkit;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.FormTools;
import util.SOfficeFactory;
import util.WriterTools;
import util.utils;

import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.drawing.XControlShape;
import com.sun.star.drawing.XShape;
import com.sun.star.frame.XController;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.view.XControlAccess;

/**
* Test for <code>com.sun.star.awt.Toolkit</code> service.
*/
public class Toolkit extends TestCase {

    XTextDocument xTextDoc;

    protected void initialize ( TestParameters Param, PrintWriter log) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF() );

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
        xTextDoc.dispose();
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates <code>com.sun.star.awt.Toolkit</code> service.
    */
    public TestEnvironment createTestEnvironment( TestParameters Param,
                                                  PrintWriter log )
                                                    throws StatusException {
        XInterface oObj = null;
        XWindowPeer the_win = null;
        XWindow win = null ;
        //Insert a ControlShape and get the ControlModel
        XControlShape aShape = FormTools.createControlShape(
                                xTextDoc,3000,4500,15000,10000,"CommandButton");

        WriterTools.getDrawPage(xTextDoc).add((XShape) aShape);

        XControlModel the_Model = aShape.getControl();

        //Try to query XControlAccess
        XControlAccess the_access = (XControlAccess) UnoRuntime.queryInterface(
                        XControlAccess.class,xTextDoc.getCurrentController());
        XController cntrlr = (XController) UnoRuntime.queryInterface(
                        XController.class,xTextDoc.getCurrentController());

        //now get the toolkit
        try {
            win = cntrlr.getFrame().getContainerWindow();
            //win = (XWindow) UnoRuntime.queryInterface(XWindow.class, ctrl) ;
            the_win = the_access.getControl(the_Model).getPeer();
            oObj = (XInterface) ((XMultiServiceFactory)Param.getMSF()).createInstance
                ("com.sun.star.awt.Toolkit") ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get toolkit");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get toolkit", e );
        }

        log.println( "    creating a new environment for toolkit object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        log.println("Implementation Name: "+utils.getImplName(oObj));

        tEnv.addObjRelation("WINPEER",the_win);

        // adding relation for XDataTransferProviderAccess
        tEnv.addObjRelation("XDataTransferProviderAccess.XWindow", win) ;

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class Toolkit


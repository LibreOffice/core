/*************************************************************************
 *
 *  $RCSfile: ScDrawPageObj.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 12:08:56 $
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

package mod._sc;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DrawTools;
import util.InstCreator;
import util.SOfficeFactory;
import util.ShapeDsc;

import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XShape;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class ScDrawPageObj extends TestCase {

    XSpreadsheetDocument xDoc = null;

    /**
     * Creates a new Draw document.
     */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());

        try {
            log.println( "creating a sheetdocument" );
            xDoc = SOF.createCalcDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
     * Disposes the Draw document created before
     */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent xComp = (XComponent)
                            UnoRuntime.queryInterface(XComponent.class, xDoc);
        xComp.dispose();
    }


    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * From the Calc document created a collection of its draw
     * pages is obtained. Two new pages are inserted. And one
     * page is obtained as a testing component. A shape is added
     * to this page. <p>
     *
     *     Object relations created :
     * <ul>
     *  <li> <code>'DrawPage'</code> for
     *      {@link ifc.drawing._XShapeGrouper} :
     *      the draw page tested. </li>
     *  <li> <code>'Shape'</code> for
     *      {@link ifc.drawing._XShapes} :
     *      the creator which can create instances of
     *      <code>com.sun.star.drawing.Line</code> service </li>
     * </ul>
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {

        XInterface oObj = null;
        XShape oShape = null ;
        XDrawPages oDP = null;

        XComponent xComp = (XComponent)
                            UnoRuntime.queryInterface(XComponent.class, xDoc);

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );
        try {
            log.println( "getting Drawpages" );
            XDrawPagesSupplier oDPS = (XDrawPagesSupplier)
                UnoRuntime.queryInterface(XDrawPagesSupplier.class,xDoc);
            oDP = (XDrawPages) oDPS.getDrawPages();
            oDP.insertNewByIndex(1);
            oDP.insertNewByIndex(2);
            oObj = (XDrawPage) AnyConverter.toObject(
                    new Type(XDrawPage.class),oDP.getByIndex(0));

            SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());

            oShape = SOF.createShape(xComp,5000,3500,7500,5000,"Rectangle");
            DrawTools.getShapes((XDrawPage) oObj).add(oShape);
            XShape oShape1 = SOF.createShape(xComp,
                5000,5500,5000,5000,"Rectangle");
            DrawTools.getShapes((XDrawPage) oObj).add(oShape1);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Couldn't create insance");
            e.printStackTrace(log);
            throw new StatusException("Can't create enviroment", e) ;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Couldn't create insance");
            e.printStackTrace(log);
            throw new StatusException("Can't create enviroment", e) ;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Couldn't create insance");
            e.printStackTrace(log);
            throw new StatusException("Can't create enviroment", e) ;
        }

        // create test environment here
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // relation for XShapes interface
        ShapeDsc sDsc = new ShapeDsc(5000,3500,7500,10000,"Line");
        tEnv.addObjRelation("Shape", new InstCreator(xDoc, sDsc)) ;

        log.println("ImplementationName: "+util.utils.getImplName(oObj));

        // adding relation for XShapeGrouper
        tEnv.addObjRelation("DrawPage", oObj);

        return tEnv;
    } // finish method getTestEnvironment

}


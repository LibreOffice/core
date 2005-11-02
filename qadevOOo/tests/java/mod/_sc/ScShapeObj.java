/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ScShapeObj.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 18:07:34 $
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

package mod._sc;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DefaultDsc;
import util.DrawTools;
import util.InstCreator;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.beans.XPropertySet;
import com.sun.star.drawing.XShape;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.style.XStyle;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class ScShapeObj extends TestCase {

    static XComponent xSheetDoc;

    protected void initialize( TestParameters tParam, PrintWriter log ) {

        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            log.println( "creating a sheetdoc" );
            xSheetDoc = (XComponent) UnoRuntime.queryInterface(XComponent.class, SOF.createCalcDoc(null));
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        util.DesktopTools.closeDoc(xSheetDoc);
    }

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.drawing.EllipseShape</code> as tested component
     * and adds it to the document.
     *     Object relations created :
     * <ul>
     *  <li> <code>'Style1', 'Style2'</code> for
     *      {@link ifc.drawing._Shape} :
     *       two values of 'Style' property. The first is taken
     *       from the shape tested, the second from another
     *       shape added to the draw page. </li>
     *  <li> <code>'XTEXTINFO'</code> for
     *      {@link ifc.text._XText} :
     *      creator which can create instnaces of
     *      <code>com.sun.star.text.TextField.URL</code>
     *      service. </li>
     * </ul>
     */
    protected TestEnvironment createTestEnvironment
            (TestParameters tParam, PrintWriter log) {

        XInterface oObj = null;
        XShape oShape = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());
        oShape = SOF.createShape(xSheetDoc,5000, 3500, 7500, 5000,"Rectangle");
        DrawTools.getShapes(DrawTools.getDrawPage(xSheetDoc,0)).add(oShape);
        oObj = oShape ;

        for (int i=0; i < 10; i++) {
            DrawTools.getShapes(DrawTools.getDrawPage(xSheetDoc,0)).add(
                SOF.createShape(xSheetDoc,
                    5000, 3500, 7510 + 10 * i, 5010 + 10 * i, "Rectangle"));
        }

        // create test environment here
        TestEnvironment tEnv = new TestEnvironment( oShape );

        log.println("Implementation name: "+util.utils.getImplName(oObj));
        tEnv.addObjRelation("DOCUMENT",xSheetDoc);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class ScShapeObj

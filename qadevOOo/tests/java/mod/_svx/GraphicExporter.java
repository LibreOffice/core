/*************************************************************************
 *
 *  $RCSfile: GraphicExporter.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 12:36:39 $
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

package mod._svx;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DrawTools;
import util.SOfficeFactory;
import util.XMLTools;

import com.sun.star.beans.XPropertySet;
import com.sun.star.document.XExporter;
import com.sun.star.drawing.XShape;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.drawing.GraphicExportFilter</code>. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::document::XFilter</code></li>
 *  <li> <code>com::sun::star::document::XMimeTypeInfo</code></li>
 *  <li> <code>com::sun::star::document::XExporter</code></li>
 * </ul> <p>
 *
 * The following files used by this test :
 * <ul>
 *  <li><b> space-metal.jpg </b> : the file used for GraphicObject
 *  creation. This image must be then exported. </li>
 * </ul> <p>
 *
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 *
 * @see com.sun.star.document.XFilter
 * @see com.sun.star.document.XMimeTypeInfo
 * @see com.sun.star.document.XExporter
 * @see ifc.document._XFilter
 * @see ifc.document._XMimeTypeInfo
 * @see ifc.document._XExporter
 */
public class GraphicExporter extends TestCase {

    XComponent xDrawDoc;

    /**
     * Creates a new draw document.
     */
    protected void initialize( TestParameters tParam, PrintWriter log ) {

        log.println( "creating a drawdoc" );
        xDrawDoc = DrawTools.createDrawDoc((XMultiServiceFactory)tParam.getMSF());
    }

    /**
     * Disposes the draw document created before
     */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xDrawDoc " );
        xDrawDoc.dispose();
    }

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.drawing.GraphicExportFilter</code> as
     * a tested component. Then a <code>GraphicObjectShape</code>
     * instance is added into the document and its image is obtained
     * from JPEG file. This shape content is intended to be exported.
     *
     *     Object relations created :
     * <ul>
     *  <li> <code>'MediaDescriptor'</code> for
     *      {@link ifc.document._XFilter} :
     *      descriptor which contains target file name in
     *      the temporary directory, file type (JPEG)
     *      </li>
     *  <li> <code>'XFilter.Checker'</code> for
     *      {@link ifc.document._XFilter} :
     *      checks if the target file exists.
     *      In the case if SOffice is started in 'Hide' mode
     *      ('soapi.test.hidewindows' test parameter is 'true')
     *      the checker always returns <code>true</code>.
     *      </li>
     *  <li> <code>'SourceDocument'</code> for
     *      {@link ifc.document._XExporter} :
     *      the <code>GraphicObjectShape</code> component
     *      with loaded image.
     *   </li>
     * </ul>
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {

        XInterface oObj = null;
        XShape oShape = null;
        Object go=null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );
        try {
            go = ((XMultiServiceFactory)tParam.getMSF()).createInstance
                ("com.sun.star.drawing.GraphicExportFilter");
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't create instance");
            e.printStackTrace(log);
        }

        // create testobject here
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());
        oShape = SOF.createShape(xDrawDoc,5000,5000,1500,1000,"GraphicObject");
        DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape);
        XPropertySet oShapeProps = (XPropertySet)
            UnoRuntime.queryInterface(XPropertySet.class,oShape);
        XComponent xComp = null;
        try {
            oShapeProps.setPropertyValue(
                "GraphicURL",util.utils.getFullTestURL("space-metal.jpg"));
            xComp = (XComponent) UnoRuntime.queryInterface
                (XComponent.class,oShape);
            XExporter xEx = (XExporter) UnoRuntime.queryInterface
                (XExporter.class,(XInterface) go);
            xEx.setSourceDocument(xComp);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log) ;
            throw new StatusException("Error while preparing component", e) ;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log) ;
            throw new StatusException("Error while preparing component", e) ;
        } catch (com.sun.star.beans.PropertyVetoException e) {
            e.printStackTrace(log) ;
            throw new StatusException("Error while preparing component", e) ;
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace(log) ;
            throw new StatusException("Error while preparing component", e) ;
        }

        final URL aURL = new URL() ;
        aURL.Complete = util.utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF())+"picture.jpg";

        final XSimpleFileAccess fAcc ;
        try {
            Object oFAcc = ((XMultiServiceFactory)tParam.getMSF()).createInstance
                ("com.sun.star.ucb.SimpleFileAccess") ;
            fAcc = (XSimpleFileAccess) UnoRuntime.queryInterface
                (XSimpleFileAccess.class, oFAcc) ;
            if (fAcc.exists(aURL.Complete)) {
                fAcc.kill(aURL.Complete);
            }
        } catch (com.sun.star.uno.Exception e) {
            log.println("Error accessing file system :");
            e.printStackTrace(log);
            throw new StatusException("Error accessing file system.", e) ;
        }

        oObj = (XInterface) go;
        log.println("ImplName "+ util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment( oObj );
        tEnv.addObjRelation("MediaDescriptor", XMLTools.createMediaDescriptor(
            new String[] {"FilterName","URL", "MediaType"},
            new Object[] {"JPG",aURL, "image/jpeg"}));
        tEnv.addObjRelation("SourceDocument",xComp);

        log.println("adding ObjRelation for XFilter");
        log.println("This Component doesn't really support the cancel method");
        log.println("See #101725");
        tEnv.addObjRelation("CANCEL", new Boolean(false));


        final String hideMode = (String) tParam.get("soapi.test.hidewindows") ;
        tEnv.addObjRelation("XFilter.Checker",
            new ifc.document._XFilter.FilterChecker() {
                public boolean checkFilter() {
                    try {
                        if (hideMode != null && hideMode.equals("true"))
                            return true ;
                        return fAcc.exists(aURL.Complete) ;
                    } catch (com.sun.star.uno.Exception e) {
                        return false ;
                    }
                }
            }) ;

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class GraphicExporter


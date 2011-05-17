/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package mod._xmloff.Draw;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;
import util.XMLTools;

import com.sun.star.container.XNamed;
import com.sun.star.document.XExporter;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Any;
import com.sun.star.uno.Exception;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.xml.sax.XDocumentHandler;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.comp.Draw.XMLContentExporter</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li><code>com::sun::star::lang::XInitialization</code></li>
 *  <li><code>com::sun::star::document::ExportFilter</code></li>
 *  <li><code>com::sun::star::document::XFilter</code></li>
 *  <li><code>com::sun::star::document::XExporter</code></li>
 *  <li><code>com::sun::star::beans::XPropertySet</code></li>
 * </ul>
 * @see com.sun.star.lang.XInitialization
 * @see com.sun.star.document.ExportFilter
 * @see com.sun.star.document.XFilter
 * @see com.sun.star.document.XExporter
 * @see com.sun.star.beans.XPropertySet
 * @see ifc.lang._XInitialization
 * @see ifc.document._ExportFilter
 * @see ifc.document._XFilter
 * @see ifc.document._XExporter
 * @see ifc.beans._XPropertySet
 */
public class XMLContentExporter extends TestCase {
    XComponent xDrawDoc = null;

    /**
    * New text document created.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());

        try {
            log.println( "creating a drawdocument" );
            xDrawDoc = SOF.createDrawDoc(null);
        } catch ( Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
     * Document disposed here.
     */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xDrawDoc " );
        xDrawDoc.dispose();
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.comp.Draw.XMLContentExporter</code> with
    * argument which is an implementation of <code>XDocumentHandler</code>
    * and which can check if required tags and character data is
    * exported. <p>
    * The draw document is set as a source document for exporter
    * created. Then a new page (Slide) created in this document and
    * both two pages renamed to 'NewSlide1' and 'NewSlide2'. After this
    * filter checks that new tags exist in the XML output.
    *     Object relations created :
    * <ul>
    *  <li> <code>'MediaDescriptor'</code> for
    *      {@link ifc.document._XFilter} interface </li>
    *  <li> <code>'XFilter.Checker'</code> for
    *      {@link ifc.document._XFilter} interface </li>
    *  <li> <code>'SourceDocument'</code> for
    *      {@link ifc.document._XExporter} interface </li>
    * </ul>
    */
    protected synchronized TestEnvironment createTestEnvironment
            (TestParameters tParam, PrintWriter log) {

        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF() ;
        XInterface oObj = null;
        FilterChecker filter = new FilterChecker(log);
        Any arg = new Any(new Type(XDocumentHandler.class),filter);

        // Adding tags for checking existance of head tag and other tags
        filter.addTag(new XMLTools.Tag("office:document-content"));
        filter.addTag(new XMLTools.Tag("office:body"));
        filter.addTagEnclosed(
            new XMLTools.Tag("draw:page"),
            new XMLTools.Tag("office:body"));
        filter.addTag(new XMLTools.Tag("draw:page","draw:name","NewSlide1"));
        filter.addTag(new XMLTools.Tag("draw:page","draw:name","NewSlide2"));

        try {
            oObj = (XInterface) xMSF.createInstanceWithArguments(
                "com.sun.star.comp.Draw.XMLContentExporter",
                new Object[] {arg});
            XExporter xEx = (XExporter)
                UnoRuntime.queryInterface(XExporter.class,oObj);
            //xEx.setSourceDocument(xDrawDoc);

            XDrawPagesSupplier supp = (XDrawPagesSupplier)
                UnoRuntime.queryInterface(XDrawPagesSupplier.class, xDrawDoc);
            XDrawPages set = supp.getDrawPages();

            // This is an XML-export BUG (new slide named "NewSlide2"
            // can not be exported to XML)
            set.insertNewByIndex(1);

            XDrawPage page1 = (XDrawPage)
                UnoRuntime.queryInterface(XDrawPage.class, set.getByIndex(0));
            XNamed NPage1 = (XNamed)
                UnoRuntime.queryInterface(XNamed.class,page1);
            NPage1.setName("NewSlide1");
            XDrawPage page2 = (XDrawPage)
                UnoRuntime.queryInterface(XDrawPage.class, set.getByIndex(1));
            XNamed NPage2 = (XNamed)
                UnoRuntime.queryInterface(XNamed.class,page2);
            NPage2.setName("NewSlide2");
            xEx.setSourceDocument(xDrawDoc);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log) ;
            throw new StatusException("Can't create component.", e) ;
        }

        // create testobject here
        log.println( "creating a new environment" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("MediaDescriptor", XMLTools.createMediaDescriptor(
            new String[] {"FilterName"},
            new Object[] {"sdraw: StarOffice XML (Draw)"}));
        tEnv.addObjRelation("SourceDocument",xDrawDoc);
        tEnv.addObjRelation("XFilter.Checker", filter);
        return tEnv;

    }

    /**
     * This class checks the XML for tags and data required and returns
     * checking result to <code>XFilter</code> interface test. All
     * the information about errors occurred in XML data is written
     * to log specified.
     * @see ifc.document._XFilter
     */
    protected class FilterChecker extends XMLTools.XMLChecker
        implements ifc.document._XFilter.FilterChecker {

        /**
         * Creates a class which will write information
         * into log specified.
         */
        public FilterChecker(PrintWriter log) {
            super(log,true) ;
        }
        /**
         * <code>_XFilter.FilterChecker</code> interface method
         * which returns the result of XML checking.
         * @return <code>true</code> if the XML data exported was
         * valid (i.e. all necessary tags and character data exists),
         * <code>false</code> if some errors occurred.
         */
        public boolean checkFilter() {
            return check();
        }
    }
}



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

package mod._sm;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.beans.XPropertySet;
import com.sun.star.document.XDocumentInfo;
import com.sun.star.document.XDocumentInfoSupplier;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.comp.Math.XMLMetaImporter</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li><code>com::sun::star::lang::XInitialization</code></li>
 *  <li><code>com::sun::star::document::XImporter</code></li>
 *  <li><code>com::sun::star::document::XFilter</code></li>
 *  <li><code>com::sun::star::document::ImportFilter</code></li>
 *  <li><code>com::sun::star::beans::XPropertySet</code></li>
 *  <li><code>com::sun::star::xml::sax::XDocumentHandler</code></li>
 * </ul>
 * @see com.sun.star.lang.XInitialization
 * @see com.sun.star.document.XImporter
 * @see com.sun.star.document.XFilter
 * @see com.sun.star.document.ImportFilter
 * @see com.sun.star.beans.XPropertySet
 * @see com.sun.star.xml.sax.XDocumentHandler
 * @see ifc.lang._XInitialization
 * @see ifc.document._XImporter
 * @see ifc.document._XFilter
 * @see ifc.document._XExporter
 * @see ifc.beans._XPropertySet
 * @see ifc.xml.sax._XDocumentHandler
 */
public class XMLMetaImporter extends TestCase {
    XComponent xMathDoc;

    /**
    * New math document created.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {

        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );
        try {
            xMathDoc = SOF.openDoc("smath","_blank");
        } catch (com.sun.star.uno.Exception ex) {
            ex.printStackTrace( log );
            throw new StatusException( "Couldn't create document", ex );
        }
    }

    /**
    * Disposes document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xMathDoc " );
        xMathDoc.dispose();
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.comp.Math.XMLMetaImporter</code><p>
    *
    * The math document is set as a target document for importer.
    * Imported XML-data contains the tag which specifies new user info
    * field and a title of document.
    * After import user fields info and the title of target document
    * is checked.
    *     Object relations created :
    * <ul>
    *  <li> <code>'XDocumentHandler.XMLData'</code> for
    *      {@link ifc.xml.sax._XDocumentHandler} interface </li>
    *  <li> <code>'XDocumentHandler.ImportChecker'</code> for
    *      {@link ifc.xml.sax._XDocumentHandler} interface </li>
    *  <li> <code>'TargetDocument'</code> for
    *      {@link ifc.document._XImporter} interface </li>
    * </ul>
    */
    public synchronized TestEnvironment createTestEnvironment
            ( TestParameters Param, PrintWriter log )
            throws StatusException {

        XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
        XInterface oObj = null;
        final String impName = "XMLMetaImporter" ;
        final String impValue = "XMLMetaImporter_Value" ;
        final String impTitle = "XMLMetaImporter Title" ;

        final XDocumentInfoSupplier xDocInfoSup ;
        try {
            oObj = (XInterface)xMSF.createInstance(
                    "com.sun.star.comp.Math.XMLMetaImporter");

            xDocInfoSup = (XDocumentInfoSupplier) UnoRuntime.queryInterface
                (XDocumentInfoSupplier.class, xMathDoc) ;
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("TargetDocument",xMathDoc);

        String[][] xml = new String[][] {
            {"start", "office:document-meta",
                "xmlns:office", "CDATA", "http://openoffice.org/2000/office",
                "xmlns:meta", "CDATA", "http://openoffice.org/2000/meta",
                "xmlns:xlink", "CDATA", "http://www.w3.org/1999/xlink",
                "xmlns:dc", "CDATA", "http://purl.org/dc/elements/1.1/"
            },
            {"start", "office:meta"},
            {"start", "dc:title"},
            {"chars", impTitle},
            {"end", "dc:title"},
            {"start", "meta:user-defined",
                "meta:name", "CDATA", impName},
            {"chars", impValue},
            {"end", "meta:user-defined"},
            {"end", "office:meta"},
            {"end", "office:document-meta"}} ;

        tEnv.addObjRelation("XDocumentHandler.XMLData", xml) ;

        final PrintWriter logF = log ;

        tEnv.addObjRelation("XDocumentHandler.ImportChecker",
            new ifc.xml.sax._XDocumentHandler.ImportChecker() {
                public boolean checkImport() {
                    try {
                        XDocumentInfo xDocInfo = xDocInfoSup.getDocumentInfo() ;
                        XPropertySet xDocInfoProp = (XPropertySet)
                            UnoRuntime.queryInterface
                            (XPropertySet.class, xDocInfo) ;
                        boolean result = false ;
                        for (short i = 0; i < xDocInfo.getUserFieldCount(); i++) {
                            String gName = xDocInfo.getUserFieldName(i) ;
                            String gValue = xDocInfo.getUserFieldValue(i) ;
                            logF.println("Field '" + gName + "' = '"
                                + gValue + "'") ;
                            if (impName.equals(gName) && impValue.equals(gValue))
                                result = true ;
                        }
                        String gTitle = (String) xDocInfoProp.getPropertyValue
                            ("Title");
                        logF.println("Title returned : '" + gTitle + "'");
                        result &= impTitle.equals(gTitle) ;

                        return result ;
                    } catch (com.sun.star.uno.Exception e) {
                        logF.println("Exception occurred while checking filter :") ;
                        e.printStackTrace(logF) ;
                        return false ;
                    }
                }
            }) ;

        return tEnv;
    }
}


/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package mod._sm;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.comp.Math.XMLImporter</code>. <p>
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
public class XMLImporter extends TestCase {
    XComponent xMathDoc;


    /**
    * New math document created.
    */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {

        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF() );
        xMathDoc = SOF.openDoc("smath","_blank");
    }

    /**
    * Disposes document.
    */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xMathDoc " );
        xMathDoc.dispose();
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.comp.Math.XMLImporter</code><p>
    *
    * The chart document is set as a target document for importer.
    * Imported XML-data contains the tag which specifies a formula
    * in the document.
    * After import the formula getting from target document is checked.
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
    @Override
    public TestEnvironment createTestEnvironment
            ( TestParameters Param, PrintWriter log )
            throws Exception {

        XMultiServiceFactory xMSF = Param.getMSF();
        XInterface oObj = null;
        final String impFormula = "a - b" ;

        oObj = (XInterface)xMSF.createInstance(
                "com.sun.star.comp.Math.XMLImporter");

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("TargetDocument",xMathDoc);

        String[][] xml = new String[][] {
            {"start", "math:math",
                "xmlns:math", "CDATA", "http://www.w3.org/1998/Math/MathML"},
            {"start", "math:semantics"},
            {"start", "math:annotation",
                "math:encoding", "CDATA", "StarMath 5.0"},
            {"chars", impFormula},
            {"end", "math:annotation"},
            {"end", "math:semantics"},
            {"end", "math:math"}} ;

        tEnv.addObjRelation("XDocumentHandler.XMLData", xml) ;

        final PrintWriter logF = log ;
        final XPropertySet xPS = UnoRuntime.queryInterface
            (XPropertySet.class, xMathDoc) ;

        tEnv.addObjRelation("XDocumentHandler.ImportChecker",
            new ifc.xml.sax._XDocumentHandler.ImportChecker() {
                public boolean checkImport() {
                    try {
                        String gFormula = (String) xPS.getPropertyValue
                            ("Formula") ;
                        logF.println("Formula returned = '" + gFormula + "'") ;
                        return impFormula.equals(gFormula) ;
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


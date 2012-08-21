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

package mod._xmloff.Impress;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;
import util.XMLTools;

import com.sun.star.beans.XPropertySet;
import com.sun.star.document.XExporter;
import com.sun.star.frame.XController;
import com.sun.star.frame.XModel;
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
 * <code>com.sun.star.comp.Impress.XMLSettingsExporter</code>. <p>
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
public class XMLSettingsExporter extends TestCase {
    XComponent xImpressDoc = null;

    /**
    * New text document created.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());

        try {
            log.println( "creating an impress document" );
            xImpressDoc = SOF.createImpressDoc(null);
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
        log.println( "    disposing xImpressDoc " );
        xImpressDoc.dispose();
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.comp.Impress.XMLSettingsExporter</code> with
    * argument which is an implementation of <code>XDocumentHandler</code>
    * and which can check if required tags and character data is
    * exported. <p>
    * The Impress document is set as a source document for exporter
    * created. Then setting 'IsLayerMode' is changed to a new value.
    * After this filter checks that value has changed in the XML output.
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
    public synchronized TestEnvironment createTestEnvironment
        (TestParameters tParam, PrintWriter log ) throws StatusException {

        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();
        XInterface oObj = null;
        FilterChecker filter = new FilterChecker(log);
        Any arg = new Any(new Type(XDocumentHandler.class),filter);
        final boolean NewDataValue;

        try {
            oObj = (XInterface) xMSF.createInstanceWithArguments(
                "com.sun.star.comp.Impress.XMLSettingsExporter",
                new Object[] {arg});
            XExporter xEx = UnoRuntime.queryInterface(XExporter.class,oObj);
            xEx.setSourceDocument(xImpressDoc);

            //set some settings
            XModel xImpressModel = UnoRuntime.queryInterface(XModel.class, xImpressDoc);
            XController xController = xImpressModel.getCurrentController();
            XPropertySet xPropSet = UnoRuntime.queryInterface(XPropertySet.class, xController);
            NewDataValue = ! ((Boolean) xPropSet.getPropertyValue
                ("IsLayerMode")).booleanValue();
            xPropSet.setPropertyValue("IsLayerMode",
                new Boolean (NewDataValue));

/*            XViewDataSupplier xViewDataSupp = (XViewDataSupplier)
                UnoRuntime.queryInterface(XViewDataSupplier.class, xImpressDoc);
            XIndexAccess xViewData = xViewDataSupp.getViewData();
            PropertyValue[] xData = (PropertyValue[]) xViewData.getByIndex(0);
            NewDataValue = ! ((Boolean) xData[1].Value).booleanValue();
            xData[1].Value = new Boolean(NewDataValue);
            XIndexReplace xViewDataReplacable = (XIndexReplace)
                UnoRuntime.queryInterface(XIndexReplace.class, xViewData);
            xViewDataReplacable.replaceByIndex(0,xData);
*/

        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log) ;
            throw new StatusException("Can't create component.", e) ;
        }

        // Adding tags for checking existance of head tag and other tags
        filter.addTagEnclosed(new XMLTools.Tag("office:settings"),
            new XMLTools.Tag("office:document-settings"));
        filter.addTagEnclosed(new XMLTools.Tag("config:config-item-set"),
            new XMLTools.Tag("office:settings"));
        filter.addTagEnclosed(new XMLTools.Tag("config:config-item-map-indexed"),
            new XMLTools.Tag("config:config-item-set"));
        filter.addTagEnclosed(new XMLTools.Tag("config:config-item-map-entry"),
            new XMLTools.Tag("config:config-item-map-indexed"));
        filter.addTagEnclosed(new XMLTools.Tag("config:config-item"),
            new XMLTools.Tag("config:config-item-map-entry"));
        filter.addCharactersEnclosed(String.valueOf(NewDataValue),
            new XMLTools.Tag("config:config-item","config:name","IsLayerMode"));

        // create testobject here
        log.println( "creating a new environment" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("MediaDescriptor", XMLTools.createMediaDescriptor(
            new String[] {"FilterName"},
            new Object[] {"simpress: StarOffice XML (Impress)"}));
        tEnv.addObjRelation("SourceDocument",xImpressDoc);
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
            super(log,false) ;
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

/*************************************************************************
 *
 *  $RCSfile: XMLSettingsExporter.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 13:18:41 $
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
            xImpressDoc = SOF.createImpressDoc(null);;
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
            XExporter xEx = (XExporter)
                UnoRuntime.queryInterface(XExporter.class,oObj);
            xEx.setSourceDocument(xImpressDoc);

            //set some settings
            XModel xImpressModel = (XModel)
                UnoRuntime.queryInterface(XModel.class, xImpressDoc);
            XController xController = xImpressModel.getCurrentController();
            XPropertySet xPropSet = (XPropertySet)
                UnoRuntime.queryInterface(XPropertySet.class, xController);
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
    * the information about errors occured in XML data is written
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
        * <code>false</code> if some errors occured.
        */
        public boolean checkFilter() {
            return check();
        }
    }
}

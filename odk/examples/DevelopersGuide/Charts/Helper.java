/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// __________ Imports __________

import java.util.Random;

// base classes
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.*;

// factory for creating components
import com.sun.star.comp.servicemanager.ServiceManager;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.XNamingService;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XComponentLoader;

// property access
import com.sun.star.beans.*;

// container access
import com.sun.star.container.*;

// application specific classes
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.text.XTextDocument;

import com.sun.star.document.XEmbeddedObjectSupplier;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XController;

// Exceptions
import com.sun.star.uno.RuntimeException;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.lang.IndexOutOfBoundsException;

// __________ Implementation __________

/** Helper for creating a calc document adding cell values and charts
    @author Bj&ouml;rn Milcke
 */
public class Helper
{
    public Helper( String[] args )
    {
        // connect to a running office and get the ServiceManager
        try {
            // get the remote office component context
            maContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            System.out.println("Connected to a running office ...");

            // get the remote office service manager
            maMCFactory = maContext.getServiceManager();
        }
        catch( Exception e) {
            System.out.println( "Couldn't get ServiceManager: " + e );
            e.printStackTrace();
            System.exit(1);
        }
    }

    // ____________________

    public XSpreadsheetDocument createSpreadsheetDocument()
    {
        return (XSpreadsheetDocument) UnoRuntime.queryInterface(
            XSpreadsheetDocument.class, createDocument( "scalc" ));
    }

    // ____________________

    public XModel createPresentationDocument()
    {
        return createDocument( "simpress" );
    }

    // ____________________

    public XModel createDrawingDocument()
    {
        return createDocument( "sdraw" );
    }

    // ____________________

    public XModel createTextDocument()
    {
        return createDocument( "swriter" );
    }

    // ____________________

    public XModel createDocument( String sDocType )
    {
        XModel aResult = null;
        try
        {
            XComponentLoader aLoader = (XComponentLoader)
                UnoRuntime.queryInterface(XComponentLoader.class,
                maMCFactory.createInstanceWithContext("com.sun.star.frame.Desktop",
                                                      maContext) );

            aResult = (XModel) UnoRuntime.queryInterface(
                XModel.class,
                aLoader.loadComponentFromURL( "private:factory/" + sDocType,
                                              "_blank",
                                              0,
                                              new PropertyValue[ 0 ] ) );
        }
        catch( Exception e )
        {
            System.err.println("Couldn't create Document of type "+ sDocType +": "+e);
            e.printStackTrace();
            System.exit( 0 );
        }

        return aResult;
    }

    public XComponentContext getComponentContext(){
        return maContext;

    }

    // __________ private members and methods __________

    private final String  msDataSheetName  = "Data";
    private final String  msChartSheetName = "Chart";
    private final String  msChartName      = "SampleChart";

    private XComponentContext      maContext;
    private XMultiComponentFactory maMCFactory;
    private XSpreadsheetDocument   maSpreadSheetDoc;
}

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



package mod._sch;
import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.chart.XChartData;
import com.sun.star.chart.XChartDocument;
import com.sun.star.lang.XMultiServiceFactory;

/**
* Test for object which is represented by service
* <code>com.sun.star.chart.ChartDataArray</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::chart::XChartData</code></li>
*  <li> <code>com::sun::star::chart::XChartDataArray</code></li>
* </ul>
* @see com.sun.star.chart.ChartDataArray
* @see com.sun.star.chart.XChartData
* @see com.sun.star.chart.XChartDataArray
* @see ifc.chart._XChartData
* @see ifc.chart._XChartDataArray
*/
public class ChXChartDataArray extends TestCase {
    XChartDocument xChartDoc = null;

    /**
    * Creates Chart document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());

        try {
            log.println( "creating a chartdocument" );
            xChartDoc = SOF.createChartDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
    * Disposes Chart document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        if( xChartDoc!=null ) {
            log.println( "    closing xChartDoc" );
            util.DesktopTools.closeDoc(xChartDoc);
            xChartDoc = null;
        }
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves the data source of the chart. This data source is the instance
    * of the service <code>com.sun.star.chart.ChartDataArray</code>.
    * <code>com.sun.star.chart.ChartDataArray</code>.
    * @see com.sun.star.chart.ChartDataArray
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        // get the Data
        log.println( "getting Data" );
        XChartData oObj = (XChartData) xChartDoc.getData();

        log.println( "creating a new environment for chartdocument object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class ChXChartDataArray


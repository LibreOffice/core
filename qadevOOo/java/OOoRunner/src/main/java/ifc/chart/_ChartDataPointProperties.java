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



package ifc.chart;

import lib.MultiPropertyTest;
import lib.Status;
import lib.StatusException;
import util.utils;

import com.sun.star.chart.XChartDocument;
import com.sun.star.chart.XDiagram;

/**
* Testing <code>com.sun.star.chart.ChartDataPointProperties</code>
* service properties :
* <ul>
*  <li><code> DataCaption</code></li>
*  <li><code> SymbolType</code></li>
*  <li><code> SymbolBitmapURL</code></li>
* </ul> <p>
* The following predefined files needed to complete the test:
* <ul>
*  <li> <code>space-metal.jpg</code> :
*  for test of property 'SymbolBitmapURL' </li>
* <ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'CHARTDOC'</code> (of type <code>XChartDocument</code>):
*  to have reference to chart document </li>
*  <li> <code>'LINE'</code> (of type <code>XDiagram</code>):
*  relation that use as parameter for method setDiagram of chart document </li>
* </ul> <p>
* @see com.sun.star.chart.ChartDataPointProperties
*/
public class _ChartDataPointProperties extends MultiPropertyTest {

    /**
    * Retrieves object relations and prepares a chart document.
    * @throws StatusException if one of relations not found.
    */
    protected void before() {
        log.println("Setting Diagram type to LineDiagram");
        XChartDocument doc = (XChartDocument) tEnv.getObjRelation("CHARTDOC");
        if (doc == null) throw new StatusException(Status.failed
            ("Relation 'CHARTDOC' not found"));

        XDiagram Line = (XDiagram) tEnv.getObjRelation("LINE");
        if (Line == null) throw new StatusException(Status.failed
            ("Relation 'LINE' not found"));

        doc.setDiagram(Line);
    }

    protected PropertyTester URLTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {
            return utils.getFullTestURL("space-metal.jpg");
        }
    };

    /**
    * Tests property 'SymbolBitmapURL' using file <code>polibal.gif</code>.
    */
    public void _SymbolBitmapURL() {
        try {
            oObj.setPropertyValue(
                "SymbolType",
                new Integer(com.sun.star.chart.ChartSymbolType.BITMAPURL) );
        } catch(com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception while set property value");
            e.printStackTrace(log);
            throw new StatusException("Exception while set property value", e);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception while set property value");
            e.printStackTrace(log);
            throw new StatusException("Exception while set property value", e);
        } catch(com.sun.star.beans.PropertyVetoException e) {
            log.println("Exception while set property value");
            e.printStackTrace(log);
            throw new StatusException("Exception while set property value", e);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            log.println("Exception while set property value");
            e.printStackTrace(log);
            throw new StatusException("Exception while set property value", e);
        }

        testProperty("SymbolBitmapURL", URLTester);
    }
}


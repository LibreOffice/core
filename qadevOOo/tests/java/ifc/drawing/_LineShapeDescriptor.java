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



package ifc.drawing;

import lib.MultiMethodTest;
import util.ValueChanger;

import com.sun.star.awt.Point;
import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.UnoRuntime;


public class _LineShapeDescriptor extends MultiMethodTest {

    public XPropertySet oObj = null;        // oObj filled by MultiMethodTest


    public void _LineColor() {
        XServiceInfo xInfo = (XServiceInfo)
            UnoRuntime.queryInterface(XServiceInfo.class, oObj);
        if ( ! xInfo.supportsService
                ("com.sun.star.drawing.LineShapeDescriptor")) {
            log.println("Service not available !!!!!!!!!!!!!");
            tRes.tested("Supported", false);
        }
        changeProp("LineColor");
    }

    public void _LineDash() {
        changeProp("LineDash");
    }
    public void _LineEnd() {
        changeProp("LineEnd");
    }
    public void _LineEndCenter() {
        changeProp("LineEndCenter");
    }
    public void _LineEndWidth() {
        changeProp("LineEndWidth");
    }
    public void _LineJoint() {
        changeProp("LineJoint");
    }
    public void _LineStart() {
        changeProp("LineStart");
    }
    public void _LineStartCenter() {
        changeProp("LineStartCenter");
    }
    public void _LineStartWidth() {
        changeProp("LineStartWidth");
    }
    public void _LineStyle() {
        changeProp("LineStyle");
    }
    public void _LineTransparence() {
        changeProp("LineTransparence");
    }
    public void _LineWidth() {
        changeProp("LineWidth");
    }

    public void changeProp(String name) {

        Object gValue = null;
        Object sValue = null;
        Object ValueToSet = null;

        try {
            //waitForAllThreads();
            gValue = oObj.getPropertyValue(name);
            //waitForAllThreads();
            if ( (name.equals("LineEnd")) || (name.equals("LineStart")) ) {
                if (gValue == null) gValue = newPoints(null);
                ValueToSet = newPoints( (Point[]) gValue);
            }
            else {
                ValueToSet = ValueChanger.changePValue(gValue);
            }
            //waitForAllThreads();
            oObj.setPropertyValue(name,ValueToSet);
            sValue = oObj.getPropertyValue(name);


         //check get-set methods
         if (gValue.equals(sValue)) {
            log.println("Value for '"+name+"' hasn't changed");
            tRes.tested(name, false);
        }
        else {
            log.println("Property '"+name+"' OK");
            tRes.tested(name, true);
        }
        } catch (Exception e) {
            log.println("Exception occurred while testing property '" +
                 name + "'");
            e.printStackTrace(log);
            tRes.tested(name, false);
        }


    } // end of ChangeProp

    public Point[] newPoints(Point[] old) {

        Point [] newP = new Point[3];

        if (old == null) {
            newP[0] = new Point(0, 2);
            newP[1] = new Point(2, 0);
            newP[2] = new Point(0, -2);
        }
        else {
            newP = old;
            newP[0].X += 1;
            newP[1].X += 1;
            newP[2].X += 1;
        }
        return newP;
    }

}



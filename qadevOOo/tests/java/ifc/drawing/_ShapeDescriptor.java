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

package ifc.drawing;

import lib.MultiMethodTest;
import util.ValueChanger;

import com.sun.star.beans.XPropertySet;
import com.sun.star.style.XStyle;
import com.sun.star.uno.UnoRuntime;


public class _ShapeDescriptor extends MultiMethodTest {

    public XPropertySet oObj = null;        // oObj filled by MultiMethodTest
    public boolean ro = false;

    public void _LayerID() {
        com.sun.star.lang.XServiceInfo xInfo = UnoRuntime.queryInterface
            (com.sun.star.lang.XServiceInfo.class, oObj);
        if ( ! xInfo.supportsService("com.sun.star.drawing.ShapeDescriptor")) {
            log.println("Service not available !!!!!!!!!!!!!");
            tRes.tested("Supported", false);
        }
        ro = true;
        changeProp("LayerID");
        ro = false;
    }
    public void _LayerName() {
        ro = true;
        changeProp("LayerName");
        ro = false;
    }
    public void _MoveProtect() {
        changeProp("MoveProtect");
    }
    public void _Name() {
        changeProp("Name");
    }
    public void _Printable() {
        changeProp("Printable");
    }
    public void _SizeProtect() {
        changeProp("SizeProtect");
    }
    public void _Style() {
        changeProp("Style");
    }

    public void _Transformation() {
        changeProp("Transformation");
    }

    public void changeProp(String name) {

        Object gValue = null;
          Object sValue = null;
        Object ValueToSet = null;


        try {
            gValue = oObj.getPropertyValue(name);
            if (!ro) {
                ValueToSet = ValueChanger.changePValue(gValue);
                if ( name.equals("Style") ) {
                    ValueToSet = newStyle(gValue);
                }
                oObj.setPropertyValue(name,ValueToSet);
                sValue = oObj.getPropertyValue(name);
            }

             //check get-set methods
            if (gValue.equals(sValue)) {
                log.println("Value for '"+name+"' hasn't changed");
                tRes.tested(name, false);
            } else {
                log.println("Property '"+name+"' OK");
                tRes.tested(name, true);
            }
        } catch (com.sun.star.beans.UnknownPropertyException ex) {
            if (isOptional(name)) {
                log.println("Property '"+name+
                    "' is optional and not supported");
                tRes.tested(name,true);
            } else {
                log.println("Exception occurred while testing property '" +
                    name + "'");
                ex.printStackTrace(log);
                tRes.tested(name, false);
            }
        }
        catch (Exception e) {
             log.println("Exception occurred while testing property '" +
                name + "'");
             e.printStackTrace(log);
             tRes.tested(name, false);
        }


    }// end of changeProp

    public XStyle newStyle(Object oldStyle) {
        XStyle Style1 = (XStyle) tEnv.getObjRelation("Style1");
        XStyle Style2 = (XStyle) tEnv.getObjRelation("Style2");
        XStyle back = null;
        if ( (Style1!=null) && (Style2!=null) ) {
            if ( ((XStyle) oldStyle).equals(Style1) ) {
                back = Style2;
            } else {
                back = Style1;
            }
        }
        return back;
    }

}



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
        com.sun.star.lang.XServiceInfo xInfo = (com.sun.star.lang.XServiceInfo)
            UnoRuntime.queryInterface
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
            //waitForAllThreads();
            gValue = oObj.getPropertyValue(name);
            //waitForAllThreads();
            if (!ro) {
                ValueToSet = ValueChanger.changePValue(gValue);
                if ( name.equals("Style") ) {
                    ValueToSet = newStyle(gValue);
                }
                //waitForAllThreads();
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



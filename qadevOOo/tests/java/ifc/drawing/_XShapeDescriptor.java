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

import com.sun.star.drawing.XShapeDescriptor;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.drawing.XShapeDescriptor</code>
* interface methods :
* <ul>
*  <li><code> getShapeType()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.drawing.XShapeDescriptor
*/
public class _XShapeDescriptor extends MultiMethodTest {

    public XShapeDescriptor    oObj = null;
    boolean result = true;

    /**
    * Gets the type of shape. This type must be a service name which
    * is supported by object (except of ChartObject and Frame shapes).
    * The object is queried for <code>XServiceInfo</code> and
    * the type is checked to be among supported service names.<p>
    *
    * Has <b> OK </b> status if the type is found among supported
    * services. Or if the object represents a text frame.<p>
    */
    public void _getShapeType() {
        result = false;

        String stype = oObj.getShapeType();
        log.println("Current Shape Type is " + stype);

        XServiceInfo SI = (XServiceInfo)
                        UnoRuntime.queryInterface(XServiceInfo.class, oObj);

        String[] serviceNames = SI.getSupportedServiceNames();

        log.println("Supported services :");
        for (int i = 0; i < serviceNames.length; i++) {
            log.println("  " + serviceNames[i]);
            if (serviceNames[i].equals(stype))
                result = true;
        }

        //remark: we should provide the expected name as ObjRelation

        //Chart has its own behaviour it always return 'ChartObject'
        if (stype.equals("com.sun.star.chart.ChartObject")) result=true;
        if (stype.equals("com.sun.star.drawing.ControlShape")) result=true;
        if (stype.equals("com.sun.star.drawing.ClosedBezierShape")) result=true;
        if (stype.equals("com.sun.star.drawing.CaptionShape")) result=true;

        //Writer has its own behaviour it returns a 'FrameShape'
        if (stype.equals("FrameShape")) result=true;

        if (!result) {
            log.println("Service " + stype + " not supported in the object.");
        }

        tRes.tested("getShapeType()", result);
    }

}



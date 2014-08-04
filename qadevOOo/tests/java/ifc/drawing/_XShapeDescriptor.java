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

import com.sun.star.drawing.XShapeDescriptor;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.drawing.XShapeDescriptor</code>
* interface methods :
* <ul>
*  <li><code> getShapeType()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
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

        XServiceInfo SI = UnoRuntime.queryInterface(XServiceInfo.class, oObj);

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



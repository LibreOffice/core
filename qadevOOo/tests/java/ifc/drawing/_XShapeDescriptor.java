/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XShapeDescriptor.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:42:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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



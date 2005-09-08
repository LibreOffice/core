/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _LineShapeDescriptor.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:34:47 $
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
            log.println("Exception occured while testing property '" +
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



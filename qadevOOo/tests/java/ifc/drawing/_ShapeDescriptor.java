/*************************************************************************
 *
 *  $RCSfile: _ShapeDescriptor.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:29:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
                log.println("Exception occured while testing property '" +
                    name + "'");
                ex.printStackTrace(log);
                tRes.tested(name, false);
            }
        }
        catch (Exception e) {
             log.println("Exception occured while testing property '" +
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



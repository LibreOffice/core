/*************************************************************************
 *
 *  $RCSfile: _XHierarchicalPropertySet.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:14:50 $
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
package ifc.beans;

import lib.MultiMethodTest;
import lib.Status;
import util.ValueChanger;
import util.ValueComparer;

import com.sun.star.beans.XHierarchicalPropertySet;
import com.sun.star.beans.XHierarchicalPropertySetInfo;


public class _XHierarchicalPropertySet extends MultiMethodTest {
    public XHierarchicalPropertySet oObj;

    public void _getHierarchicalPropertySetInfo() {
        XHierarchicalPropertySetInfo hpsi = oObj.getHierarchicalPropertySetInfo();
        boolean res = true;

        if (hpsi != null) {
            res = checkHPSI(hpsi);
        } else {
            log.println(
                    "The component doesn't provide HierarchicalPropertySetInfo");
            tRes.tested("getHierarchicalPropertySetInfo()",
                        Status.skipped(true));

            return;
        }

        tRes.tested("getHierarchicalPropertySetInfo()", res);
    }

    public void _getHierarchicalPropertyValue() {
        String[] pNames = (String[]) tEnv.getObjRelation("PropertyNames");
        String[] pTypes = (String[]) tEnv.getObjRelation("PropertyTypes");
        boolean res = true;

        for (int i = 0; i < pNames.length; i++) {
            try {
                log.print("Property " + pNames[i]);

                Object getting = oObj.getHierarchicalPropertyValue(pNames[i]);
                log.println(" has Value " + getting.toString());
                res &= checkType(pNames[i], pTypes[i], getting);
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                log.println(" is unknown");
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                log.println(" is illegal");
            } catch (com.sun.star.lang.WrappedTargetException e) {
                log.println(" throws expeption " + e.getMessage());
            }
        }

        tRes.tested("getHierarchicalPropertyValue()", res);
    }

    public void _setHierarchicalPropertyValue() {
        String ro = (String) tEnv.getObjRelation("allReadOnly");

        if (ro != null) {
            log.println(ro);
            tRes.tested("setHierarchicalPropertyValue()", Status.skipped(true));

            return;
        }

        boolean res = true;

        String[] pNames = (String[]) tEnv.getObjRelation("PropertyNames");

        for (int k = 0; k < pNames.length; k++) {
            try {
                Object oldValue = oObj.getHierarchicalPropertyValue(pNames[k]);
                Object newValue = ValueChanger.changePValue(oldValue);
                oObj.setHierarchicalPropertyValue(pNames[k], newValue);

                Object getValue = oObj.getHierarchicalPropertyValue(pNames[k]);
                boolean localRes = ValueComparer.equalValue(getValue, newValue);

                if (!localRes) {
                    log.println("Expected " + newValue.toString());
                    log.println("Gained " + getValue.toString());
                }


                //reset Value
                oObj.setHierarchicalPropertyValue(pNames[k], oldValue);

                res &= localRes;
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                log.println("Property is unknown");
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                log.println("IllegalArgument "+e.getMessage());
            } catch (com.sun.star.beans.PropertyVetoException e) {
                log.println("VetoException "+e.getMessage());
            } catch (com.sun.star.lang.WrappedTargetException e) {
                log.println("WrappedTarget "+e.getMessage());
            }

        }

        tRes.tested("setHierarchicalPropertyValue()", res);
    }

    protected boolean checkHPSI(XHierarchicalPropertySetInfo hpsi) {
        log.println("Checking the resulting HierarchicalPropertySetInfo");
        log.println("### NOT yet implemented");

        return true;
    }

    protected boolean checkType(String name, String type, Object value) {
        boolean result = true;

        if (type.equals("Boolean")) {
            result = (value instanceof Boolean);

            if (!result) {
                log.println("Wrong Type for property " + name);
                log.println("Expected " + type);
                log.println("getting " + value.getClass());
            }
        } else if (type.equals("Short")) {
            result = (value instanceof Short);

            if (!result) {
                log.println("Wrong Type for property " + name);
                log.println("Expected " + type);
                log.println("getting " + value.getClass());
            }
        }

        return result;
    }
}
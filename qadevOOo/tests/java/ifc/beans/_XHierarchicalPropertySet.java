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
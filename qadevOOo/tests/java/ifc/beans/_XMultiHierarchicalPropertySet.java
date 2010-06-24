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

import com.sun.star.beans.XHierarchicalPropertySetInfo;
import com.sun.star.beans.XMultiHierarchicalPropertySet;


public class _XMultiHierarchicalPropertySet extends MultiMethodTest {
    public XMultiHierarchicalPropertySet oObj;

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

        tRes.tested("getMultiHierarchicalPropertySetInfo()", res);
    }

    public void _getHierarchicalPropertyValues() {
        String[] pNames = (String[]) tEnv.getObjRelation("PropertyNames");
        String[] pTypes = (String[]) tEnv.getObjRelation("PropertyTypes");
        boolean res = true;

        try {
            Object[] getting = oObj.getHierarchicalPropertyValues(pNames);
            res &= checkType(pNames, pTypes, getting);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception " + e.getMessage());
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception " + e.getMessage());
        }

        tRes.tested("getHierarchicalPropertyValues()", res);
    }

    public void _setHierarchicalPropertyValues() {
        String ro = (String) tEnv.getObjRelation("allReadOnly");

        if (ro != null) {
            log.println(ro);
            tRes.tested("setHierarchicalPropertyValues()",
                        Status.skipped(true));

            return;
        }

        String[] pNames = (String[]) tEnv.getObjRelation("PropertyNames");
        boolean res = true;

        try {
            Object[] oldValues = oObj.getHierarchicalPropertyValues(pNames);
            Object[] newValues = new Object[oldValues.length];

            for (int k = 0; k < oldValues.length; k++) {
                newValues[k] = ValueChanger.changePValue(oldValues[k]);
            }

            oObj.setHierarchicalPropertyValues(pNames, newValues);

            Object[] getValues = oObj.getHierarchicalPropertyValues(pNames);

            for (int k = 0; k < pNames.length; k++) {
                boolean localRes = ValueComparer.equalValue(getValues[k],
                                                            newValues[k]);

                if (!localRes) {
                    log.println("didn't work for " + pNames[k]);
                    log.println("Expected " + newValues[k].toString());
                    log.println("Getting " + getValues[k].toString());
                }
                //reset properties
                oObj.setHierarchicalPropertyValues(pNames, oldValues);
            }
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("IllegalArgument " + e.getMessage());
        } catch (com.sun.star.beans.PropertyVetoException e) {
            log.println("VetoException " + e.getMessage());
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("WrappedTarget " + e.getMessage());
        }

        tRes.tested("setHierarchicalPropertyValues()", res);
    }

    protected boolean checkHPSI(XHierarchicalPropertySetInfo hpsi) {
        log.println("Checking the resulting HierarchicalPropertySetInfo");
        log.println("### NOT yet implemented");

        return true;
    }

    protected boolean checkType(String[] name, String[] type, Object[] value) {
        boolean result = true;

        for (int k = 0; k < name.length; k++) {
            if (type[k].equals("Boolean")) {
                result &= (value[k] instanceof Boolean);

                if (!(value[k] instanceof Boolean)) {
                    log.println("Wrong Type for property " + name[k]);
                    log.println("Expected " + type[k]);
                    log.println("getting " + value[k].getClass());
                }
            } else if (type[k].equals("Short")) {
                result &= (value[k] instanceof Short);

                if (!(value[k] instanceof Short)) {
                    log.println("Wrong Type for property " + name[k]);
                    log.println("Expected " + type[k]);
                    log.println("getting " + value[k].getClass());
                }
            }
        }

        return result;
    }
}
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

        if (hpsi == null) {
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
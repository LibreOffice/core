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

import com.sun.star.beans.XHierarchicalPropertySet;
import com.sun.star.beans.XHierarchicalPropertySetInfo;


public class _XHierarchicalPropertySet extends MultiMethodTest {
    public XHierarchicalPropertySet oObj;

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
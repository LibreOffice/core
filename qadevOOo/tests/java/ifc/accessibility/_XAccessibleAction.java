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

package ifc.accessibility;

import com.sun.star.accessibility.XAccessibleAction;

public class _XAccessibleAction extends lib.MultiMethodTest {

    public XAccessibleAction oObj = null;
    public int count = 0;

    /**
     * calls the method and stores the result in the <br>
     * variable count. It is OK if no exception occurs
     */

    public void _getAccessibleActionCount() {
        count = oObj.getAccessibleActionCount();
        tRes.tested("getAccessibleActionCount()",count > 0);
    }

    /**
     * calls the method with invalid argument and check if the <br>
     * expected Exception is thrown.<br>
     * Calls the method afterwards the first valid parameter.<br>
     * This is the last method called and the environment is disposed<br>
     * afterwards.
     */

    public void _doAccessibleAction() {
        requiredMethod("getAccessibleActionKeyBinding()");
        boolean res = true;

        log.println("Calling method with wrong argument");
        try {
            oObj.doAccessibleAction(count);
            log.println("Exception expected -- FAILED");
            res &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException ioe) {
            log.println("Expected exception -- OK");
            res &= true;
        }

        try {
            boolean act = false;
            for (int i = 0; i< count; i++) {
                log.println("do Action "+ oObj.getAccessibleActionDescription(i));
                act = oObj.doAccessibleAction(i);
                log.println("Worked: "+act);
            }
            log.println("Did action: "+act);
            res &= act ;
        } catch (com.sun.star.lang.IndexOutOfBoundsException ioe) {
            log.println("Unexpected exception -- FAILED");
            res &= false;
        }

        tRes.tested("doAccessibleAction()",res);
    }

    /**
     * calls the method with invalid argument and check if the <br>
     * expected Exception is thrown.<br>
     * Calls the method afterwards all valid parameters.<br>
     * Is ok if the exception is thrown and the resulting value
     * for the calls with valid parameters aren't null.
     */

    public void _getAccessibleActionDescription() {
        requiredMethod("getAccessibleActionCount()");
        boolean res = true;

        log.println("Calling method with wrong argument");
        try {
            oObj.getAccessibleActionDescription(count);
            log.println("Exception expected -- FAILED");
            res &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException ioe) {
            log.println("Expected exception -- OK");
            res &= true;
        }

        for (int i=0;i<count;i++) {
            try {
                String desc = oObj.getAccessibleActionDescription(i);
                log.println("Found action: "+desc);
                res &= desc!=null ;
            } catch (com.sun.star.lang.IndexOutOfBoundsException ioe) {
                log.println("Unexpected exception -- FAILED");
                res &= false;
            }
        }

        tRes.tested("getAccessibleActionDescription()",res);
    }

    /**
     * calls the method with invalid argument and check if the <br>
     * expected Exception is thrown.<br>
     * Calls the method afterwards all valid parameters.<br>
     * Is ok if the exception is thrown and the resulting value
     * for the calls with valid parameters aren't null.
     */

    public void _getAccessibleActionKeyBinding() {
        requiredMethod("getAccessibleActionDescription()");
        boolean res = true;

        log.println("Calling method with wrong argument");
        try {
            oObj.getAccessibleActionKeyBinding(count);
            log.println("Exception expected -- FAILED");
            res &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException ioe) {
            log.println("Expected exception -- OK");
            res &= true;
        }

        for (int i=0;i<count;i++) {
            try {
                Object key = oObj.getAccessibleActionKeyBinding(i);
                if (key != null ) {
                    log.println("Found key: "+key.toString());
                }
                res &= true;
            } catch (com.sun.star.lang.IndexOutOfBoundsException ioe) {
                log.println("Unexpected exception -- FAILED");
                res &= false;
            }
        }

        tRes.tested("getAccessibleActionKeyBinding()",res);
    }

    /**
    * Forces environment recreation.
    */
    @Override
    protected void after() {
        disposeEnvironment();
    }

}

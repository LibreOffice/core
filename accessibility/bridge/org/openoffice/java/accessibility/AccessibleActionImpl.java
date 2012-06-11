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

package org.openoffice.java.accessibility;

import com.sun.star.accessibility.XAccessibleAction;

/** The AccessibleActionWrapper maps the calls to the java AccessibleAction interface
 *  to the corresponding methods of the UNO XAccessibleAction interface.
 */
public class AccessibleActionImpl implements javax.accessibility.AccessibleAction {

    protected XAccessibleAction unoObject;

    /** Creates new AccessibleActionWrapper */
    public AccessibleActionImpl(XAccessibleAction xAccessibleAction) {
        unoObject = xAccessibleAction;
    }

    public boolean doAccessibleAction(int param) {
        try {
            return unoObject.doAccessibleAction(param);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            return false;
        } catch (com.sun.star.uno.RuntimeException e) {
            return false;
        }
    }

    public java.lang.String getAccessibleActionDescription(int param) {
        try {
            return unoObject.getAccessibleActionDescription(param);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            return null;
        } catch (com.sun.star.uno.RuntimeException e) {
            return null;
        }
    }

    public int getAccessibleActionCount() {
        try {
            return unoObject.getAccessibleActionCount();
        } catch (com.sun.star.uno.RuntimeException e) {
            return 0;
        }
    }
}

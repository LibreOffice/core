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

import javax.accessibility.AccessibleRole;
import com.sun.star.accessibility.*;

public class Alert extends Dialog {

    protected Alert(java.awt.Frame owner, XAccessibleComponent xAccessibleComponent) {
        super(owner, xAccessibleComponent);
    }

    protected Alert(java.awt.Frame owner, String name, XAccessibleComponent xAccessibleComponent) {
        super(owner, name, xAccessibleComponent);
    }

    protected Alert(java.awt.Frame owner, String name, boolean modal, XAccessibleComponent xAccessibleComponent) {
        super(owner, name, modal, xAccessibleComponent);
    }

    /** Returns the AccessibleContext associated with this object */
    public javax.accessibility.AccessibleContext getAccessibleContext() {
        if (accessibleContext == null) {
            accessibleContext = new AccessibleAlert();
            accessibleContext.setAccessibleName(getTitle());
        }
        return accessibleContext;
    }

    protected class AccessibleAlert extends AccessibleDialog {

        protected AccessibleAlert() {
            super();
        }

        public AccessibleRole getAccessibleRole() {
            return AccessibleRole.ALERT;
        }
    }
}

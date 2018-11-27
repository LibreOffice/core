/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

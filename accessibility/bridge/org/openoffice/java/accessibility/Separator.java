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

import com.sun.star.accessibility.*;
import com.sun.star.uno.*;

import javax.accessibility.AccessibleState;
import javax.accessibility.AccessibleStateSet;

import javax.swing.SwingConstants;


/**
 */
public class Separator extends Component implements SwingConstants,
    javax.accessibility.Accessible {

    public Separator(XAccessible xAccessible,
        XAccessibleContext xAccessibleContext) {
        super(xAccessible, xAccessibleContext);
        setFocusable(false);
    }

    /** Creates the AccessibleContext associated with this object */
    public javax.accessibility.AccessibleContext createAccessibleContext() {
        return new AccessibleSeparator();
    }

    protected class AccessibleSeparator extends AccessibleUNOComponent {
        /**
        * Though the class is abstract, this should be called by all sub-classes
        */
        protected AccessibleSeparator() {
            super();
        }

        /*
        * AccessibleContext
        */

        /** Gets the role of this object */
        public javax.accessibility.AccessibleRole getAccessibleRole() {
            return javax.accessibility.AccessibleRole.SEPARATOR;
        }
    }
}

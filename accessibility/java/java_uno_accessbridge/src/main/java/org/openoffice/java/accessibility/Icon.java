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


/**
 */
public class Icon extends Component implements javax.accessibility.Accessible {
    protected Icon(XAccessible xAccessible, XAccessibleContext xAccessibleContext) {
        super(xAccessible, xAccessibleContext);
    }

    /** Creates the AccessibleContext associated with this object */
    public javax.accessibility.AccessibleContext createAccessibleContext() {
        return new AccessibleIcon();
    }

    protected class AccessibleIcon extends AccessibleUNOComponent {
        /**
        * Though the class is abstract, this should be called by all sub-classes
        */
        protected AccessibleIcon() {
            super();
        }

        /** Gets the AccessibleText associated with this object presenting text on the display */
        public javax.accessibility.AccessibleIcon[] getAccessibleIcon() {
            try {
                XAccessibleImage unoAccessibleImage = (XAccessibleImage) UnoRuntime.queryInterface(XAccessibleImage.class,
                        unoAccessibleComponent);

                if (unoAccessibleImage != null) {
                    javax.accessibility.AccessibleIcon[] icons = {
                        new AccessibleIconImpl(unoAccessibleImage)
                    };

                    return icons;
                }
            } catch (com.sun.star.uno.RuntimeException e) {
            }

            return null;
        }

    }
}

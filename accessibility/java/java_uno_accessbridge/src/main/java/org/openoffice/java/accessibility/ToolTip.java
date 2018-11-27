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
public class ToolTip extends Component implements javax.accessibility.Accessible {
    protected ToolTip(XAccessible xAccessible, XAccessibleContext xAccessibleContext) {
        super(xAccessible, xAccessibleContext);
    }

    /** Creates the AccessibleContext associated with this object */
    public javax.accessibility.AccessibleContext createAccessibleContext() {
        return new AccessibleToolTip();
    }

    protected class AccessibleToolTip extends AccessibleUNOComponent {

        /*
        * AccessibleContext
        */

        /** Gets the role of this object */
        public javax.accessibility.AccessibleRole getAccessibleRole() {
            return javax.accessibility.AccessibleRole.TOOL_TIP;
        }

        /** Gets the AccessibleText associated with this object presenting text on the display */
        public javax.accessibility.AccessibleText getAccessibleText() {

            if (disposed)
                return null;

            try {
                XAccessibleText unoAccessibleText = (XAccessibleText) UnoRuntime.queryInterface(XAccessibleText.class,
                        unoAccessibleComponent);

                if (unoAccessibleText != null) {
                    return new AccessibleTextImpl(unoAccessibleText);
                } else {
                    return null;
                }
            } catch (com.sun.star.uno.RuntimeException e) {
                return null;
            }
        }

        /** Returns the relation set of this object */

        /*
                        public javax.accessibility.AccessibleRelationSet getAccessibleRelationSet() {
                                try {
                                        XAccessibleRelationSet unoAccessibleRelationSet = unoAccessibleContext.getAccessibleRelationSet();
                                        if (unoAccessibleRelationSet == null) {
                                                return null;
                                        }

                                        javax.accessibility.AccessibleRelationSet relationSet = new javax.accessibility.AccessibleRelationSet();
                                        int count = unoAccessibleRelationSet.getRelationCount();
                                        for (int i = 0; i < count; i++) {
                                                AccessibleRelation unoAccessibleRelation = unoAccessibleRelationSet.getRelation(i);
                                                switch (unoAccessibleRelation.RelationType) {
                                                        case AccessibleRelationType.LABEL_FOR:
                                                                relationSet.add(new javax.accessibility.AccessibleRelation(
                                                                        javax.accessibility.AccessibleRelation.LABEL_FOR,
                                                                        getAccessibleComponents(unoAccessibleRelation.TargetSet)));
                                                                break;
                                                        case AccessibleRelationType.MEMBER_OF:
                                                                relationSet.add(new javax.accessibility.AccessibleRelation(
                                                                        javax.accessibility.AccessibleRelation.MEMBER_OF,
                                                                        getAccessibleComponents(unoAccessibleRelation.TargetSet)));
                                                                break;
                                                        default:
                                                                break;
                                                }
                                        }
                                        return relationSet;
                                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                                        return null;
                                } catch (com.sun.star.uno.RuntimeException e) {
                                        return null;
                                }
                        }
        */
    }
}

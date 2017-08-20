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

package com.sun.star.sdbcx.comp.postgresql.comphelper;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.UnoRuntime;

public class CompHelper {
    /**
     * If the given parameter is an XComponent, calls dispose() on it.
     * @param object the UNO interface to try dispose; may be null.
     */
    public static void disposeComponent(final Object object) {
        final XComponent component = UnoRuntime.queryInterface(XComponent.class, object);
        if (component != null) {
            component.dispose();
        }
    }

    public static void copyProperties(final XPropertySet src, final XPropertySet dst) {
        if (src == null || dst == null) {
            return;
        }

        XPropertySetInfo srcPropertySetInfo = src.getPropertySetInfo();
        XPropertySetInfo dstPropertySetInfo = dst.getPropertySetInfo();

        for (Property srcProperty : srcPropertySetInfo.getProperties()) {
            if (dstPropertySetInfo.hasPropertyByName(srcProperty.Name)) {
                try {
                    Property dstProperty = dstPropertySetInfo.getPropertyByName(srcProperty.Name);
                    if ((dstProperty.Attributes & PropertyAttribute.READONLY) == 0) {
                        Object value = src.getPropertyValue(srcProperty.Name);
                        if ((dstProperty.Attributes & PropertyAttribute.MAYBEVOID) == 0 || value != null) {
                            dst.setPropertyValue(srcProperty.Name, value);
                        }
                    }
                } catch (Exception e) {
                    String error = "Could not copy property '" + srcProperty.Name +
                            "' to the destination set";
                    XServiceInfo serviceInfo = UnoRuntime.queryInterface(XServiceInfo.class, dst);
                    if (serviceInfo != null) {
                        error += " (a '" + serviceInfo.getImplementationName() + "' implementation)";
                    }

                }
            }
        }
    }
}

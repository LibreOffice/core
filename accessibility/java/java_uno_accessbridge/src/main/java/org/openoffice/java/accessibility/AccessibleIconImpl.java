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

import com.sun.star.accessibility.XAccessibleImage;

/**
 */
public class AccessibleIconImpl implements javax.accessibility.AccessibleIcon {

    XAccessibleImage unoAccessibleImage;

    public AccessibleIconImpl(XAccessibleImage xImage) {
        unoAccessibleImage = xImage;
    }

    /** Gets the description of the icon */
    public String getAccessibleIconDescription() {
        try {
            return unoAccessibleImage.getAccessibleImageDescription();
        } catch (com.sun.star.uno.RuntimeException e) {
            return null;
        }
    }

    /** Gets the height of the icon */
    public int getAccessibleIconHeight() {
        try {
            return unoAccessibleImage.getAccessibleImageHeight();
        } catch (com.sun.star.uno.RuntimeException e) {
            return 0;
        }
    }

    /** Gets the width of the icon */
    public int getAccessibleIconWidth() {
        try {
            return unoAccessibleImage.getAccessibleImageWidth();
        } catch (com.sun.star.uno.RuntimeException e) {
            return 0;
        }
    }

    /** Sets the description of the icon */
    public void setAccessibleIconDescription(String s) {
        // Not supported
    }
}

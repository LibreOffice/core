/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AccessibleIconImpl.java,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

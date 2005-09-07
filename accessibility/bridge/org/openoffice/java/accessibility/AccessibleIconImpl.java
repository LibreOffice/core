/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleIconImpl.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:32:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Icon.java,v $
 * $Revision: 1.7 $
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

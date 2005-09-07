/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Icon.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:38:19 $
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

/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MemberDescriptionHelper.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 13:16:57 $
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

package com.sun.star.lib.uno.typedesc;

import com.sun.star.uno.ITypeDescription;
import com.sun.star.uno.TypeClass;

final class MemberDescriptionHelper {
    public static boolean isUnsigned(ITypeDescription desc) {
        switch (getElementTypeClass(desc).getValue()) {
        case TypeClass.UNSIGNED_SHORT_value:
        case TypeClass.UNSIGNED_LONG_value:
        case TypeClass.UNSIGNED_HYPER_value:
            return true;

        default:
            return false;
        }
    }

    public static boolean isAny(ITypeDescription desc) {
        return getElementTypeClass(desc) == TypeClass.ANY;
    }

    public static boolean isInterface(ITypeDescription desc) {
        return getElementTypeClass(desc) == TypeClass.INTERFACE;
    }

    private static TypeClass getElementTypeClass(ITypeDescription desc) {
        for (;; desc = desc.getComponentType()) {
            TypeClass tc = desc.getTypeClass();
            if (tc != TypeClass.SEQUENCE) {
                return tc;
            }
        }
    }

    private MemberDescriptionHelper() {} // do not instantiate
}

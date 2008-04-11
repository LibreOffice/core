/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MemberDescriptionHelper.java,v $
 * $Revision: 1.4 $
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

/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package com.sun.star.lib.uno.typedesc;

import com.sun.star.uno.TypeClass;

final class MemberDescriptionHelper {
    public static boolean isUnsigned(TypeDescription desc) {
        switch (getElementTypeClass(desc).getValue()) {
        case TypeClass.UNSIGNED_SHORT_value:
        case TypeClass.UNSIGNED_LONG_value:
        case TypeClass.UNSIGNED_HYPER_value:
            return true;

        default:
            return false;
        }
    }

    public static boolean isAny(TypeDescription desc) {
        return getElementTypeClass(desc) == TypeClass.ANY;
    }

    public static boolean isInterface(TypeDescription desc) {
        return getElementTypeClass(desc) == TypeClass.INTERFACE;
    }

    private static TypeClass getElementTypeClass(TypeDescription desc) {
        for (;; desc = desc.getComponentType()) {
            TypeClass tc = desc.getTypeClass();
            if (tc != TypeClass.SEQUENCE) {
                return tc;
            }
        }
    }

    private MemberDescriptionHelper() {} // do not instantiate
}

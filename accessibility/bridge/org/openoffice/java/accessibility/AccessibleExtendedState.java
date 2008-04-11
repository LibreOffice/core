/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AccessibleExtendedState.java,v $
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

import javax.accessibility.AccessibleState;

public class AccessibleExtendedState extends AccessibleState {
    public static final AccessibleState DEFUNCT;
    public static final AccessibleState INDETERMINATE;
    public static final AccessibleState MANAGES_DESCENDANTS;
    public static final AccessibleState SENSITIVE;
    public static final AccessibleState STALE;

    static {
        DEFUNCT = new AccessibleExtendedState("defunct");

        // JAVA 1.5: will come with manages_descendants and indeterminate
        INDETERMINATE = new AccessibleExtendedState("indeterminate");
        MANAGES_DESCENDANTS = new AccessibleExtendedState("managesDescendants");

        SENSITIVE = new AccessibleExtendedState("sensitive");
        STALE = new AccessibleExtendedState("stale");
    }

    protected AccessibleExtendedState(String key) {
        super(key);
    }
}

/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleExtendedState.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:31:23 $
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

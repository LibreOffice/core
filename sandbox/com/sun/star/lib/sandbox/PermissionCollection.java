/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PermissionCollection.java,v $
 * $Revision: 1.3 $
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

package com.sun.star.lib.sandbox;

import java.util.Vector;

public final class PermissionCollection {
    private Vector permissions = new Vector();

    public PermissionCollection() {
        SecurityManager security = System.getSecurityManager();
        if(security != null) security.checkSecurityAccess("PermissionCollection");
    }

    public PermissionCollection(PermissionCollection permissionCollection) {
        SecurityManager security = System.getSecurityManager();
        if(security != null) security.checkSecurityAccess("PermissionCollection");

        permissions = (Vector)permissionCollection.permissions.clone();
    }

//      public PermissionCollection(PermissionCollection p1, PermissionCollection p2) {
//          SecurityManager security = System.getSecurityManager();
//          if(security != null) security.checkSecurityAccess("PermissionCollection");

//          if(p1 != null)
//              this.permissions = (Vector)p1.permissions.clone();

//          if(p2 != null)
//              for(int i = 0; i < p2.permissions.size(); i ++)
//                  this.permissions.addElement(p2.permissions.elementAt(i));
//      }

    public void reset() {
        SecurityManager security = System.getSecurityManager();
        if(security != null) security.checkSecurityAccess("PermissionCollection");

        permissions.removeAllElements();
    }

    public void add(Permission permission) {
        SecurityManager security = System.getSecurityManager();
        if(security != null) security.checkSecurityAccess("PermissionCollection");

        permissions.addElement(permission);
    }

    public boolean implies(Permission permission) {
        boolean granted = false;

        for(int i = 0; i < permissions.size() && !granted; i++) {
            granted = ((Permission)permissions.elementAt(i)).implies(permission);
        }

        return granted;
    }
}


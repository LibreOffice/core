/*************************************************************************
 *
 *  $RCSfile: PermissionCollection.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:24:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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


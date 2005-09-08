/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ProtectionDomain.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:07:19 $
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

package com.sun.star.lib.sandbox;

import java.security.Identity;

import sun.security.provider.SystemIdentity;
import sun.security.provider.SystemSigner;

public final class ProtectionDomain {
    private PermissionCollection permissionCollection;
    private CodeSource codeSource;

    public ProtectionDomain(CodeSource codeSource, PermissionCollection permissionCollection) {
//          this.permissionCollection = new PermissionCollection(codeSource.getPermissionCollection(), permissionCollection);
        SecurityManager security = System.getSecurityManager();
        if(security != null) security.checkSecurityAccess("ProtectionDomain");

        this.permissionCollection = permissionCollection;
        this.codeSource           = codeSource;
    }

    public PermissionCollection getPermissions() {
        SecurityManager security = System.getSecurityManager();
        if(security != null) security.checkSecurityAccess("ProtectionDomain");

        return permissionCollection;
    }

    boolean isTrusted() {
        Identity identities[] = codeSource.identities;

        boolean trusted = false;

        if(identities != null) {
            for(int i = 0; i < identities.length && !trusted; i++)
                System.out.println("XXX:" + identities[i].toString(true));

            for(int i = 0; i < identities.length; i ++) {
                if(identities[i] instanceof SystemIdentity) {
                    SystemIdentity systemIdentity = (SystemIdentity)identities[i];

                    trusted = systemIdentity.isTrusted();
                }
                else if(identities[i] instanceof SystemSigner) {
                    SystemSigner systemSigner = (SystemSigner)identities[i];

                    trusted = systemSigner.isTrusted();
                }
            }
        }
        return trusted;
    }

    boolean implies(Permission permission) {
        return isTrusted() || permissionCollection.implies(permission);
    }
}


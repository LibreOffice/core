/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ProtectionDomain.java,v $
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


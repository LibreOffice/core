/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: remote_environment.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:02:51 $
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

package com.sun.star.lib.uno.environments.remote;

import com.sun.star.uno.IEnvironment;
import com.sun.star.uno.Type;

public final class remote_environment implements IEnvironment {
    public remote_environment(Object context) {
        this.context = context;
    }

    public Object getContext() {
        return context;
    }

    public String getName() {
        return "remote";
    }

    public Object registerInterface(Object object, String[] oid, Type type) {
        throw new UnsupportedOperationException(
            "java_remote environment is not functional");
    }

    public void revokeInterface(String oid, Type type) {
        throw new UnsupportedOperationException(
            "java_remote environment is not functional");
    }

    public Object getRegisteredInterface(String oid, Type type) {
        throw new UnsupportedOperationException(
            "java_remote environment is not functional");
    }

    public String getRegisteredObjectIdentifier(Object object) {
        throw new UnsupportedOperationException(
            "java_remote environment is not functional");
    }

    public void list() {
        throw new UnsupportedOperationException(
            "java_remote environment is not functional");
    }

    private final Object context;
}

/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Resolver.java,v $
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

package com.sun.star.script.framework.provider.java;

/**
 * The Resolver interface is an interface common to all classes which
 * want to implement an algorithm for obtaining a ScriptProxy object
 * for a particular ScriptDescriptor and Class
 *
 * @author     Tomas O'Connor
 * @created    August 2, 2002
 */

public interface Resolver {
    /**
     * Returns a ScriptProxy object for the given ScriptDescriptor and Class
     *
     * @param  sd  A script Descriptor
     * @param  c   A Class
     * @return     The ScriptProxy value
     */
    public ScriptProxy getProxy( ScriptDescriptor sd, Class c )
    throws NoSuchMethodException;
}


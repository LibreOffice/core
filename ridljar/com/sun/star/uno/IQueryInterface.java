/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: IQueryInterface.java,v $
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

package com.sun.star.uno;

/**
 * This is the delegator interface for Java objects implementing interfaces of
 * an underlying UNO object.
 *
 * <p>Calls are delegated through the <code>UnoRuntime</code> to this
 * interface.  Implement this interface in case you want to customize the
 * behaviour of <code>UnoRuntime.queryInterface</code>.</p>
 *
 * @see com.sun.star.uno.UnoRuntime
 */
public interface IQueryInterface {
    /**
     * Returns the unique object identifier (OID) of the underlying UNO object.
     *
     * @return the OID of the underlying object
     */
    String getOid();

    /**
     * Returns an object implementing the requested interface type.
     *
     * @param type the requested UNO interface type; must be a <code>Type</code>
     * object representing a UNO interface type
     * @return a reference to the requested UNO interface type if available,
     * otherwise <code>null</code>
     * @see com.sun.star.uno.UnoRuntime
     */
    Object queryInterface(Type type);

    /**
     * Tests if the given reference represents a facet of the underlying UNO
     * object.
     *
     * @param object a reference to any Java object representing (a facet of) a
     * UNO object; may be <code>null</code>
     * @return <code>true</code> if and only if <code>object</code> is not
     * <code>null</code> and represents the same UNO object as this object
     */
    boolean isSame(Object object);
}

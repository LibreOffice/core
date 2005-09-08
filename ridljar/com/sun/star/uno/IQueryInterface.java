/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IQueryInterface.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 13:23:41 $
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

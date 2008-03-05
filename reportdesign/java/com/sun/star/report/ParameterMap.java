/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ParameterMap.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:24:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *    Copyright 2007 by Pentaho Corporation
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
package com.sun.star.report;

public interface ParameterMap
{

    /**
     * Adds a property to this properties collection. If a property with the given name
     * exist, the property will be replaced with the new value. If the value is null, the
     * property will be removed.
     *
     * @param key   the property key.
     * @param value the property value.
     */
    public void put(final String key, final Object value);

    /**
     * Retrieves the value stored for a key in this properties collection.
     *
     * @param key the property key.
     * @return The stored value, or <code>null</code> if the key does not exist in this
     *         collection.
     */
    public Object get(final String key);

    /**
     * Retrieves the value stored for a key in this properties collection, and returning the
     * default value if the key was not stored in this properties collection.
     *
     * @param key          the property key.
     * @param defaultValue the default value to be returned when the key is not stored in
     *                     this properties collection.
     * @return The stored value, or the default value if the key does not exist in this
     *         collection.
     */
    public Object get(final String key, final Object defaultValue);

    public String[] keys();

    public void clear();

    public int size();
}

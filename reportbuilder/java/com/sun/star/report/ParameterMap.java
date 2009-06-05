/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ParameterMap.java,v $
 * $Revision: 1.4 $
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
    Object get(final String key);

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
    Object get(final String key, final Object defaultValue);

    String[] keys();

    void clear();

    int size();
}

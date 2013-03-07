/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package org.libreoffice.report;

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

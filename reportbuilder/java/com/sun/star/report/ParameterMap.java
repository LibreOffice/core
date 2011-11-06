/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

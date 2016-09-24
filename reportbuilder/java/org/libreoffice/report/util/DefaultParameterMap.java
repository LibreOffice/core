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
package org.libreoffice.report.util;

import org.libreoffice.report.ParameterMap;

import java.util.HashMap;
import java.util.Map;


public class DefaultParameterMap implements ParameterMap
{

    private final Map<String,Object> backend;

    public DefaultParameterMap()
    {
        backend = new HashMap<String,Object>();
    }

    /**
     * Retrieves the value stored for a key in this properties collection.
     *
     * @param key the property key.
     * @return The stored value, or <code>null</code> if the key does not exist in this
     *         collection.
     */
    public Object get(final String key)
    {
        if (key == null)
        {
            throw new NullPointerException("DefaultParameterMap.get (..): Parameter 'key' must not be null");
        }
        return backend.get(key);
    }

    public String[] keys()
    {
        return this.backend.keySet().toArray(new String[backend.size()]);
    }

}

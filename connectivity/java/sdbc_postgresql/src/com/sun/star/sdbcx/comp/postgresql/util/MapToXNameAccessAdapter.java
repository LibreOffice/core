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

package com.sun.star.sdbcx.comp.postgresql.util;

import java.util.Map;

import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.uno.Type;

public class MapToXNameAccessAdapter extends WeakBase implements XNameAccess {
    protected final Map<String,Object> map;
    protected final Object lock;
    private final Type elementType;

    public MapToXNameAccessAdapter(Map<String,Object> map, Object lock, Type elementType) {
        this.map = map;
        this.lock = lock;
        this.elementType = elementType;
    }

    // XNameAccess:

    @Override
    public Object getByName(String key)
            throws NoSuchElementException, WrappedTargetException {
        Object object;
        synchronized (lock) {
            object = map.get(key);
        }
        if (object == null) {
            throw new NoSuchElementException();
        }
        return object;
    }

    @Override
    public String[] getElementNames() {
        synchronized (lock) {
            String[] names = new String[map.size()];
            int next = 0;
            for (Map.Entry<String,Object> entry : map.entrySet()) {
                names[next++] = entry.getKey().toString();
            }
            return names;
        }
    }

    @Override
    public boolean hasByName(String key) {
        synchronized (lock) {
            return map.containsKey(key);
        }
    }

    // XElementAccess:

    @Override
    public Type getElementType() {
        return elementType;
    }


    @Override
    public boolean hasElements() {
        synchronized (lock) {
            return !map.isEmpty();
        }
    }
}

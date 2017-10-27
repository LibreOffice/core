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

import com.sun.star.container.ElementExistException;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.uno.Type;

public class MapToXNameContainerAdapter extends MapToXNameAccessAdapter implements XNameContainer {
    public MapToXNameContainerAdapter(Map<String,Object> map, Object lock, Type elementType) {
        super(map, lock, elementType);
    }

    // XNameContainer:

    @Override
    public void insertByName(String key, Object value)
            throws IllegalArgumentException, ElementExistException,
            WrappedTargetException {
        synchronized (lock) {
            if (map.containsKey(key)) {
                throw new ElementExistException();
            }
            map.put(key, value);
        }
    }

    @Override
    public void removeByName(String key)
            throws NoSuchElementException, WrappedTargetException {
        synchronized (lock) {
            if (map.containsKey(key)) {
                map.remove(key);
            } else {
                throw new NoSuchElementException();
            }
        }
    }

    // XNameReplace:

    @Override
    public void replaceByName(String key, Object value)
            throws IllegalArgumentException, NoSuchElementException,
            WrappedTargetException {
        synchronized (lock) {
            if (!map.containsKey(key)) {
                throw new NoSuchElementException();
            }
            map.put(key, value);
        }
    }
}

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


/*
 * A special HashMap,
 * can be constructed of
 * two Arrays
 */
package com.sun.star.tooling.converter;

import java.util.HashMap;
import java.util.Map;

/**
 * @author Christian Schmidt
 *
 * Create a Hash Map from two Arrays
 *
 */
public class ExtMap extends HashMap {

    /**
     *
     */
    public ExtMap() {
        super();

    }

    /**
     * @see java.util.HashMap
     * @param arg0
     */
    public ExtMap(int arg0) {
        super(arg0);

    }

    /**
     * @param arg0
     * @param arg1
     */
    public ExtMap(int arg0, float arg1) {
        super(arg0, arg1);

    }

    /**
     * @param arg0
     */
    public ExtMap(Map arg0) {
        super(arg0);

    }

    // create a new Map from two string arrays
    public ExtMap(String[] names, String[] content) {
        super(names.length);
        if (content == null)
            content = new String[names.length];
        for (int i = 0; i < names.length; i++) {
            if (i >= content.length) {
                break;
            } else {
                this.put(names[i], content[i]);
            }
        }

    }

}
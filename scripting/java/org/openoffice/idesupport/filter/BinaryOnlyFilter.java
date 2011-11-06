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



package org.openoffice.idesupport.filter;

public class BinaryOnlyFilter implements FileFilter {
    private static final String[] EXTENSIONS = {".class", ".jar", ".bsh"};
    private static final String DESCRIPTION = "Executable Files Only";
    private static final BinaryOnlyFilter filter = new BinaryOnlyFilter();

    private BinaryOnlyFilter() {
    }

    public static BinaryOnlyFilter getInstance() {
        return filter;
    }
    public boolean validate(String name) {
        for (int i = 0; i < EXTENSIONS.length; i++)
            if (name.endsWith(EXTENSIONS[i]))
                return true;
        return false;
    }

    public String toString() {
        /* StringBuffer buf = new StringBuffer(DESCRIPTION + ": ");

        for (int i = 0; i < EXTENSIONS.length - 1; i++)
            buf.append("<" + EXTENSIONS[i] + "> ");
        buf.append("<" + EXTENSIONS[EXTENSIONS.length - 1] + ">");

        return buf.toString(); */
        return DESCRIPTION;
    }
}

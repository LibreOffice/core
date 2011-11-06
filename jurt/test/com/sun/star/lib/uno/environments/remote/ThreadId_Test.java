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



package com.sun.star.lib.uno.environments.remote;

import complexlib.ComplexTestCase;
import java.util.Arrays;

public final class ThreadId_Test extends ComplexTestCase {
    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() {
        ThreadId i1 = ThreadId.createFresh();
        assure(i1.equals(i1));
        assure(!i1.equals(null));
        assure(!i1.equals(new Object()));
        assure(i1.hashCode() == i1.hashCode());
        byte[] i1bytes = i1.getBytes();
        assure(i1bytes != null);
        assure(
            i1bytes.length >= 5 && i1bytes[0] == 'j' && i1bytes[1] == 'a'
            && i1bytes[2] == 'v' && i1bytes[3] == 'a' && i1bytes[4] == ':');
        assure(Arrays.equals(i1bytes, i1.getBytes()));

        ThreadId i2 = ThreadId.createFresh();
        assure(!i1.equals(i2));
        assure(!i2.equals(i1));
        assure(!Arrays.equals(i1bytes, i2.getBytes()));

        ThreadId i3 = new ThreadId(i1bytes);
        assure(i3.equals(i1));
        assure(!i3.equals(i2));
        assure(i1.equals(i3));
        assure(i1.hashCode() == i3.hashCode());
        assure(Arrays.equals(i1bytes, i3.getBytes()));
    }
}

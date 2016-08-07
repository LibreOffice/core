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



package org.openoffice.starmath.qa.unoapi;

import org.openoffice.Runner;
import org.openoffice.test.OfficeConnection;
import org.openoffice.test.Argument;
import static org.junit.Assert.*;

public final class Test {
    @org.junit.Before public void setUp() throws Exception {
        connection.setUp();
    }

    @org.junit.After public void tearDown()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        connection.tearDown();
    }

    @org.junit.Test public void test() {
        assertTrue(
            Runner.run(
                "-sce", Argument.get("sce"), "-xcl", Argument.get("xcl"),
                "-cs", connection.getDescription()));
    }

    private final OfficeConnection connection = new OfficeConnection();
}

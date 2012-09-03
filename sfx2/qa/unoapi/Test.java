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

package unoapi;

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
                "-sce", Argument.get("sce"), "-xcl", Argument.get("xcl"), "-tdoc",
                Argument.get("tdoc"), "-cs", connection.getDescription()));
    }

    private final OfficeConnection connection = new OfficeConnection();
}

/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2010 Oracle and/or its affiliates.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* This file is part of OpenOffice.org.
*
* OpenOffice.org is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License version 3
* only, as published by the Free Software Foundation.
*
* OpenOffice.org is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License version 3 for more details
* (a copy is included in the LICENSE file that accompanied this code).
*
* You should have received a copy of the GNU Lesser General Public License
* version 3 along with OpenOffice.org.  If not, see
* <http://www.openoffice.org/license.html>
* for a copy of the LGPLv3 License.
************************************************************************/

package unoapi.dbaccess;

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
                "-sce", Argument.get("sce"),
                "-xcl", Argument.get("xcl"),
                "-ini", Argument.get("ini"),
                "-tdoc", Argument.get("tdoc"),
                "-cs", connection.getDescription()));
    }

    private final OfficeConnection connection = new OfficeConnection();
}

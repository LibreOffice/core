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

package org.openoffice.testgraphical.qa.graphical;

import org.openoffice.Runner;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

import helper.ProcessHandler;
import graphical.FileHelper;
import java.io.File;

public final class Test {
    @org.junit.Before
    public void setUp() throws Exception
        {
            connection.setUp();
        }

    @org.junit.After
    public void tearDown() throws Exception
        {
            connection.tearDown();
        }

    @org.junit.Test
    public void test()
        {
            boolean good = true;

            final String sPerlEXE = System.getenv("PERL");
            // System.out.println("PERL:=" + sPerlEXE);

            final String sPRJ = System.getenv("PRJ");
            // System.out.println("PRJ:=" + sPRJ);

            String sShow = "";
            if (System.getProperty("SHOW") != null)
            {
                sShow = "-show";
            }

            final String sComparePath = FileHelper.appendPath(sPRJ, "source");
            final String sCompareName = FileHelper.appendPath(sComparePath, "compare.pl");

            File aCompareFile = new File(sCompareName);
            if (!aCompareFile.exists())
            {
                System.out.println("Path to compare.pl is wrong: '" + aCompareFile.getAbsolutePath() + "'");
                assertTrue(false);
            }

            final String sConnectionString = connection.getDescription();

            String[] sCommandArray =
                {
                    sPerlEXE,
                    aCompareFile.getAbsolutePath(),
                    "-creatortype", "pdf",

// If you make changes here, do it also in ../../source/makefile.mk in selftest: target!

                    "-pool",        "singletest",
                    "-document",    "eis-test.odt",
                    // "-pool",        "demo",
                    // "-document",    "CurrentTime.ods",
                    "-connectionstring", sConnectionString,
                    // "-verbose",
                    sShow
                };

            ProcessHandler aHandler = new ProcessHandler(sCommandArray);
            boolean bBackValue = aHandler.executeSynchronously();
            int nExitCode = aHandler.getExitCode();

            // String sBack = aHandler.getOutputText();
            if (nExitCode != 0)
            {
                good = false;
            }

            assertTrue(good);

            // Runner.run(
            //     "-sce", "sw.sce", "-xcl", "knownissues.xcl", "-tdoc",
            //     "testdocuments", "-cs", connection.getDescription()));
    }

    private final OfficeConnection connection = new OfficeConnection();
}

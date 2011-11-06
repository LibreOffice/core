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

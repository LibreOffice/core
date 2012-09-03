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
package cli;


import complexlib.ComplexTestCase;
import java.io.*;

public class CLITest extends ComplexTestCase
{
    public String[] getTestMethodNames()
    {
        // TODO think about trigger of sub-tests from outside
        return new String[]
        {
            "runCLITests"
        };
    }

    public void runCLITests()
    {
        try
        {
            String testProgram = System.getProperty("cli_test", "");
            if (testProgram.length() == 0)
                failed("Check the make file. Java must be called with -Dcli_test=pathtoexe");

            String arg1 = System.getProperty("cli_test_arg", "");
            if (arg1.length() == 0)
                failed("Check the make file. Java must be called with " +
                       "-Dcli_test_arg=path_to_bootstrap_ini");
            String[] cmdarray = new String[] {testProgram, arg1};

            Process proc = null;
            Reader outReader;
            Reader errReader;
            try{

                proc = Runtime.getRuntime().exec(cmdarray);
                outReader = new Reader(proc.getInputStream());
                errReader = new Reader(proc.getErrorStream());


            }
            catch(Exception e)
            {
                System.out.println("\n ###" +  e.getMessage() + "\n");

            }
//           System.out.println("### waiting for " + testProgram);
            proc.waitFor();
//            System.out.println("### " + testProgram + " finished");
            int retVal = proc.exitValue();
            if (retVal != 0)
                failed("CLI test failed.");
        } catch( java.lang.Exception e)
        {
            failed("Unexpected exception.");
        }

    }
}


/*  This reads reads from an InputStream and discards the data.
 */
class Reader extends Thread
{
    InputStream is;
    public Reader(InputStream stream)
    {
        is = stream;
        start();
    }

    public void run()
    {
        try
        {
            byte[] buf = new byte[1024];
            while (-1 != is.read(buf));
        }
        catch (java.io.IOException exc)
        {
        }
    }
}

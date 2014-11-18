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
package climaker;


import complexlib.ComplexTestCase;


public class ClimakerTestCase extends ComplexTestCase
{
    @Override
    public String[] getTestMethodNames()
    {
        // TODO think about trigger of sub-tests from outside
        return new String[]
        {
            "checkGeneratedCLITypes"
        };
    }

    public void checkGeneratedCLITypes()
    {
        try
        {
            String testProgram = System.getProperty("cli_ure_test");
            if (testProgram == null || testProgram.length() == 0)
                failed("Check the make file. Java must be called with -Dcli_ure_test=pathtoexe");
            Process proc = null;
            try{

             proc = Runtime.getRuntime().exec(testProgram);
             new Reader(proc.getInputStream());
             new Reader(proc.getErrorStream());

            } catch(Exception e)
            {
                System.out.println("\n ###" +  e.getMessage() + "\n");

            }
            proc.waitFor();
            int retVal = proc.exitValue();
            if (retVal != 0)
                failed("Tests for generated CLI code failed.");
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
    private final java.io.InputStream is;
    public Reader(java.io.InputStream stream)
    {
        is = stream;
        start();
    }

    @Override
    public void run()
    {
        try
        {
            byte[] buf = new byte[1024];
            while (-1 != is.read(buf)) {}
        }
        catch (java.io.IOException exc)
        {
        }
    }
}

/*************************************************************************
 *
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
 *
 ************************************************************************/
package climaker;


import complexlib.ComplexTestCase;


public class ClimakerTestCase extends ComplexTestCase
{
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
             Reader outReader = new Reader(proc.getInputStream());
             Reader errReader = new Reader(proc.getErrorStream());

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
    java.io.InputStream is;
    public Reader(java.io.InputStream stream)
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

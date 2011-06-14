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
package cliversion;


import complexlib.ComplexTestCase;


public class VersionTestCase extends ComplexTestCase
{
    public String[] getTestMethodNames()
    {
        return new String[]
        {
            "checkVersion"
        };
    }

    public void checkVersion()
    {
        int retVal = 0;
        try
        {
            String testProgram = System.getProperty("cli_test_program");
            if (testProgram == null || testProgram.length() == 0)
                failed("Check the make file. Java must be called with -Dcli_ure_test=pathtoexe");
            String unoPath = System.getProperty("path");
            if (unoPath == null || unoPath.length() == 0)
                failed("Check the make file. Java must be called with -Duno_path=path_to_ure_bin_folder");
            String sSystemRoot = System.getProperty("SystemRoot");
            if (sSystemRoot == null || sSystemRoot.length() == 0)
                failed("Check the make file. Java  must be called with -DSystemRoot=%SystemRoot%.");

//            System.out.println("UNO_PATH="+unoPath);
            //We need to set the PATH because otherwise it appears that runtests inherits the PATH
            //from build environment. Then the bootstrapping fails because the libraries
            //are not used from the office.
            //.NET 2 requires SystemRoot being set.
            String[] arEnv = new String[] {
                    "PATH=" + unoPath, "SystemRoot=" + sSystemRoot};
            Process proc = null;

            proc = Runtime.getRuntime().exec(testProgram, arEnv);
            Reader outReader = new Reader(proc.getInputStream());
            Reader errReader = new Reader(proc.getErrorStream());
            proc.waitFor();
            retVal = proc.exitValue();
        } catch(Exception e)
        {
            e.printStackTrace();
            System.out.println(e.getMessage());
            failed("Unexpected exception.");
        }
        if (retVal != 0)
            failed("Tests for library versioning failed.");
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

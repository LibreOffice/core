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
package cliversioning;


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

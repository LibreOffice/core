/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VersionTestCase.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-25 07:52:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
            String unoPath = System.getProperty("uno_path");
            if (unoPath == null || unoPath.length() == 0)
                failed("Check the make file. Java must be called with -Duno_path=path_to_program_directory");
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

/*************************************************************************
 *
 *  $RCSfile: CLITest.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2005-09-23 11:47:30 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
package clitest;


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

/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: runtests.cs,v $
 * $Revision: 1.2 $
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

using System;
using System.Reflection;
using System.IO;

// __________  implementation  ____________________________________

/** Create and modify a spreadsheet document.
 */
namespace cliversion
{
public class RunTests
{

    public static int Main(String[] args)
    {
//        System.Diagnostics.Debugger.Launch();
        //get the path to the directory
        string sLocation = Assembly.GetExecutingAssembly().Location;
        sLocation = sLocation.Substring(0, sLocation.LastIndexOf('\\'));
        // Create a reference to the current directory.
        DirectoryInfo di = new DirectoryInfo(sLocation);
        // Create an array representing the files in the current directory.
        FileInfo[] fi = di.GetFiles();

        //For every found dll try to figure out if it contains a
        //cliversion.Version class
		foreach (FileInfo fiTemp in fi)
		{
			if (fiTemp.Extension != ".dll" 
                || ! fiTemp.Name.StartsWith("version"))
				continue;

            Assembly ass = null;
            Object objVersion = null;
			try
			{
                string sName = fiTemp.Name.Substring(0, fiTemp.Name.LastIndexOf(".dll"));
				ass = Assembly.Load(sName);
			}
			catch (BadImageFormatException)
			{
				continue;
			}
			catch (Exception e)
			{
				Console.WriteLine("#Unexpected Exception");
				Console.WriteLine(e.Message);
                return -1;
			}

            //Assembly is loaded, instantiate cliversion.Version
			try
			{
				//This runs the test
				objVersion = ass.CreateInstance("cliversion.Version");
				if (objVersion == null)
					continue;
				Console.WriteLine("#Tested successfully " + fiTemp.Name);
				//Starting the office the second time may fail without this pause
				System.Threading.Thread.Sleep(2000);
			}
			catch (Exception e)
			{
				TargetInvocationException te = e as TargetInvocationException;
				if (te != null)
				{
					FileNotFoundException fe = e.InnerException as FileNotFoundException;
					if (fe != null)
					{
						Console.WriteLine(fiTemp.Name + " did not find " + fe.FileName +
							". Maybe the " + fe.FileName + " is not installed or does not match the referenced version."  +
							"Original message: " + fe.Message + "\n\n FusionLog: \n" + fe.FusionLog );
						return -1;
					}
					FileLoadException fl = e.InnerException as FileLoadException;
					if (fl != null)
					{
						Console.WriteLine(fiTemp.Name + " could not load " + fl.FileName +
							". Maybe the version of " + fl.FileName + " does not match the referenced version. " +
							"Original message: " + fl.Message + "\n\n FusionLog: \n" + fl.FusionLog );
						return -1;
					}

                    if (e.InnerException != null)
                    {
                        Console.WriteLine(e.InnerException);
                    }
				}
				Console.WriteLine("#Unexpected Exception");
				Console.WriteLine(e.Message);
				return -1;
			}
		}
        //For some unknown reason this program hangs sometimes when started from java. This is
        //a workaround that makes the problem disappear.
        System.Threading.Thread.Sleep(1000);
        return 0;
    }
}
}

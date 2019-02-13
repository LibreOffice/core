//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file incorporates work covered by the following license notice:
//
//   Licensed to the Apache Software Foundation (ASF) under one or more
//   contributor license agreements. See the NOTICE file distributed
//   with this work for additional information regarding copyright
//   ownership. The ASF licenses this file to you under the Apache
//   License, Version 2.0 (the "License"); you may not use this file
//   except in compliance with the License. You may obtain a copy of
//   the License at http://www.apache.org/licenses/LICENSE-2.0 .
//
// This script asks for variables, which are necessary for building the
// examples of the Office Development Kit. The script duplicates the template
// script and inserts the variables into the copied script.
// The Script was developed for the operating systems Microsoft Windows.
var regKeyOfficeCurrentUser = "HKEY_CURRENT_USER\\Software\\LibreOffice\\UNO\\InstallPath\\";
var regKeyOfficeLocaleMachine = "HKEY_LOCAL_MACHINE\\Software\\LibreOffice\\UNO\\InstallPath\\";
var regKeyDotNetInstallRoot = "HKEY_LOCAL_MACHINE\\Software\\Microsoft\\.NETFramework\\InstallRoot";
var regKeyDotNet1_1 = "HKLM\\Software\\Microsoft\\.NETFramework\\policy\\v1.1\\4322";
var sDirDotNet1_1 = "v1.1.4322";
var regKeyDotNet2_0 = "HKLM\\Software\\Microsoft\\.NETFramework\\Policy\\v2.0\\50727";
var sDirDotNet2_0 = "v2.0.50727";

var regKeyJDK = "HKLM\\Software\\JavaSoft\\Java Development Kit\\";
//var regKeyVC70 = "HKLM\\SOFTWARE\\Microsoft\\VisualStudio\\7.0\\Setup\\VC\\ProductDir";
//var regKeyVC71 = "HKLM\\SOFTWARE\\Microsoft\\VisualStudio\\7.1\\Setup\\VC\\ProductDir";
var regKeyVC90 = "HKLM\\SOFTWARE\\Microsoft\\VisualStudio\\9.0\\Setup\\VC\\ProductDir";

//var regKeyVCExpress80 = "HKLM\\SOFTWARE\\Microsoft\\VCExpress\\8.0\\Setup\\VC\\ProductDir";
var regKeyVCExpress90 = "HKLM\\SOFTWARE\\Microsoft\\VCExpress\\9.0\\Setup\\VC\\ProductDir";

var regKeyWindowsSDK = "HKLM\\SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\CurrentInstallFolder";

var WshShell = WScript.CreateObject("WScript.Shell");
var WshSysEnv = WshShell.Environment("process");
var aFileSystemObject =  new ActiveXObject("Scripting.FileSystemObject");
var stdin = WScript.StdIn;
var stdout = WScript.StdOut;

stdout.WriteLine("\n" +
" *** Configure your SDK environment ***\n\n" +
" NOTE: This script is working only for Windows 2000, Windows XP or newer versions!\n");

var oo_sdk_name=WshSysEnv("OO_SDK_NAME");
var oo_sdk_home=getSdkHome();
var oo_user_sdk_dir=WshSysEnv("APPDATA") + "\\" + oo_sdk_name;
var oo_user_sdk_env_script=oo_user_sdk_dir + "\\setsdkenv_windows.bat";

var office_home=getOfficeHome();

var oo_sdk_make_home=getMakeHome();
var oo_sdk_zip_home=getZipHome();
var oo_sdk_cat_home=getCatHome();
var oo_sdk_sed_home=getSedHome();
var oo_sdk_manifest_used="";
var oo_sdk_windowssdk="";
var oo_sdk_cpp_home=getCppHome();
var oo_sdk_cli_home=getCliHome();
var oo_sdk_java_home=getJavaHome();
var oo_sdk_out=getOutputDir();
var sdk_auto_deployment=getAutoDeployment();

writeBatFile(oo_user_sdk_dir, oo_user_sdk_env_script);

stdout.Write(
	"\n ******************************************************************\n" +
    " * ... \"" + oo_user_sdk_env_script + "\"\n" +
    " * batch file has been prepared.\n" +
    " * This batch file will be used in the future to prepare your\n" +
	" * personal configured SDK environment.\n" +
	" ******************************************************************\n\n");
//    " * For each time you want to use this configured SDK environment,\n" +
//    " * you have to run the \"setsdkenv_windows.bat\" file in a new shell!\n" +

// done -------------------------------------------------------------------------


function skipChoice(msg)
{
	stdout.Write("\n Do you want to skip the choice of " + msg + " (YES/NO) [YES]:");
	var sChoice = stdin.ReadLine();
    if (sChoice == "" || sChoice.toLowerCase() == "yes")
	   return true;

	return false;
}

function getSdkHome()
{
	var sSuggestedHome = WshSysEnv("OO_SDK_HOME");
    if (sSuggestedHome.length == 0) {
        var scriptname = WScript.ScriptFullName;
        sSuggestedHome = scriptname.substr(0,scriptname.length-10);
    }

    while(true)
    {
        stdout.Write("\n Enter the Office Software Development Kit directory [" +
                     sSuggestedHome + "]:");
        var sHome = stdin.ReadLine();
        if (sHome.length == 0)
        {
            //No user input, use default.
            if ( ! aFileSystemObject.FolderExists(sSuggestedHome))
            {
                stdout.WriteLine("\n Error: Could not find directory \"" +
                                 sSuggestedHome + "\". An SDK is required, please" +
								 " specify the path to a valid installation.");
                continue;
            }
            sHome = sSuggestedHome;
        }
        else
        {
            //validate the user input
            if ( ! aFileSystemObject.FolderExists(sHome))
            {
                stdout.WriteLine("\n Error: The directory \"" + sHome +
                                 "\" does not exist. Please enter the path to a" +
								 "valid SDK installation.");
                continue;
            }
        }
        //Check if this is an sdk folder by looking for the idl sub - dir
        var idlDir = sHome + "\\idl";
        if (! aFileSystemObject.FolderExists(idlDir))
        {
            stdout.WriteLine("\n Error: Could not find directory \"" +
                             idlDir + "\". An SDK is required, please specify " +
                             "the path to a valid SDK installation.");
            continue;
        }

        return sHome;
    }
}

function getOfficeHome()
{
    var sSuggestedHome = WshSysEnv("OFFICE_HOME");
    if (sSuggestedHome.length == 0)
    {
        try {
            sSuggestedHome = WshShell.RegRead(regKeyOfficeCurrentUser);
			//The registry entry points to the program folder but we need the
			//installation folder
        } catch(exc) {}
        if (sSuggestedHome.length == 0)
        {
            try {
                sSuggestedHome = WshShell.RegRead(regKeyOfficeLocaleMachine);
                //The registry entry points to the program folder but we need
				//the installation folder
            } catch (exc) {}
        }

		var index=0;
        if ((index = sSuggestedHome.lastIndexOf("\\")) != -1)
		   sSuggestedHome = sSuggestedHome.substr(0, index);

        if (sSuggestedHome.length == 0)
		   sSuggestedHome = searchOffice();
    }

    while(true)
    {
        stdout.Write("\n Enter the Office base installation directory [" +
                     sSuggestedHome + "]:");
        var sHome = stdin.ReadLine();
        if (sHome.length == 0)
        {
            //No user input, use default.
            if ( ! aFileSystemObject.FolderExists(sSuggestedHome))
            {
                stdout.WriteLine("\n Error: Could not find directory \"" +
                                 sSuggestedHome + "\" An office installation is " +
								 "required, please specify the path to a valid " +
								 "office installation.");
                sSuggestedHome = "";
                continue;
            }
            sHome = sSuggestedHome;
        } else
        {
            //validate the user input
            if ( ! aFileSystemObject.FolderExists(sHome))
            {
                stdout.WriteLine("\n Error: The directory \"" + sHome +
                                 "\" does not exist. Please specify the path to " +
								 "a valid office installation.");
                continue;
            }
        }
        //Check if this is a valid office installation folder by looking for the
		//program sub-directory
        var progDir = sHome + "\\program";
        if (! aFileSystemObject.FolderExists(progDir))
        {
            stdout.WriteLine("\n Error: Could not find directory \"" +
                             progDir + "\". An office installation is required, " +
							 "please specify the path to a valid office " +
							 "installation.");
            continue;
        }
        return sHome;
    }
}

function searchOffice()
{
	var tmp = oo_sdk_home;

	if (aFileSystemObject.FileExists(tmp + "\\program\\soffice.exe")) {
	   return tmp;
	}

	return "";
}

function getMakeHome()
{
    var sSuggestedHome = WshSysEnv("OO_SDK_MAKE_HOME");

    while(true)
    {
        stdout.Write("\n Enter GNU make (3.79.1 or higher) tools directory [" +
                     sSuggestedHome + "]:");
        var sHome = stdin.ReadLine();
        if (sHome.length == 0)
        {
            //No user input, use default.
            if ( ! aFileSystemObject.FolderExists(sSuggestedHome))
            {
                stdout.WriteLine("\n Error: Could not find directory \"" +
								 sSuggestedHome + "\". GNU make is required, " +
								 "please specify a GNU make tools directory.");
                sSuggestedHome = "";
                continue;
            }
            sHome = sSuggestedHome;
        } else
        {
            //validate the user input
            if ( ! aFileSystemObject.FolderExists(sHome))
            {
                stdout.WriteLine("\n Error: The directory \"" + sHome +
                                 "\" does not exist. GNU make is required, " +
								 "please specify a GNU make tools directory.");
                continue;
            }
        }
        //Check for the make executable
        var sMakePath = sHome + "\\make.exe";
        if (! aFileSystemObject.FileExists(sMakePath))
	{
		sMakePath = sHome + "\\mingw32-make.exe";
	}
        if (! aFileSystemObject.FileExists(sMakePath))
        {
            stdout.WriteLine("\n Error: Could not find \"" + sMakePath +
							 "\". GNU make is required, please specify a GNU " +
							 "make tools directory.");
            continue;
        }
        return sHome;
    }
}

function getZipHome()
{
    var sSuggestedHome = WshSysEnv("OO_SDK_ZIP_HOME");

    while(true)
    {
        stdout.Write("\n Enter a zip (2.3 or higher) tools directory [" +
                     sSuggestedHome + "]:");
        var sHome = stdin.ReadLine();
        if (sHome.length == 0)
        {
            //No user input, use default.
            if ( ! aFileSystemObject.FolderExists(sSuggestedHome))
            {
                stdout.WriteLine("\n Error: Could not find directory \"" +
								 sSuggestedHome + "\". zip is required, please " +
								 "specify a zip tools directory.");
                sSuggestedHome = "";
                continue;
            }
            sHome = sSuggestedHome;
        }
        else
        {
            //validate the user input
            if ( ! aFileSystemObject.FolderExists(sHome))
            {
                stdout.WriteLine("\n Error: The directory \"" + sHome +
                                 "\" does not exist. zip is required, please " +
								 "specify a zip tools directory.");
                continue;
            }
        }
        //Check for the make executable
        var sZipPath = sHome + "\\zip.exe";
        if (! aFileSystemObject.FileExists(sZipPath))
        {
            stdout.WriteLine("\n Error: Could not find \"" + sZipPath +
                             "\". zip is required, please specify a zip tools " +
							 "directory.");
            continue;
        }
        return sHome;
    }
}

function getCatHome()
{
    var sSuggestedHome = WshSysEnv("OO_SDK_CAT_HOME");

    while(true)
    {
        stdout.Write("\n Enter a cat (2.0 or higher) tools directory [" +
                     sSuggestedHome + "]:");
        var sHome = stdin.ReadLine();
        if (sHome.length == 0)
        {
            //No user input, use default.
            if ( ! aFileSystemObject.FolderExists(sSuggestedHome))
            {
                stdout.WriteLine("\n Error: Could not find directory \"" +
                                 sSuggestedHome + "\". cat is required, please " +
                                 "specify a cat tools directory." +
                                 "\nYou can get cat from " +
                                 "http://sourceforge.net/projects/unxutils/files/latest/download");
                sSuggestedHome = "";
                continue;
            }
            sHome = sSuggestedHome;
        }
        else
        {
            //validate the user input
            if ( ! aFileSystemObject.FolderExists(sHome))
            {
                stdout.WriteLine("\n Error: The directory \"" + sHome +
                                 "\" does not exist. cat is required, please " +
                                 "specify a cat tools directory." +
                                 "\nYou can get cat from " +
                                 "http://sourceforge.net/projects/unxutils/files/latest/download");
                continue;
            }
        }
        //Check for the make executable
        var sCatPath = sHome + "\\cat.exe";
        if (! aFileSystemObject.FileExists(sCatPath))
        {
            stdout.WriteLine("\n Error: Could not find \"" + sCatPath +
                             "\". cat is required, please specify a cat tools " +
                             "directory." +
                             "\nYou can get cat from " +
                             "http://sourceforge.net/projects/unxutils/files/latest/download");
            continue;
        }
        return sHome;
    }
}

function getSedHome()
{
    var sSuggestedHome = WshSysEnv("OO_SDK_SED_HOME");

    while(true)
    {
        stdout.Write("\n Enter a sed (3.02 or higher) tools directory [" +
                     sSuggestedHome + "]:");
        var sHome = stdin.ReadLine();
        if (sHome.length == 0)
        {
            //No user input, use default.
            if ( ! aFileSystemObject.FolderExists(sSuggestedHome))
            {
                stdout.WriteLine("\n Error: Could not find directory \"" +
                                 sSuggestedHome + "\". sed is required, please " +
                                 "specify a sed tools directory." +
                                 "\nYou can get sed from " +
                                 "http://sourceforge.net/projects/unxutils/files/latest/download");
                sSuggestedHome = "";
                continue;
            }
            sHome = sSuggestedHome;
        }
        else
        {
            //validate the user input
            if ( ! aFileSystemObject.FolderExists(sHome))
            {
                stdout.WriteLine("\n Error: The directory \"" + sHome +
                                 "\" does not exist. sed is required, please " +
                                 "specify a sed tools directory." +
                                 "\nYou can get sed from " +
                                 "http://sourceforge.net/projects/unxutils/files/latest/download");
                continue;
            }
        }
        //Check for the make executable
        var sSedPath = sHome + "\\sed.exe";
        if (! aFileSystemObject.FileExists(sSedPath))
        {
            stdout.WriteLine("\n Error: Could not find \"" + sSedPath +
                             "\". sed is required, please specify a sed tools " +
                             "directory." +
                             "\nYou can get sed from " +
                             "http://sourceforge.net/projects/unxutils/files/latest/download");
            continue;
        }
        return sHome;
    }
}

function getCppHome()
{
    var sSuggestedHome = WshSysEnv("OO_SDK_CPP_HOME");
    if (sSuggestedHome.length == 0)
    {
        var sVC="";
        try {
            sVC = WshShell.RegRead(regKeyVCExpress90);
        }catch (exc) {}
        if (sVC.length == 0)
        {
	        try {
				sVC = WshShell.RegRead(regKeyVC90);
			}catch (exc) {}
		}
		// check Windows SDK if VC 9
        if (sVC.length > 0)
        {
		    oo_sdk_manifest_used="true";
			try {
				oo_sdk_windowssdk = WshShell.RegRead(regKeyWindowsSDK);
			}catch (exc) {}
		}
        if (sVC.length == 0)
        {
	        try {
				sVC = WshShell.RegRead(regKeyVCExpress80);
			}catch (exc) {}
	        if (sVC.length > 0)
			   oo_sdk_manifest_used="true";
		}
        if (sVC.length == 0)
        {
	        try {
				sVC = WshShell.RegRead(regKeyVC80);
			}catch (exc) {}
	        if (sVC.length > 0)
			   oo_sdk_manifest_used="true";
		}
        if (sVC.length == 0)
        {
	        try {
				sVC = WshShell.RegRead(regKeyVC71);
			}catch (exc) {}
		}
        if (sVC.length > 0)
        {
            sVC += "bin";
            if (aFileSystemObject.FileExists(sVC + "\\cl.exe"))
                sSuggestedHome = sVC;
        }
    }

    var bSkip = false;
    while(true)
    {
        stdout.Write("\n Enter the directory of the C++ compiler (optional) [" +
                     sSuggestedHome + "]:");
        var sHome = stdin.ReadLine();
        if (sHome.length == 0)
        {
            //No user input, check OO_SDK_CPP_HOME or suggested value
	    if ( sSuggestedHome.length == 0 ) {
		bSkip = true;
	    } else {
	        if ( !aFileSystemObject.FolderExists(sSuggestedHome) )
		{
		    stdout.WriteLine("\n Error: Could not find directory \"" +
				     sSuggestedHome + "\".");
		    sSuggestedHome = "";
		    bSkip = true;
		}
	    }

            sHome = sSuggestedHome;
        } else
        {
            //validate the user input
            if ( ! aFileSystemObject.FolderExists(sHome))
            {
                stdout.WriteLine("\n Error: Could not find directory \"" +
                                 sHome + "\".");
                bSkip = true;
            }
        }

		if ( !bSkip) {
		    //Check if the C++ compiler exist
		    var cl = sHome + "\\cl.exe";
		    var mt = sHome + "\\mt.exe";

			if (! aFileSystemObject.FileExists(cl))
			{
				stdout.WriteLine("\n Error: Could not find the C++ compiler \""
								 + cl + "\".");
				sHome = "";
				bSkip = true;
			} else {
			    if (aFileSystemObject.FileExists(mt)) {
				    oo_sdk_vc8_used="true";
				}
			}
		}

		if ( bSkip ) {
		   if ( skipChoice("the C++ compiler") ) {
			   return "";
		   } else {
			   bSkip = false;
			   continue;
		   }
		}

        return sHome;
    }
}

function getCliHome()
{
    var sSuggestedHome = WshSysEnv("OO_SDK_CLI_HOME");

    if (sSuggestedHome.length == 0)
    {
        try {
            var _ver = WshShell.RegRead(regKeyDotNet2_0);
            if (_ver.length > 0)
            {
                sSuggestedHome = WshShell.RegRead(regKeyDotNetInstallRoot);
                sSuggestedHome += sDirDotNet2_0;
                if ( ! aFileSystemObject.FolderExists(sSuggestedHome))
                    sSuggestedHome = "";
            }

			if (sSuggestedHome.length == 0)
			{
				_ver = WshShell.RegRead(regKeyDotNet1_1);
				if (_ver.length > 0)
				{
					sSuggestedHome = WshShell.RegRead(regKeyDotNetInstallRoot);
					sSuggestedHome += sDirDotNet1_1;
					if ( ! aFileSystemObject.FolderExists(sSuggestedHome))
					   sSuggestedHome = "";
				}
			}
		} catch (exc) {}
    }

    var bSkip = false;
    while(true)
    {
        stdout.Write("\n Enter the directory of the C# and VB.NET compilers (optional) [" + sSuggestedHome + "]:");
        var sHome = stdin.ReadLine();
        if (sHome.length == 0)
        {
            //No user input, check OO_SDK_CLI_HOME or suggested value
			if ( sSuggestedHome.length == 0 ) {
			    bSkip = true;
			} else {
			    if ( !aFileSystemObject.FolderExists(sSuggestedHome) )
				{
					stdout.WriteLine("\n Error: Could not find directory \"" +
									 sSuggestedHome + "\".");
					sSuggestedHome = "";
					bSkip = true;
				}
			}

		    sHome = sSuggestedHome;
        }
        else
        {
            //validate the user input
            if ( ! aFileSystemObject.FolderExists(sHome))
            {
                stdout.WriteLine("\n Error: The directory \"" + sHome +
                                 "\" does not exist.");
                bSkip = true;
            }
        }

		if ( !bSkip ) {
		    //Check if the C# and VB.NET compiler exist
		    var csc = sHome + "\\csc.exe";
		    var vbc = sHome + "\\vbc.exe";

			if (! aFileSystemObject.FileExists(csc))
			{
				stdout.WriteLine("\n Error: Could not find the C# compiler \"" +
								 csc + "\".");
				bSkip = true;
			}
			if (! aFileSystemObject.FileExists(vbc))
			{
				stdout.WriteLine("\n Error: Could not find the VB.NET compiler \"" +
								 vbc + "\".");
				bSkip = true;
			}
		}

		if ( bSkip ) {
		   if ( skipChoice("the C# and VB.NET compilers") ) {
			   return "";
		   } else {
			   bSkip = false;
			   continue;
		   }
		}

        return sHome;
    }
}

function getJavaHome()
{
    var sSuggestedHome = WshSysEnv("OO_SDK_JAVA_HOME");
    if (sSuggestedHome.length == 0)
    {
        try {
            var currentVersion = WshShell.RegRead(regKeyJDK + "CurrentVersion");
            if (currentVersion.length > 0)
            {
                sSuggestedHome = WshShell.RegRead(regKeyJDK + currentVersion +
												  "\\JavaHome");
                if ( ! aFileSystemObject.FolderExists(sSuggestedHome) )
                    sSuggestedHome = "";
            }
        } catch (exc) {}
    }

    var bSkip = false;
    while(true)
    {
        stdout.Write("\n Enter JAVA SDK (1.6 or higher) installation directory (optional) [" + sSuggestedHome + "]:");
        var sHome = stdin.ReadLine();
        if (sHome.length == 0)
        {
            //No user input, check OO_SDK_JAVA_HOME or suggested value
			if ( sSuggestedHome.length == 0 ) {
			    bSkip = true;
			} else {
			    if ( !aFileSystemObject.FolderExists(sSuggestedHome) )
				{
					stdout.WriteLine("\n Error: Could not find directory \"" +
									 sSuggestedHome + "\".");
					sSuggestedHome = "";
					bSkip=true;
				}
			}

			sHome = sSuggestedHome;
        } else
        {
            //validate the user input
            if ( ! aFileSystemObject.FolderExists(sHome))
            {
                stdout.WriteLine("\n Error: The directory \"" + sHome +
                                 "\" does not exist.");
				bSkip = true;
            }
        }

		if ( !bSkip) {
		    //Check if this is an sdk folder by looking for the javac compiler
			var javacompiler = sHome + "\\bin\\javac.exe";
			if (! aFileSystemObject.FileExists(javacompiler))
			{
				stdout.WriteLine("\n Error: Could not find \"" +
								 javacompiler + "\".");
				bSkip = true;
			}
        }

		if ( bSkip ) {
		   if ( skipChoice("the Java SDK") ) {
			   return "";
		   } else {
			   bSkip = false;
			   continue;
		   }
		}

        return sHome;
    }
}

function getOutputDir()
{
	var defaultdir = "c:\\" + oo_sdk_name;
    var sSuggestedDir = WshSysEnv("OO_SDK_OUT");
	if (sSuggestedDir.length == 0)
	   sSuggestedDir = defaultdir;

    var bSkip = false;
    while(true)
    {
        stdout.Write(
			"\n Default output directory is the \"c:\\" + oo_sdk_name + "\".\n" +
            " Enter an existent directory if you prefer a different one. But note" +
			" that only\n a path without spaces is allowed because of a" +
			" limitation of gnu make. (optional) [" + sSuggestedDir + "]:");
        var sDir = stdin.ReadLine();
        if (sDir.length == 0)
        {
            //No user input, check OO_SDK_OUT or suggested value
			if ( sSuggestedDir.length == 0 ) {
			    bSkip = true;
			} else {
				if (sSuggestedDir == oo_user_sdk_dir || sSuggestedDir == defaultdir) {
				    var fso = new ActiveXObject("Scripting.FileSystemObject");
					if ( !fso.FolderExists(sSuggestedDir) )
						fso.CreateFolder(sSuggestedDir);
				}
			    if ( !aFileSystemObject.FolderExists(sSuggestedDir) )
				{
					stdout.WriteLine("\n Error: Could not find directory \"" +
									 sSuggestedDir + "\".");
					sSuggestedDir = "";
					bSkip = true;
				}
			}

            sDir = sSuggestedDir;
        }
        else
        {
			if (sDir.indexOf(' ') != -1) {
				stdout.WriteLine("\n Error: your specified output directory " +
								 "\"" + sDir + "\" " +
								 "contains one or more spaces.\n        That " +
								 "causes problems with gnu make. Please specify" +
								 " a directory without spaces.");
				bSkip = true;
			}
            //validate the user input
            if ( ! aFileSystemObject.FolderExists(sDir))
            {
                stdout.WriteLine("\n Error: Could not find directory \"" +
                                 sDir + "\".");
                bSkip = true;
            }
        }

		if ( bSkip ) {
		   if ( skipChoice("a special output directory") ) {
			   return "";
		   } else {
			   bSkip = false;
			   continue;
		   }
		}

        return sDir;
    }
}

function getAutoDeployment()
{
    var sSuggestedAuto = WshSysEnv("SDK_AUTO_DEPLOYMENT");
    if (sSuggestedAuto.length == 0)
        sSuggestedAuto = "YES";

    while(true)
    {
        stdout.Write("\n Automatic deployment of UNO components (YES/NO) ["+
                     sSuggestedAuto + "]:");
        var sAuto = stdin.ReadLine();
        if (sAuto.length == 0)
            sAuto = sSuggestedAuto;
        else
        {
            sAutoU = sAuto.toUpperCase();
            if (sAutoU != "YES" && sAutoU != "NO")
            {
                stdout.WriteLine("\n Error: The value \"" + sAuto + "\" is " +
								 "invalid. Please answer YES or NO.")
                    continue;
            }
            sAuto = sAutoU;
        }
        return sAuto;
    }
}

//The function uses sp2bv.exe to obtain a file URL from a
//system path. The URL is already escaped for use as bootstrap variable.
//($ -> \$). Then the resulting string is escaped for use in a bat file.
//That is % signs are made to double % (% -> %%);
function makeBootstrapFileUrl(systemPath)
{
    var oExec = WshShell.Exec("sp2bv.exe \"" + systemPath + "\"");
    var output="";
    while (true)
    {
        if (!oExec.StdOut.AtEndOfStream)
        {
            var next = oExec.StdOut.Read(1);
            if (next == '%')
                output += "%%";
            else
                output += next;
        }
        else
            break;
    }
    return output;
}

function writeBatFile(fdir, file)
{
    var fso = new ActiveXObject("Scripting.FileSystemObject");
    if ( !fso.FolderExists(fdir) )
       fso.CreateFolder(fdir);
    var newFile = fso.CreateTextFile(file, true);

    newFile.Write(
        "@echo off\n" +
        "REM This script sets all environment variables, which\n" +
        "REM are necessary for building the examples of the Office Development Kit.\n" +
        "REM The Script was developed for the operating systems Windows.\n" +
        "REM The SDK name\n" +
        "REM Example: set OO_SDK_NAME=libreoffice3.4_sdk\n" +
        "set OO_SDK_NAME=" + oo_sdk_name  +
        "\n\n" +
        "REM Installation directory of the Software Development Kit.\n" +
        "REM Example: set OO_SDK_HOME=C:\\Program Files\\LibreOffice 3\\sdk\n" +
        "set OO_SDK_HOME=" + oo_sdk_home  +
        "\n\n" +
        "REM Office installation directory.\n" +
        "REM Example: set OFFICE_HOME=C:\\Program Files\\LibreOffice 3\n" +
        "set OFFICE_HOME=" + office_home +
        "\n\n" +
        "REM Directory of the make command.\n" +
        "REM Example: set OO_SDK_MAKE_HOME=D:\\NextGenerationMake\\make\n" +
        "set OO_SDK_MAKE_HOME=" + oo_sdk_make_home +
        "\n\n" +
		"REM Directory of the zip tool.\n" +
		"REM Example: set OO_SDK_ZIP_HOME=D:\\infozip\\bin\n" +
		"set OO_SDK_ZIP_HOME=" + oo_sdk_zip_home +
        "\n\n" +
        "REM Directory of the cat tool.\n" +
        "REM Example: set OO_SDK_CAT_HOME=C:\\UnxUtils\\usr\\local\\wbin\n" +
        "set OO_SDK_CAT_HOME=" + oo_sdk_cat_home +
        "\n\n" +
        "REM Directory of the sed tool.\n" +
        "REM Example: set OO_SDK_SED_HOME=C:\\UnxUtils\\usr\\local\\wbin\n" +
        "set OO_SDK_SED_HOME=" + oo_sdk_sed_home +
        "\n\n" +
        "REM Directory of the C++ compiler.\n" +
        "REM Example:set OO_SDK_CPP_HOME=C:\\Program Files\\Microsoft Visual Studio 9.0\\VC\\bin\n" +
        "set OO_SDK_CPP_HOME=" + oo_sdk_cpp_home +
		"\nset CPP_MANIFEST=" + oo_sdk_manifest_used +
		"\nset CPP_WINDOWS_SDK=" + oo_sdk_windowssdk +
        "\n\n" +
        "REM Directory of the C# and VB.NET compilers.\n" +
        "REM Example:set OO_SDK_CLI_HOME=C:\\WINXP\\Microsoft.NET\\Framework\\v1.0.3705\n" +
        "set OO_SDK_CLI_HOME=" + oo_sdk_cli_home +
        "\n\n" +
        "REM Java SDK installation directory.\n" +
        "REM Example: set OO_SDK_JAVA_HOME=C:\\Program Files\\Java\\jdk1.6.0_05\n" +
        "set OO_SDK_JAVA_HOME=" + oo_sdk_java_home +
        "\n\n" +
        "REM Special output directory\n" +
        "REM Example: set OO_SDK_OUT=C:\\" + oo_sdk_name + "\n" +
        "set OO_SDK_OUT=" + oo_sdk_out +
        "\n\n" +
        "REM Automatic deployment\n" +
        "REM Example: set SDK_AUTO_DEPLOYMENT=YES\n" +
        "set SDK_AUTO_DEPLOYMENT=" + sdk_auto_deployment +
        "\n\n" +
        "REM Check installation path for the Office Development Kit.\n" +
        "if not defined OO_SDK_HOME (\n" +
        "   echo Error: the variable OO_SDK_HOME is missing!\n" +
        "   goto :error\n" +
        " )\n" +
        "\n" +
        "REM Check installation path for the office.\n" +
        "REM if not defined OFFICE_HOME (\n" +
        "REM    echo Error: the variable OFFICE_HOME is missing!\n" +
        "REM    goto :error\n" +
        "REM  )\n" +
        "\n" +
        "REM Check installation path for GNU make.\n" +
        "if not defined OO_SDK_MAKE_HOME (\n" +
        "   echo Error: the variable OO_SDK_MAKE_HOME is missing!\n" +
        "   goto :error\n" +
        " )\n" +
        "\n" +
        "REM Check installation path for the zip tool.\n" +
        "if not defined OO_SDK_ZIP_HOME (\n" +
        "   echo Error: the variable OO_SDK_ZIP_HOME is missing!\n" +
        "   goto :error\n" +
        " )\n" +
        "\n" +
        "REM Check installation path for the cat tool.\n" +
        "if not defined OO_SDK_CAT_HOME (\n" +
        "   echo Error: the variable OO_SDK_CAT_HOME is missing!\n" +
        "   goto :error\n" +
        " )\n" +
        "\n" +
        "REM Check installation path for the sed tool.\n" +
        "if not defined OO_SDK_SED_HOME (\n" +
        "   echo Error: the variable OO_SDK_SED_HOME is missing!\n" +
        "   goto :error\n" +
        " )\n" +
        "\n" +
        "REM Set library path. \n" +
        "set LIB=%OO_SDK_HOME%\\lib;%LIB%\n" +
        "if defined CPP_WINDOWS_SDK (\n" +
        "   set LIB=%LIB%;%CPP_WINDOWS_SDK%\\lib\n" +
        " )\n" +
        "\n" +
        "REM Set office program path.\n" +
        "if defined OFFICE_HOME (\n" +
        "   set OFFICE_PROGRAM_PATH=%OFFICE_HOME%\\program\n" +
        " )\n" +
        "\n" +
		"REM Set UNO path, necessary to ensure that the cpp examples using the\n" +
		"REM new UNO bootstrap mechanism use the configured office installation\n" +
		"REM (only set when using an Office).\n" +
		"if defined OFFICE_HOME (\n" +
		"   set UNO_PATH=%OFFICE_PROGRAM_PATH%\n" +
		" )\n" +
        "\n" +
        "set OO_SDK_URE_BIN_DIR=%OFFICE_PROGRAM_PATH%\n" +
        "set OO_SDK_URE_LIB_DIR=%OFFICE_PROGRAM_PATH%\n" +
        "set OO_SDK_URE_JAVA_DIR=%OFFICE_PROGRAM_PATH%\\classes\n" +
        "set OO_SDK_OFFICE_BIN_DIR=%OFFICE_PROGRAM_PATH%\n" +
        "set OO_SDK_OFFICE_LIB_DIR=%OFFICE_PROGRAM_PATH%\n" +
        "set OO_SDK_OFFICE_JAVA_DIR=%OFFICE_PROGRAM_PATH%\\classes\n" +
        "\n" +
        "REM Set classpath\n" +
        "set CLASSPATH=%OO_SDK_URE_JAVA_DIR%\\juh.jar;%OO_SDK_URE_JAVA_DIR%\\jurt.jar;%OO_SDK_URE_JAVA_DIR%\\ridl.jar;%OO_SDK_URE_JAVA_DIR%\\unoloader.jar;%OO_SDK_OFFICE_JAVA_DIR%\\unoil.jar\n" +
        "REM if defined OFFICE_HOME (\n" +
        "REM     set CLASSPATH=%CLASSPATH%;%OO_SDK_OFFICE_JAVA_DIR%\\unoil.jar\n" +
        "REM  )\n" +
        "\n" +
        "REM Add directory of the SDK tools to the path.\n" +
        "set PATH=%OO_SDK_HOME%\\bin;%OO_SDK_URE_BIN_DIR%;%OO_SDK_OFFICE_BIN_DIR%;%OO_SDK_HOME%\\WINexample.out\\bin;%PATH%\n" +
        "\n" +
        "REM Set PATH appropriate to the output directory\n" +
        "if defined OO_SDK_OUT set PATH=%OO_SDK_OUT%\\WINexample.out\\bin;%PATH%\n" +
        "if not defined OO_SDK_OUT set PATH=%OO_SDK_HOME%\\WINexample.out\\bin;%PATH%\n" +
        "\n" +
        "REM Add directory of the command make to the path, if necessary.\n" +
        "if defined OO_SDK_MAKE_HOME set PATH=%OO_SDK_MAKE_HOME%;%PATH%\n" +
        "\n" +
        "REM Add directory of the zip tool to the path, if necessary.\n" +
        "if defined OO_SDK_ZIP_HOME set PATH=%OO_SDK_ZIP_HOME%;%PATH%\n" +
        "\n" +
        "REM Add directory of the cat tool to the path, if necessary.\n" +
        "if defined OO_SDK_CAT_HOME set PATH=%OO_SDK_CAT_HOME%;%PATH%\n" +
        "\n" +
        "REM Add directory of the sed tool to the path, if necessary.\n" +
        "if defined OO_SDK_SED_HOME set PATH=%OO_SDK_SED_HOME%;%PATH%\n" +
        "\n" +
        "REM Add directory of the C++ compiler to the path, if necessary.\n" +
        "if defined OO_SDK_CPP_HOME set PATH=%OO_SDK_CPP_HOME%;%PATH%\n" +
        "\n" +
        "REM Add directory of the Win SDK to the path, if necessary.\n" +
        "if defined CPP_WINDOWS_SDK set PATH=%CPP_WINDOWS_SDK%\\bin;%PATH%\n" +
        "if defined CPP_WINDOWS_SDK set INCLUDE=%CPP_WINDOWS_SDK%\\Include;%INCLUDE%\n" +
        "REM Add directory of the C# and VB.NET compilers to the path, if necessary.\n" +
        "if defined OO_SDK_CLI_HOME set PATH=%OO_SDK_CLI_HOME%;%PATH%\n" +
        "\n" +
        "REM Add directory of the Java tools to the path, if necessary.\n" +
        "if defined OO_SDK_JAVA_HOME set PATH=%OO_SDK_JAVA_HOME%\\bin;%OO_SDK_JAVA_HOME%\\jre\\bin;%PATH%\n" +
        "\n" +
        "REM Set environment for C++ compiler tools, if necessary.\n" +
        "if defined OO_SDK_CPP_HOME call \"%OO_SDK_CPP_HOME%\\VCVARS32.bat\"\n" +
        "\n" +
        "REM Set title to identify the prepared shell.\n" +
        "title Shell prepared for SDK\n" +
        "\nREM Prepare shell with all necessary environment variables.\n" +
        "echo.\n" +
        "echo  ******************************************************************\n" +
        "echo  *\n" +
        "echo  * SDK environment is prepared for Windows\n" +
        "echo  *\n" +
        "echo  * SDK = %OO_SDK_HOME%\n" +
        "echo  * Office = %OFFICE_HOME%\n" +
        "echo  * Make = %OO_SDK_MAKE_HOME%\n" +
        "echo  * Zip = %OO_SDK_ZIP_HOME%\n" +
        "echo  * cat = %OO_SDK_CAT_HOME%\n" +
        "echo  * sed = %OO_SDK_SED_HOME%\n" +
        "echo  * C++ Compiler = %OO_SDK_CPP_HOME%\n" +
        "echo  * C# and VB.NET compilers = %OO_SDK_CLI_HOME%\n" +
        "echo  * Java = %OO_SDK_JAVA_HOME%\n" +
        "echo  * Special Output directory = %OO_SDK_OUT%\n" +
        "echo  * Auto deployment = %SDK_AUTO_DEPLOYMENT%\n" +
        "echo  *\n" +
        "echo  ******************************************************************\n" +
        "echo.\n" +
        "goto end\n" +
        "\n" +
        " :error\n" +
        "Error: Please insert the necessary environment variables into the batch file.\n" +
        "\n" +
        " :end\n"
        );
        newFile.Close();
}

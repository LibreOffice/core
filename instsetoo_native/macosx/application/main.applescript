(*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: main.applescript,v $
 * $Revision: 1.5 $
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
 *************************************************************************)

(*==== (global variables as get-functions) ====*)

on getOOInstallPath()
	return (((path to me) as string) & "Contents:")
end getOOInstallPath

on getOOResourcesPath()
	return (((path to me) as string) & "Contents:Resources:")
end getOOResourcesPath

on getOOProgramPath()
	return (getOOInstallPath() & "MacOS:")
end getOOProgramPath

on getScriptPath()
	-- set this to absolute path when debugging
	return (((path to me) as string) & "Contents:Resources:Scripts:")
end getScriptPath

on getOOUserSettingsPath()
	return (((path to home folder) as string) & "Library:Application Support:OpenOffice.org:%USERDIRPRODUCTVERSION:")
end getOOUserSettingsPath

on getOOUserFontPath()
	return (getOOUserSettingsPath() & "user:fonts:")
end getOOUserFontPath

on getUserLogsPath()
	return (((path to home folder) as string) & "Library:Logs:")
end getUserLogsPath

on shellTerminator()
	return (" &>/dev/null & echo $!") as string
end shellTerminator

--
-- the default handlers: run, open, idle, quit
--

on run
	if (preRun()) then
		logEvent("(Scripts/main) Running OpenOffice.org")
		openSoffice("-")
	end if
end run

on open (theFiles)
	if (preRun()) then
		openFiles(theFiles)
	end if
end open

on idle
	-- close icon only if ooo has terminated
	if (hasOOoQuit()) then
		tell me to quit
	end if
	-- check all x seconds if ok to quit
	return 3
end idle

on quit
	if (hasOOoQuit()) then
		continue quit
	end if
end quit

-------------------------------------------------------------

on preRun()
	
	
	-- Check for the required version of Mac OS X
	if (not atLeastOSXVersion(10, 3, 0)) then
		display dialog getMessage("ERROR_NEED_PANTHER")
		return false
	end if
	
	-- Check for that OOo can be found
	if (not isRealPath(getOOProgramPath())) then
		display dialog getMessage("ERROR_OOO_NOT_FOUND")
		return false
	end if
	
	if (not isRealPath(getOOUserFontPath())) then
		set createUserFont to "mkdir -p " & (quoted form of POSIX path of getOOUserFontPath()) & "; "
		do shell script createUserFont
	end if

	-- If no crash occured before, ~/Library/Logs does not exist, and OpenOffice.org cannot be started
	if (not isRealPath(getUserLogsPath())) then
		set createUserLogsPath to "mkdir -p " & (quoted form of POSIX path of getUserLogsPath()) & "; "
		do shell script createUserLogsPath
	end if

	-- Checks are ok, now do the PostInstall stuff (e.g. fondu)
	
	-- load helper library
	set postinstall to load script alias Â
		(getScriptPath() & "PostInstall.scpt")
	-- execute the postinstall script
	run of postinstall
	
	
	return true
end preRun


on hasOOoQuit()
	if (isRealPath(getOOProgramPath())) then
		-- set the location of soffice binary
		set soffice to (quoted form of (POSIX path of getOOProgramPath() & "soffice"))
		
		set isRunning to do shell script "_FOUND_=`ps -wx -o command | grep " & soffice & " | grep -v grep`; echo $_FOUND_"
		if isRunning ­ "" then
			return false
		else
			return true
		end if
	else
		return true
	end if
end hasOOoQuit


on openSoffice(aFile)
	set theDisplay to startXServer()
	if (theDisplay is equal to "error") then
		return
	end if
	set theEnv to "DISPLAY=" & theDisplay & " ; export DISPLAY; "
	set theCmd to "sh " & (quoted form of (POSIX path of getOOProgramPath() & "soffice")) & " "
	do shell script theEnv & theCmd & aFile & shellTerminator()
	-- logEvent("open CMD: " & theEnv & theCmd & aFile)
end openSoffice


-- helper function to start X11 server
on startXServer()
	
	-- get X settings
	set XSettingsList to findXServer()
	set whichserver to item 1 of XSettingsList
	set Xserverloc to item 2 of XSettingsList
	
	-- debug:
	-- logEvent("(scripts/main) X settings: " & whichserver & "--" & POSIX path of Xserverloc)
	-- set whichserver to "NOXSERVER"
	
	-- if nothing really was found, display an error message.
	if (whichserver is equal to "NOXSERVER") then
		
		display dialog getMessage("ERROR_NEED_X11") buttons {"Quit", "More Info"} default button "More Info"
		if the button returned of the result is "Quit" then
			quit
		else
			-- if more info is chosen, then open a help web page
			do shell script "open http://porting.openoffice.org/mac/faq/installing/X11.html"
			-- cannot continue, so quit the script
			return "error"
		end if
		
	end if
	
	set now_running to ""
	set now_running to do shell script "INX=`ps -wcx | grep  " & quoted form of (whichserver & "$") & "`; echo $INX"
	if whichserver = "NOXSERVER" then
		-- display dialog "No XServer Found"
		set now_running to "Skip"
	end if
	if now_running = "" then
		if whichserver = "X11" then
			set x11cmd to quoted form of (Xserverloc & "/Contents/MacOS/X11") & shellTerminator()
			do shell script x11cmd
			-- save process id
			set x11pid to the result
			-- wait until the window manager is started which is the second child process of x11
			set numchildrencmd to "ps -x -o ppid | grep " & x11pid & " | wc -l"
			set numchildren to 0
			set d to current date
			set t1 to time of d
			repeat while numchildren ­ 2
				set d to current date
				set t2 to time of d
				-- give up after 30 seconds
				if t2 - t1 > 30 then
					display dialog "Command timed out"
					exit repeat
				end if
				set result to do shell script numchildrencmd
				set numchildren to result as integer
			end repeat
		else -- startup XDarwin
			do shell script "open " & quoted form of Xserverloc & shellTerminator()
			do shell script "sleep 4"
		end if
	end if
	if whichserver is equal to "X11" then
		-- the DISPLAY variable is different for every user currently logged in
		-- X11 passes the DISPLAY as the last command line parameter to its child process
		-- we can use ps to read the command line and parse the trailing :0, :1, or whatever
		set xdisplay to do shell script "ps -wx -o command | grep X11.app | grep \":.$\" | sed \"s/^.*:/:/g\""
		--display dialog xdisplay
		return xdisplay
	else
		-- TODO: find out how XDarwin does it
		return ":0"
	end if
end startXServer


on openFiles(fileList)
	if (count of fileList) > 0 then
		repeat with i from 1 to the count of fileList
			set theDocument to (item i of fileList)
			set theFilePath to (quoted form of POSIX path of theDocument)
			set theFileInfo to (info for theDocument)
			openSoffice(theFilePath)
			logEvent("(Scripts/main) Open file: " & theFilePath)
		end repeat
	end if
end openFiles


(* ===== (Helper functions) ======= *)

-- get a localized string
on getMessage(aKey)
	try
		if (aKey is equal to "YES_KEY") then
			return "Yes"
		end if
		
		if (aKey is equal to "NO_KEY") then
			return "No"
		end if
		
		if (aKey is equal to "ERROR_OOO_NOT_FOUND") then
			return "OpenOffice.org was not found on your system. Please (re-)install OpenOffice.org first."
		end if
		
		if (aKey is equal to "ERROR_NEED_PANTHER") then
			return "This build of OpenOffice.org cannot be run on this system, OpenOffice.org requires MacOSX 10.3 (Panther) or newer system"
		end if
		
		if (aKey is equal to "ERROR_NEED_X11") then
			return "OpenOffice.org for Mac OS X cannot be started, because the X11 software is not installed. Please install Apple X11 first from the Mac OS X install DVD. More information: http://porting.openoffice.org/mac/faq/installing/X11.html"
		end if
	end try
end getMessage


-- function for logging script messages
on logEvent(themessage)
	set theLine to (do shell script Â
		"date  +'%Y-%m-%d %H:%M:%S'" as string) Â
		& " " & themessage
	do shell script "echo " & quoted form of theLine & Â
		" >> ~/Library/Logs/OpenOffice%USERDIRPRODUCTVERSION.log"
end logEvent


-- function for checking if a path exists
on isRealPath(aPath)
	try
		alias aPath
		return true
	on error
		-- error number -43 from aPath
		-- display dialog "NotRP -- " & aPath
		return false
	end try
end isRealPath

-- try to find X11 server on the Mac OS X system
-- return value: the found server or "NOXSERVER"
on findXServer()
	-- First try standard X11 location, then try standard XDarwin location
	
	set whichserver to "NOXSERVER"
	--Utilities folder of system
	set Xserverloc to ((path to utilities folder from system domain) as string) & "X11.app:"
	--display dialog " Xserverloc" & Xserverloc
	if (isRealPath(Xserverloc)) then
		set whichserver to "X11"
		set Xserverloc to (POSIX path of Xserverloc)
	else
		--Applications folder of system
		set Xserverloc to ((path to applications folder from system domain) as string) & "XDarwin.app:"
		if (isRealPath(Xserverloc)) then
			set whichserver to "XDarwin"
			set Xserverloc to (POSIX path of Xserverloc)
		end if
	end if
	
	-- if nothing found yet try using locate, first with X11.app and then with XDarwin.app
	if (whichserver is equal to "NOXSERVER") then
		set Xserverloc to do shell script "locate X11.app/Contents/MacOS/X11 | sed -e 's-/Contents/MacOS/X11--g'"
		if Xserverloc ­ "" then
			set whichserver to "X11"
		end if
	end if
	
	if (whichserver is equal to "NOXSERVER") then
		set Xserverloc to do shell script "locate XDarwin.app/Contents/MacOS/XDarwin | sed -e 's-/Contents/MacOS/XDarwin--g'"
		if Xserverloc ­ "" then
			set whichserver to "XDarwin"
		end if
	end if
	
	return {whichserver, Xserverloc}
end findXServer


-- Test for a minimum version of Mac OS X
on atLeastOSXVersion(verMajor, verMinor, verStep)
	-- The StandardAdditions's 'system attribute' used to be the Finder's 'computer' command.
	tell application "Finder" to set sysv to (system attribute "sysv")
	
	-- Generate sysv-compatible number from given version
	set reqVer to ((verMajor div 10) * 4096 + (verMajor mod 10) * 256 + verMinor * 16 + verStep)
	
	-- DEBUGGING:	
	-- display dialog ("RV:" & reqVer & " < " & sysv as string)
	
	-- set major to ((sysv div 4096) * 10 + (sysv mod 4096 div 256))
	-- set minor to (sysv mod 256 div 16)
	-- set step to (sysv mod 16)
	-- display dialog ("Your Mac OS X version: " & major & "." & minor & "." & step)
	
	if (reqVer > sysv) then
		return false
	else
		return true
	end if
end atLeastOSXVersion

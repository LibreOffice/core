(*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PostInstall.applescript,v $
 * $Revision: 1.4 $
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

-- This is the PostInstall -script for .pkg installation
-- 
-- Currently this script does the following things:
--   1) Uses fondu to extract and convert .dfont -fonts from Mac OS X system to .ttf -fonts for OpenOffice.org


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

-- OSXSystemFontPathList : {"/System/Library/Fonts/", "/Library/Fonts/"}
-- OSXUserFontPathList : {"~/Library/Fonts/"}

on getOSXSystemFontPathList()
	return {(path to fonts folder from system domain) as string, Â
		(path to fonts folder from local domain) as string}
end getOSXSystemFontPathList

on getOSXUserFontPathList()
	return {(path to fonts folder from user domain) as string}
end getOSXUserFontPathList

on getOOSystemFontPath()
	return (getOOInstallPath() & "share:fonts:truetype:")
end getOOSystemFontPath

on getOOUserSettingsPath()
	return (((path to home folder) as string) & "Library:Application Support:OpenOffice.org:%USERDIRPRODUCTVERSION:")
end getOOUserSettingsPath

on getOOUserFontPath()
	return (getOOUserSettingsPath() & "user:fonts:")
end getOOUserFontPath


on getOOCookieSystemFondu()
	-- nosystemfondu : file does exist if user does not want to use fondu for system fonts
	return "no_system_fondu"
end getOOCookieSystemFondu

on getOOCookieSystemFonduDone()
	-- systemfondudone : file does exist if native fonts already extracted from system fonts
	return "system_fondu_done"
end getOOCookieSystemFonduDone

on getOOCookieUserFondu()
	-- nouserfondu : file does exist if user does not want to use fondu for user fonts
	return "no_user_fondu"
end getOOCookieUserFondu

on getOOCookieUserFonduDone()
	-- userfondudone : file does exist if native fonts already extracted from user fonts
	return "user_fondu_done"
end getOOCookieUserFonduDone

--
-- the default handler: run
--

on run
	-- Check for that OOo can be found
	if (not isRealPath(getOOProgramPath())) then
		logEvent("(scripts/PostInstall) ERROR: could not find OOo installation from " & POSIX path of getOOProgramPath())
		return
	end if
	
	-- checks are ok, now we can start doing the real stuff
	firstLaunch()
	runSystemFondu()
	runUserFondu()
	
	return
end run


-------------------------------------------------------------


on runSystemFondu()
	-- check if user does not want font conversion 
	if (isRealPath(getOOSystemFontPath() & getOOCookieSystemFondu())) then
		return
	end if
	
	-- check if font conversion was already run
	if (isRealPath(getOOSystemFontPath() & getOOCookieSystemFonduDone())) then
		return
	end if
	
	logEvent("(scripts/PostInstall) Extracting system fonts...")
	-- else try to create footprint
	if (setCookie(getOOSystemFontPath(), getOOCookieSystemFonduDone())) then
		-- call fondu for each font (i.e. without wildcard), so if it crashes only one font is missing
		fonduConvertFonts(getOOSystemFontPath(), getOSXSystemFontPathList())
	end if
	logEvent("(scripts/PostInstall) Extracting system fonts completed.")
end runSystemFondu


on runUserFondu()
	-- check if user does not want font conversion 
	if (isRealPath(getOOUserFontPath() & getOOCookieUserFondu())) then
		return
	end if
	
	-- check if font conversion was already run
	if (isRealPath(getOOUserFontPath() & getOOCookieUserFonduDone())) then
		return
	end if
	
	logEvent("(scripts/PostInstall) Extracting user fonts...")
	-- try to create footprint
	if (setCookie(getOOUserFontPath(), getOOCookieUserFonduDone())) then
		-- call fondu for each font (i.e. without wildcard), so if it crashes only one font is missing
		fonduConvertFonts(getOOUserFontPath(), getOSXUserFontPathList())
	end if
	logEvent("(scripts/PostInstall) Extracting user fonts completed.")
end runUserFondu


on firstLaunch()
	-- continue only if OOSysFontdir exists	
	if (not isRealPath(getOOSystemFontPath())) then
		logEvent("(scripts/PostInstall) ERROR: could not find System font folder from " & POSIX path of getOOSystemFontPath())
		return
	end if
	
	if (setCookie(getOOSystemFontPath(), getOOCookieSystemFondu() & ".in_progress")) then
		-- Has user already decided that he does not want to extract system fonts ?
		if (not isRealPath(getOOSystemFontPath() & getOOCookieSystemFondu())) then
			-- Are system fonts already extracted ?
			if (not isRealPath(getOOSystemFontPath() & getOOCookieSystemFonduDone())) then
				-- ask if the user wants to use fondu to extract system fonts
				set yesKey to getMessage("YES_KEY")
				set noKey to getMessage("NO_KEY")
				display dialog getMessage("OOO_EXTRACT_NATIVE_SYSFONTS") buttons {noKey, yesKey} default button yesKey
				set theResult to the button returned of the result
				if theResult is noKey then
					-- not use fondu for system fonts extraction !
					setCookie(getOOSystemFontPath(), getOOCookieSystemFondu())
					logEvent("(scripts/PostInstall) Setting: no system fonts")
				end if
			end if
		end if
	end if
	
	-- continue only if OOUserFontdir exists	
	if (not isRealPath(getOOUserFontPath())) then
		logEvent("(scripts/PostInstall) ERROR: could not find User font folder from " & POSIX path of getOOUserFontPath())
		return
	end if
	
	-- Has user already decided that he does not want to extract user fonts ?
	if (not isRealPath(getOOUserFontPath() & getOOCookieUserFondu())) then
		-- Are system fonts already extracted ?
		if (not isRealPath(getOOUserFontPath() & getOOCookieUserFonduDone())) then
			-- ask if the user wants to use fondu to extract user fonts
			set yesKey to getMessage("YES_KEY")
			set noKey to getMessage("NO_KEY")
			display dialog getMessage("OOO_EXTRACT_NATIVE_USERFONTS") buttons {noKey, yesKey} default button yesKey
			set theResult to the button returned of the result
			if theResult is noKey then
				-- not use fondu for user fonts extraction !
				setCookie(getOOUserFontPath(), getOOCookieUserFondu())
				logEvent("(scripts/PostInstall) Setting: no user fonts")
			end if
		end if
	end if
	
end firstLaunch


on fonduConvertFonts(targetPath, sourcePathList)
	
	-- define the location of fondu
	set fondu to quoted form of (POSIX path of getOOProgramPath() & "fondu")
	
	-- first go to the target directory
	set fonduCmd to "cd " & (quoted form of POSIX path of targetPath) & "; "
	
	repeat with q from 1 to number of items in sourcePathList
		set aPath to POSIX path of (item q of sourcePathList)
		set fonduCmd to fonduCmd & "for i in " & aPath & "*; do " & fondu & " -force \"$i\" >> /dev/null 2>&1; done; "
	end repeat
	try
		-- ignore errors
		-- with admin privileges does not work well on panther
		do shell script "sh -c " & quoted form of fonduCmd
	end try
	logEvent("fonduCMD: " & fonduCmd)
	
end fonduConvertFonts


(* ===== (Helper functions) ======= *)

-- set a cookiefile. The content is empty.
on setCookie(aPath, cookieFile)
	try
		if (isRealPath(aPath)) then
			set newFile to (aPath & cookieFile)
			open for access file newFile
			close access file newFile
			return true
		else
			return false
		end if
	on error
		return false
	end try
end setCookie

-- get a localized string
on getMessage(aKey)
	try
		if (aKey is equal to "YES_KEY") then
			return "Yes"
		end if
		
		if (aKey is equal to "NO_KEY") then
			return "No"
		end if
		
		if (aKey is equal to "OOO_EXTRACT_NATIVE_SYSFONTS") then
			return "Do you want OpenOffice.org to use the Apple system fonts?"
		end if
		
		if (aKey is equal to "OOO_EXTRACT_NATIVE_USERFONTS") then
			return "Do you want OpenOffice.org to use the fonts you have installed on this system?"
		end if
	end try
end getMessage

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

-- function for logging script messages
on logEvent(themessage)
	set theLine to (do shell script Â
		"date  +'%Y-%m-%d %H:%M:%S'" as string) Â
		& " " & themessage
	do shell script "echo " & quoted form of theLine & Â
		" >> ~/Library/Logs/OpenOffice%USERDIRPRODUCTVERSION.log"
end logEvent

(*************************************************************************
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
 *************************************************************************)

-- This is the PostInstall -script for .pkg installation
-- 
-- This script used to run fondu, but currently it does nothing
-- Keeping it around as the remaining framework might become useful again

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

on getOOUserSettingsPath()
	return (((path to home folder) as string) & "Library:Application Support:OpenOffice.org:%USERDIRPRODUCTVERSION:")
end getOOUserSettingsPath

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
--	doSomething()
	
	return
end run


-------------------------------------------------------------

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

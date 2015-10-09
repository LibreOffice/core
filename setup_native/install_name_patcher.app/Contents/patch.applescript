(*

 This file is part of the LibreOffice project.

 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.

 This file incorporates work covered by the following license notice:

   Licensed to the Apache Software Foundation (ASF) under one or more
   contributor license agreements. See the NOTICE file distributed
   with this work for additional information regarding copyright
   ownership. The ASF licenses this file to you under the Apache
   License, Version 2.0 (the "License"); you may not use this file
   except in compliance with the License. You may obtain a copy of
   the License at http://www.apache.org/licenses/LICENSE-2.0 .
*)

global frameworksRelativePath
global macosRelativePath
set frameworksRelativePath to "/Contents/Frameworks"
set macosRelativePath to "/Contents/MacOS"

set tarballName to "package.tar.bz2"

set libsSystemDefaultBasepath to "/usr/lib/"
set libsSystemBasepaths to {libsSystemDefaultBasepath}

set libNameStdcxx to "libstdc++.6.dylib"
set libNameGccS to "libgcc_s.1.dylib"
set listOfLibNames to {libNameStdcxx, libNameGccS}
set newPathForLibs to "@loader_path/"
set newPathForProgs to "@executable_path/../Frameworks/"

set toolname to "install-name-tool"
set toobindir to "Tools/"
set otoolname to "otool"
set isOtoolHere to false

set path2me to POSIX path of ((path to me as text) & "::") -- path to this script's parent folder

set OKLabel to "Ok"
set doItLabel to "Do it"
set PatchLabel to "Patch"
set BreakLabel to "Break"
set ByeLabel to "Bye"

set welcomeMsg to "Welcome to Patcher!

It may be used to fix your versions of LibreOffice"

set chooseMyOwn to "not listed here (choose location)"
set listPrompt to "Choose LibreOffice bundle which you want to patch"
set chooseManualMsg to "Point to your LibreOffice application"
set listDoneLabel to "This one"
set listCancelLabel to BreakLabel
set appIsNotLibreOfficeMsg to "is not a real LibreOffice

Run the patcher again and choose another LibreOffice bundle"

set IdentifyQ to "Can't extract package, most likely your account does not have the necessary privileges

Do you want to identify as administrator and try again?"
set IdentifyYES to "Yes, identify"
set IdentifyNO to "Nope, cancel"
set identifyFailedMsg to "Identification failed"

set installCompleteMessage to "Everything done

Enjoy!"

set proceedOrNotMessage to "Proceed? Below is the list of files to fix entries for shared libraries"
set messageAfterFixing to "Fix is done. Here is the updated list of files"
set installOrNotMessage to "Install? Contents of package to expand to"

set noToolMsg to "This patcher does not include " & quote & toolname & quote & " utility, so it is useless"

set noLibsProgsMsg to "Search for libraries and programs failed"
set alreadyFixedMsg to "It looks that all entries for shared libraries are already fixed"

global appBundleType
set appBundleType to "com.apple.application-bundle"

global strNewline
set strNewline to "
"
global strTab
set strTab to "	"

--*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*--

-- ++++++++++++++++++++++++
-- common functions
-- ++++++++++++++++++++++++

on replace_chars(this_text, search_string, replacement_string)
	set AppleScript's text item delimiters to search_string
	set item_list to every text item of this_text
	set AppleScript's text item delimiters to replacement_string
	set this_text to item_list as string
	set AppleScript's text item delimiters to ""
	return this_text
end replace_chars

on trim_string(theString, trimming)
	if (trimming) then
		repeat with i from 1 to (count theString) - 1
			if (theString begins with space) then
				set theString to text 2 thru -1 of theString
			else
				exit repeat
			end if
		end repeat
		repeat with i from 1 to (count theString) - 1
			if (theString ends with space) then
				set theString to text 1 thru -2 of theString
			else
				exit repeat
			end if
		end repeat
		if (theString is space) then set theString to ""
	end if
	return theString
end trim_string

on list2string(theList, itemSeparator, addBrackets)
	set resultString to ""
	if addBrackets as boolean then
		set resultString to resultString & "{ "
	end if
	set isFirstItem to true
	repeat with itemString in theList
		if not isFirstItem then
			set resultString to resultString & itemSeparator
		else
			set isFirstItem to false
		end if
		set resultString to resultString & itemString as string
	end repeat
	if addBrackets as boolean then
		set resultString to resultString & " }"
	end if
	return resultString
end list2string

-- ++++++++++++++++++++++++
-- special functions
-- ++++++++++++++++++++++++

on choose_bundle_dialog(theMessage)
	choose file with prompt theMessage of type appBundleType without showing package contents and invisibles
end choose_bundle_dialog

on isValidLibreOfficeBundle(pathToBundle)
	if pathToBundle is "" then
		return false
	end if
	
	set pathToLibs to (pathToBundle as string) & frameworksRelativePath
	set pathToProgs to (pathToBundle as string) & macosRelativePath
	set pathToLibs to replace_chars(pathToLibs, "//", "/")
	set pathToProgs to replace_chars(pathToProgs, "//", "/")
	
	set stringForGreping to "<string>LibreOffice"
	set whereIsPlist to "/Contents/Info.plist"
	set grepingScript to "grep " & quoted form of stringForGreping & " " & quoted form of ((pathToBundle as string) & whereIsPlist)
	set findSofficeScript to "find " & quoted form of (pathToProgs as string) & " -iname soffice -type f -print"
	set findLibappScript to "find " & quoted form of (pathToLibs as string) & " -iname libsofficeapp* -type f -print"
	
	try
		set grepLOresult to (do shell script grepingScript)
		---display dialog grepLOresult
		
		set foundSoffice to (do shell script findSofficeScript)
		if foundSoffice is "" then
			return false
		end if
		---display dialog foundSoffice
		
		set foundLibapp to (do shell script findLibappScript)
		if foundLibapp is "" then
			return false
		end if
		return true
	on error
		-- fall thru
	end try
	return false
end isValidLibreOfficeBundle

on libfile_to_entriesFromList(fillPath2Lib, listOfLibNames, path2otool)
	set theResult to {}
	set shellLine to quoted form of (path2otool as string) & " -L " & quoted form of fillPath2Lib
	set otoolResult to ""
	try
		set otoolResult to (do shell script shellLine)
	end try
	if otoolResult is not "" then
		repeat with aLibName in listOfLibNames
			repeat with aLine in (paragraphs in otoolResult)
				if aLine contains aLibName then
					if aLine does not contain fillPath2Lib then -- it is that library itself
						set theLine to replace_chars(aLine, strTab, " ") -- tabs to spaces
						set theLine to replace_chars(theLine, ")", "") -- no closing brackets
						set theLine to replace_chars(theLine, " (", strNewline) -- opening brackets to new lines
						---set theLine to replace_chars(theLine, ",", strNewline) -- commas to new lines
						set entryList to {}
						repeat with lineInTheLine in (paragraphs in theLine)
							set lineInTheLine to trim_string(lineInTheLine, true)
							if lineInTheLine is not "" then
								copy lineInTheLine to the end of entryList
							end if
						end repeat
						copy first item in entryList to the end of theResult
					else
						exit repeat -- second entry is this library's id, skip it too
					end if
				end if
			end repeat
		end repeat
	end if
	if (count of theResult) is not 0 then
		return list2string(theResult, strNewline, false) -- newline-separated string full of entries
	end if
	return ""
end libfile_to_entriesFromList

on libfile_to_entriesFromList_withBasepathList(fillPath2Lib, listOfLibNames, basePathList, path2otool)
	set stringOfEntries to libfile_to_entriesFromList(fillPath2Lib, listOfLibNames, path2otool)
	if stringOfEntries is "" then
		return ""
	end if
	set listOfEntries to paragraphs in stringOfEntries
	set listOfEntriesWithBasenames to {}
	repeat with entry in listOfEntries
		repeat with aBasepath in basePathList
			if entry starts with aBasepath then
				copy entry to the end of listOfEntriesWithBasenames
			end if
		end repeat
	end repeat
	if (count of listOfEntriesWithBasenames) is not 0 then
		return list2string(listOfEntriesWithBasenames, strNewline, false)
	end if
	return ""
end libfile_to_entriesFromList_withBasepathList

on toResultStringOfIdentifyError(errNumber)
	if errNumber is 2 then
		return "tarball not found"
	else if errNumber is -128 then
		return "cancelled by user"
	else if errNumber is -60005 then
		return "username/password wrong"
	end if
	return "unknown error #" & errNumber as string
end toResultStringOfIdentifyError

--*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*--

activate

display dialog welcomeMsg buttons {BreakLabel, PatchLabel} default button 2 without icon
if (button returned of result) is BreakLabel then
	return 2 -- nevermind bye
end if

-- ************************
-- see if "install name tool" is here
-- ************************
set path2tool to POSIX path of ((path2me as string) & toobindir & toolname)
try
	do shell script "test -f " & quoted form of (path2tool as string)
on error
	display dialog noToolMsg buttons {OKLabel} default button 1 with icon 0
	return 1
end try

-- ************************
-- see if "otool" is here
-- ************************
set path2otool to POSIX path of ((path2me as string) & toobindir & otoolname)
try
	do shell script "test -f " & quoted form of (path2otool as string)
	set isOtoolHere to true
on error
	set isOtoolHere to false
end try

if not isOtoolHere then
	-- without otool it's not possible to get actual basepath from the dylib
	-- so assume it is only default /usr/lib/
	set libsSystemBasepaths to {libsSystemDefaultBasepath}
end if

-- ************************
-- check if tar is installed
-- ************************
set tarname to "tar"
set fullPath2tar to ""
try
	set fullPath2tar to (do shell script "which " & tarname)
on error
	set noTarMsg to "Your system does not have " & quote & tarname & quote & " utility"
	display dialog noTarMsg buttons {OKLabel} default button 1 with icon 1
	return 1
end try

-- ************************
-- search for LibreOffice app bundles
-- ************************
set found_LOes_all to ""

try -- 'mdfind' may return an error when Spotlight is disabled
	---set searchForAppBundlesScript to "mdfind \"kMDItemContentType == " & quoted form of appBundleType & "\"" -- find all bundles
	set appBundleName to "LibreOffice*"
	set searchForAppBundlesScript to "mdfind \"kMDItemContentType == " & quoted form of appBundleType & " && kMDItemDisplayName == " & quoted form of appBundleName & "\""
	set found_LOes_all to do shell script searchForAppBundlesScript
end try

set found_LOes to {} -- empty AppleScript list
repeat with currentApp in paragraphs in found_LOes_all
	if currentApp does not start with "/Volumes" then
		if isValidLibreOfficeBundle(currentApp) then
			copy currentApp to the end of found_LOes
		end if
	end if
end repeat

if (count of found_LOes) is not 0 then
	copy chooseMyOwn to the end of found_LOes
end if

-- ************************
-- show file dialog if no LibreOffice found or show list of bundles to choose from
-- ************************
set didManualSelection to false
if (count of found_LOes) is 0 then -- no LibreOffices found
	-- yeah, one is supposed to "choose file"
	set chosenLObundle to POSIX path of choose_bundle_dialog(chooseManualMsg)
	set didManualSelection to true
	---
	---else if (get second item of found_LOes as string) is chooseMyOwn then -- only one LibreOffice found
	---set chosenLObundle to (get first item of found_LOes) -- don't show dialog with list of choices
else
	set chosenLObundle to (choose from list found_LOes default items (get first item of found_LOes) with prompt listPrompt OK button name listDoneLabel cancel button name listCancelLabel)
	if chosenLObundle is false then
		return 2 -- cancelled by user
	else if (chosenLObundle as string) is chooseMyOwn then
		set chosenLObundle to POSIX path of choose_bundle_dialog(chooseManualMsg)
		set didManualSelection to true
	end if
end if

-- ************************
-- check whether chosen path is really a LibreOffice bundle
-- ************************
if didManualSelection then
	---set loBundleNameWithoutTrailingSlash to replace_chars(chosenLObundle as string, ".app/", ".app")
	set loBundleNameWithoutTrailingSlash to text 1 through -2 of chosenLObundle as string
	if not isValidLibreOfficeBundle(chosenLObundle) then
		display dialog loBundleNameWithoutTrailingSlash & " " & appIsNotLibreOfficeMsg buttons {OKLabel} default button 1 with icon 1
		return 3 -- wrong target-directory
	end if
end if

-- ************************
-- search for dylibs, jnilibs, programs
-- ************************
set search_path_libs to (chosenLObundle as string) & frameworksRelativePath
set search_path_progs to (chosenLObundle as string) & macosRelativePath
set search_path_libs to replace_chars(search_path_libs, "//", "/")
set search_path_progs to replace_chars(search_path_progs, "//", "/")

try
	set found_dylibs_all to (do shell script "find " & quoted form of (search_path_libs as string) & " -iname *dylib* -type f -print")
	---(do shell script "mdfind -onlyin " & quoted form of (search_path_libs as string) & " \" kMDItemDisplayName == '*dylib*' \" ")
	set found_jnilibs_all to (do shell script "find " & quoted form of (search_path_libs as string) & " -iname *jnilib* -type f -print")
	set found_programs_all to (do shell script "find " & quoted form of (search_path_progs as string) & " -type f -print | grep -v \".framework\" ")
on error
	display dialog noLibsProgsMsg buttons {OKLabel} default button 1 with icon 1
	return 1
end try

set libraries_list to {}
repeat with currentEntry in paragraphs in found_dylibs_all
	if currentEntry does not end with ".py" and currentEntry does not end with ".pyc" and currentEntry does not end with ".pyo" then -- filter python stuff out
		copy currentEntry to the end of libraries_list
	end if
end repeat
repeat with currentEntry in paragraphs in found_jnilibs_all
	copy currentEntry to the end of libraries_list
end repeat

set programs_list to {}
repeat with currentEntry in paragraphs in found_programs_all
	set file_result to (do shell script "file " & quoted form of (currentEntry as string))
	if file_result contains "executable" and file_result does not contain "script" then
		copy currentEntry to the end of programs_list
	end if
	---if file_result contains "shared library" then
	-----last chance for this library
	-----copy currentEntry to the end of executables_list
	---end if
end repeat

-- ************************
-- read actual basepath from one of dylibs
-- ************************
if isOtoolHere then
	-- libsofficeapp.dylib is guaranteed to exist by isValidLibreOfficeBundle
	set search_path_libs to (chosenLObundle as string) & frameworksRelativePath
	set search_path_libs to replace_chars(search_path_libs, "//", "/")
	set libToFind to "libsofficeapp*"
	set findLibappScript to "find " & quoted form of (search_path_libs as string) & " -iname " & quote & libToFind & quote & " -type f -print"
	set foundLibapp to (do shell script findLibappScript)
	if foundLibapp is not "" then
		set stringOfEntries to libfile_to_entriesFromList(foundLibapp, {libNameStdcxx}, path2otool)
		if stringOfEntries is not "" then
			set listOfEntries to paragraphs in stringOfEntries
			set libEntry to first item of listOfEntries -- hope there's always only one entry for libstdc++
			set libBasename to replace_chars(libEntry, libNameStdcxx, "") -- path without lib's name
			---set libBasenameNoTrailingSlash to text 1 through -2 of libBasename
			if libBasename is newPathForLibs then
				-- looks like libsofficeapp dylib has already been fixed together with remaining libs
				copy "/tmp/" to the end of libsSystemBasepaths
			else
				if libBasename is not libsSystemDefaultBasepath then -- not "/usr/lib/"
					-- wow here is the real basepath
					copy libBasename to the end of libsSystemBasepaths
				end if
			end if
		else
			-- it's unusual to have no entries at all but ...
		end if
	end if
end if

-- ************************
-- show the list of files
-- ************************
set full_list_of_files to programs_list & libraries_list -- concatenate two separate lists into one
set list_of_files_to_fix to {}

set listToShowToUser to {}
repeat with currentEntry in full_list_of_files
	if not isOtoolHere then
		copy currentEntry to the end of listToShowToUser -- just copy names
		copy currentEntry to the end of list_of_files_to_fix
	else
		set stringOfEntries to libfile_to_entriesFromList_withBasepathList(currentEntry, listOfLibNames, libsSystemBasepaths, path2otool)
		if stringOfEntries is not "" then
			set listOfEntries to paragraphs in stringOfEntries
			copy currentEntry & ": " & list2string(listOfEntries, ", ", false) to the end of listToShowToUser
			copy currentEntry to the end of list_of_files_to_fix
		else
			---copy currentEntry & ": ¥ there's no shared library entries to fix ¥" to the end of listToShowToUser
		end if
	end if
end repeat

set needToFix to true
if (count of list_of_files_to_fix) is not 0 then
	set proceedOrNotFullMessage to proceedOrNotMessage & " " & list2string(listOfLibNames, " & ", false) & " from " & list2string(libsSystemBasepaths, ", ", true) & " using " & quote & toolname & quote
	set the listchoice to (choose from list listToShowToUser with prompt proceedOrNotFullMessage OK button name OKLabel cancel button name BreakLabel with empty selection allowed)
	if listchoice is false then
		return 2 -- cancelled by user
	end if
else
	set needToFix to false
	display dialog alreadyFixedMsg & " for " & quoted form of (chosenLObundle as string) buttons {OKLabel} default button 1 with icon 1
end if

-- ************************
-- fix programs & libraries
-- ************************
if needToFix then
	repeat with currentFile in list_of_files_to_fix
		set newPath to ""
		if currentFile contains macosRelativePath then -- it is program
			set newPath to newPathForProgs
		else if currentFile contains frameworksRelativePath then -- it is library
			set newPath to newPathForLibs
		end if
		
		if newPath is not "" then
			repeat with libName in listOfLibNames
				repeat with libSysBasepath in libsSystemBasepaths
					set nametool_call to quoted form of (path2tool as string) & " -change " & quoted form of (libSysBasepath & libName) & " " & quoted form of (newPath & libName) & " " & quoted form of (currentFile as string)
					try
						set nametool_result to (do shell script nametool_call)
					on error
						-- just ignore it
					end try
				end repeat
			end repeat
		end if
	end repeat
end if

-- ************************
-- check if everything is fixed
-- show updated list of files
-- ************************
if needToFix then
	if not isOtoolHere then
		-- without otool it's not possible to check if install_name_tool succeeded
		-- let's just hope that everything is okay
	else
		set listToShowToUser to {}
		repeat with currentEntry in list_of_files_to_fix
			set stringOfEntries to libfile_to_entriesFromList(currentEntry, listOfLibNames, path2otool)
			---display dialog currentEntry & strNewline & "----------------------------------------" & strNewline & stringOfEntries
			if stringOfEntries is not "" then
				set listOfEntries to paragraphs in stringOfEntries
				copy currentEntry & ": " & list2string(listOfEntries, ", ", false) to the end of listToShowToUser
			end if
		end repeat
		if (count of listToShowToUser) is 0 then
			copy "¥ there's no files ¥" to the end of listToShowToUser
		end if
		set the listchoice to (choose from list listToShowToUser with prompt messageAfterFixing OK button name OKLabel cancel button name BreakLabel with empty selection allowed)
		if listchoice is false then
			return 2
		end if
	end if
end if

-- ************************
-- now unpack patch's package
-- ************************

set tarListCommand to quoted form of fullPath2tar & " -tjf " & quoted form of ((path2me as string) & "/" & tarballName)
set tarExtractCommand to quoted form of fullPath2tar & " -C " & quoted form of (chosenLObundle as string) & " -xjf " & quoted form of ((path2me as string) & "/" & tarballName)
set tarListCommand to replace_chars(tarListCommand, "//", "/")
set tarExtractCommand to replace_chars(tarExtractCommand, "//", "/")

try
	set tarball_list_all to (do shell script tarListCommand)
	set tarball_list_files to {}
	repeat with currentEntry in (paragraphs in tarball_list_all)
		if currentEntry does not end with "/" then
			copy currentEntry to the end of tarball_list_files
		end if
	end repeat
	set loBundleNameWithoutTrailingSlash to replace_chars(chosenLObundle as string, ".app/", ".app")
	set the tarball_choice to (choose from list tarball_list_files with prompt installOrNotMessage & " " & (loBundleNameWithoutTrailingSlash as string) & "/" OK button name doItLabel cancel button name BreakLabel with empty selection allowed)
	if tarball_choice is false then
		return 2 -- cancelled by user's choice
	end if
on error
	-- do nothing
end try

try
	do shell script tarExtractCommand
on error errMSG number errNUM
	display dialog IdentifyQ buttons {IdentifyYES, IdentifyNO} with icon 2
	if (button returned of result) is IdentifyYES then
		try
			do shell script tarExtractCommand with administrator privileges
		on error errMSG number errNUM
			set identifyFailedFullMessage to identifyFailedMsg & ": " & quote & toResultStringOfIdentifyError(errNUM) & quote
			display dialog identifyFailedFullMessage buttons {OKLabel} default button 1 with icon 0
			return errNUM
		end try
	else
		return 2 -- cancelled by user
	end if
end try

-- ************************
-- fix libgcc_s.1.dylib entry of libstdc++.6.dylib itself
-- ************************
set locationOfLibStdCXXInsideBundle to (chosenLObundle as string) & frameworksRelativePath & "/" & libNameStdcxx
repeat with aBasepath in libsSystemBasepaths
	set nametoolCall to (quoted form of (path2tool as string) & " -change " & quoted form of (aBasepath & libNameGccS) & " " & quoted form of (newPathForLibs & libNameGccS) & " " & quoted form of locationOfLibStdCXXInsideBundle)
	try
		set nametool_result to (do shell script nametoolCall)
	end try
end repeat

-- ************************
-- * b y e
-- ************************
display dialog installCompleteMessage buttons {ByeLabel} default button 1 without icon

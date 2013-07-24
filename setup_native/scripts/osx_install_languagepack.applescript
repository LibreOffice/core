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

This script is meant to
	1) Identify installed instances of the product
	2) check whether the user has write-access (and if not
	ask for authentification)
	3) install the shipped tarball
*)

-- strings for localisations - to be meant to be replaced
-- by a makefile or similar
set OKLabel to "[OKLabel]"
set InstallLabel to "[InstallLabel]"
set AbortLabel to "[AbortLabel]"
set intro to "[IntroText1]

[IntroText2]

[IntroText3]"
set chooseMyOwn to "[ChooseMyOwnText]"
set listPrompt to "[ListPromptText]"
set chooseManual to "[ChooseManualText]"
set listOKLabel to "[ListOKLabelText]"
set listCancelLabel to "[ListCancelLabel]"
set appInvalid to "[AppInvalidText1]

[AppInvalidText2]" -- string will begin with the chosen application's name
set startInstall to "[StartInstallText1]

[StartInstallText2]"
set IdentifyQ to "[IdentifyQText]

[IdentifyQText2]"
set IdentifyYES to "[IdentifyYES]"
set IdentifyNO to "[IdentifyNO]"
set installFailed to "[InstallFailedText]"
set installComplete to "[InstallCompleteText]

[InstallCompleteText2]"

set sourcedir to (do shell script "dirname " & quoted form of POSIX path of (path to of me))

activate
display dialog intro buttons {AbortLabel, InstallLabel} default button 2

if (button returned of result) is AbortLabel then
	return 2
end if

set found_ooos_all to ""
-- command might return an error if spotlight is disabled completely
try
	set found_ooos_all to (do shell script "mdfind \"kMDItemContentType == 'com.apple.application-bundle' && kMDItemDisplayName == '[PRODUCTNAME]*' && kMDItemDisplayName != '[FULLAPPPRODUCTNAME].app'\"")
end try
set found_ooos_all to found_ooos_all & "
" & chooseMyOwn

set found_ooos_all_paragraphs to paragraphs in found_ooos_all

set found_ooos to {}
repeat with currentApp in found_ooos_all_paragraphs
	if currentApp does not start with "/Volumes" then
		copy currentApp to the end of found_ooos
	end if
end repeat

-- repeat with oneApp in found_ooos
--  display dialog oneApp
-- end repeat

-- the choice returned is of type "list"
-- Show selection dialog only if more than one or no product was found
-- The first item is an empty string, if no app was found and no app started with "/Volumes"
-- The first item is chooseMyOwn, if no app was found and at least one app started with "/Volumes"
if (get first item of found_ooos as string) is "" then
  set the choice to (choose from list found_ooos default items (get second item of found_ooos) with prompt listPrompt OK button name listOKLabel cancel button name listCancelLabel)
  if choice is false then
	  -- do nothing, the user cancelled the installation
    	return 2 --aborted by user
  else if (choice as string) is chooseMyOwn then
	  -- yeah, one needs to use "choose file", otherwise
	  -- the user would not be able to select the .app
	  set the choice to POSIX path of (choose file with prompt chooseManual of type "com.apple.application-bundle" without showing package contents and invisibles)
  end if
else if (get first item of found_ooos as string) is chooseMyOwn then
  set the choice to (choose from list found_ooos default items (get first item of found_ooos) with prompt listPrompt OK button name listOKLabel cancel button name listCancelLabel)
  if choice is false then
	  -- do nothing, the user cancelled the installation
    	return 2 --aborted by user
  else if (choice as string) is chooseMyOwn then
	  -- yeah, one needs to use "choose file", otherwise
	  -- the user would not be able to select the .app
	  set the choice to POSIX path of (choose file with prompt chooseManual of type "com.apple.application-bundle" without showing package contents and invisibles)
  end if
else if (get second item of found_ooos as string) is chooseMyOwn then
  -- set choice to found installation
  -- set the choice to (get first paragraph of found_ooos)
  set the choice to (get first item of found_ooos)
else 
  set the choice to (choose from list found_ooos default items (get first item of found_ooos) with prompt listPrompt OK button name listOKLabel cancel button name listCancelLabel)
  if choice is false then
	  -- do nothing, the user cancelled the installation
    	return 2 --aborted by user
  else if (choice as string) is chooseMyOwn then
	  -- yeah, one needs to use "choose file", otherwise
	  -- the user would not be able to select the .app
	  set the choice to POSIX path of (choose file with prompt chooseManual of type "com.apple.application-bundle" without showing package contents and invisibles)
  end if
end if	

-- now only check whether the path is really from [PRODUCTNAME]
try
	do shell script "grep '<string>[PRODUCTNAME] [PRODUCTVERSION]'   " & quoted form of (choice as string) & "/Contents/Info.plist"
on error
	display dialog (choice as string) & appInvalid buttons {InstallLabel} default button 1 with icon 0
	return 3 --wrong target-directory
end try

(*
display dialog startInstall buttons {AbortLabel, InstallLabel} default button 2

if (button returned of result) is AbortLabel then
	return 2
end if
*)

set tarCommand to "/usr/bin/tar -C " & quoted form of (choice as string) & " -xjf " & quoted form of sourcedir & "/tarball.tar.bz2"
try
	do shell script tarCommand
	
on error errMSG number errNUM
	display dialog IdentifyQ buttons {IdentifyYES, IdentifyNO} with icon 2
	if (button returned of result) is IdentifyYES then
		try
			do shell script tarCommand with administrator privileges
		on error errMSG number errNUM
			display dialog installFailed buttons {OKLabel} default button 1 with icon 0
			-- -60005 username/password wrong
			-- -128   aborted by user
			-- 2 error from tar - tarball not found (easy to test)
			return errNUM
		end try
	else
		return 2 -- aborted by user
	end if
end try

display dialog installComplete buttons {OKLabel} default button 1

'**************************************************************************
' DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'
' Copyright 2000, 2010 Oracle and/or its affiliates.
'
' OpenOffice.org - a multi-platform office productivity suite
'
' This file is part of OpenOffice.org.
'
' OpenOffice.org is free software: you can redistribute it and/or modify
' it under the terms of the GNU Lesser General Public License version 3
' only, as published by the Free Software Foundation.
'
' OpenOffice.org is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty of
' MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' GNU Lesser General Public License version 3 for more details
' (a copy is included in the LICENSE file that accompanied this code).
'
' You should have received a copy of the GNU Lesser General Public License
' version 3 along with OpenOffice.org.  If not, see
' <http://www.openoffice.org/license.html>
' for a copy of the LGPLv3 License.
'
'/******************************************************************
'*                                                                **
'* owner : helge.delfs@oracle.com                                    **
'*                                                                **
'* short description : Misc tools for writer hyphenation tests    **
'*                                                                **
'\******************************************************************



function fPrepareHyphenationDocument(testWord as string)

	Dim SpaceTickCount as integer
	
	SpaceTickCount = 1
	Do
		Call wTypeKeys "<Mod1 Home>"
		Call wTypeKeys "<Down>"
		Call wTypeKeys "<Mod1 Shift Right>"
		try
			EditCopy
		catch
		endcatch
		if trim(GetClipboardText) <> testWord then
			EditSearchAndReplace
			Kontext "FindAndReplace"
			SearchFor.SetText testWord
			SearchNow.Click
			Kontext "Active"
			if Active.Exists then
				if Active.GetRT = 304 then
					try
						active.Yes
					catch
						Warnlog Active.Gettext
						Active.ok
					endcatch
				end if
			end if
			Kontext "FindAndReplace"
			FindAndReplace.Close
			Call wTypeKeys "<Mod1 Left>"
			Call wTypeKeys "<Space>"
			inc SpaceTickCount
			if SpaceTickCount >= 30 then
				fPrepareHyphenationDocument = false
				exit do
			end if
		else
			fPrepareHyphenationDocument = true
			exit do			
		end if
	Loop

end function

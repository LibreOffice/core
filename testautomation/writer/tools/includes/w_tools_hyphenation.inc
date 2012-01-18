'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
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

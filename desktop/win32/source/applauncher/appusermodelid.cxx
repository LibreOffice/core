/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Jesús Corrius <jesus@softcatala.org> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "appusermodelid.hxx"

#include <winbase.h>
#include <shlwapi.h>

#define PACKVERSION(major,minor) MAKELONG(minor,major)

void SetExplicitAppUserModelID(PCWSTR AppID)
{
	WCHAR szShell32[MAX_PATH];
	GetSystemDirectoryW(szShell32, MAX_PATH);
	wcscat_s(szShell32, MAX_PATH, L"\\Shell32.dll");

	HINSTANCE hinstDll = LoadLibraryW(szShell32);

	if(hinstDll)
	{
		DLLVERSIONINFO dvi;
		ZeroMemory(&dvi, sizeof(dvi));
		dvi.cbSize = sizeof(dvi);

		DLLGETVERSIONPROC pDllGetVersion;
		pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, "DllGetVersion");
		HRESULT hr = (*pDllGetVersion)(&dvi);

		if(SUCCEEDED(hr))
		{
			DWORD dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
			if(dwVersion >= PACKVERSION(6,1)) // Shell32 version in Windows 7
			{
				typedef HRESULT (WINAPI *SETCURRENTPROCESSEXPLICITAPPUSERMODELID)(PCWSTR);
				SETCURRENTPROCESSEXPLICITAPPUSERMODELID pSetCurrentProcessExplicitAppUserModelID;
				pSetCurrentProcessExplicitAppUserModelID =
					(SETCURRENTPROCESSEXPLICITAPPUSERMODELID)GetProcAddress(hinstDll, "SetCurrentProcessExplicitAppUserModelID");

				if(pSetCurrentProcessExplicitAppUserModelID)
					(*pSetCurrentProcessExplicitAppUserModelID) (AppID);
			}
		}
	}
	FreeLibrary(hinstDll);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

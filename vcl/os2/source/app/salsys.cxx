/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salsys.cxx,v $
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
 ************************************************************************/

#define _SV_SALSYS_CXX

#include <string.h>

#define INCL_DOS
#define INCL_DOSERRORS
#ifndef _SVPM_H
#include <svpm.h>
#endif
#include <saldata.hxx>
#include <salinst.hxx>
#include <salframe.hxx>
#include <salbmp.hxx>
#include <salsys.hxx>
#ifndef _VOS_PROCESS_HXX
#include <vos/process.hxx>
#endif
#ifndef _TOOLS_FASTFSYS_HXX
#include <tools/fastfsys.hxx>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// =======================================================================

SalSystem* SalInstance::CreateSystem()
{
    return new SalSystem();
}

// -----------------------------------------------------------------------

void SalInstance::DestroySystem( SalSystem* pSystem )
{
    delete pSystem;
}

// -----------------------------------------------------------------------

SalSystem::SalSystem()
{
}

// -----------------------------------------------------------------------

SalSystem::~SalSystem()
{
}

// -----------------------------------------------------------------------

BOOL SalSystem::StartProcess( SalFrame* pFrame, const XubString& rFileName,
                              const XubString& rParam,
                              const XubString& rWorkingDirectory )
{
    ItemIDPath aFile(rFileName);

    //
    // first check if item has a context menu with open command
    //

    ItemIDPath aParent, aChild;

    if(aFile.Split(aParent, aChild))
    {
        IfcContextMenu *pConMenu = Folder(aParent).GetContextMenu(1, &aChild);

        if(pConMenu)
        {
            UINT32 nCount = pConMenu->GetItemCount();

            for(UINT32 n = 0; n < nCount; n++)
            {
                MenuItem aMenuItem;

                if(pConMenu->GetMenuItem(n, aMenuItem) &&
                   aMenuItem.aVerb.Compare("open") == COMPARE_EQUAL)
                {
                   return pConMenu->ExecuteCommand(aMenuItem.aCommand);
                }
            }
        }
    }

    // Dateinamen mit Wildcards lehnen wir ab
    if ( (rFileName.Search( '*' ) != STRING_NOTFOUND) ||
         (rFileName.Search( '?' ) != STRING_NOTFOUND) )
        return FALSE;

    XubString       aFileName = aFile.GetHostNotationPath();
    char*           aStdExtAry[] = { "exe", "com", "cmd", "bat" };
    const char*     pStdExt;
    const xub_Unicode*  pFileName = aFileName.GetStr();
    const xub_Unicode*  pParam = rParam.GetStr();
    XubString       aSearchFileName;
    XubString       aExt;
    BOOL            bExe = FALSE;
    BOOL            bProcess = FALSE;
    BOOL            bRet = FALSE;
    BOOL            bExtension;
    int             i;

    // Parameter und Extension ermitteln
    if ( !rParam.Len() )
        pParam = NULL;
    // Wenn keine Extension, dann versuchen wir eine zu ermitteln,
    // indem wir nach EXE-, COM-, CMD- oder BAT-Dateien suchen.
    bExtension = ImplSalGetExtension( aFileName, aExt );
    if ( !bExtension )
    {
        i = 0;
        do
        {
            pStdExt = aStdExtAry[i];
            aSearchFileName = aFileName;
            aSearchFileName += '.';
            aSearchFileName += pStdExt;
            if ( ImplSalFindFile( aSearchFileName.GetStr(), aSearchFileName ) )
            {
                pFileName = aSearchFileName.GetStr();
                bExtension = ImplSalGetExtension( aSearchFileName, aExt );
                break;
            }
            i++;
        }
        while ( i < 4 );
    }
    else
    {
        // Ansonsten Filename im Pfad suchen
        if ( ImplSalFindFile( pFileName, aSearchFileName ) )
        {
            pFileName = aSearchFileName.GetStr();
            bExtension = ImplSalGetExtension( aSearchFileName, aExt );
        }
    }
    // Wenn wir eine Extension haben, testen wir, ob es eine
    // Standard-Extension ist, womit wir einen Process starten koennen
    if ( bExtension )
    {
        aExt.ToLower();
        i = 0;
        do
        {
            if ( aExt == aStdExtAry[i] )
            {
                bExe = TRUE;
                break;
            }
            i++;
        }
        while ( i < 4 );
    }

    // change to path of executable if no working dir set
    XubString aWorkingDir(rWorkingDirectory);

    if(aWorkingDir.Len() == 0)
    {
        USHORT nIndex;

        aWorkingDir = pFileName;
        nIndex = aWorkingDir.SearchCharBackward("\\/:");

        if(nIndex == STRING_NOTFOUND)
            nIndex = 0;
        else if(aWorkingDir.GetChar(nIndex) == ':')
            aWorkingDir[nIndex++] = '\\';

        aWorkingDir.Erase(nIndex);
    }

    // start executables with process execute
    if ( bExe )
    {
        NAMESPACE_VOS( OProcess )::TProcessError nProcessError;
        NAMESPACE_VOS( OProcess ) aProcess( pFileName, pParam );

        aProcess.setDirectory(aWorkingDir.GetStr());

        nProcessError = aProcess.execute( (NAMESPACE_VOS(OProcess)::TProcessOption)
                                          (NAMESPACE_VOS(OProcess)::TOption_Detached) );
        bRet = nProcessError == NAMESPACE_VOS( OProcess )::E_None;
    }

    // when not startet, start programm with WPS
    if ( !bRet )
    {
        HOBJECT hObject = WinQueryObject( pFileName );
        if ( hObject )
        {
            if ( WinOpenObject( hObject, 0, FALSE ) )
                bRet = TRUE;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------

BOOL SalSystem::AddRecentDoc( SalFrame* pFrame, const XubString& rFileName )
{
    return FALSE;
}

// -----------------------------------------------------------------------

String SalSystem::GetSummarySystemInfos( ULONG nFlags )
{
    return String();
}


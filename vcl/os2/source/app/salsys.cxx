/*************************************************************************
 *
 *  $RCSfile: salsys.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_SALSYS_CXX

#include <string.h>

#define INCL_DOS
#define INCL_DOSERRORS
#ifndef _SVPM_H
#include <tools/svpm.h>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_SALBMP_HXX
#include <salbmp.hxx>
#endif
#ifndef _SV_SALSYS_HXX
#include <salsys.hxx>
#endif
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


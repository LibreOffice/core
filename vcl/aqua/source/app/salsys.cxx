/*************************************************************************
 *
 *  $RCSfile: salsys.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:26 $
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

#include <string.h>

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#define _SV_SALSYS_CXX

#ifndef _SV_SALAQUA_HXX
#include <salaqua.hxx>
#endif
#ifndef _SV_SALBMP_HXX
#include <salbmp.hxx>
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
#ifndef _SV_SALSYS_HXX
#include <salsys.hxx>
#endif

#ifndef _SV_KEYCOES_HXX
#include <keycodes.hxx>
#endif

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

// =======================================================================

SalSystem::SalSystem()
{
}

// -----------------------------------------------------------------------

SalSystem::~SalSystem()
{
}

// -----------------------------------------------------------------------

BOOL SalSystem::StartProcess( SalFrame* pFrame, const XubString& rFileName,
                              const XubString& rParam, const XubString& rWorkDir )
{
    return FALSE;
    // !!! UNICODE Was ist ... --> Mit HRO nach seinem Urlaub klaeren
/*
    XubString   aParam = rParam;
    XubString   aFileName = rFileName;

    // HRO: #75283#
    // Hack for Office2000 Links
    // We can only start the lnk-file, so we packed it within the arguments
    xub_StrLen nIndex = aParam.Search( (sal_Unicode)0x07 );
    if ( nIndex != STRING_NOTFOUND )
    {
        aFileName = aParam.Copy( nIndex + 1 );
        aParam.Erase( nIndex );
    }

    BOOL bSuccess;
    if ( aSalShlData.mbWNT )
    {
        LPCWSTR pszWorkDir = NULL;
        if ( rWorkDir.Len() )
            pszWorkDir = rWorkDir.GetBuffer();
        bSuccess = SHInvokeCommandW( pFrame->maFrameData.mhWnd, 0,
                                     aFileName.GetBuffer(),
                                     CMDSTR_DEFAULT,
                                     aParam.GetBuffer(), pszWorkDir,
                                     SW_SHOWNORMAL );
    }
    else
    {
        ByteString  aFileNameA  = ImplSalGetWinAnsiString( aFileName, TRUE );
        ByteString  aParamA     = ImplSalGetWinAnsiString( aParam, TRUE );
        ByteString  aWorkDirA   = ImplSalGetWinAnsiString( rWorkDir, TRUE );
        LPCSTR      pszWorkDir  = NULL;
        if ( aWorkDirA.Len() )
            pszWorkDir = aWorkDirA.GetBuffer();
        bSuccess = SHInvokeCommandA( pFrame->maFrameData.mhWnd, 0,
                                     aFileNameA.GetBuffer(),
                                     CMDSTR_DEFAULT,
                                     aParamA.GetBuffer(), pszWorkDir,
                                     SW_SHOWNORMAL );
    }
*/
/*
    // HRO: Tasks #62485# #64619#
    // Weil ein paar Naddels jeden Scheiss hier reinstopfen und sich nicht
    // entscheiden koennen, was sie wie und wann aufrufen, darf ich
    // um die BUGs drumrumkurven. GRRRRR !!!

    if ( !bSuccess )
    {
        ItemIDPath aIDPath( aFileName );

        if ( aIDPath.GetTokenCount() )
            bSuccess = WIN_SHInvokeCommand( pFrame->maFrameData.mhWnd, SHIC_PIDL,
                (LPCTSTR)aIDPath.GetDataPtr(), CMDSTR_DEFAULT, aParam.GetStr(),
                pszWorkDir, SW_SHOWNORMAL );
    }
*/
//    return bSuccess;
}

// -----------------------------------------------------------------------

BOOL SalSystem::AddRecentDoc( SalFrame*, const XubString& rFileName )
{
    return FALSE;
    // !!! UNICODE Was ist ... --> Mit HRO nach seinem Urlaub klaeren
/*
    if ( aSalShlData.mbWNT )
        SHAddToRecentDocsW( SHARD_PATH, (LPCVOID)rFileName.GetBuffer() );
    else
    {
        ByteString aFileNameA  = ImplSalGetWinAnsiString( rFileName, TRUE );
        SHAddToRecentDocsA( SHARD_PATH, (LPCVOID)aFileNameA.GetBuffer() );
    }
    return TRUE;
*/
}

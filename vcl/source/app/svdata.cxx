/*************************************************************************
 *
 *  $RCSfile: svdata.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:35 $
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

#define _SV_SVDATA_CXX

#ifndef REMOTE_APPSERVER
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#endif

#ifndef _VOS_MUTEX_HXX
#include <vos/mutex.hxx>
#endif

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#define private public
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_WINDOW_H
#include <window.h>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <wrkwin.hxx>
#endif

#ifndef _VCL_UNOWRAP_HXX
#include <unowrap.hxx>
#endif

#pragma hdrstop

// =======================================================================

// static Empty-SV-String
static XubString aImplSVEmptyStr;
XubString& rImplSVEmptyStr = aImplSVEmptyStr;
#ifdef ENABLEUNICODE
static ByteString aImplSVEmptyByteStr;
ByteString& rImplSVEmptyByteStr = aImplSVEmptyByteStr;
#endif

#ifndef WIN
ImplSVData private_aImplSVData;
// static SV-Data
ImplSVData* pImplSVData = &private_aImplSVData;
#endif

// static SharedLib SV-Data
ImplSVShlData aImplSVShlData;

// =======================================================================

void ImplInitSVData()
{
#ifndef WIN
    ImplSVData* pSVData = pImplSVData;
    ImplSVData** ppSVData = (ImplSVData**)GetAppData( SHL_SV );
    *ppSVData = &private_aImplSVData;
#else
    // alloc global instance data
    ImplSVData* pSVData = new ImplSVData;
    ImplSVData** ppSVData = (ImplSVData**)GetAppData( SHL_SV );
    *ppSVData = pSVData;
#endif

    // init global sharedlib data
    // ...

    // init global instance data
    memset( pSVData, 0, sizeof( ImplSVData ) );
}

// -----------------------------------------------------------------------

void ImplDeInitSVData()
{
    ImplSVData* pSVData = ImplGetSVData();

    // delete global instance data

    if ( pSVData->mpUnoWrapper )
    {
        pSVData->mpUnoWrapper->Destroy();
        pSVData->mpUnoWrapper = NULL;
    }
}

// -----------------------------------------------------------------------

void ImplDestroySVData()
{
    ImplSVData** ppSVData = (ImplSVData**)GetAppData( SHL_SV );
    ImplSVData*  pSVData = *ppSVData;
#ifdef WIN
    delete pSVData;
#endif

    // delete global sharedlib data
    // ...

    *ppSVData = NULL;
#ifndef WIN
    pImplSVData = NULL;
#endif
}

// -----------------------------------------------------------------------

Window* ImplGetDefaultWindow()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maWinData.mpAppWin )
        return pSVData->maWinData.mpAppWin;

    if ( !pSVData->mpDefaultWin )
    {
        DBG_WARNING( "ImplGetDefaultWindow(): No AppWindow" );
        Application::GetSolarMutex().acquire();
        pSVData->mpDefaultWin = new WorkWindow( 0, 0 );
        Application::GetSolarMutex().release();
    }

    return pSVData->mpDefaultWin;
}

// -----------------------------------------------------------------------

#define VCL_CREATERESMGR_NAME( Name )   #Name MAKE_NUMSTR( SUPD )
#define VCL_CREATERESMGR( Name )        ResMgr::CreateResMgr( VCL_CREATERESMGR_NAME( Name ) )

ResMgr* ImplGetResMgr()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->mpResMgr )
        pSVData->mpResMgr = VCL_CREATERESMGR( vcl );
    return pSVData->mpResMgr;
}

// -----------------------------------------------------------------------

#ifndef REMOTE_APPSERVER

Window* ImplFindWindow( const SalFrame* pFrame, Point& rSalFramePos )
{
    ImplSVData* pSVData = ImplGetSVData();
    Window*     pFrameWindow = pSVData->maWinData.mpFirstFrame;
    while ( pFrameWindow )
    {
        if ( pFrameWindow->ImplGetFrame() == pFrame )
        {
            Window* pWindow = pFrameWindow->ImplFindWindow( rSalFramePos );
            if ( !pWindow )
                pWindow = pFrameWindow->ImplGetWindow();
            rSalFramePos = pWindow->ImplFrameToOutput( rSalFramePos );
            return pWindow;
        }
        pFrameWindow = pFrameWindow->mpFrameData->mpNextFrame;
    }

    return NULL;
}

#endif

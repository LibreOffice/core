/*************************************************************************
 *
 *  $RCSfile: svdata.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: th $ $Date: 2001-07-25 10:52:11 $
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

#include <osl/file.hxx>

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

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#include <com/sun/star/lang/XComponent.hpp>

#include <stdio.h>

#pragma hdrstop

// =======================================================================

// static Empty-SV-String
static XubString aImplSVEmptyStr;
XubString& rImplSVEmptyStr = aImplSVEmptyStr;
static ByteString aImplSVEmptyByteStr;
ByteString& rImplSVEmptyByteStr = aImplSVEmptyByteStr;

ImplSVData private_aImplSVData;
// static SV-Data
ImplSVData* pImplSVData = &private_aImplSVData;

// static SharedLib SV-Data
ImplSVShlData aImplSVShlData;

// =======================================================================

void ImplInitSVData()
{
    ImplSVData* pSVData = pImplSVData;
    ImplSVData** ppSVData = (ImplSVData**)GetAppData( SHL_SV );
    *ppSVData = &private_aImplSVData;

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

    if ( pSVData->maAppData.mpMSFTempFileName )
    {
        if ( pSVData->maAppData.mxMSF.is() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xComp( pSVData->maAppData.mxMSF, ::com::sun::star::uno::UNO_QUERY );
            xComp->dispose();
            pSVData->maAppData.mxMSF = NULL;
        }

        ::rtl::OUString aFileUrl;
        ::osl::File::getFileURLFromSystemPath( *pSVData->maAppData.mpMSFTempFileName, aFileUrl );
        osl::File::remove( aFileUrl );
        delete pSVData->maAppData.mpMSFTempFileName;
        pSVData->maAppData.mpMSFTempFileName = NULL;
    }
}

// -----------------------------------------------------------------------

void ImplDestroySVData()
{
    ImplSVData** ppSVData = (ImplSVData**)GetAppData( SHL_SV );
    ImplSVData*  pSVData = *ppSVData;

    // delete global sharedlib data
    // ...

    *ppSVData = NULL;
    pImplSVData = NULL;
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

ResMgr* ImplGetResMgr()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->mpResMgr )
    {
        pSVData->mpResMgr = ResMgr::CreateResMgr( VCL_CREATERESMGR_NAME( vcl ) );
        if ( !pSVData->mpResMgr )
        {
            LanguageType eLang = Application::GetSettings().GetUILanguage();
            pSVData->mpResMgr = ResMgr::SearchCreateResMgr( VCL_CREATERESMGR_NAME( vcl ), eLang );
        }
    }
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

// -----------------------------------------------------------------------

#ifdef REMOTE_APPSERVER

void rvpExceptionHandler()
{
#ifdef DBG_UTIL
    fprintf( stderr, "RVP exception caught!\n" );
#endif
}

#endif

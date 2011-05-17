/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <svl/httpcook.hxx>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>

#include <cookiedg.hrc>
#include <cookiedg.hxx>
#include <ids.hrc>

// CookiesDialog ---------------------------------------------------------

IMPL_LINK( CookiesDialog, ButtonHdl_Impl, PushButton *, pBtn )
{
    short nRet = ( &maSendBtn == pBtn ) ? RET_OK : RET_CANCEL;
    EndDialog( nRet );
    return 1;
}

// -----------------------------------------------------------------------
CookiesDialog::CookiesDialog( Window* pParent,
                              CntHTTPCookieRequest* pRequest,
                              ResMgr* pResMgr ) :

    ModalDialog( pParent, ResId( DLG_COOKIES, *pResMgr ) ),

    maCookieFB              ( this, ResId( FB_COOKIES, *pResMgr ) ),
    maCookieFT              ( this, ResId( FT_COOKIES, *pResMgr ) ),
    maInFutureLine          ( this, ResId( FL_COOKIES, *pResMgr ) ),
    maInFutureSendBtn       ( this, ResId( RB_INFUTURE_SEND, *pResMgr ) ),
    maInFutureIgnoreBtn     ( this, ResId( RB_INFUTURE_IGNORE, *pResMgr ) ),
    maInFutureInteractiveBtn( this, ResId( RB_INFUTURE_INTERACTIVE, *pResMgr ) ),
    maInFutureGB            ( this, ResId( GB_INFUTURE, *pResMgr ) ),
    maIgnoreBtn             ( this, ResId( BTN_COOKIES_CANCEL, *pResMgr ) ),
    maSendBtn               ( this, ResId( BTN_COOKIES_OK, *pResMgr ) ),

    mpCookieRequest         ( pRequest )

{
    FreeResource();

    Link aLink( LINK( this, CookiesDialog, ButtonHdl_Impl ) );
    maIgnoreBtn.SetClickHdl( aLink );
    maSendBtn.SetClickHdl( aLink );
    const Bitmap& rBitmap = maCookieFB.GetBitmap();
    Size aSize = rBitmap.GetSizePixel();
    SetMapMode( MapMode( MAP_APPFONT ) );
    Size aLogicSize = PixelToLogic( aSize );
    Point aPoint( 6 ,
                  6 + ( 145 - aLogicSize.Height() ) / 2 );
    maCookieFB.SetPosSizePixel( LogicToPixel( aPoint ), aSize );
    maCookieFB.Show();

    sal_uInt16 nOffset = CNTHTTP_COOKIE_REQUEST_RECV == mpCookieRequest->m_eType
        ? 0 : STR_COOKIES_SEND_START - STR_COOKIES_RECV_START;
    INetURLObject aObj( mpCookieRequest->m_rURL );
    SetText( String( ResId( STR_COOKIES_RECV_TITLE + nOffset, *pResMgr ) ) );
    String aMsg( ResId( STR_COOKIES_RECV_START + nOffset, *pResMgr ) );
    aMsg.SearchAndReplaceAscii( "${HOST}", aObj.GetHost() );
    aMsg.SearchAndReplaceAscii( "${PATH}", aObj.GetPath() );
    String aTemplate( ResId( STR_COOKIES_RECV_COOKIES, *pResMgr ) );
    List& rList =mpCookieRequest->m_rCookieList;
    String aPair, aCookie;

    for ( sal_uInt16 i = (sal_uInt16)rList.Count(); i--; )
    {
        CntHTTPCookie* pCookie = (CntHTTPCookie*)rList.GetObject(i);

        if ( CNTHTTP_COOKIE_POLICY_INTERACTIVE == pCookie->m_nPolicy )
        {
            aCookie = aTemplate;
            aCookie.SearchAndReplaceAscii( "${DOMAIN}", pCookie->m_aDomain );
            aCookie.SearchAndReplaceAscii( "${PATH}", pCookie->m_aPath );
            aPair = pCookie->m_aName;
            aPair += '=';
            aPair += pCookie->m_aValue;
            aCookie.SearchAndReplaceAscii( "${COOKIE}", aPair );
            aMsg += aCookie;
        }
    }
    maInFutureInteractiveBtn.Check( sal_True );
    maCookieFT.SetText( aMsg );
}

// -----------------------------------------------------------------------

short CookiesDialog::Execute()
{
    maSendBtn.GrabFocus();
    short nRet = ModalDialog::Execute();
    sal_uInt16 nStatus = CNTHTTP_COOKIE_POLICY_INTERACTIVE;

    if ( maInFutureSendBtn.IsChecked() )
        nStatus = CNTHTTP_COOKIE_POLICY_ACCEPTED;

    if ( maInFutureIgnoreBtn.IsChecked() )
        nStatus = CNTHTTP_COOKIE_POLICY_BANNED;
    List& rList = mpCookieRequest->m_rCookieList;

    for ( sal_uInt16 i = (sal_uInt16)rList.Count(); i--; )
    {
        sal_uInt16& rStatus = ( (CntHTTPCookie*)rList.GetObject(i) )->m_nPolicy;

        if ( rStatus == CNTHTTP_COOKIE_POLICY_INTERACTIVE )
            rStatus = nStatus;
    }

    if ( nRet == RET_OK )
        mpCookieRequest->m_nRet = CNTHTTP_COOKIE_POLICY_ACCEPTED;
    else
        mpCookieRequest->m_nRet = CNTHTTP_COOKIE_POLICY_BANNED;

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include <svl/httpcook.hxx>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>

#ifndef UUI_COOKIEDG_HRC
#include <cookiedg.hrc>
#endif
#include <cookiedg.hxx>
#ifndef UUI_IDS_HRC
#include <ids.hrc>
#endif

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


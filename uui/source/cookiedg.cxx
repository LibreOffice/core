/*************************************************************************
 *
 *  $RCSfile: cookiedg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:04:10 $
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

#ifndef SVTOOLS_HTTPCOOK_HXX
#include <svtools/httpcook.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#ifndef UUI_COOKIEDG_HRC
#include <cookiedg.hrc>
#endif
#ifndef UUI_COOKIEDG_HXX
#include <cookiedg.hxx>
#endif
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

    ModalDialog( pParent, ResId( DLG_COOKIES, pResMgr ) ),

    maCookieFB              ( this, ResId( FB_COOKIES ) ),
    maCookieFT              ( this, ResId( FT_COOKIES ) ),
    maInFutureLine          ( this, ResId( FL_COOKIES ) ),
    maInFutureSendBtn       ( this, ResId( RB_INFUTURE_SEND ) ),
    maInFutureIgnoreBtn     ( this, ResId( RB_INFUTURE_IGNORE ) ),
    maInFutureInteractiveBtn( this, ResId( RB_INFUTURE_INTERACTIVE ) ),
    maInFutureGB            ( this, ResId( GB_INFUTURE ) ),
    maIgnoreBtn             ( this, ResId( BTN_COOKIES_CANCEL ) ),
    maSendBtn               ( this, ResId( BTN_COOKIES_OK ) ),

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

    USHORT nOffset = CNTHTTP_COOKIE_REQUEST_RECV == mpCookieRequest->m_eType
        ? 0 : STR_COOKIES_SEND_START - STR_COOKIES_RECV_START;
    INetURLObject aObj( mpCookieRequest->m_rURL );
    SetText( String( ResId( STR_COOKIES_RECV_TITLE + nOffset, pResMgr ) ) );
    String aMsg( ResId( STR_COOKIES_RECV_START + nOffset, pResMgr ) );
    aMsg.SearchAndReplaceAscii( "${HOST}", aObj.GetHost() );
    aMsg.SearchAndReplaceAscii( "${PATH}", aObj.GetPath() );
    String aTemplate( ResId( STR_COOKIES_RECV_COOKIES, pResMgr ) );
    List& rList =mpCookieRequest->m_rCookieList;
    String aPair, aCookie;

    for ( USHORT i = (USHORT)rList.Count(); i--; )
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
    maInFutureInteractiveBtn.Check( TRUE );
    maCookieFT.SetText( aMsg );
}

// -----------------------------------------------------------------------

short CookiesDialog::Execute()
{
    maSendBtn.GrabFocus();
    short nRet = ModalDialog::Execute();
    USHORT nStatus = CNTHTTP_COOKIE_POLICY_INTERACTIVE;

    if ( maInFutureSendBtn.IsChecked() )
        nStatus = CNTHTTP_COOKIE_POLICY_ACCEPTED;

    if ( maInFutureIgnoreBtn.IsChecked() )
        nStatus = CNTHTTP_COOKIE_POLICY_BANNED;
    List& rList = mpCookieRequest->m_rCookieList;

    for ( USHORT i = (USHORT)rList.Count(); i--; )
    {
        USHORT& rStatus = ( (CntHTTPCookie*)rList.GetObject(i) )->m_nPolicy;

        if ( rStatus == CNTHTTP_COOKIE_POLICY_INTERACTIVE )
            rStatus = nStatus;
    }

    if ( nRet == RET_OK )
        mpCookieRequest->m_nRet = CNTHTTP_COOKIE_POLICY_ACCEPTED;
    else
        mpCookieRequest->m_nRet = CNTHTTP_COOKIE_POLICY_BANNED;

    return nRet;
}


/*************************************************************************
 *
 *  $RCSfile: txtatr2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:27 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif

#ifndef _XMLOFF_XMLCNITM_HXX
#include <xmloff/xmlcnitm.hxx>
#endif

#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_KERNITEM_HXX //autogen
#include <svx/kernitem.hxx>
#endif
#ifndef _SVX_NHYPITEM_HXX //autogen
#include <svx/nhypitem.hxx>
#endif
#ifndef _SVX_BLNKITEM_HXX //autogen
#include <svx/blnkitem.hxx>
#endif
#ifndef _SVX_CMAPITEM_HXX //autogen
#include <svx/cmapitem.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX //autogen
#include <svx/escpitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif

#ifndef _TXTINET_HXX //autogen
#include <txtinet.hxx>
#endif
#ifndef _TXTATR_HXX //autogen
#include <txtatr.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _CHARATR_HXX
#include <charatr.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef _SWFONT_HXX
#include <swfont.hxx>
#endif
#ifndef _FNTCACHE_HXX
#include <fntcache.hxx>     // SwFntAccess
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>        // SwCharFmt, SwTxtNode
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>        // SwCharFmt, SwUpdateAttr
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>      // RES_POOLCHR_INET_...
#endif
#ifndef _DOC_HXX
#include <doc.hxx>          // SwDoc
#endif

TYPEINIT1(SwTxtINetFmt,SwClient);

/*************************************************************************
 *                      class SwTxtCharFmt
 *************************************************************************/

SwTxtCharFmt::~SwTxtCharFmt( )
{
    delete pPrevFont;
    delete pPrevColor;
}


SwTxtCharFmt::SwTxtCharFmt( const SwFmtCharFmt& rAttr,
                    xub_StrLen nStart, xub_StrLen nEnd )
    : SwTxtAttrEnd( rAttr, nStart, nEnd ),
    pMyTxtNd( 0 ),
    pPrevFont( 0 ),
    pPrevColor( 0 )
{
    ((SwFmtCharFmt&)rAttr).pTxtAttr = this;
}

void SwTxtCharFmt::ChgFnt( SwFont *pFont )
{
    SwCharFmt* pFmt = SwTxtAttrEnd::GetCharFmt().GetCharFmt();
    if ( pFmt )
    {
        // Das 0 != bNew-Geraffel bringt M80 zum Schweigen
        if ( pPrevFont )
            *pPrevFont = pFont->GetFnt( SW_LATIN );
        else
            pPrevFont = new SvxFont( pFont->GetFnt( SW_LATIN ) );
        bPrevNoHyph = pFont->IsNoHyph();
        bPrevBlink = pFont->IsBlink();
        bPrevURL = pFont->IsURL();
        pFont->GetMagic( pFontNo, nFntIndex, SW_LATIN );
        pFont->SetDiffFnt( &pFmt->GetAttrSet() );
        delete pPrevColor;
        bColor = SFX_ITEM_SET ==
                 pFmt->GetAttrSet().GetItemState( RES_CHRATR_BACKGROUND, TRUE );
        if( bColor )
        {
            pPrevColor = new Color( pFmt->GetChrBackground().GetColor() );
            pPrevColor = pFont->XChgBackColor( pPrevColor );
        }
        else
            pPrevColor = NULL;
    }
}

void SwTxtCharFmt::RstFnt(SwFont *pFont)
{
    if ( pPrevFont )
    {
        // Das 0 != bNew-Geraffel bringt M80 zum Schweigen
        pFont->SetFnt( *pPrevFont, SW_LATIN );
        pFont->SetMagic( pFontNo, nFntIndex, SW_LATIN );
        pFont->SetBlink( bPrevBlink );
        pFont->SetNoHyph( bPrevNoHyph );
        pFont->SetURL( bPrevURL );
        if( bColor )
        {
            pFont->SetBackColor( pPrevColor );
            pPrevColor = NULL;
        }
    }
}

void SwTxtCharFmt::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
{
    USHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
#ifndef PRODUCT
    if ( (nWhich<RES_CHRATR_BEGIN || nWhich>RES_CHRATR_END)
            && (nWhich!=RES_OBJECTDYING)
            && (nWhich!=RES_ATTRSET_CHG)
            && (nWhich!=RES_FMT_CHG) )
        ASSERT(!this, "SwTxtCharFmt::Modify(): unbekanntes Modify!");
#endif

    if( pMyTxtNd )
    {
        SwUpdateAttr aUpdateAttr( *GetStart(), *GetEnd(), nWhich );
        pMyTxtNd->SwCntntNode::Modify( &aUpdateAttr, &aUpdateAttr );
    }
}

    // erfrage vom Modify Informationen
BOOL SwTxtCharFmt::GetInfo( SfxPoolItem& rInfo ) const
{
    if( RES_AUTOFMT_DOCNODE != rInfo.Which() || !pMyTxtNd ||
        &pMyTxtNd->GetNodes() != ((SwAutoFmtGetDocNode&)rInfo).pNodes )
        return TRUE;

    ((SwAutoFmtGetDocNode&)rInfo).pCntntNode = pMyTxtNd;
    return FALSE;
}

/*************************************************************************
 *                      class SwTxtINetFmt
 *************************************************************************/

SwTxtINetFmt::~SwTxtINetFmt( )
{
    delete pPrevFont;
    delete pPrevBackColor;
}


SwTxtINetFmt::SwTxtINetFmt( const SwFmtINetFmt& rAttr,
                            xub_StrLen nStart, xub_StrLen nEnd )
    : SwTxtAttrEnd( rAttr, nStart, nEnd ),
    SwClient( 0 ),
    pMyTxtNd( 0 ),
    pPrevFont( 0 ),
    pPrevBackColor( 0 )
{
    bValidVis = FALSE;
    ((SwFmtINetFmt&)rAttr).pTxtAttr  = this;
}

void SwTxtINetFmt::ChgFnt( SwFont *pFont )
{
    const SwCharFmt* pFmt = GetCharFmt();
    if( pFmt )
    {
        // Das 0 != bNew-Geraffel bringt M80 zum Schweigen
        if ( pPrevFont )
            *pPrevFont = pFont->GetFnt( SW_LATIN );
        else
            pPrevFont = new SvxFont( pFont->GetFnt( SW_LATIN ) );
        bPrevNoHyph = pFont->IsNoHyph();
        bPrevBlink = pFont->IsBlink();
        bPrevURL = pFont->IsURL();
        pFont->GetMagic( pFontNo, nFntIndex, SW_LATIN );
        pFont->SetDiffFnt( &pFmt->GetAttrSet() );
        pFont->SetURL( TRUE );
        delete pPrevBackColor;
        bColor = SFX_ITEM_SET ==
                 pFmt->GetAttrSet().GetItemState( RES_CHRATR_BACKGROUND, TRUE );
        if( bColor )
        {
            pPrevBackColor = new Color( pFmt->GetChrBackground().GetColor() );
            pPrevBackColor = pFont->XChgBackColor( pPrevBackColor );
        }
        else
            pPrevBackColor = NULL;
    }
}

SwCharFmt* SwTxtINetFmt::GetCharFmt()
{
    const SwFmtINetFmt& rFmt = SwTxtAttrEnd::GetINetFmt();
    SwCharFmt* pRet = NULL;

    if( rFmt.GetValue().Len() )
    {
        const SwDoc* pDoc = GetTxtNode().GetDoc();
        if( !IsValidVis() )
        {
            SetVisited( pDoc->IsVisitedURL( rFmt.GetValue() ) );
            SetValidVis( TRUE );
        }
        USHORT nId;
        const String& rStr = IsVisited() ? rFmt.GetVisitedFmt()
                                           : rFmt.GetINetFmt();
        if( rStr.Len() )
            nId = IsVisited() ? rFmt.GetVisitedFmtId() : rFmt.GetINetFmtId();
        else
            nId = IsVisited() ? RES_POOLCHR_INET_VISIT : RES_POOLCHR_INET_NORMAL;

        // JP 10.02.2000, Bug 72806: dont modify the doc for getting the
        //      correct charstyle.
        BOOL bResetMod = !pDoc->IsModified();
        Link aOle2Lnk;
        if( bResetMod )
        {
            aOle2Lnk = pDoc->GetOle2Link();
            ((SwDoc*)pDoc)->SetOle2Link( Link() );
        }

        pRet = IsPoolUserFmt( nId )
                ? ((SwDoc*)pDoc)->FindCharFmtByName( rStr )
                : ((SwDoc*)pDoc)->GetCharFmtFromPool( nId );

        if( bResetMod )
        {
            ((SwDoc*)pDoc)->ResetModified();
            ((SwDoc*)pDoc)->SetOle2Link( aOle2Lnk );
        }
    }

    if( pRet )
        pRet->Add( this );
    else if( GetRegisteredIn() )
        pRegisteredIn->Remove( this );

    return pRet;
}

void SwTxtINetFmt::RstFnt(SwFont *pFont)
{
    const SwFmtINetFmt& rFmt = SwTxtAttrEnd::GetINetFmt();
    if( pPrevFont && rFmt.GetValue().Len() )
    {
        // Das 0 != bNew-Geraffel bringt M80 zum Schweigen
        pFont->SetFnt( *pPrevFont, SW_LATIN );
        pFont->SetMagic( pFontNo, nFntIndex, SW_LATIN );
        pFont->SetBlink( bPrevBlink );
        pFont->SetNoHyph( bPrevNoHyph );
        pFont->SetURL( bPrevURL );
        if( bColor )
        {
            pFont->SetBackColor( pPrevBackColor );
            pPrevBackColor = NULL;
        }
        delete pPrevFont;
        pPrevFont = NULL;
    }
}

void SwTxtINetFmt::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
{
    USHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
#ifndef PRODUCT
    if ( (nWhich<RES_CHRATR_BEGIN || nWhich>RES_CHRATR_END)
            && (nWhich!=RES_OBJECTDYING)
            && (nWhich!=RES_ATTRSET_CHG)
            && (nWhich!=RES_FMT_CHG) )
        ASSERT(!this, "SwTxtCharFmt::Modify(): unbekanntes Modify!");
#endif

    if( pMyTxtNd )
    {
        SwUpdateAttr aUpdateAttr( *GetStart(), *GetEnd(), nWhich );
        pMyTxtNd->SwCntntNode::Modify( &aUpdateAttr, &aUpdateAttr );
    }
}

    // erfrage vom Modify Informationen
BOOL SwTxtINetFmt::GetInfo( SfxPoolItem& rInfo ) const
{
    if( RES_AUTOFMT_DOCNODE != rInfo.Which() || !pMyTxtNd ||
        &pMyTxtNd->GetNodes() != ((SwAutoFmtGetDocNode&)rInfo).pNodes )
        return TRUE;

    ((SwAutoFmtGetDocNode&)rInfo).pCntntNode = pMyTxtNd;
    return FALSE;
}

BOOL SwTxtINetFmt::IsProtect( ) const
{
    return pMyTxtNd && pMyTxtNd->IsProtect();
}

/*************************************************************************
 *                      class SwTxtEscapement
 *************************************************************************/

SwTxtEscapement::SwTxtEscapement( const SvxEscapementItem& rAttr,
                    USHORT nStart, USHORT nEnd )
    : SwTxtAttrEnd( rAttr, nStart, nEnd )
{}

void SwTxtEscapement::ChgFnt(SwFont *pFont)
{
    short nNewEsc   = GetEscapement().GetEsc();
    BYTE  nNewPropr = GetEscapement().GetProp();
    if( !nNewEsc )
        nNewPropr = 100;
    nPrevEsc   = pFont->GetEscapement();
    nPrevPropr = pFont->GetPropr();
    pFont->SetEscapement( nNewEsc );
    pFont->SetProportion( nNewPropr );
}

void SwTxtEscapement::RstFnt(SwFont *pFont)
{
    pFont->SetEscapement( nPrevEsc );
    pFont->SetProportion( nPrevPropr );
}

void SwTxtEscapement::ChgTxtAttr( SwTxtAttr &rAttr )
{
    nPrevEsc = ((SwTxtEscapement&)rAttr).nPrevEsc;
    nPrevPropr = ((SwTxtEscapement&)rAttr).nPrevPropr;
    short nNewEsc   = GetEscapement().GetEsc();
    BYTE  nNewPropr = GetEscapement().GetProp();
    if( !nNewEsc )
        nNewPropr = 100;
    ((SwTxtEscapement&)rAttr).nPrevEsc = nNewEsc;
    ((SwTxtEscapement&)rAttr).nPrevPropr = nNewPropr;
}

void SwTxtEscapement::RstTxtAttr( SwTxtAttr &rAttr )
{
    ((SwTxtEscapement&)rAttr).nPrevEsc = nPrevEsc;
    ((SwTxtEscapement&)rAttr).nPrevPropr = nPrevPropr;
}

/*************************************************************************
 *                      class SwTxtCaseMap
 *************************************************************************/

SwTxtCaseMap::SwTxtCaseMap( const SvxCaseMapItem& rAttr,
                    xub_StrLen nStart, xub_StrLen nEnd )
    : SwTxtAttrEnd( rAttr, nStart, nEnd )
{}

void SwTxtCaseMap::ChgFnt(SwFont *pFont)
{
    ePrevCaseMap = pFont->GetCaseMap();
    pFont->SetCaseMap( GetCaseMap().GetCaseMap() );
}

void SwTxtCaseMap::RstFnt(SwFont *pFont)
{
    pFont->SetCaseMap( ePrevCaseMap );
}

void SwTxtCaseMap::ChgTxtAttr( SwTxtAttr &rAttr )
{
    ePrevCaseMap = ((SwTxtCaseMap&)rAttr).ePrevCaseMap;
    ((SwTxtCaseMap&)rAttr).ePrevCaseMap = GetCaseMap().GetCaseMap();
}

void SwTxtCaseMap::RstTxtAttr( SwTxtAttr &rAttr )
{
    ((SwTxtCaseMap&)rAttr).ePrevCaseMap = ePrevCaseMap;
}

/*************************************************************************
 *                      class SwTxtBlink
 *************************************************************************/

SwTxtBlink::SwTxtBlink( const SvxBlinkItem& rAttr,
                    xub_StrLen nStart, xub_StrLen nEnd )
    : SwTxtAttrEnd( rAttr, nStart, nEnd )
{}

void SwTxtBlink::ChgFnt(SwFont *pFont)
{
    bPrev = pFont->IsBlink();
    pFont->SetBlink( GetBlink().GetValue() );
}

void SwTxtBlink::RstFnt(SwFont *pFont)
{
    pFont->SetBlink( bPrev );
}

void SwTxtBlink::ChgTxtAttr( SwTxtAttr &rAttr )
{
    bPrev = ((SwTxtBlink&)rAttr).bPrev;
    ((SwTxtBlink&)rAttr).bPrev = GetBlink().GetValue();
}

void SwTxtBlink::RstTxtAttr( SwTxtAttr &rAttr )
{
    ((SwTxtBlink&)rAttr).bPrev = bPrev;
}

/*************************************************************************
 *                      class SwTxtBackground
 *************************************************************************/

SwTxtBackground::SwTxtBackground( const SvxBrushItem& rAttr,
                    xub_StrLen nStart, xub_StrLen nEnd )
    : SwTxtAttrEnd( rAttr, nStart, nEnd ),
    pPrevColor( NULL )
{
}

SwTxtBackground::~SwTxtBackground( )
{
    delete pPrevColor;
}

void SwTxtBackground::ChgFnt(SwFont *pFont)
{
    delete pPrevColor;
    pPrevColor = new Color( GetChrBackground().GetColor() );
    pPrevColor = pFont->XChgBackColor( pPrevColor );
}

void SwTxtBackground::RstFnt(SwFont *pFont)
{
    pFont->SetBackColor( pPrevColor );
    pPrevColor = NULL;
}

void SwTxtBackground::ChgTxtAttr( SwTxtAttr &rAttr )
{
    delete pPrevColor;
    pPrevColor = ((SwTxtBackground&)rAttr).pPrevColor;
    ((SwTxtBackground&)rAttr).pPrevColor =
        new Color( GetChrBackground().GetColor() );
}

void SwTxtBackground::RstTxtAttr( SwTxtAttr &rAttr )
{
    delete ((SwTxtBackground&)rAttr).pPrevColor;
    ((SwTxtBackground&)rAttr).pPrevColor = pPrevColor;
    pPrevColor = NULL;
}

/*************************************************************************
 *                      class SwTxtNoHyphHere
 *************************************************************************/

SwTxtNoHyphenHere::SwTxtNoHyphenHere( const SvxNoHyphenItem& rAttr,
                    xub_StrLen nStart, xub_StrLen nEnd )
    : SwTxtAttrEnd( rAttr, nStart, nEnd )
{}

void SwTxtNoHyphenHere::ChgFnt(SwFont *pFont)
{
    bPrev = pFont->IsNoHyph();
    pFont->SetNoHyph( GetNoHyphenHere().GetValue() );
}

void SwTxtNoHyphenHere::RstFnt(SwFont *pFont)
{
    pFont->SetNoHyph( bPrev );
}

/*************************************************************************
 *                      class SwTxtKerning
 *************************************************************************/

SwTxtKerning::SwTxtKerning( const SvxKerningItem& rAttr,
                    xub_StrLen nStart, xub_StrLen nEnd )
    : SwTxtAttrEnd( rAttr, nStart, nEnd )
{}

void SwTxtKerning::ChgFnt( SwFont *pFont )
{
    const SvxKerningItem& rAttr = GetKerning();
    nPrevKern = pFont->GetFixKerning();
    pFont->SetFixKerning( rAttr.GetValue() );
}

void SwTxtKerning::RstFnt( SwFont *pFont )
{
    pFont->SetFixKerning( nPrevKern );
}

void SwTxtKerning::ChgTxtAttr( SwTxtAttr &rAttr )
{
    nPrevKern = ((SwTxtKerning&)rAttr).nPrevKern;
    ((SwTxtKerning&)rAttr).nPrevKern = GetKerning().GetValue();
}

void SwTxtKerning::RstTxtAttr( SwTxtAttr &rAttr )
{
    ((SwTxtKerning&)rAttr).nPrevKern = nPrevKern;
}

/*************************************************************************
 *                      class SwTxtLanguage
 *************************************************************************/

SwTxtLanguage::SwTxtLanguage( const SvxLanguageItem& rAttr,
                    xub_StrLen nStart, xub_StrLen nEnd )
    : SwTxtAttrEnd( rAttr, nStart, nEnd )
{}

void SwTxtLanguage::ChgFnt( SwFont *pFont )
{
    ePrevLang = pFont->GetLanguage( SW_LATIN );
    pFont->SetLanguage( GetLanguage().GetLanguage(), SW_LATIN );
}

void SwTxtLanguage::RstFnt( SwFont *pFont )
{
    pFont->SetLanguage( ePrevLang, SW_LATIN );
}


// ATT_XNLCONTAINERITEM ******************************

SwTxtXMLAttrContainer::SwTxtXMLAttrContainer(
                            const SvXMLAttrContainerItem& rAttr,
                            xub_StrLen nStart, xub_StrLen nEnd )
    : SwTxtAttrEnd( rAttr, nStart, nEnd )
{}




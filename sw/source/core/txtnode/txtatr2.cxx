/*************************************************************************
 *
 *  $RCSfile: txtatr2.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ama $ $Date: 2001-03-15 15:49:57 $
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
#ifndef _SVX_TWOLINESITEM_HXX
#include <svx/twolinesitem.hxx>
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
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
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
#ifndef _FMTRUBY_HXX
#include <fmtruby.hxx>
#endif
#ifndef _FMTHBSH_HXX //autogen
#include <fmthbsh.hxx>
#endif

TYPEINIT1(SwTxtINetFmt,SwClient);
TYPEINIT1(SwTxtRuby,SwClient);

/*************************************************************************
 *                      class SwTxtHardBlank
 *************************************************************************/

SwTxtHardBlank::SwTxtHardBlank( const SwFmtHardBlank& rAttr, xub_StrLen nStart )
    : SwTxtAttr( rAttr, nStart ),
    cChar( rAttr.GetChar() )
{
    ASSERT( ' ' != cChar && '-' != cChar,
            "Invalid character for the HardBlank attribute - "
            "must be a normal unicode character" );
}


/*************************************************************************
 *                      class SwTxtCharFmt
 *************************************************************************/

SwTxtCharFmt::SwTxtCharFmt( const SwFmtCharFmt& rAttr,
                    xub_StrLen nStart, xub_StrLen nEnd )
    : SwTxtAttrEnd( rAttr, nStart, nEnd ),
    pMyTxtNd( 0 )
{
    ((SwFmtCharFmt&)rAttr).pTxtAttr = this;
    SetCharFmtAttr( TRUE );
}

SwTxtCharFmt::~SwTxtCharFmt( )
{
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

SwTxtINetFmt::SwTxtINetFmt( const SwFmtINetFmt& rAttr,
                            xub_StrLen nStart, xub_StrLen nEnd )
    : SwTxtAttrEnd( rAttr, nStart, nEnd ),
    SwClient( 0 ),
    pMyTxtNd( 0 )
{
    bValidVis = FALSE;
    ((SwFmtINetFmt&)rAttr).pTxtAttr  = this;
    SetCharFmtAttr( TRUE );
}

SwTxtINetFmt::~SwTxtINetFmt( )
{
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

// ATT_XNLCONTAINERITEM ******************************

SwTxtXMLAttrContainer::SwTxtXMLAttrContainer(
                            const SvXMLAttrContainerItem& rAttr,
                            xub_StrLen nStart, xub_StrLen nEnd )
    : SwTxtAttrEnd( rAttr, nStart, nEnd )
{}



// ******************************

SwTxtRuby::SwTxtRuby( const SwFmtRuby& rAttr,
                        xub_StrLen nStart, xub_StrLen nEnd )
    : SwTxtAttrEnd( rAttr, nStart, nEnd ),
    SwClient( 0 ),
    pMyTxtNd( 0 )
{
    ((SwFmtRuby&)rAttr).pTxtAttr  = this;
    SetDontExpand( TRUE );              // never expand this attribut
    SetLockExpandFlag( TRUE );
    SetDontMergeAttr( TRUE );
}

SwTxtRuby::~SwTxtRuby()
{
}

void SwTxtRuby::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew )
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

BOOL SwTxtRuby::GetInfo( SfxPoolItem& rInfo ) const
{
    if( RES_AUTOFMT_DOCNODE != rInfo.Which() || !pMyTxtNd ||
        &pMyTxtNd->GetNodes() != ((SwAutoFmtGetDocNode&)rInfo).pNodes )
        return TRUE;

    ((SwAutoFmtGetDocNode&)rInfo).pCntntNode = pMyTxtNd;
    return FALSE;
}

SwCharFmt* SwTxtRuby::GetCharFmt()
{
    const SwFmtRuby& rFmt = SwTxtAttrEnd::GetRuby();
    SwCharFmt* pRet = 0;

    if( rFmt.GetText().Len() )
    {
        const SwDoc* pDoc = GetTxtNode().GetDoc();
        const String& rStr = rFmt.GetCharFmtName();
        USHORT nId = rStr.Len() ? rFmt.GetCharFmtId() : RES_POOLCHR_RUBYTEXT;

        // JP 10.02.2000, Bug 72806: dont modify the doc for getting the
        //              correct charstyle.
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

// ******************************

SwTxt2Lines::SwTxt2Lines( const SvxTwoLinesItem& rAttr,
                        xub_StrLen nStart, xub_StrLen nEnd )
    : SwTxtAttrEnd( rAttr, nStart, nEnd )
{
}


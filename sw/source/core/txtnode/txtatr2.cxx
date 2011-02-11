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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <hintids.hxx>
#include <hints.hxx>
#include <sfx2/objsh.hxx>
#include <editeng/xmlcnitm.hxx>
#include <editeng/twolinesitem.hxx>
#include <txtinet.hxx>
#include <txtatr.hxx>
#include <fchrfmt.hxx>
#include <fmtinfmt.hxx>
#include <charfmt.hxx>
#include <ndtxt.hxx>        // SwCharFmt, SwTxtNode
#include <poolfmt.hxx>      // RES_POOLCHR_INET_...
#include <doc.hxx>          // SwDoc
#include <fmtruby.hxx>
#include <fmtmeta.hxx>


TYPEINIT1(SwTxtINetFmt,SwClient);
TYPEINIT1(SwTxtRuby,SwClient);


/*************************************************************************
 *                      class SwTxtCharFmt
 *************************************************************************/

SwTxtCharFmt::SwTxtCharFmt( SwFmtCharFmt& rAttr,
                    xub_StrLen nStt, xub_StrLen nEnde )
    : SwTxtAttrEnd( rAttr, nStt, nEnde )
    , m_pTxtNode( 0 )
    , m_nSortNumber( 0 )
{
    rAttr.pTxtAttr = this;
    SetCharFmtAttr( sal_True );
}

SwTxtCharFmt::~SwTxtCharFmt( )
{
}

void SwTxtCharFmt::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
{
    sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    ASSERT(  isCHRATR(nWhich) || (RES_OBJECTDYING == nWhich)
             || (RES_ATTRSET_CHG == nWhich) || (RES_FMT_CHG == nWhich),
        "SwTxtCharFmt::Modify(): unknown Modify");

    if ( m_pTxtNode )
    {
        SwUpdateAttr aUpdateAttr( *GetStart(), *GetEnd(), nWhich );
        m_pTxtNode->Modify( &aUpdateAttr, &aUpdateAttr );
    }
}

    // erfrage vom Modify Informationen
sal_Bool SwTxtCharFmt::GetInfo( SfxPoolItem& rInfo ) const
{
    if ( RES_AUTOFMT_DOCNODE != rInfo.Which() || !m_pTxtNode ||
        &m_pTxtNode->GetNodes() != static_cast<SwAutoFmtGetDocNode&>(rInfo).pNodes )
    {
        return sal_True;
    }

    static_cast<SwAutoFmtGetDocNode&>(rInfo).pCntntNode = m_pTxtNode;
    return sal_False;
}


/*************************************************************************
 *                        class SwTxtAttrNesting
 *************************************************************************/

SwTxtAttrNesting::SwTxtAttrNesting( SfxPoolItem & i_rAttr,
            const xub_StrLen i_nStart, const xub_StrLen i_nEnd )
    : SwTxtAttrEnd( i_rAttr, i_nStart, i_nEnd )
{
    SetDontExpand( true );  // never expand this attribute
    // lock the expand flag: simple guarantee that nesting will not be
    // invalidated by expand operations
    SetLockExpandFlag( true );
    SetDontExpandStartAttr( true );
    SetNesting( true );
}

SwTxtAttrNesting::~SwTxtAttrNesting()
{
}


/*************************************************************************
 *                      class SwTxtINetFmt
 *************************************************************************/

SwTxtINetFmt::SwTxtINetFmt( SwFmtINetFmt& rAttr,
                            xub_StrLen nStart, xub_StrLen nEnd )
    : SwTxtAttrNesting( rAttr, nStart, nEnd )
    , SwClient( 0 )
    , m_pTxtNode( 0 )
    , m_bVisited( false )
    , m_bVisitedValid( false )
{
    rAttr.pTxtAttr  = this;
    SetCharFmtAttr( true );
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
        if( !IsVisitedValid() )
        {
            SetVisited( pDoc->IsVisitedURL( rFmt.GetValue() ) );
            SetVisitedValid( true );
        }
        sal_uInt16 nId;
        const String& rStr = IsVisited() ? rFmt.GetVisitedFmt()
                                           : rFmt.GetINetFmt();
        if( rStr.Len() )
            nId = IsVisited() ? rFmt.GetVisitedFmtId() : rFmt.GetINetFmtId();
        else
            nId = static_cast<sal_uInt16>(IsVisited() ? RES_POOLCHR_INET_VISIT : RES_POOLCHR_INET_NORMAL);

        // JP 10.02.2000, Bug 72806: dont modify the doc for getting the
        //      correct charstyle.
        sal_Bool bResetMod = !pDoc->IsModified();
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
    sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    ASSERT(  isCHRATR(nWhich) || (RES_OBJECTDYING == nWhich)
             || (RES_ATTRSET_CHG == nWhich) || (RES_FMT_CHG == nWhich),
        "SwTxtINetFmt::Modify(): unknown Modify");

    if ( m_pTxtNode )
    {
        SwUpdateAttr aUpdateAttr( *GetStart(), *GetEnd(), nWhich );
        m_pTxtNode->Modify( &aUpdateAttr, &aUpdateAttr );
    }
}

    // erfrage vom Modify Informationen
sal_Bool SwTxtINetFmt::GetInfo( SfxPoolItem& rInfo ) const
{
    if ( RES_AUTOFMT_DOCNODE != rInfo.Which() || !m_pTxtNode ||
        &m_pTxtNode->GetNodes() != static_cast<SwAutoFmtGetDocNode&>(rInfo).pNodes )
    {
        return sal_True;
    }

    static_cast<SwAutoFmtGetDocNode&>(rInfo).pCntntNode = m_pTxtNode;
    return sal_False;
}

sal_Bool SwTxtINetFmt::IsProtect( ) const
{
    return m_pTxtNode && m_pTxtNode->IsProtect();
}

/*************************************************************************
 *                      class SwTxtRuby
 *************************************************************************/

SwTxtRuby::SwTxtRuby( SwFmtRuby& rAttr,
                      xub_StrLen nStart, xub_StrLen nEnd )
    : SwTxtAttrNesting( rAttr, nStart, nEnd )
    , SwClient( 0 )
    , m_pTxtNode( 0 )
{
    rAttr.pTxtAttr  = this;
}

SwTxtRuby::~SwTxtRuby()
{
}

void SwTxtRuby::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew )
{
    sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    ASSERT(  isCHRATR(nWhich) || (RES_OBJECTDYING == nWhich)
             || (RES_ATTRSET_CHG == nWhich) || (RES_FMT_CHG == nWhich),
        "SwTxtRuby::Modify(): unknown Modify");

    if ( m_pTxtNode )
    {
        SwUpdateAttr aUpdateAttr( *GetStart(), *GetEnd(), nWhich );
        m_pTxtNode->Modify( &aUpdateAttr, &aUpdateAttr );
    }
}

sal_Bool SwTxtRuby::GetInfo( SfxPoolItem& rInfo ) const
{
    if( RES_AUTOFMT_DOCNODE != rInfo.Which() || !m_pTxtNode ||
        &m_pTxtNode->GetNodes() != static_cast<SwAutoFmtGetDocNode&>(rInfo).pNodes )
    {
        return sal_True;
    }

    static_cast<SwAutoFmtGetDocNode&>(rInfo).pCntntNode = m_pTxtNode;
    return sal_False;
}

SwCharFmt* SwTxtRuby::GetCharFmt()
{
    const SwFmtRuby& rFmt = SwTxtAttrEnd::GetRuby();
    SwCharFmt* pRet = 0;

    if( rFmt.GetText().Len() )
    {
        const SwDoc* pDoc = GetTxtNode().GetDoc();
        const String& rStr = rFmt.GetCharFmtName();
        sal_uInt16 nId = RES_POOLCHR_RUBYTEXT;
        if ( rStr.Len() )
            nId = rFmt.GetCharFmtId();

        // JP 10.02.2000, Bug 72806: dont modify the doc for getting the
        //              correct charstyle.
        sal_Bool bResetMod = !pDoc->IsModified();
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


/*************************************************************************
 *                        class SwTxtMeta
 *************************************************************************/

SwTxtMeta *
SwTxtMeta::CreateTxtMeta(
    ::sw::MetaFieldManager & i_rTargetDocManager,
    SwTxtNode *const i_pTargetTxtNode,
    SwFmtMeta & i_rAttr,
    xub_StrLen const i_nStart, xub_StrLen const i_nEnd, bool const i_bIsCopy)
{
    if (COPY == i_bIsCopy)
    {   // i_rAttr is already cloned, now call DoCopy to copy the sw::Meta
        OSL_ENSURE(i_pTargetTxtNode, "cannot copy Meta without target node");
        i_rAttr.DoCopy(i_rTargetDocManager, *i_pTargetTxtNode);
    }
    SwTxtMeta *const pTxtMeta(new SwTxtMeta(i_rAttr, i_nStart, i_nEnd));
    return pTxtMeta;
}

SwTxtMeta::SwTxtMeta( SwFmtMeta & i_rAttr,
        const xub_StrLen i_nStart, const xub_StrLen i_nEnd )
    : SwTxtAttrNesting( i_rAttr, i_nStart, i_nEnd )
{
    i_rAttr.SetTxtAttr( this );
    SetHasDummyChar(true);
}

SwTxtMeta::~SwTxtMeta()
{
    SwFmtMeta & rFmtMeta( static_cast<SwFmtMeta &>(GetAttr()) );
    if (rFmtMeta.GetTxtAttr() == this)
    {
        rFmtMeta.SetTxtAttr(0);
    }
}

void SwTxtMeta::ChgTxtNode(SwTxtNode * const pNode)
{
    SwFmtMeta & rFmtMeta( static_cast<SwFmtMeta &>(GetAttr()) );
    if (rFmtMeta.GetTxtAttr() == this)
    {
        rFmtMeta.NotifyChangeTxtNode(pNode);
    }
}


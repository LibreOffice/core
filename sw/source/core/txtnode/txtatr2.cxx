/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


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

void SwTxtCharFmt::ModifyNotification( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    OSL_ENSURE(  isCHRATR(nWhich) || (RES_OBJECTDYING == nWhich)
             || (RES_ATTRSET_CHG == nWhich) || (RES_FMT_CHG == nWhich),
        "SwTxtCharFmt::Modify(): unknown Modify");

    if ( m_pTxtNode )
    {
        SwUpdateAttr aUpdateAttr( *GetStart(), *GetEnd(), nWhich );
        m_pTxtNode->ModifyNotification( &aUpdateAttr, &aUpdateAttr );
    }
}

bool SwTxtCharFmt::GetInfo( SfxPoolItem& rInfo ) const
{
    if ( RES_AUTOFMT_DOCNODE != rInfo.Which() || !m_pTxtNode ||
        &m_pTxtNode->GetNodes() != static_cast<SwAutoFmtGetDocNode&>(rInfo).pNodes )
    {
        return true;
    }

    static_cast<SwAutoFmtGetDocNode&>(rInfo).pCntntNode = m_pTxtNode;
    return false;
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

    if( !rFmt.GetValue().isEmpty() )
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
        bool bResetMod = !pDoc->IsModified();
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
        GetRegisteredInNonConst()->Remove( this );

    return pRet;
}

void SwTxtINetFmt::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    OSL_ENSURE(  isCHRATR(nWhich) || (RES_OBJECTDYING == nWhich)
             || (RES_ATTRSET_CHG == nWhich) || (RES_FMT_CHG == nWhich),
        "SwTxtINetFmt::Modify(): unknown Modify");

    if ( m_pTxtNode )
    {
        SwUpdateAttr aUpdateAttr( *GetStart(), *GetEnd(), nWhich );
        m_pTxtNode->ModifyNotification( &aUpdateAttr, &aUpdateAttr );
    }
}

    // erfrage vom Modify Informationen
bool SwTxtINetFmt::GetInfo( SfxPoolItem& rInfo ) const
{
    if ( RES_AUTOFMT_DOCNODE != rInfo.Which() || !m_pTxtNode ||
        &m_pTxtNode->GetNodes() != static_cast<SwAutoFmtGetDocNode&>(rInfo).pNodes )
    {
        return true;
    }

    static_cast<SwAutoFmtGetDocNode&>(rInfo).pCntntNode = m_pTxtNode;
    return false;
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

void SwTxtRuby::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    OSL_ENSURE(  isCHRATR(nWhich) || (RES_OBJECTDYING == nWhich)
             || (RES_ATTRSET_CHG == nWhich) || (RES_FMT_CHG == nWhich),
        "SwTxtRuby::Modify(): unknown Modify");

    if ( m_pTxtNode )
    {
        SwUpdateAttr aUpdateAttr( *GetStart(), *GetEnd(), nWhich );
        m_pTxtNode->ModifyNotification( &aUpdateAttr, &aUpdateAttr );
    }
}

bool SwTxtRuby::GetInfo( SfxPoolItem& rInfo ) const
{
    if( RES_AUTOFMT_DOCNODE != rInfo.Which() || !m_pTxtNode ||
        &m_pTxtNode->GetNodes() != static_cast<SwAutoFmtGetDocNode&>(rInfo).pNodes )
    {
        return true;
    }

    static_cast<SwAutoFmtGetDocNode&>(rInfo).pCntntNode = m_pTxtNode;
    return false;
}

SwCharFmt* SwTxtRuby::GetCharFmt()
{
    const SwFmtRuby& rFmt = SwTxtAttrEnd::GetRuby();
    SwCharFmt* pRet = 0;

    if( !rFmt.GetText().isEmpty() )
    {
        const SwDoc* pDoc = GetTxtNode().GetDoc();
        const String& rStr = rFmt.GetCharFmtName();
        sal_uInt16 nId = RES_POOLCHR_RUBYTEXT;
        if ( rStr.Len() )
            nId = rFmt.GetCharFmtId();

        // JP 10.02.2000, Bug 72806: dont modify the doc for getting the
        //              correct charstyle.
        bool bResetMod = !pDoc->IsModified();
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
        GetRegisteredInNonConst()->Remove( this );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

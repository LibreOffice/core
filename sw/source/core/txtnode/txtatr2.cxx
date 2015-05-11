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
#include <ndtxt.hxx>
#include <poolfmt.hxx>
#include <doc.hxx>
#include <fmtruby.hxx>
#include <fmtmeta.hxx>
#include <IDocumentState.hxx>
#include <IDocumentStylePoolAccess.hxx>

TYPEINIT1(SwTxtINetFmt,SwClient);
TYPEINIT1(SwTxtRuby,SwClient);

SwTxtCharFmt::SwTxtCharFmt( SwFmtCharFmt& rAttr,
                    sal_Int32 nStt, sal_Int32 nEnde )
    : SwTxtAttr( rAttr, nStt )
    , SwTxtAttrEnd( rAttr, nStt, nEnde )
    , m_pTxtNode( 0 )
    , m_nSortNumber( 0 )
{
    rAttr.pTxtAttr = this;
    SetCharFmtAttr( true );
}

SwTxtCharFmt::~SwTxtCharFmt( )
{
}

void SwTxtCharFmt::ModifyNotification( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    OSL_ENSURE(  isCHRATR(nWhich) || (RES_OBJECTDYING == nWhich)
             || (RES_ATTRSET_CHG == nWhich) || (RES_FMT_CHG == nWhich),
        "SwTxtCharFmt::Modify(): unknown Modify");

    if ( m_pTxtNode )
    {
        SwUpdateAttr aUpdateAttr(
            GetStart(),
            *GetEnd(),
            nWhich);

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

SwTxtAttrNesting::SwTxtAttrNesting( SfxPoolItem & i_rAttr,
            const sal_Int32 i_nStart, const sal_Int32 i_nEnd )
    : SwTxtAttr( i_rAttr, i_nStart )
    , SwTxtAttrEnd( i_rAttr, i_nStart, i_nEnd )
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

SwTxtINetFmt::SwTxtINetFmt( SwFmtINetFmt& rAttr,
                            sal_Int32 nStart, sal_Int32 nEnd )
    : SwTxtAttr( rAttr, nStart )
    , SwTxtAttrNesting( rAttr, nStart, nEnd )
    , SwClient( 0 )
    , m_pTxtNode( 0 )
    , m_bVisited( false )
    , m_bVisitedValid( false )
{
    rAttr.mpTxtAttr  = this;
    SetCharFmtAttr( true );
}

SwTxtINetFmt::~SwTxtINetFmt( )
{
}

SwCharFmt* SwTxtINetFmt::GetCharFmt()
{
    const SwFmtINetFmt& rFmt = SwTxtAttrEnd::GetINetFmt();
    SwCharFmt* pRet = NULL;

    if (!rFmt.GetValue().isEmpty())
    {
        SwDoc* pDoc = GetTxtNode().GetDoc();
        if( !IsVisitedValid() )
        {
            SetVisited( pDoc->IsVisitedURL( rFmt.GetValue() ) );
            SetVisitedValid( true );
        }

        const sal_uInt16 nId = IsVisited() ? rFmt.GetVisitedFmtId() : rFmt.GetINetFmtId();
        const OUString& rStr = IsVisited() ? rFmt.GetVisitedFmt() : rFmt.GetINetFmt();
        if (rStr.isEmpty())
        {
            OSL_ENSURE( false, "<SwTxtINetFmt::GetCharFmt()> - missing character format at hyperlink attribute");
        }

        // JP 10.02.2000, Bug 72806: dont modify the doc for getting the
        //      correct charstyle.
        bool bResetMod = !pDoc->getIDocumentState().IsModified();
        Link<> aOle2Lnk;
        if ( bResetMod )
        {
            aOle2Lnk = pDoc->GetOle2Link();
            pDoc->SetOle2Link( Link<>() );
        }

        pRet = IsPoolUserFmt( nId )
               ? pDoc->FindCharFmtByName( rStr )
               : pDoc->getIDocumentStylePoolAccess().GetCharFmtFromPool( nId );

        if ( bResetMod )
        {
            pDoc->getIDocumentState().ResetModified();
            pDoc->SetOle2Link( aOle2Lnk );
        }
    }

    if ( pRet )
        pRet->Add( this );
    else if ( GetRegisteredIn() )
        GetRegisteredInNonConst()->Remove( this );

    return pRet;
}

void SwTxtINetFmt::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    OSL_ENSURE(  isCHRATR(nWhich) || (RES_OBJECTDYING == nWhich)
             || (RES_ATTRSET_CHG == nWhich) || (RES_FMT_CHG == nWhich),
        "SwTxtINetFmt::Modify(): unknown Modify");

    if ( m_pTxtNode )
    {
        SwUpdateAttr aUpdateAttr(
            GetStart(),
            *GetEnd(),
            nWhich);

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

bool SwTxtINetFmt::IsProtect( ) const
{
    return m_pTxtNode && m_pTxtNode->IsProtect();
}

SwTxtRuby::SwTxtRuby( SwFmtRuby& rAttr,
                      sal_Int32 nStart, sal_Int32 nEnd )
    : SwTxtAttr( rAttr, nStart )
    , SwTxtAttrNesting( rAttr, nStart, nEnd )
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
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    OSL_ENSURE(  isCHRATR(nWhich) || (RES_OBJECTDYING == nWhich)
             || (RES_ATTRSET_CHG == nWhich) || (RES_FMT_CHG == nWhich),
        "SwTxtRuby::Modify(): unknown Modify");

    if ( m_pTxtNode )
    {
        SwUpdateAttr aUpdateAttr(
            GetStart(),
            *GetEnd(),
            nWhich);

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
        const OUString rStr = rFmt.GetCharFmtName();
        const sal_uInt16 nId = rStr.isEmpty()
                             ? static_cast<sal_uInt16>(RES_POOLCHR_RUBYTEXT)
                             : rFmt.GetCharFmtId();

        // JP 10.02.2000, Bug 72806: dont modify the doc for getting the
        //              correct charstyle.
        const bool bResetMod = !pDoc->getIDocumentState().IsModified();
        Link<> aOle2Lnk;
        if( bResetMod )
        {
            aOle2Lnk = pDoc->GetOle2Link();
            const_cast<SwDoc*>(pDoc)->SetOle2Link( Link<>() );
        }

        pRet = IsPoolUserFmt( nId )
                ? pDoc->FindCharFmtByName( rStr )
                : const_cast<SwDoc*>(pDoc)->getIDocumentStylePoolAccess().GetCharFmtFromPool( nId );

        if( bResetMod )
        {
            const_cast<SwDoc*>(pDoc)->getIDocumentState().ResetModified();
            const_cast<SwDoc*>(pDoc)->SetOle2Link( aOle2Lnk );
        }
    }

    if( pRet )
        pRet->Add( this );
    else if( GetRegisteredIn() )
        GetRegisteredInNonConst()->Remove( this );

    return pRet;
}

SwTxtMeta *
SwTxtMeta::CreateTxtMeta(
    ::sw::MetaFieldManager & i_rTargetDocManager,
    SwTxtNode *const i_pTargetTxtNode,
    SwFmtMeta & i_rAttr,
    sal_Int32 const i_nStart,
    sal_Int32 const i_nEnd,
    bool const i_bIsCopy)
{
    if (i_bIsCopy)
    {   // i_rAttr is already cloned, now call DoCopy to copy the sw::Meta
        OSL_ENSURE(i_pTargetTxtNode, "cannot copy Meta without target node");
        i_rAttr.DoCopy(i_rTargetDocManager, *i_pTargetTxtNode);
    }
    SwTxtMeta *const pTxtMeta(new SwTxtMeta(i_rAttr, i_nStart, i_nEnd));
    return pTxtMeta;
}

SwTxtMeta::SwTxtMeta( SwFmtMeta & i_rAttr,
        const sal_Int32 i_nStart, const sal_Int32 i_nEnd )
    : SwTxtAttr( i_rAttr, i_nStart )
    , SwTxtAttrNesting( i_rAttr, i_nStart, i_nEnd )
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

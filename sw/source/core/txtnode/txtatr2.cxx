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

TYPEINIT1(SwTextINetFormat,SwClient);
TYPEINIT1(SwTextRuby,SwClient);


SwTextCharFormat::SwTextCharFormat( SwFormatCharFormat& rAttr,
                    sal_Int32 nStt, sal_Int32 nEnde )
    : SwTextAttr( rAttr, nStt )
    , SwTextAttrEnd( rAttr, nStt, nEnde )
    , m_pTextNode( 0 )
    , m_nSortNumber( 0 )
{
    rAttr.pTextAttr = this;
    SetCharFormatAttr( true );
}

SwTextCharFormat::~SwTextCharFormat( )
{
}

void SwTextCharFormat::ModifyNotification( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    OSL_ENSURE(  isCHRATR(nWhich) || (RES_OBJECTDYING == nWhich)
             || (RES_ATTRSET_CHG == nWhich) || (RES_FMT_CHG == nWhich),
        "SwTextCharFormat::Modify(): unknown Modify");

    if ( m_pTextNode )
    {
        SwUpdateAttr aUpdateAttr(
            GetStart(),
            *GetEnd(),
            nWhich);

        m_pTextNode->ModifyNotification( &aUpdateAttr, &aUpdateAttr );
    }
}

bool SwTextCharFormat::GetInfo( SfxPoolItem& rInfo ) const
{
    if ( RES_AUTOFMT_DOCNODE != rInfo.Which() || !m_pTextNode ||
        &m_pTextNode->GetNodes() != static_cast<SwAutoFormatGetDocNode&>(rInfo).pNodes )
    {
        return true;
    }

    static_cast<SwAutoFormatGetDocNode&>(rInfo).pContentNode = m_pTextNode;
    return false;
}

SwTextAttrNesting::SwTextAttrNesting( SfxPoolItem & i_rAttr,
            const sal_Int32 i_nStart, const sal_Int32 i_nEnd )
    : SwTextAttr( i_rAttr, i_nStart )
    , SwTextAttrEnd( i_rAttr, i_nStart, i_nEnd )
{
    SetDontExpand( true );  // never expand this attribute
    // lock the expand flag: simple guarantee that nesting will not be
    // invalidated by expand operations
    SetLockExpandFlag( true );
    SetDontExpandStartAttr( true );
    SetNesting( true );
}

SwTextAttrNesting::~SwTextAttrNesting()
{
}

SwTextINetFormat::SwTextINetFormat( SwFormatINetFormat& rAttr,
                            sal_Int32 nStart, sal_Int32 nEnd )
    : SwTextAttr( rAttr, nStart )
    , SwTextAttrNesting( rAttr, nStart, nEnd )
    , SwClient( 0 )
    , m_pTextNode( 0 )
    , m_bVisited( false )
    , m_bVisitedValid( false )
{
    rAttr.mpTextAttr  = this;
    SetCharFormatAttr( true );
}

SwTextINetFormat::~SwTextINetFormat( )
{
}

SwCharFormat* SwTextINetFormat::GetCharFormat()
{
    const SwFormatINetFormat& rFormat = SwTextAttrEnd::GetINetFormat();
    SwCharFormat* pRet = NULL;

    if (!rFormat.GetValue().isEmpty())
    {
        SwDoc* pDoc = GetTextNode().GetDoc();
        if( !IsVisitedValid() )
        {
            SetVisited( pDoc->IsVisitedURL( rFormat.GetValue() ) );
            SetVisitedValid( true );
        }

        const sal_uInt16 nId = IsVisited() ? rFormat.GetVisitedFormatId() : rFormat.GetINetFormatId();
        const OUString& rStr = IsVisited() ? rFormat.GetVisitedFormat() : rFormat.GetINetFormat();
        if (rStr.isEmpty())
        {
            OSL_ENSURE( false, "<SwTextINetFormat::GetCharFormat()> - missing character format at hyperlink attribute");
        }

        // JP 10.02.2000, Bug 72806: don't modify the doc for getting the
        //      correct charstyle.
        bool bResetMod = !pDoc->getIDocumentState().IsModified();
        Link<bool,void> aOle2Lnk;
        if ( bResetMod )
        {
            aOle2Lnk = pDoc->GetOle2Link();
            pDoc->SetOle2Link( Link<bool,void>() );
        }

        pRet = IsPoolUserFormat( nId )
               ? pDoc->FindCharFormatByName( rStr )
               : pDoc->getIDocumentStylePoolAccess().GetCharFormatFromPool( nId );

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

void SwTextINetFormat::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    OSL_ENSURE(  isCHRATR(nWhich) || (RES_OBJECTDYING == nWhich)
             || (RES_ATTRSET_CHG == nWhich) || (RES_FMT_CHG == nWhich),
        "SwTextINetFormat::Modify(): unknown Modify");

    if ( m_pTextNode )
    {
        SwUpdateAttr aUpdateAttr(
            GetStart(),
            *GetEnd(),
            nWhich);

        m_pTextNode->ModifyNotification( &aUpdateAttr, &aUpdateAttr );
    }
}

    // erfrage vom Modify Informationen
bool SwTextINetFormat::GetInfo( SfxPoolItem& rInfo ) const
{
    if ( RES_AUTOFMT_DOCNODE != rInfo.Which() || !m_pTextNode ||
        &m_pTextNode->GetNodes() != static_cast<SwAutoFormatGetDocNode&>(rInfo).pNodes )
    {
        return true;
    }

    static_cast<SwAutoFormatGetDocNode&>(rInfo).pContentNode = m_pTextNode;
    return false;
}

bool SwTextINetFormat::IsProtect( ) const
{
    return m_pTextNode && m_pTextNode->IsProtect();
}

SwTextRuby::SwTextRuby( SwFormatRuby& rAttr,
                      sal_Int32 nStart, sal_Int32 nEnd )
    : SwTextAttr( rAttr, nStart )
    , SwTextAttrNesting( rAttr, nStart, nEnd )
    , SwClient( 0 )
    , m_pTextNode( 0 )
{
    rAttr.pTextAttr  = this;
}

SwTextRuby::~SwTextRuby()
{
}

void SwTextRuby::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    OSL_ENSURE(  isCHRATR(nWhich) || (RES_OBJECTDYING == nWhich)
             || (RES_ATTRSET_CHG == nWhich) || (RES_FMT_CHG == nWhich),
        "SwTextRuby::Modify(): unknown Modify");

    if ( m_pTextNode )
    {
        SwUpdateAttr aUpdateAttr(
            GetStart(),
            *GetEnd(),
            nWhich);

        m_pTextNode->ModifyNotification( &aUpdateAttr, &aUpdateAttr );
    }
}

bool SwTextRuby::GetInfo( SfxPoolItem& rInfo ) const
{
    if( RES_AUTOFMT_DOCNODE != rInfo.Which() || !m_pTextNode ||
        &m_pTextNode->GetNodes() != static_cast<SwAutoFormatGetDocNode&>(rInfo).pNodes )
    {
        return true;
    }

    static_cast<SwAutoFormatGetDocNode&>(rInfo).pContentNode = m_pTextNode;
    return false;
}

SwCharFormat* SwTextRuby::GetCharFormat()
{
    const SwFormatRuby& rFormat = SwTextAttrEnd::GetRuby();
    SwCharFormat* pRet = 0;

    if( !rFormat.GetText().isEmpty() )
    {
        const SwDoc* pDoc = GetTextNode().GetDoc();
        const OUString rStr = rFormat.GetCharFormatName();
        const sal_uInt16 nId = rStr.isEmpty()
                             ? static_cast<sal_uInt16>(RES_POOLCHR_RUBYTEXT)
                             : rFormat.GetCharFormatId();

        // JP 10.02.2000, Bug 72806: don't modify the doc for getting the
        //              correct charstyle.
        const bool bResetMod = !pDoc->getIDocumentState().IsModified();
        Link<bool,void> aOle2Lnk;
        if( bResetMod )
        {
            aOle2Lnk = pDoc->GetOle2Link();
            const_cast<SwDoc*>(pDoc)->SetOle2Link( Link<bool,void>() );
        }

        pRet = IsPoolUserFormat( nId )
                ? pDoc->FindCharFormatByName( rStr )
                : const_cast<SwDoc*>(pDoc)->getIDocumentStylePoolAccess().GetCharFormatFromPool( nId );

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

SwTextMeta *
SwTextMeta::CreateTextMeta(
    ::sw::MetaFieldManager & i_rTargetDocManager,
    SwTextNode *const i_pTargetTextNode,
    SwFormatMeta & i_rAttr,
    sal_Int32 const i_nStart,
    sal_Int32 const i_nEnd,
    bool const i_bIsCopy)
{
    if (i_bIsCopy)
    {   // i_rAttr is already cloned, now call DoCopy to copy the sw::Meta
        OSL_ENSURE(i_pTargetTextNode, "cannot copy Meta without target node");
        i_rAttr.DoCopy(i_rTargetDocManager, *i_pTargetTextNode);
    }
    SwTextMeta *const pTextMeta(new SwTextMeta(i_rAttr, i_nStart, i_nEnd));
    return pTextMeta;
}

SwTextMeta::SwTextMeta( SwFormatMeta & i_rAttr,
        const sal_Int32 i_nStart, const sal_Int32 i_nEnd )
    : SwTextAttr( i_rAttr, i_nStart )
    , SwTextAttrNesting( i_rAttr, i_nStart, i_nEnd )
{
    i_rAttr.SetTextAttr( this );
    SetHasDummyChar(true);
}

SwTextMeta::~SwTextMeta()
{
    SwFormatMeta & rFormatMeta( static_cast<SwFormatMeta &>(GetAttr()) );
    if (rFormatMeta.GetTextAttr() == this)
    {
        rFormatMeta.SetTextAttr(0);
    }
}

void SwTextMeta::ChgTextNode(SwTextNode * const pNode)
{
    SwFormatMeta & rFormatMeta( static_cast<SwFormatMeta &>(GetAttr()) );
    if (rFormatMeta.GetTextAttr() == this)
    {
        rFormatMeta.NotifyChangeTextNode(pNode);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

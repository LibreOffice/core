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
#include <osl/diagnose.h>
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


namespace {

bool lcl_CheckAutoFormatHint(const SfxHint& rHint)
{
    if (rHint.GetId() != SfxHintId::SwAutoFormatUsedHint)
        return false;
    auto& rAutoFormatUsed = static_cast<const sw::AutoFormatUsedHint&>(rHint);
    rAutoFormatUsed.SetUsed();
    return true;
}
bool lcl_CheckAutoFormatHint(const SfxHint& rHint, const SwTextNode* pTextNode)
{
    if (rHint.GetId() != SfxHintId::SwAutoFormatUsedHint)
        return false;
    auto& rAutoFormatUsed = static_cast<const sw::AutoFormatUsedHint&>(rHint);
    rAutoFormatUsed.CheckNode(pTextNode);
    return true;
}
}

SwTextCharFormat::SwTextCharFormat(
    const SfxPoolItemHolder& rAttr,
    sal_Int32 nStt,
    sal_Int32 nEnd )
    : SwTextAttr( rAttr, nStt )
    , SwTextAttrEnd( rAttr, nStt, nEnd )
    , m_pTextNode( nullptr )
    , m_nSortNumber( 0 )
{
    SwFormatCharFormat& rSwFormatCharFormat(static_cast<SwFormatCharFormat&>(GetAttr()));
    rSwFormatCharFormat.m_pTextAttribute = this;
    SetCharFormatAttr( true );
}

SwTextCharFormat::~SwTextCharFormat( )
{
}

void SwTextCharFormat::TriggerNodeUpdate(const sw::LegacyModifyHint& rHint)
{
    const auto nWhich = rHint.GetWhich();
    SAL_WARN_IF(
            !isCHRATR(nWhich) &&
            RES_OBJECTDYING != nWhich &&
            RES_ATTRSET_CHG != nWhich &&
            RES_FMT_CHG != nWhich, "sw.core", "SwTextCharFormat::TriggerNodeUpdate: unknown hint type");

    if(m_pTextNode)
    {
        SwUpdateAttr aUpdateAttr(
            GetStart(),
            *GetEnd(),
            nWhich);
        m_pTextNode->TriggerNodeUpdate(sw::LegacyModifyHint(&aUpdateAttr, &aUpdateAttr));
    }
}

void SwTextCharFormat::HandleAutoFormatUsedHint(const sw::AutoFormatUsedHint& rHint)
{
    rHint.CheckNode(m_pTextNode);
}

SwTextAttrNesting::SwTextAttrNesting(
    const SfxPoolItemHolder& i_rAttr,
    const sal_Int32 i_nStart,
    const sal_Int32 i_nEnd )
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

SwTextINetFormat::SwTextINetFormat(
    const SfxPoolItemHolder& rAttr,
    sal_Int32 nStart,
    sal_Int32 nEnd )
    : SwTextAttr( rAttr, nStart )
    , SwTextAttrNesting( rAttr, nStart, nEnd )
    , SwClient( nullptr )
    , m_pTextNode( nullptr )
    , m_bVisited( false )
    , m_bVisitedValid( false )
{
    SwFormatINetFormat& rSwFormatINetFormat(static_cast<SwFormatINetFormat&>(GetAttr()));
    rSwFormatINetFormat.mpTextAttr  = this;
    SetCharFormatAttr( true );
}

SwTextINetFormat::~SwTextINetFormat( )
{
}

SwCharFormat* SwTextINetFormat::GetCharFormat()
{
    const SwFormatINetFormat& rFormat = SwTextAttrEnd::GetINetFormat();
    SwCharFormat* pRet = nullptr;

    if (!rFormat.GetValue().isEmpty())
    {
        SwDoc& rDoc = GetTextNode().GetDoc();
        if( !IsVisitedValid() )
        {
            SetVisited( rDoc.IsVisitedURL( rFormat.GetValue() ) );
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
        bool bModifiedEnabled = rDoc.getIDocumentState().IsEnableSetModified();
        rDoc.getIDocumentState().SetEnableSetModified(false);

        pRet = IsPoolUserFormat( nId )
               ? rDoc.FindCharFormatByName( rStr )
               : rDoc.getIDocumentStylePoolAccess().GetCharFormatFromPool( nId );

        rDoc.getIDocumentState().SetEnableSetModified(bModifiedEnabled);
    }

    if ( pRet )
        pRet->Add(*this);
    else
        EndListeningAll();

    return pRet;
}

void SwTextINetFormat::SwClientNotify(const SwModify&, const SfxHint& rHint)
{
    if(lcl_CheckAutoFormatHint(rHint))
        return;

    if (rHint.GetId() != SfxHintId::SwLegacyModify)
        return;
    auto pLegacy = static_cast<const sw::LegacyModifyHint*>(&rHint);
    const auto nWhich = pLegacy->GetWhich();
    OSL_ENSURE(isCHRATR(nWhich) || (RES_OBJECTDYING == nWhich)
            || (RES_ATTRSET_CHG == nWhich) || (RES_FMT_CHG == nWhich),
            "SwTextINetFormat::SwClientNotify: unknown hint.");
    if(!m_pTextNode)
        return;

    const SwUpdateAttr aUpdateAttr(GetStart(), *GetEnd(), nWhich);
    m_pTextNode->TriggerNodeUpdate(sw::LegacyModifyHint(&aUpdateAttr, &aUpdateAttr));
}

bool SwTextINetFormat::IsProtect( ) const
{
    return m_pTextNode && m_pTextNode->IsProtect();
}

SwTextRuby::SwTextRuby(
    const SfxPoolItemHolder& rAttr,
    sal_Int32 nStart,
    sal_Int32 nEnd )
    : SwTextAttr( rAttr, nStart )
    , SwTextAttrNesting( rAttr, nStart, nEnd )
    , SwClient( nullptr )
    , m_pTextNode( nullptr )
{
    SwFormatRuby& rSwFormatRuby(static_cast<SwFormatRuby&>(GetAttr()));
    rSwFormatRuby.m_pTextAttr  = this;
}

SwTextRuby::~SwTextRuby()
{
}

void SwTextRuby::SwClientNotify(const SwModify&, const SfxHint& rHint)
{
    if(lcl_CheckAutoFormatHint(rHint, m_pTextNode))
        return;
    if (rHint.GetId() != SfxHintId::SwLegacyModify)
        return;
    auto pLegacy = static_cast<const sw::LegacyModifyHint*>(&rHint);
    const auto nWhich = pLegacy->GetWhich();
    SAL_WARN_IF( !isCHRATR(nWhich)
            && (RES_OBJECTDYING == nWhich)
            && (RES_ATTRSET_CHG == nWhich)
            && (RES_FMT_CHG == nWhich), "sw.core", "SwTextRuby::SwClientNotify(): unknown legacy hint");
    if(!m_pTextNode)
        return;
    SwUpdateAttr aUpdateAttr(GetStart(), *GetEnd(), nWhich);
    m_pTextNode->TriggerNodeUpdate(sw::LegacyModifyHint(&aUpdateAttr, &aUpdateAttr));
}

SwCharFormat* SwTextRuby::GetCharFormat()
{
    const SwFormatRuby& rFormat = SwTextAttrEnd::GetRuby();
    SwCharFormat* pRet = nullptr;

    if( !rFormat.GetText().isEmpty() )
    {
        const SwDoc& rDoc = GetTextNode().GetDoc();
        const OUString& rStr = rFormat.GetCharFormatName();
        const sal_uInt16 nId = rStr.isEmpty()
                             ? o3tl::narrowing<sal_uInt16>(RES_POOLCHR_RUBYTEXT)
                             : rFormat.GetCharFormatId();

        // JP 10.02.2000, Bug 72806: don't modify the doc for getting the
        //              correct charstyle.
        const bool bResetMod = !rDoc.getIDocumentState().IsModified();
        Link<bool,void> aOle2Lnk;
        if( bResetMod )
        {
            aOle2Lnk = rDoc.GetOle2Link();
            const_cast<SwDoc&>(rDoc).SetOle2Link( Link<bool,void>() );
        }

        pRet = IsPoolUserFormat( nId )
                ? rDoc.FindCharFormatByName( rStr )
                : const_cast<SwDoc&>(rDoc).getIDocumentStylePoolAccess().GetCharFormatFromPool( nId );

        if( bResetMod )
        {
            const_cast<SwDoc&>(rDoc).getIDocumentState().ResetModified();
            const_cast<SwDoc&>(rDoc).SetOle2Link( aOle2Lnk );
        }
    }

    if( pRet )
        pRet->Add(*this);
    else
        EndListeningAll();

    return pRet;
}

SwTextMeta *
SwTextMeta::CreateTextMeta(
    ::sw::MetaFieldManager & i_rTargetDocManager,
    SwTextNode *const i_pTargetTextNode,
    const SfxPoolItemHolder& i_rAttr,
    sal_Int32 const i_nStart,
    sal_Int32 const i_nEnd,
    bool const i_bIsCopy)
{
    if (i_bIsCopy)
    {   // i_rAttr is already cloned, now call DoCopy to copy the sw::Meta
        assert(i_pTargetTextNode && "cannot copy Meta without target node");
        SwFormatMeta* pSwFormatMeta(static_cast<SwFormatMeta*>(const_cast<SfxPoolItem*>(i_rAttr.getItem())));
        pSwFormatMeta->DoCopy(i_rTargetDocManager, *i_pTargetTextNode);
    }
    SwTextMeta *const pTextMeta(new SwTextMeta(i_rAttr, i_nStart, i_nEnd));
    return pTextMeta;
}

SwTextMeta::SwTextMeta(
    const SfxPoolItemHolder& i_rAttr,
    const sal_Int32 i_nStart,
    const sal_Int32 i_nEnd )
    : SwTextAttr( i_rAttr, i_nStart )
    , SwTextAttrNesting( i_rAttr, i_nStart, i_nEnd )
{
    SwFormatMeta& rSwFormatMeta(static_cast<SwFormatMeta&>(GetAttr()));
    rSwFormatMeta.SetTextAttr( this );
    SetHasDummyChar(true);
}

SwTextMeta::~SwTextMeta()
{
    SwFormatMeta & rFormatMeta( static_cast<SwFormatMeta &>(GetAttr()) );
    if (rFormatMeta.GetTextAttr() == this)
    {
        rFormatMeta.SetTextAttr(nullptr);
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

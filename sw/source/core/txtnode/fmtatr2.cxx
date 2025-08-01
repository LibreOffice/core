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

#include <libxml/xmlwriter.h>
#include <hintids.hxx>
#include <poolfmt.hxx>
#include <unomid.h>
#include <editeng/memberids.h>

#include <o3tl/any.hxx>
#include <svl/macitem.hxx>
#include <svl/stylepool.hxx>
#include <fmtautofmt.hxx>
#include <fchrfmt.hxx>
#include <fmtinfmt.hxx>
#include <txtatr.hxx>
#include <fmtruby.hxx>
#include <charfmt.hxx>
#include <unoevent.hxx>
#include <unoport.hxx>
#include <com/sun/star/text/RubyAdjust.hpp>
#include <com/sun/star/text/RubyPosition.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <com/sun/star/uno/Any.h>
#include <SwStyleNameMapper.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <fmtmeta.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <unometa.hxx>
#include <unotext.hxx>
#include <docsh.hxx>
#include <names.hxx>
#include <osl/diagnose.h>

#include <algorithm>
#include <utility>

using namespace ::com::sun::star;


SfxPoolItem* SwFormatINetFormat::CreateDefault() { return new SwFormatINetFormat; }

SwFormatCharFormat::SwFormatCharFormat(SwCharFormat* pFormat)
    : SfxPoolItem(RES_TXTATR_CHARFMT)
    , m_pTextAttribute(nullptr)
    , m_pCharFormat(pFormat)
{
    if(m_pCharFormat)
        StartListening(m_pCharFormat->GetNotifier());
    setNonShareable();
}

SwFormatCharFormat::SwFormatCharFormat( const SwFormatCharFormat& rAttr )
    : SfxPoolItem(RES_TXTATR_CHARFMT)
    , m_pTextAttribute(nullptr)
    , m_pCharFormat(rAttr.m_pCharFormat)
{
    setNonShareable();
    if(m_pCharFormat)
        StartListening(m_pCharFormat->GetNotifier());
}

SwFormatCharFormat::~SwFormatCharFormat() {}

bool SwFormatCharFormat::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return m_pCharFormat == static_cast<const SwFormatCharFormat&>(rAttr).m_pCharFormat;
}

SwFormatCharFormat* SwFormatCharFormat::Clone( SfxItemPool* ) const
{
    return new SwFormatCharFormat( *this );
}

// forward to the TextAttribute
void SwFormatCharFormat::Notify(const SfxHint& rHint)
{
    if(!m_pTextAttribute)
        return;
    switch(rHint.GetId())
    {
        case SfxHintId::SwAutoFormatUsedHint:
            m_pTextAttribute->HandleAutoFormatUsedHint(static_cast<const sw::AutoFormatUsedHint&>(rHint));
            break;
        case SfxHintId::SwFormatChange:
            m_pTextAttribute->TriggerNodeUpdate(*static_cast<const SwFormatChangeHint*>(&rHint));
            break;
        case SfxHintId::SwAttrSetChange:
            m_pTextAttribute->TriggerNodeUpdate(*static_cast<const sw::AttrSetChangeHint*>(&rHint));
            break;
        case SfxHintId::SwObjectDying:
            m_pTextAttribute->TriggerNodeUpdate(*static_cast<const sw::ObjectDyingHint*>(&rHint));
            m_pCharFormat = nullptr;
            break;
        case SfxHintId::SwLegacyModify:
            m_pTextAttribute->TriggerNodeUpdate(*static_cast<const sw::LegacyModifyHint*>(&rHint));
            break;
        default:
            break;
    }
}

bool SwFormatCharFormat::QueryValue( uno::Any& rVal, sal_uInt8 ) const
{
    ProgName sCharFormatName;
    if(m_pCharFormat)
        SwStyleNameMapper::FillProgName(m_pCharFormat->GetName(), sCharFormatName,  SwGetPoolIdFromName::ChrFmt );
    rVal <<= sCharFormatName.toString();
    return true;
}
bool SwFormatCharFormat::PutValue( const uno::Any& , sal_uInt8   )
{
    OSL_FAIL("format cannot be set with PutValue!");
    return false;
}

void SwFormatCharFormat::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatCharFormat"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("text-attribute"), "%p", m_pTextAttribute);
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("char-format"), "%p", GetCharFormat());
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("char-format-name"),
                                      BAD_CAST(GetCharFormat()->GetName().toString().toUtf8().getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

SwFormatAutoFormat::SwFormatAutoFormat( sal_uInt16 nInitWhich )
    : SfxPoolItem( nInitWhich  )
{
    setNonShareable();
}

bool SwFormatAutoFormat::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return mpHandle == static_cast<const SwFormatAutoFormat&>(rAttr).mpHandle;
}

SwFormatAutoFormat* SwFormatAutoFormat::Clone( SfxItemPool* ) const
{
    return new SwFormatAutoFormat( *this );
}

bool SwFormatAutoFormat::QueryValue( uno::Any& rVal, sal_uInt8 ) const
{
    rVal <<= StylePool::nameOf( mpHandle );
    return true;
}

bool SwFormatAutoFormat::PutValue( const uno::Any& , sal_uInt8 )
{
    //the format is not renameable via API
    return false;
}

void SwFormatAutoFormat::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatAutoFormat"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    if (mpHandle) // pool default doesn't have one
    {
        mpHandle->dumpAsXml(pWriter);
    }
    (void)xmlTextWriterEndElement(pWriter);
}

SwFormatINetFormat::SwFormatINetFormat()
    : SfxPoolItem( RES_TXTATR_INETFMT  )
    , msURL()
    , msTargetFrame()
    , msINetFormatName()
    , msVisitedFormatName()
    , msHyperlinkName()
    , mpTextAttr( nullptr )
    , mnINetFormatId( 0 )
    , mnVisitedFormatId( 0 )
{
    setNonShareable();
}

SwFormatINetFormat::SwFormatINetFormat( OUString aURL, OUString aTarget )
    : SfxPoolItem( RES_TXTATR_INETFMT )
    , msURL( std::move(aURL) )
    , msTargetFrame( std::move(aTarget) )
    , msINetFormatName()
    , msVisitedFormatName()
    , msHyperlinkName()
    , mpTextAttr( nullptr )
    , mnINetFormatId( RES_POOLCHR_INET_NORMAL )
    , mnVisitedFormatId( RES_POOLCHR_INET_VISIT )
{
    setNonShareable();
    SwStyleNameMapper::FillUIName( mnINetFormatId, msINetFormatName );
    SwStyleNameMapper::FillUIName( mnVisitedFormatId, msVisitedFormatName );
}

SwFormatINetFormat::SwFormatINetFormat( const SwFormatINetFormat& rAttr )
    : SfxPoolItem( RES_TXTATR_INETFMT  )
    , sw::BroadcasterMixin()
    , msURL( rAttr.GetValue() )
    , msTargetFrame( rAttr.msTargetFrame )
    , msINetFormatName( rAttr.msINetFormatName )
    , msVisitedFormatName( rAttr.msVisitedFormatName )
    , msHyperlinkName( rAttr.msHyperlinkName )
    , mpTextAttr( nullptr )
    , mnINetFormatId( rAttr.mnINetFormatId )
    , mnVisitedFormatId( rAttr.mnVisitedFormatId )
{
    setNonShareable();
    if ( rAttr.GetMacroTable() )
        mpMacroTable.reset( new SvxMacroTableDtor( *rAttr.GetMacroTable() ) );
}

SwFormatINetFormat::~SwFormatINetFormat()
{
}

bool SwFormatINetFormat::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    bool bRet = SfxPoolItem::operator==( rAttr )
                && msURL == static_cast<const SwFormatINetFormat&>(rAttr).msURL
                && msHyperlinkName == static_cast<const SwFormatINetFormat&>(rAttr).msHyperlinkName
                && msTargetFrame == static_cast<const SwFormatINetFormat&>(rAttr).msTargetFrame
                && msINetFormatName == static_cast<const SwFormatINetFormat&>(rAttr).msINetFormatName
                && msVisitedFormatName == static_cast<const SwFormatINetFormat&>(rAttr).msVisitedFormatName
                && mnINetFormatId == static_cast<const SwFormatINetFormat&>(rAttr).mnINetFormatId
                && mnVisitedFormatId == static_cast<const SwFormatINetFormat&>(rAttr).mnVisitedFormatId;

    if( !bRet )
        return false;

    const SvxMacroTableDtor* pOther = static_cast<const SwFormatINetFormat&>(rAttr).mpMacroTable.get();
    if( !mpMacroTable )
        return ( !pOther || pOther->empty() );
    if( !pOther )
        return mpMacroTable->empty();

    const SvxMacroTableDtor& rOwn = *mpMacroTable;
    const SvxMacroTableDtor& rOther = *pOther;

    return rOwn == rOther;
}

SwFormatINetFormat* SwFormatINetFormat::Clone( SfxItemPool* ) const
{
    return new SwFormatINetFormat( *this );
}

void SwFormatINetFormat::SetMacroTable( const SvxMacroTableDtor* pNewTable )
{
    if( pNewTable )
    {
        if( mpMacroTable )
            *mpMacroTable = *pNewTable;
        else
            mpMacroTable.reset( new SvxMacroTableDtor( *pNewTable ) );
    }
    else
    {
        mpMacroTable.reset();
    }
}

void SwFormatINetFormat::SetMacro( SvMacroItemId nEvent, const SvxMacro& rMacro )
{
    if( !mpMacroTable )
        mpMacroTable.reset( new SvxMacroTableDtor );

    mpMacroTable->Insert( nEvent, rMacro );
}

const SvxMacro* SwFormatINetFormat::GetMacro( SvMacroItemId nEvent ) const
{
    const SvxMacro* pRet = nullptr;
    if( mpMacroTable && mpMacroTable->IsKeyValid( nEvent ) )
        pRet = mpMacroTable->Get( nEvent );
    return pRet;
}

bool SwFormatINetFormat::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch(nMemberId)
    {
        case MID_URL_URL:
            rVal <<= msURL;
        break;
        case MID_URL_TARGET:
            rVal <<= msTargetFrame;
        break;
        case MID_URL_HYPERLINKNAME:
            rVal <<= msHyperlinkName;
        break;
        case MID_URL_VISITED_FMT:
        {
            UIName sVal = msVisitedFormatName;
            if (sVal.isEmpty() && mnVisitedFormatId != 0)
                SwStyleNameMapper::FillUIName(mnVisitedFormatId, sVal);
            ProgName aRet;
            if (!sVal.isEmpty())
                SwStyleNameMapper::FillProgName(sVal, aRet,
                        SwGetPoolIdFromName::ChrFmt);
            rVal <<= aRet.toString();
        }
        break;
        case MID_URL_UNVISITED_FMT:
        {
            UIName sVal = msINetFormatName;
            if (sVal.isEmpty() && mnINetFormatId != 0)
                SwStyleNameMapper::FillUIName(mnINetFormatId, sVal);
            ProgName aRet;
            if (!sVal.isEmpty())
                SwStyleNameMapper::FillProgName(sVal, aRet,
                        SwGetPoolIdFromName::ChrFmt);
            rVal <<= aRet.toString();
        }
        break;
        case MID_URL_HYPERLINKEVENTS:
        {
            // create (and return) event descriptor
            rtl::Reference<SwHyperlinkEventDescriptor> pEvents =
                new SwHyperlinkEventDescriptor();
            pEvents->copyMacrosFromINetFormat(*this);

            // all others return a string; so we just set rVal here and exit
            rVal <<= uno::Reference<container::XNameReplace>(pEvents);
        }
        break;
        default:
            rVal <<= OUString();
        break;
    }
    return true;
}
bool SwFormatINetFormat::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bRet = true;
    nMemberId &= ~CONVERT_TWIPS;

    // all properties except HyperlinkEvents are of type string, hence
    // we treat HyperlinkEvents specially
    if (MID_URL_HYPERLINKEVENTS == nMemberId)
    {
        uno::Reference<container::XNameReplace> xReplace;
        rVal >>= xReplace;
        if (xReplace.is())
        {
            // Create hyperlink event descriptor. Then copy events
            // from argument into descriptor. Then copy events from
            // the descriptor into the format.
            rtl::Reference<SwHyperlinkEventDescriptor> pEvents = new SwHyperlinkEventDescriptor();
            pEvents->copyMacrosFromNameReplace(xReplace);
            pEvents->copyMacrosIntoINetFormat(*this);
        }
        else
        {
            // wrong type!
            bRet = false;
        }
    }
    else
    {
        // all string properties:
        if(rVal.getValueType() != ::cppu::UnoType<OUString>::get())
            return false;

        switch(nMemberId)
        {
            case MID_URL_URL:
                rVal >>= msURL;
                break;
            case MID_URL_TARGET:
                rVal >>= msTargetFrame;
                break;
            case MID_URL_HYPERLINKNAME:
                rVal >>= msHyperlinkName;
                break;
            case MID_URL_VISITED_FMT:
            {
                OUString sVal;
                rVal >>= sVal;
                UIName aString;
                SwStyleNameMapper::FillUIName( ProgName(sVal), aString, SwGetPoolIdFromName::ChrFmt );
                msVisitedFormatName = std::move(aString);
                mnVisitedFormatId = SwStyleNameMapper::GetPoolIdFromUIName( msVisitedFormatName,
                                               SwGetPoolIdFromName::ChrFmt );
            }
            break;
            case MID_URL_UNVISITED_FMT:
            {
                OUString sVal;
                rVal >>= sVal;
                UIName aString;
                SwStyleNameMapper::FillUIName( ProgName(sVal), aString, SwGetPoolIdFromName::ChrFmt );
                msINetFormatName = std::move(aString);
                mnINetFormatId = SwStyleNameMapper::GetPoolIdFromUIName( msINetFormatName, SwGetPoolIdFromName::ChrFmt );
            }
            break;
            default:
                bRet = false;
        }
    }
    return bRet;
}

SwFormatRuby::SwFormatRuby( OUString aRubyText )
    : SfxPoolItem( RES_TXTATR_CJK_RUBY  ),
    m_sRubyText( std::move(aRubyText) ),
    m_pTextAttr( nullptr ),
    m_nCharFormatId( 0 ),
    m_nPosition( 0 ),
    m_eAdjustment( css::text::RubyAdjust_LEFT )
{
    setNonShareable();
}

SwFormatRuby::SwFormatRuby( const SwFormatRuby& rAttr )
    : SfxPoolItem( RES_TXTATR_CJK_RUBY  ),
    m_sRubyText( rAttr.m_sRubyText ),
    m_sCharFormatName( rAttr.m_sCharFormatName ),
    m_pTextAttr( nullptr ),
    m_nCharFormatId( rAttr.m_nCharFormatId),
    m_nPosition( rAttr.m_nPosition ),
    m_eAdjustment( rAttr.m_eAdjustment )
{
    setNonShareable();
}

SwFormatRuby::~SwFormatRuby()
{
}

void SwFormatRuby::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatRuby"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("m_pTextAttr"), "%p", m_pTextAttr);
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("ruby-text"),
                                      BAD_CAST(m_sRubyText.toUtf8().getStr()));
    SfxPoolItem::dumpAsXml(pWriter);


    (void)xmlTextWriterEndElement(pWriter);
}

SwFormatRuby& SwFormatRuby::operator=( const SwFormatRuby& rAttr )
{
    // SwFormatRuby is not shareable, so ptr compare is OK
    if (areSfxPoolItemPtrsEqual(this, &rAttr))
        return *this;

    m_sRubyText = rAttr.m_sRubyText;
    m_sCharFormatName = rAttr.m_sCharFormatName;
    m_nCharFormatId = rAttr.m_nCharFormatId;
    m_nPosition = rAttr.m_nPosition;
    m_eAdjustment = rAttr.m_eAdjustment;
    m_pTextAttr =  nullptr;
    return *this;
}

bool SwFormatRuby::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return m_sRubyText == static_cast<const SwFormatRuby&>(rAttr).m_sRubyText &&
           m_sCharFormatName == static_cast<const SwFormatRuby&>(rAttr).m_sCharFormatName &&
           m_nCharFormatId == static_cast<const SwFormatRuby&>(rAttr).m_nCharFormatId &&
           m_nPosition == static_cast<const SwFormatRuby&>(rAttr).m_nPosition &&
           m_eAdjustment == static_cast<const SwFormatRuby&>(rAttr).m_eAdjustment;
}

SwFormatRuby* SwFormatRuby::Clone( SfxItemPool* ) const
{
    return new SwFormatRuby( *this );
}

bool SwFormatRuby::QueryValue( uno::Any& rVal,
                            sal_uInt8 nMemberId ) const
{
    bool bRet = true;
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case MID_RUBY_TEXT: rVal <<= m_sRubyText;                    break;
        case MID_RUBY_ADJUST:  rVal <<= static_cast<sal_Int16>(m_eAdjustment);    break;
        case MID_RUBY_CHARSTYLE:
        {
            ProgName aString;
            SwStyleNameMapper::FillProgName(m_sCharFormatName, aString, SwGetPoolIdFromName::ChrFmt );
            rVal <<= aString.toString();
        }
        break;
        case MID_RUBY_ABOVE:
        {
            rVal <<= static_cast<bool>(!m_nPosition);
        }
        break;
        case MID_RUBY_POSITION:
        {
            rVal <<= m_nPosition;
        }
        break;
        default:
            bRet = false;
    }
    return bRet;
}
bool SwFormatRuby::PutValue( const uno::Any& rVal,
                            sal_uInt8 nMemberId )
{
    bool bRet = true;
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case MID_RUBY_TEXT:
            bRet = rVal >>= m_sRubyText;
            break;
        case MID_RUBY_ADJUST:
        {
            sal_Int16 nSet = 0;
            rVal >>= nSet;
            if(nSet >= sal_Int16(text::RubyAdjust_LEFT) && nSet <= sal_Int16(text::RubyAdjust_INDENT_BLOCK))
                m_eAdjustment = static_cast<text::RubyAdjust>(nSet);
            else
                bRet = false;
        }
        break;
        case MID_RUBY_ABOVE:
        {
            const uno::Type& rType = cppu::UnoType<bool>::get();
            if(rVal.hasValue() && rVal.getValueType() == rType)
            {
                bool bAbove = *o3tl::doAccess<bool>(rVal);
                m_nPosition = bAbove ? 0 : 1;
            }
        }
        break;
        case MID_RUBY_POSITION:
        {
            sal_Int16 nSet = 0;
            rVal >>= nSet;
            if(nSet >= sal_Int16(text::RubyPosition::ABOVE) && nSet <= sal_Int16(text::RubyPosition::INTER_CHARACTER))
                m_nPosition = nSet;
            else
                bRet = false;
        }
        break;
        case MID_RUBY_CHARSTYLE:
        {
            OUString sTmp;
            bRet = rVal >>= sTmp;
            if(bRet)
                m_sCharFormatName = SwStyleNameMapper::GetUIName(ProgName(sTmp), SwGetPoolIdFromName::ChrFmt );
        }
        break;
        default:
            bRet = false;
    }
    return bRet;
}

SwFormatMeta * SwFormatMeta::CreatePoolDefault(const sal_uInt16 i_nWhich)
{
    return new SwFormatMeta(i_nWhich);
}

SwFormatMeta::SwFormatMeta(const sal_uInt16 i_nWhich)
    : SfxPoolItem( i_nWhich )
    , m_pMeta()
    , m_pTextAttr( nullptr )
{
    setNonShareable();
    OSL_ENSURE((RES_TXTATR_META == i_nWhich) || (RES_TXTATR_METAFIELD == i_nWhich),
             "ERROR: SwFormatMeta: invalid which id!");
}

SwFormatMeta::SwFormatMeta( std::shared_ptr< ::sw::Meta > i_pMeta,
                        const sal_uInt16 i_nWhich )
    : SfxPoolItem( i_nWhich  )
    , m_pMeta( std::move(i_pMeta) )
    , m_pTextAttr( nullptr )
{
    setNonShareable();
    OSL_ENSURE((RES_TXTATR_META == i_nWhich) || (RES_TXTATR_METAFIELD == i_nWhich),
            "ERROR: SwFormatMeta: invalid which id!");
    OSL_ENSURE(m_pMeta, "SwFormatMeta: no Meta ?");
    // DO NOT call m_pMeta->SetFormatMeta(this) here; only from SetTextAttr!
}

SwFormatMeta::~SwFormatMeta()
{
    // SwFormatMeta is not shareable, so ptr compare is OK
    if (m_pMeta && areSfxPoolItemPtrsEqual(m_pMeta->GetFormatMeta(), this))
    {
        NotifyChangeTextNode(nullptr);
        m_pMeta->SetFormatMeta(nullptr);
    }
}

bool SwFormatMeta::operator==( const SfxPoolItem & i_rOther ) const
{
    return SfxPoolItem::operator==( i_rOther )
        && m_pMeta == static_cast<SwFormatMeta const &>( i_rOther ).m_pMeta;
}

SwFormatMeta* SwFormatMeta::Clone( SfxItemPool * /*pPool*/ ) const
{
    // if this is indeed a copy, then DoCopy must be called later!
    return m_pMeta // #i105148# pool default may be cloned also!
        ? new SwFormatMeta( m_pMeta, Which() ) : new SwFormatMeta( Which() );
}

void SwFormatMeta::SetTextAttr(SwTextMeta * const i_pTextAttr)
{
    OSL_ENSURE(!(m_pTextAttr && i_pTextAttr),
        "SwFormatMeta::SetTextAttr: already has text attribute?");
    OSL_ENSURE(  m_pTextAttr || i_pTextAttr ,
        "SwFormatMeta::SetTextAttr: no attribute to remove?");
    m_pTextAttr = i_pTextAttr;
    OSL_ENSURE(m_pMeta, "inserted SwFormatMeta has no sw::Meta?");
    // the sw::Meta must be able to find the current text attribute!
    if (m_pMeta)
    {
        if (i_pTextAttr)
        {
            m_pMeta->SetFormatMeta(this);
        }
        // SwFormatMeta is not shareable, so ptr compare is OK
        else if (areSfxPoolItemPtrsEqual(m_pMeta->GetFormatMeta(), this))
        {   // text attribute gone => de-register from text node!
            NotifyChangeTextNode(nullptr);
            m_pMeta->SetFormatMeta(nullptr);
        }
    }
}

void SwFormatMeta::NotifyChangeTextNode(SwTextNode *const pTextNode)
{
    // N.B.: do not reset m_pTextAttr here: see call in nodes.cxx,
    // where the hint is not deleted!
    OSL_ENSURE(m_pMeta, "SwFormatMeta::NotifyChangeTextNode: no Meta?");
    // SwFormatMeta is not shareable, so ptr compare is OK
    if (m_pMeta && areSfxPoolItemPtrsEqual(m_pMeta->GetFormatMeta(), this))
    {   // do not call Modify, that would call SwXMeta::SwClientNotify
        m_pMeta->NotifyChangeTextNode(pTextNode);
    }
}

// this SwFormatMeta has been cloned and points at the same sw::Meta as the source
// this method copies the sw::Meta
void SwFormatMeta::DoCopy(::sw::MetaFieldManager & i_rTargetDocManager,
        SwTextNode & i_rTargetTextNode)
{
    OSL_ENSURE(m_pMeta, "DoCopy called for SwFormatMeta with no sw::Meta?");
    if (!m_pMeta)
        return;

    const std::shared_ptr< ::sw::Meta> pOriginal( m_pMeta );
    if (RES_TXTATR_META == Which())
    {
        m_pMeta = std::make_shared<::sw::Meta>(this);
    }
    else
    {
        ::sw::MetaField *const pMetaField(
            static_cast< ::sw::MetaField* >(pOriginal.get()));
        m_pMeta = i_rTargetDocManager.makeMetaField( this,
            pMetaField->m_nNumberFormat, pMetaField->IsFixedLanguage() );
    }
    // Meta must have a text node before calling RegisterAsCopyOf
    m_pMeta->NotifyChangeTextNode(& i_rTargetTextNode);
    // this cannot be done in Clone: a Clone is not necessarily a copy!
    m_pMeta->RegisterAsCopyOf(*pOriginal);
}

namespace sw {

Meta::Meta(SwFormatMeta * const i_pFormat)
    : ::sfx2::Metadatable()
    , sw::BroadcastingModify()
    , m_pFormat(i_pFormat)
    , m_pTextNode(nullptr)
{
}

Meta::~Meta()
{
}

SwTextMeta * Meta::GetTextAttr() const
{
    return m_pFormat ? m_pFormat->GetTextAttr() : nullptr;
}

void Meta::SetXMeta(rtl::Reference<SwXMeta> const& xMeta)
{ m_wXMeta = xMeta.get(); }

void Meta::NotifyChangeTextNode(SwTextNode *const pTextNode)
{
    m_pTextNode = pTextNode;
    if (m_pTextNode && (GetRegisteredIn() != m_pTextNode))
    {
        m_pTextNode->Add(*this);
    }
    else if (!m_pTextNode)
    {
        EndListeningAll();
    }
    if (!pTextNode) // text node gone? invalidate UNO object!
    {
        GetNotifier().Broadcast(SfxHint(SfxHintId::Deinitializing));
    }
}

void Meta::SwClientNotify(const SwModify&, const SfxHint& rHint)
{
    if(SfxHintId::SwRemoveUnoObject == rHint.GetId())
    {
        CallSwClientNotify(rHint);
        GetNotifier().Broadcast(SfxHint(SfxHintId::DataChanged));
        // invalidate cached uno object
        SetXMeta(nullptr);
        GetNotifier().Broadcast(SfxHint(SfxHintId::Deinitializing));
    }
    else if (rHint.GetId() == SfxHintId::SwLegacyModify
            || rHint.GetId() == SfxHintId::SwFormatChange
            || rHint.GetId() == SfxHintId::SwAttrSetChange
            || rHint.GetId() == SfxHintId::SwObjectDying
            || rHint.GetId() == SfxHintId::SwUpdateAttr)
    {
        CallSwClientNotify(rHint);
        GetNotifier().Broadcast(SfxHint(SfxHintId::DataChanged));
    }
}

// sfx2::Metadatable
::sfx2::IXmlIdRegistry& Meta::GetRegistry()
{
    SwTextNode * const pTextNode( GetTextNode() );
    // GetRegistry may only be called on a meta that is actually in the
    // document, which means it has a pointer to its text node
    OSL_ENSURE(pTextNode, "ERROR: GetRegistry: no text node?");
    if (!pTextNode)
        throw uno::RuntimeException();
    return pTextNode->GetRegistry();
}

bool Meta::IsInClipboard() const
{
    const SwTextNode * const pTextNode( GetTextNode() );
// no text node: in UNDO  OSL_ENSURE(pTextNode, "IsInClipboard: no text node?");
    return pTextNode && pTextNode->IsInClipboard();
}

bool Meta::IsInUndo() const
{
    const SwTextNode * const pTextNode( GetTextNode() );
// no text node: in UNDO  OSL_ENSURE(pTextNode, "IsInUndo: no text node?");
    return pTextNode == nullptr || pTextNode->IsInUndo();
}

bool Meta::IsInContent() const
{
    const SwTextNode * const pTextNode( GetTextNode() );
    OSL_ENSURE(pTextNode, "IsInContent: no text node?");
    return pTextNode == nullptr || pTextNode->IsInContent();
}

css::uno::Reference< css::rdf::XMetadatable > Meta::MakeUnoObject()
{
    return SwXMeta::CreateXMeta(*this, {}, {});
}

MetaField::MetaField(SwFormatMeta * const i_pFormat,
            const sal_uInt32 nNumberFormat, const bool bIsFixedLanguage)
    : Meta(i_pFormat)
    , m_nNumberFormat( nNumberFormat )
    , m_bIsFixedLanguage( bIsFixedLanguage )
{
}

void MetaField::GetPrefixAndSuffix(
        OUString *const o_pPrefix, OUString *const o_pSuffix, OUString *const o_pShadowsColor)
{
    try
    {
        const uno::Reference<rdf::XMetadatable> xMetaField( MakeUnoObject() );
        assert(dynamic_cast<SwXMetaField*>(xMetaField.get()) && "GetPrefixAndSuffix: no SwXMetaField?");
        if (xMetaField.is())
        {
            SwTextNode * const pTextNode( GetTextNode() );
            SwDocShell const * const pShell(pTextNode->GetDoc().GetDocShell());
            const uno::Reference<frame::XModel> xModel(
                pShell ? pShell->GetModel() : nullptr,  uno::UNO_SET_THROW);
            getPrefixAndSuffix(xModel, xMetaField, o_pPrefix, o_pSuffix, o_pShadowsColor);
        }
    }
    catch (const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION( "sw", "");
    }
}

sal_uInt32 MetaField::GetNumberFormat(const OUString& aContent) const
{
    //TODO: this probably lacks treatment for some special cases
    sal_uInt32 nNumberFormat( m_nNumberFormat );
    SwTextNode * const pTextNode( GetTextNode() );
    if (pTextNode)
    {
        double number;
        (void) pTextNode->GetDoc().IsNumberFormat( aContent, nNumberFormat, number );
    }
    return nNumberFormat;
}

void MetaField::SetNumberFormat(sal_uInt32 nNumberFormat)
{
    // effectively, the member is only a default:
    // GetNumberFormat checks if the text actually conforms
    m_nNumberFormat = nNumberFormat;
}

MetaFieldManager::MetaFieldManager()
{
}

std::shared_ptr<MetaField>
MetaFieldManager::makeMetaField(SwFormatMeta * const i_pFormat,
        const sal_uInt32 nNumberFormat, const bool bIsFixedLanguage)
{
    std::shared_ptr<MetaField> pMetaField(
        new MetaField(i_pFormat, nNumberFormat, bIsFixedLanguage) );
    m_MetaFields.push_back(pMetaField);
    return pMetaField;
}

namespace {

struct IsInUndo
{
    bool operator()(std::weak_ptr<MetaField> const & pMetaField) {
        return pMetaField.lock()->IsInUndo();
    }
};

struct MakeUnoObject
{
    uno::Reference<text::XTextField>
    operator()(std::weak_ptr<MetaField> const & pMetaField) {
        return uno::Reference<text::XTextField>(
                pMetaField.lock()->MakeUnoObject(), uno::UNO_QUERY);
    }
};

}

std::vector< uno::Reference<text::XTextField> >
MetaFieldManager::getMetaFields()
{
    // erase deleted fields
    std::erase_if(m_MetaFields, [] (std::weak_ptr<MetaField> const& rField) { return rField.expired(); });
    // filter out fields in UNDO
    MetaFieldList_t filtered(m_MetaFields.size());
    const MetaFieldList_t::iterator iter2(
    std::remove_copy_if(m_MetaFields.begin(), m_MetaFields.end(),
        filtered.begin(), IsInUndo()));
    filtered.erase(iter2, filtered.end());
    // create uno objects
    std::vector< uno::Reference<text::XTextField> > ret(filtered.size());
    std::transform(filtered.begin(), filtered.end(), ret.begin(),
            MakeUnoObject());
    return ret;
}

void MetaFieldManager::copyDocumentProperties(const SwDoc& rSource)
{
    const SwDocShell* pDocShell = rSource.GetDocShell();
    if (!pDocShell)
        return;

    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(pDocShell->GetModel(), uno::UNO_QUERY);
    uno::Reference<util::XCloneable> xCloneable(xDocumentPropertiesSupplier->getDocumentProperties(), uno::UNO_QUERY);
    m_xDocumentProperties.set(xCloneable->createClone(), uno::UNO_QUERY);
}

const uno::Reference<document::XDocumentProperties>& MetaFieldManager::getDocumentProperties() const
{
    return m_xDocumentProperties;
}

} // namespace sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

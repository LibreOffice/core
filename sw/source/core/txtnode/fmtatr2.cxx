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
#include "unomid.h"

#include <basic/sbxvar.hxx>
#include <svl/macitem.hxx>
#include <svl/stritem.hxx>
#include <svl/stylepool.hxx>
#include <fmtautofmt.hxx>
#include <fchrfmt.hxx>
#include <fmtinfmt.hxx>
#include <txtatr.hxx>
#include <fmtruby.hxx>
#include <charfmt.hxx>
#include <hints.hxx>
#include <unostyle.hxx>
#include <unoevent.hxx>
#include <com/sun/star/text/RubyAdjust.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/util/XCloneable.hpp>

#include <cmdid.h>
#include <com/sun/star/uno/Any.h>
#include <SwStyleNameMapper.hxx>

#include <fmtmeta.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <unometa.hxx>
#include <docsh.hxx>
#include <svl/zforlist.hxx>

#include <algorithm>

using namespace ::com::sun::star;


SfxPoolItem* SwFormatINetFormat::CreateDefault() { return new SwFormatINetFormat; }

SwFormatCharFormat::SwFormatCharFormat( SwCharFormat *pFormat )
    : SfxPoolItem( RES_TXTATR_CHARFMT ),
    SwClient(pFormat),
    pTextAttr( nullptr )
{
}

SwFormatCharFormat::SwFormatCharFormat( const SwFormatCharFormat& rAttr )
    : SfxPoolItem( RES_TXTATR_CHARFMT ),
    SwClient( rAttr.GetCharFormat() ),
    pTextAttr( nullptr )
{
}

SwFormatCharFormat::~SwFormatCharFormat() {}

bool SwFormatCharFormat::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return GetCharFormat() == static_cast<const SwFormatCharFormat&>(rAttr).GetCharFormat();
}

SfxPoolItem* SwFormatCharFormat::Clone( SfxItemPool* ) const
{
    return new SwFormatCharFormat( *this );
}

// weiterleiten an das TextAttribut
void SwFormatCharFormat::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    if( pTextAttr )
        pTextAttr->ModifyNotification( pOld, pNew );
}

// weiterleiten an das TextAttribut
bool SwFormatCharFormat::GetInfo( SfxPoolItem& rInfo ) const
{
    return pTextAttr && pTextAttr->GetInfo( rInfo );
}
bool SwFormatCharFormat::QueryValue( uno::Any& rVal, sal_uInt8 ) const
{
    OUString sCharFormatName;
    if(GetCharFormat())
        SwStyleNameMapper::FillProgName(GetCharFormat()->GetName(), sCharFormatName,  nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, true );
    rVal <<= sCharFormatName;
    return true;
}
bool SwFormatCharFormat::PutValue( const uno::Any& , sal_uInt8   )
{
    OSL_FAIL("Zeichenvorlage kann mit PutValue nicht gesetzt werden!");
    return false;
}

SwFormatAutoFormat::SwFormatAutoFormat( sal_uInt16 nInitWhich )
    : SfxPoolItem( nInitWhich )
{
}

SwFormatAutoFormat::SwFormatAutoFormat( const SwFormatAutoFormat& rAttr )
    : SfxPoolItem( rAttr.Which() ), mpHandle( rAttr.mpHandle )
{
}

SwFormatAutoFormat::~SwFormatAutoFormat()
{
}

bool SwFormatAutoFormat::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return mpHandle == static_cast<const SwFormatAutoFormat&>(rAttr).mpHandle;
}

SfxPoolItem* SwFormatAutoFormat::Clone( SfxItemPool* ) const
{
    return new SwFormatAutoFormat( *this );
}

bool SwFormatAutoFormat::QueryValue( uno::Any& rVal, sal_uInt8 ) const
{
    OUString sCharFormatName = StylePool::nameOf( mpHandle );
    rVal <<= OUString( sCharFormatName );
    return true;
}

bool SwFormatAutoFormat::PutValue( const uno::Any& , sal_uInt8 )
{
    //the format is not renameable via API
    return false;
}

void SwFormatAutoFormat::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swFormatAutoFormat"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    mpHandle->dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);
}

SwFormatINetFormat::SwFormatINetFormat()
    : SfxPoolItem( RES_TXTATR_INETFMT )
    , msURL()
    , msTargetFrame()
    , msINetFormatName()
    , msVisitedFormatName()
    , msHyperlinkName()
    , mpMacroTable( nullptr )
    , mpTextAttr( nullptr )
    , mnINetFormatId( 0 )
    , mnVisitedFormatId( 0 )
{}

SwFormatINetFormat::SwFormatINetFormat( const OUString& rURL, const OUString& rTarget )
    : SfxPoolItem( RES_TXTATR_INETFMT )
    , msURL( rURL )
    , msTargetFrame( rTarget )
    , msINetFormatName()
    , msVisitedFormatName()
    , msHyperlinkName()
    , mpMacroTable( nullptr )
    , mpTextAttr( nullptr )
    , mnINetFormatId( RES_POOLCHR_INET_NORMAL )
    , mnVisitedFormatId( RES_POOLCHR_INET_VISIT )
{
    SwStyleNameMapper::FillUIName( mnINetFormatId, msINetFormatName );
    SwStyleNameMapper::FillUIName( mnVisitedFormatId, msVisitedFormatName );
}

SwFormatINetFormat::SwFormatINetFormat( const SwFormatINetFormat& rAttr )
    : SfxPoolItem( RES_TXTATR_INETFMT )
    , msURL( rAttr.GetValue() )
    , msTargetFrame( rAttr.msTargetFrame )
    , msINetFormatName( rAttr.msINetFormatName )
    , msVisitedFormatName( rAttr.msVisitedFormatName )
    , msHyperlinkName( rAttr.msHyperlinkName )
    , mpMacroTable( nullptr )
    , mpTextAttr( nullptr )
    , mnINetFormatId( rAttr.mnINetFormatId )
    , mnVisitedFormatId( rAttr.mnVisitedFormatId )
{
    if ( rAttr.GetMacroTable() )
        mpMacroTable = new SvxMacroTableDtor( *rAttr.GetMacroTable() );
}

SwFormatINetFormat::~SwFormatINetFormat()
{
    delete mpMacroTable;
}

bool SwFormatINetFormat::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    bool bRet = SfxPoolItem::operator==( (SfxPoolItem&) rAttr )
                && msURL == static_cast<const SwFormatINetFormat&>(rAttr).msURL
                && msHyperlinkName == static_cast<const SwFormatINetFormat&>(rAttr).msHyperlinkName
                && msTargetFrame == static_cast<const SwFormatINetFormat&>(rAttr).msTargetFrame
                && msINetFormatName == static_cast<const SwFormatINetFormat&>(rAttr).msINetFormatName
                && msVisitedFormatName == static_cast<const SwFormatINetFormat&>(rAttr).msVisitedFormatName
                && mnINetFormatId == static_cast<const SwFormatINetFormat&>(rAttr).mnINetFormatId
                && mnVisitedFormatId == static_cast<const SwFormatINetFormat&>(rAttr).mnVisitedFormatId;

    if( !bRet )
        return false;

    const SvxMacroTableDtor* pOther = static_cast<const SwFormatINetFormat&>(rAttr).mpMacroTable;
    if( !mpMacroTable )
        return ( !pOther || pOther->empty() );
    if( !pOther )
        return mpMacroTable->empty();

    const SvxMacroTableDtor& rOwn = *mpMacroTable;
    const SvxMacroTableDtor& rOther = *pOther;

    return rOwn == rOther;
}

SfxPoolItem* SwFormatINetFormat::Clone( SfxItemPool* ) const
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
            mpMacroTable = new SvxMacroTableDtor( *pNewTable );
    }
    else
    {
        delete mpMacroTable;
        mpMacroTable = nullptr;
    }
}

void SwFormatINetFormat::SetMacro( sal_uInt16 nEvent, const SvxMacro& rMacro )
{
    if( !mpMacroTable )
        mpMacroTable = new SvxMacroTableDtor;

    mpMacroTable->Insert( nEvent, rMacro );
}

const SvxMacro* SwFormatINetFormat::GetMacro( sal_uInt16 nEvent ) const
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
            OUString sVal = msVisitedFormatName;
            if (sVal.isEmpty() && mnVisitedFormatId != 0)
                SwStyleNameMapper::FillUIName(mnVisitedFormatId, sVal);
            if (!sVal.isEmpty())
                SwStyleNameMapper::FillProgName(sVal, sVal,
                        nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, true);
            rVal <<= sVal;
        }
        break;
        case MID_URL_UNVISITED_FMT:
        {
            OUString sVal = msINetFormatName;
            if (sVal.isEmpty() && mnINetFormatId != 0)
                SwStyleNameMapper::FillUIName(mnINetFormatId, sVal);
            if (!sVal.isEmpty())
                SwStyleNameMapper::FillProgName(sVal, sVal,
                        nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, true);
            rVal <<= sVal;
        }
        break;
        case MID_URL_HYPERLINKEVENTS:
        {
            // create (and return) event descriptor
            SwHyperlinkEventDescriptor* pEvents =
                new SwHyperlinkEventDescriptor();
            pEvents->copyMacrosFromINetFormat(*this);
            uno::Reference<container::XNameReplace> xNameReplace(pEvents);

            // all others return a string; so we just set rVal here and exit
            rVal <<= xNameReplace;
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
            SwHyperlinkEventDescriptor* pEvents = new SwHyperlinkEventDescriptor();
            uno::Reference< lang::XServiceInfo> xHold = pEvents;
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
                OUString aString;
                SwStyleNameMapper::FillUIName( sVal, aString, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, true );
                msVisitedFormatName = aString;
                mnVisitedFormatId = SwStyleNameMapper::GetPoolIdFromUIName( msVisitedFormatName,
                                               nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
            }
            break;
            case MID_URL_UNVISITED_FMT:
            {
                OUString sVal;
                rVal >>= sVal;
                OUString aString;
                SwStyleNameMapper::FillUIName( sVal, aString, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, true );
                msINetFormatName = aString;
                mnINetFormatId = SwStyleNameMapper::GetPoolIdFromUIName( msINetFormatName, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
            }
            break;
            default:
                bRet = false;
        }
    }
    return bRet;
}

SwFormatRuby::SwFormatRuby( const OUString& rRubyText )
    : SfxPoolItem( RES_TXTATR_CJK_RUBY ),
    sRubyText( rRubyText ),
    pTextAttr( nullptr ),
    nCharFormatId( 0 ),
    nPosition( 0 ),
    nAdjustment( 0 )
{
}

SwFormatRuby::SwFormatRuby( const SwFormatRuby& rAttr )
    : SfxPoolItem( RES_TXTATR_CJK_RUBY ),
    sRubyText( rAttr.sRubyText ),
    sCharFormatName( rAttr.sCharFormatName ),
    pTextAttr( nullptr ),
    nCharFormatId( rAttr.nCharFormatId),
    nPosition( rAttr.nPosition ),
    nAdjustment( rAttr.nAdjustment )
{
}

SwFormatRuby::~SwFormatRuby()
{
}

SwFormatRuby& SwFormatRuby::operator=( const SwFormatRuby& rAttr )
{
    sRubyText = rAttr.sRubyText;
    sCharFormatName = rAttr.sCharFormatName;
    nCharFormatId = rAttr.nCharFormatId;
    nPosition = rAttr.nPosition;
    nAdjustment = rAttr.nAdjustment;
    pTextAttr =  nullptr;
    return *this;
}

bool SwFormatRuby::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return sRubyText == static_cast<const SwFormatRuby&>(rAttr).sRubyText &&
           sCharFormatName == static_cast<const SwFormatRuby&>(rAttr).sCharFormatName &&
           nCharFormatId == static_cast<const SwFormatRuby&>(rAttr).nCharFormatId &&
           nPosition == static_cast<const SwFormatRuby&>(rAttr).nPosition &&
           nAdjustment == static_cast<const SwFormatRuby&>(rAttr).nAdjustment;
}

SfxPoolItem* SwFormatRuby::Clone( SfxItemPool* ) const
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
        case MID_RUBY_TEXT: rVal <<= sRubyText;                    break;
        case MID_RUBY_ADJUST:  rVal <<= (sal_Int16)nAdjustment;    break;
        case MID_RUBY_CHARSTYLE:
        {
            OUString aString;
            SwStyleNameMapper::FillProgName(sCharFormatName, aString, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, true );
            rVal <<= aString;
        }
        break;
        case MID_RUBY_ABOVE:
        {
            rVal <<= static_cast<bool>(!nPosition);
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
        {
            OUString sTmp;
            bRet = rVal >>= sTmp;
            sRubyText = sTmp;
        }
        break;
         case MID_RUBY_ADJUST:
        {
            sal_Int16 nSet = 0;
            rVal >>= nSet;
            if(nSet >= 0 && nSet <= text::RubyAdjust_INDENT_BLOCK)
                nAdjustment = nSet;
            else
                bRet = false;
        }
        break;
        case MID_RUBY_ABOVE:
        {
            const uno::Type& rType = cppu::UnoType<bool>::get();
            if(rVal.hasValue() && rVal.getValueType() == rType)
            {
                bool bAbove = *static_cast<sal_Bool const *>(rVal.getValue());
                nPosition = bAbove ? 0 : 1;
            }
        }
        break;
        case MID_RUBY_CHARSTYLE:
        {
            OUString sTmp;
            bRet = rVal >>= sTmp;
            if(bRet)
                sCharFormatName = SwStyleNameMapper::GetUIName(sTmp, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
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
   OSL_ENSURE((RES_TXTATR_META == i_nWhich) || (RES_TXTATR_METAFIELD == i_nWhich),
            "ERROR: SwFormatMeta: invalid which id!");
}

SwFormatMeta::SwFormatMeta( std::shared_ptr< ::sw::Meta > const & i_pMeta,
                        const sal_uInt16 i_nWhich )
    : SfxPoolItem( i_nWhich )
    , m_pMeta( i_pMeta )
    , m_pTextAttr( nullptr )
{
   OSL_ENSURE((RES_TXTATR_META == i_nWhich) || (RES_TXTATR_METAFIELD == i_nWhich),
            "ERROR: SwFormatMeta: invalid which id!");
    OSL_ENSURE(m_pMeta, "SwFormatMeta: no Meta ?");
    // DO NOT call m_pMeta->SetFormatMeta(this) here; only from SetTextAttr!
}

SwFormatMeta::~SwFormatMeta()
{
    if (m_pMeta && (m_pMeta->GetFormatMeta() == this))
    {
        NotifyChangeTextNode(nullptr);
        m_pMeta->SetFormatMeta(nullptr);
    }
}

bool SwFormatMeta::operator==( const SfxPoolItem & i_rOther ) const
{
    assert(SfxPoolItem::operator==(i_rOther));
    return SfxPoolItem::operator==( i_rOther )
        && (m_pMeta == static_cast<SwFormatMeta const &>( i_rOther ).m_pMeta);
}

SfxPoolItem * SwFormatMeta::Clone( SfxItemPool * /*pPool*/ ) const
{
    // if this is indeed a copy, then DoCopy must be called later!
    return (m_pMeta) // #i105148# pool default may be cloned also!
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
        else if (m_pMeta->GetFormatMeta() == this)
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
    if (m_pMeta && (m_pMeta->GetFormatMeta() == this))
    {   // do not call Modify, that would call SwXMeta::Modify!
        m_pMeta->NotifyChangeTextNode(pTextNode);
    }
}

// this SwFormatMeta has been cloned and points at the same sw::Meta as the source
// this method copies the sw::Meta
void SwFormatMeta::DoCopy(::sw::MetaFieldManager & i_rTargetDocManager,
        SwTextNode & i_rTargetTextNode)
{
    OSL_ENSURE(m_pMeta, "DoCopy called for SwFormatMeta with no sw::Meta?");
    if (m_pMeta)
    {
        const std::shared_ptr< ::sw::Meta> pOriginal( m_pMeta );
        if (RES_TXTATR_META == Which())
        {
            m_pMeta.reset( new ::sw::Meta(this) );
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
}

namespace sw {

Meta::Meta(SwFormatMeta * const i_pFormat)
    : ::sfx2::Metadatable()
    , SwModify()
    , m_pFormat(i_pFormat)
    , m_pTextNode(nullptr)
{
}

Meta::~Meta()
{
}

SwTextMeta * Meta::GetTextAttr() const
{
    return (m_pFormat) ? m_pFormat->GetTextAttr() : nullptr;
}


void Meta::NotifyChangeTextNodeImpl()
{
    if (m_pTextNode && (GetRegisteredIn() != m_pTextNode))
    {
        m_pTextNode->Add(this);
    }
    else if (!m_pTextNode && GetRegisteredIn())
    {
        GetRegisteredInNonConst()->Remove(this);
    }
}

void Meta::NotifyChangeTextNode(SwTextNode *const pTextNode)
{
    m_pTextNode = pTextNode;
    NotifyChangeTextNodeImpl();
    if (!pTextNode) // text node gone? invalidate UNO object!
    {
        SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT,
            &static_cast<SwModify&>(*this) ); // cast to base class!
        this->Modify(&aMsgHint, &aMsgHint);
    }
}

// SwClient
void Meta::Modify( const SfxPoolItem *pOld, const SfxPoolItem *pNew )
{
    NotifyClients(pOld, pNew);
    if (pOld && (RES_REMOVE_UNO_OBJECT == pOld->Which()))
    {   // invalidate cached uno object
        SetXMeta(uno::Reference<rdf::XMetadatable>(nullptr));
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
    return SwXMeta::CreateXMeta(*this);
}

MetaField::MetaField(SwFormatMeta * const i_pFormat,
            const sal_uInt32 nNumberFormat, const bool bIsFixedLanguage)
    : Meta(i_pFormat)
    , m_nNumberFormat( nNumberFormat )
    , m_bIsFixedLanguage( bIsFixedLanguage )
{
}

void MetaField::GetPrefixAndSuffix(
        OUString *const o_pPrefix, OUString *const o_pSuffix)
{
    try
    {
        const uno::Reference<rdf::XMetadatable> xMetaField( MakeUnoObject() );
        OSL_ENSURE(dynamic_cast<SwXMetaField*>(xMetaField.get()),
                "GetPrefixAndSuffix: no SwXMetaField?");
        if (xMetaField.is())
        {
            SwTextNode * const pTextNode( GetTextNode() );
            SwDocShell const * const pShell(pTextNode->GetDoc()->GetDocShell());
            const uno::Reference<frame::XModel> xModel(
                (pShell) ? pShell->GetModel() : nullptr,  uno::UNO_SET_THROW);
            getPrefixAndSuffix(xModel, xMetaField, o_pPrefix, o_pSuffix);
        }
    }
    catch (const uno::Exception&)
    {
        OSL_FAIL("exception?");
    }
}

sal_uInt32 MetaField::GetNumberFormat(OUString const & rContent) const
{
    //TODO: this probably lacks treatment for some special cases
    sal_uInt32 nNumberFormat( m_nNumberFormat );
    SwTextNode * const pTextNode( GetTextNode() );
    if (pTextNode)
    {
        SvNumberFormatter *const pNumberFormatter(
                pTextNode->GetDoc()->GetNumberFormatter() );
        double number;
        (void) pNumberFormatter->IsNumberFormat(
                rContent, nNumberFormat, number );
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
    const std::shared_ptr<MetaField> pMetaField(
        new MetaField(i_pFormat, nNumberFormat, bIsFixedLanguage) );
    m_MetaFields.push_back(pMetaField);
    return pMetaField;
}

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

::std::vector< uno::Reference<text::XTextField> >
MetaFieldManager::getMetaFields()
{
    // erase deleted fields
    const MetaFieldList_t::iterator iter(
        ::std::remove_if(m_MetaFields.begin(), m_MetaFields.end(),
            [] (std::weak_ptr<MetaField> const& rField) { return rField.expired(); }));
    m_MetaFields.erase(iter, m_MetaFields.end());
    // filter out fields in UNDO
    MetaFieldList_t filtered(m_MetaFields.size());
    const MetaFieldList_t::iterator iter2(
    ::std::remove_copy_if(m_MetaFields.begin(), m_MetaFields.end(),
        filtered.begin(), IsInUndo()));
    filtered.erase(iter2, filtered.end());
    // create uno objects
    ::std::vector< uno::Reference<text::XTextField> > ret(filtered.size());
    ::std::transform(filtered.begin(), filtered.end(), ret.begin(),
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

uno::Reference<document::XDocumentProperties> MetaFieldManager::getDocumentProperties()
{
    return m_xDocumentProperties;
}

} // namespace sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

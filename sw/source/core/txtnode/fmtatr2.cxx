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

#include <cmdid.h>
#include <com/sun/star/uno/Any.h>
#include <SwStyleNameMapper.hxx>

#include <fmtmeta.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <unometa.hxx>
#include <docsh.hxx>
#include <svl/zforlist.hxx>

#include <boost/bind.hpp>
#include <algorithm>

using namespace ::com::sun::star;

TYPEINIT1_AUTOFACTORY(SwFmtINetFmt, SfxPoolItem);
TYPEINIT1_AUTOFACTORY(SwFmtAutoFmt, SfxPoolItem);

SwFmtCharFmt::SwFmtCharFmt( SwCharFmt *pFmt )
    : SfxPoolItem( RES_TXTATR_CHARFMT ),
    SwClient(pFmt),
    pTxtAttr( 0 )
{
}

SwFmtCharFmt::SwFmtCharFmt( const SwFmtCharFmt& rAttr )
    : SfxPoolItem( RES_TXTATR_CHARFMT ),
    SwClient( rAttr.GetCharFmt() ),
    pTxtAttr( 0 )
{
}

SwFmtCharFmt::~SwFmtCharFmt() {}

bool SwFmtCharFmt::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return GetCharFmt() == static_cast<const SwFmtCharFmt&>(rAttr).GetCharFmt();
}

SfxPoolItem* SwFmtCharFmt::Clone( SfxItemPool* ) const
{
    return new SwFmtCharFmt( *this );
}

// weiterleiten an das TextAttribut
void SwFmtCharFmt::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    if( pTxtAttr )
        pTxtAttr->ModifyNotification( pOld, pNew );
}

// weiterleiten an das TextAttribut
bool SwFmtCharFmt::GetInfo( SfxPoolItem& rInfo ) const
{
    return pTxtAttr && pTxtAttr->GetInfo( rInfo );
}
bool SwFmtCharFmt::QueryValue( uno::Any& rVal, sal_uInt8 ) const
{
    OUString sCharFmtName;
    if(GetCharFmt())
        SwStyleNameMapper::FillProgName(GetCharFmt()->GetName(), sCharFmtName,  nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, true );
    rVal <<= sCharFmtName;
    return true;
}
bool SwFmtCharFmt::PutValue( const uno::Any& , sal_uInt8   )
{
    OSL_FAIL("Zeichenvorlage kann mit PutValue nicht gesetzt werden!");
    return false;
}

SwFmtAutoFmt::SwFmtAutoFmt( sal_uInt16 nInitWhich )
    : SfxPoolItem( nInitWhich )
{
}

SwFmtAutoFmt::SwFmtAutoFmt( const SwFmtAutoFmt& rAttr )
    : SfxPoolItem( rAttr.Which() ), mpHandle( rAttr.mpHandle )
{
}

SwFmtAutoFmt::~SwFmtAutoFmt()
{
}

bool SwFmtAutoFmt::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return mpHandle == static_cast<const SwFmtAutoFmt&>(rAttr).mpHandle;
}

SfxPoolItem* SwFmtAutoFmt::Clone( SfxItemPool* ) const
{
    return new SwFmtAutoFmt( *this );
}

bool SwFmtAutoFmt::QueryValue( uno::Any& rVal, sal_uInt8 ) const
{
    OUString sCharFmtName = StylePool::nameOf( mpHandle );
    rVal <<= OUString( sCharFmtName );
    return true;
}

bool SwFmtAutoFmt::PutValue( const uno::Any& , sal_uInt8 )
{
    //the format is not renameable via API
    return false;
}

SwFmtINetFmt::SwFmtINetFmt()
    : SfxPoolItem( RES_TXTATR_INETFMT )
    , msURL()
    , msTargetFrame()
    , msINetFmtName()
    , msVisitedFmtName()
    , msHyperlinkName()
    , mpMacroTbl( 0 )
    , mpTxtAttr( 0 )
    , mnINetFmtId( 0 )
    , mnVisitedFmtId( 0 )
{}

SwFmtINetFmt::SwFmtINetFmt( const OUString& rURL, const OUString& rTarget )
    : SfxPoolItem( RES_TXTATR_INETFMT )
    , msURL( rURL )
    , msTargetFrame( rTarget )
    , msINetFmtName()
    , msVisitedFmtName()
    , msHyperlinkName()
    , mpMacroTbl( 0 )
    , mpTxtAttr( 0 )
    , mnINetFmtId( RES_POOLCHR_INET_NORMAL )
    , mnVisitedFmtId( RES_POOLCHR_INET_VISIT )
{
    SwStyleNameMapper::FillUIName( mnINetFmtId, msINetFmtName );
    SwStyleNameMapper::FillUIName( mnVisitedFmtId, msVisitedFmtName );
}

SwFmtINetFmt::SwFmtINetFmt( const SwFmtINetFmt& rAttr )
    : SfxPoolItem( RES_TXTATR_INETFMT )
    , msURL( rAttr.GetValue() )
    , msTargetFrame( rAttr.msTargetFrame )
    , msINetFmtName( rAttr.msINetFmtName )
    , msVisitedFmtName( rAttr.msVisitedFmtName )
    , msHyperlinkName( rAttr.msHyperlinkName )
    , mpMacroTbl( 0 )
    , mpTxtAttr( 0 )
    , mnINetFmtId( rAttr.mnINetFmtId )
    , mnVisitedFmtId( rAttr.mnVisitedFmtId )
{
    if ( rAttr.GetMacroTbl() )
        mpMacroTbl = new SvxMacroTableDtor( *rAttr.GetMacroTbl() );
}

SwFmtINetFmt::~SwFmtINetFmt()
{
    delete mpMacroTbl;
}

bool SwFmtINetFmt::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    bool bRet = SfxPoolItem::operator==( (SfxPoolItem&) rAttr )
                && msURL == static_cast<const SwFmtINetFmt&>(rAttr).msURL
                && msHyperlinkName == static_cast<const SwFmtINetFmt&>(rAttr).msHyperlinkName
                && msTargetFrame == static_cast<const SwFmtINetFmt&>(rAttr).msTargetFrame
                && msINetFmtName == static_cast<const SwFmtINetFmt&>(rAttr).msINetFmtName
                && msVisitedFmtName == static_cast<const SwFmtINetFmt&>(rAttr).msVisitedFmtName
                && mnINetFmtId == static_cast<const SwFmtINetFmt&>(rAttr).mnINetFmtId
                && mnVisitedFmtId == static_cast<const SwFmtINetFmt&>(rAttr).mnVisitedFmtId;

    if( !bRet )
        return false;

    const SvxMacroTableDtor* pOther = static_cast<const SwFmtINetFmt&>(rAttr).mpMacroTbl;
    if( !mpMacroTbl )
        return ( !pOther || pOther->empty() );
    if( !pOther )
        return mpMacroTbl->empty();

    const SvxMacroTableDtor& rOwn = *mpMacroTbl;
    const SvxMacroTableDtor& rOther = *pOther;

    return rOwn == rOther;
}

SfxPoolItem* SwFmtINetFmt::Clone( SfxItemPool* ) const
{
    return new SwFmtINetFmt( *this );
}

void SwFmtINetFmt::SetMacroTbl( const SvxMacroTableDtor* pNewTbl )
{
    if( pNewTbl )
    {
        if( mpMacroTbl )
            *mpMacroTbl = *pNewTbl;
        else
            mpMacroTbl = new SvxMacroTableDtor( *pNewTbl );
    }
    else
        delete mpMacroTbl, mpMacroTbl = 0;
}

void SwFmtINetFmt::SetMacro( sal_uInt16 nEvent, const SvxMacro& rMacro )
{
    if( !mpMacroTbl )
        mpMacroTbl = new SvxMacroTableDtor;

    mpMacroTbl->Insert( nEvent, rMacro );
}

const SvxMacro* SwFmtINetFmt::GetMacro( sal_uInt16 nEvent ) const
{
    const SvxMacro* pRet = 0;
    if( mpMacroTbl && mpMacroTbl->IsKeyValid( nEvent ) )
        pRet = mpMacroTbl->Get( nEvent );
    return pRet;
}

bool SwFmtINetFmt::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
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
            OUString sVal = msVisitedFmtName;
            if (sVal.isEmpty() && mnVisitedFmtId != 0)
                SwStyleNameMapper::FillUIName(mnVisitedFmtId, sVal);
            if (!sVal.isEmpty())
                SwStyleNameMapper::FillProgName(sVal, sVal,
                        nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, true);
            rVal <<= sVal;
        }
        break;
        case MID_URL_UNVISITED_FMT:
        {
            OUString sVal = msINetFmtName;
            if (sVal.isEmpty() && mnINetFmtId != 0)
                SwStyleNameMapper::FillUIName(mnINetFmtId, sVal);
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
            pEvents->copyMacrosFromINetFmt(*this);
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
bool SwFmtINetFmt::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId  )
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
            pEvents->copyMacrosIntoINetFmt(*this);
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
                msVisitedFmtName = aString;
                mnVisitedFmtId = SwStyleNameMapper::GetPoolIdFromUIName( msVisitedFmtName,
                                               nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
            }
            break;
            case MID_URL_UNVISITED_FMT:
            {
                OUString sVal;
                rVal >>= sVal;
                OUString aString;
                SwStyleNameMapper::FillUIName( sVal, aString, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, true );
                msINetFmtName = aString;
                mnINetFmtId = SwStyleNameMapper::GetPoolIdFromUIName( msINetFmtName, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
            }
            break;
            default:
                bRet = false;
        }
    }
    return bRet;
}

SwFmtRuby::SwFmtRuby( const OUString& rRubyTxt )
    : SfxPoolItem( RES_TXTATR_CJK_RUBY ),
    sRubyTxt( rRubyTxt ),
    pTxtAttr( 0 ),
    nCharFmtId( 0 ),
    nPosition( 0 ),
    nAdjustment( 0 )
{
}

SwFmtRuby::SwFmtRuby( const SwFmtRuby& rAttr )
    : SfxPoolItem( RES_TXTATR_CJK_RUBY ),
    sRubyTxt( rAttr.sRubyTxt ),
    sCharFmtName( rAttr.sCharFmtName ),
    pTxtAttr( 0 ),
    nCharFmtId( rAttr.nCharFmtId),
    nPosition( rAttr.nPosition ),
    nAdjustment( rAttr.nAdjustment )
{
}

SwFmtRuby::~SwFmtRuby()
{
}

SwFmtRuby& SwFmtRuby::operator=( const SwFmtRuby& rAttr )
{
    sRubyTxt = rAttr.sRubyTxt;
    sCharFmtName = rAttr.sCharFmtName;
    nCharFmtId = rAttr.nCharFmtId;
    nPosition = rAttr.nPosition;
    nAdjustment = rAttr.nAdjustment;
    pTxtAttr =  0;
    return *this;
}

bool SwFmtRuby::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return sRubyTxt == static_cast<const SwFmtRuby&>(rAttr).sRubyTxt &&
           sCharFmtName == static_cast<const SwFmtRuby&>(rAttr).sCharFmtName &&
           nCharFmtId == static_cast<const SwFmtRuby&>(rAttr).nCharFmtId &&
           nPosition == static_cast<const SwFmtRuby&>(rAttr).nPosition &&
           nAdjustment == static_cast<const SwFmtRuby&>(rAttr).nAdjustment;
}

SfxPoolItem* SwFmtRuby::Clone( SfxItemPool* ) const
{
    return new SwFmtRuby( *this );
}

bool SwFmtRuby::QueryValue( uno::Any& rVal,
                            sal_uInt8 nMemberId ) const
{
    bool bRet = true;
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case MID_RUBY_TEXT: rVal <<= sRubyTxt;                    break;
        case MID_RUBY_ADJUST:  rVal <<= (sal_Int16)nAdjustment;    break;
        case MID_RUBY_CHARSTYLE:
        {
            OUString aString;
            SwStyleNameMapper::FillProgName(sCharFmtName, aString, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, true );
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
bool SwFmtRuby::PutValue( const uno::Any& rVal,
                            sal_uInt8 nMemberId  )
{
    bool bRet = true;
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case MID_RUBY_TEXT:
        {
            OUString sTmp;
            bRet = rVal >>= sTmp;
            sRubyTxt = sTmp;
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
            const uno::Type& rType = ::getBooleanCppuType();
            if(rVal.hasValue() && rVal.getValueType() == rType)
            {
                bool bAbove = *(sal_Bool*)rVal.getValue();
                nPosition = bAbove ? 0 : 1;
            }
        }
        break;
        case MID_RUBY_CHARSTYLE:
        {
            OUString sTmp;
            bRet = rVal >>= sTmp;
            if(bRet)
                sCharFmtName = SwStyleNameMapper::GetUIName(sTmp, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
        }
        break;
        default:
            bRet = false;
    }
    return bRet;
}

SwFmtMeta * SwFmtMeta::CreatePoolDefault(const sal_uInt16 i_nWhich)
{
    return new SwFmtMeta(i_nWhich);
}

SwFmtMeta::SwFmtMeta(const sal_uInt16 i_nWhich)
    : SfxPoolItem( i_nWhich )
    , m_pMeta()
    , m_pTxtAttr( 0 )
{
   OSL_ENSURE((RES_TXTATR_META == i_nWhich) || (RES_TXTATR_METAFIELD == i_nWhich),
            "ERROR: SwFmtMeta: invalid which id!");
}

SwFmtMeta::SwFmtMeta( ::boost::shared_ptr< ::sw::Meta > const & i_pMeta,
                        const sal_uInt16 i_nWhich )
    : SfxPoolItem( i_nWhich )
    , m_pMeta( i_pMeta )
    , m_pTxtAttr( 0 )
{
   OSL_ENSURE((RES_TXTATR_META == i_nWhich) || (RES_TXTATR_METAFIELD == i_nWhich),
            "ERROR: SwFmtMeta: invalid which id!");
    OSL_ENSURE(m_pMeta, "SwFmtMeta: no Meta ?");
    // DO NOT call m_pMeta->SetFmtMeta(this) here; only from SetTxtAttr!
}

SwFmtMeta::~SwFmtMeta()
{
    if (m_pMeta && (m_pMeta->GetFmtMeta() == this))
    {
        NotifyChangeTxtNode(0);
        m_pMeta->SetFmtMeta(0);
    }
}

bool SwFmtMeta::operator==( const SfxPoolItem & i_rOther ) const
{
    assert(SfxPoolItem::operator==(i_rOther));
    return SfxPoolItem::operator==( i_rOther )
        && (m_pMeta == static_cast<SwFmtMeta const &>( i_rOther ).m_pMeta);
}

SfxPoolItem * SwFmtMeta::Clone( SfxItemPool * /*pPool*/ ) const
{
    // if this is indeed a copy, then DoCopy must be called later!
    return (m_pMeta) // #i105148# pool default may be cloned also!
        ? new SwFmtMeta( m_pMeta, Which() ) : new SwFmtMeta( Which() );
}

void SwFmtMeta::SetTxtAttr(SwTxtMeta * const i_pTxtAttr)
{
    OSL_ENSURE(!(m_pTxtAttr && i_pTxtAttr),
        "SwFmtMeta::SetTxtAttr: already has text attribute?");
    OSL_ENSURE(  m_pTxtAttr || i_pTxtAttr ,
        "SwFmtMeta::SetTxtAttr: no attribute to remove?");
    m_pTxtAttr = i_pTxtAttr;
    OSL_ENSURE(m_pMeta, "inserted SwFmtMeta has no sw::Meta?");
    // the sw::Meta must be able to find the current text attribute!
    if (m_pMeta)
    {
        if (i_pTxtAttr)
        {
            m_pMeta->SetFmtMeta(this);
        }
        else if (m_pMeta->GetFmtMeta() == this)
        {   // text attribute gone => de-register from text node!
            NotifyChangeTxtNode(0);
            m_pMeta->SetFmtMeta(0);
        }
    }
}

void SwFmtMeta::NotifyChangeTxtNode(SwTxtNode *const pTxtNode)
{
    // N.B.: do not reset m_pTxtAttr here: see call in nodes.cxx,
    // where the hint is not deleted!
    OSL_ENSURE(m_pMeta, "SwFmtMeta::NotifyChangeTxtNode: no Meta?");
    if (m_pMeta && (m_pMeta->GetFmtMeta() == this))
    {   // do not call Modify, that would call SwXMeta::Modify!
        m_pMeta->NotifyChangeTxtNode(pTxtNode);
    }
}

// this SwFmtMeta has been cloned and points at the same sw::Meta as the source
// this method copies the sw::Meta
void SwFmtMeta::DoCopy(::sw::MetaFieldManager & i_rTargetDocManager,
        SwTxtNode & i_rTargetTxtNode)
{
    OSL_ENSURE(m_pMeta, "DoCopy called for SwFmtMeta with no sw::Meta?");
    if (m_pMeta)
    {
        const ::boost::shared_ptr< ::sw::Meta> pOriginal( m_pMeta );
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
        m_pMeta->NotifyChangeTxtNode(& i_rTargetTxtNode);
        // this cannot be done in Clone: a Clone is not necessarily a copy!
        m_pMeta->RegisterAsCopyOf(*pOriginal);
    }
}

namespace sw {

Meta::Meta(SwFmtMeta * const i_pFmt)
    : ::sfx2::Metadatable()
    , SwModify()
    , m_pFmt(i_pFmt)
    , m_pTxtNode(0)
{
}

Meta::~Meta()
{
}

SwTxtMeta * Meta::GetTxtAttr() const
{
    return (m_pFmt) ? m_pFmt->GetTxtAttr() : 0;
}


void Meta::NotifyChangeTxtNodeImpl()
{
    if (m_pTxtNode && (GetRegisteredIn() != m_pTxtNode))
    {
        m_pTxtNode->Add(this);
    }
    else if (!m_pTxtNode && GetRegisteredIn())
    {
        GetRegisteredInNonConst()->Remove(this);
    }
}

void Meta::NotifyChangeTxtNode(SwTxtNode *const pTxtNode)
{
    m_pTxtNode = pTxtNode;
    NotifyChangeTxtNodeImpl();
    if (!pTxtNode) // text node gone? invalidate UNO object!
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
        SetXMeta(uno::Reference<rdf::XMetadatable>(0));
    }
}

// sfx2::Metadatable
::sfx2::IXmlIdRegistry& Meta::GetRegistry()
{
    SwTxtNode * const pTxtNode( GetTxtNode() );
    // GetRegistry may only be called on a meta that is actually in the
    // document, which means it has a pointer to its text node
    OSL_ENSURE(pTxtNode, "ERROR: GetRegistry: no text node?");
    if (!pTxtNode)
        throw uno::RuntimeException();
    return pTxtNode->GetRegistry();
}

bool Meta::IsInClipboard() const
{
    const SwTxtNode * const pTxtNode( GetTxtNode() );
// no text node: in UNDO  OSL_ENSURE(pTxtNode, "IsInClipboard: no text node?");
    return pTxtNode && pTxtNode->IsInClipboard();
}

bool Meta::IsInUndo() const
{
    const SwTxtNode * const pTxtNode( GetTxtNode() );
// no text node: in UNDO  OSL_ENSURE(pTxtNode, "IsInUndo: no text node?");
    return (pTxtNode) ? pTxtNode->IsInUndo() : true;
}

bool Meta::IsInContent() const
{
    const SwTxtNode * const pTxtNode( GetTxtNode() );
    OSL_ENSURE(pTxtNode, "IsInContent: no text node?");
    return (pTxtNode) ? pTxtNode->IsInContent() : true;
}

::com::sun::star::uno::Reference< ::com::sun::star::rdf::XMetadatable >
Meta::MakeUnoObject()
{
    return SwXMeta::CreateXMeta(*this);
}

MetaField::MetaField(SwFmtMeta * const i_pFmt,
            const sal_uInt32 nNumberFormat, const bool bIsFixedLanguage)
    : Meta(i_pFmt)
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
            SwTxtNode * const pTxtNode( GetTxtNode() );
            SwDocShell const * const pShell(pTxtNode->GetDoc()->GetDocShell());
            const uno::Reference<frame::XModel> xModel(
                (pShell) ? pShell->GetModel() : 0,  uno::UNO_SET_THROW);
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
    SwTxtNode * const pTxtNode( GetTxtNode() );
    if (pTxtNode)
    {
        SvNumberFormatter *const pNumberFormatter(
                pTxtNode->GetDoc()->GetNumberFormatter() );
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

::boost::shared_ptr<MetaField>
MetaFieldManager::makeMetaField(SwFmtMeta * const i_pFmt,
        const sal_uInt32 nNumberFormat, const bool bIsFixedLanguage)
{
    const ::boost::shared_ptr<MetaField> pMetaField(
        new MetaField(i_pFmt, nNumberFormat, bIsFixedLanguage) );
    m_MetaFields.push_back(pMetaField);
    return pMetaField;
}

struct IsInUndo
{
    bool operator()(::boost::weak_ptr<MetaField> const & pMetaField) {
        return pMetaField.lock()->IsInUndo();
    }
};

struct MakeUnoObject
{
    uno::Reference<text::XTextField>
    operator()(::boost::weak_ptr<MetaField> const & pMetaField) {
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
            ::boost::bind(&::boost::weak_ptr<MetaField>::expired, _1)));
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

} // namespace sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

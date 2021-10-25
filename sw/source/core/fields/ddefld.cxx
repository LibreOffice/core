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

#include <sal/config.h>

#include <o3tl/any.hxx>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <rtl/ustrbuf.hxx>
#include <sfx2/linkmgr.hxx>
#include <sot/exchange.hxx>
#include <doc.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <editsh.hxx>
#include <fmtfld.hxx>
#include <ddefld.hxx>
#include <swddetbl.hxx>
#include <swbaslnk.hxx>
#include <unofldmid.h>
#include <hints.hxx>
#include <calbck.hxx>

using namespace ::com::sun::star;

#define DDE_TXT_ENCODING    osl_getThreadTextEncoding()

namespace {

class SwIntrnlRefLink : public SwBaseLink
{
    SwDDEFieldType& m_rFieldType;

public:
    SwIntrnlRefLink(SwDDEFieldType& rType, SfxLinkUpdateMode nUpdateType)
        : SwBaseLink(nUpdateType, SotClipboardFormatId::STRING)
        , m_rFieldType(rType)
    {}

    virtual void Closed() override;
    virtual ::sfx2::SvBaseLink::UpdateResult DataChanged(
        const OUString& rMimeType, const css::uno::Any & rValue ) override;

    virtual const SwNode* GetAnchor() const override;
    virtual bool IsInRange( sal_uLong nSttNd, sal_uLong nEndNd ) const override;
};

}

::sfx2::SvBaseLink::UpdateResult SwIntrnlRefLink::DataChanged( const OUString& rMimeType,
                                const uno::Any & rValue )
{
    switch( SotExchange::GetFormatIdFromMimeType( rMimeType ) )
    {
    case SotClipboardFormatId::STRING:
        if( !IsNoDataFlag() )
        {
            uno::Sequence< sal_Int8 > aSeq;
            rValue >>= aSeq;
            OUString sStr( reinterpret_cast<char const *>(aSeq.getConstArray()), aSeq.getLength(), DDE_TXT_ENCODING );

            // remove not needed CR-LF at the end
            sal_Int32 n = sStr.getLength();
            while( n && 0 == sStr[ n-1 ] )
                --n;
            if( n && 0x0a == sStr[ n-1 ] )
                --n;
            if( n && 0x0d == sStr[ n-1 ] )
                --n;

            bool bDel = n != sStr.getLength();
            if( bDel )
                sStr = sStr.copy( 0, n );

            m_rFieldType.SetExpansion(sStr);
            // set Expansion first! (otherwise this flag will be deleted)
            m_rFieldType.SetCRLFDelFlag(bDel);
        }
        break;

    // other formats
    default:
        return SUCCESS;
    }

    OSL_ENSURE(m_rFieldType.GetDoc(), "no pDoc");

    // no dependencies left?
    if (!m_rFieldType.IsModifyLocked() && !ChkNoDataFlag())
    {
        SwViewShell* pSh = m_rFieldType.GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
        SwEditShell* pESh = m_rFieldType.GetDoc()->GetEditShell();

        // Search for fields. If no valid found, disconnect.
        SwMsgPoolItem aUpdateDDE( RES_UPDATEDDETBL );
        m_rFieldType.LockModify();

        std::vector<SwFormatField*> vFields;
        std::vector<SwDDETable*> vTables;
        m_rFieldType.GatherFields(vFields, false);
        m_rFieldType.GatherDdeTables(vTables);
        const bool bDoAction = vFields.size() || vTables.size();
        if(bDoAction)
        {
            if(pESh)
                pESh->StartAllAction();
            else if(pSh)
                pSh->StartAction();
        }

        // DDE fields attribute in the text
        for(auto pFormatField: vFields)
        {
            if(pFormatField->GetTextField())
                pFormatField->UpdateTextNode( nullptr, &aUpdateDDE );
        }
        // a DDE tables in the text
        for(auto pTable: vTables)
            pTable->ChangeContent();

        m_rFieldType.UnlockModify();

        if(bDoAction)
        {
            if(pESh)
                pESh->EndAllAction();
            else if(pSh)
                pSh->EndAction();

            if(pSh)
                pSh->GetDoc()->getIDocumentState().SetModified();
        }
    }

    return SUCCESS;
}

void SwIntrnlRefLink::Closed()
{
    if (m_rFieldType.GetDoc() && !m_rFieldType.GetDoc()->IsInDtor())
    {
        // advise goes, convert all fields into text?
        SwViewShell* pSh = m_rFieldType.GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
        SwEditShell* pESh = m_rFieldType.GetDoc()->GetEditShell();
        if( pESh )
        {
            pESh->StartAllAction();
            pESh->FieldToText(&m_rFieldType);
            pESh->EndAllAction();
        }
        else
        {
            pSh->StartAction();
            // to call at the doc ??
            pSh->EndAction();
        }
    }
    SvBaseLink::Closed();
}

sw::LinkAnchorSearchHint::~LinkAnchorSearchHint() {};

const SwNode* SwIntrnlRefLink::GetAnchor() const
{
    // here, any anchor of the normal NodesArray should be sufficient
    const SwNode* pNd = nullptr;
    m_rFieldType.CallSwClientNotify(
        sw::LinkAnchorSearchHint(m_rFieldType.GetDoc()->GetNodes(), pNd));
    return pNd;
}

bool SwIntrnlRefLink::IsInRange( sal_uLong nSttNd, sal_uLong nEndNd ) const
{
    bool bInRange = false;
    m_rFieldType.CallSwClientNotify(sw::InRangeSearchHint(nSttNd, nEndNd, bInRange));
    return bInRange;
}

SwDDEFieldType::SwDDEFieldType(const OUString& rName,
                               const OUString& rCmd, SfxLinkUpdateMode nUpdateType )
    : SwFieldType( SwFieldIds::Dde ),
    m_aName( rName ), m_pDoc( nullptr ), m_nRefCount( 0 )
{
    m_bCRLFFlag = m_bDeleted = false;
    m_RefLink = new SwIntrnlRefLink( *this, nUpdateType );
    SetCmd( rCmd );
}

SwDDEFieldType::~SwDDEFieldType()
{
    if( m_pDoc && !m_pDoc->IsInDtor() )
        m_pDoc->getIDocumentLinksAdministration().GetLinkManager().Remove( m_RefLink.get() );
    m_RefLink->Disconnect();
}

std::unique_ptr<SwFieldType> SwDDEFieldType::Copy() const
{
    std::unique_ptr<SwDDEFieldType> pType(new SwDDEFieldType( m_aName, GetCmd(), GetType() ));
    pType->m_aExpansion = m_aExpansion;
    pType->m_bCRLFFlag = m_bCRLFFlag;
    pType->m_bDeleted = m_bDeleted;
    pType->SetDoc( m_pDoc );
    return pType;
}

OUString SwDDEFieldType::GetName() const
{
    return m_aName;
}

void SwDDEFieldType::SetCmd( const OUString& _aStr )
{
    OUString aStr = _aStr;
    sal_Int32 nIndex = 0;
    do
    {
        aStr = aStr.replaceFirst("  ", " ", &nIndex);
    } while (nIndex>=0);
    m_RefLink->SetLinkSourceName( aStr );
}

OUString const & SwDDEFieldType::GetCmd() const
{
    return m_RefLink->GetLinkSourceName();
}

void SwDDEFieldType::SetDoc( SwDoc* pNewDoc )
{
    if( pNewDoc == m_pDoc )
        return;

    if( m_pDoc && m_RefLink.is() )
    {
        OSL_ENSURE( !m_nRefCount, "How do we get the references?" );
        m_pDoc->getIDocumentLinksAdministration().GetLinkManager().Remove( m_RefLink.get() );
    }

    m_pDoc = pNewDoc;
    if( m_pDoc && m_nRefCount )
    {
        m_RefLink->SetVisible( m_pDoc->getIDocumentLinksAdministration().IsVisibleLinks() );
        m_pDoc->getIDocumentLinksAdministration().GetLinkManager().InsertDDELink( m_RefLink.get() );
    }
}

void SwDDEFieldType::RefCntChgd()
{
    if( m_nRefCount )
    {
        m_RefLink->SetVisible( m_pDoc->getIDocumentLinksAdministration().IsVisibleLinks() );
        m_pDoc->getIDocumentLinksAdministration().GetLinkManager().InsertDDELink( m_RefLink.get() );
        if( m_pDoc->getIDocumentLayoutAccess().GetCurrentViewShell() )
            m_RefLink->Update();
    }
    else
    {
        Disconnect();
        m_pDoc->getIDocumentLinksAdministration().GetLinkManager().Remove( m_RefLink.get() );
    }
}

void SwDDEFieldType::QueryValue( uno::Any& rVal, sal_uInt16 nWhichId ) const
{
    sal_Int32 nPart = -1;
    switch( nWhichId )
    {
    case FIELD_PROP_PAR2:      nPart = 2; break;
    case FIELD_PROP_PAR4:      nPart = 1; break;
    case FIELD_PROP_SUBTYPE:   nPart = 0; break;
    case FIELD_PROP_BOOL1:
        rVal <<= GetType() == SfxLinkUpdateMode::ALWAYS;
        break;
    case FIELD_PROP_PAR5:
        rVal <<= m_aExpansion;
        break;
    default:
        assert(false);
    }
    if ( nPart>=0 )
        rVal <<= GetCmd().getToken(nPart, sfx2::cTokenSeparator);
}

void SwDDEFieldType::PutValue( const uno::Any& rVal, sal_uInt16 nWhichId )
{
    sal_Int32 nPart = -1;
    switch( nWhichId )
    {
    case FIELD_PROP_PAR2:      nPart = 2; break;
    case FIELD_PROP_PAR4:      nPart = 1; break;
    case FIELD_PROP_SUBTYPE:   nPart = 0; break;
    case FIELD_PROP_BOOL1:
        SetType( *o3tl::doAccess<bool>(rVal) ?
                 SfxLinkUpdateMode::ALWAYS :
                 SfxLinkUpdateMode::ONCALL );
        break;
    case FIELD_PROP_PAR5:
        rVal >>= m_aExpansion;
        break;
    default:
        assert(false);
    }
    if( nPart<0 )
        return;

    const OUString sOldCmd( GetCmd() );
    OUStringBuffer sNewCmd;
    sal_Int32 nIndex = 0;
    for (sal_Int32 i=0; i<3; ++i)
    {
        OUString sToken = sOldCmd.getToken(0, sfx2::cTokenSeparator, nIndex);
        if (i==nPart)
        {
            rVal >>= sToken;
        }
        sNewCmd.append((i < 2)
            ? sToken + OUStringChar(sfx2::cTokenSeparator) : sToken);
    }
    SetCmd( sNewCmd.makeStringAndClear() );
}

SwDDEField::SwDDEField( SwDDEFieldType* pInitType )
    : SwField(pInitType)
{
}

SwDDEField::~SwDDEField()
{
    if( GetTyp()->HasOnlyOneListener() )
        static_cast<SwDDEFieldType*>(GetTyp())->Disconnect();
}

OUString SwDDEField::ExpandImpl(SwRootFrame const*const) const
{
    OUString aStr = static_cast<SwDDEFieldType*>(GetTyp())->GetExpansion();
    aStr = aStr.replaceAll("\r", "");
    aStr = aStr.replaceAll("\t", " ");
    aStr = aStr.replaceAll("\n", "|");
    if (aStr.endsWith("|"))
    {
        return aStr.copy(0, aStr.getLength()-1);
    }
    return aStr;
}

std::unique_ptr<SwField> SwDDEField::Copy() const
{
    return std::make_unique<SwDDEField>(static_cast<SwDDEFieldType*>(GetTyp()));
}

/// get field type name
OUString SwDDEField::GetPar1() const
{
    return static_cast<const SwDDEFieldType*>(GetTyp())->GetName();
}

/// get field type command
OUString SwDDEField::GetPar2() const
{
    return static_cast<const SwDDEFieldType*>(GetTyp())->GetCmd();
}

/// set field type command
void SwDDEField::SetPar2(const OUString& rStr)
{
    static_cast<SwDDEFieldType*>(GetTyp())->SetCmd(rStr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

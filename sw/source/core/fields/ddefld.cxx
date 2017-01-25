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
#include <osl/thread.h>
#include <sfx2/linkmgr.hxx>
#include <doc.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <editsh.hxx>
#include <ndtxt.hxx>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <ddefld.hxx>
#include <swtable.hxx>
#include <swbaslnk.hxx>
#include <swddetbl.hxx>
#include <unofldmid.h>
#include <hints.hxx>
#include <calbck.hxx>

using namespace ::com::sun::star;

#define DDE_TXT_ENCODING    osl_getThreadTextEncoding()

class SwIntrnlRefLink : public SwBaseLink
{
    SwDDEFieldType& rFieldType;
public:
    SwIntrnlRefLink( SwDDEFieldType& rType, SfxLinkUpdateMode nUpdateType )
        : SwBaseLink( nUpdateType, SotClipboardFormatId::STRING ),
        rFieldType( rType )
    {}

    virtual void Closed() override;
    virtual ::sfx2::SvBaseLink::UpdateResult DataChanged(
        const OUString& rMimeType, const css::uno::Any & rValue ) override;

    virtual const SwNode* GetAnchor() const override;
    virtual bool IsInRange( sal_uLong nSttNd, sal_uLong nEndNd, sal_Int32 nStt = 0,
                            sal_Int32 nEnd = -1 ) const override;
};

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

            rFieldType.SetExpansion( sStr );
            // set Expansion first! (otherwise this flag will be deleted)
            rFieldType.SetCRLFDelFlag( bDel );
        }
        break;

    // other formats
    default:
        return SUCCESS;
    }

    OSL_ENSURE( rFieldType.GetDoc(), "no pDoc" );

    // no dependencies left?
    if( rFieldType.HasWriterListeners() && !rFieldType.IsModifyLocked() && !ChkNoDataFlag() )
    {
        SwViewShell* pSh = rFieldType.GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
        SwEditShell* pESh = rFieldType.GetDoc()->GetEditShell();

        // Search for fields. If no valid found, disconnect.
        SwMsgPoolItem aUpdateDDE( RES_UPDATEDDETBL );
        bool bCallModify = false;
        rFieldType.LockModify();

        SwIterator<SwClient,SwFieldType> aIter(rFieldType);
        for(SwClient* pLast = aIter.First(); pLast; pLast = aIter.Next())
        {
            // a DDE table or a DDE field attribute in the text
            if( dynamic_cast<const SwFormatField *>(pLast) == nullptr ||
                static_cast<SwFormatField*>(pLast)->GetTextField() )
            {
                if( !bCallModify )
                {
                    if( pESh )
                        pESh->StartAllAction();
                    else if( pSh )
                        pSh->StartAction();
                }
                pLast->ModifyNotification( nullptr, &aUpdateDDE );
                bCallModify = true;
            }
        }

        rFieldType.UnlockModify();

        if( bCallModify )
        {
            if( pESh )
                pESh->EndAllAction();
            else if( pSh )
                pSh->EndAction();

            if( pSh )
                pSh->GetDoc()->getIDocumentState().SetModified();
        }
    }

    return SUCCESS;
}

void SwIntrnlRefLink::Closed()
{
    if( rFieldType.GetDoc() && !rFieldType.GetDoc()->IsInDtor() )
    {
        // advise goes, convert all fields into text?
        SwViewShell* pSh = rFieldType.GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
        SwEditShell* pESh = rFieldType.GetDoc()->GetEditShell();
        if( pESh )
        {
            pESh->StartAllAction();
            pESh->FieldToText( &rFieldType );
            pESh->EndAllAction();
        }
        else
        {
            pSh->StartAction();
            // am Doc aufrufen ??
            pSh->EndAction();
        }
    }
    SvBaseLink::Closed();
}

const SwNode* SwIntrnlRefLink::GetAnchor() const
{
    // here, any anchor of the normal NodesArray should be sufficient
    const SwNode* pNd = nullptr;
    SwIterator<SwClient,SwFieldType> aIter(rFieldType);
    for(SwClient* pLast = aIter.First(); pLast; pLast = aIter.Next())
    {
        // a DDE table or a DDE field attribute in the text
        if( dynamic_cast<const SwFormatField *>(pLast) == nullptr)
        {
            SwDepend* pDep = static_cast<SwDepend*>(pLast);
            SwDDETable* pDDETable = static_cast<SwDDETable*>(pDep->GetToTell());
            pNd = pDDETable->GetTabSortBoxes()[0]->GetSttNd();
        }
        else if( static_cast<SwFormatField*>(pLast)->GetTextField() )
            pNd = static_cast<SwFormatField*>(pLast)->GetTextField()->GetpTextNode();

        if( pNd && &rFieldType.GetDoc()->GetNodes() == &pNd->GetNodes() )
            break;
        pNd = nullptr;
    }
    return pNd;
}

bool SwIntrnlRefLink::IsInRange( sal_uLong nSttNd, sal_uLong nEndNd,
                                sal_Int32 nStt, sal_Int32 nEnd ) const
{
    // here, any anchor of the normal NodesArray should be sufficient
    SwNodes* pNds = &rFieldType.GetDoc()->GetNodes();
    SwIterator<SwClient,SwFieldType> aIter(rFieldType);
    for(SwClient* pLast = aIter.First(); pLast; pLast = aIter.Next())
    {
        // a DDE table or a DDE field attribute in the text
        if( dynamic_cast<const SwFormatField *>(pLast) == nullptr)
        {
            SwDepend* pDep = static_cast<SwDepend*>(pLast);
            SwDDETable* pDDETable = static_cast<SwDDETable*>(pDep->GetToTell());
            const SwTableNode* pTableNd = pDDETable->GetTabSortBoxes()[0]->
                            GetSttNd()->FindTableNode();
            if( pTableNd->GetNodes().IsDocNodes() &&
                nSttNd < pTableNd->EndOfSectionIndex() &&
                nEndNd > pTableNd->GetIndex() )
                return true;
        }
        else if( static_cast<SwFormatField*>(pLast)->GetTextField() )
        {
            const SwTextField* pTField = static_cast<SwFormatField*>(pLast)->GetTextField();
            const SwTextNode* pNd = pTField->GetpTextNode();
            if( pNd && pNds == &pNd->GetNodes() )
            {
                sal_uLong nNdPos = pNd->GetIndex();
                if( nSttNd <= nNdPos && nNdPos <= nEndNd &&
                    ( nNdPos != nSttNd || pTField->GetStart() >= nStt ) &&
                    ( nNdPos != nEndNd || pTField->GetStart() < nEnd ))
                    return true;
            }
        }
    }

    return false;
}

SwDDEFieldType::SwDDEFieldType(const OUString& rName,
                               const OUString& rCmd, SfxLinkUpdateMode nUpdateType )
    : SwFieldType( RES_DDEFLD ),
    aName( rName ), pDoc( nullptr ), nRefCnt( 0 )
{
    bCRLFFlag = bDeleted = false;
    refLink = new SwIntrnlRefLink( *this, nUpdateType );
    SetCmd( rCmd );
}

SwDDEFieldType::~SwDDEFieldType()
{
    if( pDoc && !pDoc->IsInDtor() )
        pDoc->getIDocumentLinksAdministration().GetLinkManager().Remove( refLink.get() );
    refLink->Disconnect();
}

SwFieldType* SwDDEFieldType::Copy() const
{
    SwDDEFieldType* pType = new SwDDEFieldType( aName, GetCmd(), GetType() );
    pType->aExpansion = aExpansion;
    pType->bCRLFFlag = bCRLFFlag;
    pType->bDeleted = bDeleted;
    pType->SetDoc( pDoc );
    return pType;
}

OUString SwDDEFieldType::GetName() const
{
    return aName;
}

void SwDDEFieldType::SetCmd( const OUString& _aStr )
{
    OUString aStr = _aStr;
    sal_Int32 nIndex = 0;
    do
    {
        aStr = aStr.replaceFirst("  ", " ", &nIndex);
    } while (nIndex>=0);
    refLink->SetLinkSourceName( aStr );
}

OUString SwDDEFieldType::GetCmd() const
{
    return refLink->GetLinkSourceName();
}

void SwDDEFieldType::SetDoc( SwDoc* pNewDoc )
{
    if( pNewDoc == pDoc )
        return;

    if( pDoc && refLink.is() )
    {
        OSL_ENSURE( !nRefCnt, "How do we get the references?" );
        pDoc->getIDocumentLinksAdministration().GetLinkManager().Remove( refLink.get() );
    }

    pDoc = pNewDoc;
    if( pDoc && nRefCnt )
    {
        refLink->SetVisible( pDoc->getIDocumentLinksAdministration().IsVisibleLinks() );
        pDoc->getIDocumentLinksAdministration().GetLinkManager().InsertDDELink( refLink.get() );
    }
}

void SwDDEFieldType::RefCntChgd()
{
    if( nRefCnt )
    {
        refLink->SetVisible( pDoc->getIDocumentLinksAdministration().IsVisibleLinks() );
        pDoc->getIDocumentLinksAdministration().GetLinkManager().InsertDDELink( refLink.get() );
        if( pDoc->getIDocumentLayoutAccess().GetCurrentViewShell() )
            refLink->Update();
    }
    else
    {
        Disconnect();
        pDoc->getIDocumentLinksAdministration().GetLinkManager().Remove( refLink.get() );
    }
}

bool SwDDEFieldType::QueryValue( uno::Any& rVal, sal_uInt16 nWhichId ) const
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
        rVal <<= aExpansion;
        break;
    default:
        assert(false);
    }
    if ( nPart>=0 )
        rVal <<= GetCmd().getToken(nPart, sfx2::cTokenSeparator);
    return true;
}

bool SwDDEFieldType::PutValue( const uno::Any& rVal, sal_uInt16 nWhichId )
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
        rVal >>= aExpansion;
        break;
    default:
        assert(false);
    }
    if( nPart>=0 )
    {
        const OUString sOldCmd( GetCmd() );
        OUString sNewCmd;
        sal_Int32 nIndex = 0;
        for (sal_Int32 i=0; i<3; ++i)
        {
            OUString sToken = sOldCmd.getToken(0, sfx2::cTokenSeparator, nIndex);
            if (i==nPart)
            {
                rVal >>= sToken;
            }
            sNewCmd += (i < 2)
                ? sToken + OUStringLiteral1(sfx2::cTokenSeparator) : sToken;
        }
        SetCmd( sNewCmd );
    }
    return true;
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

OUString SwDDEField::Expand() const
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

SwField* SwDDEField::Copy() const
{
    return new SwDDEField(static_cast<SwDDEFieldType*>(GetTyp()));
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

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

#include <memory>
#include <hintids.hxx>
#include <comphelper/string.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/langitem.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <o3tl/any.hxx>
#include <swtypes.hxx>
#include <strings.hrc>
#include <authfld.hxx>
#include <expfld.hxx>
#include <pam.hxx>
#include <cntfrm.hxx>
#include <rootfrm.hxx>
#include <tox.hxx>
#include <txmsrt.hxx>
#include <doctxm.hxx>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <unofldmid.h>
#include <unoprnms.hxx>
#include <calbck.hxx>
#include <unomid.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

SwAuthEntry::SwAuthEntry(const SwAuthEntry& rCopy)
    : SimpleReferenceObject()
{
    for(int i = 0; i < AUTH_FIELD_END; ++i)
        aAuthFields[i] = rCopy.aAuthFields[i];
}

bool    SwAuthEntry::operator==(const SwAuthEntry& rComp)
{
    for(int i = 0; i < AUTH_FIELD_END; ++i)
        if(aAuthFields[i] != rComp.aAuthFields[i])
            return false;
    return true;
}

SwAuthorityFieldType::SwAuthorityFieldType(SwDoc* pDoc)
    : SwFieldType( SwFieldIds::TableOfAuthorities ),
    m_pDoc(pDoc),
    m_cPrefix('['),
    m_cSuffix(']'),
    m_bIsSequence(false),
    m_bSortByDocument(true),
    m_eLanguage(::GetAppLanguage())
{
}

SwAuthorityFieldType::~SwAuthorityFieldType()
{
}

SwFieldType*    SwAuthorityFieldType::Copy()  const
{
    return new SwAuthorityFieldType(m_pDoc);
}

void SwAuthorityFieldType::RemoveField(const SwAuthEntry* nHandle)
{
    for(SwAuthDataArr::size_type j = 0; j < m_DataArr.size(); ++j)
    {
        if(m_DataArr[j].get() == nHandle)
        {
            m_DataArr.erase(m_DataArr.begin() + j);
            //re-generate positions of the fields
            DelSequenceArray();
            return;
        }
    }
    OSL_FAIL("Field unknown" );
}

SwAuthEntry* SwAuthorityFieldType::AddField(const OUString& rFieldContents)
{
    rtl::Reference<SwAuthEntry> pEntry(new SwAuthEntry);
    sal_Int32 nIdx{ 0 };
    for( sal_Int32 i = 0; i < AUTH_FIELD_END; ++i )
        pEntry->SetAuthorField( static_cast<ToxAuthorityField>(i),
                        rFieldContents.getToken( 0, TOX_STYLE_DELIMITER, nIdx ));

    for (auto &rpTemp : m_DataArr)
    {
        if (*rpTemp == *pEntry)
        {
            return rpTemp.get();
        }
    }

    //if it is a new Entry - insert
    m_DataArr.push_back(std::move(pEntry));
    //re-generate positions of the fields
    DelSequenceArray();
    return m_DataArr.back().get();
}

void SwAuthorityFieldType::GetAllEntryIdentifiers(
    std::vector<OUString>& rToFill )const
{
    for (const auto & rpTemp : m_DataArr)
    {
        rToFill.push_back(rpTemp->GetAuthorField(AUTH_FIELD_IDENTIFIER));
    }
}

SwAuthEntry* SwAuthorityFieldType::GetEntryByIdentifier(
                                const OUString& rIdentifier)const
{
    for (const auto &rpTemp : m_DataArr)
    {
        if (rIdentifier == rpTemp->GetAuthorField(AUTH_FIELD_IDENTIFIER))
        {
            return rpTemp.get();
        }
    }
    return nullptr;
}

bool SwAuthorityFieldType::ChangeEntryContent(const SwAuthEntry* pNewEntry)
{
    for (auto &rpTemp : m_DataArr)
    {
        if (rpTemp->GetAuthorField(AUTH_FIELD_IDENTIFIER) ==
                    pNewEntry->GetAuthorField(AUTH_FIELD_IDENTIFIER))
        {
            for(int i = 0; i < AUTH_FIELD_END; ++i)
            {
                rpTemp->SetAuthorField(static_cast<ToxAuthorityField>(i),
                    pNewEntry->GetAuthorField(static_cast<ToxAuthorityField>(i)));
            }
            return true;
        }
    }
    return false;
}

/// appends a new entry (if new) and returns the array position
sal_uInt16  SwAuthorityFieldType::AppendField( const SwAuthEntry& rInsert )
{
    for( SwAuthDataArr::size_type nRet = 0; nRet < m_DataArr.size(); ++nRet )
    {
        if( *m_DataArr[ nRet ] == rInsert )
            return nRet;
    }

    //if it is a new Entry - insert
    m_DataArr.push_back(new SwAuthEntry(rInsert));
    return m_DataArr.size()-1;
}

sal_uInt16 SwAuthorityFieldType::GetSequencePos(const SwAuthEntry* pAuthEntry,
        SwRootFrame const*const pLayout)
{
    //find the field in a sorted array of handles,
#if OSL_DEBUG_LEVEL > 0
    bool bCurrentFieldWithoutTextNode = false;
#endif
    if(!m_SequArr.empty() && m_SequArr.size() != m_DataArr.size())
        DelSequenceArray();
    if(m_SequArr.empty())
    {
        IDocumentRedlineAccess const& rIDRA(m_pDoc->getIDocumentRedlineAccess());
        // sw_redlinehide: need 2 arrays because the sorting may be different,
        // if multiple fields refer to the same entry and first one is deleted
        std::vector<std::unique_ptr<SwTOXSortTabBase>> aSortArr;
        std::vector<std::unique_ptr<SwTOXSortTabBase>> aSortArrRLHidden;
        SwIterator<SwFormatField,SwFieldType> aIter( *this );

        SwTOXInternational aIntl(m_eLanguage, SwTOIOptions::NONE, m_sSortAlgorithm);

        for( SwFormatField* pFormatField = aIter.First(); pFormatField; pFormatField = aIter.Next() )
        {
            const SwTextField* pTextField = pFormatField->GetTextField();
            if(!pTextField || !pTextField->GetpTextNode())
            {
#if OSL_DEBUG_LEVEL > 0
                if(pAuthEntry == static_cast<SwAuthorityField*>(pFormatField->GetField())->GetAuthEntry())
                    bCurrentFieldWithoutTextNode = true;
#endif
                continue;
            }
            const SwTextNode& rFieldTextNode = pTextField->GetTextNode();
            SwPosition aFieldPos(rFieldTextNode);
            SwDoc& rDoc = *const_cast<SwDoc*>(rFieldTextNode.GetDoc());
            SwContentFrame *pFrame = rFieldTextNode.getLayoutFrame( rDoc.getIDocumentLayoutAccess().GetCurrentLayout() );
            const SwTextNode* pTextNode = nullptr;
            if(pFrame && !pFrame->IsInDocBody())
                pTextNode = GetBodyTextNode( rDoc, aFieldPos, *pFrame );
            //if no text node could be found or the field is in the document
            //body the directly available text node will be used
            if(!pTextNode)
                pTextNode = &rFieldTextNode;
            if (pTextNode->GetText().isEmpty()
                || !pTextNode->getLayoutFrame(rDoc.getIDocumentLayoutAccess().GetCurrentLayout())
                || !pTextNode->GetNodes().IsDocNodes())
            {
                continue;
            }
            auto const InsertImpl = [&aIntl, pTextNode, pFormatField]
                (std::vector<std::unique_ptr<SwTOXSortTabBase>> & rSortArr)
            {
                std::unique_ptr<SwTOXAuthority> pNew(
                    new SwTOXAuthority(*pTextNode, *pFormatField, aIntl));

                for (size_t i = 0; i < rSortArr.size(); ++i)
                {
                    SwTOXSortTabBase* pOld = rSortArr[i].get();
                    if (pOld->equivalent(*pNew))
                    {
                        //only the first occurrence in the document
                        //has to be in the array
                        if (pOld->sort_lt(*pNew))
                            pNew.reset();
                        else // remove the old content
                            rSortArr.erase(rSortArr.begin() + i);
                        break;
                    }
                }
                //if it still exists - insert at the correct position
                if (pNew)
                {
                    size_t j {0};

                    while (j < rSortArr.size())
                    {
                        SwTOXSortTabBase* pOld = rSortArr[j].get();
                        if (pNew->sort_lt(*pOld))
                            break;
                        ++j;
                    }
                    rSortArr.insert(rSortArr.begin() + j, std::move(pNew));
                }
            };
            InsertImpl(aSortArr);
            if (!sw::IsFieldDeletedInModel(rIDRA, *pTextField))
            {
                InsertImpl(aSortArrRLHidden);
            }
        }

        for(auto & pBase : aSortArr)
        {
            SwFormatField& rFormatField = static_cast<SwTOXAuthority&>(*pBase).GetFieldFormat();
            SwAuthorityField* pAField = static_cast<SwAuthorityField*>(rFormatField.GetField());
            m_SequArr.push_back(pAField->GetAuthEntry());
        }
        for (auto & pBase : aSortArrRLHidden)
        {
            SwFormatField& rFormatField = static_cast<SwTOXAuthority&>(*pBase).GetFieldFormat();
            SwAuthorityField* pAField = static_cast<SwAuthorityField*>(rFormatField.GetField());
            m_SequArrRLHidden.push_back(pAField->GetAuthEntry());
        }
    }
    //find nHandle
    auto const& rSequArr(pLayout && pLayout->IsHideRedlines() ? m_SequArrRLHidden : m_SequArr);
    for (std::vector<sal_IntPtr>::size_type i = 0; i < rSequArr.size(); ++i)
    {
        if (rSequArr[i] == pAuthEntry)
        {
            return i + 1;
        }
    }
#if OSL_DEBUG_LEVEL > 0
    OSL_ENSURE(bCurrentFieldWithoutTextNode, "Handle not found");
#endif
    return 0;
}

void SwAuthorityFieldType::QueryValue( Any& rVal, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
    case FIELD_PROP_PAR2:
        {
            OUString sVal;
            sal_Unicode uRet = FIELD_PROP_PAR1 == nWhichId ? m_cPrefix : m_cSuffix;
            if(uRet)
                sVal = OUString(uRet);
            rVal <<= sVal;
        }
        break;
    case FIELD_PROP_PAR3:
        rVal <<= GetSortAlgorithm();
        break;

    case FIELD_PROP_BOOL1:
        rVal <<= m_bIsSequence;
        break;

    case FIELD_PROP_BOOL2:
        rVal <<= m_bSortByDocument;
        break;

    case FIELD_PROP_LOCALE:
        rVal <<= LanguageTag(GetLanguage()).getLocale();
        break;

    case FIELD_PROP_PROP_SEQ:
        {
            Sequence<PropertyValues> aRet(m_SortKeyArr.size());
            PropertyValues* pValues = aRet.getArray();
            for(SortKeyArr::size_type i = 0; i < m_SortKeyArr.size(); ++i)
            {
                const SwTOXSortKey* pKey = &m_SortKeyArr[i];
                pValues[i].realloc(2);
                PropertyValue* pValue = pValues[i].getArray();
                pValue[0].Name = UNO_NAME_SORT_KEY;
                pValue[0].Value <<= sal_Int16(pKey->eField);
                pValue[1].Name = UNO_NAME_IS_SORT_ASCENDING;
                pValue[1].Value <<= pKey->bSortAscending;
            }
            rVal <<= aRet;
        }
        break;
    default:
        assert(false);
    }
}

void SwAuthorityFieldType::PutValue( const Any& rAny, sal_uInt16 nWhichId )
{
    bool bRet = true;
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
    case FIELD_PROP_PAR2:
    {
        OUString sTmp;
        rAny >>= sTmp;
        const sal_Unicode uSet = !sTmp.isEmpty() ? sTmp[0] : 0;
        if( FIELD_PROP_PAR1 == nWhichId )
            m_cPrefix = uSet;
        else
            m_cSuffix = uSet;
    }
    break;
    case FIELD_PROP_PAR3:
    {
        OUString sTmp;
        rAny >>= sTmp;
        SetSortAlgorithm(sTmp);
        break;
    }
    case FIELD_PROP_BOOL1:
        m_bIsSequence = *o3tl::doAccess<bool>(rAny);
        break;
    case FIELD_PROP_BOOL2:
        m_bSortByDocument = *o3tl::doAccess<bool>(rAny);
        break;

    case FIELD_PROP_LOCALE:
        {
            css::lang::Locale aLocale;
            bRet = rAny >>= aLocale;
            if( bRet )
                SetLanguage( LanguageTag::convertToLanguageType( aLocale ));
        }
        break;

    case FIELD_PROP_PROP_SEQ:
        {
            Sequence<PropertyValues> aSeq;
            bRet = rAny >>= aSeq;
            if( bRet )
            {
                m_SortKeyArr.clear();
                const PropertyValues* pValues = aSeq.getConstArray();
                //TODO: Limiting to the first SAL_MAX_UINT16 elements of aSeq so that size of
                // m_SortKeyArr remains in range of sal_uInt16, as GetSortKeyCount and GetSortKey
                // still expect m_SortKeyArr to be indexed by sal_uInt16:
                for(sal_Int32 i = 0; i < aSeq.getLength() && i < SAL_MAX_UINT16; i++)
                {
                    const PropertyValue* pValue = pValues[i].getConstArray();
                    SwTOXSortKey aSortKey;
                    for(sal_Int32 j = 0; j < pValues[i].getLength(); j++)
                    {
                        if(pValue[j].Name == UNO_NAME_SORT_KEY)
                        {
                            sal_Int16 nVal = -1; pValue[j].Value >>= nVal;
                            if(nVal >= 0 && nVal < AUTH_FIELD_END)
                                aSortKey.eField = static_cast<ToxAuthorityField>(nVal);
                            else
                                bRet = false;
                        }
                        else if(pValue[j].Name == UNO_NAME_IS_SORT_ASCENDING)
                        {
                            aSortKey.bSortAscending = *o3tl::doAccess<bool>(pValue[j].Value);
                        }
                    }
                    m_SortKeyArr.push_back(aSortKey);
                }
            }
        }
        break;
    default:
        assert(false);
    }
}

void SwAuthorityFieldType::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    //re-generate positions of the fields
    DelSequenceArray();
    NotifyClients( pOld, pNew );
}

sal_uInt16 SwAuthorityFieldType::GetSortKeyCount() const
{
    return m_SortKeyArr.size();
}

const SwTOXSortKey*  SwAuthorityFieldType::GetSortKey(sal_uInt16 nIdx) const
{
    if(m_SortKeyArr.size() > nIdx)
        return &m_SortKeyArr[nIdx];
    OSL_FAIL("Sort key not found");
    return nullptr;
}

void SwAuthorityFieldType::SetSortKeys(sal_uInt16 nKeyCount, SwTOXSortKey  const aKeys[])
{
    m_SortKeyArr.clear();
    for(sal_uInt16 i = 0; i < nKeyCount; i++)
        if(aKeys[i].eField < AUTH_FIELD_END)
            m_SortKeyArr.push_back(aKeys[i]);
}

SwAuthorityField::SwAuthorityField( SwAuthorityFieldType* pInitType,
                                    const OUString& rFieldContents )
    : SwField(pInitType)
    , m_nTempSequencePos( -1 )
    , m_nTempSequencePosRLHidden( -1 )
{
    m_xAuthEntry = pInitType->AddField( rFieldContents );
}

SwAuthorityField::SwAuthorityField( SwAuthorityFieldType* pInitType,
                                    SwAuthEntry* pAuthEntry )
    : SwField( pInitType )
    , m_xAuthEntry( pAuthEntry )
    , m_nTempSequencePos( -1 )
    , m_nTempSequencePosRLHidden( -1 )
{
}

SwAuthorityField::~SwAuthorityField()
{
    static_cast<SwAuthorityFieldType* >(GetTyp())->RemoveField(m_xAuthEntry.get());
}

OUString SwAuthorityField::ExpandImpl(SwRootFrame const*const pLayout) const
{
    return ConditionalExpandAuthIdentifier(pLayout);
}

OUString SwAuthorityField::ConditionalExpandAuthIdentifier(
        SwRootFrame const*const pLayout) const
{
    SwAuthorityFieldType* pAuthType = static_cast<SwAuthorityFieldType*>(GetTyp());
    OUString sRet;
    if(pAuthType->GetPrefix())
        sRet = OUString(pAuthType->GetPrefix());

    if( pAuthType->IsSequence() )
    {
        sal_IntPtr & rnTempSequencePos(pLayout && pLayout->IsHideRedlines()
                ? m_nTempSequencePosRLHidden : m_nTempSequencePos);
        if(!pAuthType->GetDoc()->getIDocumentFieldsAccess().IsExpFieldsLocked())
            rnTempSequencePos = pAuthType->GetSequencePos(m_xAuthEntry.get(), pLayout);
        if (0 <= rnTempSequencePos)
            sRet += OUString::number(rnTempSequencePos);
    }
    else
    {
        //TODO: Expand to: identifier, number sequence, ...
        if(m_xAuthEntry)
            sRet += m_xAuthEntry->GetAuthorField(AUTH_FIELD_IDENTIFIER);
    }
    if(pAuthType->GetSuffix())
        sRet += OUStringLiteral1(pAuthType->GetSuffix());
    return sRet;
}

OUString SwAuthorityField::ExpandCitation(ToxAuthorityField eField,
        SwRootFrame const*const pLayout) const
{
    SwAuthorityFieldType* pAuthType = static_cast<SwAuthorityFieldType*>(GetTyp());
    OUString sRet;

    if( pAuthType->IsSequence() )
    {
        sal_IntPtr & rnTempSequencePos(pLayout && pLayout->IsHideRedlines()
                ? m_nTempSequencePosRLHidden : m_nTempSequencePos);
        if(!pAuthType->GetDoc()->getIDocumentFieldsAccess().IsExpFieldsLocked())
            rnTempSequencePos = pAuthType->GetSequencePos(m_xAuthEntry.get(), pLayout);
        if (0 <= rnTempSequencePos)
            sRet += OUString::number(rnTempSequencePos);
    }
    else
    {
        //TODO: Expand to: identifier, number sequence, ...
        if(m_xAuthEntry)
            sRet += m_xAuthEntry->GetAuthorField(eField);
    }
    return sRet;
}

std::unique_ptr<SwField> SwAuthorityField::Copy() const
{
    SwAuthorityFieldType* pAuthType = static_cast<SwAuthorityFieldType*>(GetTyp());
    return std::make_unique<SwAuthorityField>(pAuthType, m_xAuthEntry.get());
}

OUString SwAuthorityField::GetFieldText(ToxAuthorityField eField) const
{
    return m_xAuthEntry->GetAuthorField( eField );
}

void SwAuthorityField::SetPar1(const OUString& rStr)
{
    SwAuthorityFieldType* pInitType = static_cast<SwAuthorityFieldType* >(GetTyp());
    pInitType->RemoveField(m_xAuthEntry.get());
    m_xAuthEntry = pInitType->AddField(rStr);
}

OUString SwAuthorityField::GetDescription() const
{
    return SwResId(STR_AUTHORITY_ENTRY);
}

const char* const aFieldNames[] =
{
    "Identifier",
    "BibiliographicType",
    "Address",
    "Annote",
    "Author",
    "Booktitle",
    "Chapter",
    "Edition",
    "Editor",
    "Howpublished",
    "Institution",
    "Journal",
    "Month",
    "Note",
    "Number",
    "Organizations",
    "Pages",
    "Publisher",
    "School",
    "Series",
    "Title",
    "Report_Type",
    "Volume",
    "Year",
    "URL",
    "Custom1",
    "Custom2",
    "Custom3",
    "Custom4",
    "Custom5",
    "ISBN"
};

bool    SwAuthorityField::QueryValue( Any& rAny, sal_uInt16 /*nWhichId*/ ) const
{
    if(!GetTyp())
        return false;
    if(!m_xAuthEntry)
        return false;
    Sequence <PropertyValue> aRet(AUTH_FIELD_END);
    PropertyValue* pValues = aRet.getArray();
    for(int i = 0; i < AUTH_FIELD_END; ++i)
    {
        pValues[i].Name = OUString::createFromAscii(aFieldNames[i]);
        const OUString& sField = m_xAuthEntry->GetAuthorField(static_cast<ToxAuthorityField>(i));
        if(i == AUTH_FIELD_AUTHORITY_TYPE)
            pValues[i].Value <<= sal_Int16(sField.toInt32());
        else
            pValues[i].Value <<= sField;
    }
    rAny <<= aRet;
    /* FIXME: it is weird that we always return false here */
    return false;
}

static sal_Int32 lcl_Find(const OUString& rFieldName)
{
    for(sal_Int32 i = 0; i < AUTH_FIELD_END; ++i)
        if(rFieldName.equalsAscii(aFieldNames[i]))
            return i;
    return -1;
}

bool    SwAuthorityField::PutValue( const Any& rAny, sal_uInt16 /*nWhichId*/ )
{
    if(!GetTyp() || !m_xAuthEntry)
        return false;

    Sequence <PropertyValue> aParam;
    if(!(rAny >>= aParam))
        return false;

    OUStringBuffer sBuf;
    comphelper::string::padToLength(sBuf, AUTH_FIELD_ISBN, TOX_STYLE_DELIMITER);
    OUString sToSet(sBuf.makeStringAndClear());
    const PropertyValue* pParam = aParam.getConstArray();
    for(sal_Int32 i = 0; i < aParam.getLength(); i++)
    {
        const sal_Int32 nFound = lcl_Find(pParam[i].Name);
        if(nFound >= 0)
        {
            OUString sContent;
            if(AUTH_FIELD_AUTHORITY_TYPE == nFound)
            {
                sal_Int16 nVal = 0;
                pParam[i].Value >>= nVal;
                sContent = OUString::number(nVal);
            }
            else
                pParam[i].Value >>= sContent;
            sToSet = comphelper::string::setToken(sToSet, nFound, TOX_STYLE_DELIMITER, sContent);
        }
    }

    static_cast<SwAuthorityFieldType*>(GetTyp())->RemoveField(m_xAuthEntry.get());
    m_xAuthEntry = static_cast<SwAuthorityFieldType*>(GetTyp())->AddField(sToSet);

    /* FIXME: it is weird that we always return false here */
    return false;
}

SwFieldType* SwAuthorityField::ChgTyp( SwFieldType* pFieldTyp )
{
    SwAuthorityFieldType* pSrcTyp = static_cast<SwAuthorityFieldType*>(GetTyp()),
                        * pDstTyp = static_cast<SwAuthorityFieldType*>(pFieldTyp);
    if( pSrcTyp != pDstTyp )
    {
        const SwAuthEntry* pSrcEntry = m_xAuthEntry.get();
        pDstTyp->AppendField( *pSrcEntry );
        pSrcTyp->RemoveField( pSrcEntry );
        SwField::ChgTyp( pFieldTyp );
    }
    return pSrcTyp;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

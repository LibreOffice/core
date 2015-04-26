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
#include <comphelper/string.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/langitem.hxx>
#include <swtypes.hxx>
#include <tools/resid.hxx>
#include <comcore.hrc>
#include <authfld.hxx>
#include <expfld.hxx>
#include <pam.hxx>
#include <cntfrm.hxx>
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
    : nRefCount(0)
{
    for(sal_uInt16 i = 0; i < AUTH_FIELD_END; i++)
        aAuthFields[i] = rCopy.aAuthFields[i];
}

bool    SwAuthEntry::operator==(const SwAuthEntry& rComp)
{
    for(sal_uInt16 i = 0; i < AUTH_FIELD_END; i++)
        if(aAuthFields[i] != rComp.aAuthFields[i])
            return false;
    return true;
}

SwAuthorityFieldType::SwAuthorityFieldType(SwDoc* pDoc)
    : SwFieldType( RES_AUTHORITY ),
    m_pDoc(pDoc),
    m_DataArr(),
    m_SortKeyArr(3),
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

void    SwAuthorityFieldType::RemoveField(sal_IntPtr nHandle)
{
    for(sal_uInt16 j = 0; j < m_DataArr.size(); j++)
    {
        SwAuthEntry* pTemp = &m_DataArr[j];
        sal_IntPtr nRet = reinterpret_cast<sal_IntPtr>((void*)pTemp);
        if(nRet == nHandle)
        {
            pTemp->RemoveRef();
            if(!pTemp->GetRefCount())
            {
                m_DataArr.erase(m_DataArr.begin() + j);
                //re-generate positions of the fields
                DelSequenceArray();
            }
            return;
        }
    }
    OSL_FAIL("Field unknown" );
}

sal_IntPtr SwAuthorityFieldType::AddField(const OUString& rFieldContents)
{
    sal_IntPtr nRet = 0;
    SwAuthEntry* pEntry = new SwAuthEntry;
    for( sal_uInt16 i = 0; i < AUTH_FIELD_END; ++i )
        pEntry->SetAuthorField( (ToxAuthorityField)i,
                        rFieldContents.getToken( i, TOX_STYLE_DELIMITER ));

    for(sal_uInt16 j = 0; j < m_DataArr.size(); j++)
    {
        SwAuthEntry* pTemp = &m_DataArr[j];
        if(*pTemp == *pEntry)
        {
            delete pEntry;
            nRet = reinterpret_cast<sal_IntPtr>((void*)pTemp);
            pTemp->AddRef();
            return nRet;
        }
    }

    //if it is a new Entry - insert
    nRet = reinterpret_cast<sal_IntPtr>((void*)pEntry);
    pEntry->AddRef();
    m_DataArr.push_back(pEntry);
    //re-generate positions of the fields
    DelSequenceArray();
    return nRet;
}

bool SwAuthorityFieldType::AddField(sal_IntPtr nHandle)
{
    for( sal_uInt16 j = 0; j < m_DataArr.size(); j++ )
    {
        SwAuthEntry* pTemp = &m_DataArr[j];
        sal_IntPtr nTmp = reinterpret_cast<sal_IntPtr>((void*)pTemp);
        if( nTmp == nHandle )
        {
            pTemp->AddRef();
            //re-generate positions of the fields
            DelSequenceArray();
            return true;
        }
    }
    OSL_FAIL("SwAuthorityFieldType::AddField(sal_IntPtr) failed");
    return false;
}

const SwAuthEntry*  SwAuthorityFieldType::GetEntryByHandle(sal_IntPtr nHandle) const
{
    for(sal_uInt16 j = 0; j < m_DataArr.size(); j++)
    {
        const SwAuthEntry* pTemp = &m_DataArr[j];
        sal_IntPtr nTmp = reinterpret_cast<sal_IntPtr>((void*)pTemp);
        if( nTmp == nHandle )
        {
            return pTemp;
        }
    }
    OSL_FAIL( "invalid Handle" );
    return nullptr;
}

void SwAuthorityFieldType::GetAllEntryIdentifiers(
    std::vector<OUString>& rToFill )const
{
    for(sal_uInt16 j = 0; j < m_DataArr.size(); j++)
    {
        const SwAuthEntry* pTemp = &m_DataArr[j];
        rToFill.push_back(pTemp->GetAuthorField(AUTH_FIELD_IDENTIFIER));
    }
}

const SwAuthEntry*  SwAuthorityFieldType::GetEntryByIdentifier(
                                const OUString& rIdentifier)const
{
    for( sal_uInt16 j = 0; j < m_DataArr.size(); ++j )
    {
        const SwAuthEntry* pTemp = &m_DataArr[j];
        if( rIdentifier == pTemp->GetAuthorField( AUTH_FIELD_IDENTIFIER ))
        {
            return pTemp;
        }
    }
    return nullptr;
}

bool SwAuthorityFieldType::ChangeEntryContent(const SwAuthEntry* pNewEntry)
{
    for( sal_uInt16 j = 0; j < m_DataArr.size(); ++j )
    {
        SwAuthEntry* pTemp = &m_DataArr[j];
        if(pTemp->GetAuthorField(AUTH_FIELD_IDENTIFIER) ==
                    pNewEntry->GetAuthorField(AUTH_FIELD_IDENTIFIER))
        {
            for(sal_uInt16 i = 0; i < AUTH_FIELD_END; i++)
                pTemp->SetAuthorField((ToxAuthorityField) i,
                    pNewEntry->GetAuthorField((ToxAuthorityField)i));
            return true;
        }
    }
    return false;
}

/// appends a new entry (if new) and returns the array position
sal_uInt16  SwAuthorityFieldType::AppendField( const SwAuthEntry& rInsert )
{
    sal_uInt16 nRet = 0;
    for( nRet = 0; nRet < m_DataArr.size(); ++nRet )
    {
        SwAuthEntry* pTemp = &m_DataArr[ nRet ];
        if( *pTemp == rInsert )
        {
            //ref count unchanged
            return nRet;
        }
    }

    //if it is a new Entry - insert
    m_DataArr.push_back( new SwAuthEntry( rInsert ) );
    return m_DataArr.size()-1;
}

sal_IntPtr SwAuthorityFieldType::GetHandle(sal_uInt16 nPos)
{
    if( nPos < m_DataArr.size() )
    {
        SwAuthEntry* pTemp = &m_DataArr[nPos];
        return reinterpret_cast<sal_IntPtr>((void*)pTemp);
    }
    return 0;
}

sal_uInt16  SwAuthorityFieldType::GetSequencePos(sal_IntPtr nHandle)
{
    //find the field in a sorted array of handles,
#if OSL_DEBUG_LEVEL > 0
    bool bCurrentFieldWithoutTextNode = false;
#endif
    if(!m_SequArr.empty() && m_SequArr.size() != m_DataArr.size())
        DelSequenceArray();
    if(m_SequArr.empty())
    {
        SwTOXSortTabBases aSortArr;
        SwIterator<SwFmtFld,SwFieldType> aIter( *this );

        SwTOXInternational aIntl(m_eLanguage, 0, m_sSortAlgorithm);

        for( SwFmtFld* pFmtFld = aIter.First(); pFmtFld; pFmtFld = aIter.Next() )
        {
            const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
            if(!pTxtFld || !pTxtFld->GetpTxtNode())
            {
#if OSL_DEBUG_LEVEL > 0
                if(nHandle == static_cast<SwAuthorityField*>(pFmtFld->GetField())->GetHandle())
                    bCurrentFieldWithoutTextNode = true;
#endif
                continue;
            }
            const SwTxtNode& rFldTxtNode = pTxtFld->GetTxtNode();
            SwPosition aFldPos(rFldTxtNode);
            SwDoc& rDoc = *const_cast<SwDoc*>(rFldTxtNode.GetDoc());
            SwCntntFrm *pFrm = rFldTxtNode.getLayoutFrm( rDoc.getIDocumentLayoutAccess().GetCurrentLayout() );
            const SwTxtNode* pTxtNode = 0;
            if(pFrm && !pFrm->IsInDocBody())
                pTxtNode = GetBodyTxtNode( rDoc, aFldPos, *pFrm );
            //if no text node could be found or the field is in the document
            //body the directly available text node will be used
            if(!pTxtNode)
                pTxtNode = &rFldTxtNode;
            if (!pTxtNode->GetTxt().isEmpty() &&
                pTxtNode->getLayoutFrm( rDoc.getIDocumentLayoutAccess().GetCurrentLayout() ) &&
                pTxtNode->GetNodes().IsDocNodes() )
            {
                SwTOXAuthority* pNew = new SwTOXAuthority( *pTxtNode,
                                                            *pFmtFld, aIntl );

                for(short i = 0; i < (short)aSortArr.size(); ++i)
                {
                    SwTOXSortTabBase* pOld = aSortArr[i];
                    if(*pOld == *pNew)
                    {
                        //only the first occurrence in the document
                        //has to be in the array
                        if(*pOld < *pNew)
                            DELETEZ(pNew);
                        else // remove the old content
                        {
                            aSortArr.erase(aSortArr.begin() + i);
                            delete pOld;
                        }
                        break;
                    }
                }
                //if it still exists - insert at the correct position
                if(pNew)
                {
                    short j;

                    for( j = 0; j < (short)aSortArr.size(); ++j)
                    {
                        SwTOXSortTabBase* pOld = aSortArr[j];
                        if(*pNew < *pOld)
                            break;
                    }
                    aSortArr.insert(aSortArr.begin() + j, pNew);
                }
            }
        }

        for(sal_uInt16 i = 0; i < aSortArr.size(); i++)
        {
            const SwTOXSortTabBase& rBase = *aSortArr[i];
            SwFmtFld& rFmtFld = const_cast<SwTOXAuthority&>(static_cast<const SwTOXAuthority&>(rBase)).GetFldFmt();
            SwAuthorityField* pAFld = static_cast<SwAuthorityField*>(rFmtFld.GetField());
            m_SequArr.push_back(pAFld->GetHandle());
        }
        for (SwTOXSortTabBases::const_iterator it = aSortArr.begin(); it != aSortArr.end(); ++it)
            delete *it;
        aSortArr.clear();
    }
    //find nHandle
    for(sal_uInt16 i = 0; i < m_SequArr.size(); ++i)
    {
        if(m_SequArr[i] == nHandle)
        {
            return i + 1;
        }
    }
#if OSL_DEBUG_LEVEL > 0
    OSL_ENSURE(bCurrentFieldWithoutTextNode, "Handle not found");
#endif
    return 0;
}

bool SwAuthorityFieldType::QueryValue( Any& rVal, sal_uInt16 nWhichId ) const
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
            for(sal_uInt16 i = 0; i < m_SortKeyArr.size(); i++)
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
        OSL_FAIL("illegal property");
    }
    return true;
}

bool    SwAuthorityFieldType::PutValue( const Any& rAny, sal_uInt16 nWhichId )
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
        m_bIsSequence = *static_cast<sal_Bool const *>(rAny.getValue());
        break;
    case FIELD_PROP_BOOL2:
        m_bSortByDocument = *static_cast<sal_Bool const *>(rAny.getValue());
        break;

    case FIELD_PROP_LOCALE:
        {
            com::sun::star::lang::Locale aLocale;
            if( (bRet = rAny >>= aLocale ))
                SetLanguage( LanguageTag::convertToLanguageType( aLocale ));
        }
        break;

    case FIELD_PROP_PROP_SEQ:
        {
            Sequence<PropertyValues> aSeq;
            if( (bRet = rAny >>= aSeq) )
            {
                m_SortKeyArr.clear();
                const PropertyValues* pValues = aSeq.getConstArray();
                for(sal_Int32 i = 0; i < aSeq.getLength() && i < USHRT_MAX / 4; i++)
                {
                    const PropertyValue* pValue = pValues[i].getConstArray();
                    SwTOXSortKey* pSortKey = new SwTOXSortKey;
                    for(sal_Int32 j = 0; j < pValues[i].getLength(); j++)
                    {
                        if(pValue[j].Name == UNO_NAME_SORT_KEY)
                        {
                            sal_Int16 nVal = -1; pValue[j].Value >>= nVal;
                            if(nVal >= 0 && nVal < AUTH_FIELD_END)
                                pSortKey->eField = (ToxAuthorityField) nVal;
                            else
                                bRet = false;
                        }
                        else if(pValue[j].Name == UNO_NAME_IS_SORT_ASCENDING)
                        {
                            pSortKey->bSortAscending = *static_cast<sal_Bool const *>(pValue[j].Value.getValue());
                        }
                    }
                    m_SortKeyArr.push_back(pSortKey);
                }
            }
        }
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return bRet;
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

void SwAuthorityFieldType::SetSortKeys(sal_uInt16 nKeyCount, SwTOXSortKey aKeys[])
{
    m_SortKeyArr.clear();
    for(sal_uInt16 i = 0; i < nKeyCount; i++)
        if(aKeys[i].eField < AUTH_FIELD_END)
            m_SortKeyArr.push_back(new SwTOXSortKey(aKeys[i]));
}

SwAuthorityField::SwAuthorityField( SwAuthorityFieldType* pInitType,
                                    const OUString& rFieldContents )
    : SwField(pInitType),
    m_nTempSequencePos( -1 )
{
    m_nHandle = pInitType->AddField( rFieldContents );
}

SwAuthorityField::SwAuthorityField( SwAuthorityFieldType* pInitType,
                                                sal_IntPtr nSetHandle )
    : SwField( pInitType ),
    m_nHandle( nSetHandle ),
    m_nTempSequencePos( -1 )
{
    pInitType->AddField( m_nHandle );
}

SwAuthorityField::~SwAuthorityField()
{
    static_cast<SwAuthorityFieldType* >(GetTyp())->RemoveField(m_nHandle);
}

OUString SwAuthorityField::Expand() const
{
    return ConditionalExpand(AUTH_FIELD_IDENTIFIER);
}

OUString SwAuthorityField::ConditionalExpand(ToxAuthorityField eField) const
{
    SwAuthorityFieldType* pAuthType = static_cast<SwAuthorityFieldType*>(GetTyp());
    OUString sRet;
    if(pAuthType->GetPrefix() && eField != AUTH_FIELD_TITLE)
        sRet = OUString(pAuthType->GetPrefix());

    if( pAuthType->IsSequence() )
    {
       if(!pAuthType->GetDoc()->getIDocumentFieldsAccess().IsExpFldsLocked())
           m_nTempSequencePos = pAuthType->GetSequencePos( m_nHandle );
       if( m_nTempSequencePos >= 0 )
           sRet += OUString::number( m_nTempSequencePos );
    }
    else
    {
        const SwAuthEntry* pEntry = pAuthType->GetEntryByHandle(m_nHandle);
        //TODO: Expand to: identifier, number sequence, ...
        if(pEntry)
            sRet += pEntry->GetAuthorField(AUTH_FIELD_IDENTIFIER);
    }
    if(pAuthType->GetSuffix() && eField != AUTH_FIELD_TITLE)
        sRet += OUString(pAuthType->GetSuffix());
    return sRet;
}

OUString SwAuthorityField::ExpandCitation(ToxAuthorityField eField) const
{
    SwAuthorityFieldType* pAuthType = static_cast<SwAuthorityFieldType*>(GetTyp());
    OUString sRet;

    if( pAuthType->IsSequence() )
    {
       if(!pAuthType->GetDoc()->getIDocumentFieldsAccess().IsExpFldsLocked())
           m_nTempSequencePos = pAuthType->GetSequencePos( m_nHandle );
       if( m_nTempSequencePos >= 0 )
           sRet += OUString::number( m_nTempSequencePos );
    }
    else
    {
        const SwAuthEntry* pEntry = pAuthType->GetEntryByHandle(m_nHandle);
        //TODO: Expand to: identifier, number sequence, ...
        if(pEntry)
            sRet += pEntry->GetAuthorField(eField);
    }
    return sRet;
}

SwField* SwAuthorityField::Copy() const
{
    SwAuthorityFieldType* pAuthType = static_cast<SwAuthorityFieldType*>(GetTyp());
    return new SwAuthorityField(pAuthType, m_nHandle);
}

OUString SwAuthorityField::GetFieldText(ToxAuthorityField eField) const
{
    SwAuthorityFieldType* pAuthType = static_cast<SwAuthorityFieldType*>(GetTyp());
    const SwAuthEntry* pEntry = pAuthType->GetEntryByHandle( m_nHandle );
    return pEntry->GetAuthorField( eField );
}

void    SwAuthorityField::SetPar1(const OUString& rStr)
{
    SwAuthorityFieldType* pInitType = static_cast<SwAuthorityFieldType* >(GetTyp());
    pInitType->RemoveField(m_nHandle);
    m_nHandle = pInitType->AddField(rStr);
}

OUString SwAuthorityField::GetDescription() const
{
    return SW_RES(STR_AUTHORITY_ENTRY);
}

const char* aFieldNames[] =
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
    const SwAuthEntry* pAuthEntry = static_cast<SwAuthorityFieldType*>(GetTyp())->GetEntryByHandle(m_nHandle);
    if(!pAuthEntry)
        return false;
    Sequence <PropertyValue> aRet(AUTH_FIELD_END);
    PropertyValue* pValues = aRet.getArray();
    for(sal_Int16 i = 0; i < AUTH_FIELD_END; i++)
    {
        pValues[i].Name = OUString::createFromAscii(aFieldNames[i]);
        const OUString sField = pAuthEntry->GetAuthorField((ToxAuthorityField) i);
        if(i == AUTH_FIELD_AUTHORITY_TYPE)
            pValues[i].Value <<= sal_Int16(sField.toInt32());
        else
            pValues[i].Value <<= sField;
    }
    rAny <<= aRet;
    /* FIXME: it is weird that we always return false here */
    return false;
}

static sal_Int16 lcl_Find(const OUString& rFieldName)
{
    for(sal_Int16 i = 0; i < AUTH_FIELD_END; i++)
        if(rFieldName.equalsAscii(aFieldNames[i]))
            return i;
    return -1;
}

bool    SwAuthorityField::PutValue( const Any& rAny, sal_uInt16 /*nWhichId*/ )
{
    if(!GetTyp() || !static_cast<SwAuthorityFieldType*>(GetTyp())->GetEntryByHandle(m_nHandle))
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
        sal_Int16 nFound = lcl_Find(pParam[i].Name);
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

    static_cast<SwAuthorityFieldType*>(GetTyp())->RemoveField(m_nHandle);
    m_nHandle = static_cast<SwAuthorityFieldType*>(GetTyp())->AddField(sToSet);

    /* FIXME: it is weird that we always return false here */
    return false;
}

SwFieldType* SwAuthorityField::ChgTyp( SwFieldType* pFldTyp )
{
    SwAuthorityFieldType* pSrcTyp = static_cast<SwAuthorityFieldType*>(GetTyp()),
                        * pDstTyp = static_cast<SwAuthorityFieldType*>(pFldTyp);
    if( pSrcTyp != pDstTyp )
    {

        const SwAuthEntry* pEntry = pSrcTyp->GetEntryByHandle( m_nHandle );
        sal_uInt16 nHdlPos = pDstTyp->AppendField( *pEntry );
        pSrcTyp->RemoveField( m_nHandle );
        m_nHandle = pDstTyp->GetHandle( nHdlPos );
        pDstTyp->AddField( m_nHandle );
        SwField::ChgTyp( pFldTyp );
    }
    return pSrcTyp;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

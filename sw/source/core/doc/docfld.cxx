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

#include <config_features.h>

#include <hintids.hxx>

#include <string.h>
#include <float.h>
#include <comphelper/string.hxx>
#include <tools/datetime.hxx>
#include <vcl/svapp.hxx>
#include <unotools/charclass.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <cntfrm.hxx>
#include <txtfrm.hxx>
#include <notxtfrm.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <swtable.hxx>
#include <calc.hxx>
#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <tox.hxx>
#include <txttxmrk.hxx>
#include <docfld.hxx>
#include <docufld.hxx>
#include <ddefld.hxx>
#include <usrfld.hxx>
#include <expfld.hxx>
#include <dbfld.hxx>
#include <flddat.hxx>
#include <chpfld.hxx>
#include <reffld.hxx>
#include <flddropdown.hxx>
#include <dbmgr.hxx>
#include <section.hxx>
#include <cellatr.hxx>
#include <docary.hxx>
#include <authfld.hxx>
#include <txtinet.hxx>
#include <fmtcntnt.hxx>

#include <SwUndoField.hxx>
#include <calbck.hxx>

using namespace ::com::sun::star::uno;

// the StartIndex can be supplied optionally (e.g. if it was queried before - is a virtual
// method otherwise!)
SetGetExpField::SetGetExpField(
    const SwNodeIndex& rNdIdx,
    const SwTextField* pField,
    const SwIndex* pIdx )
{
    m_eSetGetExpFieldType = TEXTFIELD;
    m_CNTNT.pTextField = pField;
    m_nNode = rNdIdx.GetIndex();
    if( pIdx )
        m_nContent = pIdx->GetIndex();
    else if( pField )
        m_nContent = pField->GetStart();
    else
        m_nContent = 0;
}

SetGetExpField::SetGetExpField( const SwNodeIndex& rNdIdx,
                            const SwTextINetFormat& rINet )
{
    m_eSetGetExpFieldType = TEXTINET;
    m_CNTNT.pTextINet = &rINet;
    m_nNode = rNdIdx.GetIndex();
    m_nContent = rINet.GetStart();
}

// Extension for Sections:
// these always have content position 0xffffffff!
// There is never a field on this, only up to COMPLETE_STRING possible
SetGetExpField::SetGetExpField( const SwSectionNode& rSectNd,
                                const SwPosition* pPos )
{
    m_eSetGetExpFieldType = SECTIONNODE;
    m_CNTNT.pSection = &rSectNd.GetSection();

    if( pPos )
    {
        m_nNode = pPos->nNode.GetIndex();
        m_nContent = pPos->nContent.GetIndex();
    }
    else
    {
        m_nNode = rSectNd.GetIndex();
        m_nContent = 0;
    }
}

SetGetExpField::SetGetExpField( const SwTableBox& rTBox )
{
    m_eSetGetExpFieldType = TABLEBOX;
    m_CNTNT.pTBox = &rTBox;

    m_nNode = 0;
    m_nContent = 0;
    if( rTBox.GetSttNd() )
    {
        SwNodeIndex aIdx( *rTBox.GetSttNd() );
        const SwContentNode* pNd = aIdx.GetNode().GetNodes().GoNext( &aIdx );
        if( pNd )
            m_nNode = pNd->GetIndex();
    }
}

SetGetExpField::SetGetExpField( const SwNodeIndex& rNdIdx,
                                const SwTextTOXMark& rTOX )
{
    m_eSetGetExpFieldType = TEXTTOXMARK;
    m_CNTNT.pTextTOX = &rTOX;
    m_nNode = rNdIdx.GetIndex();
    m_nContent = rTOX.GetStart();
}

SetGetExpField::SetGetExpField( const SwPosition& rPos )
{
    m_eSetGetExpFieldType = CRSRPOS;
    m_CNTNT.pPos = &rPos;
    m_nNode = rPos.nNode.GetIndex();
    m_nContent = rPos.nContent.GetIndex();
}

SetGetExpField::SetGetExpField( const SwFlyFrameFormat& rFlyFormat,
                                const SwPosition* pPos  )
{
    m_eSetGetExpFieldType = FLYFRAME;
    m_CNTNT.pFlyFormat = &rFlyFormat;
    if( pPos )
    {
        m_nNode = pPos->nNode.GetIndex();
        m_nContent = pPos->nContent.GetIndex();
    }
    else
    {
        const SwFormatContent& rContent = rFlyFormat.GetContent();
        m_nNode = rContent.GetContentIdx()->GetIndex() + 1;
        m_nContent = 0;
    }
}

void SetGetExpField::GetPosOfContent( SwPosition& rPos ) const
{
    const SwNode* pNd = GetNodeFromContent();
    if( pNd )
        pNd = pNd->GetContentNode();

    if( pNd )
    {
        rPos.nNode = *pNd;
        rPos.nContent.Assign( const_cast<SwContentNode*>(static_cast<const SwContentNode*>(pNd)), GetCntPosFromContent() );
    }
    else
    {
        rPos.nNode = m_nNode;
        rPos.nContent.Assign( rPos.nNode.GetNode().GetContentNode(), m_nContent );
    }
}

void SetGetExpField::SetBodyPos( const SwContentFrame& rFrame )
{
    if( !rFrame.IsInDocBody() )
    {
        SwNodeIndex aIdx( rFrame.IsTextFrame()
                ? *static_cast<SwTextFrame const&>(rFrame).GetTextNodeFirst()
                : *static_cast<SwNoTextFrame const&>(rFrame).GetNode() );
        SwDoc& rDoc = *aIdx.GetNodes().GetDoc();
        SwPosition aPos( aIdx );
        bool const bResult = ::GetBodyTextNode( rDoc, aPos, rFrame );
        OSL_ENSURE(bResult, "Where is the field?");
        m_nNode = aPos.nNode.GetIndex();
        m_nContent = aPos.nContent.GetIndex();
    }
}

bool SetGetExpField::operator==( const SetGetExpField& rField ) const
{
    return m_nNode == rField.m_nNode
           && m_nContent == rField.m_nContent
           && ( !m_CNTNT.pTextField
                || !rField.m_CNTNT.pTextField
                || m_CNTNT.pTextField == rField.m_CNTNT.pTextField );
}

bool SetGetExpField::operator<( const SetGetExpField& rField ) const
{
    if( m_nNode < rField.m_nNode || ( m_nNode == rField.m_nNode && m_nContent < rField.m_nContent ))
        return true;
    else if( m_nNode != rField.m_nNode || m_nContent != rField.m_nContent )
        return false;

    const SwNode *pFirst = GetNodeFromContent(),
                 *pNext = rField.GetNodeFromContent();

    // Position is the same: continue only if both field pointers are set!
    if( !pFirst || !pNext )
        return false;

    // same Section?
    if( pFirst->StartOfSectionNode() != pNext->StartOfSectionNode() )
    {
        // is one in the table?
        const SwNode *pFirstStt, *pNextStt;
        const SwTableNode* pTableNd = pFirst->FindTableNode();
        if( pTableNd )
            pFirstStt = pTableNd->StartOfSectionNode();
        else
            pFirstStt = pFirst->StartOfSectionNode();

        if( nullptr != ( pTableNd = pNext->FindTableNode() ) )
            pNextStt = pTableNd->StartOfSectionNode();
        else
            pNextStt = pNext->StartOfSectionNode();

        if( pFirstStt != pNextStt )
        {
            if( pFirst->IsTextNode() && pNext->IsTextNode() &&
                ( pFirst->FindFlyStartNode() || pNext->FindFlyStartNode() ))
            {
                // FIXME: in NewFieldPortion(), SwGetExpField are expanded via
                // DocumentFieldsManager::FieldsToExpand() calling
                // std::upper_bound binary search function - the sort order
                // depends on the fly positions in the layout, but the fly
                // positions depend on the expansion of the SwGetExpField!
                // This circular dep will cause trouble, it would be better to
                // use only model positions (anchor), but then how to compare
                // at-page anchored flys which don't have a model anchor?
                return ::IsFrameBehind( *pNext->GetTextNode(), m_nContent, *pFirst->GetTextNode(), m_nContent );
            }
            return pFirstStt->GetIndex() < pNextStt->GetIndex();
        }
    }

    // same Section: is the field in the same Node?
    if( pFirst != pNext )
        return pFirst->GetIndex() < pNext->GetIndex();

    // same Node in the Section, check Position in the Node
    return GetCntPosFromContent() < rField.GetCntPosFromContent();
}

const SwNode* SetGetExpField::GetNodeFromContent() const
{
    const SwNode* pRet = nullptr;
    if( m_CNTNT.pTextField )
        switch( m_eSetGetExpFieldType )
        {
        case TEXTFIELD:
            pRet = &m_CNTNT.pTextField->GetTextNode();
            break;

        case TEXTINET:
            pRet = &m_CNTNT.pTextINet->GetTextNode();
            break;

        case SECTIONNODE:
            pRet = m_CNTNT.pSection->GetFormat()->GetSectionNode();
            break;

        case CRSRPOS:
            pRet = &m_CNTNT.pPos->nNode.GetNode();
            break;

        case TEXTTOXMARK:
            pRet = &m_CNTNT.pTextTOX->GetTextNode();
            break;

        case TABLEBOX:
            if( m_CNTNT.pTBox->GetSttNd() )
            {
                SwNodeIndex aIdx( *m_CNTNT.pTBox->GetSttNd() );
                pRet = aIdx.GetNode().GetNodes().GoNext( &aIdx );
            }
            break;

        case FLYFRAME:
            {
                SwNodeIndex aIdx( *m_CNTNT.pFlyFormat->GetContent().GetContentIdx() );
                pRet = aIdx.GetNode().GetNodes().GoNext( &aIdx );
            }
            break;
        }
    return pRet;
}

sal_Int32 SetGetExpField::GetCntPosFromContent() const
{
    sal_Int32 nRet = 0;
    if( m_CNTNT.pTextField )
        switch( m_eSetGetExpFieldType )
        {
        case TEXTFIELD:
            nRet = m_CNTNT.pTextField->GetStart();
            break;
        case TEXTINET:
            nRet = m_CNTNT.pTextINet->GetStart();
            break;
        case TEXTTOXMARK:
            nRet = m_CNTNT.pTextTOX->GetStart();
            break;
        case CRSRPOS:
            nRet =  m_CNTNT.pPos->nContent.GetIndex();
            break;
        default:
            break;
        }
    return nRet;
}

HashStr::HashStr( const OUString& rName, const OUString& rText,
                    HashStr* pNxt )
    : SwHash( rName ), aSetStr( rText )
{
    pNext.reset( pNxt );
}

/// Look up the Name, if it is present, return its String, otherwise return an empty String
OUString LookString( SwHashTable<HashStr> const & rTable, const OUString& rName )
{
    HashStr* pFnd = rTable.Find( comphelper::string::strip(rName, ' ') );
    if( pFnd )
        return pFnd->aSetStr;

    return OUString();
}

SwDBData const & SwDoc::GetDBData()
{
    return GetDBDesc();
}

const SwDBData& SwDoc::GetDBDesc()
{
#if HAVE_FEATURE_DBCONNECTIVITY
    if(maDBData.sDataSource.isEmpty())
    {
        const SwFieldTypes::size_type nSize = getIDocumentFieldsAccess().GetFieldTypes()->size();
        for(SwFieldTypes::size_type i = 0; i < nSize && maDBData.sDataSource.isEmpty(); ++i)
        {
            SwFieldType& rFieldType = *((*getIDocumentFieldsAccess().GetFieldTypes())[i]);
            SwFieldIds nWhich = rFieldType.Which();
            if(IsUsed(rFieldType))
            {
                switch(nWhich)
                {
                    case SwFieldIds::Database:
                    case SwFieldIds::DbNextSet:
                    case SwFieldIds::DbNumSet:
                    case SwFieldIds::DbSetNumber:
                    {
                        SwIterator<SwFormatField,SwFieldType> aIter( rFieldType );
                        for( SwFormatField* pField = aIter.First(); pField; pField = aIter.Next() )
                        {
                            if(pField->IsFieldInDoc())
                            {
                                if(SwFieldIds::Database == nWhich)
                                    maDBData = static_cast < SwDBFieldType * > (pField->GetField()->GetTyp())->GetDBData();
                                else
                                    maDBData = static_cast < SwDBNameInfField* > (pField->GetField())->GetRealDBData();
                                break;
                            }
                        }
                    }
                    break;
                    default: break;
                }
            }
        }
    }
    if(maDBData.sDataSource.isEmpty())
        maDBData = SwDBManager::GetAddressDBName();
#endif
    return maDBData;
}

void SwDoc::SetInitDBFields( bool b )
{
#if !HAVE_FEATURE_DBCONNECTIVITY
    (void) b;
#else
    GetDBManager()->SetInitDBFields( b );
#endif
}

#if HAVE_FEATURE_DBCONNECTIVITY

/// Get all databases that are used by fields
static OUString lcl_DBDataToString(const SwDBData& rData)
{
    return rData.sDataSource + OUStringLiteral1(DB_DELIM)
        + rData.sCommand + OUStringLiteral1(DB_DELIM)
        + OUString::number(rData.nCommandType);
}

#endif

void SwDoc::GetAllUsedDB( std::vector<OUString>& rDBNameList,
                          const std::vector<OUString>* pAllDBNames )
{
#if !HAVE_FEATURE_DBCONNECTIVITY
    (void) rDBNameList;
    (void) pAllDBNames;
#else
    std::vector<OUString> aUsedDBNames;
    std::vector<OUString> aAllDBNames;

    if( !pAllDBNames )
    {
        GetAllDBNames( aAllDBNames );
        pAllDBNames = &aAllDBNames;
    }

    SwSectionFormats& rArr = GetSections();
    for (auto n = rArr.size(); n; )
    {
        SwSection* pSect = rArr[ --n ]->GetSection();

        if( pSect )
        {
            AddUsedDBToList( rDBNameList, FindUsedDBs( *pAllDBNames,
                                                pSect->GetCondition(), aUsedDBNames ) );
            aUsedDBNames.clear();
        }
    }

    sal_uInt32 nMaxItems = GetAttrPool().GetItemCount2( RES_TXTATR_FIELD );
    for (sal_uInt32 n = 0; n < nMaxItems; ++n)
    {
        const SfxPoolItem* pItem;
        if( nullptr == (pItem = GetAttrPool().GetItem2( RES_TXTATR_FIELD, n ) ))
            continue;

        const SwFormatField* pFormatField = static_cast<const SwFormatField*>(pItem);
        const SwTextField* pTextField = pFormatField->GetTextField();
        if( !pTextField || !pTextField->GetTextNode().GetNodes().IsDocNodes() )
            continue;

        const SwField* pField = pFormatField->GetField();
        switch( pField->GetTyp()->Which() )
        {
            case SwFieldIds::Database:
                AddUsedDBToList( rDBNameList,
                                lcl_DBDataToString(static_cast<const SwDBField*>(pField)->GetDBData() ));
                break;

            case SwFieldIds::DbSetNumber:
            case SwFieldIds::DatabaseName:
                AddUsedDBToList( rDBNameList,
                                lcl_DBDataToString(static_cast<const SwDBNameInfField*>(pField)->GetRealDBData() ));
                break;

            case SwFieldIds::DbNumSet:
            case SwFieldIds::DbNextSet:
                AddUsedDBToList( rDBNameList,
                                lcl_DBDataToString(static_cast<const SwDBNameInfField*>(pField)->GetRealDBData() ));
                [[fallthrough]]; // JP: is that right like that?

            case SwFieldIds::HiddenText:
            case SwFieldIds::HiddenPara:
                AddUsedDBToList(rDBNameList, FindUsedDBs( *pAllDBNames,
                                            pField->GetPar1(), aUsedDBNames ));
                aUsedDBNames.clear();
                break;

            case SwFieldIds::SetExp:
            case SwFieldIds::GetExp:
            case SwFieldIds::Table:
                AddUsedDBToList(rDBNameList, FindUsedDBs( *pAllDBNames,
                                        pField->GetFormula(), aUsedDBNames ));
                aUsedDBNames.clear();
                break;
            default: break;
        }
    }
#endif
}

void SwDoc::GetAllDBNames( std::vector<OUString>& rAllDBNames )
{
#if !HAVE_FEATURE_DBCONNECTIVITY
    (void) rAllDBNames;
#else
    SwDBManager* pMgr = GetDBManager();

    const SwDSParams_t& rArr = pMgr->GetDSParamArray();
    for (const auto& pParam : rArr)
    {
        rAllDBNames.emplace_back(pParam->sDataSource + OUStringLiteral1(DB_DELIM) + pParam->sCommand);
    }
#endif
}

std::vector<OUString>& SwDoc::FindUsedDBs( const std::vector<OUString>& rAllDBNames,
                                   const OUString& rFormula,
                                   std::vector<OUString>& rUsedDBNames )
{
    const CharClass& rCC = GetAppCharClass();
#ifndef UNX
    const OUString sFormula(rCC.uppercase( rFormula ));
#else
    const OUString sFormula(rFormula);
#endif

    for (const auto &sItem : rAllDBNames)
    {
        sal_Int32 nPos = sFormula.indexOf( sItem );
        if( nPos>=0 &&
            sFormula[ nPos + sItem.getLength() ] == '.' &&
            (!nPos || !rCC.isLetterNumeric( sFormula, nPos - 1 )))
        {
            // Look up table name
            nPos += sItem.getLength() + 1;
            const sal_Int32 nEndPos = sFormula.indexOf('.', nPos);
            if( nEndPos>=0 )
            {
                rUsedDBNames.emplace_back(sItem + OUStringLiteral1(DB_DELIM) + sFormula.copy( nPos, nEndPos - nPos ));
            }
        }
    }
    return rUsedDBNames;
}

void SwDoc::AddUsedDBToList( std::vector<OUString>& rDBNameList,
                             const std::vector<OUString>& rUsedDBNames )
{
    for ( const auto &sName : rUsedDBNames )
        AddUsedDBToList( rDBNameList, sName );
}

void SwDoc::AddUsedDBToList( std::vector<OUString>& rDBNameList, const OUString& rDBName)
{
#if !HAVE_FEATURE_DBCONNECTIVITY
    (void) rDBNameList;
    (void) rDBName;
#else
    if( rDBName.isEmpty() )
        return;

#ifdef UNX
    for( const auto &sName : rDBNameList )
        if( rDBName == sName.getToken(0, ';') )
            return;
#else
    const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
    for( const auto &sName : rDBNameList )
        if( rSCmp.isEqual( rDBName, sName.getToken(0, ';') ) )
            return;
#endif

    SwDBData aData;
    sal_Int32 nIdx{ 0 };
    aData.sDataSource = rDBName.getToken(0, DB_DELIM, nIdx);
    aData.sCommand = rDBName.getToken(0, DB_DELIM, nIdx);
    aData.nCommandType = -1;
    GetDBManager()->CreateDSData(aData);
    rDBNameList.push_back(rDBName);
#endif
}

void SwDoc::ChangeDBFields( const std::vector<OUString>& rOldNames,
                            const OUString& rNewName )
{
#if !HAVE_FEATURE_DBCONNECTIVITY
    (void) rOldNames;
    (void) rNewName;
#else
    SwDBData aNewDBData;
    sal_Int32 nIdx{ 0 };
    aNewDBData.sDataSource = rNewName.getToken(0, DB_DELIM, nIdx);
    aNewDBData.sCommand = rNewName.getToken(0, DB_DELIM, nIdx);
    aNewDBData.nCommandType = static_cast<short>(rNewName.getToken(0, DB_DELIM, nIdx).toInt32());

    SwSectionFormats& rArr = GetSections();
    for (auto n = rArr.size(); n; )
    {
        SwSection* pSect = rArr[ --n ]->GetSection();

        if( pSect )
        {
            pSect->SetCondition(ReplaceUsedDBs(rOldNames, rNewName, pSect->GetCondition()));
        }
    }

    sal_uInt32 nMaxItems = GetAttrPool().GetItemCount2( RES_TXTATR_FIELD );

    for (sal_uInt32 n = 0; n < nMaxItems; ++n )
    {
        const SfxPoolItem* pItem = GetAttrPool().GetItem2( RES_TXTATR_FIELD, n );
        if( !pItem )
            continue;

        SwFormatField* pFormatField = const_cast<SwFormatField*>(static_cast<const SwFormatField*>(pItem));
        SwTextField* pTextField = pFormatField->GetTextField();
        if( !pTextField || !pTextField->GetTextNode().GetNodes().IsDocNodes() )
            continue;

        SwField* pField = pFormatField->GetField();
        bool bExpand = false;

        switch( pField->GetTyp()->Which() )
        {
            case SwFieldIds::Database:
#if HAVE_FEATURE_DBCONNECTIVITY
                if( IsNameInArray( rOldNames, lcl_DBDataToString(static_cast<SwDBField*>(pField)->GetDBData())))
                {
                    SwDBFieldType* pOldTyp = static_cast<SwDBFieldType*>(pField->GetTyp());

                    SwDBFieldType* pTyp = static_cast<SwDBFieldType*>(getIDocumentFieldsAccess().InsertFieldType(
                            SwDBFieldType(this, pOldTyp->GetColumnName(), aNewDBData)));

                    pFormatField->RegisterToFieldType( *pTyp );
                    pField->ChgTyp(pTyp);

                    static_cast<SwDBField*>(pField)->ClearInitialized();
                    static_cast<SwDBField*>(pField)->InitContent();

                    bExpand = true;
                }
#endif
                break;

            case SwFieldIds::DbSetNumber:
            case SwFieldIds::DatabaseName:
                if( IsNameInArray( rOldNames,
                                lcl_DBDataToString(static_cast<SwDBNameInfField*>(pField)->GetRealDBData())))
                {
                    static_cast<SwDBNameInfField*>(pField)->SetDBData(aNewDBData);
                    bExpand = true;
                }
                break;

            case SwFieldIds::DbNumSet:
            case SwFieldIds::DbNextSet:
                if( IsNameInArray( rOldNames,
                                lcl_DBDataToString(static_cast<SwDBNameInfField*>(pField)->GetRealDBData())))
                {
                    static_cast<SwDBNameInfField*>(pField)->SetDBData(aNewDBData);
                }
                [[fallthrough]];
            case SwFieldIds::HiddenText:
            case SwFieldIds::HiddenPara:
                pField->SetPar1( ReplaceUsedDBs(rOldNames, rNewName, pField->GetPar1()) );
                bExpand = true;
                break;

            case SwFieldIds::SetExp:
            case SwFieldIds::GetExp:
            case SwFieldIds::Table:
                pField->SetPar2( ReplaceUsedDBs(rOldNames, rNewName, pField->GetFormula()) );
                bExpand = true;
                break;
            default: break;
        }

        if (bExpand)
            pTextField->ExpandTextField( true );
    }
    getIDocumentState().SetModified();
#endif
}

namespace
{

OUString lcl_CutOffDBCommandType(const OUString& rName)
{
    return rName.replaceFirst(OUStringLiteral1(DB_DELIM), ".").getToken(0, DB_DELIM);
}

}

OUString SwDoc::ReplaceUsedDBs( const std::vector<OUString>& rUsedDBNames,
                                const OUString& rNewName, const OUString& rFormula )
{
    const CharClass& rCC = GetAppCharClass();
    const OUString sNewName( lcl_CutOffDBCommandType(rNewName) );
    OUString sFormula(rFormula);

    for(const auto & rUsedDBName : rUsedDBNames)
    {
        const OUString sDBName( lcl_CutOffDBCommandType(rUsedDBName) );

        if (sDBName!=sNewName)
        {
            sal_Int32 nPos = 0;
            for (;;)
            {
                nPos = sFormula.indexOf(sDBName, nPos);
                if (nPos<0)
                {
                    break;
                }

                if( sFormula[nPos + sDBName.getLength()] == '.' &&
                    (!nPos || !rCC.isLetterNumeric( sFormula, nPos - 1 )))
                {
                    sFormula = sFormula.replaceAt(nPos, sDBName.getLength(), sNewName);
                    //prevent re-searching - this is useless and provokes
                    //endless loops when names containing each other and numbers are exchanged
                    //e.g.: old ?12345.12345  new: i12345.12345
                    nPos += sNewName.getLength();
                }
            }
        }
    }
    return sFormula;
}

bool SwDoc::IsNameInArray( const std::vector<OUString>& rArr, const OUString& rName )
{
#ifdef UNX
    for( const auto &sName : rArr )
        if( rName == sName )
            return true;
#else
    const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
    for( const auto &sName : rArr )
        if( rSCmp.isEqual( rName, sName ))
            return true;
#endif
    return false;
}

void SwDoc::ChangeAuthorityData( const SwAuthEntry* pNewData )
{
    const SwFieldTypes::size_type nSize = getIDocumentFieldsAccess().GetFieldTypes()->size();

    for( SwFieldTypes::size_type i = INIT_FLDTYPES; i < nSize; ++i )
    {
        SwFieldType* pFieldType = (*getIDocumentFieldsAccess().GetFieldTypes())[i];
        if( SwFieldIds::TableOfAuthorities  == pFieldType->Which() )
        {
            SwAuthorityFieldType* pAuthType = static_cast<SwAuthorityFieldType*>(pFieldType);
            pAuthType->ChangeEntryContent(pNewData);
            break;
        }
    }

}

void SwDocUpdateField::InsDelFieldInFieldLst( bool bIns, const SwTextField& rField )
{
    const SwFieldIds nWhich = rField.GetFormatField().GetField()->GetTyp()->Which();
    switch( nWhich )
    {
    case SwFieldIds::Database:
    case SwFieldIds::SetExp:
    case SwFieldIds::HiddenPara:
    case SwFieldIds::HiddenText:
    case SwFieldIds::DbNumSet:
    case SwFieldIds::DbNextSet:
    case SwFieldIds::DbSetNumber:
    case SwFieldIds::GetExp:
        break;          // these have to be added/removed!

    default:
        return;
    }

    SetFieldsDirty( true );
    if (!m_pFieldSortList)
    {
        if( !bIns )             // if list is present and deleted
            return;             // don't do a thing
        m_pFieldSortList.reset(new SetGetExpFields);
    }

    if( bIns )      // insert anew:
        GetBodyNode( rField, nWhich );
    else
    {
        // look up via the pTextField pointer. It is a sorted list, but it's sorted by node
        // position. Until this is found, the search for the pointer is already done.
        for (SetGetExpFields::size_type n = 0; n < m_pFieldSortList->size(); ++n)
        {
            if (&rField == (*m_pFieldSortList)[n]->GetPointer())
            {
                m_pFieldSortList->erase(n);
                n--; // one field can occur multiple times
            }
        }
    }
}

void SwDocUpdateField::MakeFieldList( SwDoc& rDoc, bool bAll, int eGetMode )
{
    if (!m_pFieldSortList || bAll
        || ((eGetMode & m_nFieldListGetMode) != eGetMode)
        || rDoc.GetNodes().Count() != m_nNodes)
    {
        MakeFieldList_( rDoc, eGetMode );
    }
}

void SwDocUpdateField::MakeFieldList_( SwDoc& rDoc, int eGetMode )
{
    // new version: walk all fields of the attribute pool
    m_pFieldSortList.reset(new SetGetExpFields);

    // consider and unhide sections
    //     with hide condition, only in mode GETFLD_ALL (<eGetMode == GETFLD_ALL>)
    //     notes by OD:
    //         eGetMode == GETFLD_CALC in call from methods SwDoc::FieldsToCalc
    //         eGetMode == GETFLD_EXPAND in call from method SwDoc::FieldsToExpand
    //         eGetMode == GETFLD_ALL in call from method SwDoc::UpdateExpFields
    //         I figured out that hidden section only have to be shown,
    //         if fields have updated (call by SwDoc::UpdateExpFields) and thus
    //         the hide conditions of section have to be updated.
    //         For correct updating the hide condition of a section, its position
    //         have to be known in order to insert the hide condition as a new
    //         expression field into the sorted field list (<m_pFieldSortList>).
    if ( eGetMode == GETFLD_ALL )
    // Collect the sections first. Supply sections that are hidden by condition
    // with frames so that the contained fields are sorted properly.
    {
        // In order for the frames to be created the right way, they have to be expanded
        // from top to bottom
        std::vector<sal_uLong> aTmpArr;
        std::vector<sal_uLong>::size_type nArrStt = 0;
        SwSectionFormats& rArr = rDoc.GetSections();
        SwSectionNode* pSectNd = nullptr;
        sal_uLong nSttContent = rDoc.GetNodes().GetEndOfExtras().GetIndex();

        for (SwSectionFormats::size_type n = rArr.size(); n; )
        {
            SwSection* pSect = rArr[ --n ]->GetSection();
            if( pSect && pSect->IsHidden() && !pSect->GetCondition().isEmpty() &&
                nullptr != ( pSectNd = pSect->GetFormat()->GetSectionNode() ))
            {
                sal_uLong nIdx = pSectNd->GetIndex();
                aTmpArr.push_back( nIdx );
                if( nIdx < nSttContent )
                    ++nArrStt;
            }
        }
        std::sort(aTmpArr.begin(), aTmpArr.end());

        // Display all first so that we have frames. The BodyAnchor is defined by that.
        // First the ContentArea, then the special areas!
        for (std::vector<sal_uLong>::size_type n = nArrStt; n < aTmpArr.size(); ++n)
        {
            pSectNd = rDoc.GetNodes()[ aTmpArr[ n ] ]->GetSectionNode();
            OSL_ENSURE( pSectNd, "Where is my SectionNode" );
            pSectNd->GetSection().SetCondHidden( false );
        }
        for (std::vector<sal_uLong>::size_type n = 0; n < nArrStt; ++n)
        {
            pSectNd = rDoc.GetNodes()[ aTmpArr[ n ] ]->GetSectionNode();
            OSL_ENSURE( pSectNd, "Where is my SectionNode" );
            pSectNd->GetSection().SetCondHidden( false );
        }

        // add all to the list so that they are sorted
        for (const auto &nId : aTmpArr)
        {
            GetBodyNode( *rDoc.GetNodes()[ nId ]->GetSectionNode() );
        }
    }

    const OUString sTrue("TRUE");
    const OUString sFalse("FALSE");

#if HAVE_FEATURE_DBCONNECTIVITY
    bool bIsDBManager = nullptr != rDoc.GetDBManager();
#endif

    const sal_uInt32 nMaxItems = rDoc.GetAttrPool().GetItemCount2( RES_TXTATR_FIELD );
    for( sal_uInt32 n = 0; n < nMaxItems; ++n )
    {
        const SfxPoolItem* pItem = rDoc.GetAttrPool().GetItem2( RES_TXTATR_FIELD, n );
        if( !pItem )
            continue;

        const SwFormatField* pFormatField = static_cast<const SwFormatField*>(pItem);
        const SwTextField* pTextField = pFormatField->GetTextField();
        if( !pTextField || !pTextField->GetTextNode().GetNodes().IsDocNodes() )
            continue;

        OUString sFormula;
        const SwField* pField = pFormatField->GetField();
        const SwFieldIds nWhich = pField->GetTyp()->Which();
        switch( nWhich )
        {
            case SwFieldIds::DbSetNumber:
            case SwFieldIds::GetExp:
                if( GETFLD_ALL == eGetMode )
                    sFormula = sTrue;
                break;

            case SwFieldIds::Database:
                if( GETFLD_EXPAND & eGetMode )
                    sFormula = sTrue;
                break;

            case SwFieldIds::SetExp:
                if ( (eGetMode != GETFLD_EXPAND) ||
                     (nsSwGetSetExpType::GSE_STRING & pField->GetSubType()) )
                {
                    sFormula = sTrue;
                }
                break;

            case SwFieldIds::HiddenPara:
                if( GETFLD_ALL == eGetMode )
                {
                    sFormula = pField->GetPar1();
                    if (sFormula.isEmpty() || sFormula==sFalse)
                        const_cast<SwHiddenParaField*>(static_cast<const SwHiddenParaField*>(pField))->SetHidden( false );
                    else if (sFormula==sTrue)
                        const_cast<SwHiddenParaField*>(static_cast<const SwHiddenParaField*>(pField))->SetHidden( true );
                    else
                        break;

                    sFormula.clear();
                    // trigger formatting
                    const_cast<SwFormatField*>(pFormatField)->ModifyNotification( nullptr, nullptr );
                }
                break;

            case SwFieldIds::HiddenText:
                if( GETFLD_ALL == eGetMode )
                {
                    sFormula = pField->GetPar1();
                    if (sFormula.isEmpty() || sFormula==sFalse)
                        const_cast<SwHiddenTextField*>(static_cast<const SwHiddenTextField*>(pField))->SetValue( true );
                    else if (sFormula==sTrue)
                        const_cast<SwHiddenTextField*>(static_cast<const SwHiddenTextField*>(pField))->SetValue( false );
                    else
                        break;

                    sFormula.clear();

                    // evaluate field
                    const_cast<SwHiddenTextField*>(static_cast<const SwHiddenTextField*>(pField))->Evaluate(&rDoc);
                    // trigger formatting
                    const_cast<SwFormatField*>(pFormatField)->ModifyNotification( nullptr, nullptr );
                }
                break;

#if HAVE_FEATURE_DBCONNECTIVITY
            case SwFieldIds::DbNumSet:
            {
                SwDBData aDBData(const_cast<SwDBNumSetField*>(static_cast<const SwDBNumSetField*>(pField))->GetDBData(&rDoc));

                if (
                     (bIsDBManager && rDoc.GetDBManager()->OpenDataSource(aDBData.sDataSource, aDBData.sCommand)) &&
                     (GETFLD_ALL == eGetMode || (GETFLD_CALC & eGetMode && static_cast<const SwDBNumSetField*>(pField)->IsCondValid()))
                   )
                {
                    sFormula = pField->GetPar1();
                }
            }
            break;
            case SwFieldIds::DbNextSet:
            {
                SwDBData aDBData(const_cast<SwDBNextSetField*>(static_cast<const SwDBNextSetField*>(pField))->GetDBData(&rDoc));

                if (
                     (bIsDBManager && rDoc.GetDBManager()->OpenDataSource(aDBData.sDataSource, aDBData.sCommand)) &&
                     (GETFLD_ALL == eGetMode || (GETFLD_CALC & eGetMode && static_cast<const SwDBNextSetField*>(pField)->IsCondValid()))
                   )
                {
                    sFormula = pField->GetPar1();
                }
            }
            break;
#endif
            default: break;
        }

        if (!sFormula.isEmpty())
        {
            GetBodyNode( *pTextField, nWhich );
        }
    }
    m_nFieldListGetMode = eGetMode;
    m_nNodes = rDoc.GetNodes().Count();
}

void SwDocUpdateField::GetBodyNode( const SwTextField& rTField, SwFieldIds nFieldWhich )
{
    const SwTextNode& rTextNd = rTField.GetTextNode();
    const SwDoc& rDoc = *rTextNd.GetDoc();

    // always the first! (in tab headline, header-/footer)
    Point aPt;
    std::pair<Point, bool> const tmp(aPt, false);
    const SwContentFrame* pFrame = rTextNd.getLayoutFrame(
        rDoc.getIDocumentLayoutAccess().GetCurrentLayout(), nullptr, &tmp);

    std::unique_ptr<SetGetExpField> pNew;
    bool bIsInBody = false;

    if( !pFrame || pFrame->IsInDocBody() )
    {
        // create index to determine the TextNode
        SwNodeIndex aIdx( rTextNd );
        bIsInBody = rDoc.GetNodes().GetEndOfExtras().GetIndex() < aIdx.GetIndex();

        // We don't want to update fields in redlines, or those
        // in frames whose anchor is in redline. However, we do want to update
        // fields in hidden sections. So: In order to be updated, a field 1)
        // must have a frame, or 2) it must be in the document body.
        if( (pFrame != nullptr) || bIsInBody )
            pNew.reset(new SetGetExpField( aIdx, &rTField ));
    }
    else
    {
        // create index to determine the TextNode
        SwPosition aPos( rDoc.GetNodes().GetEndOfPostIts() );
        bool const bResult = GetBodyTextNode( rDoc, aPos, *pFrame );
        OSL_ENSURE(bResult, "where is the Field");
        pNew.reset(new SetGetExpField( aPos.nNode, &rTField, &aPos.nContent ));
    }

    // always set the BodyTextFlag in GetExp or DB fields
    if( SwFieldIds::GetExp == nFieldWhich )
    {
        SwGetExpField* pGetField = const_cast<SwGetExpField*>(static_cast<const SwGetExpField*>(rTField.GetFormatField().GetField()));
        pGetField->ChgBodyTextFlag( bIsInBody );
    }
#if HAVE_FEATURE_DBCONNECTIVITY
    else if( SwFieldIds::Database == nFieldWhich )
    {
        SwDBField* pDBField = const_cast<SwDBField*>(static_cast<const SwDBField*>(rTField.GetFormatField().GetField()));
        pDBField->ChgBodyTextFlag( bIsInBody );
    }
#endif
    if( pNew != nullptr )
        m_pFieldSortList->insert( std::move(pNew) );
}

void SwDocUpdateField::GetBodyNode( const SwSectionNode& rSectNd )
{
    const SwDoc& rDoc = *rSectNd.GetDoc();
    std::unique_ptr<SetGetExpField> pNew;

    if( rSectNd.GetIndex() < rDoc.GetNodes().GetEndOfExtras().GetIndex() )
    {
        do {            // middle check loop

            // we need to get the anchor first
            // create index to determine the TextNode
            SwPosition aPos( rSectNd );
            SwContentNode* pCNd = rDoc.GetNodes().GoNext( &aPos.nNode ); // to the next ContentNode

            if( !pCNd || !pCNd->IsTextNode() )
                break;

            // always the first! (in tab headline, header-/footer)
            Point aPt;
            std::pair<Point, bool> const tmp(aPt, false);
            const SwContentFrame* pFrame = pCNd->getLayoutFrame(
                    rDoc.getIDocumentLayoutAccess().GetCurrentLayout(),
                    nullptr, &tmp);
            if( !pFrame )
                break;

            bool const bResult = GetBodyTextNode( rDoc, aPos, *pFrame );
            OSL_ENSURE(bResult, "where is the Field");
            pNew.reset(new SetGetExpField( rSectNd, &aPos ));

        } while( false );
    }

    if( !pNew )
        pNew.reset(new SetGetExpField( rSectNd ));

    m_pFieldSortList->insert( std::move(pNew) );
}

void SwDocUpdateField::InsertFieldType( const SwFieldType& rType )
{
    OUString sFieldName;
    switch( rType.Which() )
    {
    case SwFieldIds::User :
        sFieldName = static_cast<const SwUserFieldType&>(rType).GetName();
        break;
    case SwFieldIds::SetExp:
        sFieldName = static_cast<const SwSetExpFieldType&>(rType).GetName();
        break;
    default:
        OSL_ENSURE( false, "No valid field type" );
    }

    if( !sFieldName.isEmpty() )
    {
        SetFieldsDirty( true );
        // look up and remove from the hash table
        sFieldName = GetAppCharClass().lowercase( sFieldName );
        sal_uInt16 n;

        SwCalcFieldType* pFnd = GetFieldTypeTable().Find( sFieldName, &n );

        if( !pFnd )
        {
            SwCalcFieldType* pNew = new SwCalcFieldType( sFieldName, &rType );
            pNew->pNext.reset( m_FieldTypeTable[n].release() );
            m_FieldTypeTable[n].reset(pNew);
        }
    }
}

void SwDocUpdateField::RemoveFieldType( const SwFieldType& rType )
{
    OUString sFieldName;
    switch( rType.Which() )
    {
    case SwFieldIds::User :
        sFieldName = static_cast<const SwUserFieldType&>(rType).GetName();
        break;
    case SwFieldIds::SetExp:
        sFieldName = static_cast<const SwSetExpFieldType&>(rType).GetName();
        break;
    default: break;
    }

    if( !sFieldName.isEmpty() )
    {
        SetFieldsDirty( true );
        // look up and remove from the hash table
        sFieldName = GetAppCharClass().lowercase( sFieldName );
        sal_uInt16 n;

        SwCalcFieldType* pFnd = GetFieldTypeTable().Find( sFieldName, &n );
        if( pFnd )
        {
            if (m_FieldTypeTable[n].get() == pFnd)
            {
                m_FieldTypeTable[n].reset(static_cast<SwCalcFieldType*>(pFnd->pNext.release()));
            }
            else
            {
                SwHash* pPrev = m_FieldTypeTable[n].get();
                while( pPrev->pNext.get() != pFnd )
                    pPrev = pPrev->pNext.get();
                pPrev->pNext = std::move(pFnd->pNext);
                // no need to explicitly delete here, the embedded linked list uses unique_ptr
            }
        }
    }
}

SwDocUpdateField::SwDocUpdateField(SwDoc& rDoc)
    : m_FieldTypeTable(TBLSZ)
    , m_nNodes(0)
    , m_nFieldListGetMode(0)
    , m_rDoc(rDoc)
    , m_bInUpdateFields(false)
    , m_bFieldsDirty(false)
{
}

SwDocUpdateField::~SwDocUpdateField()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

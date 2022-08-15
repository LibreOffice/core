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
#include <config_fuzzers.h>

#include <hintids.hxx>

#include <comphelper/string.hxx>
#include <osl/diagnose.h>
#include <unotools/charclass.hxx>
#ifndef UNX
#include <unotools/transliterationwrapper.hxx>
#endif
#include <o3tl/string_view.hxx>
#include <doc.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentMarkAccess.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <node2lay.hxx>
#include <cntfrm.hxx>
#include <pagefrm.hxx>
#include <txtfrm.hxx>
#include <notxtfrm.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <swtable.hxx>
#include <calc.hxx>
#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <txttxmrk.hxx>
#include <docfld.hxx>
#include <docufld.hxx>
#include <usrfld.hxx>
#include <expfld.hxx>
#include <dbfld.hxx>
#include <reffld.hxx>
#include <dbmgr.hxx>
#include <section.hxx>
#include <docary.hxx>
#include <authfld.hxx>
#include <txtinet.hxx>
#include <fmtcntnt.hxx>
#include <utility>

using namespace ::com::sun::star::uno;

// the StartIndex can be supplied optionally (e.g. if it was queried before - is a virtual
// method otherwise!)
SetGetExpField::SetGetExpField(
    const SwNode& rNdIdx,
    const SwTextField* pField,
    std::optional<sal_Int32> oContentIdx,
    sal_uInt16 const nPageNumber)
    : m_nPageNumber(nPageNumber)
{
    m_eSetGetExpFieldType = TEXTFIELD;
    m_CNTNT.pTextField = pField;
    m_nNode = rNdIdx.GetIndex();
    if( oContentIdx )
        m_nContent = *oContentIdx;
    else if( pField )
        m_nContent = pField->GetStart();
    else
        m_nContent = 0;
}

SetGetExpField::SetGetExpField( const SwNode& rNdIdx,
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
                                const SwPosition* pPos,
                                sal_uInt16 const nPageNumber)
    : m_nPageNumber(nPageNumber)
{
    m_eSetGetExpFieldType = SECTIONNODE;
    m_CNTNT.pSection = &rSectNd.GetSection();

    if( pPos )
    {
        m_nNode = pPos->GetNodeIndex();
        m_nContent = pPos->GetContentIndex();
    }
    else
    {
        m_nNode = rSectNd.GetIndex();
        m_nContent = 0;
    }
}

SetGetExpField::SetGetExpField(::sw::mark::IBookmark const& rBookmark,
                               SwPosition const*const pPos,
                               sal_uInt16 const nPageNumber)
    : m_nPageNumber(nPageNumber)
{
    m_eSetGetExpFieldType = BOOKMARK;
    m_CNTNT.pBookmark = &rBookmark;

    if (pPos)
    {
        m_nNode = pPos->GetNodeIndex();
        m_nContent = pPos->GetContentIndex();
    }
    else
    {
        m_nNode = rBookmark.GetMarkStart().GetNodeIndex();
        m_nContent = rBookmark.GetMarkStart().GetContentIndex();;
    }
}

SetGetExpField::SetGetExpField( const SwTableBox& rTBox )
{
    m_eSetGetExpFieldType = TABLEBOX;
    m_CNTNT.pTBox = &rTBox;

    m_nNode = SwNodeOffset(0);
    m_nContent = 0;
    if( rTBox.GetSttNd() )
    {
        SwNodeIndex aIdx( *rTBox.GetSttNd() );
        const SwContentNode* pNd = aIdx.GetNode().GetNodes().GoNext( &aIdx );
        if( pNd )
            m_nNode = pNd->GetIndex();
    }
}

SetGetExpField::SetGetExpField( const SwNode& rNdIdx,
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
    m_nNode = rPos.GetNodeIndex();
    m_nContent = rPos.GetContentIndex();
}

SetGetExpField::SetGetExpField( const SwFlyFrameFormat& rFlyFormat,
                                const SwPosition* pPos  )
{
    m_eSetGetExpFieldType = FLYFRAME;
    m_CNTNT.pFlyFormat = &rFlyFormat;
    if( pPos )
    {
        m_nNode = pPos->GetNodeIndex();
        m_nContent = pPos->GetContentIndex();
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
        rPos.nContent.Assign( static_cast<const SwContentNode*>(pNd), GetCntPosFromContent() );
    }
    else
    {
        rPos.Assign( m_nNode, m_nContent );
    }
}

void SetGetExpField::SetBodyPos( const SwContentFrame& rFrame )
{
    if( !rFrame.IsInDocBody() )
    {
        SwNodeIndex aIdx( rFrame.IsTextFrame()
                ? *static_cast<SwTextFrame const&>(rFrame).GetTextNodeFirst()
                : *static_cast<SwNoTextFrame const&>(rFrame).GetNode() );
        SwDoc& rDoc = aIdx.GetNodes().GetDoc();
        SwPosition aPos( aIdx );
        bool const bResult = ::GetBodyTextNode( rDoc, aPos, rFrame );
        OSL_ENSURE(bResult, "Where is the field?");
        m_nNode = aPos.GetNodeIndex();
        m_nContent = aPos.GetContentIndex();
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
    if (m_nPageNumber != 0 && rField.m_nPageNumber != 0 && m_nPageNumber != rField.m_nPageNumber)
    {
        return m_nPageNumber < rField.m_nPageNumber;
    }
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

        pTableNd = pNext->FindTableNode();
        if( pTableNd )
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

        case BOOKMARK:
            pRet = &m_CNTNT.pBookmark->GetMarkStart().GetNode();
            break;

        case CRSRPOS:
            pRet = &m_CNTNT.pPos->GetNode();
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
        case BOOKMARK:
            nRet = m_CNTNT.pBookmark->GetMarkStart().GetContentIndex();
            break;
        case CRSRPOS:
            nRet =  m_CNTNT.pPos->GetContentIndex();
            break;
        default:
            break;
        }
    return nRet;
}

HashStr::HashStr( const OUString& rName, OUString aText,
                    HashStr* pNxt )
    : SwHash( rName ), aSetStr(std::move( aText ))
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
#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
    if(maDBData.sDataSource.isEmpty())
    {
        // Similar to: SwEditShell::IsAnyDatabaseFieldInDoc
        for (const auto& pFieldType : *getIDocumentFieldsAccess().GetFieldTypes())
        {
            if (IsUsed(*pFieldType))
            {
                SwFieldIds nWhich = pFieldType->Which();
                switch(nWhich)
                {
                    case SwFieldIds::Database:
                    case SwFieldIds::DbNextSet:
                    case SwFieldIds::DbNumSet:
                    case SwFieldIds::DbSetNumber:
                    {
                        std::vector<SwFormatField*> vFields;
                        pFieldType->GatherFields(vFields);
                        if(vFields.size())
                        {
                            if(SwFieldIds::Database == nWhich)
                                maDBData = static_cast<SwDBFieldType*>(vFields.front()->GetField()->GetTyp())->GetDBData();
                            else
                                maDBData = static_cast<SwDBNameInfField*> (vFields.front()->GetField())->GetRealDBData();
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
#if !HAVE_FEATURE_DBCONNECTIVITY || ENABLE_FUZZERS
    (void) b;
#else
    GetDBManager()->SetInitDBFields( b );
#endif
}

#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS

/// Get all databases that are used by fields
static OUString lcl_DBDataToString(const SwDBData& rData)
{
    return rData.sDataSource + OUStringChar(DB_DELIM)
        + rData.sCommand + OUStringChar(DB_DELIM)
        + OUString::number(rData.nCommandType);
}

#endif

void SwDoc::GetAllUsedDB( std::vector<OUString>& rDBNameList,
                          const std::vector<OUString>* pAllDBNames )
{
#if !HAVE_FEATURE_DBCONNECTIVITY || ENABLE_FUZZERS
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

    for (sal_uInt16 const nWhichHint : { RES_TXTATR_FIELD, RES_TXTATR_INPUTFIELD })
    {
        for (const SfxPoolItem* pItem : GetAttrPool().GetItemSurrogates(nWhichHint))
        {
            const SwFormatField* pFormatField = static_cast<const SwFormatField*>(pItem);
            const SwTextField* pTextField = pFormatField->GetTextField();
            if (!pTextField || !pTextField->GetTextNode().GetNodes().IsDocNodes())
                continue;

            const SwField* pField = pFormatField->GetField();
            switch (pField->GetTyp()->Which())
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
    }
#endif
}

void SwDoc::GetAllDBNames( std::vector<OUString>& rAllDBNames )
{
#if !HAVE_FEATURE_DBCONNECTIVITY || ENABLE_FUZZERS
    (void) rAllDBNames;
#else
    SwDBManager* pMgr = GetDBManager();

    const SwDSParams_t& rArr = pMgr->GetDSParamArray();
    for (const auto& pParam : rArr)
    {
        rAllDBNames.emplace_back(pParam->sDataSource + OUStringChar(DB_DELIM) + pParam->sCommand);
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
                rUsedDBNames.emplace_back(sItem + OUStringChar(DB_DELIM) + sFormula.subView( nPos, nEndPos - nPos ));
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
#if !HAVE_FEATURE_DBCONNECTIVITY || ENABLE_FUZZERS
    (void) rDBNameList;
    (void) rDBName;
#else
    if( rDBName.isEmpty() )
        return;

#ifdef UNX
    for( const auto &sName : rDBNameList )
        if( rDBName == o3tl::getToken(sName, 0, ';') )
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
#if !HAVE_FEATURE_DBCONNECTIVITY || ENABLE_FUZZERS
    (void) rOldNames;
    (void) rNewName;
#else
    SwDBData aNewDBData;
    sal_Int32 nIdx{ 0 };
    aNewDBData.sDataSource = rNewName.getToken(0, DB_DELIM, nIdx);
    aNewDBData.sCommand = rNewName.getToken(0, DB_DELIM, nIdx);
    aNewDBData.nCommandType = o3tl::toInt32(o3tl::getToken(rNewName, 0, DB_DELIM, nIdx));

    SwSectionFormats& rArr = GetSections();
    for (auto n = rArr.size(); n; )
    {
        SwSection* pSect = rArr[ --n ]->GetSection();

        if( pSect )
        {
            pSect->SetCondition(ReplaceUsedDBs(rOldNames, rNewName, pSect->GetCondition()));
        }
    }

    for (sal_uInt16 const nWhichHint : { RES_TXTATR_FIELD, RES_TXTATR_INPUTFIELD })
    {
        for (const SfxPoolItem* pItem : GetAttrPool().GetItemSurrogates(nWhichHint))
        {
            SwFormatField* pFormatField = const_cast<SwFormatField*>(static_cast<const SwFormatField*>(pItem));
            SwTextField* pTextField = pFormatField->GetTextField();
            if (!pTextField || !pTextField->GetTextNode().GetNodes().IsDocNodes())
                continue;

            SwField* pField = pFormatField->GetField();
            bool bExpand = false;

            switch( pField->GetTyp()->Which() )
            {
                case SwFieldIds::Database:
#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
                    if (IsNameInArray(rOldNames, lcl_DBDataToString(static_cast<SwDBField*>(pField)->GetDBData())))
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
                    if (IsNameInArray(rOldNames,
                                lcl_DBDataToString(static_cast<SwDBNameInfField*>(pField)->GetRealDBData())))
                    {
                        static_cast<SwDBNameInfField*>(pField)->SetDBData(aNewDBData);
                        bExpand = true;
                    }
                    break;

                case SwFieldIds::DbNumSet:
                case SwFieldIds::DbNextSet:
                    if (IsNameInArray(rOldNames,
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
    }
    getIDocumentState().SetModified();
#endif
}

namespace
{

OUString lcl_CutOffDBCommandType(const OUString& rName)
{
    return rName.replaceFirst(OUStringChar(DB_DELIM), ".").getToken(0, DB_DELIM);
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
        SwFieldType* pFieldType = (*getIDocumentFieldsAccess().GetFieldTypes())[i].get();
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
                m_pFieldSortList->erase_at(n);
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

    // remember sections that were unhidden and need to be hidden again
    std::vector<std::reference_wrapper<SwSection>> aUnhiddenSections;

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
        std::vector<SwNodeOffset> aTmpArr;
        std::vector<SwNodeOffset>::size_type nArrStt = 0;
        SwSectionFormats& rArr = rDoc.GetSections();
        SwSectionNode* pSectNd = nullptr;
        SwNodeOffset nSttContent = rDoc.GetNodes().GetEndOfExtras().GetIndex();

        for (SwSectionFormats::size_type n = rArr.size(); n; )
        {
            SwSection* pSect = rArr[ --n ]->GetSection();
            if( !pSect || !pSect->IsHidden() || pSect->GetCondition().isEmpty() )
                continue;
            pSectNd = pSect->GetFormat()->GetSectionNode();
            if( pSectNd )
            {
                SwNodeOffset nIdx = pSectNd->GetIndex();
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

            auto& rSection = pSectNd->GetSection();
            // unhide and remember the conditionally hidden sections
            if (rSection.IsHidden() && !rSection.GetCondition().isEmpty() && rSection.IsCondHidden())
            {
                aUnhiddenSections.push_back(std::ref(rSection)); // remember to later hide again
                rSection.SetCondHidden(false);
            }
        }
        for (std::vector<sal_uLong>::size_type n = 0; n < nArrStt; ++n)
        {
            pSectNd = rDoc.GetNodes()[ aTmpArr[ n ] ]->GetSectionNode();
            OSL_ENSURE( pSectNd, "Where is my SectionNode" );

            auto& rSection = pSectNd->GetSection();
            // unhide and remember the conditionally hidden sections
            if (rSection.IsHidden() && !rSection.GetCondition().isEmpty() && rSection.IsCondHidden())
            {
                aUnhiddenSections.push_back(std::ref(rSection)); // remember to later hide again
                rSection.SetCondHidden(false);
            }
        }

        // add all to the list so that they are sorted
        for (const auto &nId : aTmpArr)
        {
            SwSectionNode const& rSectionNode(*rDoc.GetNodes()[ nId ]->GetSectionNode());
            GetBodyNodeGeneric(rSectionNode, rSectionNode);
        }

        // bookmarks with hide conditions, handle similar to sections
        auto const& rIDMA(*rDoc.getIDocumentMarkAccess());
        for (auto it = rIDMA.getBookmarksBegin(); it != rIDMA.getBookmarksEnd(); ++it)
        {
            auto const pBookmark(dynamic_cast<::sw::mark::IBookmark const*>(*it));
            assert(pBookmark);
            if (!pBookmark->GetHideCondition().isEmpty())
            {
                GetBodyNodeGeneric((*it)->GetMarkStart().GetNode(), *pBookmark);
            }
        }
    }

    static const OUStringLiteral sTrue(u"TRUE");
    static const OUStringLiteral sFalse(u"FALSE");

#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
    bool bIsDBManager = nullptr != rDoc.GetDBManager();
#endif

    for (sal_uInt16 const nWhichHint : { RES_TXTATR_FIELD, RES_TXTATR_INPUTFIELD })
    {
        for (const SfxPoolItem* pItem : rDoc.GetAttrPool().GetItemSurrogates(nWhichHint))
        {
            const SwFormatField* pFormatField = static_cast<const SwFormatField*>(pItem);
            const SwTextField* pTextField = pFormatField->GetTextField();
            if (!pTextField || !pTextField->GetTextNode().GetNodes().IsDocNodes())
                continue;

            OUString sFormula;
            const SwField* pField = pFormatField->GetField();
            const SwFieldIds nWhich = pField->GetTyp()->Which();
            switch (nWhich)
            {
                case SwFieldIds::DbSetNumber:
                case SwFieldIds::GetExp:
                    if (GETFLD_ALL == eGetMode)
                        sFormula = sTrue;
                    break;

                case SwFieldIds::Database:
                    if (GETFLD_EXPAND & eGetMode)
                        sFormula = sTrue;
                    break;

                case SwFieldIds::SetExp:
                    if ((eGetMode != GETFLD_EXPAND) ||
                        (nsSwGetSetExpType::GSE_STRING & pField->GetSubType()))
                    {
                        sFormula = sTrue;
                    }
                    break;

                case SwFieldIds::HiddenPara:
                    if (GETFLD_ALL == eGetMode)
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
                        const_cast<SwFormatField*>(pFormatField)->UpdateTextNode( nullptr, nullptr );
                    }
                    break;

                case SwFieldIds::HiddenText:
                    if (GETFLD_ALL == eGetMode)
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
                        const_cast<SwHiddenTextField*>(static_cast<const SwHiddenTextField*>(pField))->Evaluate(rDoc);
                        // trigger formatting
                        const_cast<SwFormatField*>(pFormatField)->UpdateTextNode(nullptr, nullptr);
                    }
                    break;

#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
                case SwFieldIds::DbNumSet:
                {
                    SwDBData aDBData(const_cast<SwDBNumSetField*>(static_cast<const SwDBNumSetField*>(pField))->GetDBData(&rDoc));

                    if (   (bIsDBManager && rDoc.GetDBManager()->OpenDataSource(aDBData.sDataSource, aDBData.sCommand))
                        && (GETFLD_ALL == eGetMode
                            || (GETFLD_CALC & eGetMode
                                && static_cast<const SwDBNumSetField*>(pField)->IsCondValid()))
                       )
                    {
                        sFormula = pField->GetPar1();
                    }
                }
                break;
                case SwFieldIds::DbNextSet:
                {
                    SwDBData aDBData(const_cast<SwDBNextSetField*>(static_cast<const SwDBNextSetField*>(pField))->GetDBData(&rDoc));

                    if (   (bIsDBManager && rDoc.GetDBManager()->OpenDataSource(aDBData.sDataSource, aDBData.sCommand))
                        && (GETFLD_ALL == eGetMode
                            || (GETFLD_CALC & eGetMode
                                && static_cast<const SwDBNextSetField*>(pField)->IsCondValid()))
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
    }
    m_nFieldListGetMode = eGetMode;
    m_nNodes = rDoc.GetNodes().Count();

    // return the conditional hidden value back to the previous value
    for (auto& rSectionWrapper : aUnhiddenSections)
    {
        auto& rSection = rSectionWrapper.get();
        rSection.SetCondHidden(true);
    }
}

void SwDocUpdateField::GetBodyNode( const SwTextField& rTField, SwFieldIds nFieldWhich )
{
    const SwTextNode& rTextNd = rTField.GetTextNode();
    const SwDoc& rDoc = rTextNd.GetDoc();

    // always the first! (in tab headline, header-/footer)
    Point aPt;
    std::pair<Point, bool> const tmp(aPt, false);
    // need pos to get the frame on the correct page
    SwPosition const pos(rTextNd, rTField.GetStart());
    const SwFrame* pFrame = rTextNd.getLayoutFrame(
        rDoc.getIDocumentLayoutAccess().GetCurrentLayout(), &pos, &tmp);

    std::unique_ptr<SetGetExpField> pNew;
    bool bIsInBody = false;

    if( !pFrame || pFrame->IsInDocBody() )
    {
        bIsInBody = rDoc.GetNodes().GetEndOfExtras().GetIndex() < rTextNd.GetIndex();

        // We don't want to update fields in redlines, or those
        // in frames whose anchor is in redline. However, we do want to update
        // fields in hidden sections. So: In order to be updated, a field 1)
        // must have a frame, or 2) it must be in the document body.
        if (pFrame == nullptr && bIsInBody)
        {   // try harder to get a frame for the page number
            pFrame = ::sw::FindNeighbourFrameForNode(rTextNd);
            // possibly there is no layout at all, happens in mail merge
        }
        if( (pFrame != nullptr) || bIsInBody )
        {
            pNew.reset(new SetGetExpField(rTextNd, &rTField, std::nullopt,
                pFrame ? pFrame->GetPhyPageNum() : 0));
        }
    }
    else
    {
        // create index to determine the TextNode
        SwPosition aPos( rDoc.GetNodes().GetEndOfPostIts() );
        bool const bResult = GetBodyTextNode( rDoc, aPos, *pFrame );
        OSL_ENSURE(bResult, "where is the Field");
        pNew.reset(new SetGetExpField(aPos.GetNode(), &rTField, aPos.GetContentIndex(),
            pFrame->GetPhyPageNum()));
    }

    // always set the BodyTextFlag in GetExp or DB fields
    if( SwFieldIds::GetExp == nFieldWhich )
    {
        SwGetExpField* pGetField = const_cast<SwGetExpField*>(static_cast<const SwGetExpField*>(rTField.GetFormatField().GetField()));
        pGetField->ChgBodyTextFlag( bIsInBody );
    }
#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
    else if( SwFieldIds::Database == nFieldWhich )
    {
        SwDBField* pDBField = const_cast<SwDBField*>(static_cast<const SwDBField*>(rTField.GetFormatField().GetField()));
        pDBField->ChgBodyTextFlag( bIsInBody );
    }
#endif
    if( pNew != nullptr )
        m_pFieldSortList->insert( std::move(pNew) );
}

template<typename T>
void SwDocUpdateField::GetBodyNodeGeneric(SwNode const& rNode, T const& rCond)
{
    const SwDoc& rDoc = rNode.GetDoc();
    std::unique_ptr<SetGetExpField> pNew;

    if (rNode.GetIndex() < rDoc.GetNodes().GetEndOfExtras().GetIndex())
    {
        do {            // middle check loop

            // we need to get the anchor first
            // create index to determine the TextNode
            SwPosition aPos(rNode);
            SwContentNode const*const pCNd = rNode.IsSectionNode()
                ? rDoc.GetNodes().GoNext(&aPos.nNode) // to the next ContentNode
                : rNode.GetContentNode();

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
            pNew.reset(new SetGetExpField(rCond, &aPos, pFrame->GetPhyPageNum()));

        } while( false );
    }

    if( !pNew )
    {
        // try harder to get a frame for the page number
        SwFrame const*const pFrame = ::sw::FindNeighbourFrameForNode(rNode);
        pNew.reset(new SetGetExpField(rCond, nullptr, pFrame ? pFrame->GetPhyPageNum() : 0));
    }

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

    if( sFieldName.isEmpty() )
        return;

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

    if( sFieldName.isEmpty() )
        return;

    SetFieldsDirty( true );
    // look up and remove from the hash table
    sFieldName = GetAppCharClass().lowercase( sFieldName );
    sal_uInt16 n;

    SwCalcFieldType* pFnd = GetFieldTypeTable().Find( sFieldName, &n );
    if( !pFnd )
        return;

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

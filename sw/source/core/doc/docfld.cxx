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
#include <poolfmt.hrc>

#include <SwUndoField.hxx>
#include <calbck.hxx>

using namespace ::com::sun::star::uno;

// the StartIndex can be supplied optionally (e.g. if it was queried before - is a virtual
// method otherwise!)
_SetGetExpField::_SetGetExpField(
    const SwNodeIndex& rNdIdx,
    const SwTextField* pField,
    const SwIndex* pIdx )
{
    eSetGetExpFieldType = TEXTFIELD;
    CNTNT.pTextField = pField;
    nNode = rNdIdx.GetIndex();
    if( pIdx )
        nContent = pIdx->GetIndex();
    else if( pField )
        nContent = pField->GetStart();
    else
        nContent = 0;
}

_SetGetExpField::_SetGetExpField( const SwNodeIndex& rNdIdx,
                            const SwTextINetFormat& rINet, const SwIndex* pIdx )
{
    eSetGetExpFieldType = TEXTINET;
    CNTNT.pTextINet = &rINet;
    nNode = rNdIdx.GetIndex();
    if( pIdx )
        nContent = pIdx->GetIndex();
    else
        nContent = rINet.GetStart();
}

// Extension for Sections:
// these always have content position 0xffffffff!
// There is never a field on this, only up to COMPLETE_STRING possible
_SetGetExpField::_SetGetExpField( const SwSectionNode& rSectNd,
                                const SwPosition* pPos )
{
    eSetGetExpFieldType = SECTIONNODE;
    CNTNT.pSection = &rSectNd.GetSection();

    if( pPos )
    {
        nNode = pPos->nNode.GetIndex();
        nContent = pPos->nContent.GetIndex();
    }
    else
    {
        nNode = rSectNd.GetIndex();
        nContent = 0;
    }
}

_SetGetExpField::_SetGetExpField( const SwTableBox& rTBox, const SwPosition* pPos )
{
    eSetGetExpFieldType = TABLEBOX;
    CNTNT.pTBox = &rTBox;

    if( pPos )
    {
        nNode = pPos->nNode.GetIndex();
        nContent = pPos->nContent.GetIndex();
    }
    else
    {
        nNode = 0;
        nContent = 0;
        if( rTBox.GetSttNd() )
        {
            SwNodeIndex aIdx( *rTBox.GetSttNd() );
            const SwContentNode* pNd = aIdx.GetNode().GetNodes().GoNext( &aIdx );
            if( pNd )
                nNode = pNd->GetIndex();
        }
    }
}

_SetGetExpField::_SetGetExpField( const SwNodeIndex& rNdIdx,
                                const SwTextTOXMark& rTOX,
                                const SwIndex* pIdx )
{
    eSetGetExpFieldType = TEXTTOXMARK;
    CNTNT.pTextTOX = &rTOX;
    nNode = rNdIdx.GetIndex();
    if( pIdx )
        nContent = pIdx->GetIndex();
    else
        nContent = rTOX.GetStart();
}

_SetGetExpField::_SetGetExpField( const SwPosition& rPos )
{
    eSetGetExpFieldType = CRSRPOS;
    CNTNT.pPos = &rPos;
    nNode = rPos.nNode.GetIndex();
    nContent = rPos.nContent.GetIndex();
}

_SetGetExpField::_SetGetExpField( const SwFlyFrameFormat& rFlyFormat,
                                const SwPosition* pPos  )
{
    eSetGetExpFieldType = FLYFRAME;
    CNTNT.pFlyFormat = &rFlyFormat;
    if( pPos )
    {
        nNode = pPos->nNode.GetIndex();
        nContent = pPos->nContent.GetIndex();
    }
    else
    {
        const SwFormatContent& rContent = rFlyFormat.GetContent();
        nNode = rContent.GetContentIdx()->GetIndex() + 1;
        nContent = 0;
    }
}

void _SetGetExpField::GetPosOfContent( SwPosition& rPos ) const
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
        rPos.nNode = nNode;
        rPos.nContent.Assign( rPos.nNode.GetNode().GetContentNode(), nContent );
    }
}

void _SetGetExpField::SetBodyPos( const SwContentFrame& rFrame )
{
    if( !rFrame.IsInDocBody() )
    {
        SwNodeIndex aIdx( *rFrame.GetNode() );
        SwDoc& rDoc = *aIdx.GetNodes().GetDoc();
        SwPosition aPos( aIdx );
        bool const bResult = ::GetBodyTextNode( rDoc, aPos, rFrame );
        OSL_ENSURE(bResult, "Where is the field?");
        (void) bResult; // unused in non-debug
        nNode = aPos.nNode.GetIndex();
        nContent = aPos.nContent.GetIndex();
    }
}

bool _SetGetExpField::operator==( const _SetGetExpField& rField ) const
{
    return nNode == rField.nNode
           && nContent == rField.nContent
           && ( !CNTNT.pTextField
                || !rField.CNTNT.pTextField
                || CNTNT.pTextField == rField.CNTNT.pTextField );
}

bool _SetGetExpField::operator<( const _SetGetExpField& rField ) const
{
    if( nNode < rField.nNode || ( nNode == rField.nNode && nContent < rField.nContent ))
        return true;
    else if( nNode != rField.nNode || nContent != rField.nContent )
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
                return ::IsFrameBehind( *pNext->GetTextNode(), nContent, *pFirst->GetTextNode(), nContent );
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

const SwNode* _SetGetExpField::GetNodeFromContent() const
{
    const SwNode* pRet = nullptr;
    if( CNTNT.pTextField )
        switch( eSetGetExpFieldType )
        {
        case TEXTFIELD:
            pRet = &CNTNT.pTextField->GetTextNode();
            break;

        case TEXTINET:
            pRet = &CNTNT.pTextINet->GetTextNode();
            break;

        case SECTIONNODE:
            pRet = CNTNT.pSection->GetFormat()->GetSectionNode();
            break;

        case CRSRPOS:
            pRet = &CNTNT.pPos->nNode.GetNode();
            break;

        case TEXTTOXMARK:
            pRet = &CNTNT.pTextTOX->GetTextNode();
            break;

        case TABLEBOX:
            if( CNTNT.pTBox->GetSttNd() )
            {
                SwNodeIndex aIdx( *CNTNT.pTBox->GetSttNd() );
                pRet = aIdx.GetNode().GetNodes().GoNext( &aIdx );
            }
            break;

        case FLYFRAME:
            {
                SwNodeIndex aIdx( *CNTNT.pFlyFormat->GetContent().GetContentIdx() );
                pRet = aIdx.GetNode().GetNodes().GoNext( &aIdx );
            }
            break;
        }
    return pRet;
}

sal_Int32 _SetGetExpField::GetCntPosFromContent() const
{
    sal_Int32 nRet = 0;
    if( CNTNT.pTextField )
        switch( eSetGetExpFieldType )
        {
        case TEXTFIELD:
        case TEXTINET:
            nRet = CNTNT.pTextField->GetStart();
            break;
        case TEXTTOXMARK:
            nRet = CNTNT.pTextTOX->GetStart();
            break;
        case CRSRPOS:
            nRet =  CNTNT.pPos->nContent.GetIndex();
            break;
        default:
            break;
        }
    return nRet;
}

_HashStr::_HashStr( const OUString& rName, const OUString& rText,
                    _HashStr* pNxt )
    : SwHash( rName ), aSetStr( rText )
{
    pNext = pNxt;
}

/// Look up the Name, if it is present, return its String, otherwise return an empty String
OUString LookString( SwHash** ppTable, sal_uInt16 nSize, const OUString& rName,
                     sal_uInt16* pPos )
{
    SwHash* pFnd = Find( comphelper::string::strip(rName, ' '), ppTable, nSize, pPos );
    if( pFnd )
        return static_cast<_HashStr*>(pFnd)->aSetStr;

    return OUString();
}

SwDBData SwDoc::GetDBData()
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
            sal_uInt16 nWhich = rFieldType.Which();
            if(IsUsed(rFieldType))
            {
                switch(nWhich)
                {
                    case RES_DBFLD:
                    case RES_DBNEXTSETFLD:
                    case RES_DBNUMSETFLD:
                    case RES_DBSETNUMBERFLD:
                    {
                        SwIterator<SwFormatField,SwFieldType> aIter( rFieldType );
                        for( SwFormatField* pField = aIter.First(); pField; pField = aIter.Next() )
                        {
                            if(pField->IsFieldInDoc())
                            {
                                if(RES_DBFLD == nWhich)
                                    maDBData = (static_cast < SwDBFieldType * > (pField->GetField()->GetTyp()))->GetDBData();
                                else
                                    maDBData = (static_cast < SwDBNameInfField* > (pField->GetField()))->GetRealDBData();
                                break;
                            }
                        }
                    }
                    break;
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
    return rData.sDataSource + OUString(DB_DELIM)
        + rData.sCommand + OUString(DB_DELIM)
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
            case RES_DBFLD:
                AddUsedDBToList( rDBNameList,
                                lcl_DBDataToString(static_cast<const SwDBField*>(pField)->GetDBData() ));
                break;

            case RES_DBSETNUMBERFLD:
            case RES_DBNAMEFLD:
                AddUsedDBToList( rDBNameList,
                                lcl_DBDataToString(static_cast<const SwDBNameInfField*>(pField)->GetRealDBData() ));
                break;

            case RES_DBNUMSETFLD:
            case RES_DBNEXTSETFLD:
                AddUsedDBToList( rDBNameList,
                                lcl_DBDataToString(static_cast<const SwDBNameInfField*>(pField)->GetRealDBData() ));
                SAL_FALLTHROUGH; // JP: is that right like that?

            case RES_HIDDENTXTFLD:
            case RES_HIDDENPARAFLD:
                AddUsedDBToList(rDBNameList, FindUsedDBs( *pAllDBNames,
                                            pField->GetPar1(), aUsedDBNames ));
                aUsedDBNames.clear();
                break;

            case RES_SETEXPFLD:
            case RES_GETEXPFLD:
            case RES_TABLEFLD:
                AddUsedDBToList(rDBNameList, FindUsedDBs( *pAllDBNames,
                                        pField->GetFormula(), aUsedDBNames ));
                aUsedDBNames.clear();
                break;
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
        rAllDBNames.push_back(pParam->sDataSource + OUString(DB_DELIM) + pParam->sCommand);
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
                rUsedDBNames.push_back(sItem + OUString( DB_DELIM ) + sFormula.copy( nPos, nEndPos - nPos ));
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
    aData.sDataSource = rDBName.getToken(0, DB_DELIM);
    aData.sCommand = rDBName.getToken(1, DB_DELIM);
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
    aNewDBData.sDataSource = rNewName.getToken(0, DB_DELIM);
    aNewDBData.sCommand = rNewName.getToken(1, DB_DELIM);
    aNewDBData.nCommandType = (short)rNewName.getToken(2, DB_DELIM).toInt32();

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
            case RES_DBFLD:
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

            case RES_DBSETNUMBERFLD:
            case RES_DBNAMEFLD:
                if( IsNameInArray( rOldNames,
                                lcl_DBDataToString(static_cast<SwDBNameInfField*>(pField)->GetRealDBData())))
                {
                    static_cast<SwDBNameInfField*>(pField)->SetDBData(aNewDBData);
                    bExpand = true;
                }
                break;

            case RES_DBNUMSETFLD:
            case RES_DBNEXTSETFLD:
                if( IsNameInArray( rOldNames,
                                lcl_DBDataToString(static_cast<SwDBNameInfField*>(pField)->GetRealDBData())))
                {
                    static_cast<SwDBNameInfField*>(pField)->SetDBData(aNewDBData);
                }
                SAL_FALLTHROUGH;
            case RES_HIDDENTXTFLD:
            case RES_HIDDENPARAFLD:
                pField->SetPar1( ReplaceUsedDBs(rOldNames, rNewName, pField->GetPar1()) );
                bExpand = true;
                break;

            case RES_SETEXPFLD:
            case RES_GETEXPFLD:
            case RES_TABLEFLD:
                pField->SetPar2( ReplaceUsedDBs(rOldNames, rNewName, pField->GetFormula()) );
                bExpand = true;
                break;
        }

        if (bExpand)
            pTextField->ExpandTextField( true );
    }
    getIDocumentState().SetModified();
#endif
}

namespace
{

inline OUString lcl_CutOffDBCommandType(const OUString& rName)
{
    return rName.replaceFirst(OUString(DB_DELIM), ".").getToken(0, DB_DELIM);
}

}

OUString SwDoc::ReplaceUsedDBs( const std::vector<OUString>& rUsedDBNames,
                                const OUString& rNewName, const OUString& rFormula )
{
    const CharClass& rCC = GetAppCharClass();
    const OUString sNewName( lcl_CutOffDBCommandType(rNewName) );
    OUString sFormula(rFormula);

    for( size_t i = 0; i < rUsedDBNames.size(); ++i )
    {
        const OUString sDBName( lcl_CutOffDBCommandType(rUsedDBNames[i]) );

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
        if( RES_AUTHORITY  == pFieldType->Which() )
        {
            SwAuthorityFieldType* pAuthType = static_cast<SwAuthorityFieldType*>(pFieldType);
            pAuthType->ChangeEntryContent(pNewData);
            break;
        }
    }

}

void SwDocUpdateField::InsDelFieldInFieldLst( bool bIns, const SwTextField& rField )
{
    const sal_uInt16 nWhich = rField.GetFormatField().GetField()->GetTyp()->Which();
    switch( nWhich )
    {
    case RES_DBFLD:
    case RES_SETEXPFLD:
    case RES_HIDDENPARAFLD:
    case RES_HIDDENTXTFLD:
    case RES_DBNUMSETFLD:
    case RES_DBNEXTSETFLD:
    case RES_DBSETNUMBERFLD:
    case RES_GETEXPFLD:
        break;          // these have to be added/removed!

    default:
        return;
    }

    SetFieldsDirty( true );
    if( !pFieldSortLst )
    {
        if( !bIns )             // if list is present and deleted
            return;             // don't do a thing
        pFieldSortLst = new _SetGetExpFields;
    }

    if( bIns )      // insert anew:
        GetBodyNode( rField, nWhich );
    else
    {
        // look up via the pTextField pointer. It is a sorted list, but it's sorted by node
        // position. Until this is found, the search for the pointer is already done.
        for( _SetGetExpFields::size_type n = 0; n < pFieldSortLst->size(); ++n )
            if( &rField == (*pFieldSortLst)[ n ]->GetPointer() )
            {
                delete (*pFieldSortLst)[n];
                pFieldSortLst->erase(n);
                n--; // one field can occur multiple times
            }
    }
}

void SwDocUpdateField::MakeFieldList( SwDoc& rDoc, bool bAll, int eGetMode )
{
    if( !pFieldSortLst || bAll || !( eGetMode & nFieldLstGetMode ) ||
        rDoc.GetNodes().Count() != nNodes )
        _MakeFieldList( rDoc, eGetMode );
}

void SwDocUpdateField::_MakeFieldList( SwDoc& rDoc, int eGetMode )
{
    // new version: walk all fields of the attribute pool
    delete pFieldSortLst;
    pFieldSortLst = new _SetGetExpFields;

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
    //         expression field into the sorted field list (<pFieldSortLst>).
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
        const sal_uInt16 nWhich = pField->GetTyp()->Which();
        switch( nWhich )
        {
            case RES_DBSETNUMBERFLD:
            case RES_GETEXPFLD:
                if( GETFLD_ALL == eGetMode )
                    sFormula = sTrue;
                break;

            case RES_DBFLD:
                if( GETFLD_EXPAND & eGetMode )
                    sFormula = sTrue;
                break;

            case RES_SETEXPFLD:
                if ( !(eGetMode == GETFLD_EXPAND) ||
                     (nsSwGetSetExpType::GSE_STRING & pField->GetSubType()) )
                {
                    sFormula = sTrue;
                }
                break;

            case RES_HIDDENPARAFLD:
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

            case RES_HIDDENTXTFLD:
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
            case RES_DBNUMSETFLD:
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
            case RES_DBNEXTSETFLD:
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
        }

        if (!sFormula.isEmpty())
        {
            GetBodyNode( *pTextField, nWhich );
        }
    }
    nFieldLstGetMode = static_cast<sal_uInt8>( eGetMode );
    nNodes = rDoc.GetNodes().Count();
}

void SwDocUpdateField::GetBodyNode( const SwTextField& rTField, sal_uInt16 nFieldWhich )
{
    const SwTextNode& rTextNd = rTField.GetTextNode();
    const SwDoc& rDoc = *rTextNd.GetDoc();

    // always the first! (in tab headline, header-/footer)
    Point aPt;
    const SwContentFrame* pFrame = rTextNd.getLayoutFrame( rDoc.getIDocumentLayoutAccess().GetCurrentLayout(), &aPt, nullptr, false );

    _SetGetExpField* pNew = nullptr;
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
            pNew = new _SetGetExpField( aIdx, &rTField );
    }
    else
    {
        // create index to determine the TextNode
        SwPosition aPos( rDoc.GetNodes().GetEndOfPostIts() );
        bool const bResult = GetBodyTextNode( rDoc, aPos, *pFrame );
        OSL_ENSURE(bResult, "where is the Field");
        (void) bResult; // unused in non-debug
        pNew = new _SetGetExpField( aPos.nNode, &rTField, &aPos.nContent );
    }

    // always set the BodyTextFlag in GetExp or DB fields
    if( RES_GETEXPFLD == nFieldWhich )
    {
        SwGetExpField* pGetField = const_cast<SwGetExpField*>(static_cast<const SwGetExpField*>(rTField.GetFormatField().GetField()));
        pGetField->ChgBodyTextFlag( bIsInBody );
    }
#if HAVE_FEATURE_DBCONNECTIVITY
    else if( RES_DBFLD == nFieldWhich )
    {
        SwDBField* pDBField = const_cast<SwDBField*>(static_cast<const SwDBField*>(rTField.GetFormatField().GetField()));
        pDBField->ChgBodyTextFlag( bIsInBody );
    }
#endif
    if( pNew != nullptr )
        if( !pFieldSortLst->insert( pNew ).second )
            delete pNew;
}

void SwDocUpdateField::GetBodyNode( const SwSectionNode& rSectNd )
{
    const SwDoc& rDoc = *rSectNd.GetDoc();
    _SetGetExpField* pNew = nullptr;

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
            const SwContentFrame* pFrame = pCNd->getLayoutFrame( rDoc.getIDocumentLayoutAccess().GetCurrentLayout(), &aPt, nullptr, false );
            if( !pFrame )
                break;

            bool const bResult = GetBodyTextNode( rDoc, aPos, *pFrame );
            OSL_ENSURE(bResult, "where is the Field");
            (void) bResult; // unused in non-debug
            pNew = new _SetGetExpField( rSectNd, &aPos );

        } while( false );
    }

    if( !pNew )
        pNew = new _SetGetExpField( rSectNd );

    if( !pFieldSortLst->insert( pNew ).second )
        delete pNew;
}

void SwDocUpdateField::InsertFieldType( const SwFieldType& rType )
{
    OUString sFieldName;
    switch( rType.Which() )
    {
    case RES_USERFLD :
        sFieldName = static_cast<const SwUserFieldType&>(rType).GetName();
        break;
    case RES_SETEXPFLD:
        sFieldName = static_cast<const SwSetExpFieldType&>(rType).GetName();
        break;
    default:
        OSL_ENSURE( false, "kein gueltiger FeldTyp" );
    }

    if( !sFieldName.isEmpty() )
    {
        SetFieldsDirty( true );
        // look up and remove from the hash table
        sFieldName = GetAppCharClass().lowercase( sFieldName );
        sal_uInt16 n;

        SwHash* pFnd = Find( sFieldName, GetFieldTypeTable(), TBLSZ, &n );

        if( !pFnd )
        {
            SwCalcFieldType* pNew = new SwCalcFieldType( sFieldName, &rType );
            pNew->pNext = aFieldTypeTable[ n ];
            aFieldTypeTable[ n ] = pNew;
        }
    }
}

void SwDocUpdateField::RemoveFieldType( const SwFieldType& rType )
{
    OUString sFieldName;
    switch( rType.Which() )
    {
    case RES_USERFLD :
        sFieldName = static_cast<const SwUserFieldType&>(rType).GetName();
        break;
    case RES_SETEXPFLD:
        sFieldName = static_cast<const SwSetExpFieldType&>(rType).GetName();
        break;
    }

    if( !sFieldName.isEmpty() )
    {
        SetFieldsDirty( true );
        // look up and remove from the hash table
        sFieldName = GetAppCharClass().lowercase( sFieldName );
        sal_uInt16 n;

        SwHash* pFnd = Find( sFieldName, GetFieldTypeTable(), TBLSZ, &n );
        if( pFnd )
        {
            if( aFieldTypeTable[ n ] == pFnd )
                aFieldTypeTable[ n ] = static_cast<SwCalcFieldType*>(pFnd->pNext);
            else
            {
                SwHash* pPrev = aFieldTypeTable[ n ];
                while( pPrev->pNext != pFnd )
                    pPrev = pPrev->pNext;
                pPrev->pNext = pFnd->pNext;
            }
            pFnd->pNext = nullptr;
            delete pFnd;
        }
    }
}

SwDocUpdateField::SwDocUpdateField(SwDoc* pDoc)
    : pFieldSortLst(nullptr)
    , nNodes(0)
    , nFieldLstGetMode(0)
    , pDocument(pDoc)
    , bInUpdateFields(false)
    , bFieldsDirty(false)

{
    memset( aFieldTypeTable, 0, sizeof( aFieldTypeTable ) );
}

SwDocUpdateField::~SwDocUpdateField()
{
    delete pFieldSortLst;

    for( int n = 0; n < TBLSZ; ++n )
        delete aFieldTypeTable[n];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

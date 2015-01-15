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
#include "switerator.hxx"

using namespace ::com::sun::star::uno;

// the StartIndex can be supplied optionally (e.g. if it was queried before - is a virtual
// method otherwise!)
_SetGetExpFld::_SetGetExpFld(
    const SwNodeIndex& rNdIdx,
    const SwTxtFld* pFld,
    const SwIndex* pIdx )
{
    eSetGetExpFldType = TEXTFIELD;
    CNTNT.pTxtFld = pFld;
    nNode = rNdIdx.GetIndex();
    if( pIdx )
        nCntnt = pIdx->GetIndex();
    else if( pFld )
        nCntnt = pFld->GetStart();
    else
        nCntnt = 0;
}

_SetGetExpFld::_SetGetExpFld( const SwNodeIndex& rNdIdx,
                            const SwTxtINetFmt& rINet, const SwIndex* pIdx )
{
    eSetGetExpFldType = TEXTINET;
    CNTNT.pTxtINet = &rINet;
    nNode = rNdIdx.GetIndex();
    if( pIdx )
        nCntnt = pIdx->GetIndex();
    else
        nCntnt = rINet.GetStart();
}

// Extension for Sections:
// these always have content position 0xffffffff!
// There is never a field on this, only up to COMPLETE_STRING possible
_SetGetExpFld::_SetGetExpFld( const SwSectionNode& rSectNd,
                                const SwPosition* pPos )
{
    eSetGetExpFldType = SECTIONNODE;
    CNTNT.pSection = &rSectNd.GetSection();

    if( pPos )
    {
        nNode = pPos->nNode.GetIndex();
        nCntnt = pPos->nContent.GetIndex();
    }
    else
    {
        nNode = rSectNd.GetIndex();
        nCntnt = 0;
    }
}

_SetGetExpFld::_SetGetExpFld( const SwTableBox& rTBox, const SwPosition* pPos )
{
    eSetGetExpFldType = TABLEBOX;
    CNTNT.pTBox = &rTBox;

    if( pPos )
    {
        nNode = pPos->nNode.GetIndex();
        nCntnt = pPos->nContent.GetIndex();
    }
    else
    {
        nNode = 0;
        nCntnt = 0;
        if( rTBox.GetSttNd() )
        {
            SwNodeIndex aIdx( *rTBox.GetSttNd() );
            const SwCntntNode* pNd = aIdx.GetNode().GetNodes().GoNext( &aIdx );
            if( pNd )
                nNode = pNd->GetIndex();
        }
    }
}

_SetGetExpFld::_SetGetExpFld( const SwNodeIndex& rNdIdx,
                                const SwTxtTOXMark& rTOX,
                                const SwIndex* pIdx )
{
    eSetGetExpFldType = TEXTTOXMARK;
    CNTNT.pTxtTOX = &rTOX;
    nNode = rNdIdx.GetIndex();
    if( pIdx )
        nCntnt = pIdx->GetIndex();
    else
        nCntnt = rTOX.GetStart();
}

_SetGetExpFld::_SetGetExpFld( const SwPosition& rPos )
{
    eSetGetExpFldType = CRSRPOS;
    CNTNT.pPos = &rPos;
    nNode = rPos.nNode.GetIndex();
    nCntnt = rPos.nContent.GetIndex();
}

_SetGetExpFld::_SetGetExpFld( const SwFlyFrmFmt& rFlyFmt,
                                const SwPosition* pPos  )
{
    eSetGetExpFldType = FLYFRAME;
    CNTNT.pFlyFmt = &rFlyFmt;
    if( pPos )
    {
        nNode = pPos->nNode.GetIndex();
        nCntnt = pPos->nContent.GetIndex();
    }
    else
    {
        const SwFmtCntnt& rCntnt = rFlyFmt.GetCntnt();
        nNode = rCntnt.GetCntntIdx()->GetIndex() + 1;
        nCntnt = 0;
    }
}

void _SetGetExpFld::GetPosOfContent( SwPosition& rPos ) const
{
    const SwNode* pNd = GetNodeFromCntnt();
    if( pNd )
        pNd = pNd->GetCntntNode();

    if( pNd )
    {
        rPos.nNode = *pNd;
        rPos.nContent.Assign( const_cast<SwCntntNode*>(static_cast<const SwCntntNode*>(pNd)), GetCntPosFromCntnt() );
    }
    else
    {
        rPos.nNode = nNode;
        rPos.nContent.Assign( rPos.nNode.GetNode().GetCntntNode(), nCntnt );
    }
}

void _SetGetExpFld::SetBodyPos( const SwCntntFrm& rFrm )
{
    if( !rFrm.IsInDocBody() )
    {
        SwNodeIndex aIdx( *rFrm.GetNode() );
        SwDoc& rDoc = *aIdx.GetNodes().GetDoc();
        SwPosition aPos( aIdx );
        bool const bResult = ::GetBodyTxtNode( rDoc, aPos, rFrm );
        OSL_ENSURE(bResult, "Where is the field?");
        (void) bResult; // unused in non-debug
        nNode = aPos.nNode.GetIndex();
        nCntnt = aPos.nContent.GetIndex();
    }
}

bool _SetGetExpFld::operator==( const _SetGetExpFld& rFld ) const
{
    return nNode == rFld.nNode
           && nCntnt == rFld.nCntnt
           && ( !CNTNT.pTxtFld
                || !rFld.CNTNT.pTxtFld
                || CNTNT.pTxtFld == rFld.CNTNT.pTxtFld );
}

bool _SetGetExpFld::operator<( const _SetGetExpFld& rFld ) const
{
    if( nNode < rFld.nNode || ( nNode == rFld.nNode && nCntnt < rFld.nCntnt ))
        return true;
    else if( nNode != rFld.nNode || nCntnt != rFld.nCntnt )
        return false;

    const SwNode *pFirst = GetNodeFromCntnt(),
                 *pNext = rFld.GetNodeFromCntnt();

    // Position is the same: continue only if both field pointers are set!
    if( !pFirst || !pNext )
        return false;

    // same Section?
    if( pFirst->StartOfSectionNode() != pNext->StartOfSectionNode() )
    {
        // is one in the table?
        const SwNode *pFirstStt, *pNextStt;
        const SwTableNode* pTblNd = pFirst->FindTableNode();
        if( pTblNd )
            pFirstStt = pTblNd->StartOfSectionNode();
        else
            pFirstStt = pFirst->StartOfSectionNode();

        if( 0 != ( pTblNd = pNext->FindTableNode() ) )
            pNextStt = pTblNd->StartOfSectionNode();
        else
            pNextStt = pNext->StartOfSectionNode();

        if( pFirstStt != pNextStt )
        {
            if( pFirst->IsTxtNode() && pNext->IsTxtNode() &&
                ( pFirst->FindFlyStartNode() || pNext->FindFlyStartNode() ))
            {
                return ::IsFrameBehind( *pNext->GetTxtNode(), nCntnt, *pFirst->GetTxtNode(), nCntnt );
            }
            return pFirstStt->GetIndex() < pNextStt->GetIndex();
        }
    }

    // same Section: is the field in the same Node?
    if( pFirst != pNext )
        return pFirst->GetIndex() < pNext->GetIndex();

    // same Node in the Section, check Position in the Node
    return GetCntPosFromCntnt() < rFld.GetCntPosFromCntnt();
}

const SwNode* _SetGetExpFld::GetNodeFromCntnt() const
{
    const SwNode* pRet = 0;
    if( CNTNT.pTxtFld )
        switch( eSetGetExpFldType )
        {
        case TEXTFIELD:
            pRet = &CNTNT.pTxtFld->GetTxtNode();
            break;

        case TEXTINET:
            pRet = &CNTNT.pTxtINet->GetTxtNode();
            break;

        case SECTIONNODE:
            pRet = CNTNT.pSection->GetFmt()->GetSectionNode();
            break;

        case CRSRPOS:
            pRet = &CNTNT.pPos->nNode.GetNode();
            break;

        case TEXTTOXMARK:
            pRet = &CNTNT.pTxtTOX->GetTxtNode();
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
                SwNodeIndex aIdx( *CNTNT.pFlyFmt->GetCntnt().GetCntntIdx() );
                pRet = aIdx.GetNode().GetNodes().GoNext( &aIdx );
            }
            break;
        }
    return pRet;
}

sal_Int32 _SetGetExpFld::GetCntPosFromCntnt() const
{
    sal_Int32 nRet = 0;
    if( CNTNT.pTxtFld )
        switch( eSetGetExpFldType )
        {
        case TEXTFIELD:
        case TEXTINET:
        case TEXTTOXMARK:
            nRet = CNTNT.pTxtFld->GetStart();
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

/// Look up the Name, if it is present, return it's String, otherwise return an empty String
OUString LookString( SwHash** ppTbl, sal_uInt16 nSize, const OUString& rName,
                     sal_uInt16* pPos )
{
    SwHash* pFnd = Find( comphelper::string::strip(rName, ' '), ppTbl, nSize, pPos );
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
        const sal_uInt16 nSize = getIDocumentFieldsAccess().GetFldTypes()->size();
        for(sal_uInt16 i = 0; i < nSize && maDBData.sDataSource.isEmpty(); ++i)
        {
            SwFieldType& rFldType = *((*getIDocumentFieldsAccess().GetFldTypes())[i]);
            sal_uInt16 nWhich = rFldType.Which();
            if(IsUsed(rFldType))
            {
                switch(nWhich)
                {
                    case RES_DBFLD:
                    case RES_DBNEXTSETFLD:
                    case RES_DBNUMSETFLD:
                    case RES_DBSETNUMBERFLD:
                    {
                        SwIterator<SwFmtFld,SwFieldType> aIter( rFldType );
                        for( SwFmtFld* pFld = aIter.First(); pFld; pFld = aIter.Next() )
                        {
                            if(pFld->IsFldInDoc())
                            {
                                if(RES_DBFLD == nWhich)
                                    maDBData = (static_cast < SwDBFieldType * > (pFld->GetField()->GetTyp()))->GetDBData();
                                else
                                    maDBData = (static_cast < SwDBNameInfField* > (pFld->GetField()))->GetRealDBData();
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
        maDBData = GetDBManager()->GetAddressDBName();
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
    OUString sRet = rData.sDataSource;
    sRet += OUString(DB_DELIM);
    sRet += rData.sCommand;
    sRet += OUString(DB_DELIM);
    sRet += OUString::number(rData.nCommandType);
    return sRet;
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

    SwSectionFmts& rArr = GetSections();
    for (sal_uInt16 n = rArr.size(); n; )
    {
        SwSection* pSect = rArr[ --n ]->GetSection();

        if( pSect )
        {
            OUString aCond( pSect->GetCondition() );
            AddUsedDBToList( rDBNameList, FindUsedDBs( *pAllDBNames,
                                                aCond, aUsedDBNames ) );
            aUsedDBNames.clear();
        }
    }

    sal_uInt32 nMaxItems = GetAttrPool().GetItemCount2( RES_TXTATR_FIELD );
    for (sal_uInt32 n = 0; n < nMaxItems; ++n)
    {
        const SfxPoolItem* pItem;
        if( 0 == (pItem = GetAttrPool().GetItem2( RES_TXTATR_FIELD, n ) ))
            continue;

        const SwFmtFld* pFmtFld = static_cast<const SwFmtFld*>(pItem);
        const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
        if( !pTxtFld || !pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
            continue;

        const SwField* pFld = pFmtFld->GetField();
        switch( pFld->GetTyp()->Which() )
        {
            case RES_DBFLD:
                AddUsedDBToList( rDBNameList,
                                lcl_DBDataToString(static_cast<const SwDBField*>(pFld)->GetDBData() ));
                break;

            case RES_DBSETNUMBERFLD:
            case RES_DBNAMEFLD:
                AddUsedDBToList( rDBNameList,
                                lcl_DBDataToString(static_cast<const SwDBNameInfField*>(pFld)->GetRealDBData() ));
                break;

            case RES_DBNUMSETFLD:
            case RES_DBNEXTSETFLD:
                AddUsedDBToList( rDBNameList,
                                lcl_DBDataToString(static_cast<const SwDBNameInfField*>(pFld)->GetRealDBData() ));
                // no break  // JP: is that right like that?

            case RES_HIDDENTXTFLD:
            case RES_HIDDENPARAFLD:
                AddUsedDBToList(rDBNameList, FindUsedDBs( *pAllDBNames,
                                            pFld->GetPar1(), aUsedDBNames ));
                aUsedDBNames.clear();
                break;

            case RES_SETEXPFLD:
            case RES_GETEXPFLD:
            case RES_TABLEFLD:
                AddUsedDBToList(rDBNameList, FindUsedDBs( *pAllDBNames,
                                        pFld->GetFormula(), aUsedDBNames ));
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

    const SwDSParamArr& rArr = pMgr->GetDSParamArray();
    for(sal_uInt16 i = 0; i < rArr.size(); i++)
    {
        const SwDSParam* pParam = &rArr[i];
        rAllDBNames.push_back(pParam->sDataSource + OUString(DB_DELIM) + pParam->sCommand);
    }
#endif
}

std::vector<OUString>& SwDoc::FindUsedDBs( const std::vector<OUString>& rAllDBNames,
                                   const OUString& rFormula,
                                   std::vector<OUString>& rUsedDBNames )
{
    const CharClass& rCC = GetAppCharClass();
    OUString  sFormula(rFormula);
#ifndef UNX
    sFormula = rCC.uppercase( sFormula );
#endif

    sal_Int32 nPos;
    for (sal_uInt16 i = 0; i < rAllDBNames.size(); ++i )
    {
        OUString pStr(rAllDBNames[i]);

        if( -1 != (nPos = sFormula.indexOf( pStr )) &&
            sFormula[ nPos + pStr.getLength() ] == '.' &&
            (!nPos || !rCC.isLetterNumeric( sFormula, nPos - 1 )))
        {
            // Look up table name
            sal_Int32 nEndPos;
            nPos += pStr.getLength() + 1;
            if( -1 != (nEndPos = sFormula.indexOf('.', nPos)) )
            {
                pStr += OUString( DB_DELIM );
                pStr += sFormula.copy( nPos, nEndPos - nPos );
                rUsedDBNames.push_back(pStr);
            }
        }
    }
    return rUsedDBNames;
}

void SwDoc::AddUsedDBToList( std::vector<OUString>& rDBNameList,
                             const std::vector<OUString>& rUsedDBNames )
{
    for (sal_uInt16 i = 0; i < rUsedDBNames.size(); ++i)
        AddUsedDBToList( rDBNameList, rUsedDBNames[i] );
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
    for( sal_uInt16 i = 0; i < rDBNameList.size(); ++i )
        if( rDBName == rDBNameList[i].getToken(0, ';') )
            return;
#else
    const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
    for( sal_uInt16 i = 0; i < rDBNameList.size(); ++i )
        if( rSCmp.isEqual( rDBName, rDBNameList[i].getToken(0, ';') ) )
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

    SwSectionFmts& rArr = GetSections();
    for (sal_uInt16 n = rArr.size(); n; )
    {
        SwSection* pSect = rArr[ --n ]->GetSection();

        if( pSect )
        {
            pSect->SetCondition(ReplaceUsedDBs(rOldNames, rNewName, pSect->GetCondition()));
        }
    }

    const SfxPoolItem* pItem;
    sal_uInt32 nMaxItems = GetAttrPool().GetItemCount2( RES_TXTATR_FIELD );

    for (sal_uInt32 n = 0; n < nMaxItems; ++n )
    {
        if( 0 == (pItem = GetAttrPool().GetItem2( RES_TXTATR_FIELD, n ) ))
            continue;

        SwFmtFld* pFmtFld = const_cast<SwFmtFld*>(static_cast<const SwFmtFld*>(pItem));
        SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
        if( !pTxtFld || !pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
            continue;

        SwField* pFld = pFmtFld->GetField();
        bool bExpand = false;

        switch( pFld->GetTyp()->Which() )
        {
            case RES_DBFLD:
#if HAVE_FEATURE_DBCONNECTIVITY
                if( IsNameInArray( rOldNames, lcl_DBDataToString(static_cast<SwDBField*>(pFld)->GetDBData())))
                {
                    SwDBFieldType* pOldTyp = static_cast<SwDBFieldType*>(pFld->GetTyp());

                    SwDBFieldType* pTyp = static_cast<SwDBFieldType*>(getIDocumentFieldsAccess().InsertFldType(
                            SwDBFieldType(this, pOldTyp->GetColumnName(), aNewDBData)));

                    pFmtFld->RegisterToFieldType( *pTyp );
                    pFld->ChgTyp(pTyp);

                    static_cast<SwDBField*>(pFld)->ClearInitialized();
                    static_cast<SwDBField*>(pFld)->InitContent();

                    bExpand = true;
                }
#endif
                break;

            case RES_DBSETNUMBERFLD:
            case RES_DBNAMEFLD:
                if( IsNameInArray( rOldNames,
                                lcl_DBDataToString(static_cast<SwDBNameInfField*>(pFld)->GetRealDBData())))
                {
                    static_cast<SwDBNameInfField*>(pFld)->SetDBData(aNewDBData);
                    bExpand = true;
                }
                break;

            case RES_DBNUMSETFLD:
            case RES_DBNEXTSETFLD:
                if( IsNameInArray( rOldNames,
                                lcl_DBDataToString(static_cast<SwDBNameInfField*>(pFld)->GetRealDBData())))
                {
                    static_cast<SwDBNameInfField*>(pFld)->SetDBData(aNewDBData);
                }
                // no break;
            case RES_HIDDENTXTFLD:
            case RES_HIDDENPARAFLD:
                pFld->SetPar1( ReplaceUsedDBs(rOldNames, rNewName, pFld->GetPar1()) );
                bExpand = true;
                break;

            case RES_SETEXPFLD:
            case RES_GETEXPFLD:
            case RES_TABLEFLD:
                pFld->SetPar2( ReplaceUsedDBs(rOldNames, rNewName, pFld->GetFormula()) );
                bExpand = true;
                break;
        }

        if (bExpand)
            pTxtFld->ExpandTxtFld( true );
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
    for( sal_uInt16 i = 0; i < rArr.size(); ++i )
        if( rName == rArr[ i ] )
            return true;
#else
    const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
    for( sal_uInt16 i = 0; i < rArr.size(); ++i )
        if( rSCmp.isEqual( rName, rArr[ i] ))
            return true;
#endif
    return false;
}

void SwDoc::ChangeAuthorityData( const SwAuthEntry* pNewData )
{
    const sal_uInt16 nSize = getIDocumentFieldsAccess().GetFldTypes()->size();

    for( sal_uInt16 i = INIT_FLDTYPES; i < nSize; ++i )
    {
        SwFieldType* pFldType = (*getIDocumentFieldsAccess().GetFldTypes())[i];
        if( RES_AUTHORITY  == pFldType->Which() )
        {
            SwAuthorityFieldType* pAuthType = static_cast<SwAuthorityFieldType*>(pFldType);
            pAuthType->ChangeEntryContent(pNewData);
            break;
        }
    }

}

void SwDocUpdtFld::InsDelFldInFldLst( bool bIns, const SwTxtFld& rFld )
{
    const sal_uInt16 nWhich = rFld.GetFmtFld().GetField()->GetTyp()->Which();
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
    if( !pFldSortLst )
    {
        if( !bIns )             // if list is present and deleted
            return;             // don't do a thing
        pFldSortLst = new _SetGetExpFlds;
    }

    if( bIns )      // insert anew:
        GetBodyNode( rFld, nWhich );
    else
    {
        // look up via the pTxtFld pointer. It is a sorted list, but it's sorted by node
        // position. Until this is found, the search for the pointer is already done.
        for( sal_uInt16 n = 0; n < pFldSortLst->size(); ++n )
            if( &rFld == (*pFldSortLst)[ n ]->GetPointer() )
            {
                delete (*pFldSortLst)[n];
                pFldSortLst->erase(n);
                n--; // one field can occur multiple times
            }
    }
}

void SwDocUpdtFld::MakeFldList( SwDoc& rDoc, bool bAll, int eGetMode )
{
    if( !pFldSortLst || bAll || !( eGetMode & nFldLstGetMode ) ||
        rDoc.GetNodes().Count() != nNodes )
        _MakeFldList( rDoc, eGetMode );
}

void SwDocUpdtFld::_MakeFldList( SwDoc& rDoc, int eGetMode )
{
    // new version: walk all fields of the attribute pool
    delete pFldSortLst;
    pFldSortLst = new _SetGetExpFlds;

    // consider and unhide sections
    //     with hide condition, only in mode GETFLD_ALL (<eGetMode == GETFLD_ALL>)
    //     notes by OD:
    //         eGetMode == GETFLD_CALC in call from methods SwDoc::FldsToCalc
    //         eGetMode == GETFLD_EXPAND in call from method SwDoc::FldsToExpand
    //         eGetMode == GETFLD_ALL in call from method SwDoc::UpdateExpFlds
    //         I figured out that hidden section only have to be shown,
    //         if fields have updated (call by SwDoc::UpdateExpFlds) and thus
    //         the hide conditions of section have to be updated.
    //         For correct updating the hide condition of a section, its position
    //         have to be known in order to insert the hide condition as a new
    //         expression field into the sorted field list (<pFldSortLst>).
    if ( eGetMode == GETFLD_ALL )
    // Collect the sections first. Supply sections that are hidden by condition
    // with frames so that the contained fields are sorted properly.
    {
        // In order for the frames to be created the right way, they have to be expanded
        // from top to bottom
        std::vector<sal_uLong> aTmpArr;
        SwSectionFmts& rArr = rDoc.GetSections();
        SwSectionNode* pSectNd = 0;
        sal_uInt16 nArrStt = 0;
        sal_uLong nSttCntnt = rDoc.GetNodes().GetEndOfExtras().GetIndex();

        for (sal_uInt16 n = rArr.size(); n; )
        {
            SwSection* pSect = rArr[ --n ]->GetSection();
            if( pSect && pSect->IsHidden() && !pSect->GetCondition().isEmpty() &&
                0 != ( pSectNd = pSect->GetFmt()->GetSectionNode() ))
            {
                sal_uLong nIdx = pSectNd->GetIndex();
                aTmpArr.push_back( nIdx );
                if( nIdx < nSttCntnt )
                    ++nArrStt;
            }
        }
        std::sort(aTmpArr.begin(), aTmpArr.end());

        // Display all first so that we have frames. The BodyAnchor is defined by that.
        // First the ContentArea, then the special areas!
        for (sal_uInt16 n = nArrStt; n < aTmpArr.size(); ++n)
        {
            pSectNd = rDoc.GetNodes()[ aTmpArr[ n ] ]->GetSectionNode();
            OSL_ENSURE( pSectNd, "Where is my SectionNode" );
            pSectNd->GetSection().SetCondHidden( false );
        }
        for (sal_uInt16 n = 0; n < nArrStt; ++n)
        {
            pSectNd = rDoc.GetNodes()[ aTmpArr[ n ] ]->GetSectionNode();
            OSL_ENSURE( pSectNd, "Where is my SectionNode" );
            pSectNd->GetSection().SetCondHidden( false );
        }

        // add all to the list so that they are sorted
        for (sal_uInt16 n = 0; n < aTmpArr.size(); ++n)
        {
            GetBodyNode( *rDoc.GetNodes()[ aTmpArr[ n ] ]->GetSectionNode() );
        }
    }

    const OUString sTrue("TRUE");
    const OUString sFalse("FALSE");

#if HAVE_FEATURE_DBCONNECTIVITY
    bool bIsDBManager = 0 != rDoc.GetDBManager();
#endif
    sal_uInt16 nWhich, n;
    const SfxPoolItem* pItem;
    sal_uInt32 nMaxItems = rDoc.GetAttrPool().GetItemCount2( RES_TXTATR_FIELD );
    for( n = 0; n < nMaxItems; ++n )
    {
        if( 0 == (pItem = rDoc.GetAttrPool().GetItem2( RES_TXTATR_FIELD, n )) )
            continue;

        const SwFmtFld* pFmtFld = static_cast<const SwFmtFld*>(pItem);
        const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
        if( !pTxtFld || !pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
            continue;

        OUString sFormula;
        const SwField* pFld = pFmtFld->GetField();
        switch( nWhich = pFld->GetTyp()->Which() )
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
                     (nsSwGetSetExpType::GSE_STRING & pFld->GetSubType()) )
                {
                    sFormula = sTrue;
                }
                break;

            case RES_HIDDENPARAFLD:
                if( GETFLD_ALL == eGetMode )
                {
                    sFormula = pFld->GetPar1();
                    if (sFormula.isEmpty() || sFormula==sFalse)
                        const_cast<SwHiddenParaField*>(static_cast<const SwHiddenParaField*>(pFld))->SetHidden( false );
                    else if (sFormula==sTrue)
                        const_cast<SwHiddenParaField*>(static_cast<const SwHiddenParaField*>(pFld))->SetHidden( true );
                    else
                        break;

                    sFormula.clear();
                    // trigger formatting
                    const_cast<SwFmtFld*>(pFmtFld)->ModifyNotification( 0, 0 );
                }
                break;

            case RES_HIDDENTXTFLD:
                if( GETFLD_ALL == eGetMode )
                {
                    sFormula = pFld->GetPar1();
                    if (sFormula.isEmpty() || sFormula==sFalse)
                        const_cast<SwHiddenTxtField*>(static_cast<const SwHiddenTxtField*>(pFld))->SetValue( true );
                    else if (sFormula==sTrue)
                        const_cast<SwHiddenTxtField*>(static_cast<const SwHiddenTxtField*>(pFld))->SetValue( false );
                    else
                        break;

                    sFormula.clear();

                    // evaluate field
                    const_cast<SwHiddenTxtField*>(static_cast<const SwHiddenTxtField*>(pFld))->Evaluate(&rDoc);
                    // trigger formatting
                    const_cast<SwFmtFld*>(pFmtFld)->ModifyNotification( 0, 0 );
                }
                break;

#if HAVE_FEATURE_DBCONNECTIVITY
            case RES_DBNUMSETFLD:
            {
                SwDBData aDBData(const_cast<SwDBNumSetField*>(static_cast<const SwDBNumSetField*>(pFld))->GetDBData(&rDoc));

                if (
                     (bIsDBManager && rDoc.GetDBManager()->OpenDataSource(aDBData.sDataSource, aDBData.sCommand)) &&
                     (GETFLD_ALL == eGetMode || (GETFLD_CALC & eGetMode && static_cast<const SwDBNumSetField*>(pFld)->IsCondValid()))
                   )
                {
                    sFormula = pFld->GetPar1();
                }
            }
            break;
            case RES_DBNEXTSETFLD:
            {
                SwDBData aDBData(const_cast<SwDBNextSetField*>(static_cast<const SwDBNextSetField*>(pFld))->GetDBData(&rDoc));

                if (
                     (bIsDBManager && rDoc.GetDBManager()->OpenDataSource(aDBData.sDataSource, aDBData.sCommand)) &&
                     (GETFLD_ALL == eGetMode || (GETFLD_CALC & eGetMode && static_cast<const SwDBNextSetField*>(pFld)->IsCondValid()))
                   )
                {
                    sFormula = pFld->GetPar1();
                }
            }
            break;
#endif
        }

        if (!sFormula.isEmpty())
        {
            GetBodyNode( *pTxtFld, nWhich );
        }
    }
    nFldLstGetMode = static_cast<sal_uInt8>( eGetMode );
    nNodes = rDoc.GetNodes().Count();
}

void SwDocUpdtFld::GetBodyNode( const SwTxtFld& rTFld, sal_uInt16 nFldWhich )
{
    const SwTxtNode& rTxtNd = rTFld.GetTxtNode();
    const SwDoc& rDoc = *rTxtNd.GetDoc();

    // always the first! (in tab headline, header-/footer)
    Point aPt;
    const SwCntntFrm* pFrm = rTxtNd.getLayoutFrm( rDoc.getIDocumentLayoutAccess().GetCurrentLayout(), &aPt, 0, false );

    _SetGetExpFld* pNew = NULL;
    bool bIsInBody = false;

    if( !pFrm || pFrm->IsInDocBody() )
    {
        // create index to determine the TextNode
        SwNodeIndex aIdx( rTxtNd );
        bIsInBody = rDoc.GetNodes().GetEndOfExtras().GetIndex() < aIdx.GetIndex();

        // We don't want to update fields in redlines, or those
        // in frames whose anchor is in redline. However, we do want to update
        // fields in hidden sections. So: In order to be updated, a field 1)
        // must have a frame, or 2) it must be in the document body.
        if( (pFrm != NULL) || bIsInBody )
            pNew = new _SetGetExpFld( aIdx, &rTFld );
    }
    else
    {
        // create index to determine the TextNode
        SwPosition aPos( rDoc.GetNodes().GetEndOfPostIts() );
        bool const bResult = GetBodyTxtNode( rDoc, aPos, *pFrm );
        OSL_ENSURE(bResult, "where is the Field");
        (void) bResult; // unused in non-debug
        pNew = new _SetGetExpFld( aPos.nNode, &rTFld, &aPos.nContent );
    }

    // always set the BodyTxtFlag in GetExp or DB fields
    if( RES_GETEXPFLD == nFldWhich )
    {
        SwGetExpField* pGetFld = const_cast<SwGetExpField*>(static_cast<const SwGetExpField*>(rTFld.GetFmtFld().GetField()));
        pGetFld->ChgBodyTxtFlag( bIsInBody );
    }
#if HAVE_FEATURE_DBCONNECTIVITY
    else if( RES_DBFLD == nFldWhich )
    {
        SwDBField* pDBFld = const_cast<SwDBField*>(static_cast<const SwDBField*>(rTFld.GetFmtFld().GetField()));
        pDBFld->ChgBodyTxtFlag( bIsInBody );
    }
#endif
    if( pNew != NULL )
        if( !pFldSortLst->insert( pNew ).second )
            delete pNew;
}

void SwDocUpdtFld::GetBodyNode( const SwSectionNode& rSectNd )
{
    const SwDoc& rDoc = *rSectNd.GetDoc();
    _SetGetExpFld* pNew = 0;

    if( rSectNd.GetIndex() < rDoc.GetNodes().GetEndOfExtras().GetIndex() )
    {
        do {            // middle check loop

            // we need to get the anchor first
            // create index to determine the TextNode
            SwPosition aPos( rSectNd );
            SwCntntNode* pCNd = rDoc.GetNodes().GoNext( &aPos.nNode ); // to the next ContentNode

            if( !pCNd || !pCNd->IsTxtNode() )
                break;

            // always the first! (in tab headline, header-/footer)
            Point aPt;
            const SwCntntFrm* pFrm = pCNd->getLayoutFrm( rDoc.getIDocumentLayoutAccess().GetCurrentLayout(), &aPt, 0, false );
            if( !pFrm )
                break;

            bool const bResult = GetBodyTxtNode( rDoc, aPos, *pFrm );
            OSL_ENSURE(bResult, "where is the Field");
            (void) bResult; // unused in non-debug
            pNew = new _SetGetExpFld( rSectNd, &aPos );

        } while( false );
    }

    if( !pNew )
        pNew = new _SetGetExpFld( rSectNd );

    if( !pFldSortLst->insert( pNew ).second )
        delete pNew;
}

void SwDocUpdtFld::InsertFldType( const SwFieldType& rType )
{
    OUString sFldName;
    switch( rType.Which() )
    {
    case RES_USERFLD :
        sFldName = static_cast<const SwUserFieldType&>(rType).GetName();
        break;
    case RES_SETEXPFLD:
        sFldName = static_cast<const SwSetExpFieldType&>(rType).GetName();
        break;
    default:
        OSL_ENSURE( false, "kein gueltiger FeldTyp" );
    }

    if( !sFldName.isEmpty() )
    {
        SetFieldsDirty( true );
        // look up and remove from the hash table
        sFldName = GetAppCharClass().lowercase( sFldName );
        sal_uInt16 n;

        SwHash* pFnd = Find( sFldName, GetFldTypeTable(), TBLSZ, &n );

        if( !pFnd )
        {
            SwCalcFldType* pNew = new SwCalcFldType( sFldName, &rType );
            pNew->pNext = aFldTypeTable[ n ];
            aFldTypeTable[ n ] = pNew;
        }
    }
}

void SwDocUpdtFld::RemoveFldType( const SwFieldType& rType )
{
    OUString sFldName;
    switch( rType.Which() )
    {
    case RES_USERFLD :
        sFldName = static_cast<const SwUserFieldType&>(rType).GetName();
        break;
    case RES_SETEXPFLD:
        sFldName = static_cast<const SwSetExpFieldType&>(rType).GetName();
        break;
    }

    if( !sFldName.isEmpty() )
    {
        SetFieldsDirty( true );
        // look up and remove from the hash table
        sFldName = GetAppCharClass().lowercase( sFldName );
        sal_uInt16 n;

        SwHash* pFnd = Find( sFldName, GetFldTypeTable(), TBLSZ, &n );
        if( pFnd )
        {
            if( aFldTypeTable[ n ] == pFnd )
                aFldTypeTable[ n ] = static_cast<SwCalcFldType*>(pFnd->pNext);
            else
            {
                SwHash* pPrev = aFldTypeTable[ n ];
                while( pPrev->pNext != pFnd )
                    pPrev = pPrev->pNext;
                pPrev->pNext = pFnd->pNext;
            }
            pFnd->pNext = 0;
            delete pFnd;
        }
    }
}

SwDocUpdtFld::SwDocUpdtFld(SwDoc* pDoc)
    : pFldSortLst(0)
    , nNodes(0)
    , nFldLstGetMode(0)
    , pDocument(pDoc)
    , bInUpdateFlds(false)
    , bFldsDirty(false)

{
    memset( aFldTypeTable, 0, sizeof( aFldTypeTable ) );
}

SwDocUpdtFld::~SwDocUpdtFld()
{
    delete pFldSortLst;

    for( sal_uInt16 n = 0; n < TBLSZ; ++n )
        delete aFldTypeTable[n];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

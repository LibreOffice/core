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
 *   with &m_rDoc work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#include <DocumentFieldsManager.hxx>
#include <config_features.h>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <dbmgr.hxx>
#include <chpfld.hxx>
#include <dbfld.hxx>
#include <reffld.hxx>
#include <flddropdown.hxx>
#include <poolfmt.hrc>
#include <SwUndoField.hxx>
#include <flddat.hxx>
#include <cntfrm.hxx>
#include <section.hxx>
#include <docufld.hxx>
#include <calbck.hxx>
#include <cellatr.hxx>
#include <swtable.hxx>
#include <frmfmt.hxx>
#include <fmtfld.hxx>
#include <ndtxt.hxx>
#include <txtfld.hxx>
#include <docfld.hxx>
#include <hints.hxx>
#include <docary.hxx>
#include <fldbas.hxx>
#include <expfld.hxx>
#include <ddefld.hxx>
#include <authfld.hxx>
#include <usrfld.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <com/sun/star/uno/Any.hxx>

using namespace ::com::sun::star::uno;

namespace
{
    #if HAVE_FEATURE_DBCONNECTIVITY

    static OUString lcl_GetDBVarName( SwDoc& rDoc, SwDBNameInfField& rDBFld )
    {
        SwDBData aDBData( rDBFld.GetDBData( &rDoc ));
        OUString sDBNumNm;
        SwDBData aDocData = rDoc.GetDBData();

        if( aDBData != aDocData )
        {
            sDBNumNm = aDBData.sDataSource;
            sDBNumNm += OUString(DB_DELIM);
            sDBNumNm += aDBData.sCommand;
            sDBNumNm += OUString(DB_DELIM);
        }
        sDBNumNm += SwFieldType::GetTypeStr(TYP_DBSETNUMBERFLD);

        return sDBNumNm;
    }

    #endif

    static void lcl_CalcFld( SwDoc& rDoc, SwCalc& rCalc, const _SetGetExpFld& rSGEFld,
                            SwDBManager* pMgr )
    {
        const SwTxtFld* pTxtFld = rSGEFld.GetTxtFld();
        if( !pTxtFld )
            return ;

        const SwField* pFld = pTxtFld->GetFmtFld().GetField();
        const sal_uInt16 nFldWhich = pFld->GetTyp()->Which();

        if( RES_SETEXPFLD == nFldWhich )
        {
            SwSbxValue aValue;
            if( nsSwGetSetExpType::GSE_EXPR & pFld->GetSubType() )
                aValue.PutDouble( static_cast<const SwSetExpField*>(pFld)->GetValue() );
            else
                // Extension to calculate with Strings
                aValue.PutString( static_cast<const SwSetExpField*>(pFld)->GetExpStr() );

            // set the new value in Calculator
            rCalc.VarChange( pFld->GetTyp()->GetName(), aValue );
        }
        else if( pMgr )
        {
    #if !HAVE_FEATURE_DBCONNECTIVITY
            (void) rDoc;
    #else
            switch( nFldWhich )
            {
            case RES_DBNUMSETFLD:
                {
                    SwDBNumSetField* pDBFld = const_cast<SwDBNumSetField*>(static_cast<const SwDBNumSetField*>(pFld));

                    SwDBData aDBData(pDBFld->GetDBData(&rDoc));

                    if( pDBFld->IsCondValid() &&
                        pMgr->OpenDataSource( aDBData.sDataSource, aDBData.sCommand ))
                        rCalc.VarChange( lcl_GetDBVarName( rDoc, *pDBFld),
                                        pDBFld->GetFormat() );
                }
                break;
            case RES_DBNEXTSETFLD:
                {
                    SwDBNextSetField* pDBFld = const_cast<SwDBNextSetField*>(static_cast<const SwDBNextSetField*>(pFld));
                    SwDBData aDBData(pDBFld->GetDBData(&rDoc));
                    if( !pDBFld->IsCondValid() ||
                        !pMgr->OpenDataSource( aDBData.sDataSource, aDBData.sCommand ))
                        break;

                    OUString sDBNumNm(lcl_GetDBVarName( rDoc, *pDBFld));
                    SwCalcExp* pExp = rCalc.VarLook( sDBNumNm );
                    if( pExp )
                        rCalc.VarChange( sDBNumNm, pExp->nValue.GetLong() + 1 );
                }
                break;

            }
    #endif
        }
    }
}

namespace sw
{

DocumentFieldsManager::DocumentFieldsManager( SwDoc& i_rSwdoc ) : m_rDoc( i_rSwdoc ),
                                                                  mbNewFldLst(true),
                                                                  mpUpdtFlds( new SwDocUpdtFld( &m_rDoc ) ),
                                                                  mpFldTypes( new SwFldTypes() ),
                                                                  mnLockExpFld( 0 )
{
}

const SwFldTypes* DocumentFieldsManager::GetFldTypes() const
{
    return mpFldTypes;
}

/** Insert field types
 *
 * @param rFldTyp ???
 * @return Always returns a pointer to the type, if it's new or already added.
 */
SwFieldType* DocumentFieldsManager::InsertFldType(const SwFieldType &rFldTyp)
{
    sal_uInt16 nSize = mpFldTypes->size(),
            nFldWhich = rFldTyp.Which();

    sal_uInt16 i = INIT_FLDTYPES;

    switch( nFldWhich )
    {
    case RES_SETEXPFLD:
            //JP 29.01.96: SequenceFields start at INIT_FLDTYPES - 3!!
            //             Or we get doubble number circles!!
            //MIB 14.03.95: From now on also the SW3-Reader relies on &m_rDoc, when
            //constructing string pools and when reading SetExp fields
            if( nsSwGetSetExpType::GSE_SEQ & static_cast<const SwSetExpFieldType&>(rFldTyp).GetType() )
                i -= INIT_SEQ_FLDTYPES;
        // no break;
    case RES_DBFLD:
    case RES_USERFLD:
    case RES_DDEFLD:
        {
            const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
            OUString sFldNm( rFldTyp.GetName() );
            for( ; i < nSize; ++i )
                if( nFldWhich == (*mpFldTypes)[i]->Which() &&
                    rSCmp.isEqual( sFldNm, (*mpFldTypes)[i]->GetName() ))
                        return (*mpFldTypes)[i];
        }
        break;

    case RES_AUTHORITY:
        for( ; i < nSize; ++i )
            if( nFldWhich == (*mpFldTypes)[i]->Which() )
                return (*mpFldTypes)[i];
        break;

    default:
        for( i = 0; i < nSize; ++i )
            if( nFldWhich == (*mpFldTypes)[i]->Which() )
                return (*mpFldTypes)[i];
    }

    SwFieldType* pNew = rFldTyp.Copy();
    switch( nFldWhich )
    {
    case RES_DDEFLD:
        static_cast<SwDDEFieldType*>(pNew)->SetDoc( &m_rDoc );
        break;

    case RES_DBFLD:
    case RES_TABLEFLD:
    case RES_DATETIMEFLD:
    case RES_GETEXPFLD:
        static_cast<SwValueFieldType*>(pNew)->SetDoc( &m_rDoc );
        break;

    case RES_USERFLD:
    case RES_SETEXPFLD:
        static_cast<SwValueFieldType*>(pNew)->SetDoc( &m_rDoc );
        // JP 29.07.96: Optionally prepare FieldList for Calculator:
        mpUpdtFlds->InsertFldType( *pNew );
        break;
    case RES_AUTHORITY :
        static_cast<SwAuthorityFieldType*>(pNew)->SetDoc( &m_rDoc );
        break;
    }

    mpFldTypes->insert( mpFldTypes->begin() + nSize, pNew );
    m_rDoc.getIDocumentState().SetModified();

    return (*mpFldTypes)[ nSize ];
}

/// @returns the field type of the Doc
SwFieldType *DocumentFieldsManager::GetSysFldType( const sal_uInt16 eWhich ) const
{
    for( sal_uInt16 i = 0; i < INIT_FLDTYPES; ++i )
        if( eWhich == (*mpFldTypes)[i]->Which() )
            return (*mpFldTypes)[i];
    return 0;
}

/// Find first type with ResId and name
SwFieldType* DocumentFieldsManager::GetFldType(
    sal_uInt16 nResId,
    const OUString& rName,
    bool bDbFieldMatching // used in some UNO calls for RES_DBFLD to use different string matching code #i51815#
    ) const
{
    sal_uInt16 nSize = mpFldTypes->size(), i = 0;
    const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();

    switch( nResId )
    {
    case RES_SETEXPFLD:
            //JP 29.01.96: SequenceFields start at INIT_FLDTYPES - 3!!
            //             Or we get doubble number circles!!
            //MIB 14.03.95: From now on also the SW3-Reader relies on &m_rDoc, when
            //constructing string pools and when reading SetExp fields
        i = INIT_FLDTYPES - INIT_SEQ_FLDTYPES;
        break;

    case RES_DBFLD:
    case RES_USERFLD:
    case RES_DDEFLD:
    case RES_AUTHORITY:
        i = INIT_FLDTYPES;
        break;
    }

    SwFieldType* pRet = 0;
    for( ; i < nSize; ++i )
    {
        SwFieldType* pFldType = (*mpFldTypes)[i];

        OUString aFldName( pFldType->GetName() );
        if (bDbFieldMatching && nResId == RES_DBFLD)    // #i51815#
            aFldName = aFldName.replace(DB_DELIM, '.');

        if( nResId == pFldType->Which() &&
            rSCmp.isEqual( rName, aFldName ))
        {
            pRet = pFldType;
            break;
        }
    }
    return pRet;
}

/// Remove field type
void DocumentFieldsManager::RemoveFldType(size_t nFld)
{
    OSL_ENSURE( INIT_FLDTYPES <= nFld,  "don't remove InitFlds" );
    /*
     * Dependent fields present -> ErrRaise
     */
    size_t nSize = mpFldTypes->size();
    if(nFld < nSize)
    {
        SwFieldType* pTmp = (*mpFldTypes)[nFld];

        // JP 29.07.96: Optionally prepare FldLst for Calculator
        sal_uInt16 nWhich = pTmp->Which();
        switch( nWhich )
        {
        case RES_SETEXPFLD:
        case RES_USERFLD:
            mpUpdtFlds->RemoveFldType( *pTmp );
            // no break;
        case RES_DDEFLD:
            if( pTmp->HasWriterListeners() && !m_rDoc.IsUsed( *pTmp ) )
            {
                if( RES_SETEXPFLD == nWhich )
                    static_cast<SwSetExpFieldType*>(pTmp)->SetDeleted( true );
                else if( RES_USERFLD == nWhich )
                    static_cast<SwUserFieldType*>(pTmp)->SetDeleted( true );
                else
                    static_cast<SwDDEFieldType*>(pTmp)->SetDeleted( true );
                nWhich = 0;
            }
            break;
        }

        if( nWhich )
        {
            OSL_ENSURE( !pTmp->HasWriterListeners(), "Dependent fields present!" );
            // delete field type
            delete pTmp;
        }
        mpFldTypes->erase( mpFldTypes->begin() + nFld );
        m_rDoc.getIDocumentState().SetModified();
    }
}

// All have to be re-evaluated.
void DocumentFieldsManager::UpdateFlds( SfxPoolItem *pNewHt, bool bCloseDB )
{
    // Call Modify() for every field type,
    // dependent SwTxtFld get notified ...

    for( sal_uInt16 i=0; i < mpFldTypes->size(); ++i)
    {
        switch( (*mpFldTypes)[i]->Which() )
        {
            // Update table fields second to last
            // Update references last
        case RES_GETREFFLD:
        case RES_TABLEFLD:
        case RES_DBFLD:
        case RES_JUMPEDITFLD:
        case RES_REFPAGESETFLD:     // are never expanded!
            break;

        case RES_DDEFLD:
        {
            if( !pNewHt )
            {
                SwMsgPoolItem aUpdateDDE( RES_UPDATEDDETBL );
                (*mpFldTypes)[i]->ModifyNotification( 0, &aUpdateDDE );
            }
            else
                (*mpFldTypes)[i]->ModifyNotification( 0, pNewHt );
            break;
        }
        case RES_GETEXPFLD:
        case RES_SETEXPFLD:
        case RES_HIDDENTXTFLD:
        case RES_HIDDENPARAFLD:
            // Expression fields are treated separately
            if( !pNewHt )
                break;
        default:
            (*mpFldTypes)[i]->ModifyNotification ( 0, pNewHt );
        }
    }

    if( !IsExpFldsLocked() )
        UpdateExpFlds( 0, false );      // update expression fields

    // Tables
    UpdateTblFlds(pNewHt);

    // References
    UpdateRefFlds(pNewHt);
    if( bCloseDB )
    {
#if HAVE_FEATURE_DBCONNECTIVITY
        m_rDoc.GetDBManager()->CloseAll();
#endif
    }
    // Only evaluate on full update
    m_rDoc.getIDocumentState().SetModified();
}

void DocumentFieldsManager::InsDeletedFldType( SwFieldType& rFldTyp )
{
    // The FldType was marked as deleted and removed from the array.
    // One has to look &m_rDoc up again, now.
    // - If it's not present, it can be re-inserted.
    // - If the same type is found, the deleted one has to be renamed.

    sal_uInt16 nSize = mpFldTypes->size(), nFldWhich = rFldTyp.Which();
    sal_uInt16 i = INIT_FLDTYPES;

    OSL_ENSURE( RES_SETEXPFLD == nFldWhich ||
            RES_USERFLD == nFldWhich ||
            RES_DDEFLD == nFldWhich, "Wrong FldType" );

    const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
    const OUString& rFldNm = rFldTyp.GetName();
    SwFieldType* pFnd;

    for( ; i < nSize; ++i )
        if( nFldWhich == (pFnd = (*mpFldTypes)[i])->Which() &&
            rSCmp.isEqual( rFldNm, pFnd->GetName() ) )
        {
            // find new name
            sal_uInt16 nNum = 1;
            do {
                OUString sSrch = rFldNm + OUString::number( nNum );
                for( i = INIT_FLDTYPES; i < nSize; ++i )
                    if( nFldWhich == (pFnd = (*mpFldTypes)[i])->Which() &&
                        rSCmp.isEqual( sSrch, pFnd->GetName() ) )
                        break;

                if( i >= nSize )        // not found
                {
                    const_cast<OUString&>(rFldNm) = sSrch;
                    break;      // exit while loop
                }
                ++nNum;
            } while( true );
            break;
        }

    // not found, so insert and delete flag
    mpFldTypes->insert( mpFldTypes->begin() + nSize, &rFldTyp );
    switch( nFldWhich )
    {
    case RES_SETEXPFLD:
        static_cast<SwSetExpFieldType&>(rFldTyp).SetDeleted( false );
        break;
    case RES_USERFLD:
        static_cast<SwUserFieldType&>(rFldTyp).SetDeleted( false );
        break;
    case RES_DDEFLD:
        static_cast<SwDDEFieldType&>(rFldTyp).SetDeleted( false );
        break;
    }
}

bool DocumentFieldsManager::PutValueToField(const SwPosition & rPos,
                            const Any& rVal, sal_uInt16 nWhich)
{
    Any aOldVal;
    SwField * pField = GetFieldAtPos(rPos);

    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo() &&
        pField->QueryValue(aOldVal, nWhich))
    {
        SwUndo *const pUndo(new SwUndoFieldFromAPI(rPos, aOldVal, rVal, nWhich));
        m_rDoc.GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    return pField->PutValue(rVal, nWhich);
}

bool DocumentFieldsManager::UpdateFld(SwTxtFld * pDstTxtFld, SwField & rSrcFld,
                      SwMsgPoolItem * pMsgHnt,
                      bool bUpdateFlds)
{
    OSL_ENSURE(pDstTxtFld, "no field to update!");

    bool bTblSelBreak = false;

    SwFmtFld * pDstFmtFld = const_cast<SwFmtFld*>(&pDstTxtFld->GetFmtFld());
    SwField * pDstFld = pDstFmtFld->GetField();
    sal_uInt16 nFldWhich = rSrcFld.GetTyp()->Which();
    SwNodeIndex aTblNdIdx(pDstTxtFld->GetTxtNode());

    if (pDstFld->GetTyp()->Which() ==
        rSrcFld.GetTyp()->Which())
    {
        if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
        {
            SwPosition aPosition( pDstTxtFld->GetTxtNode() );
            aPosition.nContent = pDstTxtFld->GetStart();

            SwUndo *const pUndo( new SwUndoFieldFromDoc( aPosition, *pDstFld, rSrcFld, pMsgHnt, bUpdateFlds) );
            m_rDoc.GetIDocumentUndoRedo().AppendUndo(pUndo);
        }

        SwField * pNewFld = rSrcFld.CopyField();
        pDstFmtFld->SetField(pNewFld);

        switch( nFldWhich )
        {
        case RES_SETEXPFLD:
        case RES_GETEXPFLD:
        case RES_HIDDENTXTFLD:
        case RES_HIDDENPARAFLD:
            UpdateExpFlds( pDstTxtFld, true );
            break;

        case RES_TABLEFLD:
            {
                const SwTableNode* pTblNd =
                    m_rDoc.IsIdxInTbl(aTblNdIdx);
                if( pTblNd )
                {
                    SwTableFmlUpdate aTblUpdate( &pTblNd->
                                                 GetTable() );
                    if (bUpdateFlds)
                        UpdateTblFlds( &aTblUpdate );
                    else
                        pNewFld->GetTyp()->ModifyNotification(0, &aTblUpdate);

                    if (! bUpdateFlds)
                        bTblSelBreak = true;
                }
            }
            break;

        case RES_MACROFLD:
            if( bUpdateFlds && pDstTxtFld->GetpTxtNode() )
                (pDstTxtFld->GetpTxtNode())->
                    ModifyNotification( 0, pDstFmtFld );
            break;

        case RES_DBNAMEFLD:
        case RES_DBNEXTSETFLD:
        case RES_DBNUMSETFLD:
        case RES_DBSETNUMBERFLD:
            m_rDoc.ChgDBData(static_cast<SwDBNameInfField*>( pNewFld)->GetRealDBData());
            pNewFld->GetTyp()->UpdateFlds();

            break;

        case RES_DBFLD:
#if HAVE_FEATURE_DBCONNECTIVITY
            {
                // JP 10.02.96: call ChgValue, so that the style change sets the
                // ContentString correctly
                SwDBField* pDBFld = static_cast<SwDBField*>(pNewFld);
                if (pDBFld->IsInitialized())
                    pDBFld->ChgValue( pDBFld->GetValue(), true );

                pDBFld->ClearInitialized();
                pDBFld->InitContent();
            }
#endif
            // no break;

        default:
            pDstFmtFld->ModifyNotification( 0, pMsgHnt );
        }

        // The fields we can calculate here are being triggered for an update
        // here explicitly.
        if( nFldWhich == RES_USERFLD )
            UpdateUsrFlds();
    }

    return bTblSelBreak;
}

/// Update reference and table fields
void DocumentFieldsManager::UpdateRefFlds( SfxPoolItem* pHt )
{
    SwFieldType* pFldType;
    for( sal_uInt16 i = 0; i < mpFldTypes->size(); ++i )
        if( RES_GETREFFLD == ( pFldType = (*mpFldTypes)[i] )->Which() )
            pFldType->ModifyNotification( 0, pHt );
}

void DocumentFieldsManager::UpdateTblFlds( SfxPoolItem* pHt )
{
    OSL_ENSURE( !pHt || RES_TABLEFML_UPDATE  == pHt->Which(),
            "What MessageItem is &m_rDoc?" );

    SwFieldType* pFldType(0);

    for (sal_uInt16 i = 0; i < mpFldTypes->size(); ++i)
    {
        if( RES_TABLEFLD == ( pFldType = (*mpFldTypes)[i] )->Which() )
        {
            SwTableFmlUpdate* pUpdtFld = 0;
            if( pHt && RES_TABLEFML_UPDATE == pHt->Which() )
                pUpdtFld = static_cast<SwTableFmlUpdate*>(pHt);

            SwIterator<SwFmtFld,SwFieldType> aIter( *pFldType );
            for( SwFmtFld* pFmtFld = aIter.First(); pFmtFld; pFmtFld = aIter.Next() )
            {
                if( pFmtFld->GetTxtFld() )
                {
                    SwTblField* pFld = static_cast<SwTblField*>(pFmtFld->GetField());

                    if( pUpdtFld )
                    {
                        // table where &m_rDoc field is located
                        const SwTableNode* pTblNd;
                        const SwTxtNode& rTxtNd = pFmtFld->GetTxtFld()->GetTxtNode();
                        if( !rTxtNd.GetNodes().IsDocNodes() ||
                            0 == ( pTblNd = rTxtNd.FindTableNode() ) )
                            continue;

                        switch( pUpdtFld->eFlags )
                        {
                        case TBL_CALC:
                            // re-set the value flag
                            // JP 17.06.96: internal representation of all formulas
                            //              (reference to other table!!!)
                            if( nsSwExtendedSubType::SUB_CMD & pFld->GetSubType() )
                                pFld->PtrToBoxNm( pUpdtFld->pTbl );
                            else
                                pFld->ChgValid( false );
                            break;
                        case TBL_BOXNAME:
                            // is &m_rDoc the wanted table?
                            if( &pTblNd->GetTable() == pUpdtFld->pTbl )
                                // to the external representation
                                pFld->PtrToBoxNm( pUpdtFld->pTbl );
                            break;
                        case TBL_BOXPTR:
                            // to the internal representation
                            // JP 17.06.96: internal representation on all formulas
                            //              (reference to other table!!!)
                            pFld->BoxNmToPtr( pUpdtFld->pTbl );
                            break;
                        case TBL_RELBOXNAME:
                            // is &m_rDoc the wanted table?
                            if( &pTblNd->GetTable() == pUpdtFld->pTbl )
                                // to the relative representation
                                pFld->ToRelBoxNm( pUpdtFld->pTbl );
                            break;
                        default:
                            break;
                        }
                    }
                    else
                        // reset the value flag for all
                        pFld->ChgValid( false );
                }
            }

            break;
        }
        pFldType = 0;
    }

    // process all table box formulas
    const SfxPoolItem* pItem;
    sal_uInt32 nMaxItems = m_rDoc.GetAttrPool().GetItemCount2( RES_BOXATR_FORMULA );
    for (sal_uInt32 i = 0; i < nMaxItems; ++i)
    {
        if( 0 != (pItem = m_rDoc.GetAttrPool().GetItem2( RES_BOXATR_FORMULA, i ) ) &&
            static_cast<const SwTblBoxFormula*>(pItem)->GetDefinedIn() )
        {
            const_cast<SwTblBoxFormula*>(static_cast<const SwTblBoxFormula*>(pItem))->ChangeState( pHt );
        }
    }

    // all fields/boxes are now invalid, so we can start to calculate
    if( pHt && ( RES_TABLEFML_UPDATE != pHt->Which() ||
                TBL_CALC != static_cast<SwTableFmlUpdate*>(pHt)->eFlags ))
        return ;

    SwCalc* pCalc = 0;

    if( pFldType )
    {
        SwIterator<SwFmtFld,SwFieldType> aIter( *pFldType );
        for( SwFmtFld* pFmtFld = aIter.Last(); pFmtFld; pFmtFld = aIter.Previous() )
        {
                // start calculation at the end
                // new fields are inserted at the beginning of the modify chain
                // that gives faster calculation on import
                // mba: do we really need &m_rDoc "optimization"? Is it still valid?
                SwTblField* pFld;
                if( !pFmtFld->GetTxtFld() || (nsSwExtendedSubType::SUB_CMD &
                    (pFld = static_cast<SwTblField*>(pFmtFld->GetField()))->GetSubType() ))
                    continue;

                // needs to be recalculated
                if( !pFld->IsValid() )
                {
                    // table where &m_rDoc field is located
                    const SwTxtNode& rTxtNd = pFmtFld->GetTxtFld()->GetTxtNode();
                    if( !rTxtNd.GetNodes().IsDocNodes() )
                        continue;
                    const SwTableNode* pTblNd = rTxtNd.FindTableNode();
                    if( !pTblNd )
                        continue;

                    // if &m_rDoc field is not in the to-be-updated table, skip it
                    if( pHt && &pTblNd->GetTable() !=
                                            static_cast<SwTableFmlUpdate*>(pHt)->pTbl )
                        continue;

                    if( !pCalc )
                        pCalc = new SwCalc( m_rDoc );

                    // get the values of all SetExpression fields that are valid
                    // until the table
                    SwFrm* pFrm = 0;
                    if( pTblNd->GetIndex() < m_rDoc.GetNodes().GetEndOfExtras().GetIndex() )
                    {
                        // is in the special section, that's expensive!
                        Point aPt;      // return the first frame of the layout - Tab.Headline!!
                        pFrm = rTxtNd.getLayoutFrm( m_rDoc.getIDocumentLayoutAccess().GetCurrentLayout(), &aPt );
                        if( pFrm )
                        {
                            SwPosition aPos( *pTblNd );
                            if( GetBodyTxtNode( m_rDoc, aPos, *pFrm ) )
                                FldsToCalc( *pCalc, _SetGetExpFld(
                                    aPos.nNode, pFmtFld->GetTxtFld(),
                                    &aPos.nContent ));
                            else
                                pFrm = 0;
                        }
                    }
                    if( !pFrm )
                    {
                        // create index to determine the TextNode
                        SwNodeIndex aIdx( rTxtNd );
                        FldsToCalc( *pCalc,
                            _SetGetExpFld( aIdx, pFmtFld->GetTxtFld() ));
                    }

                    SwTblCalcPara aPara( *pCalc, pTblNd->GetTable() );
                    pFld->CalcField( aPara );
                    if( aPara.IsStackOverflow() )
                    {
                        bool const bResult = aPara.CalcWithStackOverflow();
                        if (bResult)
                        {
                            pFld->CalcField( aPara );
                        }
                        OSL_ENSURE(bResult,
                                "the chained formula could no be calculated");
                    }
                    pCalc->SetCalcError( CALC_NOERR );
                }
                pFmtFld->ModifyNotification( 0, pHt );
        }
    }

    // calculate the formula at the boxes
    for (sal_uInt32 i = 0; i < nMaxItems; ++i )
    {
        if( 0 != (pItem = m_rDoc.GetAttrPool().GetItem2( RES_BOXATR_FORMULA, i ) ) &&
            static_cast<const SwTblBoxFormula*>(pItem)->GetDefinedIn() &&
            !static_cast<const SwTblBoxFormula*>(pItem)->IsValid() )
        {
            SwTblBoxFormula* pFml = const_cast<SwTblBoxFormula*>(static_cast<const SwTblBoxFormula*>(pItem));
            SwTableBox* pBox = pFml->GetTableBox();
            if( pBox && pBox->GetSttNd() &&
                pBox->GetSttNd()->GetNodes().IsDocNodes() )
            {
                const SwTableNode* pTblNd = pBox->GetSttNd()->FindTableNode();
                if( !pHt || &pTblNd->GetTable() ==
                                            static_cast<SwTableFmlUpdate*>(pHt)->pTbl )
                {
                    double nValue;
                    if( !pCalc )
                        pCalc = new SwCalc( m_rDoc );

                    // get the values of all SetExpression fields that are valid
                    // until the table
                    SwFrm* pFrm = 0;
                    if( pTblNd->GetIndex() < m_rDoc.GetNodes().GetEndOfExtras().GetIndex() )
                    {
                        // is in the special section, that's expensive!
                        Point aPt;      // return the first frame of the layout - Tab.Headline!!
                        SwNodeIndex aCNdIdx( *pTblNd, +2 );
                        SwCntntNode* pCNd = aCNdIdx.GetNode().GetCntntNode();
                        if( !pCNd )
                            pCNd = m_rDoc.GetNodes().GoNext( &aCNdIdx );

                        if( pCNd && 0 != (pFrm = pCNd->getLayoutFrm( m_rDoc.getIDocumentLayoutAccess().GetCurrentLayout(), &aPt )) )
                        {
                            SwPosition aPos( *pCNd );
                            if( GetBodyTxtNode( m_rDoc, aPos, *pFrm ) )
                                FldsToCalc( *pCalc, _SetGetExpFld( aPos.nNode ));
                            else
                                pFrm = 0;
                        }
                    }
                    if( !pFrm )
                    {
                        // create index to determine the TextNode
                        SwNodeIndex aIdx( *pTblNd );
                        FldsToCalc( *pCalc, _SetGetExpFld( aIdx ));
                    }

                    SwTblCalcPara aPara( *pCalc, pTblNd->GetTable() );
                    pFml->Calc( aPara, nValue );

                    if( aPara.IsStackOverflow() )
                    {
                        bool const bResult = aPara.CalcWithStackOverflow();
                        if (bResult)
                        {
                            pFml->Calc( aPara, nValue );
                        }
                        OSL_ENSURE(bResult,
                                "the chained formula could no be calculated");
                    }

                    SwFrmFmt* pFmt = pBox->ClaimFrmFmt();
                    SfxItemSet aTmp( m_rDoc.GetAttrPool(),
                                    RES_BOXATR_BEGIN,RES_BOXATR_END-1 );

                    if( pCalc->IsCalcError() )
                        nValue = DBL_MAX;
                    aTmp.Put( SwTblBoxValue( nValue ));
                    if( SfxItemState::SET != pFmt->GetItemState( RES_BOXATR_FORMAT ))
                        aTmp.Put( SwTblBoxNumFormat( 0 ));
                    pFmt->SetFmtAttr( aTmp );

                    pCalc->SetCalcError( CALC_NOERR );
                }
            }
        }
    }

    delete pCalc;
}

void DocumentFieldsManager::UpdateExpFlds( SwTxtFld* pUpdtFld, bool bUpdRefFlds )
{
    if( IsExpFldsLocked() || m_rDoc.IsInReading() )
        return;

    bool bOldInUpdateFlds = mpUpdtFlds->IsInUpdateFlds();
    mpUpdtFlds->SetInUpdateFlds( true );

    mpUpdtFlds->MakeFldList( m_rDoc, true, GETFLD_ALL );
    mbNewFldLst = false;

    if( mpUpdtFlds->GetSortLst()->empty() )
    {
        if( bUpdRefFlds )
            UpdateRefFlds(NULL);

        mpUpdtFlds->SetInUpdateFlds( bOldInUpdateFlds );
        mpUpdtFlds->SetFieldsDirty( false );
        return ;
    }

    sal_uInt16 nWhich, n;

    // Hash table for all string replacements is filled on-the-fly.
    // Try to fabricate an uneven number.
    sal_uInt16 nStrFmtCnt = (( mpFldTypes->size() / 7 ) + 1 ) * 7;
    SwHash** pHashStrTbl = new SwHash*[ nStrFmtCnt ];
    memset( pHashStrTbl, 0, sizeof( _HashStr* ) * nStrFmtCnt );

    {
        const SwFieldType* pFldType;
        // process separately:
        for( n = mpFldTypes->size(); n; )
            switch( ( pFldType = (*mpFldTypes)[ --n ] )->Which() )
            {
            case RES_USERFLD:
                {
                    // Entry present?
                    sal_uInt16 nPos;
                    const OUString& rNm = pFldType->GetName();
                    OUString sExpand(const_cast<SwUserFieldType*>(static_cast<const SwUserFieldType*>(pFldType))->Expand(nsSwGetSetExpType::GSE_STRING, 0, 0));
                    SwHash* pFnd = Find( rNm, pHashStrTbl, nStrFmtCnt, &nPos );
                    if( pFnd )
                        // modify entry in the hash table
                        static_cast<_HashStr*>(pFnd)->aSetStr = sExpand;
                    else
                        // insert the new entry
                        *(pHashStrTbl + nPos ) = new _HashStr( rNm, sExpand,
                                                static_cast<_HashStr*>(*(pHashStrTbl + nPos)) );
                }
                break;
            case RES_SETEXPFLD:
                const_cast<SwSetExpFieldType*>(static_cast<const SwSetExpFieldType*>(pFldType))->SetOutlineChgNd( 0 );
                break;
            }
    }

    // The array is filled with all fields; start calculation.
    SwCalc aCalc( m_rDoc );

#if HAVE_FEATURE_DBCONNECTIVITY
    OUString sDBNumNm( SwFieldType::GetTypeStr( TYP_DBSETNUMBERFLD ) );

    // already set the current record number
    SwDBManager* pMgr = m_rDoc.GetDBManager();
    pMgr->CloseAll( false );

    SvtSysLocale aSysLocale;
    const LocaleDataWrapper* pLclData = aSysLocale.GetLocaleDataPtr();
    const long nLang = pLclData->getLanguageTag().getLanguageType();
    bool bCanFill = pMgr->FillCalcWithMergeData( m_rDoc.GetNumberFormatter(), nLang, true, aCalc );
#endif

    // Make sure we don't hide all sections, which would lead to a crash. First, count how many of them do we have.
    int nShownSections = 0;
    for( _SetGetExpFlds::const_iterator it = mpUpdtFlds->GetSortLst()->begin(); it != mpUpdtFlds->GetSortLst()->end(); ++it )
    {
        SwSection* pSect = const_cast<SwSection*>((*it)->GetSection());
        if ( pSect && !pSect->IsCondHidden())
            nShownSections++;
    }

    OUString aNew;
    for( _SetGetExpFlds::const_iterator it = mpUpdtFlds->GetSortLst()->begin(); it != mpUpdtFlds->GetSortLst()->end(); ++it )
    {
        SwSection* pSect = const_cast<SwSection*>((*it)->GetSection());
        if( pSect )
        {

            SwSbxValue aValue = aCalc.Calculate(
                                        pSect->GetCondition() );
            if(!aValue.IsVoidValue())
            {
                // Do we want to hide this one?
                bool bHide = aValue.GetBool();
                if (bHide && !pSect->IsCondHidden())
                {
                    // This section will be hidden, but it wasn't before
                    if (nShownSections == 1)
                    {
                        // Is the last node part of a section?
                        SwPaM aPam(m_rDoc.GetNodes());
                        aPam.Move(fnMoveForward, fnGoDoc);
                        if (aPam.Start()->nNode.GetNode().StartOfSectionNode()->IsSectionNode())
                        {
                            // This would be the last section, so set its condition to false, and avoid hiding it.
                            OUString aCond("0");
                            pSect->SetCondition(aCond);
                            bHide = false;
                        }
                    }
                    nShownSections--;
                }
                pSect->SetCondHidden( bHide );
            }
            continue;
        }

        SwTxtFld* pTxtFld = const_cast<SwTxtFld*>((*it)->GetTxtFld());
        if( !pTxtFld )
        {
            OSL_ENSURE( false, "what's wrong now'" );
            continue;
        }

        SwFmtFld* pFmtFld = const_cast<SwFmtFld*>(&pTxtFld->GetFmtFld());
        const SwField* pFld = pFmtFld->GetField();

        switch( nWhich = pFld->GetTyp()->Which() )
        {
        case RES_HIDDENTXTFLD:
        {
            SwHiddenTxtField* pHFld = const_cast<SwHiddenTxtField*>(static_cast<const SwHiddenTxtField*>(pFld));
            SwSbxValue aValue = aCalc.Calculate( pHFld->GetPar1() );
            bool bValue = !aValue.GetBool();
            if(!aValue.IsVoidValue())
            {
                pHFld->SetValue( bValue );
                // evaluate field
                pHFld->Evaluate(&m_rDoc);
            }
        }
        break;
        case RES_HIDDENPARAFLD:
        {
            SwHiddenParaField* pHPFld = const_cast<SwHiddenParaField*>(static_cast<const SwHiddenParaField*>(pFld));
            SwSbxValue aValue = aCalc.Calculate( pHPFld->GetPar1() );
            bool bValue = aValue.GetBool();
            if(!aValue.IsVoidValue())
                pHPFld->SetHidden( bValue );
        }
        break;
        case RES_DBSETNUMBERFLD:
#if HAVE_FEATURE_DBCONNECTIVITY
        {
            const_cast<SwDBSetNumberField*>(static_cast<const SwDBSetNumberField*>(pFld))->Evaluate(&m_rDoc);
            aCalc.VarChange( sDBNumNm, static_cast<const SwDBSetNumberField*>(pFld)->GetSetNumber());
        }
#endif
        break;
        case RES_DBNEXTSETFLD:
        case RES_DBNUMSETFLD:
#if HAVE_FEATURE_DBCONNECTIVITY
        {
            UpdateDBNumFlds( *const_cast<SwDBNameInfField*>(static_cast<const SwDBNameInfField*>(pFld)), aCalc );
            if( bCanFill )
                bCanFill = pMgr->FillCalcWithMergeData( m_rDoc.GetNumberFormatter(), nLang, true, aCalc );
        }
#endif
        break;
        case RES_DBFLD:
        {
#if HAVE_FEATURE_DBCONNECTIVITY
            // evaluate field
            const_cast<SwDBField*>(static_cast<const SwDBField*>(pFld))->Evaluate();

            SwDBData aTmpDBData(static_cast<const SwDBField*>(pFld)->GetDBData());

            if( pMgr->IsDataSourceOpen(aTmpDBData.sDataSource, aTmpDBData.sCommand, false))
                aCalc.VarChange( sDBNumNm, pMgr->GetSelectedRecordId(aTmpDBData.sDataSource, aTmpDBData.sCommand, aTmpDBData.nCommandType));

            const OUString& rName = pFld->GetTyp()->GetName();

            // Add entry to hash table
            // Entry present?
            sal_uInt16 nPos;
            SwHash* pFnd = Find( rName, pHashStrTbl, nStrFmtCnt, &nPos );
            OUString const value(pFld->ExpandField(m_rDoc.IsClipBoard()));
            if( pFnd )
            {
                // Modify entry in the hash table
                static_cast<_HashStr*>(pFnd)->aSetStr = value;
            }
            else
            {
                // insert new entry
                *(pHashStrTbl + nPos ) = new _HashStr( rName,
                    value, static_cast<_HashStr *>(*(pHashStrTbl + nPos)));
            }
#endif
        }
        break;
        case RES_GETEXPFLD:
        case RES_SETEXPFLD:
        {
            if( nsSwGetSetExpType::GSE_STRING & pFld->GetSubType() )        // replace String
            {
                if( RES_GETEXPFLD == nWhich )
                {
                    SwGetExpField* pGFld = const_cast<SwGetExpField*>(static_cast<const SwGetExpField*>(pFld));

                    if( (!pUpdtFld || pUpdtFld == pTxtFld )
                        && pGFld->IsInBodyTxt() )
                    {
                        aNew = LookString( pHashStrTbl, nStrFmtCnt,
                                    pGFld->GetFormula() );
                        pGFld->ChgExpStr( aNew );
                    }
                }
                else
                {
                    SwSetExpField* pSFld = const_cast<SwSetExpField*>(static_cast<const SwSetExpField*>(pFld));
                    // is the "formula" a field?
                    aNew = LookString( pHashStrTbl, nStrFmtCnt,
                                pSFld->GetFormula() );

                    if( aNew.isEmpty() )               // nothing found then the formula is the new value
                        aNew = pSFld->GetFormula();

                    // only update one field
                    if( !pUpdtFld || pUpdtFld == pTxtFld )
                        pSFld->ChgExpStr( aNew );

                    // lookup the field's name
                    aNew = static_cast<SwSetExpFieldType*>(pSFld->GetTyp())->GetSetRefName();
                    // Entry present?
                    sal_uInt16 nPos;
                    SwHash* pFnd = Find( aNew, pHashStrTbl, nStrFmtCnt, &nPos );
                    if( pFnd )
                        // Modify entry in the hash table
                        static_cast<_HashStr*>(pFnd)->aSetStr = pSFld->GetExpStr();
                    else
                        // insert new entry
                        *(pHashStrTbl + nPos ) = pFnd = new _HashStr( aNew,
                                        pSFld->GetExpStr(),
                                        static_cast<_HashStr*>(*(pHashStrTbl + nPos) ));

                    // Extension for calculation with Strings
                    SwSbxValue aValue;
                    aValue.PutString( static_cast<_HashStr*>(pFnd)->aSetStr );
                    aCalc.VarChange( aNew, aValue );
                }
            }
            else            // recalculate formula
            {
                if( RES_GETEXPFLD == nWhich )
                {
                    SwGetExpField* pGFld = const_cast<SwGetExpField*>(static_cast<const SwGetExpField*>(pFld));

                    if( (!pUpdtFld || pUpdtFld == pTxtFld )
                        && pGFld->IsInBodyTxt() )
                    {
                        SwSbxValue aValue = aCalc.Calculate(
                                        pGFld->GetFormula());
                        if(!aValue.IsVoidValue())
                            pGFld->SetValue(aValue.GetDouble() );
                    }
                }
                else
                {
                    SwSetExpField* pSFld = const_cast<SwSetExpField*>(static_cast<const SwSetExpField*>(pFld));
                    SwSetExpFieldType* pSFldTyp = static_cast<SwSetExpFieldType*>(pFld->GetTyp());
                    aNew = pSFldTyp->GetName();

                    SwNode* pSeqNd = 0;

                    if( pSFld->IsSequenceFld() )
                    {
                        const sal_uInt8 nLvl = pSFldTyp->GetOutlineLvl();
                        if( MAXLEVEL > nLvl )
                        {
                            // test if the Number needs to be updated
                            pSeqNd = m_rDoc.GetNodes()[ (*it)->GetNode() ];

                            const SwTxtNode* pOutlNd = pSeqNd->
                                    FindOutlineNodeOfLevel( nLvl );
                            if( pSFldTyp->GetOutlineChgNd() != pOutlNd )
                            {
                                pSFldTyp->SetOutlineChgNd( pOutlNd );
                                aCalc.VarChange( aNew, 0 );
                            }
                        }
                    }

                    aNew += "=";
                    aNew += pSFld->GetFormula();

                    SwSbxValue aValue = aCalc.Calculate( aNew );
                    double nErg = aValue.GetDouble();
                    // only update one field
                    if( !aValue.IsVoidValue() && (!pUpdtFld || pUpdtFld == pTxtFld) )
                    {
                        pSFld->SetValue( nErg );

                        if( pSeqNd )
                            pSFldTyp->SetChapter( *pSFld, *pSeqNd );
                    }
                }
            }
        }
        } // switch

        pFmtFld->ModifyNotification( 0, 0 );        // trigger formatting

        if( pUpdtFld == pTxtFld )       // if only &m_rDoc one is updated
        {
            if( RES_GETEXPFLD == nWhich ||      // only GetField or
                RES_HIDDENTXTFLD == nWhich ||   // HiddenTxt?
                RES_HIDDENPARAFLD == nWhich)    // HiddenParaFld?
                break;                          // quit
            pUpdtFld = 0;                       // update all from here on
        }
    }

#if HAVE_FEATURE_DBCONNECTIVITY
    pMgr->CloseAll(false);
#endif
    // delete hash table
    ::DeleteHashTable( pHashStrTbl, nStrFmtCnt );

    // update reference fields
    if( bUpdRefFlds )
        UpdateRefFlds(NULL);

    mpUpdtFlds->SetInUpdateFlds( bOldInUpdateFlds );
    mpUpdtFlds->SetFieldsDirty( false );
}

/// Insert field type that was marked as deleted
void DocumentFieldsManager::UpdateUsrFlds()
{
    SwCalc* pCalc = 0;
    const SwFieldType* pFldType;
    for( sal_uInt16 i = INIT_FLDTYPES; i < mpFldTypes->size(); ++i )
        if( RES_USERFLD == ( pFldType = (*mpFldTypes)[i] )->Which() )
        {
            if( !pCalc )
                pCalc = new SwCalc( m_rDoc );
            const_cast<SwUserFieldType*>(static_cast<const SwUserFieldType*>(pFldType))->GetValue( *pCalc );
        }

    if( pCalc )
    {
        delete pCalc;
        m_rDoc.getIDocumentState().SetModified();
    }
}

void DocumentFieldsManager::UpdatePageFlds( SfxPoolItem* pMsgHnt )
{
    SwFieldType* pFldType;
    for( sal_uInt16 i = 0; i < INIT_FLDTYPES; ++i )
        switch( ( pFldType = (*mpFldTypes)[ i ] )->Which() )
        {
        case RES_PAGENUMBERFLD:
        case RES_CHAPTERFLD:
        case RES_GETEXPFLD:
        case RES_REFPAGEGETFLD:
            pFldType->ModifyNotification( 0, pMsgHnt );
            break;
        case RES_DOCSTATFLD:
            pFldType->ModifyNotification( 0, 0 );
            break;
        }
    SetNewFldLst(true);
}

void DocumentFieldsManager::LockExpFlds()
{
    ++mnLockExpFld;
}

void DocumentFieldsManager::UnlockExpFlds()
{
    assert(mnLockExpFld != 0);
    if( mnLockExpFld )
        --mnLockExpFld;
}

bool DocumentFieldsManager::IsExpFldsLocked() const
{
    return 0 != mnLockExpFld;
}

SwDocUpdtFld& DocumentFieldsManager::GetUpdtFlds() const
{
    return *mpUpdtFlds;
}

bool DocumentFieldsManager::SetFieldsDirty( bool b, const SwNode* pChk, sal_uLong nLen )
{
    // See if the supplied nodes actually contain fields.
    // If they don't, the flag doesn't need to be changed.
    bool bFldsFnd = false;
    if( b && pChk && !GetUpdtFlds().IsFieldsDirty() && !m_rDoc.IsInDtor()
        // ?? what's up with Undo, this is also wanted there!
        /*&& &pChk->GetNodes() == &GetNodes()*/ )
    {
        b = false;
        if( !nLen )
            ++nLen;
        sal_uLong nStt = pChk->GetIndex();
        const SwNodes& rNds = pChk->GetNodes();
        while( nLen-- )
        {
            const SwTxtNode* pTNd = rNds[ nStt++ ]->GetTxtNode();
            if( pTNd )
            {
                if( pTNd->GetAttrOutlineLevel() != 0 )
                    // update chapter fields
                    b = true;
                else if( pTNd->GetpSwpHints() && pTNd->GetSwpHints().Count() )
                {
                    const size_t nEnd = pTNd->GetSwpHints().Count();
                    for( size_t n = 0 ; n < nEnd; ++n )
                    {
                        const SwTxtAttr* pAttr = pTNd->GetSwpHints()[ n ];
                        if ( pAttr->Which() == RES_TXTATR_FIELD )
                        {
                            b = true;
                            break;
                        }
                    }
                }

                if( b )
                    break;
            }
        }
        bFldsFnd = b;
    }
    GetUpdtFlds().SetFieldsDirty( b );
    return bFldsFnd;
}

void DocumentFieldsManager::SetFixFields( bool bOnlyTimeDate, const DateTime* pNewDateTime )
{
    bool bIsModified = m_rDoc.getIDocumentState().IsModified();

    sal_Int32 nDate;
    sal_Int64 nTime;
    if( pNewDateTime )
    {
        nDate = pNewDateTime->GetDate();
        nTime = pNewDateTime->GetTime();
    }
    else
    {
        nDate = Date( Date::SYSTEM ).GetDate();
        nTime = tools::Time( tools::Time::SYSTEM ).GetTime();
    }

    sal_uInt16 aTypes[5] = {
        /*0*/   RES_DOCINFOFLD,
        /*1*/   RES_AUTHORFLD,
        /*2*/   RES_EXTUSERFLD,
        /*3*/   RES_FILENAMEFLD,
        /*4*/   RES_DATETIMEFLD };  // MUST be at the end!

    sal_uInt16 nStt = bOnlyTimeDate ? 4 : 0;

    for( ; nStt < 5; ++nStt )
    {
        SwFieldType* pFldType = GetSysFldType( aTypes[ nStt ] );
        SwIterator<SwFmtFld,SwFieldType> aIter( *pFldType );
        for( SwFmtFld* pFmtFld = aIter.First(); pFmtFld; pFmtFld = aIter.Next() )
        {
            if( pFmtFld && pFmtFld->GetTxtFld() )
            {
                bool bChgd = false;
                switch( aTypes[ nStt ] )
                {
                case RES_DOCINFOFLD:
                    if( static_cast<SwDocInfoField*>(pFmtFld->GetField())->IsFixed() )
                    {
                        bChgd = true;
                        SwDocInfoField* pDocInfFld = static_cast<SwDocInfoField*>(pFmtFld->GetField());
                        pDocInfFld->SetExpansion( static_cast<SwDocInfoFieldType*>(
                                    pDocInfFld->GetTyp())->Expand(
                                        pDocInfFld->GetSubType(),
                                        pDocInfFld->GetFormat(),
                                        pDocInfFld->GetLanguage(),
                                        pDocInfFld->GetName() ) );
                    }
                    break;

                case RES_AUTHORFLD:
                    if( static_cast<SwAuthorField*>(pFmtFld->GetField())->IsFixed() )
                    {
                        bChgd = true;
                        SwAuthorField* pAuthorFld = static_cast<SwAuthorField*>(pFmtFld->GetField());
                        pAuthorFld->SetExpansion( SwAuthorFieldType::Expand( pAuthorFld->GetFormat() ) );
                    }
                    break;

                case RES_EXTUSERFLD:
                    if( static_cast<SwExtUserField*>(pFmtFld->GetField())->IsFixed() )
                    {
                        bChgd = true;
                        SwExtUserField* pExtUserFld = static_cast<SwExtUserField*>(pFmtFld->GetField());
                        pExtUserFld->SetExpansion( SwExtUserFieldType::Expand(
                                            pExtUserFld->GetSubType(),
                                            pExtUserFld->GetFormat()));
                    }
                    break;

                case RES_DATETIMEFLD:
                    if( static_cast<SwDateTimeField*>(pFmtFld->GetField())->IsFixed() )
                    {
                        bChgd = true;
                        static_cast<SwDateTimeField*>(pFmtFld->GetField())->SetDateTime(
                                                    DateTime(Date(nDate), tools::Time(nTime)) );
                    }
                    break;

                case RES_FILENAMEFLD:
                    if( static_cast<SwFileNameField*>(pFmtFld->GetField())->IsFixed() )
                    {
                        bChgd = true;
                        SwFileNameField* pFileNameFld =
                            static_cast<SwFileNameField*>(pFmtFld->GetField());
                        pFileNameFld->SetExpansion( static_cast<SwFileNameFieldType*>(
                                    pFileNameFld->GetTyp())->Expand(
                                            pFileNameFld->GetFormat() ) );
                    }
                    break;
                }

                // Trigger formatting
                if( bChgd )
                    pFmtFld->ModifyNotification( 0, 0 );
            }
        }
    }

    if( !bIsModified )
        m_rDoc.getIDocumentState().ResetModified();
}

void DocumentFieldsManager::FldsToCalc( SwCalc& rCalc, const _SetGetExpFld& rToThisFld )
{
    // create the sorted list of all SetFields
    mpUpdtFlds->MakeFldList( m_rDoc, mbNewFldLst, GETFLD_CALC );
    mbNewFldLst = false;

#if !HAVE_FEATURE_DBCONNECTIVITY
    SwDBManager* pMgr = NULL;
#else
    SwDBManager* pMgr = m_rDoc.GetDBManager();
    pMgr->CloseAll(false);
#endif

    if( !mpUpdtFlds->GetSortLst()->empty() )
    {
        _SetGetExpFlds::const_iterator const itLast =
            mpUpdtFlds->GetSortLst()->upper_bound(
                const_cast<_SetGetExpFld*>(&rToThisFld));
        for( _SetGetExpFlds::const_iterator it = mpUpdtFlds->GetSortLst()->begin(); it != itLast; ++it )
            lcl_CalcFld( m_rDoc, rCalc, **it, pMgr );
    }
#if HAVE_FEATURE_DBCONNECTIVITY
    pMgr->CloseAll(false);
#endif
}

void DocumentFieldsManager::FldsToCalc( SwCalc& rCalc, sal_uLong nLastNd, sal_uInt16 nLastCnt )
{
    // create the sorted list of all SetFields
    mpUpdtFlds->MakeFldList( m_rDoc, mbNewFldLst, GETFLD_CALC );
    mbNewFldLst = false;

#if !HAVE_FEATURE_DBCONNECTIVITY
    SwDBManager* pMgr = NULL;
#else
    SwDBManager* pMgr = m_rDoc.GetDBManager();
    pMgr->CloseAll(false);
#endif

    for( _SetGetExpFlds::const_iterator it = mpUpdtFlds->GetSortLst()->begin();
        it != mpUpdtFlds->GetSortLst()->end() &&
        ( (*it)->GetNode() < nLastNd ||
          ( (*it)->GetNode() == nLastNd && (*it)->GetCntnt() <= nLastCnt )
        );
        ++it )
    {
        lcl_CalcFld( m_rDoc, rCalc, **it, pMgr );
    }

#if HAVE_FEATURE_DBCONNECTIVITY
    pMgr->CloseAll(false);
#endif
}

void DocumentFieldsManager::FldsToExpand( SwHash**& ppHashTbl, sal_uInt16& rTblSize,
                            const _SetGetExpFld& rToThisFld )
{
    // create the sorted list of all SetFields
    mpUpdtFlds->MakeFldList( m_rDoc, mbNewFldLst, GETFLD_EXPAND );
    mbNewFldLst = false;

    // Hash table for all string replacements is filled on-the-fly.
    // Try to fabricate an uneven number.
    rTblSize = (( mpUpdtFlds->GetSortLst()->size() / 7 ) + 1 ) * 7;
    ppHashTbl = new SwHash*[ rTblSize ];
    memset( ppHashTbl, 0, sizeof( _HashStr* ) * rTblSize );

    _SetGetExpFlds::const_iterator const itLast =
        mpUpdtFlds->GetSortLst()->upper_bound(
            const_cast<_SetGetExpFld*>(&rToThisFld));

    for( _SetGetExpFlds::const_iterator it = mpUpdtFlds->GetSortLst()->begin(); it != itLast; ++it )
    {
        const SwTxtFld* pTxtFld = (*it)->GetTxtFld();
        if( !pTxtFld )
            continue;

        const SwField* pFld = pTxtFld->GetFmtFld().GetField();
        switch( pFld->GetTyp()->Which() )
        {
        case RES_SETEXPFLD:
            if( nsSwGetSetExpType::GSE_STRING & pFld->GetSubType() )
            {
                // set the new value in the hash table
                // is the formula a field?
                SwSetExpField* pSFld = const_cast<SwSetExpField*>(static_cast<const SwSetExpField*>(pFld));
                OUString aNew = LookString( ppHashTbl, rTblSize, pSFld->GetFormula() );

                if( aNew.isEmpty() )               // nothing found, then the formula is
                    aNew = pSFld->GetFormula(); // the new value

                // #i3141# - update expression of field as in method
                // <SwDoc::UpdateExpFlds(..)> for string/text fields
                pSFld->ChgExpStr( aNew );

                // look up the field's name
                aNew = static_cast<SwSetExpFieldType*>(pSFld->GetTyp())->GetSetRefName();
                // Entry present?
                sal_uInt16 nPos;
                SwHash* pFnd = Find( aNew, ppHashTbl, rTblSize, &nPos );
                if( pFnd )
                    // modify entry in the hash table
                    static_cast<_HashStr*>(pFnd)->aSetStr = pSFld->GetExpStr();
                else
                    // insert the new entry
                    *(ppHashTbl + nPos ) = new _HashStr( aNew,
                            pSFld->GetExpStr(), static_cast<_HashStr*>(*(ppHashTbl + nPos)) );
            }
            break;
        case RES_DBFLD:
            {
                const OUString& rName = pFld->GetTyp()->GetName();

                // Insert entry in the hash table
                // Entry present?
                sal_uInt16 nPos;
                SwHash* pFnd = Find( rName, ppHashTbl, rTblSize, &nPos );
                OUString const value(pFld->ExpandField(m_rDoc.IsClipBoard()));
                if( pFnd )
                {
                    // modify entry in the hash table
                    static_cast<_HashStr*>(pFnd)->aSetStr = value;
                }
                else
                {
                    // insert the new entry
                    *(ppHashTbl + nPos ) = new _HashStr( rName,
                        value, static_cast<_HashStr *>(*(ppHashTbl + nPos)));
                }
            }
            break;
        }
    }
}


bool DocumentFieldsManager::IsNewFldLst() const
{
    return mbNewFldLst;
}

void DocumentFieldsManager::SetNewFldLst(bool bFlag)
{
    mbNewFldLst = bFlag;
}

void DocumentFieldsManager::InsDelFldInFldLst( bool bIns, const SwTxtFld& rFld )
{
    if( !mbNewFldLst || !m_rDoc.IsInDtor() )
        mpUpdtFlds->InsDelFldInFldLst( bIns, rFld );
}

SwField * DocumentFieldsManager::GetFieldAtPos(const SwPosition & rPos)
{
    SwTxtFld * const pAttr = GetTxtFldAtPos(rPos);

    return (pAttr) ? const_cast<SwField *>( pAttr->GetFmtFld().GetField() ) : 0;
}

SwTxtFld * DocumentFieldsManager::GetTxtFldAtPos(const SwPosition & rPos)
{
    SwTxtNode * const pNode = rPos.nNode.GetNode().GetTxtNode();

    return (pNode != NULL)
        ? pNode->GetFldTxtAttrAt( rPos.nContent.GetIndex(), true )
        : 0;
}

/// @note For simplicity assume that all field types have updatable contents so
///       optimization currently only available when no fields exist.
bool DocumentFieldsManager::containsUpdatableFields()
{
    for (sal_uInt16 i = 0; i < mpFldTypes->size(); ++i)
    {
        SwFieldType* pFldType = (*mpFldTypes)[i];
        SwIterator<SwFmtFld,SwFieldType> aIter(*pFldType);
        if (aIter.First())
            return true;
    }
    return false;
}

/// Remove all unreferenced field types of a document
void DocumentFieldsManager::GCFieldTypes()
{
    for( sal_uInt16 n = mpFldTypes->size(); n > INIT_FLDTYPES; )
        if( !(*mpFldTypes)[ --n ]->HasWriterListeners() )
            RemoveFldType( n );
}

void DocumentFieldsManager::_InitFieldTypes()       // is being called by the CTOR
{
    // Field types
    mpFldTypes->push_back( new SwDateTimeFieldType(&m_rDoc) );
    mpFldTypes->push_back( new SwChapterFieldType );
    mpFldTypes->push_back( new SwPageNumberFieldType );
    mpFldTypes->push_back( new SwAuthorFieldType );
    mpFldTypes->push_back( new SwFileNameFieldType(&m_rDoc) );
    mpFldTypes->push_back( new SwDBNameFieldType(&m_rDoc) );
    mpFldTypes->push_back( new SwGetExpFieldType(&m_rDoc) );
    mpFldTypes->push_back( new SwGetRefFieldType( &m_rDoc ) );
    mpFldTypes->push_back( new SwHiddenTxtFieldType );
    mpFldTypes->push_back( new SwPostItFieldType(&m_rDoc) );
    mpFldTypes->push_back( new SwDocStatFieldType(&m_rDoc) );
    mpFldTypes->push_back( new SwDocInfoFieldType(&m_rDoc) );
    mpFldTypes->push_back( new SwInputFieldType( &m_rDoc ) );
    mpFldTypes->push_back( new SwTblFieldType( &m_rDoc ) );
    mpFldTypes->push_back( new SwMacroFieldType(&m_rDoc) );
    mpFldTypes->push_back( new SwHiddenParaFieldType );
    mpFldTypes->push_back( new SwDBNextSetFieldType );
    mpFldTypes->push_back( new SwDBNumSetFieldType );
    mpFldTypes->push_back( new SwDBSetNumberFieldType );
    mpFldTypes->push_back( new SwTemplNameFieldType(&m_rDoc) );
    mpFldTypes->push_back( new SwTemplNameFieldType(&m_rDoc) );
    mpFldTypes->push_back( new SwExtUserFieldType );
    mpFldTypes->push_back( new SwRefPageSetFieldType );
    mpFldTypes->push_back( new SwRefPageGetFieldType( &m_rDoc ) );
    mpFldTypes->push_back( new SwJumpEditFieldType( &m_rDoc ) );
    mpFldTypes->push_back( new SwScriptFieldType( &m_rDoc ) );
    mpFldTypes->push_back( new SwCombinedCharFieldType );
    mpFldTypes->push_back( new SwDropDownFieldType );

    // Types have to be at the end!
    // We expect &m_rDoc in the InsertFldType!
    // MIB 14.04.95: In Sw3StringPool::Setup (sw3imp.cxx) and
    //               lcl_sw3io_InSetExpField (sw3field.cxx) now also
    mpFldTypes->push_back( new SwSetExpFieldType(&m_rDoc,
                SW_RESSTR(STR_POOLCOLL_LABEL_ABB), nsSwGetSetExpType::GSE_SEQ) );
    mpFldTypes->push_back( new SwSetExpFieldType(&m_rDoc,
                SW_RESSTR(STR_POOLCOLL_LABEL_TABLE), nsSwGetSetExpType::GSE_SEQ) );
    mpFldTypes->push_back( new SwSetExpFieldType(&m_rDoc,
                SW_RESSTR(STR_POOLCOLL_LABEL_FRAME), nsSwGetSetExpType::GSE_SEQ) );
    mpFldTypes->push_back( new SwSetExpFieldType(&m_rDoc,
                SW_RESSTR(STR_POOLCOLL_LABEL_DRAWING), nsSwGetSetExpType::GSE_SEQ) );

    assert( mpFldTypes->size() == INIT_FLDTYPES );
}

void DocumentFieldsManager::ClearFieldTypes()
{
    for(SwFldTypes::const_iterator it = mpFldTypes->begin() + INIT_FLDTYPES;
        it != mpFldTypes->end(); ++it)
        delete *it;
    mpFldTypes->erase( mpFldTypes->begin() + INIT_FLDTYPES, mpFldTypes->end() );
}

void DocumentFieldsManager::UpdateDBNumFlds( SwDBNameInfField& rDBFld, SwCalc& rCalc )
{
#if !HAVE_FEATURE_DBCONNECTIVITY
    (void) rDBFld;
    (void) rCalc;
#else
    SwDBManager* pMgr = m_rDoc.GetDBManager();

    sal_uInt16 nFldType = rDBFld.Which();

    bool bPar1 = rCalc.Calculate( rDBFld.GetPar1() ).GetBool();

    if( RES_DBNEXTSETFLD == nFldType )
        static_cast<SwDBNextSetField&>(rDBFld).SetCondValid( bPar1 );
    else
        static_cast<SwDBNumSetField&>(rDBFld).SetCondValid( bPar1 );

    if( !rDBFld.GetRealDBData().sDataSource.isEmpty() )
    {
        // Edit a certain database
        if( RES_DBNEXTSETFLD == nFldType )
            static_cast<SwDBNextSetField&>(rDBFld).Evaluate(&m_rDoc);
        else
            static_cast<SwDBNumSetField&>(rDBFld).Evaluate(&m_rDoc);

        SwDBData aTmpDBData( rDBFld.GetDBData(&m_rDoc) );

        if( pMgr->OpenDataSource( aTmpDBData.sDataSource, aTmpDBData.sCommand, -1, false ))
            rCalc.VarChange( lcl_GetDBVarName( m_rDoc, rDBFld),
                        pMgr->GetSelectedRecordId(aTmpDBData.sDataSource, aTmpDBData.sCommand, aTmpDBData.nCommandType) );
    }
    else
    {
        OSL_FAIL("TODO: what should happen with unnamed DBFields?");
    }
#endif
}

DocumentFieldsManager::~DocumentFieldsManager()
{
    delete mpUpdtFlds;
    delete mpFldTypes;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

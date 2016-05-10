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

    static OUString lcl_GetDBVarName( SwDoc& rDoc, SwDBNameInfField& rDBField )
    {
        SwDBData aDBData( rDBField.GetDBData( &rDoc ));
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

    static void lcl_CalcField( SwDoc& rDoc, SwCalc& rCalc, const _SetGetExpField& rSGEField,
                            SwDBManager* pMgr )
    {
        const SwTextField* pTextField = rSGEField.GetTextField();
        if( !pTextField )
            return ;

        const SwField* pField = pTextField->GetFormatField().GetField();
        const sal_uInt16 nFieldWhich = pField->GetTyp()->Which();

        if( RES_SETEXPFLD == nFieldWhich )
        {
            SwSbxValue aValue;
            if( nsSwGetSetExpType::GSE_EXPR & pField->GetSubType() )
                aValue.PutDouble( static_cast<const SwSetExpField*>(pField)->GetValue() );
            else
                // Extension to calculate with Strings
                aValue.PutString( static_cast<const SwSetExpField*>(pField)->GetExpStr() );

            // set the new value in Calculator
            rCalc.VarChange( pField->GetTyp()->GetName(), aValue );
        }
        else if( pMgr )
        {
    #if !HAVE_FEATURE_DBCONNECTIVITY
            (void) rDoc;
    #else
            switch( nFieldWhich )
            {
            case RES_DBNUMSETFLD:
                {
                    SwDBNumSetField* pDBField = const_cast<SwDBNumSetField*>(static_cast<const SwDBNumSetField*>(pField));

                    SwDBData aDBData(pDBField->GetDBData(&rDoc));

                    if( pDBField->IsCondValid() &&
                        pMgr->OpenDataSource( aDBData.sDataSource, aDBData.sCommand ))
                        rCalc.VarChange( lcl_GetDBVarName( rDoc, *pDBField),
                                        pDBField->GetFormat() );
                }
                break;
            case RES_DBNEXTSETFLD:
                {
                    SwDBNextSetField* pDBField = const_cast<SwDBNextSetField*>(static_cast<const SwDBNextSetField*>(pField));
                    SwDBData aDBData(pDBField->GetDBData(&rDoc));
                    if( !pDBField->IsCondValid() ||
                        !pMgr->OpenDataSource( aDBData.sDataSource, aDBData.sCommand ))
                        break;

                    OUString sDBNumNm(lcl_GetDBVarName( rDoc, *pDBField));
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
                                                                  mbNewFieldLst(true),
                                                                  mpUpdateFields( new SwDocUpdateField( &m_rDoc ) ),
                                                                  mpFieldTypes( new SwFieldTypes() ),
                                                                  mnLockExpField( 0 )
{
}

const SwFieldTypes* DocumentFieldsManager::GetFieldTypes() const
{
    return mpFieldTypes;
}

/** Insert field types
 *
 * @param rFieldTyp ???
 * @return Always returns a pointer to the type, if it's new or already added.
 */
SwFieldType* DocumentFieldsManager::InsertFieldType(const SwFieldType &rFieldTyp)
{
    const SwFieldTypes::size_type nSize = mpFieldTypes->size();
    const sal_uInt16 nFieldWhich = rFieldTyp.Which();

    SwFieldTypes::size_type i = INIT_FLDTYPES;

    switch( nFieldWhich )
    {
    case RES_SETEXPFLD:
            //JP 29.01.96: SequenceFields start at INIT_FLDTYPES - 3!!
            //             Or we get doubble number circles!!
            //MIB 14.03.95: From now on also the SW3-Reader relies on &m_rDoc, when
            //constructing string pools and when reading SetExp fields
            if( nsSwGetSetExpType::GSE_SEQ & static_cast<const SwSetExpFieldType&>(rFieldTyp).GetType() )
                i -= INIT_SEQ_FLDTYPES;
            SAL_FALLTHROUGH;
    case RES_DBFLD:
    case RES_USERFLD:
    case RES_DDEFLD:
        {
            const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
            OUString sFieldNm( rFieldTyp.GetName() );
            for( ; i < nSize; ++i )
                if( nFieldWhich == (*mpFieldTypes)[i]->Which() &&
                    rSCmp.isEqual( sFieldNm, (*mpFieldTypes)[i]->GetName() ))
                        return (*mpFieldTypes)[i];
        }
        break;

    case RES_AUTHORITY:
        for( ; i < nSize; ++i )
            if( nFieldWhich == (*mpFieldTypes)[i]->Which() )
                return (*mpFieldTypes)[i];
        break;

    default:
        for( i = 0; i < nSize; ++i )
            if( nFieldWhich == (*mpFieldTypes)[i]->Which() )
                return (*mpFieldTypes)[i];
    }

    SwFieldType* pNew = rFieldTyp.Copy();
    switch( nFieldWhich )
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
        mpUpdateFields->InsertFieldType( *pNew );
        break;
    case RES_AUTHORITY :
        static_cast<SwAuthorityFieldType*>(pNew)->SetDoc( &m_rDoc );
        break;
    }

    mpFieldTypes->insert( mpFieldTypes->begin() + nSize, pNew );
    m_rDoc.getIDocumentState().SetModified();

    return (*mpFieldTypes)[ nSize ];
}

/// @returns the field type of the Doc
SwFieldType *DocumentFieldsManager::GetSysFieldType( const sal_uInt16 eWhich ) const
{
    for( SwFieldTypes::size_type i = 0; i < INIT_FLDTYPES; ++i )
        if( eWhich == (*mpFieldTypes)[i]->Which() )
            return (*mpFieldTypes)[i];
    return nullptr;
}

/// Find first type with ResId and name
SwFieldType* DocumentFieldsManager::GetFieldType(
    sal_uInt16 nResId,
    const OUString& rName,
    bool bDbFieldMatching // used in some UNO calls for RES_DBFLD to use different string matching code #i51815#
    ) const
{
    const SwFieldTypes::size_type nSize = mpFieldTypes->size();
    SwFieldTypes::size_type i {0};
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

    SwFieldType* pRet = nullptr;
    for( ; i < nSize; ++i )
    {
        SwFieldType* pFieldType = (*mpFieldTypes)[i];

        OUString aFieldName( pFieldType->GetName() );
        if (bDbFieldMatching && nResId == RES_DBFLD)    // #i51815#
            aFieldName = aFieldName.replace(DB_DELIM, '.');

        if( nResId == pFieldType->Which() &&
            rSCmp.isEqual( rName, aFieldName ))
        {
            pRet = pFieldType;
            break;
        }
    }
    return pRet;
}

/// Remove field type
void DocumentFieldsManager::RemoveFieldType(size_t nField)
{
    OSL_ENSURE( INIT_FLDTYPES <= nField,  "don't remove InitFields" );
    /*
     * Dependent fields present -> ErrRaise
     */
    if(nField < mpFieldTypes->size())
    {
        SwFieldType* pTmp = (*mpFieldTypes)[nField];

        // JP 29.07.96: Optionally prepare FieldList for Calculator
        sal_uInt16 nWhich = pTmp->Which();
        switch( nWhich )
        {
        case RES_SETEXPFLD:
        case RES_USERFLD:
            mpUpdateFields->RemoveFieldType( *pTmp );
            SAL_FALLTHROUGH;
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
        mpFieldTypes->erase( mpFieldTypes->begin() + nField );
        m_rDoc.getIDocumentState().SetModified();
    }
}

// All have to be re-evaluated.
void DocumentFieldsManager::UpdateFields( SfxPoolItem *pNewHt, bool bCloseDB )
{
    // Call Modify() for every field type,
    // dependent SwTextField get notified ...

    for( auto pFieldType : *mpFieldTypes )
    {
        switch( pFieldType->Which() )
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
                pFieldType->ModifyNotification( nullptr, &aUpdateDDE );
            }
            else
                pFieldType->ModifyNotification( nullptr, pNewHt );
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
            pFieldType->ModifyNotification ( nullptr, pNewHt );
        }
    }

    if( !IsExpFieldsLocked() )
        UpdateExpFields( nullptr, false );      // update expression fields

    // Tables
    UpdateTableFields(pNewHt);

    // References
    UpdateRefFields(pNewHt);
    if( bCloseDB )
    {
#if HAVE_FEATURE_DBCONNECTIVITY
        m_rDoc.GetDBManager()->CloseAll();
#endif
    }
    // Only evaluate on full update
    m_rDoc.getIDocumentState().SetModified();
}

void DocumentFieldsManager::InsDeletedFieldType( SwFieldType& rFieldTyp )
{
    // The FieldType was marked as deleted and removed from the array.
    // One has to look &m_rDoc up again, now.
    // - If it's not present, it can be re-inserted.
    // - If the same type is found, the deleted one has to be renamed.

    const SwFieldTypes::size_type nSize = mpFieldTypes->size();
    const sal_uInt16 nFieldWhich = rFieldTyp.Which();

    OSL_ENSURE( RES_SETEXPFLD == nFieldWhich ||
            RES_USERFLD == nFieldWhich ||
            RES_DDEFLD == nFieldWhich, "Wrong FieldType" );

    const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
    const OUString& rFieldNm = rFieldTyp.GetName();

    for( SwFieldTypes::size_type i = INIT_FLDTYPES; i < nSize; ++i )
    {
        SwFieldType* pFnd;
        if( nFieldWhich == (pFnd = (*mpFieldTypes)[i])->Which() &&
            rSCmp.isEqual( rFieldNm, pFnd->GetName() ) )
        {
            // find new name
            SwFieldTypes::size_type nNum = 1;
            do {
                OUString sSrch = rFieldNm + OUString::number( nNum );
                for( i = INIT_FLDTYPES; i < nSize; ++i )
                    if( nFieldWhich == (pFnd = (*mpFieldTypes)[i])->Which() &&
                        rSCmp.isEqual( sSrch, pFnd->GetName() ) )
                        break;

                if( i >= nSize )        // not found
                {
                    const_cast<OUString&>(rFieldNm) = sSrch;
                    break;      // exit while loop
                }
                ++nNum;
            } while( true );
            break;
        }
    }

    // not found, so insert and delete flag
    mpFieldTypes->insert( mpFieldTypes->begin() + nSize, &rFieldTyp );
    switch( nFieldWhich )
    {
    case RES_SETEXPFLD:
        static_cast<SwSetExpFieldType&>(rFieldTyp).SetDeleted( false );
        break;
    case RES_USERFLD:
        static_cast<SwUserFieldType&>(rFieldTyp).SetDeleted( false );
        break;
    case RES_DDEFLD:
        static_cast<SwDDEFieldType&>(rFieldTyp).SetDeleted( false );
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

bool DocumentFieldsManager::UpdateField(SwTextField * pDstTextField, SwField & rSrcField,
                      SwMsgPoolItem * pMsgHint,
                      bool bUpdateFields)
{
    OSL_ENSURE(pDstTextField, "no field to update!");

    bool bTableSelBreak = false;

    SwFormatField * pDstFormatField = const_cast<SwFormatField*>(&pDstTextField->GetFormatField());
    SwField * pDstField = pDstFormatField->GetField();
    sal_uInt16 nFieldWhich = rSrcField.GetTyp()->Which();
    SwNodeIndex aTableNdIdx(pDstTextField->GetTextNode());

    if (pDstField->GetTyp()->Which() ==
        rSrcField.GetTyp()->Which())
    {
        if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
        {
            SwPosition aPosition( pDstTextField->GetTextNode() );
            aPosition.nContent = pDstTextField->GetStart();

            SwUndo *const pUndo( new SwUndoFieldFromDoc( aPosition, *pDstField, rSrcField, pMsgHint, bUpdateFields) );
            m_rDoc.GetIDocumentUndoRedo().AppendUndo(pUndo);
        }

        SwField * pNewField = rSrcField.CopyField();
        pDstFormatField->SetField(pNewField);

        switch( nFieldWhich )
        {
        case RES_SETEXPFLD:
        case RES_GETEXPFLD:
        case RES_HIDDENTXTFLD:
        case RES_HIDDENPARAFLD:
            UpdateExpFields( pDstTextField, true );
            break;

        case RES_TABLEFLD:
            {
                const SwTableNode* pTableNd =
                    m_rDoc.IsIdxInTable(aTableNdIdx);
                if( pTableNd )
                {
                    SwTableFormulaUpdate aTableUpdate( &pTableNd->
                                                 GetTable() );
                    if (bUpdateFields)
                        UpdateTableFields( &aTableUpdate );
                    else
                        pNewField->GetTyp()->ModifyNotification(nullptr, &aTableUpdate);

                    if (! bUpdateFields)
                        bTableSelBreak = true;
                }
            }
            break;

        case RES_MACROFLD:
            if( bUpdateFields && pDstTextField->GetpTextNode() )
                (pDstTextField->GetpTextNode())->
                    ModifyNotification( nullptr, pDstFormatField );
            break;

        case RES_DBNAMEFLD:
        case RES_DBNEXTSETFLD:
        case RES_DBNUMSETFLD:
        case RES_DBSETNUMBERFLD:
            m_rDoc.ChgDBData(static_cast<SwDBNameInfField*>( pNewField)->GetRealDBData());
            pNewField->GetTyp()->UpdateFields();

            break;

        case RES_DBFLD:
#if HAVE_FEATURE_DBCONNECTIVITY
            {
                // JP 10.02.96: call ChgValue, so that the style change sets the
                // ContentString correctly
                SwDBField* pDBField = static_cast<SwDBField*>(pNewField);
                if (pDBField->IsInitialized())
                    pDBField->ChgValue( pDBField->GetValue(), true );

                pDBField->ClearInitialized();
                pDBField->InitContent();
            }
#endif
            SAL_FALLTHROUGH;

        default:
            pDstFormatField->ModifyNotification( nullptr, pMsgHint );
        }

        // The fields we can calculate here are being triggered for an update
        // here explicitly.
        if( nFieldWhich == RES_USERFLD )
            UpdateUsrFields();
    }

    return bTableSelBreak;
}

/// Update reference and table fields
void DocumentFieldsManager::UpdateRefFields( SfxPoolItem* pHt )
{
    for( auto pFieldType : *mpFieldTypes )
        if( RES_GETREFFLD == pFieldType->Which() )
            pFieldType->ModifyNotification( nullptr, pHt );
}

void DocumentFieldsManager::UpdateTableFields( SfxPoolItem* pHt )
{
    OSL_ENSURE( !pHt || RES_TABLEFML_UPDATE  == pHt->Which(),
            "What MessageItem is &m_rDoc?" );

    SwFieldType* pFieldType(nullptr);

    for (auto pFieldTypeTmp : *mpFieldTypes)
    {
        if( RES_TABLEFLD == pFieldTypeTmp->Which() )
        {
            SwTableFormulaUpdate* pUpdateField = nullptr;
            if( pHt && RES_TABLEFML_UPDATE == pHt->Which() )
                pUpdateField = static_cast<SwTableFormulaUpdate*>(pHt);

            SwIterator<SwFormatField,SwFieldType> aIter( *pFieldTypeTmp );
            for( SwFormatField* pFormatField = aIter.First(); pFormatField; pFormatField = aIter.Next() )
            {
                if( pFormatField->GetTextField() )
                {
                    SwTableField* pField = static_cast<SwTableField*>(pFormatField->GetField());

                    if( pUpdateField )
                    {
                        // table where &m_rDoc field is located
                        const SwTableNode* pTableNd;
                        const SwTextNode& rTextNd = pFormatField->GetTextField()->GetTextNode();
                        if( !rTextNd.GetNodes().IsDocNodes() ||
                            nullptr == ( pTableNd = rTextNd.FindTableNode() ) )
                            continue;

                        switch( pUpdateField->m_eFlags )
                        {
                        case TBL_CALC:
                            // re-set the value flag
                            // JP 17.06.96: internal representation of all formulas
                            //              (reference to other table!!!)
                            if( nsSwExtendedSubType::SUB_CMD & pField->GetSubType() )
                                pField->PtrToBoxNm( pUpdateField->m_pTable );
                            else
                                pField->ChgValid( false );
                            break;
                        case TBL_BOXNAME:
                            // is &m_rDoc the wanted table?
                            if( &pTableNd->GetTable() == pUpdateField->m_pTable )
                                // to the external representation
                                pField->PtrToBoxNm( pUpdateField->m_pTable );
                            break;
                        case TBL_BOXPTR:
                            // to the internal representation
                            // JP 17.06.96: internal representation on all formulas
                            //              (reference to other table!!!)
                            pField->BoxNmToPtr( pUpdateField->m_pTable );
                            break;
                        case TBL_RELBOXNAME:
                            // is &m_rDoc the wanted table?
                            if( &pTableNd->GetTable() == pUpdateField->m_pTable )
                                // to the relative representation
                                pField->ToRelBoxNm( pUpdateField->m_pTable );
                            break;
                        default:
                            break;
                        }
                    }
                    else
                        // reset the value flag for all
                        pField->ChgValid( false );
                }
            }
            pFieldType = pFieldTypeTmp;
            break;
        }
    }

    // process all table box formulas
    const SfxPoolItem* pItem;
    sal_uInt32 nMaxItems = m_rDoc.GetAttrPool().GetItemCount2( RES_BOXATR_FORMULA );
    for (sal_uInt32 i = 0; i < nMaxItems; ++i)
    {
        if( nullptr != (pItem = m_rDoc.GetAttrPool().GetItem2( RES_BOXATR_FORMULA, i ) ) &&
            static_cast<const SwTableBoxFormula*>(pItem)->GetDefinedIn() )
        {
            const_cast<SwTableBoxFormula*>(static_cast<const SwTableBoxFormula*>(pItem))->ChangeState( pHt );
        }
    }

    // all fields/boxes are now invalid, so we can start to calculate
    if( pHt && ( RES_TABLEFML_UPDATE != pHt->Which() ||
                TBL_CALC != static_cast<SwTableFormulaUpdate*>(pHt)->m_eFlags ))
        return ;

    SwCalc* pCalc = nullptr;

    if( pFieldType )
    {
        SwIterator<SwFormatField,SwFieldType> aIter( *pFieldType );
        for( SwFormatField* pFormatField = aIter.Last(); pFormatField; pFormatField = aIter.Previous() )
        {
                // start calculation at the end
                // new fields are inserted at the beginning of the modify chain
                // that gives faster calculation on import
                // mba: do we really need &m_rDoc "optimization"? Is it still valid?
                SwTableField* pField;
                if( !pFormatField->GetTextField() || (nsSwExtendedSubType::SUB_CMD &
                    (pField = static_cast<SwTableField*>(pFormatField->GetField()))->GetSubType() ))
                    continue;

                // needs to be recalculated
                if( !pField->IsValid() )
                {
                    // table where &m_rDoc field is located
                    const SwTextNode& rTextNd = pFormatField->GetTextField()->GetTextNode();
                    if( !rTextNd.GetNodes().IsDocNodes() )
                        continue;
                    const SwTableNode* pTableNd = rTextNd.FindTableNode();
                    if( !pTableNd )
                        continue;

                    // if &m_rDoc field is not in the to-be-updated table, skip it
                    if( pHt && &pTableNd->GetTable() !=
                                            static_cast<SwTableFormulaUpdate*>(pHt)->m_pTable )
                        continue;

                    if( !pCalc )
                        pCalc = new SwCalc( m_rDoc );

                    // get the values of all SetExpression fields that are valid
                    // until the table
                    SwFrame* pFrame = nullptr;
                    if( pTableNd->GetIndex() < m_rDoc.GetNodes().GetEndOfExtras().GetIndex() )
                    {
                        // is in the special section, that's expensive!
                        Point aPt;      // return the first frame of the layout - Tab.Headline!!
                        pFrame = rTextNd.getLayoutFrame( m_rDoc.getIDocumentLayoutAccess().GetCurrentLayout(), &aPt );
                        if( pFrame )
                        {
                            SwPosition aPos( *pTableNd );
                            if( GetBodyTextNode( m_rDoc, aPos, *pFrame ) )
                                FieldsToCalc( *pCalc, _SetGetExpField(
                                    aPos.nNode, pFormatField->GetTextField(),
                                    &aPos.nContent ));
                            else
                                pFrame = nullptr;
                        }
                    }
                    if( !pFrame )
                    {
                        // create index to determine the TextNode
                        SwNodeIndex aIdx( rTextNd );
                        FieldsToCalc( *pCalc,
                            _SetGetExpField( aIdx, pFormatField->GetTextField() ));
                    }

                    SwTableCalcPara aPara( *pCalc, pTableNd->GetTable() );
                    pField->CalcField( aPara );
                    if( aPara.IsStackOverflow() )
                    {
                        bool const bResult = aPara.CalcWithStackOverflow();
                        if (bResult)
                        {
                            pField->CalcField( aPara );
                        }
                        OSL_ENSURE(bResult,
                                "the chained formula could no be calculated");
                    }
                    pCalc->SetCalcError( CALC_NOERR );
                }
                pFormatField->ModifyNotification( nullptr, pHt );
        }
    }

    // calculate the formula at the boxes
    for (sal_uInt32 i = 0; i < nMaxItems; ++i )
    {
        if( nullptr != (pItem = m_rDoc.GetAttrPool().GetItem2( RES_BOXATR_FORMULA, i ) ) &&
            static_cast<const SwTableBoxFormula*>(pItem)->GetDefinedIn() &&
            !static_cast<const SwTableBoxFormula*>(pItem)->IsValid() )
        {
            SwTableBoxFormula* pFormula = const_cast<SwTableBoxFormula*>(static_cast<const SwTableBoxFormula*>(pItem));
            SwTableBox* pBox = pFormula->GetTableBox();
            if( pBox && pBox->GetSttNd() &&
                pBox->GetSttNd()->GetNodes().IsDocNodes() )
            {
                const SwTableNode* pTableNd = pBox->GetSttNd()->FindTableNode();
                if( !pHt || &pTableNd->GetTable() ==
                                            static_cast<SwTableFormulaUpdate*>(pHt)->m_pTable )
                {
                    double nValue;
                    if( !pCalc )
                        pCalc = new SwCalc( m_rDoc );

                    // get the values of all SetExpression fields that are valid
                    // until the table
                    SwFrame* pFrame = nullptr;
                    if( pTableNd->GetIndex() < m_rDoc.GetNodes().GetEndOfExtras().GetIndex() )
                    {
                        // is in the special section, that's expensive!
                        Point aPt;      // return the first frame of the layout - Tab.Headline!!
                        SwNodeIndex aCNdIdx( *pTableNd, +2 );
                        SwContentNode* pCNd = aCNdIdx.GetNode().GetContentNode();
                        if( !pCNd )
                            pCNd = m_rDoc.GetNodes().GoNext( &aCNdIdx );

                        if( pCNd && nullptr != (pFrame = pCNd->getLayoutFrame( m_rDoc.getIDocumentLayoutAccess().GetCurrentLayout(), &aPt )) )
                        {
                            SwPosition aPos( *pCNd );
                            if( GetBodyTextNode( m_rDoc, aPos, *pFrame ) )
                                FieldsToCalc( *pCalc, _SetGetExpField( aPos.nNode ));
                            else
                                pFrame = nullptr;
                        }
                    }
                    if( !pFrame )
                    {
                        // create index to determine the TextNode
                        SwNodeIndex aIdx( *pTableNd );
                        FieldsToCalc( *pCalc, _SetGetExpField( aIdx ));
                    }

                    SwTableCalcPara aPara( *pCalc, pTableNd->GetTable() );
                    pFormula->Calc( aPara, nValue );

                    if( aPara.IsStackOverflow() )
                    {
                        bool const bResult = aPara.CalcWithStackOverflow();
                        if (bResult)
                        {
                            pFormula->Calc( aPara, nValue );
                        }
                        OSL_ENSURE(bResult,
                                "the chained formula could no be calculated");
                    }

                    SwFrameFormat* pFormat = pBox->ClaimFrameFormat();
                    SfxItemSet aTmp( m_rDoc.GetAttrPool(),
                                    RES_BOXATR_BEGIN,RES_BOXATR_END-1 );

                    if( pCalc->IsCalcError() )
                        nValue = DBL_MAX;
                    aTmp.Put( SwTableBoxValue( nValue ));
                    if( SfxItemState::SET != pFormat->GetItemState( RES_BOXATR_FORMAT ))
                        aTmp.Put( SwTableBoxNumFormat( 0 ));
                    pFormat->SetFormatAttr( aTmp );

                    pCalc->SetCalcError( CALC_NOERR );
                }
            }
        }
    }

    delete pCalc;
}

void DocumentFieldsManager::UpdateExpFields( SwTextField* pUpdateField, bool bUpdRefFields )
{
    if( IsExpFieldsLocked() || m_rDoc.IsInReading() )
        return;

    bool bOldInUpdateFields = mpUpdateFields->IsInUpdateFields();
    mpUpdateFields->SetInUpdateFields( true );

    mpUpdateFields->MakeFieldList( m_rDoc, true, GETFLD_ALL );
    mbNewFieldLst = false;

    if( mpUpdateFields->GetSortLst()->empty() )
    {
        if( bUpdRefFields )
            UpdateRefFields(nullptr);

        mpUpdateFields->SetInUpdateFields( bOldInUpdateFields );
        mpUpdateFields->SetFieldsDirty( false );
        return ;
    }

    sal_uInt16 nWhich;

    // Hash table for all string replacements is filled on-the-fly.
    // Try to fabricate an uneven number.
    const SwFieldTypes::size_type nHashSize {(( mpFieldTypes->size() / 7 ) + 1 ) * 7};
    const sal_uInt16 nStrFormatCnt = static_cast<sal_uInt16>(nHashSize);
    OSL_ENSURE( nStrFormatCnt == nHashSize, "Downcasting to sal_uInt16 lost information!" );
    SwHash** pHashStrTable = new SwHash*[ nStrFormatCnt ];
    memset( pHashStrTable, 0, sizeof( _HashStr* ) * nStrFormatCnt );

    {
        const SwFieldType* pFieldType;
        // process separately:
        for( auto n = mpFieldTypes->size(); n; )
            switch( ( pFieldType = (*mpFieldTypes)[ --n ] )->Which() )
            {
            case RES_USERFLD:
                {
                    // Entry present?
                    sal_uInt16 nPos;
                    const OUString& rNm = pFieldType->GetName();
                    OUString sExpand(const_cast<SwUserFieldType*>(static_cast<const SwUserFieldType*>(pFieldType))->Expand(nsSwGetSetExpType::GSE_STRING, 0, 0));
                    SwHash* pFnd = Find( rNm, pHashStrTable, nStrFormatCnt, &nPos );
                    if( pFnd )
                        // modify entry in the hash table
                        static_cast<_HashStr*>(pFnd)->aSetStr = sExpand;
                    else
                        // insert the new entry
                        *(pHashStrTable + nPos ) = new _HashStr( rNm, sExpand,
                                                static_cast<_HashStr*>(*(pHashStrTable + nPos)) );
                }
                break;
            case RES_SETEXPFLD:
                const_cast<SwSetExpFieldType*>(static_cast<const SwSetExpFieldType*>(pFieldType))->SetOutlineChgNd( nullptr );
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
    for( _SetGetExpFields::const_iterator it = mpUpdateFields->GetSortLst()->begin(); it != mpUpdateFields->GetSortLst()->end(); ++it )
    {
        SwSection* pSect = const_cast<SwSection*>((*it)->GetSection());
        if ( pSect && !pSect->IsCondHidden())
            nShownSections++;
    }

    OUString aNew;
    for( _SetGetExpFields::const_iterator it = mpUpdateFields->GetSortLst()->begin(); it != mpUpdateFields->GetSortLst()->end(); ++it )
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

        SwTextField* pTextField = const_cast<SwTextField*>((*it)->GetTextField());
        if( !pTextField )
        {
            OSL_ENSURE( false, "what's wrong now'" );
            continue;
        }

        SwFormatField* pFormatField = const_cast<SwFormatField*>(&pTextField->GetFormatField());
        const SwField* pField = pFormatField->GetField();

        switch( nWhich = pField->GetTyp()->Which() )
        {
        case RES_HIDDENTXTFLD:
        {
            SwHiddenTextField* pHField = const_cast<SwHiddenTextField*>(static_cast<const SwHiddenTextField*>(pField));
            SwSbxValue aValue = aCalc.Calculate( pHField->GetPar1() );
            bool bValue = !aValue.GetBool();
            if(!aValue.IsVoidValue())
            {
                pHField->SetValue( bValue );
                // evaluate field
                pHField->Evaluate(&m_rDoc);
            }
        }
        break;
        case RES_HIDDENPARAFLD:
        {
            SwHiddenParaField* pHPField = const_cast<SwHiddenParaField*>(static_cast<const SwHiddenParaField*>(pField));
            SwSbxValue aValue = aCalc.Calculate( pHPField->GetPar1() );
            bool bValue = aValue.GetBool();
            if(!aValue.IsVoidValue())
                pHPField->SetHidden( bValue );
        }
        break;
        case RES_DBSETNUMBERFLD:
#if HAVE_FEATURE_DBCONNECTIVITY
        {
            const_cast<SwDBSetNumberField*>(static_cast<const SwDBSetNumberField*>(pField))->Evaluate(&m_rDoc);
            aCalc.VarChange( sDBNumNm, static_cast<const SwDBSetNumberField*>(pField)->GetSetNumber());
            pField->ExpandField(m_rDoc.IsClipBoard());
        }
#endif
        break;
        case RES_DBNEXTSETFLD:
        case RES_DBNUMSETFLD:
#if HAVE_FEATURE_DBCONNECTIVITY
        {
            UpdateDBNumFields( *const_cast<SwDBNameInfField*>(static_cast<const SwDBNameInfField*>(pField)), aCalc );
            if( bCanFill )
                bCanFill = pMgr->FillCalcWithMergeData( m_rDoc.GetNumberFormatter(), nLang, true, aCalc );
        }
#endif
        break;
        case RES_DBFLD:
        {
#if HAVE_FEATURE_DBCONNECTIVITY
            // evaluate field
            const_cast<SwDBField*>(static_cast<const SwDBField*>(pField))->Evaluate();

            SwDBData aTmpDBData(static_cast<const SwDBField*>(pField)->GetDBData());

            if( pMgr->IsDataSourceOpen(aTmpDBData.sDataSource, aTmpDBData.sCommand, false))
                aCalc.VarChange( sDBNumNm, pMgr->GetSelectedRecordId(aTmpDBData.sDataSource, aTmpDBData.sCommand, aTmpDBData.nCommandType));

            const OUString& rName = pField->GetTyp()->GetName();

            // Add entry to hash table
            // Entry present?
            sal_uInt16 nPos;
            SwHash* pFnd = Find( rName, pHashStrTable, nStrFormatCnt, &nPos );
            OUString const value(pField->ExpandField(m_rDoc.IsClipBoard()));
            if( pFnd )
            {
                // Modify entry in the hash table
                static_cast<_HashStr*>(pFnd)->aSetStr = value;
            }
            else
            {
                // insert new entry
                *(pHashStrTable + nPos ) = new _HashStr( rName,
                    value, static_cast<_HashStr *>(*(pHashStrTable + nPos)));
            }
#endif
        }
        break;
        case RES_GETEXPFLD:
        case RES_SETEXPFLD:
        {
            if( nsSwGetSetExpType::GSE_STRING & pField->GetSubType() )        // replace String
            {
                if( RES_GETEXPFLD == nWhich )
                {
                    SwGetExpField* pGField = const_cast<SwGetExpField*>(static_cast<const SwGetExpField*>(pField));

                    if( (!pUpdateField || pUpdateField == pTextField )
                        && pGField->IsInBodyText() )
                    {
                        aNew = LookString( pHashStrTable, nStrFormatCnt,
                                    pGField->GetFormula() );
                        pGField->ChgExpStr( aNew );
                    }
                }
                else
                {
                    SwSetExpField* pSField = const_cast<SwSetExpField*>(static_cast<const SwSetExpField*>(pField));
                    // is the "formula" a field?
                    aNew = LookString( pHashStrTable, nStrFormatCnt,
                                pSField->GetFormula() );

                    if( aNew.isEmpty() )               // nothing found then the formula is the new value
                        aNew = pSField->GetFormula();

                    // only update one field
                    if( !pUpdateField || pUpdateField == pTextField )
                        pSField->ChgExpStr( aNew );

                    // lookup the field's name
                    aNew = static_cast<SwSetExpFieldType*>(pSField->GetTyp())->GetSetRefName();
                    // Entry present?
                    sal_uInt16 nPos;
                    SwHash* pFnd = Find( aNew, pHashStrTable, nStrFormatCnt, &nPos );
                    if( pFnd )
                        // Modify entry in the hash table
                        static_cast<_HashStr*>(pFnd)->aSetStr = pSField->GetExpStr();
                    else
                        // insert new entry
                        *(pHashStrTable + nPos ) = pFnd = new _HashStr( aNew,
                                        pSField->GetExpStr(),
                                        static_cast<_HashStr*>(*(pHashStrTable + nPos) ));

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
                    SwGetExpField* pGField = const_cast<SwGetExpField*>(static_cast<const SwGetExpField*>(pField));

                    if( (!pUpdateField || pUpdateField == pTextField )
                        && pGField->IsInBodyText() )
                    {
                        SwSbxValue aValue = aCalc.Calculate(
                                        pGField->GetFormula());
                        if(!aValue.IsVoidValue())
                            pGField->SetValue(aValue.GetDouble() );
                    }
                }
                else
                {
                    SwSetExpField* pSField = const_cast<SwSetExpField*>(static_cast<const SwSetExpField*>(pField));
                    SwSetExpFieldType* pSFieldTyp = static_cast<SwSetExpFieldType*>(pField->GetTyp());
                    aNew = pSFieldTyp->GetName();

                    SwNode* pSeqNd = nullptr;

                    if( pSField->IsSequenceField() )
                    {
                        const sal_uInt8 nLvl = pSFieldTyp->GetOutlineLvl();
                        if( MAXLEVEL > nLvl )
                        {
                            // test if the Number needs to be updated
                            pSeqNd = m_rDoc.GetNodes()[ (*it)->GetNode() ];

                            const SwTextNode* pOutlNd = pSeqNd->
                                    FindOutlineNodeOfLevel( nLvl );
                            if( pSFieldTyp->GetOutlineChgNd() != pOutlNd )
                            {
                                pSFieldTyp->SetOutlineChgNd( pOutlNd );
                                aCalc.VarChange( aNew, 0 );
                            }
                        }
                    }

                    aNew += "=";
                    aNew += pSField->GetFormula();

                    SwSbxValue aValue = aCalc.Calculate( aNew );
                    double nErg = aValue.GetDouble();
                    // only update one field
                    if( !aValue.IsVoidValue() && (!pUpdateField || pUpdateField == pTextField) )
                    {
                        pSField->SetValue( nErg );

                        if( pSeqNd )
                            pSFieldTyp->SetChapter( *pSField, *pSeqNd );
                    }
                }
            }
        }
        } // switch

        pFormatField->ModifyNotification( nullptr, nullptr );        // trigger formatting

        if( pUpdateField == pTextField )       // if only &m_rDoc one is updated
        {
            if( RES_GETEXPFLD == nWhich ||      // only GetField or
                RES_HIDDENTXTFLD == nWhich ||   // HiddenText?
                RES_HIDDENPARAFLD == nWhich)    // HiddenParaField?
                break;                          // quit
            pUpdateField = nullptr;                       // update all from here on
        }
    }

#if HAVE_FEATURE_DBCONNECTIVITY
    pMgr->CloseAll(false);
#endif
    // delete hash table
    ::DeleteHashTable( pHashStrTable, nStrFormatCnt );

    // update reference fields
    if( bUpdRefFields )
        UpdateRefFields(nullptr);

    mpUpdateFields->SetInUpdateFields( bOldInUpdateFields );
    mpUpdateFields->SetFieldsDirty( false );
}

/// Insert field type that was marked as deleted
void DocumentFieldsManager::UpdateUsrFields()
{
    SwCalc* pCalc = nullptr;
    for( SwFieldTypes::size_type i = INIT_FLDTYPES; i < mpFieldTypes->size(); ++i )
    {
        const SwFieldType* pFieldType;
        if( RES_USERFLD == ( pFieldType = (*mpFieldTypes)[i] )->Which() )
        {
            if( !pCalc )
                pCalc = new SwCalc( m_rDoc );
            const_cast<SwUserFieldType*>(static_cast<const SwUserFieldType*>(pFieldType))->GetValue( *pCalc );
        }
    }

    if( pCalc )
    {
        delete pCalc;
        m_rDoc.getIDocumentState().SetModified();
    }
}

void DocumentFieldsManager::UpdatePageFields( SfxPoolItem* pMsgHint )
{
    for( SwFieldTypes::size_type i = 0; i < INIT_FLDTYPES; ++i )
    {
        SwFieldType* pFieldType = (*mpFieldTypes)[ i ];
        switch( pFieldType->Which() )
        {
        case RES_PAGENUMBERFLD:
        case RES_CHAPTERFLD:
        case RES_GETEXPFLD:
        case RES_REFPAGEGETFLD:
            pFieldType->ModifyNotification( nullptr, pMsgHint );
            break;
        case RES_DOCSTATFLD:
            pFieldType->ModifyNotification( nullptr, nullptr );
            break;
        }
    }
    SetNewFieldLst(true);
}

void DocumentFieldsManager::LockExpFields()
{
    ++mnLockExpField;
}

void DocumentFieldsManager::UnlockExpFields()
{
    assert(mnLockExpField != 0);
    if( mnLockExpField )
        --mnLockExpField;
}

bool DocumentFieldsManager::IsExpFieldsLocked() const
{
    return 0 != mnLockExpField;
}

SwDocUpdateField& DocumentFieldsManager::GetUpdateFields() const
{
    return *mpUpdateFields;
}

bool DocumentFieldsManager::SetFieldsDirty( bool b, const SwNode* pChk, sal_uLong nLen )
{
    // See if the supplied nodes actually contain fields.
    // If they don't, the flag doesn't need to be changed.
    bool bFieldsFnd = false;
    if( b && pChk && !GetUpdateFields().IsFieldsDirty() && !m_rDoc.IsInDtor()
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
            const SwTextNode* pTNd = rNds[ nStt++ ]->GetTextNode();
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
                        const SwTextAttr* pAttr = pTNd->GetSwpHints().Get(n);
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
        bFieldsFnd = b;
    }
    GetUpdateFields().SetFieldsDirty( b );
    return bFieldsFnd;
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
        SwFieldType* pFieldType = GetSysFieldType( aTypes[ nStt ] );
        SwIterator<SwFormatField,SwFieldType> aIter( *pFieldType );
        for( SwFormatField* pFormatField = aIter.First(); pFormatField; pFormatField = aIter.Next() )
        {
            if( pFormatField && pFormatField->GetTextField() )
            {
                bool bChgd = false;
                switch( aTypes[ nStt ] )
                {
                case RES_DOCINFOFLD:
                    if( static_cast<SwDocInfoField*>(pFormatField->GetField())->IsFixed() )
                    {
                        bChgd = true;
                        SwDocInfoField* pDocInfField = static_cast<SwDocInfoField*>(pFormatField->GetField());
                        pDocInfField->SetExpansion( static_cast<SwDocInfoFieldType*>(
                                    pDocInfField->GetTyp())->Expand(
                                        pDocInfField->GetSubType(),
                                        pDocInfField->GetFormat(),
                                        pDocInfField->GetLanguage(),
                                        pDocInfField->GetName() ) );
                    }
                    break;

                case RES_AUTHORFLD:
                    if( static_cast<SwAuthorField*>(pFormatField->GetField())->IsFixed() )
                    {
                        bChgd = true;
                        SwAuthorField* pAuthorField = static_cast<SwAuthorField*>(pFormatField->GetField());
                        pAuthorField->SetExpansion( SwAuthorFieldType::Expand( pAuthorField->GetFormat() ) );
                    }
                    break;

                case RES_EXTUSERFLD:
                    if( static_cast<SwExtUserField*>(pFormatField->GetField())->IsFixed() )
                    {
                        bChgd = true;
                        SwExtUserField* pExtUserField = static_cast<SwExtUserField*>(pFormatField->GetField());
                        pExtUserField->SetExpansion( SwExtUserFieldType::Expand(
                                            pExtUserField->GetSubType(),
                                            pExtUserField->GetFormat()));
                    }
                    break;

                case RES_DATETIMEFLD:
                    if( static_cast<SwDateTimeField*>(pFormatField->GetField())->IsFixed() )
                    {
                        bChgd = true;
                        static_cast<SwDateTimeField*>(pFormatField->GetField())->SetDateTime(
                                                    DateTime(Date(nDate), tools::Time(nTime)) );
                    }
                    break;

                case RES_FILENAMEFLD:
                    if( static_cast<SwFileNameField*>(pFormatField->GetField())->IsFixed() )
                    {
                        bChgd = true;
                        SwFileNameField* pFileNameField =
                            static_cast<SwFileNameField*>(pFormatField->GetField());
                        pFileNameField->SetExpansion( static_cast<SwFileNameFieldType*>(
                                    pFileNameField->GetTyp())->Expand(
                                            pFileNameField->GetFormat() ) );
                    }
                    break;
                }

                // Trigger formatting
                if( bChgd )
                    pFormatField->ModifyNotification( nullptr, nullptr );
            }
        }
    }

    if( !bIsModified )
        m_rDoc.getIDocumentState().ResetModified();
}

void DocumentFieldsManager::FieldsToCalc( SwCalc& rCalc, const _SetGetExpField& rToThisField )
{
    // create the sorted list of all SetFields
    mpUpdateFields->MakeFieldList( m_rDoc, mbNewFieldLst, GETFLD_CALC );
    mbNewFieldLst = false;

#if !HAVE_FEATURE_DBCONNECTIVITY
    SwDBManager* pMgr = NULL;
#else
    SwDBManager* pMgr = m_rDoc.GetDBManager();
    pMgr->CloseAll(false);
#endif

    if( !mpUpdateFields->GetSortLst()->empty() )
    {
        _SetGetExpFields::const_iterator const itLast =
            mpUpdateFields->GetSortLst()->upper_bound(
                const_cast<_SetGetExpField*>(&rToThisField));
        for( _SetGetExpFields::const_iterator it = mpUpdateFields->GetSortLst()->begin(); it != itLast; ++it )
            lcl_CalcField( m_rDoc, rCalc, **it, pMgr );
    }
#if HAVE_FEATURE_DBCONNECTIVITY
    pMgr->CloseAll(false);
#endif
}

void DocumentFieldsManager::FieldsToCalc( SwCalc& rCalc, sal_uLong nLastNd, sal_uInt16 nLastCnt )
{
    // create the sorted list of all SetFields
    mpUpdateFields->MakeFieldList( m_rDoc, mbNewFieldLst, GETFLD_CALC );
    mbNewFieldLst = false;

#if !HAVE_FEATURE_DBCONNECTIVITY
    SwDBManager* pMgr = NULL;
#else
    SwDBManager* pMgr = m_rDoc.GetDBManager();
    pMgr->CloseAll(false);
#endif

    for( _SetGetExpFields::const_iterator it = mpUpdateFields->GetSortLst()->begin();
        it != mpUpdateFields->GetSortLst()->end() &&
        ( (*it)->GetNode() < nLastNd ||
          ( (*it)->GetNode() == nLastNd && (*it)->GetContent() <= nLastCnt )
        );
        ++it )
    {
        lcl_CalcField( m_rDoc, rCalc, **it, pMgr );
    }

#if HAVE_FEATURE_DBCONNECTIVITY
    pMgr->CloseAll(false);
#endif
}

void DocumentFieldsManager::FieldsToExpand( SwHash**& ppHashTable, sal_uInt16& rTableSize,
                            const _SetGetExpField& rToThisField )
{
    // create the sorted list of all SetFields
    mpUpdateFields->MakeFieldList( m_rDoc, mbNewFieldLst, GETFLD_EXPAND );
    mbNewFieldLst = false;

    // Hash table for all string replacements is filled on-the-fly.
    // Try to fabricate an uneven number.
    rTableSize = (( mpUpdateFields->GetSortLst()->size() / 7 ) + 1 ) * 7;
    ppHashTable = new SwHash*[ rTableSize ];
    memset( ppHashTable, 0, sizeof( _HashStr* ) * rTableSize );

    _SetGetExpFields::const_iterator const itLast =
        mpUpdateFields->GetSortLst()->upper_bound(
            const_cast<_SetGetExpField*>(&rToThisField));

    for( _SetGetExpFields::const_iterator it = mpUpdateFields->GetSortLst()->begin(); it != itLast; ++it )
    {
        const SwTextField* pTextField = (*it)->GetTextField();
        if( !pTextField )
            continue;

        const SwField* pField = pTextField->GetFormatField().GetField();
        switch( pField->GetTyp()->Which() )
        {
        case RES_SETEXPFLD:
            if( nsSwGetSetExpType::GSE_STRING & pField->GetSubType() )
            {
                // set the new value in the hash table
                // is the formula a field?
                SwSetExpField* pSField = const_cast<SwSetExpField*>(static_cast<const SwSetExpField*>(pField));
                OUString aNew = LookString( ppHashTable, rTableSize, pSField->GetFormula() );

                if( aNew.isEmpty() )               // nothing found, then the formula is
                    aNew = pSField->GetFormula(); // the new value

                // #i3141# - update expression of field as in method
                // <SwDoc::UpdateExpFields(..)> for string/text fields
                pSField->ChgExpStr( aNew );

                // look up the field's name
                aNew = static_cast<SwSetExpFieldType*>(pSField->GetTyp())->GetSetRefName();
                // Entry present?
                sal_uInt16 nPos;
                SwHash* pFnd = Find( aNew, ppHashTable, rTableSize, &nPos );
                if( pFnd )
                    // modify entry in the hash table
                    static_cast<_HashStr*>(pFnd)->aSetStr = pSField->GetExpStr();
                else
                    // insert the new entry
                    *(ppHashTable + nPos ) = new _HashStr( aNew,
                            pSField->GetExpStr(), static_cast<_HashStr*>(*(ppHashTable + nPos)) );
            }
            break;
        case RES_DBFLD:
            {
                const OUString& rName = pField->GetTyp()->GetName();

                // Insert entry in the hash table
                // Entry present?
                sal_uInt16 nPos;
                SwHash* pFnd = Find( rName, ppHashTable, rTableSize, &nPos );
                OUString const value(pField->ExpandField(m_rDoc.IsClipBoard()));
                if( pFnd )
                {
                    // modify entry in the hash table
                    static_cast<_HashStr*>(pFnd)->aSetStr = value;
                }
                else
                {
                    // insert the new entry
                    *(ppHashTable + nPos ) = new _HashStr( rName,
                        value, static_cast<_HashStr *>(*(ppHashTable + nPos)));
                }
            }
            break;
        }
    }
}


bool DocumentFieldsManager::IsNewFieldLst() const
{
    return mbNewFieldLst;
}

void DocumentFieldsManager::SetNewFieldLst(bool bFlag)
{
    mbNewFieldLst = bFlag;
}

void DocumentFieldsManager::InsDelFieldInFieldLst( bool bIns, const SwTextField& rField )
{
    if( !mbNewFieldLst || !m_rDoc.IsInDtor() )
        mpUpdateFields->InsDelFieldInFieldLst( bIns, rField );
}

SwField * DocumentFieldsManager::GetFieldAtPos(const SwPosition & rPos)
{
    SwTextField * const pAttr = GetTextFieldAtPos(rPos);

    return (pAttr) ? const_cast<SwField *>( pAttr->GetFormatField().GetField() ) : nullptr;
}

SwTextField * DocumentFieldsManager::GetTextFieldAtPos(const SwPosition & rPos)
{
    SwTextNode * const pNode = rPos.nNode.GetNode().GetTextNode();

    return (pNode != nullptr)
        ? pNode->GetFieldTextAttrAt( rPos.nContent.GetIndex(), true )
        : nullptr;
}

/// @note For simplicity assume that all field types have updatable contents so
///       optimization currently only available when no fields exist.
bool DocumentFieldsManager::containsUpdatableFields()
{
    for (auto pFieldType : *mpFieldTypes)
    {
        SwIterator<SwFormatField,SwFieldType> aIter(*pFieldType);
        if (aIter.First())
            return true;
    }
    return false;
}

/// Remove all unreferenced field types of a document
void DocumentFieldsManager::GCFieldTypes()
{
    for( auto n = mpFieldTypes->size(); n > INIT_FLDTYPES; )
        if( !(*mpFieldTypes)[ --n ]->HasWriterListeners() )
            RemoveFieldType( n );
}

void DocumentFieldsManager::_InitFieldTypes()       // is being called by the CTOR
{
    // Field types
    mpFieldTypes->push_back( new SwDateTimeFieldType(&m_rDoc) );
    mpFieldTypes->push_back( new SwChapterFieldType );
    mpFieldTypes->push_back( new SwPageNumberFieldType );
    mpFieldTypes->push_back( new SwAuthorFieldType );
    mpFieldTypes->push_back( new SwFileNameFieldType(&m_rDoc) );
    mpFieldTypes->push_back( new SwDBNameFieldType(&m_rDoc) );
    mpFieldTypes->push_back( new SwGetExpFieldType(&m_rDoc) );
    mpFieldTypes->push_back( new SwGetRefFieldType( &m_rDoc ) );
    mpFieldTypes->push_back( new SwHiddenTextFieldType );
    mpFieldTypes->push_back( new SwPostItFieldType(&m_rDoc) );
    mpFieldTypes->push_back( new SwDocStatFieldType(&m_rDoc) );
    mpFieldTypes->push_back( new SwDocInfoFieldType(&m_rDoc) );
    mpFieldTypes->push_back( new SwInputFieldType( &m_rDoc ) );
    mpFieldTypes->push_back( new SwTableFieldType( &m_rDoc ) );
    mpFieldTypes->push_back( new SwMacroFieldType(&m_rDoc) );
    mpFieldTypes->push_back( new SwHiddenParaFieldType );
    mpFieldTypes->push_back( new SwDBNextSetFieldType );
    mpFieldTypes->push_back( new SwDBNumSetFieldType );
    mpFieldTypes->push_back( new SwDBSetNumberFieldType );
    mpFieldTypes->push_back( new SwTemplNameFieldType(&m_rDoc) );
    mpFieldTypes->push_back( new SwTemplNameFieldType(&m_rDoc) );
    mpFieldTypes->push_back( new SwExtUserFieldType );
    mpFieldTypes->push_back( new SwRefPageSetFieldType );
    mpFieldTypes->push_back( new SwRefPageGetFieldType( &m_rDoc ) );
    mpFieldTypes->push_back( new SwJumpEditFieldType( &m_rDoc ) );
    mpFieldTypes->push_back( new SwScriptFieldType( &m_rDoc ) );
    mpFieldTypes->push_back( new SwCombinedCharFieldType );
    mpFieldTypes->push_back( new SwDropDownFieldType );

    // Types have to be at the end!
    // We expect &m_rDoc in the InsertFieldType!
    // MIB 14.04.95: In Sw3StringPool::Setup (sw3imp.cxx) and
    //               lcl_sw3io_InSetExpField (sw3field.cxx) now also
    mpFieldTypes->push_back( new SwSetExpFieldType(&m_rDoc,
                SW_RESSTR(STR_POOLCOLL_LABEL_ABB), nsSwGetSetExpType::GSE_SEQ) );
    mpFieldTypes->push_back( new SwSetExpFieldType(&m_rDoc,
                SW_RESSTR(STR_POOLCOLL_LABEL_TABLE), nsSwGetSetExpType::GSE_SEQ) );
    mpFieldTypes->push_back( new SwSetExpFieldType(&m_rDoc,
                SW_RESSTR(STR_POOLCOLL_LABEL_FRAME), nsSwGetSetExpType::GSE_SEQ) );
    mpFieldTypes->push_back( new SwSetExpFieldType(&m_rDoc,
                SW_RESSTR(STR_POOLCOLL_LABEL_DRAWING), nsSwGetSetExpType::GSE_SEQ) );

    assert( mpFieldTypes->size() == INIT_FLDTYPES );
}

void DocumentFieldsManager::ClearFieldTypes()
{
    for(SwFieldTypes::const_iterator it = mpFieldTypes->begin() + INIT_FLDTYPES;
        it != mpFieldTypes->end(); ++it)
        delete *it;
    mpFieldTypes->erase( mpFieldTypes->begin() + INIT_FLDTYPES, mpFieldTypes->end() );
}

void DocumentFieldsManager::UpdateDBNumFields( SwDBNameInfField& rDBField, SwCalc& rCalc )
{
#if !HAVE_FEATURE_DBCONNECTIVITY
    (void) rDBField;
    (void) rCalc;
#else
    SwDBManager* pMgr = m_rDoc.GetDBManager();

    sal_uInt16 nFieldType = rDBField.Which();

    bool bPar1 = rCalc.Calculate( rDBField.GetPar1() ).GetBool();

    if( RES_DBNEXTSETFLD == nFieldType )
        static_cast<SwDBNextSetField&>(rDBField).SetCondValid( bPar1 );
    else
        static_cast<SwDBNumSetField&>(rDBField).SetCondValid( bPar1 );

    if( !rDBField.GetRealDBData().sDataSource.isEmpty() )
    {
        // Edit a certain database
        if( RES_DBNEXTSETFLD == nFieldType )
            static_cast<SwDBNextSetField&>(rDBField).Evaluate(&m_rDoc);
        else
            static_cast<SwDBNumSetField&>(rDBField).Evaluate(&m_rDoc);

        SwDBData aTmpDBData( rDBField.GetDBData(&m_rDoc) );

        if( pMgr->OpenDataSource( aTmpDBData.sDataSource, aTmpDBData.sCommand ))
            rCalc.VarChange( lcl_GetDBVarName( m_rDoc, rDBField),
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
    delete mpUpdateFields;
    delete mpFieldTypes;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

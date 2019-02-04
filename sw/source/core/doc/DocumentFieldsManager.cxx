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
#include <DocumentFieldsManager.hxx>
#include <config_features.h>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <redline.hxx>
#include <rootfrm.hxx>
#include <dbmgr.hxx>
#include <chpfld.hxx>
#include <dbfld.hxx>
#include <reffld.hxx>
#include <flddropdown.hxx>
#include <strings.hrc>
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
#include <ndindex.hxx>
#include <pam.hxx>
#include <o3tl/deleter.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <com/sun/star/uno/Any.hxx>

using namespace ::com::sun::star::uno;

namespace sw
{
    bool IsFieldDeletedInModel(IDocumentRedlineAccess const& rIDRA,
            SwTextField const& rTextField)
    {
        SwRedlineTable::size_type tmp;
        SwPosition const pos(rTextField.GetTextNode(),
                rTextField.GetStart());
        SwRangeRedline const*const pRedline(rIDRA.GetRedline(pos, &tmp));
        return (pRedline
            && pRedline->GetType() == nsRedlineType_t::REDLINE_DELETE
            && *pRedline->GetPoint() != *pRedline->GetMark());
    }
}

namespace
{
    #if HAVE_FEATURE_DBCONNECTIVITY

    OUString lcl_GetDBVarName( SwDoc& rDoc, SwDBNameInfField& rDBField )
    {
        SwDBData aDBData( rDBField.GetDBData( &rDoc ));
        OUString sDBNumNm;
        SwDBData aDocData = rDoc.GetDBData();

        if( aDBData != aDocData )
        {
            sDBNumNm = aDBData.sDataSource + OUStringLiteral1(DB_DELIM)
                + aDBData.sCommand + OUStringLiteral1(DB_DELIM);
        }
        sDBNumNm += SwFieldType::GetTypeStr(TYP_DBSETNUMBERFLD);

        return sDBNumNm;
    }

    #endif

    bool IsFieldDeleted(IDocumentRedlineAccess const& rIDRA,
            SwRootFrame const& rLayout, SwTextField const& rTextField)
    {
        SwTextNode const& rNode(rTextField.GetTextNode());
        bool const isInBody(
            rNode.GetNodes().GetEndOfExtras().GetIndex() < rNode.GetIndex());
        if (!isInBody && nullptr == rNode.getLayoutFrame(&rLayout))
        {   // see SwDocUpdateField::GetBodyNode() - fields in hidden sections
            // don't have layout frames but must be updated, so use the same
            // check as there, but do it again because GetBodyNode() checks
            // for *any* layout...
            return true;
        }
        return sw::IsFieldDeletedInModel(rIDRA, rTextField);
    }

    void lcl_CalcField( SwDoc& rDoc, SwCalc& rCalc, const SetGetExpField& rSGEField,
            SwDBManager* pMgr, SwRootFrame const*const pLayout)
    {
        const SwTextField* pTextField = rSGEField.GetTextField();
        if( !pTextField )
            return ;

        if (pLayout && pLayout->IsHideRedlines()
            && IsFieldDeleted(rDoc.getIDocumentRedlineAccess(), *pLayout, *pTextField))
        {
            return;
        }

        const SwField* pField = pTextField->GetFormatField().GetField();
        const SwFieldIds nFieldWhich = pField->GetTyp()->Which();

        if( SwFieldIds::SetExp == nFieldWhich )
        {
            SwSbxValue aValue;
            if( nsSwGetSetExpType::GSE_EXPR & pField->GetSubType() )
                aValue.PutDouble( static_cast<const SwSetExpField*>(pField)->GetValue(pLayout) );
            else
                // Extension to calculate with Strings
                aValue.PutString( static_cast<const SwSetExpField*>(pField)->GetExpStr(pLayout) );

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
            case SwFieldIds::DbNumSet:
                {
                    SwDBNumSetField* pDBField = const_cast<SwDBNumSetField*>(static_cast<const SwDBNumSetField*>(pField));

                    SwDBData aDBData(pDBField->GetDBData(&rDoc));

                    if( pDBField->IsCondValid() &&
                        pMgr->OpenDataSource( aDBData.sDataSource, aDBData.sCommand ))
                        rCalc.VarChange( lcl_GetDBVarName( rDoc, *pDBField),
                                        pDBField->GetFormat() );
                }
                break;
            case SwFieldIds::DbNextSet:
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

            default: break;
            }
    #endif
        }
    }
}

namespace sw
{

DocumentFieldsManager::DocumentFieldsManager( SwDoc& i_rSwdoc ) : m_rDoc( i_rSwdoc ),
                                                                  mbNewFieldLst(true),
                                                                  mpUpdateFields(new SwDocUpdateField(m_rDoc)),
                                                                  mpFieldTypes( new SwFieldTypes ),
                                                                  mnLockExpField( 0 )
{
}

const SwFieldTypes* DocumentFieldsManager::GetFieldTypes() const
{
    return mpFieldTypes.get();
}

/** Insert field types
 *
 * @param rFieldTyp ???
 * @return Always returns a pointer to the type, if it's new or already added.
 */
SwFieldType* DocumentFieldsManager::InsertFieldType(const SwFieldType &rFieldTyp)
{
    const SwFieldTypes::size_type nSize = mpFieldTypes->size();
    const SwFieldIds nFieldWhich = rFieldTyp.Which();

    SwFieldTypes::size_type i = INIT_FLDTYPES;

    switch( nFieldWhich )
    {
    case SwFieldIds::SetExp:
            //JP 29.01.96: SequenceFields start at INIT_FLDTYPES - 3!!
            //             Or we get doubble number circles!!
            //MIB 14.03.95: From now on also the SW3-Reader relies on this, when
            //constructing string pools and when reading SetExp fields
            if( nsSwGetSetExpType::GSE_SEQ & static_cast<const SwSetExpFieldType&>(rFieldTyp).GetType() )
                i -= INIT_SEQ_FLDTYPES;
            [[fallthrough]];
    case SwFieldIds::Database:
    case SwFieldIds::User:
    case SwFieldIds::Dde:
        {
            const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
            OUString sFieldNm( rFieldTyp.GetName() );
            for( ; i < nSize; ++i )
                if( nFieldWhich == (*mpFieldTypes)[i]->Which() &&
                    rSCmp.isEqual( sFieldNm, (*mpFieldTypes)[i]->GetName() ))
                        return (*mpFieldTypes)[i];
        }
        break;

    case SwFieldIds::TableOfAuthorities:
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
    case SwFieldIds::Dde:
        static_cast<SwDDEFieldType*>(pNew)->SetDoc( &m_rDoc );
        break;

    case SwFieldIds::Database:
    case SwFieldIds::Table:
    case SwFieldIds::DateTime:
    case SwFieldIds::GetExp:
        static_cast<SwValueFieldType*>(pNew)->SetDoc( &m_rDoc );
        break;

    case SwFieldIds::User:
    case SwFieldIds::SetExp:
        static_cast<SwValueFieldType*>(pNew)->SetDoc( &m_rDoc );
        // JP 29.07.96: Optionally prepare FieldList for Calculator:
        mpUpdateFields->InsertFieldType( *pNew );
        break;
    case SwFieldIds::TableOfAuthorities :
        static_cast<SwAuthorityFieldType*>(pNew)->SetDoc( &m_rDoc );
        break;
    default: break;
    }

    mpFieldTypes->insert( mpFieldTypes->begin() + nSize, pNew );
    m_rDoc.getIDocumentState().SetModified();

    return (*mpFieldTypes)[ nSize ];
}

/// @returns the field type of the Doc
SwFieldType *DocumentFieldsManager::GetSysFieldType( const SwFieldIds eWhich ) const
{
    for( SwFieldTypes::size_type i = 0; i < INIT_FLDTYPES; ++i )
        if( eWhich == (*mpFieldTypes)[i]->Which() )
            return (*mpFieldTypes)[i];
    return nullptr;
}

/// Find first type with ResId and name
SwFieldType* DocumentFieldsManager::GetFieldType(
    SwFieldIds nResId,
    const OUString& rName,
    bool bDbFieldMatching // used in some UNO calls for SwFieldIds::Database to use different string matching code #i51815#
    ) const
{
    const SwFieldTypes::size_type nSize = mpFieldTypes->size();
    SwFieldTypes::size_type i {0};
    const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();

    switch( nResId )
    {
    case SwFieldIds::SetExp:
            //JP 29.01.96: SequenceFields start at INIT_FLDTYPES - 3!!
            //             Or we get doubble number circles!!
            //MIB 14.03.95: From now on also the SW3-Reader relies on this, when
            //constructing string pools and when reading SetExp fields
        i = INIT_FLDTYPES - INIT_SEQ_FLDTYPES;
        break;

    case SwFieldIds::Database:
    case SwFieldIds::User:
    case SwFieldIds::Dde:
    case SwFieldIds::TableOfAuthorities:
        i = INIT_FLDTYPES;
        break;
    default: break;
    }

    SwFieldType* pRet = nullptr;
    for( ; i < nSize; ++i )
    {
        SwFieldType* pFieldType = (*mpFieldTypes)[i];

        if (nResId == pFieldType->Which())
        {
            OUString aFieldName( pFieldType->GetName() );
            if (bDbFieldMatching && nResId == SwFieldIds::Database)    // #i51815#
                aFieldName = aFieldName.replace(DB_DELIM, '.');

            if (rSCmp.isEqual( rName, aFieldName ))
            {
                pRet = pFieldType;
                break;
            }
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
        SwFieldIds nWhich = pTmp->Which();
        switch( nWhich )
        {
        case SwFieldIds::SetExp:
        case SwFieldIds::User:
            mpUpdateFields->RemoveFieldType( *pTmp );
            [[fallthrough]];
        case SwFieldIds::Dde:
            if( pTmp->HasWriterListeners() && !m_rDoc.IsUsed( *pTmp ) )
            {
                if( SwFieldIds::SetExp == nWhich )
                    static_cast<SwSetExpFieldType*>(pTmp)->SetDeleted( true );
                else if( SwFieldIds::User == nWhich )
                    static_cast<SwUserFieldType*>(pTmp)->SetDeleted( true );
                else
                    static_cast<SwDDEFieldType*>(pTmp)->SetDeleted( true );
                nWhich = SwFieldIds::Database;
            }
            break;
        default: break;
        }

        if( nWhich != SwFieldIds::Database )
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
void DocumentFieldsManager::UpdateFields( bool bCloseDB )
{
    // Call Modify() for every field type,
    // dependent SwTextField get notified ...

    for( auto pFieldType : *mpFieldTypes )
    {
        switch( pFieldType->Which() )
        {
            // Update table fields second to last
            // Update references last
        case SwFieldIds::GetRef:
        case SwFieldIds::Table:
        case SwFieldIds::Database:
        case SwFieldIds::JumpEdit:
        case SwFieldIds::RefPageSet:     // are never expanded!
            break;

        case SwFieldIds::Dde:
        {
            SwMsgPoolItem aUpdateDDE( RES_UPDATEDDETBL );
            pFieldType->ModifyNotification( nullptr, &aUpdateDDE );
            break;
        }
        case SwFieldIds::GetExp:
        case SwFieldIds::SetExp:
        case SwFieldIds::HiddenText:
        case SwFieldIds::HiddenPara:
            // Expression fields are treated separately
            break;
        default:
            pFieldType->ModifyNotification ( nullptr, nullptr );
        }
    }

    if( !IsExpFieldsLocked() )
        UpdateExpFields( nullptr, false );      // update expression fields

    // Tables
    UpdateTableFields(nullptr);

    // References
    UpdateRefFields();
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
    // One has to look this up again, now.
    // - If it's not present, it can be re-inserted.
    // - If the same type is found, the deleted one has to be renamed.

    const SwFieldTypes::size_type nSize = mpFieldTypes->size();
    const SwFieldIds nFieldWhich = rFieldTyp.Which();

    OSL_ENSURE( SwFieldIds::SetExp == nFieldWhich ||
            SwFieldIds::User == nFieldWhich ||
            SwFieldIds::Dde == nFieldWhich, "Wrong FieldType" );

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
    case SwFieldIds::SetExp:
        static_cast<SwSetExpFieldType&>(rFieldTyp).SetDeleted( false );
        break;
    case SwFieldIds::User:
        static_cast<SwUserFieldType&>(rFieldTyp).SetDeleted( false );
        break;
    case SwFieldIds::Dde:
        static_cast<SwDDEFieldType&>(rFieldTyp).SetDeleted( false );
        break;
    default: break;
    }
}

void DocumentFieldsManager::PutValueToField(const SwPosition & rPos,
                            const Any& rVal, sal_uInt16 nWhich)
{
    Any aOldVal;
    SwField * pField = GetFieldAtPos(rPos);

    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo() &&
        pField->QueryValue(aOldVal, nWhich))
    {
        m_rDoc.GetIDocumentUndoRedo().AppendUndo(
            std::make_unique<SwUndoFieldFromAPI>(rPos, aOldVal, rVal, nWhich));
    }

    pField->PutValue(rVal, nWhich);
}

bool DocumentFieldsManager::UpdateField(SwTextField * pDstTextField, SwField & rSrcField,
                      SwMsgPoolItem * pMsgHint,
                      bool bUpdateFields)
{
    OSL_ENSURE(pDstTextField, "no field to update!");

    bool bTableSelBreak = false;

    SwFormatField * pDstFormatField = const_cast<SwFormatField*>(&pDstTextField->GetFormatField());
    SwField * pDstField = pDstFormatField->GetField();
    SwFieldIds nFieldWhich = rSrcField.GetTyp()->Which();
    SwNodeIndex aTableNdIdx(pDstTextField->GetTextNode());

    if (pDstField->GetTyp()->Which() ==
        rSrcField.GetTyp()->Which())
    {
        if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
        {
            SwPosition aPosition( pDstTextField->GetTextNode() );
            aPosition.nContent = pDstTextField->GetStart();

            m_rDoc.GetIDocumentUndoRedo().AppendUndo(
                std::make_unique<SwUndoFieldFromDoc>( aPosition, *pDstField, rSrcField, pMsgHint, bUpdateFields) );
        }

        pDstFormatField->SetField(rSrcField.CopyField());
        SwField* pNewField = pDstFormatField->GetField();

        switch( nFieldWhich )
        {
        case SwFieldIds::SetExp:
        case SwFieldIds::GetExp:
        case SwFieldIds::HiddenText:
        case SwFieldIds::HiddenPara:
            UpdateExpFields( pDstTextField, true );
            break;

        case SwFieldIds::Table:
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

        case SwFieldIds::Macro:
            if( bUpdateFields && pDstTextField->GetpTextNode() )
                pDstTextField->GetpTextNode()->
                    ModifyNotification( nullptr, pDstFormatField );
            break;

        case SwFieldIds::DatabaseName:
        case SwFieldIds::DbNextSet:
        case SwFieldIds::DbNumSet:
        case SwFieldIds::DbSetNumber:
            m_rDoc.ChgDBData(static_cast<SwDBNameInfField*>( pNewField)->GetRealDBData());
            pNewField->GetTyp()->UpdateFields();

            break;

        case SwFieldIds::Database:
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
            [[fallthrough]];

        default:
            pDstFormatField->ModifyNotification( nullptr, pMsgHint );
        }

        // The fields we can calculate here are being triggered for an update
        // here explicitly.
        if( nFieldWhich == SwFieldIds::User )
            UpdateUsrFields();
    }

    return bTableSelBreak;
}

/// Update reference and table fields
void DocumentFieldsManager::UpdateRefFields()
{
    for( auto pFieldType : *mpFieldTypes )
        if( SwFieldIds::GetRef == pFieldType->Which() )
            pFieldType->ModifyNotification( nullptr, nullptr );
}

void DocumentFieldsManager::UpdateTableFields( SfxPoolItem* pHt )
{
    OSL_ENSURE( !pHt || RES_TABLEFML_UPDATE  == pHt->Which(),
            "What MessageItem is this?" );

    SwFieldType* pFieldType(nullptr);

    for (auto pFieldTypeTmp : *mpFieldTypes)
    {
        if( SwFieldIds::Table == pFieldTypeTmp->Which() )
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
                        // table where this field is located
                        const SwTableNode* pTableNd;
                        const SwTextNode& rTextNd = pFormatField->GetTextField()->GetTextNode();
                        if(!rTextNd.GetNodes().IsDocNodes())
                            continue;
                        pTableNd = rTextNd.FindTableNode();
                        if (pTableNd == nullptr)
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
                            // is this the wanted table?
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
                            // is this the wanted table?
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

    SwRootFrame const* pLayout(nullptr);
    for (SwRootFrame const*const pLay : m_rDoc.GetAllLayouts())
    {
        assert(!pLayout || pLay->IsHideRedlines() == pLayout->IsHideRedlines()); // TODO
        pLayout = pLay;
    }

    // all fields/boxes are now invalid, so we can start to calculate
    if( pHt && ( RES_TABLEFML_UPDATE != pHt->Which() ||
                TBL_CALC != static_cast<SwTableFormulaUpdate*>(pHt)->m_eFlags ))
        return ;

    std::unique_ptr<SwCalc, o3tl::default_delete<SwCalc>> pCalc;

    if( pFieldType )
    {
        SwIterator<SwFormatField,SwFieldType> aIter( *pFieldType );
        for( SwFormatField* pFormatField = aIter.Last(); pFormatField; pFormatField = aIter.Previous() )
        {
                // start calculation at the end
                // new fields are inserted at the beginning of the modify chain
                // that gives faster calculation on import
                // mba: do we really need this "optimization"? Is it still valid?
                if (!pFormatField->GetTextField())
                    continue;
                SwTableField *const pField(static_cast<SwTableField*>(pFormatField->GetField()));
                if (nsSwExtendedSubType::SUB_CMD & pField->GetSubType())
                    continue;

                // needs to be recalculated
                if( !pField->IsValid() )
                {
                    // table where this field is located
                    const SwTextNode& rTextNd = pFormatField->GetTextField()->GetTextNode();
                    if( !rTextNd.GetNodes().IsDocNodes() )
                        continue;
                    const SwTableNode* pTableNd = rTextNd.FindTableNode();
                    if( !pTableNd )
                        continue;

                    // if this field is not in the to-be-updated table, skip it
                    if( pHt && &pTableNd->GetTable() !=
                                            static_cast<SwTableFormulaUpdate*>(pHt)->m_pTable )
                        continue;

                    if( !pCalc )
                        pCalc.reset(new SwCalc( m_rDoc ));

                    // get the values of all SetExpression fields that are valid
                    // until the table
                    SwFrame* pFrame = nullptr;
                    if( pTableNd->GetIndex() < m_rDoc.GetNodes().GetEndOfExtras().GetIndex() )
                    {
                        // is in the special section, that's expensive!
                        Point aPt;      // return the first frame of the layout - Tab.Headline!!
                        std::pair<Point, bool> const tmp(aPt, true);
                        pFrame = rTextNd.getLayoutFrame(pLayout, nullptr, &tmp);
                        if( pFrame )
                        {
                            SwPosition aPos( *pTableNd );
                            if( GetBodyTextNode( m_rDoc, aPos, *pFrame ) )
                                FieldsToCalc( *pCalc, SetGetExpField(
                                    aPos.nNode, pFormatField->GetTextField(),
                                    &aPos.nContent), pLayout);
                            else
                                pFrame = nullptr;
                        }
                    }
                    if( !pFrame )
                    {
                        // create index to determine the TextNode
                        SwNodeIndex aIdx( rTextNd );
                        FieldsToCalc( *pCalc,
                            SetGetExpField(aIdx, pFormatField->GetTextField()),
                            pLayout);
                    }

                    SwTableCalcPara aPara(*pCalc, pTableNd->GetTable(), pLayout);
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
                    pCalc->SetCalcError( SwCalcError::NONE );
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
                        pCalc.reset(new SwCalc( m_rDoc ));

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

                        std::pair<Point, bool> const tmp(aPt, true);
                        if (pCNd && nullptr != (pFrame = pCNd->getLayoutFrame(
                                pLayout, nullptr, &tmp)))
                        {
                            SwPosition aPos( *pCNd );
                            if( GetBodyTextNode( m_rDoc, aPos, *pFrame ) )
                                FieldsToCalc(*pCalc, SetGetExpField(aPos.nNode),
                                        pLayout);
                            else
                                pFrame = nullptr;
                        }
                    }
                    if( !pFrame )
                    {
                        // create index to determine the TextNode
                        SwNodeIndex aIdx( *pTableNd );
                        FieldsToCalc(*pCalc, SetGetExpField(aIdx), pLayout);
                    }

                    SwTableCalcPara aPara(*pCalc, pTableNd->GetTable(), pLayout);
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
                                    svl::Items<RES_BOXATR_BEGIN,RES_BOXATR_END-1>{} );

                    if( pCalc->IsCalcError() )
                        nValue = DBL_MAX;
                    aTmp.Put( SwTableBoxValue( nValue ));
                    if( SfxItemState::SET != pFormat->GetItemState( RES_BOXATR_FORMAT ))
                        aTmp.Put( SwTableBoxNumFormat( 0 ));
                    pFormat->SetFormatAttr( aTmp );

                    pCalc->SetCalcError( SwCalcError::NONE );
                }
            }
        }
    }
}

void DocumentFieldsManager::UpdateExpFields( SwTextField* pUpdateField, bool bUpdRefFields )
{
    if( IsExpFieldsLocked() || m_rDoc.IsInReading() )
        return;

    bool bOldInUpdateFields = mpUpdateFields->IsInUpdateFields();
    mpUpdateFields->SetInUpdateFields( true );

    mpUpdateFields->MakeFieldList( m_rDoc, true, GETFLD_ALL );
    mbNewFieldLst = false;

    if (mpUpdateFields->GetSortList()->empty())
    {
        if( bUpdRefFields )
            UpdateRefFields();

        mpUpdateFields->SetInUpdateFields( bOldInUpdateFields );
        mpUpdateFields->SetFieldsDirty( false );
        return ;
    }

    SwRootFrame const* pLayout(nullptr);
    SwRootFrame const* pLayoutRLHidden(nullptr);
    for (SwRootFrame const*const pLay : m_rDoc.GetAllLayouts())
    {
        if (pLay->IsHideRedlines())
        {
            pLayoutRLHidden = pLay;
        }
        else
        {
            pLayout = pLay;
        }
    }
    if (pLayout || !pLayoutRLHidden) // always calc *something*...
    {
        UpdateExpFieldsImpl(pUpdateField, pLayout);
    }
    if (pLayoutRLHidden)
    {
        UpdateExpFieldsImpl(pUpdateField, pLayoutRLHidden);
    }

    // update reference fields
    if( bUpdRefFields )
        UpdateRefFields();

    mpUpdateFields->SetInUpdateFields( bOldInUpdateFields );
    mpUpdateFields->SetFieldsDirty( false );
}

void DocumentFieldsManager::UpdateExpFieldsImpl(
        SwTextField * pUpdateField, SwRootFrame const*const pLayout)
{
    SwFieldIds nWhich;

    // Hash table for all string replacements is filled on-the-fly.
    // Try to fabricate an uneven number.
    const SwFieldTypes::size_type nHashSize {(( mpFieldTypes->size() / 7 ) + 1 ) * 7};
    const sal_uInt16 nStrFormatCnt = static_cast<sal_uInt16>(nHashSize);
    OSL_ENSURE( nStrFormatCnt == nHashSize, "Downcasting to sal_uInt16 lost information!" );
    SwHashTable<HashStr> aHashStrTable(nStrFormatCnt);

    {
        const SwFieldType* pFieldType;
        // process separately:
        for( auto n = mpFieldTypes->size(); n; )
        {
            pFieldType = (*mpFieldTypes)[ --n ];
            switch( pFieldType->Which() )
            {
            case SwFieldIds::User:
                {
                    // Entry present?
                    sal_uInt16 nPos;
                    const OUString& rNm = pFieldType->GetName();
                    OUString sExpand(const_cast<SwUserFieldType*>(static_cast<const SwUserFieldType*>(pFieldType))->Expand(nsSwGetSetExpType::GSE_STRING, 0, LANGUAGE_SYSTEM));
                    SwHash* pFnd = aHashStrTable.Find( rNm, &nPos );
                    if( pFnd )
                        // modify entry in the hash table
                        static_cast<HashStr*>(pFnd)->aSetStr = sExpand;
                    else
                        // insert the new entry
                        aHashStrTable[nPos].reset( new HashStr( rNm, sExpand,
                                                                aHashStrTable[nPos].release() ) );
                }
                break;
            default: break;
            }
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
    const LanguageType nLang = pLclData->getLanguageTag().getLanguageType();
    bool bCanFill = pMgr->FillCalcWithMergeData( m_rDoc.GetNumberFormatter(), nLang, aCalc );
#endif

    // Make sure we don't hide all content, which would lead to a crash. First, count how many visible sections we have.
    int nShownSections = 0;
    sal_uLong nContentStart = m_rDoc.GetNodes().GetEndOfContent().StartOfSectionIndex() + 1;
    sal_uLong nContentEnd = m_rDoc.GetNodes().GetEndOfContent().GetIndex();
    SwSectionFormats& rSectFormats = m_rDoc.GetSections();
    for( SwSectionFormats::size_type n = 0; n<rSectFormats.size(); ++n )
    {
        SwSectionFormat& rSectFormat = *rSectFormats[ n ];
        SwSectionNode* pSectionNode = rSectFormat.GetSectionNode();
        SwSection* pSect = rSectFormat.GetSection();

        // Usually some of the content is not in a section: count that as a virtual section, so that all real sections can be hidden.
        // Only look for section gaps at the lowest level, ignoring sub-sections.
        if ( pSectionNode && !rSectFormat.GetParent() )
        {
            SwNodeIndex aNextIdx( *pSectionNode->EndOfSectionNode(), 1 );
            if ( n == 0 && pSectionNode->GetIndex() != nContentStart )
                nShownSections++;  //document does not start with a section
            if ( n == rSectFormats.size() - 1 )
            {
                if ( aNextIdx.GetIndex() != nContentEnd )
                    nShownSections++;  //document does not end in a section
            }
            else if ( !aNextIdx.GetNode().IsSectionNode() )
                    nShownSections++; //section is not immediately followed by another section
        }

        // count only visible sections
        if ( pSect && !pSect->CalcHiddenFlag())
            nShownSections++;
    }

    IDocumentRedlineAccess const& rIDRA(m_rDoc.getIDocumentRedlineAccess());
    std::unordered_map<SwSetExpFieldType const*, SwTextNode const*> SetExpOutlineNodeMap;

    for (std::unique_ptr<SetGetExpField> const& it : *mpUpdateFields->GetSortList())
    {
        SwSection* pSect = const_cast<SwSection*>(it->GetSection());
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
                        // This would be the last section, so set its condition to false, and avoid hiding it.
                        pSect->SetCondition("0");
                        bHide = false;
                    }
                    nShownSections--;
                }
                pSect->SetCondHidden( bHide );
            }
            continue;
        }

        SwTextField* pTextField = const_cast<SwTextField*>(it->GetTextField());
        if( !pTextField )
        {
            OSL_ENSURE( false, "what's wrong now'" );
            continue;
        }

        if (pLayout && pLayout->IsHideRedlines()
            && IsFieldDeleted(rIDRA, *pLayout, *pTextField))
        {
            continue;
        }

        SwFormatField* pFormatField = const_cast<SwFormatField*>(&pTextField->GetFormatField());
        const SwField* pField = pFormatField->GetField();

        nWhich = pField->GetTyp()->Which();
        switch( nWhich )
        {
        case SwFieldIds::HiddenText:
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
        case SwFieldIds::HiddenPara:
        {
            SwHiddenParaField* pHPField = const_cast<SwHiddenParaField*>(static_cast<const SwHiddenParaField*>(pField));
            SwSbxValue aValue = aCalc.Calculate( pHPField->GetPar1() );
            bool bValue = aValue.GetBool();
            if(!aValue.IsVoidValue())
                pHPField->SetHidden( bValue );
        }
        break;
        case SwFieldIds::DbSetNumber:
#if HAVE_FEATURE_DBCONNECTIVITY
        {
            const_cast<SwDBSetNumberField*>(static_cast<const SwDBSetNumberField*>(pField))->Evaluate(&m_rDoc);
            aCalc.VarChange( sDBNumNm, static_cast<const SwDBSetNumberField*>(pField)->GetSetNumber());
            pField->ExpandField(m_rDoc.IsClipBoard(), nullptr);
        }
#endif
        break;
        case SwFieldIds::DbNextSet:
        case SwFieldIds::DbNumSet:
#if HAVE_FEATURE_DBCONNECTIVITY
        {
            UpdateDBNumFields( *const_cast<SwDBNameInfField*>(static_cast<const SwDBNameInfField*>(pField)), aCalc );
            if( bCanFill )
                bCanFill = pMgr->FillCalcWithMergeData( m_rDoc.GetNumberFormatter(), nLang, aCalc );
        }
#endif
        break;
        case SwFieldIds::Database:
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
            HashStr* pFnd = aHashStrTable.Find( rName, &nPos );
            OUString const value(pField->ExpandField(m_rDoc.IsClipBoard(), nullptr));
            if( pFnd )
            {
                // Modify entry in the hash table
                pFnd->aSetStr = value;
            }
            else
            {
                // insert new entry
               aHashStrTable[nPos].reset( new HashStr( rName,
                    value, aHashStrTable[nPos].release()) );
            }
#endif
        }
        break;
        case SwFieldIds::GetExp:
        case SwFieldIds::SetExp:
        {
            if( nsSwGetSetExpType::GSE_STRING & pField->GetSubType() )        // replace String
            {
                if( SwFieldIds::GetExp == nWhich )
                {
                    SwGetExpField* pGField = const_cast<SwGetExpField*>(static_cast<const SwGetExpField*>(pField));

                    if( (!pUpdateField || pUpdateField == pTextField )
                        && pGField->IsInBodyText() )
                    {
                        OUString aNew = LookString( aHashStrTable, pGField->GetFormula() );
                        pGField->ChgExpStr( aNew, pLayout );
                    }
                }
                else
                {
                    SwSetExpField* pSField = const_cast<SwSetExpField*>(static_cast<const SwSetExpField*>(pField));
                    // is the "formula" a field?
                    OUString aNew = LookString( aHashStrTable, pSField->GetFormula() );

                    if( aNew.isEmpty() )               // nothing found then the formula is the new value
                        aNew = pSField->GetFormula();

                    // only update one field
                    if( !pUpdateField || pUpdateField == pTextField )
                        pSField->ChgExpStr( aNew, pLayout );

                    // lookup the field's name
                    aNew = static_cast<SwSetExpFieldType*>(pSField->GetTyp())->GetSetRefName();
                    // Entry present?
                    sal_uInt16 nPos;
                    HashStr* pFnd = aHashStrTable.Find( aNew, &nPos );
                    if( pFnd )
                        // Modify entry in the hash table
                        pFnd->aSetStr = pSField->GetExpStr(pLayout);
                    else
                    {
                        // insert new entry
                        aHashStrTable[nPos].reset( new HashStr( aNew,
                                        pSField->GetExpStr(pLayout),
                                        aHashStrTable[nPos].release() ) );
                        pFnd = aHashStrTable[nPos].get();
                    }

                    // Extension for calculation with Strings
                    SwSbxValue aValue;
                    aValue.PutString( pFnd->aSetStr );
                    aCalc.VarChange( aNew, aValue );
                }
            }
            else            // recalculate formula
            {
                if( SwFieldIds::GetExp == nWhich )
                {
                    SwGetExpField* pGField = const_cast<SwGetExpField*>(static_cast<const SwGetExpField*>(pField));

                    if( (!pUpdateField || pUpdateField == pTextField )
                        && pGField->IsInBodyText() )
                    {
                        SwSbxValue aValue = aCalc.Calculate(
                                        pGField->GetFormula());
                        if(!aValue.IsVoidValue())
                            pGField->SetValue(aValue.GetDouble(), pLayout);
                    }
                }
                else
                {
                    SwSetExpField* pSField = const_cast<SwSetExpField*>(static_cast<const SwSetExpField*>(pField));
                    SwSetExpFieldType* pSFieldTyp = static_cast<SwSetExpFieldType*>(pField->GetTyp());
                    OUString aNew = pSFieldTyp->GetName();

                    SwNode* pSeqNd = nullptr;

                    if( pSField->IsSequenceField() )
                    {
                        const sal_uInt8 nLvl = pSFieldTyp->GetOutlineLvl();
                        if( MAXLEVEL > nLvl )
                        {
                            // test if the Number needs to be updated
                            pSeqNd = m_rDoc.GetNodes()[ it->GetNode() ];

                            const SwTextNode* pOutlNd = pSeqNd->
                                    FindOutlineNodeOfLevel(nLvl, pLayout);
                            auto const iter(SetExpOutlineNodeMap.find(pSFieldTyp));
                            if (iter == SetExpOutlineNodeMap.end()
                                || iter->second != pOutlNd)
                            {
                                SetExpOutlineNodeMap[pSFieldTyp] = pOutlNd;
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
                        pSField->SetValue(nErg, pLayout);

                        if( pSeqNd )
                            pSFieldTyp->SetChapter(*pSField, *pSeqNd, pLayout);
                    }
                }
            }
        }
        break;
        default: break;
        } // switch

        pFormatField->ModifyNotification( nullptr, nullptr );        // trigger formatting

        if (pUpdateField == pTextField) // if only this one is updated
        {
            if( SwFieldIds::GetExp == nWhich ||      // only GetField or
                SwFieldIds::HiddenText == nWhich ||   // HiddenText?
                SwFieldIds::HiddenPara == nWhich)    // HiddenParaField?
                break;                          // quit
            pUpdateField = nullptr;                       // update all from here on
        }
    }

#if HAVE_FEATURE_DBCONNECTIVITY
    pMgr->CloseAll(false);
#endif
}

/// Insert field type that was marked as deleted
void DocumentFieldsManager::UpdateUsrFields()
{
    SwCalc* pCalc = nullptr;
    for( SwFieldTypes::size_type i = INIT_FLDTYPES; i < mpFieldTypes->size(); ++i )
    {
        const SwFieldType* pFieldType;
        if( SwFieldIds::User == ( pFieldType = (*mpFieldTypes)[i] )->Which() )
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

sal_Int32 DocumentFieldsManager::GetRecordsPerDocument() const
{
    sal_Int32 nRecords = 1;

    mpUpdateFields->MakeFieldList( m_rDoc, true, GETFLD_ALL );
    if (mpUpdateFields->GetSortList()->empty())
        return nRecords;

    for (std::unique_ptr<SetGetExpField> const& it : *mpUpdateFields->GetSortList())
    {
        const SwTextField *pTextField = it->GetTextField();
        if( !pTextField )
            continue;

        const SwFormatField &pFormatField = pTextField->GetFormatField();
        const SwField* pField = pFormatField.GetField();

        switch( pField->GetTyp()->Which() )
        {
        case SwFieldIds::DbNextSet:
        case SwFieldIds::DbNumSet:
            nRecords++;
            break;
        default:
            break;
        }
    }

    return nRecords;
}

void DocumentFieldsManager::UpdatePageFields( SfxPoolItem* pMsgHint )
{
    for( SwFieldTypes::size_type i = 0; i < INIT_FLDTYPES; ++i )
    {
        SwFieldType* pFieldType = (*mpFieldTypes)[ i ];
        switch( pFieldType->Which() )
        {
        case SwFieldIds::PageNumber:
        case SwFieldIds::Chapter:
        case SwFieldIds::GetExp:
        case SwFieldIds::RefPageGet:
            pFieldType->ModifyNotification( nullptr, pMsgHint );
            break;
        case SwFieldIds::DocStat:
            pFieldType->ModifyNotification( nullptr, nullptr );
            break;
        default: break;
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

void DocumentFieldsManager::SetFixFields( const DateTime* pNewDateTime )
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
        DateTime aDateTime( DateTime::SYSTEM );
        nDate = aDateTime.GetDate();
        nTime = aDateTime.GetTime();
    }

    SwFieldIds const aTypes[] {
        /*0*/   SwFieldIds::DocInfo,
        /*1*/   SwFieldIds::Author,
        /*2*/   SwFieldIds::ExtUser,
        /*3*/   SwFieldIds::Filename,
        /*4*/   SwFieldIds::DateTime };  // MUST be at the end!

    for(SwFieldIds aType : aTypes)
    {
        SwFieldType* pFieldType = GetSysFieldType( aType );
        SwIterator<SwFormatField,SwFieldType> aIter( *pFieldType );
        for( SwFormatField* pFormatField = aIter.First(); pFormatField; pFormatField = aIter.Next() )
        {
            if (pFormatField->GetTextField())
            {
                bool bChgd = false;
                switch( aType )
                {
                case SwFieldIds::DocInfo:
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

                case SwFieldIds::Author:
                    if( static_cast<SwAuthorField*>(pFormatField->GetField())->IsFixed() )
                    {
                        bChgd = true;
                        SwAuthorField* pAuthorField = static_cast<SwAuthorField*>(pFormatField->GetField());
                        pAuthorField->SetExpansion( SwAuthorFieldType::Expand( pAuthorField->GetFormat() ) );
                    }
                    break;

                case SwFieldIds::ExtUser:
                    if( static_cast<SwExtUserField*>(pFormatField->GetField())->IsFixed() )
                    {
                        bChgd = true;
                        SwExtUserField* pExtUserField = static_cast<SwExtUserField*>(pFormatField->GetField());
                        pExtUserField->SetExpansion( SwExtUserFieldType::Expand(pExtUserField->GetSubType()) );
                    }
                    break;

                case SwFieldIds::DateTime:
                    if( static_cast<SwDateTimeField*>(pFormatField->GetField())->IsFixed() )
                    {
                        bChgd = true;
                        static_cast<SwDateTimeField*>(pFormatField->GetField())->SetDateTime(
                                                    DateTime(Date(nDate), tools::Time(nTime)) );
                    }
                    break;

                case SwFieldIds::Filename:
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
                default: break;
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

void DocumentFieldsManager::FieldsToCalc(SwCalc& rCalc,
        const SetGetExpField& rToThisField, SwRootFrame const*const pLayout)
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

    if (!mpUpdateFields->GetSortList()->empty())
    {
        SetGetExpFields::const_iterator const itLast =
            mpUpdateFields->GetSortList()->upper_bound(
                &rToThisField);
        for (auto it = mpUpdateFields->GetSortList()->begin(); it != itLast; ++it)
        {
            lcl_CalcField(m_rDoc, rCalc, **it, pMgr, pLayout);
        }
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

    SwRootFrame const* pLayout(nullptr);
    SwRootFrame const* pLayoutRLHidden(nullptr);
    for (SwRootFrame const*const pLay : m_rDoc.GetAllLayouts())
    {
        if (pLay->IsHideRedlines())
        {
            pLayoutRLHidden = pLay;
        }
        else
        {
            pLayout = pLay;
        }
    }

    // note this is not duplicate of the other FieldsToCalc because there is
    // (currently) no SetGetExpField that compares only a position
    for(auto it = mpUpdateFields->GetSortList()->begin();
        it != mpUpdateFields->GetSortList()->end() &&
        ( (*it)->GetNode() < nLastNd ||
          ( (*it)->GetNode() == nLastNd && (*it)->GetContent() <= nLastCnt )
        );
        ++it )
    {
        if (pLayout || !pLayoutRLHidden) // always calc *something*...
        {
            lcl_CalcField( m_rDoc, rCalc, **it, pMgr, pLayout );
        }
        if (pLayoutRLHidden)
        {
            lcl_CalcField( m_rDoc, rCalc, **it, pMgr, pLayoutRLHidden );
        }
    }

#if HAVE_FEATURE_DBCONNECTIVITY
    pMgr->CloseAll(false);
#endif
}

void DocumentFieldsManager::FieldsToExpand( SwHashTable<HashStr> & rHashTable,
        const SetGetExpField& rToThisField, SwRootFrame const& rLayout)
{
    // create the sorted list of all SetFields
    mpUpdateFields->MakeFieldList( m_rDoc, mbNewFieldLst, GETFLD_EXPAND );
    mbNewFieldLst = false;

    IDocumentRedlineAccess const& rIDRA(m_rDoc.getIDocumentRedlineAccess());

    // Hash table for all string replacements is filled on-the-fly.
    // Try to fabricate an uneven number.
    sal_uInt16 nTableSize = ((mpUpdateFields->GetSortList()->size() / 7) + 1) * 7;
    rHashTable.resize(nTableSize);

    SetGetExpFields::const_iterator const itLast =
        mpUpdateFields->GetSortList()->upper_bound(&rToThisField);

    for (auto it = mpUpdateFields->GetSortList()->begin(); it != itLast; ++it)
    {
        const SwTextField* pTextField = (*it)->GetTextField();
        if( !pTextField )
            continue;

        if (rLayout.IsHideRedlines()
            && IsFieldDeleted(rIDRA, rLayout, *pTextField))
        {
            continue;
        }

        const SwField* pField = pTextField->GetFormatField().GetField();
        switch( pField->GetTyp()->Which() )
        {
        case SwFieldIds::SetExp:
            if( nsSwGetSetExpType::GSE_STRING & pField->GetSubType() )
            {
                // set the new value in the hash table
                // is the formula a field?
                SwSetExpField* pSField = const_cast<SwSetExpField*>(static_cast<const SwSetExpField*>(pField));
                OUString aNew = LookString( rHashTable, pSField->GetFormula() );

                if( aNew.isEmpty() )               // nothing found, then the formula is
                    aNew = pSField->GetFormula(); // the new value

                // #i3141# - update expression of field as in method
                // <SwDoc::UpdateExpFields(..)> for string/text fields
                pSField->ChgExpStr(aNew, &rLayout);

                // look up the field's name
                aNew = static_cast<SwSetExpFieldType*>(pSField->GetTyp())->GetSetRefName();
                // Entry present?
                sal_uInt16 nPos;
                SwHash* pFnd = rHashTable.Find( aNew, &nPos );
                if( pFnd )
                    // modify entry in the hash table
                    static_cast<HashStr*>(pFnd)->aSetStr = pSField->GetExpStr(&rLayout);
                else
                    // insert the new entry
                    rHashTable[nPos].reset( new HashStr( aNew,
                            pSField->GetExpStr(&rLayout), rHashTable[nPos].release()));
            }
            break;
        case SwFieldIds::Database:
            {
                const OUString& rName = pField->GetTyp()->GetName();

                // Insert entry in the hash table
                // Entry present?
                sal_uInt16 nPos;
                HashStr* pFnd = rHashTable.Find( rName, &nPos );
                OUString const value(pField->ExpandField(m_rDoc.IsClipBoard(), nullptr));
                if( pFnd )
                {
                    // modify entry in the hash table
                    pFnd->aSetStr = value;
                }
                else
                {
                    // insert the new entry
                    rHashTable[nPos].reset( new HashStr( rName,
                        value, rHashTable[nPos].release()) );
                }
            }
            break;
        default: break;
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
    if (!mbNewFieldLst && !m_rDoc.IsInDtor())
        mpUpdateFields->InsDelFieldInFieldLst( bIns, rField );
}

SwField * DocumentFieldsManager::GetFieldAtPos(const SwPosition & rPos)
{
    SwTextField * const pAttr = GetTextFieldAtPos(rPos);

    return pAttr ? const_cast<SwField *>( pAttr->GetFormatField().GetField() ) : nullptr;
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

void DocumentFieldsManager::InitFieldTypes()       // is being called by the CTOR
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
    // We expect this in the InsertFieldType!
    // MIB 14.04.95: In Sw3StringPool::Setup (sw3imp.cxx) and
    //               lcl_sw3io_InSetExpField (sw3field.cxx) now also
    mpFieldTypes->push_back( new SwSetExpFieldType(&m_rDoc,
                SwResId(STR_POOLCOLL_LABEL_ABB), nsSwGetSetExpType::GSE_SEQ) );
    mpFieldTypes->push_back( new SwSetExpFieldType(&m_rDoc,
                SwResId(STR_POOLCOLL_LABEL_TABLE), nsSwGetSetExpType::GSE_SEQ) );
    mpFieldTypes->push_back( new SwSetExpFieldType(&m_rDoc,
                SwResId(STR_POOLCOLL_LABEL_FRAME), nsSwGetSetExpType::GSE_SEQ) );
    mpFieldTypes->push_back( new SwSetExpFieldType(&m_rDoc,
                SwResId(STR_POOLCOLL_LABEL_DRAWING), nsSwGetSetExpType::GSE_SEQ) );
    mpFieldTypes->push_back( new SwSetExpFieldType(&m_rDoc,
                SwResId(STR_POOLCOLL_LABEL_FIGURE), nsSwGetSetExpType::GSE_SEQ) );

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

    SwFieldIds nFieldType = rDBField.Which();

    bool bPar1 = rCalc.Calculate( rDBField.GetPar1() ).GetBool();

    if( SwFieldIds::DbNextSet == nFieldType )
        static_cast<SwDBNextSetField&>(rDBField).SetCondValid( bPar1 );
    else
        static_cast<SwDBNumSetField&>(rDBField).SetCondValid( bPar1 );

    if( !rDBField.GetRealDBData().sDataSource.isEmpty() )
    {
        // Edit a certain database
        if( SwFieldIds::DbNextSet == nFieldType )
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
    mpUpdateFields.reset();
    mpFieldTypes.reset();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#include <cntfrm.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <swtable.hxx>
#include <calc.hxx>
#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <tox.hxx>
#include <txttxmrk.hxx>
#include <docfld.hxx>   // for expression fields
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
#include <poolfmt.hrc>      // for InitFldTypes

#include <SwUndoField.hxx>
#include "switerator.hxx"

using namespace ::com::sun::star::uno;

extern sal_Bool IsFrameBehind( const SwTxtNode& rMyNd, sal_uInt16 nMySttPos,
                        const SwTxtNode& rBehindNd, sal_uInt16 nSttPos );

/*--------------------------------------------------------------------
    Description: Insert field types
 --------------------------------------------------------------------*/
/*
 *  Implementation of field methods at the Doc
 *  Always returns a pointer to the type, if it's new or already added.
 */
SwFieldType* SwDoc::InsertFldType(const SwFieldType &rFldTyp)
{
    sal_uInt16 nSize = pFldTypes->size(),
            nFldWhich = rFldTyp.Which();

    sal_uInt16 i = INIT_FLDTYPES;

    switch( nFldWhich )
    {
    case RES_SETEXPFLD:
            //JP 29.01.96: SequenceFields start at INIT_FLDTYPES - 3!!
            //             Or we get doubble number circles!!
            //MIB 14.03.95: From now on also the SW3-Reader relies on this, when
            //constructing string pools and when reading SetExp fields
            if( nsSwGetSetExpType::GSE_SEQ & ((SwSetExpFieldType&)rFldTyp).GetType() )
                i -= INIT_SEQ_FLDTYPES;
        // no break;
    case RES_DBFLD:
    case RES_USERFLD:
    case RES_DDEFLD:
        {
            const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
            String sFldNm( rFldTyp.GetName() );
            for( ; i < nSize; ++i )
                if( nFldWhich == (*pFldTypes)[i]->Which() &&
                    rSCmp.isEqual( sFldNm, (*pFldTypes)[i]->GetName() ))
                        return (*pFldTypes)[i];
        }
        break;

    case RES_AUTHORITY:
        for( ; i < nSize; ++i )
            if( nFldWhich == (*pFldTypes)[i]->Which() )
                return (*pFldTypes)[i];
        break;

    default:
        for( i = 0; i < nSize; ++i )
            if( nFldWhich == (*pFldTypes)[i]->Which() )
                return (*pFldTypes)[i];
    }

    SwFieldType* pNew = rFldTyp.Copy();
    switch( nFldWhich )
    {
    case RES_DDEFLD:
        ((SwDDEFieldType*)pNew)->SetDoc( this );
        break;

    case RES_DBFLD:
    case RES_TABLEFLD:
    case RES_DATETIMEFLD:
    case RES_GETEXPFLD:
        ((SwValueFieldType*)pNew)->SetDoc( this );
        break;

    case RES_USERFLD:
    case RES_SETEXPFLD:
        ((SwValueFieldType*)pNew)->SetDoc( this );
        // JP 29.07.96: Optionally prepare FieldList for Calculator:
        pUpdtFlds->InsertFldType( *pNew );
        break;
    case RES_AUTHORITY :
        ((SwAuthorityFieldType*)pNew)->SetDoc( this );
        break;
    }

    pFldTypes->insert( pFldTypes->begin() + nSize, pNew );
    SetModified();

    return (*pFldTypes)[ nSize ];
}

void SwDoc::InsDeletedFldType( SwFieldType& rFldTyp )
{
    // The FldType was marked as deleted and removed from the array.
    // One has to look this up again, now.
    // - If it's not present, it can be re-inserted.
    // - If the same type is found, the deleted one has to be renamed.

    sal_uInt16 nSize = pFldTypes->size(), nFldWhich = rFldTyp.Which();
    sal_uInt16 i = INIT_FLDTYPES;

    OSL_ENSURE( RES_SETEXPFLD == nFldWhich ||
            RES_USERFLD == nFldWhich ||
            RES_DDEFLD == nFldWhich, "Wrong FldType" );

    const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
    const String& rFldNm = rFldTyp.GetName();
    SwFieldType* pFnd;

    for( ; i < nSize; ++i )
        if( nFldWhich == (pFnd = (*pFldTypes)[i])->Which() &&
            rSCmp.isEqual( rFldNm, pFnd->GetName() ) )
        {
            // find new name
            sal_uInt16 nNum = 1;
            do {
                String sSrch( rFldNm );
                sSrch.Append( String::CreateFromInt32( nNum ));
                for( i = INIT_FLDTYPES; i < nSize; ++i )
                    if( nFldWhich == (pFnd = (*pFldTypes)[i])->Which() &&
                        rSCmp.isEqual( sSrch, pFnd->GetName() ) )
                        break;

                if( i >= nSize )        // not found
                {
                    ((String&)rFldNm) = sSrch;
                    break;      // exit while loop
                }
                ++nNum;
            } while( sal_True );
            break;
        }

    // not found, so insert and delete flag
    pFldTypes->insert( pFldTypes->begin() + nSize, &rFldTyp );
    switch( nFldWhich )
    {
    case RES_SETEXPFLD:
        ((SwSetExpFieldType&)rFldTyp).SetDeleted( sal_False );
        break;
    case RES_USERFLD:
        ((SwUserFieldType&)rFldTyp).SetDeleted( sal_False );
        break;
    case RES_DDEFLD:
        ((SwDDEFieldType&)rFldTyp).SetDeleted( sal_False );
        break;
    }
}

/*--------------------------------------------------------------------
    Description: Remove field type
 --------------------------------------------------------------------*/
void SwDoc::RemoveFldType(sal_uInt16 nFld)
{
    OSL_ENSURE( INIT_FLDTYPES <= nFld,  "don't remove InitFlds" );
    /*
     * Dependent fields present -> ErrRaise
     */
    sal_uInt16 nSize = pFldTypes->size();
    if(nFld < nSize)
    {
        SwFieldType* pTmp = (*pFldTypes)[nFld];

        // JP 29.07.96: Optionally prepare FldLst for Calculator
        sal_uInt16 nWhich = pTmp->Which();
        switch( nWhich )
        {
        case RES_SETEXPFLD:
        case RES_USERFLD:
            pUpdtFlds->RemoveFldType( *pTmp );
            // no break;
        case RES_DDEFLD:
            if( pTmp->GetDepends() && !IsUsed( *pTmp ) )
            {
                if( RES_SETEXPFLD == nWhich )
                    ((SwSetExpFieldType*)pTmp)->SetDeleted( sal_True );
                else if( RES_USERFLD == nWhich )
                    ((SwUserFieldType*)pTmp)->SetDeleted( sal_True );
                else
                    ((SwDDEFieldType*)pTmp)->SetDeleted( sal_True );
                nWhich = 0;
            }
            break;
        }

        if( nWhich )
        {
            OSL_ENSURE( !pTmp->GetDepends(), "Dependent fields present!" );
            // delete field type
            delete pTmp;
        }
        pFldTypes->erase( pFldTypes->begin() + nFld );
        SetModified();
    }
}

const SwFldTypes* SwDoc::GetFldTypes() const
{
    return pFldTypes;
}

/*--------------------------------------------------------------------
    Description: Find first type with ResId and name
 --------------------------------------------------------------------*/
SwFieldType* SwDoc::GetFldType( sal_uInt16 nResId, const String& rName,
         bool bDbFieldMatching // used in some UNO calls for RES_DBFLD
                                   // to use different string matching code
                                   // #i51815#
         ) const
{
    sal_uInt16 nSize = pFldTypes->size(), i = 0;
    const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();

    switch( nResId )
    {
    case RES_SETEXPFLD:
            //JP 29.01.96: SequenceFields start at INIT_FLDTYPES - 3!!
            //             Or we get doubble number circles!!
            //MIB 14.03.95: From now on also the SW3-Reader relies on this, when
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
        SwFieldType* pFldType = (*pFldTypes)[i];

        String aFldName( pFldType->GetName() );
        if (bDbFieldMatching && nResId == RES_DBFLD)    // #i51815#
            aFldName.SearchAndReplaceAll(DB_DELIM, '.');

        if( nResId == pFldType->Which() &&
            rSCmp.isEqual( rName, aFldName ))
        {
            pRet = pFldType;
            break;
        }
    }
    return pRet;
}

/*************************************************************************
|*    SwDoc::UpdateFlds()
|*    Description: Update fields
*************************************************************************/
/*
 *    All have to be re-evaluated.
 */
void SwDoc::UpdateFlds( SfxPoolItem *pNewHt, bool bCloseDB )
{
    // Call Modify() for every field type,
    // dependent SwTxtFld get notified ...

    for( sal_uInt16 i=0; i < pFldTypes->size(); ++i)
    {
        switch( (*pFldTypes)[i]->Which() )
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
                (*pFldTypes)[i]->ModifyNotification( 0, &aUpdateDDE );
            }
            else
                (*pFldTypes)[i]->ModifyNotification( 0, pNewHt );
            break;
        }
        case RES_GETEXPFLD:
        case RES_SETEXPFLD:
        case RES_HIDDENTXTFLD:
        case RES_HIDDENPARAFLD:
            // Expression fields are treated seperately
            if( !pNewHt )
                break;
        default:
            (*pFldTypes)[i]->ModifyNotification ( 0, pNewHt );
        }
    }

    if( !IsExpFldsLocked() )
        UpdateExpFlds( 0, sal_False );      // update expression fields

    // Tables
    UpdateTblFlds(pNewHt);

    // References
    UpdateRefFlds(pNewHt);

    if( bCloseDB )
        GetNewDBMgr()->CloseAll();

    // Only evaluate on full update
    SetModified();
}

/******************************************************************************
 *                      void SwDoc::UpdateUsrFlds()
 ******************************************************************************/
void SwDoc::UpdateUsrFlds()
{
    SwCalc* pCalc = 0;
    const SwFieldType* pFldType;
    for( sal_uInt16 i = INIT_FLDTYPES; i < pFldTypes->size(); ++i )
        if( RES_USERFLD == ( pFldType = (*pFldTypes)[i] )->Which() )
        {
            if( !pCalc )
                pCalc = new SwCalc( *this );
            ((SwUserFieldType*)pFldType)->GetValue( *pCalc );
        }

    if( pCalc )
    {
        delete pCalc;
        SetModified();
    }
}

/*--------------------------------------------------------------------
    Description: Update reference and table fields
 --------------------------------------------------------------------*/
void SwDoc::UpdateRefFlds( SfxPoolItem* pHt )
{
    SwFieldType* pFldType;
    for( sal_uInt16 i = 0; i < pFldTypes->size(); ++i )
        if( RES_GETREFFLD == ( pFldType = (*pFldTypes)[i] )->Which() )
            pFldType->ModifyNotification( 0, pHt );
}

//For simplicity assume that all field types have updatable contents so
//optimization currently only available when no fields exist.
bool SwDoc::containsUpdatableFields()
{
    for (sal_uInt16 i = 0; i < pFldTypes->size(); ++i)
    {
        SwFieldType* pFldType = (*pFldTypes)[i];
        SwIterator<SwFmtFld,SwFieldType> aIter(*pFldType);
        if (aIter.First())
            return true;
    }
    return false;
}

void SwDoc::UpdateTblFlds( SfxPoolItem* pHt )
{
    OSL_ENSURE( !pHt || RES_TABLEFML_UPDATE  == pHt->Which(),
            "What MessageItem is this?" );

    SwFieldType* pFldType(0);

    for (sal_uInt16 i = 0; i < pFldTypes->size(); ++i)
    {
        if( RES_TABLEFLD == ( pFldType = (*pFldTypes)[i] )->Which() )
        {
            SwTableFmlUpdate* pUpdtFld = 0;
            if( pHt && RES_TABLEFML_UPDATE == pHt->Which() )
                pUpdtFld = (SwTableFmlUpdate*)pHt;

            SwIterator<SwFmtFld,SwFieldType> aIter( *pFldType );
            for( SwFmtFld* pFmtFld = aIter.First(); pFmtFld; pFmtFld = aIter.Next() )
            {
                if( pFmtFld->GetTxtFld() )
                {
                    SwTblField* pFld = (SwTblField*)pFmtFld->GetFld();

                    if( pUpdtFld )
                    {
                        // table where this field is located
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
                                pFld->ChgValid( sal_False );
                            break;
                        case TBL_BOXNAME:
                            // is this the wanted table?
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
                            // is this the wanted table?
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
                        pFld->ChgValid( sal_False );
                }
            }

            break;
        }
        pFldType = 0;
    }

    // process all table box formuals
    const SfxPoolItem* pItem;
    sal_uInt32 nMaxItems = GetAttrPool().GetItemCount2( RES_BOXATR_FORMULA );
    for (sal_uInt32 i = 0; i < nMaxItems; ++i)
    {
        if( 0 != (pItem = GetAttrPool().GetItem2( RES_BOXATR_FORMULA, i ) ) &&
            ((SwTblBoxFormula*)pItem)->GetDefinedIn() )
        {
            ((SwTblBoxFormula*)pItem)->ChangeState( pHt );
        }
    }


    // all fields/boxes are now invalid, so we can start to calculate
    if( pHt && ( RES_TABLEFML_UPDATE != pHt->Which() ||
                TBL_CALC != ((SwTableFmlUpdate*)pHt)->eFlags ))
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
                // mba: do we really need this "optimization"? Is it still valid?
                SwTblField* pFld;
                if( !pFmtFld->GetTxtFld() || (nsSwExtendedSubType::SUB_CMD &
                    (pFld = (SwTblField*)pFmtFld->GetFld())->GetSubType() ))
                    continue;

                // needs to be recalculated
                if( !pFld->IsValid() )
                {
                    // table where this field is located
                    const SwTxtNode& rTxtNd = pFmtFld->GetTxtFld()->GetTxtNode();
                    if( !rTxtNd.GetNodes().IsDocNodes() )
                        continue;
                    const SwTableNode* pTblNd = rTxtNd.FindTableNode();
                    if( !pTblNd )
                        continue;

                    // if this field is not in the to-be-updated table, skip it
                    if( pHt && &pTblNd->GetTable() !=
                                            ((SwTableFmlUpdate*)pHt)->pTbl )
                        continue;

                    if( !pCalc )
                        pCalc = new SwCalc( *this );

                    // get the values of all SetExpression fields that are valid
                    // until the table
                    SwFrm* pFrm = 0;
                    if( pTblNd->GetIndex() < GetNodes().GetEndOfExtras().GetIndex() )
                    {
                        // is in the special section, that's expensive!
                        Point aPt;      // return the first frame of the layout - Tab.Headline!!
                        pFrm = rTxtNd.getLayoutFrm( GetCurrentLayout(), &aPt );
                        if( pFrm )
                        {
                            SwPosition aPos( *pTblNd );
                            if( GetBodyTxtNode( *this, aPos, *pFrm ) )
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
                    if( aPara.IsStackOverFlow() )
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
        if( 0 != (pItem = GetAttrPool().GetItem2( RES_BOXATR_FORMULA, i ) ) &&
            ((SwTblBoxFormula*)pItem)->GetDefinedIn() &&
            !((SwTblBoxFormula*)pItem)->IsValid() )
        {
            SwTblBoxFormula* pFml = (SwTblBoxFormula*)pItem;
            SwTableBox* pBox = pFml->GetTableBox();
            if( pBox && pBox->GetSttNd() &&
                pBox->GetSttNd()->GetNodes().IsDocNodes() )
            {
                const SwTableNode* pTblNd = pBox->GetSttNd()->FindTableNode();
                if( !pHt || &pTblNd->GetTable() ==
                                            ((SwTableFmlUpdate*)pHt)->pTbl )
                {
                    double nValue;
                    if( !pCalc )
                        pCalc = new SwCalc( *this );

                    // get the values of all SetExpression fields that are valid
                    // until the table
                    SwFrm* pFrm = 0;
                    if( pTblNd->GetIndex() < GetNodes().GetEndOfExtras().GetIndex() )
                    {
                        // is in the special section, that's expensive!
                        Point aPt;      // return the first frame of the layout - Tab.Headline!!
                        SwNodeIndex aCNdIdx( *pTblNd, +2 );
                        SwCntntNode* pCNd = aCNdIdx.GetNode().GetCntntNode();
                        if( !pCNd )
                            pCNd = GetNodes().GoNext( &aCNdIdx );

                        if( pCNd && 0 != (pFrm = pCNd->getLayoutFrm( GetCurrentLayout(), &aPt )) )
                        {
                            SwPosition aPos( *pCNd );
                            if( GetBodyTxtNode( *this, aPos, *pFrm ) )
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

                    if( aPara.IsStackOverFlow() )
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
                    SfxItemSet aTmp( GetAttrPool(),
                                    RES_BOXATR_BEGIN,RES_BOXATR_END-1 );

                    if( pCalc->IsCalcError() )
                        nValue = DBL_MAX;
                    aTmp.Put( SwTblBoxValue( nValue ));
                    if( SFX_ITEM_SET != pFmt->GetItemState( RES_BOXATR_FORMAT ))
                        aTmp.Put( SwTblBoxNumFormat( 0 ));
                    pFmt->SetFmtAttr( aTmp );

                    pCalc->SetCalcError( CALC_NOERR );
                }
            }
        }
    }

    delete pCalc;
}

void SwDoc::UpdatePageFlds( SfxPoolItem* pMsgHnt )
{
    SwFieldType* pFldType;
    for( sal_uInt16 i = 0; i < INIT_FLDTYPES; ++i )
        switch( ( pFldType = (*pFldTypes)[ i ] )->Which() )
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

// ---- Remove all unreferenced field types of a document --
void SwDoc::GCFieldTypes()
{
    for( sal_uInt16 n = pFldTypes->size(); n > INIT_FLDTYPES; )
        if( !(*pFldTypes)[ --n ]->GetDepends() )
            RemoveFldType( n );
}

void SwDoc::LockExpFlds()
{
    ++nLockExpFld;
}

void SwDoc::UnlockExpFlds()
{
    if( nLockExpFld )
        --nLockExpFld;
}

bool SwDoc::IsExpFldsLocked() const
{
    return 0 != nLockExpFld;
}

SwDocUpdtFld& SwDoc::GetUpdtFlds() const
{
    return *pUpdtFlds;
}

bool SwDoc::IsNewFldLst() const
{
    return mbNewFldLst;
}

void SwDoc::SetNewFldLst(bool bFlag)
{
    mbNewFldLst = bFlag;
}

// the StartIndex can be supplied optionally (e.g. if it was queried before - is a virtual
// method otherwise!)
_SetGetExpFld::_SetGetExpFld( const SwNodeIndex& rNdIdx, const SwTxtFld* pFld,
                            const SwIndex* pIdx )
{
    eSetGetExpFldType = TEXTFIELD;
    CNTNT.pTxtFld = pFld;
    nNode = rNdIdx.GetIndex();
    if( pIdx )
        nCntnt = pIdx->GetIndex();
    else if( pFld )
        nCntnt = *pFld->GetStart();
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
        nCntnt = *rINet.GetStart();
}

// Extension for Sections:
// these always have content position 0xffff!
// There is never a field on this, only up to STRING_MAXLEN possible
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
        nCntnt = *rTOX.GetStart();
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

void _SetGetExpFld::GetPos( SwPosition& rPos ) const
{
    rPos.nNode = nNode;
    rPos.nContent.Assign( rPos.nNode.GetNode().GetCntntNode(), nCntnt );
}

void _SetGetExpFld::GetPosOfContent( SwPosition& rPos ) const
{
    const SwNode* pNd = GetNodeFromCntnt();
    if( pNd )
        pNd = pNd->GetCntntNode();

    if( pNd )
    {
        rPos.nNode = *pNd;
        rPos.nContent.Assign( (SwCntntNode*)pNd,GetCntPosFromCntnt() );
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
                return ::IsFrameBehind( *(SwTxtNode*)pNext, nCntnt,
                                        *(SwTxtNode*)pFirst, nCntnt );
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

xub_StrLen _SetGetExpFld::GetCntPosFromCntnt() const
{
    sal_uInt16 nRet = 0;
    if( CNTNT.pTxtFld )
        switch( eSetGetExpFldType )
        {
        case TEXTFIELD:
        case TEXTINET:
        case TEXTTOXMARK:
            nRet = *CNTNT.pTxtFld->GetStart();
            break;
        case CRSRPOS:
            nRet =  CNTNT.pPos->nContent.GetIndex();
            break;
        default:
            break;
        }
    return nRet;
}

_HashStr::_HashStr( const String& rName, const String& rText,
                    _HashStr* pNxt )
    : SwHash( rName ), aSetStr( rText )
{
    pNext = pNxt;
}

// Look up the Name, if it is present, return it's String, otherwise return an empty String
void LookString( SwHash** ppTbl, sal_uInt16 nSize, const String& rName,
                    String& rRet, sal_uInt16* pPos )
{
    rRet = comphelper::string::strip(rName, ' ');
    SwHash* pFnd = Find( rRet, ppTbl, nSize, pPos );
    if( pFnd )
        rRet = ((_HashStr*)pFnd)->aSetStr;
    else
        rRet.Erase();
}

String lcl_GetDBVarName( SwDoc& rDoc, SwDBNameInfField& rDBFld )
{
    SwDBData aDBData( rDBFld.GetDBData( &rDoc ));
    String sDBNumNm;
    SwDBData aDocData = rDoc.GetDBData();

    if( aDBData != aDocData )
    {
        sDBNumNm = aDBData.sDataSource;
        sDBNumNm += DB_DELIM;
        sDBNumNm += String(aDBData.sCommand);
        sDBNumNm += DB_DELIM;
    }
    sDBNumNm += SwFieldType::GetTypeStr(TYP_DBSETNUMBERFLD);

    return sDBNumNm;
}

void lcl_CalcFld( SwDoc& rDoc, SwCalc& rCalc, const _SetGetExpFld& rSGEFld,
                        SwNewDBMgr* pMgr )
{
    const SwTxtFld* pTxtFld = rSGEFld.GetFld();
    if( !pTxtFld )
        return ;

    const SwField* pFld = pTxtFld->GetFld().GetFld();
    const sal_uInt16 nFldWhich = pFld->GetTyp()->Which();

    if( RES_SETEXPFLD == nFldWhich )
    {
        SwSbxValue aValue;
        if( nsSwGetSetExpType::GSE_EXPR & pFld->GetSubType() )
            aValue.PutDouble( ((SwSetExpField*)pFld)->GetValue() );
        else
            // Extension to calculate with Strings
            aValue.PutString( ((SwSetExpField*)pFld)->GetExpStr() );

        // set the new value in Calculator
        rCalc.VarChange( pFld->GetTyp()->GetName(), aValue );
    }
    else if( pMgr )
    {
        switch( nFldWhich )
        {
        case RES_DBNUMSETFLD:
            {
                SwDBNumSetField* pDBFld = (SwDBNumSetField*)pFld;

                SwDBData aDBData(pDBFld->GetDBData(&rDoc));

                if( pDBFld->IsCondValid() &&
                    pMgr->OpenDataSource( aDBData.sDataSource, aDBData.sCommand ))
                    rCalc.VarChange( lcl_GetDBVarName( rDoc, *pDBFld),
                                    pDBFld->GetFormat() );
            }
            break;
        case RES_DBNEXTSETFLD:
            {
                SwDBNextSetField* pDBFld = (SwDBNextSetField*)pFld;
                SwDBData aDBData(pDBFld->GetDBData(&rDoc));
                if( !pDBFld->IsCondValid() ||
                    !pMgr->OpenDataSource( aDBData.sDataSource, aDBData.sCommand ))
                    break;

                String sDBNumNm(lcl_GetDBVarName( rDoc, *pDBFld));
                SwCalcExp* pExp = rCalc.VarLook( sDBNumNm );
                if( pExp )
                    rCalc.VarChange( sDBNumNm, pExp->nValue.GetLong() + 1 );
            }
            break;

        }
    }
}

void SwDoc::FldsToCalc( SwCalc& rCalc, const _SetGetExpFld& rToThisFld )
{
    // create the sorted list of all SetFields
    pUpdtFlds->MakeFldList( *this, mbNewFldLst, GETFLD_CALC );
    mbNewFldLst = sal_False;

    SwNewDBMgr* pMgr = GetNewDBMgr();
    pMgr->CloseAll(sal_False);

    if( !pUpdtFlds->GetSortLst()->empty() )
    {
        _SetGetExpFlds::const_iterator const itLast = std::upper_bound(
                pUpdtFlds->GetSortLst()->begin(),
                pUpdtFlds->GetSortLst()->end(),
                const_cast<_SetGetExpFld*>(&rToThisFld));
        for( _SetGetExpFlds::const_iterator it = pUpdtFlds->GetSortLst()->begin(); it != itLast; ++it )
            lcl_CalcFld( *this, rCalc, **it, pMgr );
    }

    pMgr->CloseAll(sal_False);
}

void SwDoc::FldsToCalc( SwCalc& rCalc, sal_uLong nLastNd, sal_uInt16 nLastCnt )
{
    // create the sorted list of all SetFields
    pUpdtFlds->MakeFldList( *this, mbNewFldLst, GETFLD_CALC );
    mbNewFldLst = sal_False;

    SwNewDBMgr* pMgr = GetNewDBMgr();
    pMgr->CloseAll(sal_False);

    for( _SetGetExpFlds::const_iterator it = pUpdtFlds->GetSortLst()->begin();
        it != pUpdtFlds->GetSortLst()->end() &&
        ( (*it)->GetNode() < nLastNd ||
          ( (*it)->GetNode() == nLastNd && (*it)->GetCntnt() <= nLastCnt )
        );
        ++it )
    {
        lcl_CalcFld( *this, rCalc, **it, pMgr );
    }

    pMgr->CloseAll(sal_False);
}

void SwDoc::FldsToExpand( SwHash**& ppHashTbl, sal_uInt16& rTblSize,
                            const _SetGetExpFld& rToThisFld )
{
    // create the sorted list of all SetFields
    pUpdtFlds->MakeFldList( *this, mbNewFldLst, GETFLD_EXPAND );
    mbNewFldLst = sal_False;

    // Hash table for all string replacements is filled on-the-fly.
    // Try to fabricate an uneven number.
    rTblSize = (( pUpdtFlds->GetSortLst()->size() / 7 ) + 1 ) * 7;
    ppHashTbl = new SwHash*[ rTblSize ];
    memset( ppHashTbl, 0, sizeof( _HashStr* ) * rTblSize );

    _SetGetExpFlds::const_iterator const itLast = std::upper_bound(
        pUpdtFlds->GetSortLst()->begin(),
        pUpdtFlds->GetSortLst()->end(),
        const_cast<_SetGetExpFld*>(&rToThisFld));

    for( _SetGetExpFlds::const_iterator it = pUpdtFlds->GetSortLst()->begin(); it != itLast; ++it )
    {
        const SwTxtFld* pTxtFld = (*it)->GetFld();
        if( !pTxtFld )
            continue;

        const SwField* pFld = pTxtFld->GetFld().GetFld();
        switch( pFld->GetTyp()->Which() )
        {
        case RES_SETEXPFLD:
            if( nsSwGetSetExpType::GSE_STRING & pFld->GetSubType() )
            {
                // set the new value in the hash table
                // is the formula a field?
                SwSetExpField* pSFld = (SwSetExpField*)pFld;
                String aNew;
                LookString( ppHashTbl, rTblSize, pSFld->GetFormula(), aNew );

                if( !aNew.Len() )               // nothing found, then the formula is
                    aNew = pSFld->GetFormula(); // the new value

                // #i3141# - update expression of field as in method
                // <SwDoc::UpdateExpFlds(..)> for string/text fields
                pSFld->ChgExpStr( aNew );

                // look up the field's name
                aNew = ((SwSetExpFieldType*)pSFld->GetTyp())->GetSetRefName();
                // Entry present?
                sal_uInt16 nPos;
                SwHash* pFnd = Find( aNew, ppHashTbl, rTblSize, &nPos );
                if( pFnd )
                    // modify entry in the hash table
                    ((_HashStr*)pFnd)->aSetStr = pSFld->GetExpStr();
                else
                    // insert the new entry
                    *(ppHashTbl + nPos ) = new _HashStr( aNew,
                            pSFld->GetExpStr(), (_HashStr*)*(ppHashTbl + nPos) );
            }
            break;
        case RES_DBFLD:
            {
                const String& rName = pFld->GetTyp()->GetName();

                // Insert entry in the hash table
                // Entry present?
                sal_uInt16 nPos;
                SwHash* pFnd = Find( rName, ppHashTbl, rTblSize, &nPos );
                String const value(pFld->ExpandField(IsClipBoard()));
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

void SwDoc::UpdateExpFlds( SwTxtFld* pUpdtFld, bool bUpdRefFlds )
{
    if( IsExpFldsLocked() || IsInReading() )
        return;

    sal_Bool bOldInUpdateFlds = pUpdtFlds->IsInUpdateFlds();
    pUpdtFlds->SetInUpdateFlds( sal_True );

    pUpdtFlds->MakeFldList( *this, sal_True, GETFLD_ALL );
    mbNewFldLst = sal_False;

    if( pUpdtFlds->GetSortLst()->empty() )
    {
        if( bUpdRefFlds )
            UpdateRefFlds(NULL);

        pUpdtFlds->SetInUpdateFlds( bOldInUpdateFlds );
        pUpdtFlds->SetFieldsDirty( sal_False );
        return ;
    }

    sal_uInt16 nWhich, n;

    // Hash table for all string replacements is filled on-the-fly.
    // Try to fabricate an uneven number.
    sal_uInt16 nStrFmtCnt = (( pFldTypes->size() / 7 ) + 1 ) * 7;
    SwHash** pHashStrTbl = new SwHash*[ nStrFmtCnt ];
    memset( pHashStrTbl, 0, sizeof( _HashStr* ) * nStrFmtCnt );

    {
        const SwFieldType* pFldType;
        // process seperately:
        for( n = pFldTypes->size(); n; )
            switch( ( pFldType = (*pFldTypes)[ --n ] )->Which() )
            {
            case RES_USERFLD:
                {
                    // Entry present?
                    sal_uInt16 nPos;
                    const String& rNm = pFldType->GetName();
                    String sExpand(((SwUserFieldType*)pFldType)->Expand(nsSwGetSetExpType::GSE_STRING, 0, 0));
                    SwHash* pFnd = Find( rNm, pHashStrTbl, nStrFmtCnt, &nPos );
                    if( pFnd )
                        // modify entry in the hash table
                        ((_HashStr*)pFnd)->aSetStr = sExpand;
                    else
                        // insert the new entry
                        *(pHashStrTbl + nPos ) = new _HashStr( rNm, sExpand,
                                                (_HashStr*)*(pHashStrTbl + nPos) );
                }
                break;
            case RES_SETEXPFLD:
                ((SwSetExpFieldType*)pFldType)->SetOutlineChgNd( 0 );
                break;
            }
    }

    // The array is filled with all fields; start calculation.
    SwCalc aCalc( *this );

    String sDBNumNm( SwFieldType::GetTypeStr( TYP_DBSETNUMBERFLD ) );

    // already set the current record number
    SwNewDBMgr* pMgr = GetNewDBMgr();
    pMgr->CloseAll(sal_False);

    String aNew;
    for( _SetGetExpFlds::const_iterator it = pUpdtFlds->GetSortLst()->begin(); it != pUpdtFlds->GetSortLst()->end(); ++it )
    {
        SwSection* pSect = (SwSection*)(*it)->GetSection();
        if( pSect )
        {

            SwSbxValue aValue = aCalc.Calculate(
                                        pSect->GetCondition() );
            if(!aValue.IsVoidValue())
                pSect->SetCondHidden( aValue.GetBool() );
            continue;
        }

        SwTxtFld* pTxtFld = (SwTxtFld*)(*it)->GetFld();
        if( !pTxtFld )
        {
            OSL_ENSURE( !this, "what's wrong now'" );
            continue;
        }

        SwFmtFld* pFmtFld = (SwFmtFld*)&pTxtFld->GetFld();
        SwField* pFld = pFmtFld->GetFld();

        switch( nWhich = pFld->GetTyp()->Which() )
        {
        case RES_HIDDENTXTFLD:
        {
            SwHiddenTxtField* pHFld = (SwHiddenTxtField*)pFld;
            SwSbxValue aValue = aCalc.Calculate( pHFld->GetPar1() );
            sal_Bool bValue = !aValue.GetBool();
            if(!aValue.IsVoidValue())
            {
                pHFld->SetValue( bValue );
                // evaluate field
                pHFld->Evaluate(this);
            }
        }
        break;
        case RES_HIDDENPARAFLD:
        {
            SwHiddenParaField* pHPFld = (SwHiddenParaField*)pFld;
            SwSbxValue aValue = aCalc.Calculate( pHPFld->GetPar1() );
            sal_Bool bValue = aValue.GetBool();
            if(!aValue.IsVoidValue())
                pHPFld->SetHidden( bValue );
        }
        break;
        case RES_DBSETNUMBERFLD:
        {
            ((SwDBSetNumberField*)pFld)->Evaluate(this);
            aCalc.VarChange( sDBNumNm, ((SwDBSetNumberField*)pFld)->GetSetNumber());
        }
        break;
        case RES_DBNEXTSETFLD:
        case RES_DBNUMSETFLD:
            UpdateDBNumFlds( *(SwDBNameInfField*)pFld, aCalc );
        break;
        case RES_DBFLD:
        {
            // evaluate field
            ((SwDBField*)pFld)->Evaluate();

            SwDBData aTmpDBData(((SwDBField*)pFld)->GetDBData());

            if( pMgr->IsDataSourceOpen(aTmpDBData.sDataSource, aTmpDBData.sCommand, sal_False))
                aCalc.VarChange( sDBNumNm, pMgr->GetSelectedRecordId(aTmpDBData.sDataSource, aTmpDBData.sCommand, aTmpDBData.nCommandType));

            const String& rName = pFld->GetTyp()->GetName();

            // Set value for Calculator
//JP 10.02.96: GetValue doesn't make sense here
//          ((SwDBField*)pFld)->GetValue();

//!OK           aCalc.VarChange(aName, ((SwDBField*)pFld)->GetValue(aCalc));

            // Add entry to hash table
            // Entry present?
            sal_uInt16 nPos;
            SwHash* pFnd = Find( rName, pHashStrTbl, nStrFmtCnt, &nPos );
            String const value(pFld->ExpandField(IsClipBoard()));
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
        }
        break;
        case RES_GETEXPFLD:
        case RES_SETEXPFLD:
        {
            if( nsSwGetSetExpType::GSE_STRING & pFld->GetSubType() )        // replace String
            {
                if( RES_GETEXPFLD == nWhich )
                {
                    SwGetExpField* pGFld = (SwGetExpField*)pFld;

                    if( (!pUpdtFld || pUpdtFld == pTxtFld )
                        && pGFld->IsInBodyTxt() )
                    {
                        LookString( pHashStrTbl, nStrFmtCnt,
                                    pGFld->GetFormula(), aNew );
                        pGFld->ChgExpStr( aNew );
                    }
                }
                else
                {
                    SwSetExpField* pSFld = (SwSetExpField*)pFld;
                    // is the "formula" a field?
                    LookString( pHashStrTbl, nStrFmtCnt,
                                pSFld->GetFormula(), aNew );

                    if( !aNew.Len() )               // nothing found then the formula is the new value
                        aNew = pSFld->GetFormula();

                    // only update one field
                    if( !pUpdtFld || pUpdtFld == pTxtFld )
                        pSFld->ChgExpStr( aNew );

                    // lookup the field's name
                    aNew = ((SwSetExpFieldType*)pSFld->GetTyp())->GetSetRefName();
                    // Entry present?
                    sal_uInt16 nPos;
                    SwHash* pFnd = Find( aNew, pHashStrTbl, nStrFmtCnt, &nPos );
                    if( pFnd )
                        // Modify entry in the hash table
                        ((_HashStr*)pFnd)->aSetStr = pSFld->GetExpStr();
                    else
                        // insert new entry
                        *(pHashStrTbl + nPos ) = pFnd = new _HashStr( aNew,
                                        pSFld->GetExpStr(),
                                        (_HashStr*)*(pHashStrTbl + nPos) );

                    // Extension for calculation with Strings
                    SwSbxValue aValue;
                    aValue.PutString( ((_HashStr*)pFnd)->aSetStr );
                    aCalc.VarChange( aNew, aValue );
                }
            }
            else            // recalculate formula
            {
                if( RES_GETEXPFLD == nWhich )
                {
                    SwGetExpField* pGFld = (SwGetExpField*)pFld;

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
                    SwSetExpField* pSFld = (SwSetExpField*)pFld;
                    SwSetExpFieldType* pSFldTyp = (SwSetExpFieldType*)pFld->GetTyp();
                    aNew = pSFldTyp->GetName();

                    SwNode* pSeqNd = 0;

                    if( pSFld->IsSequenceFld() )
                    {
                        const sal_uInt8 nLvl = pSFldTyp->GetOutlineLvl();
                        if( MAXLEVEL > nLvl )
                        {
                            // test if the Number needs to be updated
                            pSeqNd = GetNodes()[ (*it)->GetNode() ];

                            const SwTxtNode* pOutlNd = pSeqNd->
                                    FindOutlineNodeOfLevel( nLvl );
                            if( pSFldTyp->GetOutlineChgNd() != pOutlNd )
                            {
                                pSFldTyp->SetOutlineChgNd( pOutlNd );
                                aCalc.VarChange( aNew, 0 );
                            }
                        }
                    }

                    aNew += '=';
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

        if( pUpdtFld == pTxtFld )       // if only this one is updated
        {
            if( RES_GETEXPFLD == nWhich ||      // only GetField or
                RES_HIDDENTXTFLD == nWhich ||   // HiddenTxt?
                RES_HIDDENPARAFLD == nWhich)    // HiddenParaFld?
                break;                          // quit
            pUpdtFld = 0;                       // update all from here on
        }
    }

    pMgr->CloseAll(sal_False);
    // delete hash table
    ::DeleteHashTable( pHashStrTbl, nStrFmtCnt );

    // update reference fields
    if( bUpdRefFlds )
        UpdateRefFlds(NULL);

    pUpdtFlds->SetInUpdateFlds( bOldInUpdateFlds );
    pUpdtFlds->SetFieldsDirty( sal_False );
}

void SwDoc::UpdateDBNumFlds( SwDBNameInfField& rDBFld, SwCalc& rCalc )
{
    SwNewDBMgr* pMgr = GetNewDBMgr();

    sal_uInt16 nFldType = rDBFld.Which();

    sal_Bool bPar1 = rCalc.Calculate( rDBFld.GetPar1() ).GetBool();

    if( RES_DBNEXTSETFLD == nFldType )
        ((SwDBNextSetField&)rDBFld).SetCondValid( bPar1 );
    else
        ((SwDBNumSetField&)rDBFld).SetCondValid( bPar1 );

    if( !rDBFld.GetRealDBData().sDataSource.isEmpty() )
    {
        // Edit a certain database
        if( RES_DBNEXTSETFLD == nFldType )
            ((SwDBNextSetField&)rDBFld).Evaluate(this);
        else
            ((SwDBNumSetField&)rDBFld).Evaluate(this);

        SwDBData aTmpDBData( rDBFld.GetDBData(this) );

        if( pMgr->OpenDataSource( aTmpDBData.sDataSource, aTmpDBData.sCommand, -1, false ))
            rCalc.VarChange( lcl_GetDBVarName( *this, rDBFld),
                        pMgr->GetSelectedRecordId(aTmpDBData.sDataSource, aTmpDBData.sCommand, aTmpDBData.nCommandType) );
    }
    else
    {
        OSL_FAIL("TODO: what should happen with unnamed DBFields?");
    }
}

void SwDoc::_InitFieldTypes()       // is being called by the CTOR
{
    // Field types
    pFldTypes->push_back( new SwDateTimeFieldType(this) );
    pFldTypes->push_back( new SwChapterFieldType );
    pFldTypes->push_back( new SwPageNumberFieldType );
    pFldTypes->push_back( new SwAuthorFieldType );
    pFldTypes->push_back( new SwFileNameFieldType(this) );
    pFldTypes->push_back( new SwDBNameFieldType(this) );
    pFldTypes->push_back( new SwGetExpFieldType(this) );
    pFldTypes->push_back( new SwGetRefFieldType( this ) );
    pFldTypes->push_back( new SwHiddenTxtFieldType );
    pFldTypes->push_back( new SwPostItFieldType(this) );
    pFldTypes->push_back( new SwDocStatFieldType(this) );
    pFldTypes->push_back( new SwDocInfoFieldType(this) );
    pFldTypes->push_back( new SwInputFieldType( this ) );
    pFldTypes->push_back( new SwTblFieldType( this ) );
    pFldTypes->push_back( new SwMacroFieldType(this) );
    pFldTypes->push_back( new SwHiddenParaFieldType );
    pFldTypes->push_back( new SwDBNextSetFieldType );
    pFldTypes->push_back( new SwDBNumSetFieldType );
    pFldTypes->push_back( new SwDBSetNumberFieldType );
    pFldTypes->push_back( new SwTemplNameFieldType(this) );
    pFldTypes->push_back( new SwTemplNameFieldType(this) );
    pFldTypes->push_back( new SwExtUserFieldType );
    pFldTypes->push_back( new SwRefPageSetFieldType );
    pFldTypes->push_back( new SwRefPageGetFieldType( this ) );
    pFldTypes->push_back( new SwJumpEditFieldType( this ) );
    pFldTypes->push_back( new SwScriptFieldType( this ) );
    pFldTypes->push_back( new SwCombinedCharFieldType );
    pFldTypes->push_back( new SwDropDownFieldType );

    // Types have to be at the end!
    // We expect this in the InsertFldType!
    // MIB 14.04.95: In Sw3StringPool::Setup (sw3imp.cxx) and
    //               lcl_sw3io_InSetExpField (sw3field.cxx) now also
    pFldTypes->push_back( new SwSetExpFieldType(this,
                SW_RESSTR(STR_POOLCOLL_LABEL_ABB), nsSwGetSetExpType::GSE_SEQ) );
    pFldTypes->push_back( new SwSetExpFieldType(this,
                SW_RESSTR(STR_POOLCOLL_LABEL_TABLE), nsSwGetSetExpType::GSE_SEQ) );
    pFldTypes->push_back( new SwSetExpFieldType(this,
                SW_RESSTR(STR_POOLCOLL_LABEL_FRAME), nsSwGetSetExpType::GSE_SEQ) );
    pFldTypes->push_back( new SwSetExpFieldType(this,
                SW_RESSTR(STR_POOLCOLL_LABEL_DRAWING), nsSwGetSetExpType::GSE_SEQ) );

    OSL_ENSURE( pFldTypes->size() == INIT_FLDTYPES, "Bad initsize: SwFldTypes" );
}

void SwDoc::InsDelFldInFldLst( bool bIns, const SwTxtFld& rFld )
{
    if( !mbNewFldLst || !IsInDtor() )
        pUpdtFlds->InsDelFldInFldLst( bIns, rFld );
}

SwDBData SwDoc::GetDBData()
{
    return GetDBDesc();
}

const SwDBData& SwDoc::GetDBDesc()
{
    if(aDBData.sDataSource.isEmpty())
    {
        const sal_uInt16 nSize = pFldTypes->size();
        for(sal_uInt16 i = 0; i < nSize && aDBData.sDataSource.isEmpty(); ++i)
        {
            SwFieldType& rFldType = *((*pFldTypes)[i]);
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
                                    aDBData =
                                        (static_cast < SwDBFieldType * > (pFld->GetFld()->GetTyp()))
                                            ->GetDBData();
                                else
                                    aDBData = (static_cast < SwDBNameInfField* > (pFld->GetFld()))->GetRealDBData();
                                break;
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
    if(aDBData.sDataSource.isEmpty())
        aDBData = GetNewDBMgr()->GetAddressDBName();
    return aDBData;
}

void SwDoc::SetInitDBFields( sal_Bool b )
{
    GetNewDBMgr()->SetInitDBFields( b );
}

/*--------------------------------------------------------------------
    Description: Get all databases that are used by fields
 --------------------------------------------------------------------*/
String lcl_DBDataToString(const SwDBData& rData)
{
    String sRet = rData.sDataSource;
    sRet += DB_DELIM;
    sRet += (String)rData.sCommand;
    sRet += DB_DELIM;
    sRet += String::CreateFromInt32(rData.nCommandType);
    return sRet;
}

void SwDoc::GetAllUsedDB( std::vector<String>& rDBNameList,
                          const std::vector<String>* pAllDBNames )
{
    std::vector<String> aUsedDBNames;
    std::vector<String> aAllDBNames;

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
            String aCond( pSect->GetCondition() );
            AddUsedDBToList( rDBNameList, FindUsedDBs( *pAllDBNames,
                                                aCond, aUsedDBNames ) );
            aUsedDBNames.clear();
        }
    }

    const SfxPoolItem* pItem;
    sal_uInt32 nMaxItems = GetAttrPool().GetItemCount2( RES_TXTATR_FIELD );
    for (sal_uInt32 n = 0; n < nMaxItems; ++n)
    {
        if( 0 == (pItem = GetAttrPool().GetItem2( RES_TXTATR_FIELD, n ) ))
            continue;

        const SwFmtFld* pFmtFld = (SwFmtFld*)pItem;
        const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
        if( !pTxtFld || !pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
            continue;

        const SwField* pFld = pFmtFld->GetFld();
        switch( pFld->GetTyp()->Which() )
        {
            case RES_DBFLD:
                AddUsedDBToList( rDBNameList,
                                lcl_DBDataToString(((SwDBField*)pFld)->GetDBData() ));
                break;

            case RES_DBSETNUMBERFLD:
            case RES_DBNAMEFLD:
                AddUsedDBToList( rDBNameList,
                                lcl_DBDataToString(((SwDBNameInfField*)pFld)->GetRealDBData() ));
                break;

            case RES_DBNUMSETFLD:
            case RES_DBNEXTSETFLD:
                AddUsedDBToList( rDBNameList,
                                lcl_DBDataToString(((SwDBNameInfField*)pFld)->GetRealDBData() ));
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
}

void SwDoc::GetAllDBNames( std::vector<String>& rAllDBNames )
{
    SwNewDBMgr* pMgr = GetNewDBMgr();

    const SwDSParamArr& rArr = pMgr->GetDSParamArray();
    for(sal_uInt16 i = 0; i < rArr.size(); i++)
    {
        const SwDSParam* pParam = &rArr[i];
        String* pStr = new String( pParam->sDataSource );
        (*pStr) += DB_DELIM;
        (*pStr) += (String)pParam->sCommand;
        rAllDBNames.push_back(*pStr);
    }
}

std::vector<String>& SwDoc::FindUsedDBs( const std::vector<String>& rAllDBNames,
                                    const String& rFormel,
                                   std::vector<String>& rUsedDBNames )
{
    const CharClass& rCC = GetAppCharClass();
    String  sFormel( rFormel);
#ifndef UNX
    sFormel = rCC.uppercase( sFormel );
#endif

    xub_StrLen nPos;
    for (sal_uInt16 i = 0; i < rAllDBNames.size(); ++i )
    {
        String pStr(rAllDBNames[i]);

        if( STRING_NOTFOUND != (nPos = sFormel.Search( pStr )) &&
            sFormel.GetChar( nPos + pStr.Len() ) == '.' &&
            (!nPos || !rCC.isLetterNumeric( sFormel, nPos - 1 )))
        {
            // Look up table name
            xub_StrLen nEndPos;
            nPos += pStr.Len() + 1;
            if( STRING_NOTFOUND != (nEndPos = sFormel.Search('.', nPos)) )
            {
                pStr.Append( DB_DELIM );
                pStr.Append( sFormel.Copy( nPos, nEndPos - nPos ));
                rUsedDBNames.push_back(pStr);
            }
        }
    }
    return rUsedDBNames;
}

void SwDoc::AddUsedDBToList( std::vector<String>& rDBNameList,
                             const std::vector<String>& rUsedDBNames )
{
    for (sal_uInt16 i = 0; i < rUsedDBNames.size(); ++i)
        AddUsedDBToList( rDBNameList, rUsedDBNames[i] );
}

void SwDoc::AddUsedDBToList( std::vector<String>& rDBNameList, const String& rDBName)
{
    if( !rDBName.Len() )
        return;

#ifdef UNX
    for( sal_uInt16 i = 0; i < rDBNameList.size(); ++i )
        if( rDBName == rDBNameList[i].GetToken(0) )
            return;
#else
    const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
    for( sal_uInt16 i = 0; i < rDBNameList.size(); ++i )
        if( rSCmp.isEqual( rDBName, rDBNameList[i].GetToken(0) ) )
            return;
#endif

    SwDBData aData;
    aData.sDataSource = rDBName.GetToken(0, DB_DELIM);
    aData.sCommand = rDBName.GetToken(1, DB_DELIM);
    aData.nCommandType = -1;
    GetNewDBMgr()->CreateDSData(aData);
    rDBNameList.push_back(rDBName);
}

void SwDoc::ChangeDBFields( const std::vector<String>& rOldNames,
                            const String& rNewName )
{
    SwDBData aNewDBData;
    aNewDBData.sDataSource = rNewName.GetToken(0, DB_DELIM);
    aNewDBData.sCommand = rNewName.GetToken(1, DB_DELIM);
    aNewDBData.nCommandType = (short)rNewName.GetToken(2, DB_DELIM).ToInt32();

    String sFormel;

    SwSectionFmts& rArr = GetSections();
    for (sal_uInt16 n = rArr.size(); n; )
    {
        SwSection* pSect = rArr[ --n ]->GetSection();

        if( pSect )
        {
            sFormel = pSect->GetCondition();
            ReplaceUsedDBs( rOldNames, rNewName, sFormel);
            pSect->SetCondition(sFormel);
        }
    }

    const SfxPoolItem* pItem;
    sal_uInt32 nMaxItems = GetAttrPool().GetItemCount2( RES_TXTATR_FIELD );

    for (sal_uInt32 n = 0; n < nMaxItems; ++n )
    {
        if( 0 == (pItem = GetAttrPool().GetItem2( RES_TXTATR_FIELD, n ) ))
            continue;

        SwFmtFld* pFmtFld = (SwFmtFld*)pItem;
        SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
        if( !pTxtFld || !pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
            continue;

        SwField* pFld = pFmtFld->GetFld();
        sal_Bool bExpand = sal_False;

        switch( pFld->GetTyp()->Which() )
        {
            case RES_DBFLD:
                if( IsNameInArray( rOldNames, lcl_DBDataToString(((SwDBField*)pFld)->GetDBData())))
                {
                    SwDBFieldType* pOldTyp = (SwDBFieldType*)pFld->GetTyp();

                    SwDBFieldType* pTyp = (SwDBFieldType*)InsertFldType(
                            SwDBFieldType(this, pOldTyp->GetColumnName(), aNewDBData));

                    pFmtFld->RegisterToFieldType( *pTyp );
                    pFld->ChgTyp(pTyp);

                    ((SwDBField*)pFld)->ClearInitialized();
                    ((SwDBField*)pFld)->InitContent();

                    bExpand = sal_True;
                }
                break;

            case RES_DBSETNUMBERFLD:
            case RES_DBNAMEFLD:
                if( IsNameInArray( rOldNames,
                                lcl_DBDataToString(((SwDBNameInfField*)pFld)->GetRealDBData())))
                {
                    ((SwDBNameInfField*)pFld)->SetDBData(aNewDBData);
                    bExpand = sal_True;
                }
                break;

            case RES_DBNUMSETFLD:
            case RES_DBNEXTSETFLD:
                if( IsNameInArray( rOldNames,
                                lcl_DBDataToString(((SwDBNameInfField*)pFld)->GetRealDBData())))
                {
                    ((SwDBNameInfField*)pFld)->SetDBData(aNewDBData);
                    bExpand = sal_True;
                }
                // no break;
            case RES_HIDDENTXTFLD:
            case RES_HIDDENPARAFLD:
                sFormel = pFld->GetPar1();
                ReplaceUsedDBs( rOldNames, rNewName, sFormel);
                pFld->SetPar1( sFormel );
                bExpand = sal_True;
                break;

            case RES_SETEXPFLD:
            case RES_GETEXPFLD:
            case RES_TABLEFLD:
                sFormel = pFld->GetFormula();
                ReplaceUsedDBs( rOldNames, rNewName, sFormel);
                pFld->SetPar2( sFormel );
                bExpand = sal_True;
                break;
        }

        if (bExpand)
            pTxtFld->ExpandAlways();
    }
    SetModified();
}

void SwDoc::ReplaceUsedDBs( const std::vector<String>& rUsedDBNames,
                            const String& rNewName, String& rFormel )
{
    const CharClass& rCC = GetAppCharClass();
    String  sFormel(rFormel);
    String  sNewName( rNewName );
    sNewName.SearchAndReplace( DB_DELIM, '.');
    //the command type is not part of the condition
    sNewName = sNewName.GetToken(0, DB_DELIM);
    String sUpperNewNm( sNewName );

    for( sal_uInt16 i = 0; i < rUsedDBNames.size(); ++i )
    {
        String  sDBName( rUsedDBNames[i] );

        sDBName.SearchAndReplace( DB_DELIM, '.');
        //cut off command type
        sDBName = sDBName.GetToken(0, DB_DELIM);
        if( !sDBName.Equals( sUpperNewNm ))
        {
            xub_StrLen nPos = 0;

            while ((nPos = sFormel.Search(sDBName, nPos)) != STRING_NOTFOUND)
            {
                if( sFormel.GetChar( nPos + sDBName.Len() ) == '.' &&
                    (!nPos || !rCC.isLetterNumeric( sFormel, nPos - 1 )))
                {
                    rFormel.Erase( nPos, sDBName.Len() );
                    rFormel.Insert( sNewName, nPos );
                    //prevent re-searching - this is useless and provokes
                    //endless loops when names containing each other and numbers are exchanged
                    //e.g.: old ?12345.12345  new: i12345.12345
                    nPos = nPos + sNewName.Len();
                    sFormel = rFormel;
                }
            }
        }
    }
}

sal_Bool SwDoc::IsNameInArray( const std::vector<String>& rArr, const String& rName )
{
#ifdef UNX
    for( sal_uInt16 i = 0; i < rArr.size(); ++i )
        if( rName == rArr[ i ] )
            return sal_True;
#else
    const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
    for( sal_uInt16 i = 0; i < rArr.size(); ++i )
        if( rSCmp.isEqual( rName, rArr[ i] ))
            return sal_True;
#endif
    return sal_False;
}

void SwDoc::SetFixFields( bool bOnlyTimeDate, const DateTime* pNewDateTime )
{
    sal_Bool bIsModified = IsModified();

    sal_uLong nDate, nTime;
    if( pNewDateTime )
    {
        nDate = pNewDateTime->GetDate();
        nTime = pNewDateTime->GetTime();
    }
    else
    {
        nDate = Date( Date::SYSTEM ).GetDate();
        nTime = Time( Time::SYSTEM ).GetTime();
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
        for( SwFmtFld* pFld = aIter.First(); pFld; pFld = aIter.Next() )
        {
            if( pFld && pFld->GetTxtFld() )
            {
                sal_Bool bChgd = sal_False;
                switch( aTypes[ nStt ] )
                {
                case RES_DOCINFOFLD:
                    if( ((SwDocInfoField*)pFld->GetFld())->IsFixed() )
                    {
                        bChgd = sal_True;
                        SwDocInfoField* pDocInfFld = (SwDocInfoField*)pFld->GetFld();
                        pDocInfFld->SetExpansion( ((SwDocInfoFieldType*)
                                    pDocInfFld->GetTyp())->Expand(
                                        pDocInfFld->GetSubType(),
                                        pDocInfFld->GetFormat(),
                                        pDocInfFld->GetLanguage(),
                                        pDocInfFld->GetName() ) );
                    }
                    break;

                case RES_AUTHORFLD:
                    if( ((SwAuthorField*)pFld->GetFld())->IsFixed() )
                    {
                        bChgd = sal_True;
                        SwAuthorField* pAuthorFld = (SwAuthorField*)pFld->GetFld();
                        pAuthorFld->SetExpansion( ((SwAuthorFieldType*)
                                    pAuthorFld->GetTyp())->Expand(
                                                pAuthorFld->GetFormat() ) );
                    }
                    break;

                case RES_EXTUSERFLD:
                    if( ((SwExtUserField*)pFld->GetFld())->IsFixed() )
                    {
                        bChgd = sal_True;
                        SwExtUserField* pExtUserFld = (SwExtUserField*)pFld->GetFld();
                        pExtUserFld->SetExpansion( ((SwExtUserFieldType*)
                                    pExtUserFld->GetTyp())->Expand(
                                            pExtUserFld->GetSubType(),
                                            pExtUserFld->GetFormat()));
                    }
                    break;

                case RES_DATETIMEFLD:
                    if( ((SwDateTimeField*)pFld->GetFld())->IsFixed() )
                    {
                        bChgd = sal_True;
                        ((SwDateTimeField*)pFld->GetFld())->SetDateTime(
                                                    DateTime(Date(nDate), Time(nTime)) );
                    }
                    break;

                case RES_FILENAMEFLD:
                    if( ((SwFileNameField*)pFld->GetFld())->IsFixed() )
                    {
                        bChgd = sal_True;
                        SwFileNameField* pFileNameFld =
                            (SwFileNameField*)pFld->GetFld();
                        pFileNameFld->SetExpansion( ((SwFileNameFieldType*)
                                    pFileNameFld->GetTyp())->Expand(
                                            pFileNameFld->GetFormat() ) );
                    }
                    break;
                }

                // Trigger formatting
                if( bChgd )
                    pFld->ModifyNotification( 0, 0 );
            }
        }
    }

    if( !bIsModified )
        ResetModified();
}

bool SwDoc::SetFieldsDirty( bool b, const SwNode* pChk, sal_uLong nLen )
{
    // See if the supplied nodes actually contain fields.
    // If they don't, the flag doesn't need to be changed.
    sal_Bool bFldsFnd = sal_False;
    if( b && pChk && !GetUpdtFlds().IsFieldsDirty() && !IsInDtor()
        // ?? what's up with Undo, this is also wanted there!
        /*&& &pChk->GetNodes() == &GetNodes()*/ )
    {
        b = sal_False;
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
                    b = sal_True;
                else if( pTNd->GetpSwpHints() && pTNd->GetSwpHints().Count() )
                    for( sal_uInt16 n = 0, nEnd = pTNd->GetSwpHints().Count();
                            n < nEnd; ++n )
                    {
                        const SwTxtAttr* pAttr = pTNd->GetSwpHints()[ n ];
                        if( RES_TXTATR_FIELD == pAttr->Which() )
                        {
                            b = sal_True;
                            break;
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

void SwDoc::ChangeAuthorityData( const SwAuthEntry* pNewData )
{
    const sal_uInt16 nSize = pFldTypes->size();

    for( sal_uInt16 i = INIT_FLDTYPES; i < nSize; ++i )
    {
        SwFieldType* pFldType = (*pFldTypes)[i];
        if( RES_AUTHORITY  == pFldType->Which() )
        {
            SwAuthorityFieldType* pAuthType = (SwAuthorityFieldType*)pFldType;
            pAuthType->ChangeEntryContent(pNewData);
            break;
        }
    }

}

void SwDocUpdtFld::InsDelFldInFldLst( sal_Bool bIns, const SwTxtFld& rFld )
{
    sal_uInt16 nWhich = rFld.GetFld().GetFld()->GetTyp()->Which();
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

    SetFieldsDirty( sal_True );
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

void SwDocUpdtFld::MakeFldList( SwDoc& rDoc, int bAll, int eGetMode )
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

    /// consider and unhide sections
    ///     with hide condition, only in mode GETFLD_ALL (<eGetMode == GETFLD_ALL>)
    ///     notes by OD:
    ///         eGetMode == GETFLD_CALC in call from methods SwDoc::FldsToCalc
    ///         eGetMode == GETFLD_EXPAND in call from method SwDoc::FldsToExpand
    ///         eGetMode == GETFLD_ALL in call from method SwDoc::UpdateExpFlds
    ///         I figured out that hidden section only have to be shown,
    ///         if fields have updated (call by SwDoc::UpdateExpFlds) and thus
    ///         the hide conditions of section have to be updated.
    ///         For correct updating the hide condition of a section, its position
    ///         have to be known in order to insert the hide condition as a new
    ///         expression field into the sorted field list (<pFldSortLst>).
    if ( eGetMode == GETFLD_ALL )
    // Collect the sections first. Supply sections that are hidden by condition
    // with frames so that the contained fields are sorted properly.
    {
        // In order for the frames to be created the right way, they have to be expanded
        // from top to bottom
        std::vector<sal_uLong> aTmpArr;
        SwSectionFmts& rArr = rDoc.GetSections();
        SwSectionNode* pSectNd;
        sal_uInt16 nArrStt = 0;
        sal_uLong nSttCntnt = rDoc.GetNodes().GetEndOfExtras().GetIndex();

        for (sal_uInt16 n = rArr.size(); n; )
        {
            SwSection* pSect = rArr[ --n ]->GetSection();
            if( pSect && pSect->IsHidden() && pSect->GetCondition().Len() &&
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
            pSectNd->GetSection().SetCondHidden( sal_False );
        }
        for (sal_uInt16 n = 0; n < nArrStt; ++n)
        {
            pSectNd = rDoc.GetNodes()[ aTmpArr[ n ] ]->GetSectionNode();
            OSL_ENSURE( pSectNd, "Where is my SectionNode" );
            pSectNd->GetSection().SetCondHidden( sal_False );
        }

        // add all to the list so that they are sorted
        for (sal_uInt16 n = 0; n < aTmpArr.size(); ++n)
        {
            GetBodyNode( *rDoc.GetNodes()[ aTmpArr[ n ] ]->GetSectionNode() );
        }
    }

    rtl::OUString sTrue("TRUE"), sFalse("FALSE");

    sal_Bool bIsDBMgr = 0 != rDoc.GetNewDBMgr();
    sal_uInt16 nWhich, n;
    const rtl::OUString* pFormel = 0;
    const SfxPoolItem* pItem;
    sal_uInt32 nMaxItems = rDoc.GetAttrPool().GetItemCount2( RES_TXTATR_FIELD );
    for( n = 0; n < nMaxItems; ++n )
    {
        if( 0 == (pItem = rDoc.GetAttrPool().GetItem2( RES_TXTATR_FIELD, n )) )
            continue;

        const SwFmtFld* pFmtFld = (SwFmtFld*)pItem;
        const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
        if( !pTxtFld || !pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
            continue;

        const SwField* pFld = pFmtFld->GetFld();
        switch( nWhich = pFld->GetTyp()->Which() )
        {
            case RES_DBSETNUMBERFLD:
            case RES_GETEXPFLD:
                if( GETFLD_ALL == eGetMode )
                    pFormel = &sTrue;
                break;

            case RES_DBFLD:
                if( GETFLD_EXPAND & eGetMode )
                    pFormel = &sTrue;
                break;

            case RES_SETEXPFLD:
                if ( !(eGetMode == GETFLD_EXPAND) ||
                     (nsSwGetSetExpType::GSE_STRING & pFld->GetSubType()) )
                {
                    pFormel = &sTrue;
                }
                break;

            case RES_HIDDENPARAFLD:
                if( GETFLD_ALL == eGetMode )
                {
                    pFormel = &pFld->GetPar1();
                    if (pFormel->isEmpty() || pFormel->equals(sFalse))
                        ((SwHiddenParaField*)pFld)->SetHidden( sal_False );
                    else if (pFormel->equals(sTrue))
                        ((SwHiddenParaField*)pFld)->SetHidden( sal_True );
                    else
                        break;

                    pFormel = 0;
                    // trigger formatting
                    ((SwFmtFld*)pFmtFld)->ModifyNotification( 0, 0 );
                }
                break;

            case RES_HIDDENTXTFLD:
                if( GETFLD_ALL == eGetMode )
                {
                    pFormel = &pFld->GetPar1();
                    if (pFormel->isEmpty() || pFormel->equals(sFalse))
                        ((SwHiddenTxtField*)pFld)->SetValue( sal_True );
                    else if (pFormel->equals(sTrue))
                        ((SwHiddenTxtField*)pFld)->SetValue( sal_False );
                    else
                        break;

                    pFormel = 0;

                    // evaluate field
                    ((SwHiddenTxtField*)pFld)->Evaluate(&rDoc);
                    // trigger formatting
                    ((SwFmtFld*)pFmtFld)->ModifyNotification( 0, 0 );
                }
                break;

            case RES_DBNUMSETFLD:
            {
                SwDBData aDBData(((SwDBNumSetField*)pFld)->GetDBData(&rDoc));

                if (
                     (bIsDBMgr && rDoc.GetNewDBMgr()->OpenDataSource(aDBData.sDataSource, aDBData.sCommand)) &&
                     (GETFLD_ALL == eGetMode || (GETFLD_CALC & eGetMode && ((SwDBNumSetField*)pFld)->IsCondValid()))
                   )
                {
                    pFormel = &pFld->GetPar1();
                }
            }
            break;
            case RES_DBNEXTSETFLD:
            {
                SwDBData aDBData(((SwDBNextSetField*)pFld)->GetDBData(&rDoc));

                if (
                     (bIsDBMgr && rDoc.GetNewDBMgr()->OpenDataSource(aDBData.sDataSource, aDBData.sCommand)) &&
                     (GETFLD_ALL == eGetMode || (GETFLD_CALC & eGetMode && ((SwDBNextSetField*)pFld)->IsCondValid()))
                   )
                {
                    pFormel = &pFld->GetPar1();
                }
            }
            break;
        }

        if (pFormel && !pFormel->isEmpty())
        {
            GetBodyNode( *pTxtFld, nWhich );
            pFormel = 0;
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
    const SwCntntFrm* pFrm = rTxtNd.getLayoutFrm( rDoc.GetCurrentLayout(), &aPt, 0, sal_False );

    _SetGetExpFld* pNew = NULL;
    sal_Bool bIsInBody = sal_False;

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
        SwGetExpField* pGetFld = (SwGetExpField*)rTFld.GetFld().GetFld();
        pGetFld->ChgBodyTxtFlag( bIsInBody );
    }
    else if( RES_DBFLD == nFldWhich )
    {
        SwDBField* pDBFld = (SwDBField*)rTFld.GetFld().GetFld();
        pDBFld->ChgBodyTxtFlag( bIsInBody );
    }

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
            const SwCntntFrm* pFrm = pCNd->getLayoutFrm( rDoc.GetCurrentLayout(), &aPt, 0, sal_False );
            if( !pFrm )
                break;

            bool const bResult = GetBodyTxtNode( rDoc, aPos, *pFrm );
            OSL_ENSURE(bResult, "where is the Field");
            (void) bResult; // unused in non-debug
            pNew = new _SetGetExpFld( rSectNd, &aPos );

        } while( sal_False );
    }

    if( !pNew )
        pNew = new _SetGetExpFld( rSectNd );

    if( !pFldSortLst->insert( pNew ).second )
        delete pNew;
}

void SwDocUpdtFld::InsertFldType( const SwFieldType& rType )
{
    String sFldName;
    switch( rType.Which() )
    {
    case RES_USERFLD :
        sFldName = ((SwUserFieldType&)rType).GetName();
        break;
    case RES_SETEXPFLD:
        sFldName = ((SwSetExpFieldType&)rType).GetName();
        break;
    default:
        OSL_ENSURE( !this, "kein gueltiger FeldTyp" );
    }

    if( sFldName.Len() )
    {
        SetFieldsDirty( sal_True );
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
    String sFldName;
    switch( rType.Which() )
    {
    case RES_USERFLD :
        sFldName = ((SwUserFieldType&)rType).GetName();
        break;
    case RES_SETEXPFLD:
        sFldName = ((SwSetExpFieldType&)rType).GetName();
        break;
    }

    if( sFldName.Len() )
    {
        SetFieldsDirty( sal_True );
        // look up and remove from the hash table
        sFldName = GetAppCharClass().lowercase( sFldName );
        sal_uInt16 n;

        SwHash* pFnd = Find( sFldName, GetFldTypeTable(), TBLSZ, &n );
        if( pFnd )
        {
            if( aFldTypeTable[ n ] == pFnd )
                aFldTypeTable[ n ] = (SwCalcFldType*)pFnd->pNext;
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

SwDocUpdtFld::SwDocUpdtFld()
    : pFldSortLst(0),  nFldUpdtPos(LONG_MAX), nFldLstGetMode(0)
{
    bInUpdateFlds = bFldsDirty = sal_False;
    memset( aFldTypeTable, 0, sizeof( aFldTypeTable ) );
}

SwDocUpdtFld::~SwDocUpdtFld()
{
    delete pFldSortLst;

    for( sal_uInt16 n = 0; n < TBLSZ; ++n )
        delete aFldTypeTable[n];
}

bool SwDoc::UpdateFld(SwTxtFld * pDstTxtFld, SwField & rSrcFld,
                      SwMsgPoolItem * pMsgHnt,
                      bool bUpdateFlds)
{
    OSL_ENSURE(pDstTxtFld, "no field to update!");

    sal_Bool bTblSelBreak = sal_False;

    SwFmtFld * pDstFmtFld = (SwFmtFld*)&pDstTxtFld->GetFld();
    SwField * pDstFld = pDstFmtFld->GetFld();
    sal_uInt16 nFldWhich = rSrcFld.GetTyp()->Which();
    SwNodeIndex aTblNdIdx(pDstTxtFld->GetTxtNode());

    if (pDstFld->GetTyp()->Which() ==
        rSrcFld.GetTyp()->Which())
    {
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            SwPosition aPosition( pDstTxtFld->GetTxtNode() );
            aPosition.nContent = *pDstTxtFld->GetStart();

            SwUndo *const pUndo( new SwUndoFieldFromDoc(
                        aPosition, *pDstFld, rSrcFld, pMsgHnt, bUpdateFlds) );
            GetIDocumentUndoRedo().AppendUndo(pUndo);
        }

        SwField * pNewFld = rSrcFld.CopyField();
        pDstFmtFld->SetFld(pNewFld);

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
                    IsIdxInTbl(aTblNdIdx);
                if( pTblNd )
                {
                    SwTableFmlUpdate aTblUpdate( &pTblNd->
                                                 GetTable() );
                    if (bUpdateFlds)
                        UpdateTblFlds( &aTblUpdate );
                    else
                        pNewFld->GetTyp()->ModifyNotification(0, &aTblUpdate);

                    if (! bUpdateFlds)
                        bTblSelBreak = sal_True;
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
            ChgDBData(((SwDBNameInfField*) pNewFld)->GetRealDBData());
            pNewFld->GetTyp()->UpdateFlds();

            break;

        case RES_DBFLD:
            {
                // JP 10.02.96: call ChgValue, so that the style change sets the
                // ContentString correctly
                SwDBField* pDBFld = (SwDBField*)pNewFld;
                if (pDBFld->IsInitialized())
                    pDBFld->ChgValue( pDBFld->GetValue(), sal_True );

                pDBFld->ClearInitialized();
                pDBFld->InitContent();
            }
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

bool SwDoc::PutValueToField(const SwPosition & rPos,
                            const Any& rVal, sal_uInt16 nWhich)
{
    Any aOldVal;
    SwField * pField = GetField(rPos);


    if (GetIDocumentUndoRedo().DoesUndo() &&
        pField->QueryValue(aOldVal, nWhich))
    {
        SwUndo *const pUndo(new SwUndoFieldFromAPI(rPos, aOldVal, rVal, nWhich));
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    return pField->PutValue(rVal, nWhich);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

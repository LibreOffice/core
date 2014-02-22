/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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

extern bool IsFrameBehind( const SwTxtNode& rMyNd, sal_Int32 nMySttPos,
                        const SwTxtNode& rBehindNd, sal_Int32 nSttPos );

/** Insert field types
 *
 * @param rFldTyp ???
 * @return Always returns a pointer to the type, if it's new or already added.
 */
SwFieldType* SwDoc::InsertFldType(const SwFieldType &rFldTyp)
{
    sal_uInt16 nSize = mpFldTypes->size(),
            nFldWhich = rFldTyp.Which();

    sal_uInt16 i = INIT_FLDTYPES;

    switch( nFldWhich )
    {
    case RES_SETEXPFLD:
            
            
            
            
            if( nsSwGetSetExpType::GSE_SEQ & ((SwSetExpFieldType&)rFldTyp).GetType() )
                i -= INIT_SEQ_FLDTYPES;
        
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
        
        mpUpdtFlds->InsertFldType( *pNew );
        break;
    case RES_AUTHORITY :
        ((SwAuthorityFieldType*)pNew)->SetDoc( this );
        break;
    }

    mpFldTypes->insert( mpFldTypes->begin() + nSize, pNew );
    SetModified();

    return (*mpFldTypes)[ nSize ];
}


void SwDoc::InsDeletedFldType( SwFieldType& rFldTyp )
{
    
    
    
    

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
            
            sal_uInt16 nNum = 1;
            do {
                OUString sSrch = rFldNm + OUString::number( nNum );
                for( i = INIT_FLDTYPES; i < nSize; ++i )
                    if( nFldWhich == (pFnd = (*mpFldTypes)[i])->Which() &&
                        rSCmp.isEqual( sSrch, pFnd->GetName() ) )
                        break;

                if( i >= nSize )        
                {
                    ((OUString&)rFldNm) = sSrch;
                    break;      
                }
                ++nNum;
            } while( true );
            break;
        }

    
    mpFldTypes->insert( mpFldTypes->begin() + nSize, &rFldTyp );
    switch( nFldWhich )
    {
    case RES_SETEXPFLD:
        ((SwSetExpFieldType&)rFldTyp).SetDeleted( sal_False );
        break;
    case RES_USERFLD:
        ((SwUserFieldType&)rFldTyp).SetDeleted( false );
        break;
    case RES_DDEFLD:
        ((SwDDEFieldType&)rFldTyp).SetDeleted( sal_False );
        break;
    }
}


void SwDoc::RemoveFldType(sal_uInt16 nFld)
{
    OSL_ENSURE( INIT_FLDTYPES <= nFld,  "don't remove InitFlds" );
    /*
     * Dependent fields present -> ErrRaise
     */
    sal_uInt16 nSize = mpFldTypes->size();
    if(nFld < nSize)
    {
        SwFieldType* pTmp = (*mpFldTypes)[nFld];

        
        sal_uInt16 nWhich = pTmp->Which();
        switch( nWhich )
        {
        case RES_SETEXPFLD:
        case RES_USERFLD:
            mpUpdtFlds->RemoveFldType( *pTmp );
            
        case RES_DDEFLD:
            if( pTmp->GetDepends() && !IsUsed( *pTmp ) )
            {
                if( RES_SETEXPFLD == nWhich )
                    ((SwSetExpFieldType*)pTmp)->SetDeleted( sal_True );
                else if( RES_USERFLD == nWhich )
                    ((SwUserFieldType*)pTmp)->SetDeleted( true );
                else
                    ((SwDDEFieldType*)pTmp)->SetDeleted( sal_True );
                nWhich = 0;
            }
            break;
        }

        if( nWhich )
        {
            OSL_ENSURE( !pTmp->GetDepends(), "Dependent fields present!" );
            
            delete pTmp;
        }
        mpFldTypes->erase( mpFldTypes->begin() + nFld );
        SetModified();
    }
}

const SwFldTypes* SwDoc::GetFldTypes() const
{
    return mpFldTypes;
}


SwFieldType* SwDoc::GetFldType(
    sal_uInt16 nResId,
    const OUString& rName,
    bool bDbFieldMatching 
    ) const
{
    sal_uInt16 nSize = mpFldTypes->size(), i = 0;
    const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();

    switch( nResId )
    {
    case RES_SETEXPFLD:
            
            
            
            
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
        if (bDbFieldMatching && nResId == RES_DBFLD)    
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


void SwDoc::UpdateFlds( SfxPoolItem *pNewHt, bool bCloseDB )
{
    
    

    for( sal_uInt16 i=0; i < mpFldTypes->size(); ++i)
    {
        switch( (*mpFldTypes)[i]->Which() )
        {
            
            
        case RES_GETREFFLD:
        case RES_TABLEFLD:
        case RES_DBFLD:
        case RES_JUMPEDITFLD:
        case RES_REFPAGESETFLD:     
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
            
            if( !pNewHt )
                break;
        default:
            (*mpFldTypes)[i]->ModifyNotification ( 0, pNewHt );
        }
    }

    if( !IsExpFldsLocked() )
        UpdateExpFlds( 0, false );      

    
    UpdateTblFlds(pNewHt);

    
    UpdateRefFlds(pNewHt);

    if( bCloseDB )
        GetNewDBMgr()->CloseAll();

    
    SetModified();
}

void SwDoc::UpdateUsrFlds()
{
    SwCalc* pCalc = 0;
    const SwFieldType* pFldType;
    for( sal_uInt16 i = INIT_FLDTYPES; i < mpFldTypes->size(); ++i )
        if( RES_USERFLD == ( pFldType = (*mpFldTypes)[i] )->Which() )
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


void SwDoc::UpdateRefFlds( SfxPoolItem* pHt )
{
    SwFieldType* pFldType;
    for( sal_uInt16 i = 0; i < mpFldTypes->size(); ++i )
        if( RES_GETREFFLD == ( pFldType = (*mpFldTypes)[i] )->Which() )
            pFldType->ModifyNotification( 0, pHt );
}



bool SwDoc::containsUpdatableFields()
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

void SwDoc::UpdateTblFlds( SfxPoolItem* pHt )
{
    OSL_ENSURE( !pHt || RES_TABLEFML_UPDATE  == pHt->Which(),
            "What MessageItem is this?" );

    SwFieldType* pFldType(0);

    for (sal_uInt16 i = 0; i < mpFldTypes->size(); ++i)
    {
        if( RES_TABLEFLD == ( pFldType = (*mpFldTypes)[i] )->Which() )
        {
            SwTableFmlUpdate* pUpdtFld = 0;
            if( pHt && RES_TABLEFML_UPDATE == pHt->Which() )
                pUpdtFld = (SwTableFmlUpdate*)pHt;

            SwIterator<SwFmtFld,SwFieldType> aIter( *pFldType );
            for( SwFmtFld* pFmtFld = aIter.First(); pFmtFld; pFmtFld = aIter.Next() )
            {
                if( pFmtFld->GetTxtFld() )
                {
                    SwTblField* pFld = (SwTblField*)pFmtFld->GetField();

                    if( pUpdtFld )
                    {
                        
                        const SwTableNode* pTblNd;
                        const SwTxtNode& rTxtNd = pFmtFld->GetTxtFld()->GetTxtNode();
                        if( !rTxtNd.GetNodes().IsDocNodes() ||
                            0 == ( pTblNd = rTxtNd.FindTableNode() ) )
                            continue;

                        switch( pUpdtFld->eFlags )
                        {
                        case TBL_CALC:
                            
                            
                            
                            if( nsSwExtendedSubType::SUB_CMD & pFld->GetSubType() )
                                pFld->PtrToBoxNm( pUpdtFld->pTbl );
                            else
                                pFld->ChgValid( false );
                            break;
                        case TBL_BOXNAME:
                            
                            if( &pTblNd->GetTable() == pUpdtFld->pTbl )
                                
                                pFld->PtrToBoxNm( pUpdtFld->pTbl );
                            break;
                        case TBL_BOXPTR:
                            
                            
                            
                            pFld->BoxNmToPtr( pUpdtFld->pTbl );
                            break;
                        case TBL_RELBOXNAME:
                            
                            if( &pTblNd->GetTable() == pUpdtFld->pTbl )
                                
                                pFld->ToRelBoxNm( pUpdtFld->pTbl );
                            break;
                        default:
                            break;
                        }
                    }
                    else
                        
                        pFld->ChgValid( false );
                }
            }

            break;
        }
        pFldType = 0;
    }

    
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

    
    if( pHt && ( RES_TABLEFML_UPDATE != pHt->Which() ||
                TBL_CALC != ((SwTableFmlUpdate*)pHt)->eFlags ))
        return ;

    SwCalc* pCalc = 0;

    if( pFldType )
    {
        SwIterator<SwFmtFld,SwFieldType> aIter( *pFldType );
        for( SwFmtFld* pFmtFld = aIter.Last(); pFmtFld; pFmtFld = aIter.Previous() )
        {
                
                
                
                
                SwTblField* pFld;
                if( !pFmtFld->GetTxtFld() || (nsSwExtendedSubType::SUB_CMD &
                    (pFld = (SwTblField*)pFmtFld->GetField())->GetSubType() ))
                    continue;

                
                if( !pFld->IsValid() )
                {
                    
                    const SwTxtNode& rTxtNd = pFmtFld->GetTxtFld()->GetTxtNode();
                    if( !rTxtNd.GetNodes().IsDocNodes() )
                        continue;
                    const SwTableNode* pTblNd = rTxtNd.FindTableNode();
                    if( !pTblNd )
                        continue;

                    
                    if( pHt && &pTblNd->GetTable() !=
                                            ((SwTableFmlUpdate*)pHt)->pTbl )
                        continue;

                    if( !pCalc )
                        pCalc = new SwCalc( *this );

                    
                    
                    SwFrm* pFrm = 0;
                    if( pTblNd->GetIndex() < GetNodes().GetEndOfExtras().GetIndex() )
                    {
                        
                        Point aPt;      
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

                    
                    
                    SwFrm* pFrm = 0;
                    if( pTblNd->GetIndex() < GetNodes().GetEndOfExtras().GetIndex() )
                    {
                        
                        Point aPt;      
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


void SwDoc::GCFieldTypes()
{
    for( sal_uInt16 n = mpFldTypes->size(); n > INIT_FLDTYPES; )
        if( !(*mpFldTypes)[ --n ]->GetDepends() )
            RemoveFldType( n );
}

void SwDoc::LockExpFlds()
{
    ++mnLockExpFld;
}

void SwDoc::UnlockExpFlds()
{
    if( mnLockExpFld )
        --mnLockExpFld;
}

bool SwDoc::IsExpFldsLocked() const
{
    return 0 != mnLockExpFld;
}

SwDocUpdtFld& SwDoc::GetUpdtFlds() const
{
    return *mpUpdtFlds;
}

bool SwDoc::IsNewFldLst() const
{
    return mbNewFldLst;
}

void SwDoc::SetNewFldLst(bool bFlag)
{
    mbNewFldLst = bFlag;
}



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
        (void) bResult; 
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

    
    if( !pFirst || !pNext )
        return false;

    
    if( pFirst->StartOfSectionNode() != pNext->StartOfSectionNode() )
    {
        
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

    
    if( pFirst != pNext )
        return pFirst->GetIndex() < pNext->GetIndex();

    
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

_HashStr::_HashStr( const OUString& rName, const OUString& rText,
                    _HashStr* pNxt )
    : SwHash( rName ), aSetStr( rText )
{
    pNext = pNxt;
}


OUString LookString( SwHash** ppTbl, sal_uInt16 nSize, const OUString& rName,
                     sal_uInt16* pPos )
{
    SwHash* pFnd = Find( comphelper::string::strip(rName, ' '), ppTbl, nSize, pPos );
    if( pFnd )
        return ((_HashStr*)pFnd)->aSetStr;

    return OUString();
}

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

static void lcl_CalcFld( SwDoc& rDoc, SwCalc& rCalc, const _SetGetExpFld& rSGEFld,
                        SwNewDBMgr* pMgr )
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
            aValue.PutDouble( ((SwSetExpField*)pFld)->GetValue() );
        else
            
            aValue.PutString( ((SwSetExpField*)pFld)->GetExpStr() );

        
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

                OUString sDBNumNm(lcl_GetDBVarName( rDoc, *pDBFld));
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
    
    mpUpdtFlds->MakeFldList( *this, mbNewFldLst, GETFLD_CALC );
    mbNewFldLst = false;

    SwNewDBMgr* pMgr = GetNewDBMgr();
    pMgr->CloseAll(sal_False);

    if( !mpUpdtFlds->GetSortLst()->empty() )
    {
        _SetGetExpFlds::const_iterator const itLast =
            mpUpdtFlds->GetSortLst()->upper_bound(
                const_cast<_SetGetExpFld*>(&rToThisFld));
        for( _SetGetExpFlds::const_iterator it = mpUpdtFlds->GetSortLst()->begin(); it != itLast; ++it )
            lcl_CalcFld( *this, rCalc, **it, pMgr );
    }

    pMgr->CloseAll(sal_False);
}

void SwDoc::FldsToCalc( SwCalc& rCalc, sal_uLong nLastNd, sal_uInt16 nLastCnt )
{
    
    mpUpdtFlds->MakeFldList( *this, mbNewFldLst, GETFLD_CALC );
    mbNewFldLst = false;

    SwNewDBMgr* pMgr = GetNewDBMgr();
    pMgr->CloseAll(sal_False);

    for( _SetGetExpFlds::const_iterator it = mpUpdtFlds->GetSortLst()->begin();
        it != mpUpdtFlds->GetSortLst()->end() &&
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
    
    mpUpdtFlds->MakeFldList( *this, mbNewFldLst, GETFLD_EXPAND );
    mbNewFldLst = false;

    
    
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
                
                
                SwSetExpField* pSFld = (SwSetExpField*)pFld;
                OUString aNew = LookString( ppHashTbl, rTblSize, pSFld->GetFormula() );

                if( aNew.isEmpty() )               
                    aNew = pSFld->GetFormula(); 

                
                
                pSFld->ChgExpStr( aNew );

                
                aNew = ((SwSetExpFieldType*)pSFld->GetTyp())->GetSetRefName();
                
                sal_uInt16 nPos;
                SwHash* pFnd = Find( aNew, ppHashTbl, rTblSize, &nPos );
                if( pFnd )
                    
                    ((_HashStr*)pFnd)->aSetStr = pSFld->GetExpStr();
                else
                    
                    *(ppHashTbl + nPos ) = new _HashStr( aNew,
                            pSFld->GetExpStr(), (_HashStr*)*(ppHashTbl + nPos) );
            }
            break;
        case RES_DBFLD:
            {
                const OUString& rName = pFld->GetTyp()->GetName();

                
                
                sal_uInt16 nPos;
                SwHash* pFnd = Find( rName, ppHashTbl, rTblSize, &nPos );
                OUString const value(pFld->ExpandField(IsClipBoard()));
                if( pFnd )
                {
                    
                    static_cast<_HashStr*>(pFnd)->aSetStr = value;
                }
                else
                {
                    
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

    bool bOldInUpdateFlds = mpUpdtFlds->IsInUpdateFlds();
    mpUpdtFlds->SetInUpdateFlds( true );

    mpUpdtFlds->MakeFldList( *this, true, GETFLD_ALL );
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

    
    
    sal_uInt16 nStrFmtCnt = (( mpFldTypes->size() / 7 ) + 1 ) * 7;
    SwHash** pHashStrTbl = new SwHash*[ nStrFmtCnt ];
    memset( pHashStrTbl, 0, sizeof( _HashStr* ) * nStrFmtCnt );

    {
        const SwFieldType* pFldType;
        
        for( n = mpFldTypes->size(); n; )
            switch( ( pFldType = (*mpFldTypes)[ --n ] )->Which() )
            {
            case RES_USERFLD:
                {
                    
                    sal_uInt16 nPos;
                    const OUString& rNm = pFldType->GetName();
                    OUString sExpand(((SwUserFieldType*)pFldType)->Expand(nsSwGetSetExpType::GSE_STRING, 0, 0));
                    SwHash* pFnd = Find( rNm, pHashStrTbl, nStrFmtCnt, &nPos );
                    if( pFnd )
                        
                        ((_HashStr*)pFnd)->aSetStr = sExpand;
                    else
                        
                        *(pHashStrTbl + nPos ) = new _HashStr( rNm, sExpand,
                                                (_HashStr*)*(pHashStrTbl + nPos) );
                }
                break;
            case RES_SETEXPFLD:
                ((SwSetExpFieldType*)pFldType)->SetOutlineChgNd( 0 );
                break;
            }
    }

    
    SwCalc aCalc( *this );

    OUString sDBNumNm( SwFieldType::GetTypeStr( TYP_DBSETNUMBERFLD ) );

    
    SwNewDBMgr* pMgr = GetNewDBMgr();
    pMgr->CloseAll(sal_False);

    
    int nShownSections = 0;
    for( _SetGetExpFlds::const_iterator it = mpUpdtFlds->GetSortLst()->begin(); it != mpUpdtFlds->GetSortLst()->end(); ++it )
    {
        SwSection* pSect = (SwSection*)(*it)->GetSection();
        if ( pSect && !pSect->IsCondHidden())
            nShownSections++;
    }

    OUString aNew;
    for( _SetGetExpFlds::const_iterator it = mpUpdtFlds->GetSortLst()->begin(); it != mpUpdtFlds->GetSortLst()->end(); ++it )
    {
        SwSection* pSect = (SwSection*)(*it)->GetSection();
        if( pSect )
        {

            SwSbxValue aValue = aCalc.Calculate(
                                        pSect->GetCondition() );
            if(!aValue.IsVoidValue())
            {
                
                bool bHide = aValue.GetBool();
                if (bHide && !pSect->IsCondHidden())
                {
                    
                    if (nShownSections == 1)
                    {
                        
                        SwPaM aPam(GetNodes());
                        aPam.Move(fnMoveForward, fnGoDoc);
                        if (aPam.Start()->nNode.GetNode().StartOfSectionNode()->IsSectionNode())
                        {
                            
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

        SwTxtFld* pTxtFld = (SwTxtFld*)(*it)->GetTxtFld();
        if( !pTxtFld )
        {
            OSL_ENSURE( !this, "what's wrong now'" );
            continue;
        }

        SwFmtFld* pFmtFld = (SwFmtFld*)&pTxtFld->GetFmtFld();
        const SwField* pFld = pFmtFld->GetField();

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
            
            ((SwDBField*)pFld)->Evaluate();

            SwDBData aTmpDBData(((SwDBField*)pFld)->GetDBData());

            if( pMgr->IsDataSourceOpen(aTmpDBData.sDataSource, aTmpDBData.sCommand, sal_False))
                aCalc.VarChange( sDBNumNm, pMgr->GetSelectedRecordId(aTmpDBData.sDataSource, aTmpDBData.sCommand, aTmpDBData.nCommandType));

            const OUString& rName = pFld->GetTyp()->GetName();

            
            
            sal_uInt16 nPos;
            SwHash* pFnd = Find( rName, pHashStrTbl, nStrFmtCnt, &nPos );
            OUString const value(pFld->ExpandField(IsClipBoard()));
            if( pFnd )
            {
                
                static_cast<_HashStr*>(pFnd)->aSetStr = value;
            }
            else
            {
                
                *(pHashStrTbl + nPos ) = new _HashStr( rName,
                    value, static_cast<_HashStr *>(*(pHashStrTbl + nPos)));
            }
        }
        break;
        case RES_GETEXPFLD:
        case RES_SETEXPFLD:
        {
            if( nsSwGetSetExpType::GSE_STRING & pFld->GetSubType() )        
            {
                if( RES_GETEXPFLD == nWhich )
                {
                    SwGetExpField* pGFld = (SwGetExpField*)pFld;

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
                    SwSetExpField* pSFld = (SwSetExpField*)pFld;
                    
                    aNew = LookString( pHashStrTbl, nStrFmtCnt,
                                pSFld->GetFormula() );

                    if( aNew.isEmpty() )               
                        aNew = pSFld->GetFormula();

                    
                    if( !pUpdtFld || pUpdtFld == pTxtFld )
                        pSFld->ChgExpStr( aNew );

                    
                    aNew = ((SwSetExpFieldType*)pSFld->GetTyp())->GetSetRefName();
                    
                    sal_uInt16 nPos;
                    SwHash* pFnd = Find( aNew, pHashStrTbl, nStrFmtCnt, &nPos );
                    if( pFnd )
                        
                        ((_HashStr*)pFnd)->aSetStr = pSFld->GetExpStr();
                    else
                        
                        *(pHashStrTbl + nPos ) = pFnd = new _HashStr( aNew,
                                        pSFld->GetExpStr(),
                                        (_HashStr*)*(pHashStrTbl + nPos) );

                    
                    SwSbxValue aValue;
                    aValue.PutString( ((_HashStr*)pFnd)->aSetStr );
                    aCalc.VarChange( aNew, aValue );
                }
            }
            else            
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

                    aNew += "=";
                    aNew += pSFld->GetFormula();

                    SwSbxValue aValue = aCalc.Calculate( aNew );
                    double nErg = aValue.GetDouble();
                    
                    if( !aValue.IsVoidValue() && (!pUpdtFld || pUpdtFld == pTxtFld) )
                    {
                        pSFld->SetValue( nErg );

                        if( pSeqNd )
                            pSFldTyp->SetChapter( *pSFld, *pSeqNd );
                    }
                }
            }
        }
        } 

        pFmtFld->ModifyNotification( 0, 0 );        

        if( pUpdtFld == pTxtFld )       
        {
            if( RES_GETEXPFLD == nWhich ||      
                RES_HIDDENTXTFLD == nWhich ||   
                RES_HIDDENPARAFLD == nWhich)    
                break;                          
            pUpdtFld = 0;                       
        }
    }

    pMgr->CloseAll(sal_False);
    
    ::DeleteHashTable( pHashStrTbl, nStrFmtCnt );

    
    if( bUpdRefFlds )
        UpdateRefFlds(NULL);

    mpUpdtFlds->SetInUpdateFlds( bOldInUpdateFlds );
    mpUpdtFlds->SetFieldsDirty( false );
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

void SwDoc::_InitFieldTypes()       
{
    
    mpFldTypes->push_back( new SwDateTimeFieldType(this) );
    mpFldTypes->push_back( new SwChapterFieldType );
    mpFldTypes->push_back( new SwPageNumberFieldType );
    mpFldTypes->push_back( new SwAuthorFieldType );
    mpFldTypes->push_back( new SwFileNameFieldType(this) );
    mpFldTypes->push_back( new SwDBNameFieldType(this) );
    mpFldTypes->push_back( new SwGetExpFieldType(this) );
    mpFldTypes->push_back( new SwGetRefFieldType( this ) );
    mpFldTypes->push_back( new SwHiddenTxtFieldType );
    mpFldTypes->push_back( new SwPostItFieldType(this) );
    mpFldTypes->push_back( new SwDocStatFieldType(this) );
    mpFldTypes->push_back( new SwDocInfoFieldType(this) );
    mpFldTypes->push_back( new SwInputFieldType( this ) );
    mpFldTypes->push_back( new SwTblFieldType( this ) );
    mpFldTypes->push_back( new SwMacroFieldType(this) );
    mpFldTypes->push_back( new SwHiddenParaFieldType );
    mpFldTypes->push_back( new SwDBNextSetFieldType );
    mpFldTypes->push_back( new SwDBNumSetFieldType );
    mpFldTypes->push_back( new SwDBSetNumberFieldType );
    mpFldTypes->push_back( new SwTemplNameFieldType(this) );
    mpFldTypes->push_back( new SwTemplNameFieldType(this) );
    mpFldTypes->push_back( new SwExtUserFieldType );
    mpFldTypes->push_back( new SwRefPageSetFieldType );
    mpFldTypes->push_back( new SwRefPageGetFieldType( this ) );
    mpFldTypes->push_back( new SwJumpEditFieldType( this ) );
    mpFldTypes->push_back( new SwScriptFieldType( this ) );
    mpFldTypes->push_back( new SwCombinedCharFieldType );
    mpFldTypes->push_back( new SwDropDownFieldType );

    
    
    
    
    mpFldTypes->push_back( new SwSetExpFieldType(this,
                SW_RESSTR(STR_POOLCOLL_LABEL_ABB), nsSwGetSetExpType::GSE_SEQ) );
    mpFldTypes->push_back( new SwSetExpFieldType(this,
                SW_RESSTR(STR_POOLCOLL_LABEL_TABLE), nsSwGetSetExpType::GSE_SEQ) );
    mpFldTypes->push_back( new SwSetExpFieldType(this,
                SW_RESSTR(STR_POOLCOLL_LABEL_FRAME), nsSwGetSetExpType::GSE_SEQ) );
    mpFldTypes->push_back( new SwSetExpFieldType(this,
                SW_RESSTR(STR_POOLCOLL_LABEL_DRAWING), nsSwGetSetExpType::GSE_SEQ) );

    OSL_ENSURE( mpFldTypes->size() == INIT_FLDTYPES, "Bad initsize: SwFldTypes" );
}

void SwDoc::InsDelFldInFldLst( bool bIns, const SwTxtFld& rFld )
{
    if( !mbNewFldLst || !IsInDtor() )
        mpUpdtFlds->InsDelFldInFldLst( bIns, rFld );
}

SwDBData SwDoc::GetDBData()
{
    return GetDBDesc();
}

const SwDBData& SwDoc::GetDBDesc()
{
    if(maDBData.sDataSource.isEmpty())
    {
        const sal_uInt16 nSize = mpFldTypes->size();
        for(sal_uInt16 i = 0; i < nSize && maDBData.sDataSource.isEmpty(); ++i)
        {
            SwFieldType& rFldType = *((*mpFldTypes)[i]);
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
        maDBData = GetNewDBMgr()->GetAddressDBName();
    return maDBData;
}

void SwDoc::SetInitDBFields( sal_Bool b )
{
    GetNewDBMgr()->SetInitDBFields( b );
}


static OUString lcl_DBDataToString(const SwDBData& rData)
{
    OUString sRet = rData.sDataSource;
    sRet += OUString(DB_DELIM);
    sRet += rData.sCommand;
    sRet += OUString(DB_DELIM);
    sRet += OUString::number(rData.nCommandType);
    return sRet;
}

void SwDoc::GetAllUsedDB( std::vector<OUString>& rDBNameList,
                          const std::vector<OUString>* pAllDBNames )
{
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

        const SwField* pFld = pFmtFld->GetField();
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

void SwDoc::GetAllDBNames( std::vector<OUString>& rAllDBNames )
{
    SwNewDBMgr* pMgr = GetNewDBMgr();

    const SwDSParamArr& rArr = pMgr->GetDSParamArray();
    for(sal_uInt16 i = 0; i < rArr.size(); i++)
    {
        const SwDSParam* pParam = &rArr[i];
        OUStringBuffer sStr(pParam->sDataSource.getLength() + pParam->sCommand.getLength() + 2);
        sStr.append(pParam->sDataSource );
        sStr.append(DB_DELIM);
        sStr.append(pParam->sCommand);
        rAllDBNames.push_back(sStr.makeStringAndClear());
    }
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
    GetNewDBMgr()->CreateDSData(aData);
    rDBNameList.push_back(rDBName);
}

void SwDoc::ChangeDBFields( const std::vector<OUString>& rOldNames,
                            const OUString& rNewName )
{
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

        SwFmtFld* pFmtFld = (SwFmtFld*)pItem;
        SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
        if( !pTxtFld || !pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
            continue;

        SwField* pFld = pFmtFld->GetField();
        bool bExpand = false;

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

                    bExpand = true;
                }
                break;

            case RES_DBSETNUMBERFLD:
            case RES_DBNAMEFLD:
                if( IsNameInArray( rOldNames,
                                lcl_DBDataToString(((SwDBNameInfField*)pFld)->GetRealDBData())))
                {
                    ((SwDBNameInfField*)pFld)->SetDBData(aNewDBData);
                    bExpand = true;
                }
                break;

            case RES_DBNUMSETFLD:
            case RES_DBNEXTSETFLD:
                if( IsNameInArray( rOldNames,
                                lcl_DBDataToString(((SwDBNameInfField*)pFld)->GetRealDBData())))
                {
                    ((SwDBNameInfField*)pFld)->SetDBData(aNewDBData);
                    bExpand = true;
                }
                
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
            pTxtFld->ExpandAlways();
    }
    SetModified();
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

void SwDoc::SetFixFields( bool bOnlyTimeDate, const DateTime* pNewDateTime )
{
    sal_Bool bIsModified = IsModified();

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
        nTime = Time( Time::SYSTEM ).GetTime();
    }

    sal_uInt16 aTypes[5] = {
        /*0*/   RES_DOCINFOFLD,
        /*1*/   RES_AUTHORFLD,
        /*2*/   RES_EXTUSERFLD,
        /*3*/   RES_FILENAMEFLD,
        /*4*/   RES_DATETIMEFLD };  

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
                    if( ((SwDocInfoField*)pFmtFld->GetField())->IsFixed() )
                    {
                        bChgd = true;
                        SwDocInfoField* pDocInfFld = (SwDocInfoField*)pFmtFld->GetField();
                        pDocInfFld->SetExpansion( ((SwDocInfoFieldType*)
                                    pDocInfFld->GetTyp())->Expand(
                                        pDocInfFld->GetSubType(),
                                        pDocInfFld->GetFormat(),
                                        pDocInfFld->GetLanguage(),
                                        pDocInfFld->GetName() ) );
                    }
                    break;

                case RES_AUTHORFLD:
                    if( ((SwAuthorField*)pFmtFld->GetField())->IsFixed() )
                    {
                        bChgd = true;
                        SwAuthorField* pAuthorFld = (SwAuthorField*)pFmtFld->GetField();
                        pAuthorFld->SetExpansion( ((SwAuthorFieldType*)
                                    pAuthorFld->GetTyp())->Expand(
                                                pAuthorFld->GetFormat() ) );
                    }
                    break;

                case RES_EXTUSERFLD:
                    if( ((SwExtUserField*)pFmtFld->GetField())->IsFixed() )
                    {
                        bChgd = true;
                        SwExtUserField* pExtUserFld = (SwExtUserField*)pFmtFld->GetField();
                        pExtUserFld->SetExpansion( ((SwExtUserFieldType*)
                                    pExtUserFld->GetTyp())->Expand(
                                            pExtUserFld->GetSubType(),
                                            pExtUserFld->GetFormat()));
                    }
                    break;

                case RES_DATETIMEFLD:
                    if( ((SwDateTimeField*)pFmtFld->GetField())->IsFixed() )
                    {
                        bChgd = true;
                        ((SwDateTimeField*)pFmtFld->GetField())->SetDateTime(
                                                    DateTime(Date(nDate), Time(nTime)) );
                    }
                    break;

                case RES_FILENAMEFLD:
                    if( ((SwFileNameField*)pFmtFld->GetField())->IsFixed() )
                    {
                        bChgd = true;
                        SwFileNameField* pFileNameFld =
                            (SwFileNameField*)pFmtFld->GetField();
                        pFileNameFld->SetExpansion( ((SwFileNameFieldType*)
                                    pFileNameFld->GetTyp())->Expand(
                                            pFileNameFld->GetFormat() ) );
                    }
                    break;
                }

                
                if( bChgd )
                    pFmtFld->ModifyNotification( 0, 0 );
            }
        }
    }

    if( !bIsModified )
        ResetModified();
}

bool SwDoc::SetFieldsDirty( bool b, const SwNode* pChk, sal_uLong nLen )
{
    
    
    bool bFldsFnd = false;
    if( b && pChk && !GetUpdtFlds().IsFieldsDirty() && !IsInDtor()
        
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
                    
                    b = true;
                else if( pTNd->GetpSwpHints() && pTNd->GetSwpHints().Count() )
                    for( sal_uInt16 n = 0, nEnd = pTNd->GetSwpHints().Count();
                            n < nEnd; ++n )
                    {
                        const SwTxtAttr* pAttr = pTNd->GetSwpHints()[ n ];
                        if ( pAttr->Which() == RES_TXTATR_FIELD )
                        {
                            b = true;
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
    const sal_uInt16 nSize = mpFldTypes->size();

    for( sal_uInt16 i = INIT_FLDTYPES; i < nSize; ++i )
    {
        SwFieldType* pFldType = (*mpFldTypes)[i];
        if( RES_AUTHORITY  == pFldType->Which() )
        {
            SwAuthorityFieldType* pAuthType = (SwAuthorityFieldType*)pFldType;
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
        break;          

    default:
        return;
    }

    SetFieldsDirty( true );
    if( !pFldSortLst )
    {
        if( !bIns )             
            return;             
        pFldSortLst = new _SetGetExpFlds;
    }

    if( bIns )      
        GetBodyNode( rFld, nWhich );
    else
    {
        
        
        for( sal_uInt16 n = 0; n < pFldSortLst->size(); ++n )
            if( &rFld == (*pFldSortLst)[ n ]->GetPointer() )
            {
                delete (*pFldSortLst)[n];
                pFldSortLst->erase(n);
                n--; 
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
    
    delete pFldSortLst;
    pFldSortLst = new _SetGetExpFlds;

    
    
    
    
    
    
    
    
    
    
    
    
    if ( eGetMode == GETFLD_ALL )
    
    
    {
        
        
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

        
        for (sal_uInt16 n = 0; n < aTmpArr.size(); ++n)
        {
            GetBodyNode( *rDoc.GetNodes()[ aTmpArr[ n ] ]->GetSectionNode() );
        }
    }

    const OUString sTrue("TRUE");
    const OUString sFalse("FALSE");

    bool bIsDBMgr = 0 != rDoc.GetNewDBMgr();
    sal_uInt16 nWhich, n;
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
                        ((SwHiddenParaField*)pFld)->SetHidden( sal_False );
                    else if (sFormula==sTrue)
                        ((SwHiddenParaField*)pFld)->SetHidden( sal_True );
                    else
                        break;

                    sFormula = OUString();
                    
                    ((SwFmtFld*)pFmtFld)->ModifyNotification( 0, 0 );
                }
                break;

            case RES_HIDDENTXTFLD:
                if( GETFLD_ALL == eGetMode )
                {
                    sFormula = pFld->GetPar1();
                    if (sFormula.isEmpty() || sFormula==sFalse)
                        ((SwHiddenTxtField*)pFld)->SetValue( sal_True );
                    else if (sFormula==sTrue)
                        ((SwHiddenTxtField*)pFld)->SetValue( sal_False );
                    else
                        break;

                    sFormula = OUString();

                    
                    ((SwHiddenTxtField*)pFld)->Evaluate(&rDoc);
                    
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
                    sFormula = pFld->GetPar1();
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
                    sFormula = pFld->GetPar1();
                }
            }
            break;
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

    
    Point aPt;
    const SwCntntFrm* pFrm = rTxtNd.getLayoutFrm( rDoc.GetCurrentLayout(), &aPt, 0, sal_False );

    _SetGetExpFld* pNew = NULL;
    sal_Bool bIsInBody = sal_False;

    if( !pFrm || pFrm->IsInDocBody() )
    {
        
        SwNodeIndex aIdx( rTxtNd );
        bIsInBody = rDoc.GetNodes().GetEndOfExtras().GetIndex() < aIdx.GetIndex();

        
        
        
        
        if( (pFrm != NULL) || bIsInBody )
            pNew = new _SetGetExpFld( aIdx, &rTFld );
    }
    else
    {
        
        SwPosition aPos( rDoc.GetNodes().GetEndOfPostIts() );
        bool const bResult = GetBodyTxtNode( rDoc, aPos, *pFrm );
        OSL_ENSURE(bResult, "where is the Field");
        (void) bResult; 
        pNew = new _SetGetExpFld( aPos.nNode, &rTFld, &aPos.nContent );
    }

    
    if( RES_GETEXPFLD == nFldWhich )
    {
        SwGetExpField* pGetFld = (SwGetExpField*)rTFld.GetFmtFld().GetField();
        pGetFld->ChgBodyTxtFlag( bIsInBody );
    }
    else if( RES_DBFLD == nFldWhich )
    {
        SwDBField* pDBFld = (SwDBField*)rTFld.GetFmtFld().GetField();
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
        do {            

            
            
            SwPosition aPos( rSectNd );
            SwCntntNode* pCNd = rDoc.GetNodes().GoNext( &aPos.nNode ); 

            if( !pCNd || !pCNd->IsTxtNode() )
                break;

            
            Point aPt;
            const SwCntntFrm* pFrm = pCNd->getLayoutFrm( rDoc.GetCurrentLayout(), &aPt, 0, sal_False );
            if( !pFrm )
                break;

            bool const bResult = GetBodyTxtNode( rDoc, aPos, *pFrm );
            OSL_ENSURE(bResult, "where is the Field");
            (void) bResult; 
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
        sFldName = ((SwUserFieldType&)rType).GetName();
        break;
    case RES_SETEXPFLD:
        sFldName = ((SwSetExpFieldType&)rType).GetName();
        break;
    default:
        OSL_ENSURE( !this, "kein gueltiger FeldTyp" );
    }

    if( !sFldName.isEmpty() )
    {
        SetFieldsDirty( true );
        
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
        sFldName = ((SwUserFieldType&)rType).GetName();
        break;
    case RES_SETEXPFLD:
        sFldName = ((SwSetExpFieldType&)rType).GetName();
        break;
    }

    if( !sFldName.isEmpty() )
    {
        SetFieldsDirty( true );
        
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

bool SwDoc::UpdateFld(SwTxtFld * pDstTxtFld, SwField & rSrcFld,
                      SwMsgPoolItem * pMsgHnt,
                      bool bUpdateFlds)
{
    OSL_ENSURE(pDstTxtFld, "no field to update!");

    bool bTblSelBreak = false;

    SwFmtFld * pDstFmtFld = (SwFmtFld*)&pDstTxtFld->GetFmtFld();
    SwField * pDstFld = pDstFmtFld->GetField();
    sal_uInt16 nFldWhich = rSrcFld.GetTyp()->Which();
    SwNodeIndex aTblNdIdx(pDstTxtFld->GetTxtNode());

    if (pDstFld->GetTyp()->Which() ==
        rSrcFld.GetTyp()->Which())
    {
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            SwPosition aPosition( pDstTxtFld->GetTxtNode() );
            aPosition.nContent = *pDstTxtFld->GetStart();

            SwUndo *const pUndo( new SwUndoFieldFromDoc( aPosition, *pDstFld, rSrcFld, pMsgHnt, bUpdateFlds) );
            GetIDocumentUndoRedo().AppendUndo(pUndo);
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
            ChgDBData(((SwDBNameInfField*) pNewFld)->GetRealDBData());
            pNewFld->GetTyp()->UpdateFlds();

            break;

        case RES_DBFLD:
            {
                
                
                SwDBField* pDBFld = (SwDBField*)pNewFld;
                if (pDBFld->IsInitialized())
                    pDBFld->ChgValue( pDBFld->GetValue(), true );

                pDBFld->ClearInitialized();
                pDBFld->InitContent();
            }
            

        default:
            pDstFmtFld->ModifyNotification( 0, pMsgHnt );
        }

        
        
        if( nFldWhich == RES_USERFLD )
            UpdateUsrFlds();
    }

    return bTblSelBreak;
}

bool SwDoc::PutValueToField(const SwPosition & rPos,
                            const Any& rVal, sal_uInt16 nWhich)
{
    Any aOldVal;
    SwField * pField = GetFieldAtPos(rPos);


    if (GetIDocumentUndoRedo().DoesUndo() &&
        pField->QueryValue(aOldVal, nWhich))
    {
        SwUndo *const pUndo(new SwUndoFieldFromAPI(rPos, aOldVal, rVal, nWhich));
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    return pField->PutValue(rVal, nWhich);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

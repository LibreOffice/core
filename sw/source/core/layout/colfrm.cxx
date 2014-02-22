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

#include <editeng/ulspitem.hxx>
#include <fmtclds.hxx>
#include <fmtfordr.hxx>
#include <frmfmt.hxx>
#include "frmtool.hxx"
#include "colfrm.hxx"
#include "pagefrm.hxx"
#include "bodyfrm.hxx"
#include "rootfrm.hxx"
#include "sectfrm.hxx"
#include "switerator.hxx"


void sw_RemoveFtns( SwFtnBossFrm* pBoss, sal_Bool bPageOnly, sal_Bool bEndNotes );

SwColumnFrm::SwColumnFrm( SwFrmFmt *pFmt, SwFrm* pSib ):
    SwFtnBossFrm( pFmt, pSib )
{
    mnType = FRMC_COLUMN;
    SwBodyFrm* pColBody = new SwBodyFrm( pFmt->GetDoc()->GetDfltFrmFmt(), pSib );
    pColBody->InsertBehind( this, 0 ); 
    SetMaxFtnHeight( LONG_MAX );
}

SwColumnFrm::~SwColumnFrm()
{
    SwFrmFmt *pFmt = GetFmt();
    SwDoc *pDoc;
    if ( !(pDoc = pFmt->GetDoc())->IsInDtor() && pFmt->IsLastDepend() )
    {
        
        
        pDoc->GetDfltFrmFmt()->Add( this );
        pDoc->DelFrmFmt( pFmt );
    }
}

static void lcl_RemoveColumns( SwLayoutFrm *pCont, sal_uInt16 nCnt )
{
    OSL_ENSURE( pCont && pCont->Lower() && pCont->Lower()->IsColumnFrm(),
            "no columns to remove." );

    SwColumnFrm *pColumn = (SwColumnFrm*)pCont->Lower();
    sw_RemoveFtns( pColumn, sal_True, sal_True );
    while ( pColumn->GetNext() )
    {
        OSL_ENSURE( pColumn->GetNext()->IsColumnFrm(),
                "neighbor of ColumnFrame is no ColumnFrame." );
        pColumn = (SwColumnFrm*)pColumn->GetNext();
    }
    for ( sal_uInt16 i = 0; i < nCnt; ++i )
    {
        SwColumnFrm *pTmp = (SwColumnFrm*)pColumn->GetPrev();
        pColumn->Cut();
        delete pColumn; 
        pColumn = pTmp;
    }
}

static SwLayoutFrm * lcl_FindColumns( SwLayoutFrm *pLay, sal_uInt16 nCount )
{
    SwFrm *pCol = pLay->Lower();
    if ( pLay->IsPageFrm() )
        pCol = ((SwPageFrm*)pLay)->FindBodyCont()->Lower();

    if ( pCol && pCol->IsColumnFrm() )
    {
        SwFrm *pTmp = pCol;
        sal_uInt16 i;
        for ( i = 0; pTmp; pTmp = pTmp->GetNext(), ++i )
            /* do nothing */;
        return i == nCount ? (SwLayoutFrm*)pCol : 0;
    }
    return 0;
}

static sal_Bool lcl_AddColumns( SwLayoutFrm *pCont, sal_uInt16 nCount )
{
    SwDoc *pDoc = pCont->GetFmt()->GetDoc();
    const sal_Bool bMod = pDoc->IsModified();

    
    
    
    
    SwLayoutFrm *pAttrOwner = pCont;
    if ( pCont->IsBodyFrm() )
        pAttrOwner = pCont->FindPageFrm();
    SwLayoutFrm *pNeighbourCol = 0;
    SwIterator<SwLayoutFrm,SwFmt> aIter( *pAttrOwner->GetFmt() );
    SwLayoutFrm *pNeighbour = aIter.First();

    sal_uInt16 nAdd = 0;
    SwFrm *pCol = pCont->Lower();
    if ( pCol && pCol->IsColumnFrm() )
        for ( nAdd = 1; pCol; pCol = pCol->GetNext(), ++nAdd )
            /* do nothing */;
    while ( pNeighbour )
    {
        if ( 0 != (pNeighbourCol = lcl_FindColumns( pNeighbour, nCount+nAdd )) &&
             pNeighbourCol != pCont )
            break;
        pNeighbourCol = 0;
        pNeighbour = aIter.Next();
    }

    sal_Bool bRet;
    SwTwips nMax = pCont->IsPageBodyFrm() ?
                   pCont->FindPageFrm()->GetMaxFtnHeight() : LONG_MAX;
    if ( pNeighbourCol )
    {
        bRet = sal_False;
        SwFrm *pTmp = pCont->Lower();
        while ( pTmp )
        {
            pTmp = pTmp->GetNext();
            pNeighbourCol = (SwLayoutFrm*)pNeighbourCol->GetNext();
        }
        for ( sal_uInt16 i = 0; i < nCount; ++i )
        {
            SwColumnFrm *pTmpCol = new SwColumnFrm( pNeighbourCol->GetFmt(), pCont );
            pTmpCol->SetMaxFtnHeight( nMax );
            pTmpCol->InsertBefore( pCont, NULL );
            pNeighbourCol = (SwLayoutFrm*)pNeighbourCol->GetNext();
        }
    }
    else
    {
        bRet = sal_True;
        for ( sal_uInt16 i = 0; i < nCount; ++i )
        {
            SwFrmFmt *pFmt = pDoc->MakeFrmFmt( aEmptyOUStr, pDoc->GetDfltFrmFmt());
            SwColumnFrm *pTmp = new SwColumnFrm( pFmt, pCont );
            pTmp->SetMaxFtnHeight( nMax );
            pTmp->Paste( pCont );
        }
    }

    if ( !bMod )
        pDoc->ResetModified();
    return bRet;
}

/** add or remove columns from a layoutframe.
 *
 * Normally, a layoutframe with a column attribut of 1 or 0 columns contains
 * no columnframe. However, a sectionframe with "footnotes at the end" needs
 * a columnframe.
 *
 * @param rOld
 * @param rNew
 * @param bChgFtn if true, the columnframe will be inserted or removed, if necessary.
 */
void SwLayoutFrm::ChgColumns( const SwFmtCol &rOld, const SwFmtCol &rNew,
    const sal_Bool bChgFtn )
{
    if ( rOld.GetNumCols() <= 1 && rNew.GetNumCols() <= 1 && !bChgFtn )
        return;
    
    
    if ( Lower() && Lower()->IsNoTxtFrm() &&
         rNew.GetNumCols() > 1 )
    {
        return;
    }

    sal_uInt16 nNewNum, nOldNum = 1;
    if( Lower() && Lower()->IsColumnFrm() )
    {
        SwFrm* pCol = Lower();
        while( 0 != (pCol=pCol->GetNext()) )
            ++nOldNum;
    }
    nNewNum = rNew.GetNumCols();
    if( !nNewNum )
        ++nNewNum;
    sal_Bool bAtEnd;
    if( IsSctFrm() )
        bAtEnd = ((SwSectionFrm*)this)->IsAnyNoteAtEnd();
    else
        bAtEnd = sal_False;

    
    sal_Bool bAdjustAttributes = nOldNum != rOld.GetNumCols();

    
    SwFrm *pSave = 0;
    if( nOldNum != nNewNum || bChgFtn )
    {
        SwDoc *pDoc = GetFmt()->GetDoc();
        OSL_ENSURE( pDoc, "FrmFmt doesn't return a document." );
        
        
        if( IsPageBodyFrm() )
            pDoc->GetCurrentLayout()->RemoveFtns( (SwPageFrm*)GetUpper(), sal_True, sal_False );
        pSave = ::SaveCntnt( this );

        
        if ( nNewNum == 1 && !bAtEnd )
        {
            ::lcl_RemoveColumns( this, nOldNum );
            if ( IsBodyFrm() )
                SetFrmFmt( pDoc->GetDfltFrmFmt() );
            else
                GetFmt()->SetFmtAttr( SwFmtFillOrder() );
            if ( pSave )
                ::RestoreCntnt( pSave, this, 0, true );
            return;
        }
        if ( nOldNum == 1 )
        {
            if ( IsBodyFrm() )
                SetFrmFmt( pDoc->GetColumnContFmt() );
            else
                GetFmt()->SetFmtAttr( SwFmtFillOrder( ATT_LEFT_TO_RIGHT ) );
            if( !Lower() || !Lower()->IsColumnFrm() )
                --nOldNum;
        }
        if ( nOldNum > nNewNum )
        {
            ::lcl_RemoveColumns( this, nOldNum - nNewNum );
            bAdjustAttributes = sal_True;
        }
        else if( nOldNum < nNewNum )
        {
            sal_uInt16 nAdd = nNewNum - nOldNum;
            bAdjustAttributes = lcl_AddColumns( this, nAdd );
        }
    }

    if ( !bAdjustAttributes )
    {
        if ( rOld.GetLineWidth()    != rNew.GetLineWidth() ||
             rOld.GetWishWidth()    != rNew.GetWishWidth() ||
             rOld.IsOrtho()         != rNew.IsOrtho() )
            bAdjustAttributes = sal_True;
        else
        {
            sal_uInt16 nCount = std::min( rNew.GetColumns().size(), rOld.GetColumns().size() );
            for ( sal_uInt16 i = 0; i < nCount; ++i )
                if ( !(rOld.GetColumns()[i] == rNew.GetColumns()[i]) )
                {
                    bAdjustAttributes = sal_True;
                    break;
                }
        }
    }

    
    AdjustColumns( &rNew, bAdjustAttributes );

    
    
    if ( pSave )
    {
        OSL_ENSURE( Lower() && Lower()->IsLayoutFrm() &&
                ((SwLayoutFrm*)Lower())->Lower() &&
                ((SwLayoutFrm*)Lower())->Lower()->IsLayoutFrm(),
                "no column body." );   
        ::RestoreCntnt( pSave, (SwLayoutFrm*)((SwLayoutFrm*)Lower())->Lower(), 0, true );
    }
}

void SwLayoutFrm::AdjustColumns( const SwFmtCol *pAttr, sal_Bool bAdjustAttributes )
{
    if( !Lower()->GetNext() )
    {
        Lower()->ChgSize( Prt().SSize() );
        return;
    }

    const sal_Bool bVert = IsVertical();
    
    SwRectFn fnRect = bVert ? ( IsVertLR() ? fnRectVertL2R : fnRectVert ) : fnRectHori;

    
    
    if ( !pAttr )
    {
        pAttr = &GetFmt()->GetCol();
        if ( !bAdjustAttributes )
        {
            long nAvail = (Prt().*fnRect->fnGetWidth)();
            for ( SwLayoutFrm *pCol = (SwLayoutFrm*)Lower();
                  pCol;
                  pCol = (SwLayoutFrm*)pCol->GetNext() )
                nAvail -= (pCol->Frm().*fnRect->fnGetWidth)();
            if ( !nAvail )
                return;
        }
    }

    
    
    SwTwips nAvail = (Prt().*fnRect->fnGetWidth)();
    const sal_Bool bLine = pAttr->GetLineAdj() != COLADJ_NONE;
    const sal_uInt16 nMin = bLine ? sal_uInt16( 20 + ( pAttr->GetLineWidth() / 2) ) : 0;

    const sal_Bool bR2L = IsRightToLeft();
    SwFrm *pCol = bR2L ? GetLastLower() : Lower();

    
    
    
    const sal_Bool bOrtho = pAttr->IsOrtho() && pAttr->GetNumCols() > 0;
    long nGutter = 0;

    for ( sal_uInt16 i = 0; i < pAttr->GetNumCols() && pCol; ++i ) 
    {
        if( !bOrtho )
        {
            const SwTwips nWidth = i == (pAttr->GetNumCols() - 1) ?
                                   nAvail :
                                   pAttr->CalcColWidth( i, sal_uInt16( (Prt().*fnRect->fnGetWidth)() ) );

            const Size aColSz = bVert ?
                                Size( Prt().Width(), nWidth ) :
                                Size( nWidth, Prt().Height() );

            pCol->ChgSize( aColSz );

            
            
            
            
            if( IsBodyFrm() )
                ((SwLayoutFrm*)pCol)->Lower()->ChgSize( aColSz );

            nAvail -= nWidth;
        }

        if ( bOrtho || bAdjustAttributes )
        {
            const SwColumn *pC = &pAttr->GetColumns()[i];
            const SwAttrSet* pSet = pCol->GetAttrSet();
            SvxLRSpaceItem aLR( pSet->GetLRSpace() );

            
            
            
            
            const sal_uInt16 nLeft = pC->GetLeft();
            const sal_uInt16 nRight = pC->GetRight();

            aLR.SetLeft ( nLeft );
            aLR.SetRight( nRight );

            if ( bLine )
            {
                if ( i == 0 )
                {
                    aLR.SetRight( std::max( nRight, nMin ) );
                }
                else if ( i == pAttr->GetNumCols() - 1 )
                {
                    aLR.SetLeft ( std::max( nLeft, nMin ) );
                }
                else
                {
                    aLR.SetLeft ( std::max( nLeft,  nMin ) );
                    aLR.SetRight( std::max( nRight, nMin ) );
                }
            }

            if ( bAdjustAttributes )
            {
                SvxULSpaceItem aUL( pSet->GetULSpace() );
                aUL.SetUpper( pC->GetUpper());
                aUL.SetLower( pC->GetLower());

                ((SwLayoutFrm*)pCol)->GetFmt()->SetFmtAttr( aLR );
                ((SwLayoutFrm*)pCol)->GetFmt()->SetFmtAttr( aUL );
            }

            nGutter += aLR.GetLeft() + aLR.GetRight();
        }

        pCol = bR2L ? pCol->GetPrev() : pCol->GetNext();
    }

    if( bOrtho )
    {
        long nInnerWidth = ( nAvail - nGutter ) / pAttr->GetNumCols();
        pCol = Lower();
        for( sal_uInt16 i = 0; i < pAttr->GetNumCols() && pCol; pCol = pCol->GetNext(), ++i ) 
        {
            SwTwips nWidth;
            if ( i == pAttr->GetNumCols() - 1 )
                nWidth = nAvail;
            else
            {
                SvxLRSpaceItem aLR( pCol->GetAttrSet()->GetLRSpace() );
                nWidth = nInnerWidth + aLR.GetLeft() + aLR.GetRight();
            }
            if( nWidth < 0 )
                nWidth = 0;

            const Size aColSz = bVert ?
                                Size( Prt().Width(), nWidth ) :
                                Size( nWidth, Prt().Height() );

            pCol->ChgSize( aColSz );

            if( IsBodyFrm() )
                ((SwLayoutFrm*)pCol)->Lower()->ChgSize( aColSz );

            nAvail -= nWidth;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

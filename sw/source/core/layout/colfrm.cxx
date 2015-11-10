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
#include <calbck.hxx>
#include "ftnfrm.hxx"
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>

SwColumnFrm::SwColumnFrm( SwFrameFormat *pFormat, SwFrm* pSib ):
    SwFootnoteBossFrm( pFormat, pSib )
{
    mnFrmType = FRM_COLUMN;
    SwBodyFrm* pColBody = new SwBodyFrm( pFormat->GetDoc()->GetDfltFrameFormat(), pSib );
    pColBody->InsertBehind( this, nullptr ); // ColumnFrms now with BodyFrm
    SetMaxFootnoteHeight( LONG_MAX );
}

void SwColumnFrm::DestroyImpl()
{
    SwFrameFormat *pFormat = GetFormat();
    SwDoc *pDoc;
    if ( !(pDoc = pFormat->GetDoc())->IsInDtor() && pFormat->HasOnlyOneListener() )
    {
        //I'm the only one, delete the format.
        //Get default format before, so the base class can cope with it.
        pDoc->GetDfltFrameFormat()->Add( this );
        pDoc->DelFrameFormat( pFormat );
    }

    SwFootnoteBossFrm::DestroyImpl();
}

SwColumnFrm::~SwColumnFrm()
{
}

static void lcl_RemoveColumns( SwLayoutFrm *pCont, sal_uInt16 nCnt )
{
    OSL_ENSURE( pCont && pCont->Lower() && pCont->Lower()->IsColumnFrm(),
            "no columns to remove." );

    SwColumnFrm *pColumn = static_cast<SwColumnFrm*>(pCont->Lower());
    sw_RemoveFootnotes( pColumn, true, true );
    while ( pColumn->GetNext() )
    {
        OSL_ENSURE( pColumn->GetNext()->IsColumnFrm(),
                "neighbor of ColumnFrame is no ColumnFrame." );
        pColumn = static_cast<SwColumnFrm*>(pColumn->GetNext());
    }
    for ( sal_uInt16 i = 0; i < nCnt; ++i )
    {
        SwColumnFrm *pTmp = static_cast<SwColumnFrm*>(pColumn->GetPrev());
        pColumn->Cut();
        SwFrm::DestroyFrm(pColumn); //format is going to be destroyed in the DTor if needed.
        pColumn = pTmp;
    }
}

static SwLayoutFrm * lcl_FindColumns( SwLayoutFrm *pLay, sal_uInt16 nCount )
{
    SwFrm *pCol = pLay->Lower();
    if ( pLay->IsPageFrm() )
        pCol = static_cast<SwPageFrm*>(pLay)->FindBodyCont()->Lower();

    if ( pCol && pCol->IsColumnFrm() )
    {
        SwFrm *pTmp = pCol;
        sal_uInt16 i;
        for ( i = 0; pTmp; pTmp = pTmp->GetNext(), ++i )
            /* do nothing */;
        return i == nCount ? static_cast<SwLayoutFrm*>(pCol) : nullptr;
    }
    return nullptr;
}

static bool lcl_AddColumns( SwLayoutFrm *pCont, sal_uInt16 nCount )
{
    SwDoc *pDoc = pCont->GetFormat()->GetDoc();
    const bool bMod = pDoc->getIDocumentState().IsModified();

    //Formats should be shared whenever possible. If a neighbour already has
    //the same column settings we can add them to the same format.
    //The neighbour can be searched using the format, however the owner of the
    //attribute depends on the frame type.
    SwLayoutFrm *pAttrOwner = pCont;
    if ( pCont->IsBodyFrm() )
        pAttrOwner = pCont->FindPageFrm();
    SwLayoutFrm *pNeighbourCol = nullptr;
    SwIterator<SwLayoutFrm,SwFormat> aIter( *pAttrOwner->GetFormat() );
    SwLayoutFrm *pNeighbour = aIter.First();

    sal_uInt16 nAdd = 0;
    SwFrm *pCol = pCont->Lower();
    if ( pCol && pCol->IsColumnFrm() )
        for ( nAdd = 1; pCol; pCol = pCol->GetNext(), ++nAdd )
            /* do nothing */;
    while ( pNeighbour )
    {
        if ( nullptr != (pNeighbourCol = lcl_FindColumns( pNeighbour, nCount+nAdd )) &&
             pNeighbourCol != pCont )
            break;
        pNeighbourCol = nullptr;
        pNeighbour = aIter.Next();
    }

    bool bRet;
    SwTwips nMax = pCont->IsPageBodyFrm() ?
                   pCont->FindPageFrm()->GetMaxFootnoteHeight() : LONG_MAX;
    if ( pNeighbourCol )
    {
        bRet = false;
        SwFrm *pTmp = pCont->Lower();
        while ( pTmp )
        {
            pTmp = pTmp->GetNext();
            pNeighbourCol = static_cast<SwLayoutFrm*>(pNeighbourCol->GetNext());
        }
        for ( sal_uInt16 i = 0; i < nCount; ++i )
        {
            SwColumnFrm *pTmpCol = new SwColumnFrm( pNeighbourCol->GetFormat(), pCont );
            pTmpCol->SetMaxFootnoteHeight( nMax );
            pTmpCol->InsertBefore( pCont, nullptr );
            pNeighbourCol = static_cast<SwLayoutFrm*>(pNeighbourCol->GetNext());
        }
    }
    else
    {
        bRet = true;
        for ( sal_uInt16 i = 0; i < nCount; ++i )
        {
            SwFrameFormat *pFormat = pDoc->MakeFrameFormat( aEmptyOUStr, pDoc->GetDfltFrameFormat());
            SwColumnFrm *pTmp = new SwColumnFrm( pFormat, pCont );
            pTmp->SetMaxFootnoteHeight( nMax );
            pTmp->Paste( pCont );
        }
    }

    if ( !bMod )
        pDoc->getIDocumentState().ResetModified();
    return bRet;
}

/** add or remove columns from a layoutframe.
 *
 * Normally, a layoutframe with a column attribute of 1 or 0 columns contains
 * no columnframe. However, a sectionframe with "footnotes at the end" needs
 * a columnframe.
 *
 * @param rOld
 * @param rNew
 * @param bChgFootnote if true, the columnframe will be inserted or removed, if necessary.
 */
void SwLayoutFrm::ChgColumns( const SwFormatCol &rOld, const SwFormatCol &rNew,
    const bool bChgFootnote )
{
    if ( rOld.GetNumCols() <= 1 && rNew.GetNumCols() <= 1 && !bChgFootnote )
        return;
    // #i97379#
    // If current lower is a no text frame, then columns are not allowed
    if ( Lower() && Lower()->IsNoTextFrm() &&
         rNew.GetNumCols() > 1 )
    {
        return;
    }

    sal_uInt16 nNewNum, nOldNum = 1;
    if( Lower() && Lower()->IsColumnFrm() )
    {
        SwFrm* pCol = Lower();
        while( nullptr != (pCol=pCol->GetNext()) )
            ++nOldNum;
    }
    nNewNum = rNew.GetNumCols();
    if( !nNewNum )
        ++nNewNum;
    bool bAtEnd;
    if( IsSctFrm() )
        bAtEnd = static_cast<SwSectionFrm*>(this)->IsAnyNoteAtEnd();
    else
        bAtEnd = false;

    //Setting the column width is only needed for new formats.
    bool bAdjustAttributes = nOldNum != rOld.GetNumCols();

    //The content is saved and restored if the column count is different.
    SwFrm *pSave = nullptr;
    if( nOldNum != nNewNum || bChgFootnote )
    {
        SwDoc *pDoc = GetFormat()->GetDoc();
        OSL_ENSURE( pDoc, "FrameFormat doesn't return a document." );
        // SaveContent would also suck up the content of the footnote container
        // and store it within the normal text flow.
        if( IsPageBodyFrm() )
            pDoc->getIDocumentLayoutAccess().GetCurrentLayout()->RemoveFootnotes( static_cast<SwPageFrm*>(GetUpper()) );
        pSave = ::SaveContent( this );

        //If columns exist, they get deleted if a column count of 0 or 1 is requested.
        if ( nNewNum == 1 && !bAtEnd )
        {
            ::lcl_RemoveColumns( this, nOldNum );
            if ( IsBodyFrm() )
                SetFrameFormat( pDoc->GetDfltFrameFormat() );
            else
                GetFormat()->SetFormatAttr( SwFormatFillOrder() );
            if ( pSave )
                ::RestoreContent( pSave, this, nullptr, true );
            return;
        }
        if ( nOldNum == 1 )
        {
            if ( IsBodyFrm() )
                SetFrameFormat( pDoc->GetColumnContFormat() );
            else
                GetFormat()->SetFormatAttr( SwFormatFillOrder( ATT_LEFT_TO_RIGHT ) );
            if( !Lower() || !Lower()->IsColumnFrm() )
                --nOldNum;
        }
        if ( nOldNum > nNewNum )
        {
            ::lcl_RemoveColumns( this, nOldNum - nNewNum );
            bAdjustAttributes = true;
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
            bAdjustAttributes = true;
        else
        {
            const size_t nCount = std::min( rNew.GetColumns().size(), rOld.GetColumns().size() );
            for ( size_t i = 0; i < nCount; ++i )
                if ( !(rOld.GetColumns()[i] == rNew.GetColumns()[i]) )
                {
                    bAdjustAttributes = true;
                    break;
                }
        }
    }

    //The columns can now be easily adjusted.
    AdjustColumns( &rNew, bAdjustAttributes );

    //Don't restore the content before. An earlier restore would trigger useless
    //actions during setup.
    if ( pSave )
    {
        OSL_ENSURE( Lower() && Lower()->IsLayoutFrm() &&
                static_cast<SwLayoutFrm*>(Lower())->Lower() &&
                static_cast<SwLayoutFrm*>(Lower())->Lower()->IsLayoutFrm(),
                "no column body." );   // ColumnFrms contain BodyFrms
        ::RestoreContent( pSave, static_cast<SwLayoutFrm*>(static_cast<SwLayoutFrm*>(Lower())->Lower()), nullptr, true );
    }
}

void SwLayoutFrm::AdjustColumns( const SwFormatCol *pAttr, bool bAdjustAttributes )
{
    if( !Lower()->GetNext() )
    {
        Lower()->ChgSize( Prt().SSize() );
        return;
    }

    const bool bVert = IsVertical();

    SwRectFn fnRect = bVert ? ( IsVertLR() ? fnRectVertL2R : fnRectVert ) : fnRectHori;

    //If we have a pointer or we have to configure an attribute, we set the
    //column widths in any case. Otherwise we check if a configuration is needed.
    if ( !pAttr )
    {
        pAttr = &GetFormat()->GetCol();
        if ( !bAdjustAttributes )
        {
            long nAvail = (Prt().*fnRect->fnGetWidth)();
            for ( SwLayoutFrm *pCol = static_cast<SwLayoutFrm*>(Lower());
                  pCol;
                  pCol = static_cast<SwLayoutFrm*>(pCol->GetNext()) )
                nAvail -= (pCol->Frm().*fnRect->fnGetWidth)();
            if ( !nAvail )
                return;
        }
    }

    //The columns can now be easily adjusted.
    //The widths get counted so we can give the reminder to the last one.
    SwTwips nAvail = (Prt().*fnRect->fnGetWidth)();
    const bool bLine = pAttr->GetLineAdj() != COLADJ_NONE;
    const sal_uInt16 nMin = bLine ? sal_uInt16( 20 + ( pAttr->GetLineWidth() / 2) ) : 0;

    const bool bR2L = IsRightToLeft();
    SwFrm *pCol = bR2L ? GetLastLower() : Lower();

    // #i27399#
    // bOrtho means we have to adjust the column frames manually. Otherwise
    // we may use the values returned by CalcColWidth:
    const bool bOrtho = pAttr->IsOrtho() && pAttr->GetNumCols() > 0;
    long nGutter = 0;

    for ( sal_uInt16 i = 0; i < pAttr->GetNumCols() && pCol; ++i ) //i118878, value returned by GetNumCols() can't be trusted
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

            // With this, the ColumnBodyFrms from page columns gets adjusted and
            // their bFixHeight flag is set so they won't shrink/grow.
            // Don't use the flag with frame columns because BodyFrms in frame
            // columns can grow/shrink.
            if( IsBodyFrm() )
                static_cast<SwLayoutFrm*>(pCol)->Lower()->ChgSize( aColSz );

            nAvail -= nWidth;
        }

        if ( bOrtho || bAdjustAttributes )
        {
            const SwColumn *pC = &pAttr->GetColumns()[i];
            const SwAttrSet* pSet = pCol->GetAttrSet();
            SvxLRSpaceItem aLR( pSet->GetLRSpace() );

            //In order to have enough space for the separation lines, we have to
            //take them into account here. Every time two columns meet we
            //calculate a clearance of 20 + half the pen width on the left or
            //right side, respectively.
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

                static_cast<SwLayoutFrm*>(pCol)->GetFormat()->SetFormatAttr( aLR );
                static_cast<SwLayoutFrm*>(pCol)->GetFormat()->SetFormatAttr( aUL );
            }

            nGutter += aLR.GetLeft() + aLR.GetRight();
        }

        pCol = bR2L ? pCol->GetPrev() : pCol->GetNext();
    }

    if( bOrtho )
    {
        long nInnerWidth = ( nAvail - nGutter ) / pAttr->GetNumCols();
        pCol = Lower();
        for( sal_uInt16 i = 0; i < pAttr->GetNumCols() && pCol; pCol = pCol->GetNext(), ++i ) //i118878, value returned by GetNumCols() can't be trusted
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
                static_cast<SwLayoutFrm*>(pCol)->Lower()->ChgSize( aColSz );

            nAvail -= nWidth;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

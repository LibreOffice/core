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
#include "cntfrm.hxx"
#include "doc.hxx"

#include "hintids.hxx"
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <fmtclds.hxx>
#include <fmtfordr.hxx>
#include <frmfmt.hxx>
#include <node.hxx>
#include "frmtool.hxx"
#include "colfrm.hxx"
#include "pagefrm.hxx"
#include "bodyfrm.hxx"   // ColumnFrms now with BodyFrm
#include "rootfrm.hxx"   // wg. RemoveFtns
#include "sectfrm.hxx"   // wg. FtnAtEnd-Flag
#include "switerator.hxx"

// ftnfrm.cxx:
void sw_RemoveFtns( SwFtnBossFrm* pBoss, sal_Bool bPageOnly, sal_Bool bEndNotes );


/*************************************************************************
|*
|*  SwColumnFrm::SwColumnFrm()
|*
|*************************************************************************/
SwColumnFrm::SwColumnFrm( SwFrmFmt *pFmt, SwFrm* pSib ):
    SwFtnBossFrm( pFmt, pSib )
{
    nType = FRMC_COLUMN;
    SwBodyFrm* pColBody = new SwBodyFrm( pFmt->GetDoc()->GetDfltFrmFmt(), pSib );
    pColBody->InsertBehind( this, 0 ); // ColumnFrms now with BodyFrm
    SetMaxFtnHeight( LONG_MAX );
}

SwColumnFrm::~SwColumnFrm()
{
    SwFrmFmt *pFmt = GetFmt();
    SwDoc *pDoc;
    if ( !(pDoc = pFmt->GetDoc())->IsInDtor() && pFmt->IsLastDepend() )
    {
        //I'm the only one, delete the format.
        //Get default format before, so the base class can cope with it.
        pDoc->GetDfltFrmFmt()->Add( this );
        pDoc->DelFrmFmt( pFmt );
    }
}

/*************************************************************************
|*
|*  SwLayoutFrm::ChgColumns()
|*
|*************************************************************************/

static void lcl_RemoveColumns( SwLayoutFrm *pCont, sal_uInt16 nCnt )
{
    OSL_ENSURE( pCont && pCont->Lower() && pCont->Lower()->IsColumnFrm(),
            "Keine Spalten zu entfernen." );

    SwColumnFrm *pColumn = (SwColumnFrm*)pCont->Lower();
    sw_RemoveFtns( pColumn, sal_True, sal_True );
    while ( pColumn->GetNext() )
    {
        OSL_ENSURE( pColumn->GetNext()->IsColumnFrm(),
                "Nachbar von ColFrm kein ColFrm." );
        pColumn = (SwColumnFrm*)pColumn->GetNext();
    }
    for ( sal_uInt16 i = 0; i < nCnt; ++i )
    {
        SwColumnFrm *pTmp = (SwColumnFrm*)pColumn->GetPrev();
        pColumn->Cut();
        delete pColumn; //format is going to be destroyed in the DTor if needed.
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

    //Formats should be shared whenever possible. If a neighbour already has
    //the same column settings we can add them to the same format.
    //The neighbour can be searched using the format, however the owner of the
    //attribute depends on the frame type.
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
            SwFrmFmt *pFmt = pDoc->MakeFrmFmt( aEmptyStr, pDoc->GetDfltFrmFmt());
            SwColumnFrm *pTmp = new SwColumnFrm( pFmt, pCont );
            pTmp->SetMaxFtnHeight( nMax );
            pTmp->Paste( pCont );
        }
    }

    if ( !bMod )
        pDoc->ResetModified();
    return bRet;
}

/*--------------------------------------------------
 * ChgColumns() adds or removes columns from a layoutframe.
 * Normally, a layoutframe with a column attribut of 1 or 0 columns contains
 * no columnframe. However, a sectionframe with "footnotes at the end" needs
 * a columnframe. If the bChgFtn-flag is set, the columnframe will be inserted
 * or remove, if necessary.
 * --------------------------------------------------*/

void SwLayoutFrm::ChgColumns( const SwFmtCol &rOld, const SwFmtCol &rNew,
    const sal_Bool bChgFtn )
{
    if ( rOld.GetNumCols() <= 1 && rNew.GetNumCols() <= 1 && !bChgFtn )
        return;
    // #i97379#
    // If current lower is a no text frame, then columns are not allowed
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

    //Setting the column width is only needed for new formats.
    sal_Bool bAdjustAttributes = nOldNum != rOld.GetNumCols();

    //The content is saved and restored if the column count is different.
    SwFrm *pSave = 0;
    if( nOldNum != nNewNum || bChgFtn )
    {
        SwDoc *pDoc = GetFmt()->GetDoc();
        OSL_ENSURE( pDoc, "FrmFmt doesn't return a document." );
        // SaveCntnt would also suck up the content of the footnote container
        // and store it within the normal text flow.
        if( IsPageBodyFrm() )
            pDoc->GetCurrentLayout()->RemoveFtns( (SwPageFrm*)GetUpper(), sal_True, sal_False );    //swmod 080218
        pSave = ::SaveCntnt( this );

        //If columns exist, they get deleted if a column count of 0 or 1 is requested.
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
            sal_uInt16 nCount = Min( rNew.GetColumns().size(), rOld.GetColumns().size() );
            for ( sal_uInt16 i = 0; i < nCount; ++i )
                if ( !(rOld.GetColumns()[i] == rNew.GetColumns()[i]) )
                {
                    bAdjustAttributes = sal_True;
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
                ((SwLayoutFrm*)Lower())->Lower() &&
                ((SwLayoutFrm*)Lower())->Lower()->IsLayoutFrm(),
                "Gesucht: Spaltenbody (Tod oder Lebend)." );   // ColumnFrms jetzt mit BodyFrm
        ::RestoreCntnt( pSave, (SwLayoutFrm*)((SwLayoutFrm*)Lower())->Lower(), 0, true );
    }
}

/*************************************************************************
|*
|*  SwLayoutFrm::AdjustColumns()
|*
|*************************************************************************/

void SwLayoutFrm::AdjustColumns( const SwFmtCol *pAttr, sal_Bool bAdjustAttributes )
{
    if( !Lower()->GetNext() )
    {
        Lower()->ChgSize( Prt().SSize() );
        return;
    }

    const sal_Bool bVert = IsVertical();
    //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
    SwRectFn fnRect = bVert ? ( IsVertLR() ? fnRectVertL2R : fnRectVert ) : fnRectHori;

    //If we have a pointer or we have to configure an attribute, we set the
    //column widths in any case. Otherwise we check if a configuration is needed.
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

    //The columns can now be easily adjusted.
    //The widths get counted so we can give the reminder to the last one.
    SwTwips nAvail = (Prt().*fnRect->fnGetWidth)();
    const sal_Bool bLine = pAttr->GetLineAdj() != COLADJ_NONE;
    const sal_uInt16 nMin = bLine ? sal_uInt16( 20 + ( pAttr->GetLineWidth() / 2) ) : 0;

    // #i27399#
    // bOrtho means we have to adjust the column frames manually. Otherwise
    // we may use the values returned by CalcColWidth:
    const sal_Bool bOrtho = pAttr->IsOrtho() && pAttr->GetNumCols() > 0;
    long nGutter = 0;
    sal_uInt16 real_nb_col = 0;

    SwFrm* pColHead = Lower();;
    SwFrm* pColTail;
    for ( pColTail = pColHead; pColTail; pColTail = pColTail->GetNext(), real_nb_col += 1 );

    sal_uInt16 i = IsRightToLeft() ? real_nb_col : 0;

    for ( SwFrm* pColCursor = IsRightToLeft() ? pColTail : pColHead;
          pColCursor;
          (pColCursor = IsRightToLeft() ? pColCursor->GetPrev() : pColCursor->GetNext()), (i += IsRightToLeft() ? -1 : +1) )
    {
        if( !bOrtho )
        {
            const SwTwips nWidth = (pColCursor == (IsRightToLeft() ? pColTail : pColHead) ) ?
                                   nAvail :
                                   pAttr->CalcColWidth( i, sal_uInt16( (Prt().*fnRect->fnGetWidth)() ) );

            const Size aColSz = bVert ?
                                Size( Prt().Width(), nWidth ) :
                                Size( nWidth, Prt().Height() );

            pColCursor->ChgSize( aColSz );

            // With this, the ColumnBodyFrms from page columns gets adjusted and
            // their bFixHeight flag is set so they won't shrink/grow.
            // Don't use the flag with frame columns because BodyFrms in frame
            // columns can grow/shrink.
            if( IsBodyFrm() )
                ((SwLayoutFrm*)pColCursor)->Lower()->ChgSize( aColSz );

            nAvail -= nWidth;
        }

        if ( bOrtho || bAdjustAttributes )
        {
            const SwColumn *pC = &pAttr->GetColumns()[i];
            const SwAttrSet* pSet = pColCursor->GetAttrSet();
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
                    aLR.SetRight( Max( nRight, nMin ) );
                }
                else if ( i == pAttr->GetNumCols() - 1 )
                {
                    aLR.SetLeft ( Max( nLeft, nMin ) );
                }
                else
                {
                    aLR.SetLeft ( Max( nLeft,  nMin ) );
                    aLR.SetRight( Max( nRight, nMin ) );
                }
            }

            if ( bAdjustAttributes )
            {
                SvxULSpaceItem aUL( pSet->GetULSpace() );
                aUL.SetUpper( pC->GetUpper());
                aUL.SetLower( pC->GetLower());

                ((SwLayoutFrm*)pColCursor)->GetFmt()->SetFmtAttr( aLR );
                ((SwLayoutFrm*)pColCursor)->GetFmt()->SetFmtAttr( aUL );
            }

            nGutter += aLR.GetLeft() + aLR.GetRight();
        }
    }

    if( bOrtho )
    {
        long nInnerWidth = ( nAvail - nGutter ) / pAttr->GetNumCols();
        i = 0;
        for (SwFrm* pColCursor =  pColHead; pColCursor; pColCursor = pColCursor->GetNext(), i++)
        {
            SwTwips nWidth;
            if ( i == pAttr->GetNumCols() - 1 )
            {
                nWidth = nAvail;
            }
            else
            {
                SvxLRSpaceItem aLR( pColCursor->GetAttrSet()->GetLRSpace() );
                nWidth = nInnerWidth + aLR.GetLeft() + aLR.GetRight();
            }
            if( nWidth < 0 )
            {
                nWidth = 0;
            }
            const Size aColSz = bVert ?
                                Size( Prt().Width(), nWidth ) :
                                Size( nWidth, Prt().Height() );

            pColCursor->ChgSize( aColSz );

            if( IsBodyFrm() )
            {
                ((SwLayoutFrm*)pColCursor)->Lower()->ChgSize( aColSz );
            }
            nAvail -= nWidth;
        }
    }
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

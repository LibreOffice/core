/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: outleeng.cxx,v $
 * $Revision: 1.19 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <outl_pch.hxx>

#define _OUTLINER_CXX
#include <svx/outliner.hxx>
#include <outleeng.hxx>
#include <paralist.hxx>
#include <outliner.hrc>
#include <svtools/itemset.hxx>
#include <svx/eeitem.hxx>
#include "editstat.hxx"
#include "outlundo.hxx"

OutlinerEditEng::OutlinerEditEng( Outliner* pEngOwner, SfxItemPool* pPool )
 : EditEngine( pPool )
{
    pOwner = pEngOwner;
}

OutlinerEditEng::~OutlinerEditEng()
{
}

void OutlinerEditEng::PaintingFirstLine( USHORT nPara, const Point& rStartPos, long nBaseLineY, const Point& rOrigin, short nOrientation, OutputDevice* pOutDev )
{
    if( GetControlWord() && EE_CNTRL_OUTLINER )
    {
        PaintFirstLineInfo aInfo( nPara, rStartPos, nBaseLineY, rOrigin, nOrientation, pOutDev );
        pOwner->maPaintFirstLineHdl.Call( &aInfo );
    }

    pOwner->PaintBullet( nPara, rStartPos, rOrigin, nOrientation, pOutDev );
}

const SvxNumberFormat* OutlinerEditEng::GetNumberFormat( USHORT nPara ) const
{
    const SvxNumberFormat* pFmt = NULL;
    if (pOwner)
        pFmt = pOwner->GetNumberFormat( nPara );
    return pFmt;
}


Rectangle OutlinerEditEng::GetBulletArea( USHORT nPara )
{
    Rectangle aBulletArea = Rectangle( Point(), Point() );
    if ( nPara < pOwner->pParaList->GetParagraphCount() )
    {
        if ( pOwner->ImplHasBullet( nPara ) )
            aBulletArea = pOwner->ImpCalcBulletArea( nPara, FALSE, FALSE );
    }
    return aBulletArea;
}

void OutlinerEditEng::ParagraphInserted( USHORT nNewParagraph )
{
    pOwner->ParagraphInserted( nNewParagraph );

    EditEngine::ParagraphInserted( nNewParagraph );
}

void OutlinerEditEng::ParagraphDeleted( USHORT nDeletedParagraph )
{
    pOwner->ParagraphDeleted( nDeletedParagraph );

    EditEngine::ParagraphDeleted( nDeletedParagraph );
}

void OutlinerEditEng::ParagraphConnected( USHORT /*nLeftParagraph*/, USHORT nRightParagraph )
{
    if( pOwner && pOwner->IsUndoEnabled() && !const_cast<EditEngine&>(pOwner->GetEditEngine()).IsInUndo() )
    {
        Paragraph* pPara = pOwner->GetParagraph( nRightParagraph );
        if( pPara && pOwner->HasParaFlag( pPara, PARAFLAG_ISPAGE ) )
        {
            pOwner->InsertUndo( new OutlinerUndoChangeParaFlags( pOwner, nRightParagraph, PARAFLAG_ISPAGE, 0 ) );
        }
    }
}


void OutlinerEditEng::StyleSheetChanged( SfxStyleSheet* pStyle )
{
    pOwner->StyleSheetChanged( pStyle );
}

void OutlinerEditEng::ParaAttribsChanged( USHORT nPara )
{
    pOwner->ParaAttribsChanged( nPara );
}

BOOL OutlinerEditEng::SpellNextDocument()
{
    return pOwner->SpellNextDocument();
}

BOOL OutlinerEditEng::ConvertNextDocument()
{
    return pOwner->ConvertNextDocument();
}

XubString OutlinerEditEng::GetUndoComment( USHORT nUndoId ) const
{
    switch( nUndoId )
    {
        case OLUNDO_DEPTH:
            return XubString( EditResId( RID_OUTLUNDO_DEPTH ));

        case OLUNDO_EXPAND:
            return XubString( EditResId( RID_OUTLUNDO_EXPAND ));

        case OLUNDO_COLLAPSE:
            return XubString( EditResId( RID_OUTLUNDO_COLLAPSE ));

        case OLUNDO_ATTR:
            return XubString( EditResId( RID_OUTLUNDO_ATTR ));

        case OLUNDO_INSERT:
            return XubString( EditResId( RID_OUTLUNDO_INSERT ));

        default:
            return EditEngine::GetUndoComment( nUndoId );
    }
}

// #101498#
void OutlinerEditEng::DrawingText( const Point& rStartPos, const XubString& rText, USHORT nTextStart, USHORT nTextLen,
    const sal_Int32* pDXArray, const SvxFont& rFont, USHORT nPara, USHORT nIndex, BYTE nRightToLeft,
    const EEngineData::WrongSpellVector* pWrongSpellVector,
    const SvxFieldData* pFieldData,
    bool bEndOfLine,
    bool bEndOfParagraph,
    bool bEndOfBullet,
    const ::com::sun::star::lang::Locale* pLocale,
    const Color& rOverlineColor,
    const Color& rTextLineColor)
{
    // why do bullet here at all? Just use GetEditEnginePtr()->PaintingFirstLine
    // inside of ImpEditEngine::Paint which calls pOwner->PaintBullet with the correct
    // values for hor and ver. No change for not-layouting (painting).
    // changed, bullet rendering now using PaintBullet via
/*  if ( nIndex == 0 )
    {
        // Dann das Bullet 'malen', dort wird bStrippingPortions ausgewertet
        // und Outliner::DrawingText gerufen

        // DrawingText liefert die BaseLine, DrawBullet braucht Top().

        if(true)
        {
            // ##
            // another error: This call happens when only stripping, but the position
            // is already aligned to text output. For bullet rendering, it needs to be reset
            // to the correct value in x and y. PaintBullet takes care of X-start offset itself
            const Point aDocPosTopLeft(GetDocPosTopLeft( nPara ));
            const Point aCorrectedPos(rStartPos.X() - aDocPosTopLeft.X(), aDocPosTopLeft.Y() + GetFirstLineOffset( nPara ));
            pOwner->PaintBullet( nPara, aCorrectedPos, Point(), 0, GetRefDevice() );
        }
        else
        {
            Point aCorrectedPos( rStartPos );
            aCorrectedPos.Y() = GetDocPosTopLeft( nPara ).Y();
            aCorrectedPos.Y() += GetFirstLineOffset( nPara );
            pOwner->PaintBullet( nPara, aCorrectedPos, Point(), 0, GetRefDevice() );
        }
    } */

    // #101498#
    pOwner->DrawingText(rStartPos,rText,nTextStart,nTextLen,pDXArray,rFont,nPara,nIndex,nRightToLeft,
        pWrongSpellVector, pFieldData, bEndOfLine, bEndOfParagraph, bEndOfBullet, pLocale, rOverlineColor, rTextLineColor);
}

void OutlinerEditEng::FieldClicked( const SvxFieldItem& rField, USHORT nPara, USHORT nPos )
{
    EditEngine::FieldClicked( rField, nPara, nPos );    // Falls URL
    pOwner->FieldClicked( rField, nPara, nPos );
}

void OutlinerEditEng::FieldSelected( const SvxFieldItem& rField, USHORT nPara, USHORT nPos )
{
    pOwner->FieldSelected( rField, nPara, nPos );
}

XubString OutlinerEditEng::CalcFieldValue( const SvxFieldItem& rField, USHORT nPara, USHORT nPos, Color*& rpTxtColor, Color*& rpFldColor )
{
    return pOwner->CalcFieldValue( rField, nPara, nPos, rpTxtColor, rpFldColor );
}

void OutlinerEditEng::SetParaAttribs( USHORT nPara, const SfxItemSet& rSet )
{
    Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
    if( pPara )
    {
        if ( !IsInUndo() && IsUndoEnabled() )
            pOwner->UndoActionStart( OLUNDO_ATTR );

        EditEngine::SetParaAttribs( (USHORT)nPara, rSet );

        pOwner->ImplCheckNumBulletItem( (USHORT)nPara );
        pOwner->ImplCheckParagraphs( (USHORT)nPara, (USHORT) (pOwner->pParaList->GetParagraphCount()-1) );

        if ( !IsInUndo() && IsUndoEnabled() )
            pOwner->UndoActionEnd( OLUNDO_ATTR );
    }
}


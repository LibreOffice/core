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

#include <svl/intitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/editdata.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/fhgtitem.hxx>

#include <editeng/outliner.hxx>
#include <outleeng.hxx>
#include <paralist.hxx>
#include <editeng/editrids.hrc>
#include <svl/itemset.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editstat.hxx>
#include "outlundo.hxx"

OutlinerEditEng::OutlinerEditEng( Outliner* pEngOwner, SfxItemPool* pPool )
 : EditEngine( pPool )
{
    pOwner = pEngOwner;
}

OutlinerEditEng::~OutlinerEditEng()
{
}

void OutlinerEditEng::PaintingFirstLine( sal_Int32 nPara, const Point& rStartPos, long nBaseLineY, const Point& rOrigin, short nOrientation, OutputDevice* pOutDev )
{
    if( GetControlWord() & EE_CNTRL_OUTLINER )
    {
        PaintFirstLineInfo aInfo( nPara, rStartPos, nBaseLineY, rOrigin, nOrientation, pOutDev );
        pOwner->maPaintFirstLineHdl.Call( &aInfo );
    }

    pOwner->PaintBullet( nPara, rStartPos, rOrigin, nOrientation, pOutDev );
}

const SvxNumberFormat* OutlinerEditEng::GetNumberFormat( sal_Int32 nPara ) const
{
    const SvxNumberFormat* pFmt = NULL;
    if (pOwner)
        pFmt = pOwner->GetNumberFormat( nPara );
    return pFmt;
}


Rectangle OutlinerEditEng::GetBulletArea( sal_Int32 nPara )
{
    Rectangle aBulletArea = Rectangle( Point(), Point() );
    if ( nPara < pOwner->pParaList->GetParagraphCount() )
    {
        if ( pOwner->ImplHasNumberFormat( nPara ) )
            aBulletArea = pOwner->ImpCalcBulletArea( nPara, sal_False, sal_False );
    }
    return aBulletArea;
}

void OutlinerEditEng::ParagraphInserted( sal_Int32 nNewParagraph )
{
    pOwner->ParagraphInserted( nNewParagraph );

    EditEngine::ParagraphInserted( nNewParagraph );
}

void OutlinerEditEng::ParagraphDeleted( sal_Int32 nDeletedParagraph )
{
    pOwner->ParagraphDeleted( nDeletedParagraph );

    EditEngine::ParagraphDeleted( nDeletedParagraph );
}

void OutlinerEditEng::ParagraphConnected( sal_Int32 /*nLeftParagraph*/, sal_Int32 nRightParagraph )
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

void OutlinerEditEng::ParaAttribsChanged( sal_Int32 nPara )
{
    pOwner->ParaAttribsChanged( nPara );
}

sal_Bool OutlinerEditEng::SpellNextDocument()
{
    return pOwner->SpellNextDocument();
}

sal_Bool OutlinerEditEng::ConvertNextDocument()
{
    return pOwner->ConvertNextDocument();
}

OUString OutlinerEditEng::GetUndoComment( sal_uInt16 nUndoId ) const
{
    switch( nUndoId )
    {
        case OLUNDO_DEPTH:
            return EE_RESSTR(RID_OUTLUNDO_DEPTH);

        case OLUNDO_EXPAND:
            return EE_RESSTR(RID_OUTLUNDO_EXPAND);

        case OLUNDO_COLLAPSE:
            return EE_RESSTR(RID_OUTLUNDO_COLLAPSE);

        case OLUNDO_ATTR:
            return EE_RESSTR(RID_OUTLUNDO_ATTR);

        case OLUNDO_INSERT:
            return EE_RESSTR(RID_OUTLUNDO_INSERT);

        default:
            return EditEngine::GetUndoComment( nUndoId );
    }
}

void OutlinerEditEng::DrawingText( const Point& rStartPos, const OUString& rText, sal_uInt16 nTextStart, sal_uInt16 nTextLen,
    const sal_Int32* pDXArray, const SvxFont& rFont, sal_Int32 nPara, sal_uInt16 nIndex, sal_uInt8 nRightToLeft,
    const EEngineData::WrongSpellVector* pWrongSpellVector,
    const SvxFieldData* pFieldData,
    bool bEndOfLine,
    bool bEndOfParagraph,
    bool bEndOfBullet,
    const ::com::sun::star::lang::Locale* pLocale,
    const Color& rOverlineColor,
    const Color& rTextLineColor)
{
    pOwner->DrawingText(rStartPos,rText,nTextStart,nTextLen,pDXArray,rFont,nPara,nIndex,nRightToLeft,
        pWrongSpellVector, pFieldData, bEndOfLine, bEndOfParagraph, bEndOfBullet, pLocale, rOverlineColor, rTextLineColor);
}

void OutlinerEditEng::DrawingTab( const Point& rStartPos, long nWidth, const String& rChar,
    const SvxFont& rFont, sal_Int32 nPara, xub_StrLen nIndex, sal_uInt8 nRightToLeft,
    bool bEndOfLine, bool bEndOfParagraph,
    const Color& rOverlineColor, const Color& rTextLineColor)
{
    pOwner->DrawingTab(rStartPos, nWidth, rChar, rFont, nPara, nIndex, nRightToLeft,
            bEndOfLine, bEndOfParagraph, rOverlineColor, rTextLineColor );
}

void OutlinerEditEng::FieldClicked( const SvxFieldItem& rField, sal_Int32 nPara, sal_uInt16 nPos )
{
    EditEngine::FieldClicked( rField, nPara, nPos );    // If URL
    pOwner->FieldClicked( rField, nPara, nPos );
}

void OutlinerEditEng::FieldSelected( const SvxFieldItem& rField, sal_Int32 nPara, sal_uInt16 nPos )
{
    pOwner->FieldSelected( rField, nPara, nPos );
}

OUString OutlinerEditEng::CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_uInt16 nPos, Color*& rpTxtColor, Color*& rpFldColor )
{
    return pOwner->CalcFieldValue( rField, nPara, nPos, rpTxtColor, rpFldColor );
}

void OutlinerEditEng::SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet )
{
    Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
    if( pPara )
    {
        if ( !IsInUndo() && IsUndoEnabled() )
            pOwner->UndoActionStart( OLUNDO_ATTR );

        EditEngine::SetParaAttribs( nPara, rSet );

        pOwner->ImplCheckNumBulletItem( nPara );
        // #i100014#
        // It is not a good idea to substract 1 from a count and cast the result
        // to sal_uInt16 without check, if the count is 0.
        pOwner->ImplCheckParagraphs( nPara, pOwner->pParaList->GetParagraphCount() );

        if ( !IsInUndo() && IsUndoEnabled() )
            pOwner->UndoActionEnd( OLUNDO_ATTR );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

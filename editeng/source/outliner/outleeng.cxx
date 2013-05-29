/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_editeng.hxx"
#include <svl/intitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/editdata.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/fhgtitem.hxx>

#define _OUTLINER_CXX
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

void OutlinerEditEng::PaintingFirstLine( sal_uInt16 nPara, const Point& rStartPos, long nBaseLineY, const Point& rOrigin, short nOrientation, OutputDevice* pOutDev )
{
    if( GetControlWord() & EE_CNTRL_OUTLINER )
    {
        PaintFirstLineInfo aInfo( nPara, rStartPos, nBaseLineY, rOrigin, nOrientation, pOutDev );
        pOwner->maPaintFirstLineHdl.Call( &aInfo );
    }

    pOwner->PaintBullet( nPara, rStartPos, rOrigin, nOrientation, pOutDev );
}

const SvxNumberFormat* OutlinerEditEng::GetNumberFormat( sal_uInt16 nPara ) const
{
    const SvxNumberFormat* pFmt = NULL;
    if (pOwner)
        pFmt = pOwner->GetNumberFormat( nPara );
    return pFmt;
}


Rectangle OutlinerEditEng::GetBulletArea( sal_uInt16 nPara )
{
    Rectangle aBulletArea = Rectangle( Point(), Point() );
    if ( nPara < pOwner->pParaList->GetParagraphCount() )
    {
        if ( pOwner->ImplHasNumberFormat( nPara ) )
            aBulletArea = pOwner->ImpCalcBulletArea( nPara, sal_False, sal_False );
    }
    return aBulletArea;
}

void OutlinerEditEng::ParagraphInserted( sal_uInt16 nNewParagraph )
{
    pOwner->ParagraphInserted( nNewParagraph );

    EditEngine::ParagraphInserted( nNewParagraph );
}

void OutlinerEditEng::ParagraphDeleted( sal_uInt16 nDeletedParagraph )
{
    pOwner->ParagraphDeleted( nDeletedParagraph );

    EditEngine::ParagraphDeleted( nDeletedParagraph );
}

void OutlinerEditEng::ParagraphConnected( sal_uInt16 /*nLeftParagraph*/, sal_uInt16 nRightParagraph )
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

void OutlinerEditEng::ParaAttribsChanged( sal_uInt16 nPara )
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

XubString OutlinerEditEng::GetUndoComment( sal_uInt16 nUndoId ) const
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
void OutlinerEditEng::DrawingText( const Point& rStartPos, const XubString& rText, sal_uInt16 nTextStart, sal_uInt16 nTextLen,
    const sal_Int32* pDXArray, const SvxFont& rFont, sal_uInt16 nPara, sal_uInt16 nIndex, sal_uInt8 nRightToLeft,
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

void OutlinerEditEng::FieldClicked( const SvxFieldItem& rField, sal_uInt16 nPara, sal_uInt16 nPos )
{
    EditEngine::FieldClicked( rField, nPara, nPos );    // Falls URL
    pOwner->FieldClicked( rField, nPara, nPos );
}

void OutlinerEditEng::FieldSelected( const SvxFieldItem& rField, sal_uInt16 nPara, sal_uInt16 nPos )
{
    pOwner->FieldSelected( rField, nPara, nPos );
}

XubString OutlinerEditEng::CalcFieldValue( const SvxFieldItem& rField, sal_uInt16 nPara, sal_uInt16 nPos, Color*& rpTxtColor, Color*& rpFldColor )
{
    return pOwner->CalcFieldValue( rField, nPara, nPos, rpTxtColor, rpFldColor );
}

void OutlinerEditEng::SetParaAttribs( sal_uInt16 nPara, const SfxItemSet& rSet )
{
    Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
    if( pPara )
    {
        if ( !IsInUndo() && IsUndoEnabled() )
            pOwner->UndoActionStart( OLUNDO_ATTR );

        EditEngine::SetParaAttribs( (sal_uInt16)nPara, rSet );

        pOwner->ImplCheckNumBulletItem( (sal_uInt16)nPara );
        // --> OD 2009-03-10 #i100014#
        // It is not a good idea to substract 1 from a count and cast the result
        // to sal_uInt16 without check, if the count is 0.
        pOwner->ImplCheckParagraphs( (sal_uInt16)nPara, (sal_uInt16) (pOwner->pParaList->GetParagraphCount()) );
        // <--

        if ( !IsInUndo() && IsUndoEnabled() )
            pOwner->UndoActionEnd( OLUNDO_ATTR );
    }
}


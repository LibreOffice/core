/*************************************************************************
 *
 *  $RCSfile: editdbg.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mt $ $Date: 2000-11-07 18:25:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <eeng_pch.hxx>

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#pragma hdrstop

#include <lspcitem.hxx>

#include "lrspitem.hxx"
#include "ulspitem.hxx"
#include "cntritem.hxx"
#include "colritem.hxx"
#include "fhgtitem.hxx"
#include "fontitem.hxx"
#include "adjitem.hxx"
#include "fwdtitem.hxx"
#include "wghtitem.hxx"
#include "postitem.hxx"
#include "udlnitem.hxx"
#include "crsditem.hxx"
#include "shdditem.hxx"
#include "escpitem.hxx"
#include "kernitem.hxx"
#include "wrlmitem.hxx"
#include "akrnitem.hxx"
#include "langitem.hxx"

#include <impedit.hxx>
#include <editeng.hxx>
#include <editview.hxx>
#include <editdoc.hxx>
#include <editdbg.hxx>

#if defined( DBG_UTIL ) || defined( DEBUG )

ByteString DbgOutItem( const SfxItemPool& rPool, const SfxPoolItem& rItem )
{
    ByteString aDebStr;
    switch ( rItem.Which() )
    {
        case EE_PARA_OUTLLRSPACE:
        case EE_PARA_LRSPACE:
            aDebStr += "FI=";
            aDebStr += ByteString::CreateFromInt32( ((SvxLRSpaceItem&)rItem).GetTxtFirstLineOfst() );
            aDebStr += ", LI=";
            aDebStr += ByteString::CreateFromInt32( ((SvxLRSpaceItem&)rItem).GetTxtLeft() );
            aDebStr += ", RI=";
            aDebStr += ByteString::CreateFromInt32( ((SvxLRSpaceItem&)rItem).GetRight() );
        break;
        case EE_PARA_NUMBULLET:
            aDebStr += "NumItem=...";
        break;
        case EE_PARA_BULLETSTATE:
            aDebStr += "ShowBullet=";
            aDebStr += ByteString::CreateFromInt32( ((SfxUInt16Item&)rItem).GetValue() );
        break;
        case EE_PARA_HYPHENATE:
            aDebStr += "Hyphenate=";
            aDebStr += ByteString::CreateFromInt32( ((SfxBoolItem&)rItem).GetValue() );
        break;
        case EE_PARA_OUTLLEVEL:
            aDebStr += "Level=";
            aDebStr += ByteString::CreateFromInt32( ((SfxUInt16Item&)rItem).GetValue() );
        break;
        case EE_PARA_ULSPACE:
            aDebStr += "SB=";
            aDebStr += ByteString::CreateFromInt32( ((SvxULSpaceItem&)rItem).GetUpper() );
            aDebStr += ", SA=";
            aDebStr += ByteString::CreateFromInt32( ((SvxULSpaceItem&)rItem).GetLower() );
        break;
        case EE_PARA_SBL:
            aDebStr += "SBL=";
            if ( ((SvxLineSpacingItem&)rItem).GetLineSpaceRule() == SVX_LINE_SPACE_MIN )
            {
                aDebStr += "Min: ";
                aDebStr += ByteString::CreateFromInt32( ((SvxLineSpacingItem&)rItem).GetInterLineSpace() );
            }
            else if ( ((SvxLineSpacingItem&)rItem).GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_PROP )
            {
                aDebStr += "Prop: ";
                aDebStr += ByteString::CreateFromInt32( (ULONG)((SvxLineSpacingItem&)rItem).GetPropLineSpace() );
            }
            else
                aDebStr += "Unsupported Type!";
        break;
        case EE_PARA_JUST:
            aDebStr += "SvxAdust=";
            aDebStr += ByteString::CreateFromInt32( (USHORT)((SvxAdjustItem&)rItem).GetAdjust() );
        break;
        case EE_PARA_TABS:
            aDebStr += "Tabs = ?";
        break;
        case EE_CHAR_LANGUAGE:
        case EE_CHAR_LANGUAGE_CJK:
        case EE_CHAR_LANGUAGE_CTL:
            aDebStr += "Language=";
            aDebStr += ByteString::CreateFromInt32( (USHORT)((SvxLanguageItem&)rItem).GetLanguage() );
        break;
        case EE_CHAR_COLOR:
        {
            aDebStr += "Color= ";
            Color aColor( ((SvxColorItem&)rItem).GetValue() );
            aDebStr += ByteString::CreateFromInt32( (USHORT)aColor.GetRed() );
            aDebStr += ", ";
            aDebStr += ByteString::CreateFromInt32( (USHORT)aColor.GetGreen() );
            aDebStr += ", ";
            aDebStr += ByteString::CreateFromInt32( (USHORT)aColor.GetBlue() );
        }
        break;
        case EE_CHAR_FONTINFO:
        case EE_CHAR_FONTINFO_CJK:
        case EE_CHAR_FONTINFO_CTL:
        {
            aDebStr += "Font=";
            aDebStr += ByteString( ((SvxFontItem&)rItem).GetFamilyName(), RTL_TEXTENCODING_ASCII_US );
            aDebStr += " (CharSet: ";
            aDebStr += ByteString::CreateFromInt32( (USHORT)((SvxFontItem&)rItem).GetCharSet() );
            aDebStr += ')';
        }
        break;
        case EE_CHAR_FONTHEIGHT:
        case EE_CHAR_FONTHEIGHT_CJK:
        case EE_CHAR_FONTHEIGHT_CTL:
        {
            aDebStr += "Groesse=";
            aDebStr += ByteString::CreateFromInt32( ((SvxFontHeightItem&)rItem).GetHeight() );
            Size aSz( 0, ((SvxFontHeightItem&)rItem).GetHeight() );
            SfxMapUnit eUnit = rPool.GetMetric( rItem.Which() );
            MapMode aItemMapMode( (MapUnit) eUnit );
            MapMode aPntMap( MAP_POINT );
            aSz = OutputDevice::LogicToLogic( aSz, aItemMapMode, aPntMap );
            aDebStr += " Points=";
            aDebStr += ByteString::CreateFromInt32( aSz.Height() );
        }
        break;
        case EE_CHAR_FONTWIDTH:
        {
            aDebStr += "Breite=";
            aDebStr += ((SvxFontWidthItem&)rItem).GetWidth();
            Size aSz( 0, ((SvxFontWidthItem&)rItem).GetWidth() );
            SfxMapUnit eUnit = rPool.GetMetric( rItem.Which() );
            MapMode aItemMapMode( (MapUnit) eUnit );
            MapMode aPntMap( MAP_POINT );
            aSz = OutputDevice::LogicToLogic( aSz, aItemMapMode, aPntMap );
            aDebStr += " Points=";
            aDebStr += aSz.Width();
        }
        break;
        case EE_CHAR_WEIGHT:
        case EE_CHAR_WEIGHT_CJK:
        case EE_CHAR_WEIGHT_CTL:
            aDebStr += "FontWeight=";
            aDebStr += ByteString::CreateFromInt32( (USHORT)((SvxWeightItem&)rItem).GetWeight() );
        break;
        case EE_CHAR_UNDERLINE:
            aDebStr += "FontUnderline=";
            aDebStr += ByteString::CreateFromInt32( (USHORT)((SvxUnderlineItem&)rItem).GetUnderline() );
        break;
        case EE_CHAR_STRIKEOUT:
            aDebStr += "FontStrikeout=";
            aDebStr +=ByteString::CreateFromInt32(  (USHORT)((SvxCrossedOutItem&)rItem).GetStrikeout() );
        break;
        case EE_CHAR_ITALIC:
        case EE_CHAR_ITALIC_CJK:
        case EE_CHAR_ITALIC_CTL:
            aDebStr += "FontPosture=";
            aDebStr +=ByteString::CreateFromInt32(  (USHORT)((SvxPostureItem&)rItem).GetPosture() );
        break;
        case EE_CHAR_OUTLINE:
            aDebStr += "FontOutline=";
            aDebStr += ByteString::CreateFromInt32( (USHORT)((SvxContourItem&)rItem).GetValue() );
        break;
        case EE_CHAR_SHADOW:
            aDebStr += "FontShadowed=";
            aDebStr += ByteString::CreateFromInt32( (USHORT)((SvxShadowedItem&)rItem).GetValue() );
        break;
        case EE_CHAR_ESCAPEMENT:
            aDebStr += "Escape=";
            aDebStr += ByteString::CreateFromInt32( (short)((SvxEscapementItem&)rItem).GetEsc() );
            aDebStr += ", ";
            aDebStr += ByteString::CreateFromInt32( (short)((SvxEscapementItem&)rItem).GetProp() );
        break;
        case EE_CHAR_PAIRKERNING:
            aDebStr += "PairKerning=";
            aDebStr += ByteString::CreateFromInt32( (USHORT)((SvxAutoKernItem&)rItem).GetValue() );
        break;
        case EE_CHAR_KERNING:
        {
            aDebStr += "Kerning=";
            aDebStr += ByteString::CreateFromInt32( (short)((SvxKerningItem&)rItem).GetValue() );
            Size aSz( 0, (short)((SvxKerningItem&)rItem).GetValue() );
            SfxMapUnit eUnit = rPool.GetMetric( rItem.Which() );
            MapMode aItemMapMode( (MapUnit) eUnit );
            MapMode aPntMap( MAP_POINT );
            aSz = OutputDevice::LogicToLogic( aSz, aItemMapMode, aPntMap );
            aDebStr += " Points=";
            aDebStr += ByteString::CreateFromInt32( aSz.Height() );
        }
        break;
        case EE_CHAR_WLM:
            aDebStr += "WordLineMode=";
            aDebStr += ByteString::CreateFromInt32( (USHORT)((SvxWordLineModeItem&)rItem).GetValue() );
        break;
    }
    return aDebStr;
}

void DbgOutItemSet( FILE* fp, const SfxItemSet& rSet, BOOL bSearchInParent, BOOL bShowALL )
{
    for ( USHORT nWhich = EE_PARA_START; nWhich <= EE_CHAR_END; nWhich++ )
    {
        fprintf( fp, "\nWhich: %i\t", nWhich );
        if ( rSet.GetItemState( nWhich, bSearchInParent ) == SFX_ITEM_OFF )
            fprintf( fp, "ITEM_OFF   " );
        else if ( rSet.GetItemState( nWhich, bSearchInParent ) == SFX_ITEM_DONTCARE )
            fprintf( fp, "ITEM_DC    " );
        else if ( rSet.GetItemState( nWhich, bSearchInParent ) == SFX_ITEM_ON )
            fprintf( fp, "ITEM_ON   *" );

        if ( !bShowALL && ( rSet.GetItemState( nWhich, bSearchInParent ) != SFX_ITEM_ON ) )
            continue;

        const SfxPoolItem& rItem = rSet.Get( nWhich, bSearchInParent );
        ByteString aDebStr = DbgOutItem( *rSet.GetPool(), rItem );
        fprintf( fp, "%s", aDebStr.GetBuffer() );
    }
}

void EditDbg::ShowEditEngineData( EditEngine* pEE, BOOL bInfoBox )
{
#ifdef MAC
    FILE* fp = fopen( "debug.log", "w" );
#elif defined UNX
    FILE* fp = fopen( "/tmp/debug.log", "w" );
#else
    FILE* fp = fopen( "d:\\debug.log", "w" );
#endif
    if ( fp == 0 )
    {
        DBG_ERROR( "Log-File konnte nicht angelegt werden!" );
        return;
    }

    const SfxItemPool& rPool = *pEE->GetEmptyItemSet().GetPool();

    fprintf( fp, "================================================================================" );
    fprintf( fp, "\n==================   Dokument   ================================================" );
    fprintf( fp, "\n================================================================================" );
    for ( USHORT nPortion = 0; nPortion < pEE->pImpEditEngine->GetParaPortions(). Count(); nPortion++)
    {

        ParaPortion* pPPortion = pEE->pImpEditEngine->GetParaPortions().GetObject(nPortion );
        fprintf( fp, "\nAbsatz %i: Laenge = %i, Invalid = %i\nText = '%s'", nPortion, pPPortion->GetNode()->Len(), pPPortion->IsInvalid(), ByteString( *pPPortion->GetNode(), RTL_TEXTENCODING_ASCII_US ).GetBuffer() );
        fprintf( fp, "\nVorlage:" );
        SfxStyleSheet* pStyle = pPPortion->GetNode()->GetStyleSheet();
        if ( pStyle )
            fprintf( fp, " %s", ByteString( pStyle->GetName(), RTL_TEXTENCODING_ASCII_US ).GetBuffer() );
        fprintf( fp, "\nAbsatzattribute:" );
        DbgOutItemSet( fp, pPPortion->GetNode()->GetContentAttribs().GetItems(), FALSE, FALSE );

        fprintf( fp, "\nZeichenattribute:" );
        BOOL bZeroAttr = FALSE;
        for ( USHORT z = 0; z < pPPortion->GetNode()->GetCharAttribs().Count(); z++ )
        {
            EditCharAttrib* pAttr = pPPortion->GetNode()->GetCharAttribs().GetAttribs().GetObject( z );
            ByteString aCharAttribs;
            aCharAttribs += "\nA";
            aCharAttribs += ByteString::CreateFromInt32( nPortion );
            aCharAttribs += ":  ";
            aCharAttribs += ByteString::CreateFromInt32( pAttr->GetItem()->Which() );
            aCharAttribs += '\t';
            aCharAttribs += ByteString::CreateFromInt32( pAttr->GetStart() );
            aCharAttribs += '\t';
            aCharAttribs += ByteString::CreateFromInt32( pAttr->GetEnd() );
            if ( pAttr->IsEmpty() )
                bZeroAttr = TRUE;
            fprintf( fp, "%s => ", aCharAttribs.GetBuffer() );

            ByteString aDebStr = DbgOutItem( rPool, *pAttr->GetItem() );
            fprintf( fp, "%s", aDebStr.GetBuffer() );
        }
        if ( bZeroAttr )
            fprintf( fp, "\nNULL-Attribute!" );

        USHORT nTextPortions = pPPortion->GetTextPortions().Count();
        ByteString aPortionStr("\nTextportions: #");
        aPortionStr += ByteString::CreateFromInt32( nTextPortions );
        aPortionStr += " \nA";
        aPortionStr += ByteString::CreateFromInt32( nPortion );
        aPortionStr += ": Absatzlaenge = ";
        aPortionStr += ByteString::CreateFromInt32( pPPortion->GetNode()->Len() );
        aPortionStr += "\nA";
        aPortionStr += ByteString::CreateFromInt32( nPortion );
        aPortionStr += ": ";
        ULONG n = 0;
        for ( z = 0; z < nTextPortions; z++ )
        {
            TextPortion* pPortion = pPPortion->GetTextPortions().GetObject( z );
            aPortionStr += " ";
            aPortionStr += ByteString::CreateFromInt32( pPortion->GetLen() );
            aPortionStr += "(";
            aPortionStr += ByteString::CreateFromInt32( pPortion->GetSize().Width() );
            aPortionStr += ")";
            aPortionStr += "[";
            aPortionStr += ByteString::CreateFromInt32( (USHORT)pPortion->GetKind() );
            aPortionStr += "]";
            aPortionStr += ";";
            n += pPortion->GetLen();
        }
        aPortionStr += "\nA";
        aPortionStr += ByteString::CreateFromInt32( nPortion );
        aPortionStr += ": Gesamtlaenge: ";
        aPortionStr += ByteString::CreateFromInt32( n );
        if ( pPPortion->GetNode()->Len() != n )
            aPortionStr += " => Fehler !!!";
        fprintf( fp, "%s", aPortionStr.GetBuffer() );


        fprintf( fp, "\n\nZeilen:" );
        // Erstmal die Inhalte...
        for ( USHORT nLine = 0; nLine < pPPortion->GetLines().Count(); nLine++ )
        {
            EditLine* pLine = pPPortion->GetLines().GetObject( nLine );

            ByteString aLine( *(pPPortion->GetNode()), pLine->GetStart(), pLine->GetEnd() - pLine->GetStart(), RTL_TEXTENCODING_ASCII_US );
            fprintf( fp, "\nZeile %i\t>%s<", nLine, aLine.GetBuffer() );
        }
        // dann die internen Daten...
        for ( nLine = 0; nLine < pPPortion->GetLines().Count(); nLine++ )
        {
            EditLine* pLine = pPPortion->GetLines().GetObject( nLine );
            fprintf( fp, "\nZeile %i:\tStart: %i,\tEnd: %i", nLine, pLine->GetStart(), pLine->GetEnd() );
            fprintf( fp, "\t\tPortions: %i - %i.\tHoehe: %i, Ascent=%i", pLine->GetStartPortion(), pLine->GetEndPortion(), pLine->GetHeight(), pLine->GetMaxAscent() );
        }

        fprintf( fp, "\n-----------------------------------------------------------------------------" );
    }

    if ( pEE->pImpEditEngine->GetStyleSheetPool() )
    {
        ULONG nStyles = pEE->pImpEditEngine->GetStyleSheetPool() ? pEE->pImpEditEngine->GetStyleSheetPool()->Count() : 0;
        fprintf( fp, "\n\n================================================================================" );
        fprintf( fp, "\n==================   Stylesheets   =============================================" );
        fprintf( fp, "\n================================================================================" );
        fprintf( fp, "\n#Vorlagen:   %lu\n", nStyles );
        SfxStyleSheetBase* pStyle = pEE->pImpEditEngine->GetStyleSheetPool()->First();
        while ( pStyle )
        {
            fprintf( fp, "\nVorlage:   %s", ByteString( pStyle->GetName(), RTL_TEXTENCODING_ASCII_US ).GetBuffer() );
            fprintf( fp, "\nParent:    %s", ByteString( pStyle->GetParent(), RTL_TEXTENCODING_ASCII_US ).GetBuffer() );
            fprintf( fp, "\nFollow:    %s", ByteString( pStyle->GetFollow(), RTL_TEXTENCODING_ASCII_US ).GetBuffer() );
            DbgOutItemSet( fp, pStyle->GetItemSet(), FALSE, FALSE );
            fprintf( fp, "\n----------------------------------" );

            pStyle = pEE->pImpEditEngine->GetStyleSheetPool()->Next();
        }
    }

    fprintf( fp, "\n\n================================================================================" );
    fprintf( fp, "\n==================   Defaults   ================================================" );
    fprintf( fp, "\n================================================================================" );
    DbgOutItemSet( fp, pEE->pImpEditEngine->GetEmptyItemSet(), TRUE, TRUE );

    fprintf( fp, "\n\n================================================================================" );
    fprintf( fp, "\n==================   EditEngine & Views   ======================================" );
    fprintf( fp, "\n================================================================================" );
    fprintf( fp, "\nControl: %lx", pEE->GetControlWord() );
    fprintf( fp, "\nRefMapMode: %i", pEE->pImpEditEngine->pRefDev->GetMapMode().GetMapUnit() );
    fprintf( fp, "\nPaperSize: %li x %li", pEE->GetPaperSize().Width(), pEE->GetPaperSize().Height() );
    fprintf( fp, "\nMaxAutoPaperSize: %li x %li", pEE->GetMaxAutoPaperSize().Width(), pEE->GetMaxAutoPaperSize().Height() );
    fprintf( fp, "\nMinAutoPaperSize: %li x %li", pEE->GetMinAutoPaperSize().Width(), pEE->GetMinAutoPaperSize().Height() );
    fprintf( fp, "\nUpdate: %i", pEE->GetUpdateMode() );
    fprintf( fp, "\nAnzahl der Views: %i", pEE->GetViewCount() );
    for ( USHORT nView = 0; nView < pEE->GetViewCount(); nView++ )
    {
        EditView* pV = pEE->GetView( nView );
        DBG_ASSERT( pV, "View nicht gefunden!" );
        fprintf( fp, "\nView %i: Focus=%i", nView, pV->GetWindow()->HasFocus() );
        Rectangle aR( pV->GetOutputArea() );
        fprintf( fp, "\n  OutputArea: nX=%li, nY=%li, dX=%li, dY=%li, MapMode = %i", aR.TopLeft().X(), aR.TopLeft().Y(), aR.GetSize().Width(), aR.GetSize().Height() , pV->GetWindow()->GetMapMode().GetMapUnit() );
        aR = pV->GetVisArea();
        fprintf( fp, "\n  VisArea: nX=%li, nY=%li, dX=%li, dY=%li", aR.TopLeft().X(), aR.TopLeft().Y(), aR.GetSize().Width(), aR.GetSize().Height() );
        ESelection aSel = pV->GetSelection();
        fprintf( fp, "\n  Selektion: Start=%u,%u, End=%u,%u", aSel.nStartPara, aSel.nStartPos, aSel.nEndPara, aSel.nEndPos );
    }
    if ( pEE->GetActiveView() )
    {
        fprintf( fp, "\n\n================================================================================" );
        fprintf( fp, "\n==================   Aktuelle View   ===========================================" );
        fprintf( fp, "\n================================================================================" );
        DbgOutItemSet( fp, pEE->GetActiveView()->GetAttribs(), TRUE, FALSE );
    }
    fclose( fp );
    if ( bInfoBox )
        InfoBox(0, String( RTL_CONSTASCII_USTRINGPARAM( "D:\\DEBUG.LOG !" ) ) ).Execute();
}

ByteString EditDbg::GetPortionInfo( ParaPortion* pPPortion )
{
    ByteString aDebStr( "Absatzlaenge = " );
    aDebStr += ByteString::CreateFromInt32( pPPortion->GetNode()->Len() );

    aDebStr += "\nZeichenattribute:";
    for ( USHORT z = 0; z < pPPortion->GetNode()->GetCharAttribs().Count(); z++ )
    {
        EditCharAttrib* pAttr = pPPortion->GetNode()->GetCharAttribs().GetAttribs().GetObject( z );
        aDebStr += "\n  ";
        aDebStr += ByteString::CreateFromInt32( pAttr->GetItem()->Which() );
        aDebStr += '\t';
        aDebStr += ByteString::CreateFromInt32( pAttr->GetStart() );
        aDebStr += '\t';
        aDebStr += ByteString::CreateFromInt32( pAttr->GetEnd() );
    }

    aDebStr += "\nTextportions:";
    USHORT n = 0;
    for ( z = 0; z < pPPortion->GetTextPortions().Count(); z++ )
    {
        TextPortion* pPortion = pPPortion->GetTextPortions().GetObject( z );
        aDebStr += " ";
        aDebStr += ByteString::CreateFromInt32( pPortion->GetLen() );
        aDebStr += "(";
        aDebStr += ByteString::CreateFromInt32( pPortion->GetSize().Width() );
        aDebStr += ")";
        aDebStr += ";";
        n += pPortion->GetLen();
    }
    aDebStr += "\nGesamtlaenge: ";
    aDebStr += n;
    aDebStr += "\nSortiert nach Start:";
    for ( USHORT x = 0; x < pPPortion->GetNode()->GetCharAttribs().Count(); x++ )
    {
        EditCharAttrib* pCurAttrib = pPPortion->GetNode()->GetCharAttribs().GetAttribs().GetObject( x );
        aDebStr += "\nStart: ";
        aDebStr += ByteString::CreateFromInt32( pCurAttrib->GetStart() );
        aDebStr += "\tEnde: ";
        aDebStr += ByteString::CreateFromInt32( pCurAttrib->GetEnd() );
    }
    return aDebStr;
}

ByteString EditDbg::GetTextPortionInfo( TextPortionList& rPortions )
{
    ByteString aDebStr;
    for ( USHORT z = 0; z < rPortions.Count(); z++ )
    {
        TextPortion* pPortion = rPortions.GetObject( z );
        aDebStr += " ";
        aDebStr += ByteString::CreateFromInt32( pPortion->GetLen() );
        aDebStr += "(";
        aDebStr += ByteString::CreateFromInt32( pPortion->GetSize().Width() );
        aDebStr += ")";
        aDebStr += ";";
    }
    return aDebStr;
}

void EditDbg::ShowPortionData( ParaPortion* pPortion )
{
    ByteString aDebStr( GetPortionInfo( pPortion ) );
    InfoBox( 0, String( aDebStr, RTL_TEXTENCODING_ASCII_US )  ).Execute();
}


BOOL ParaPortion::DbgCheckTextPortions()
{
    // pruefen, ob Portionlaenge ok:
    USHORT nXLen = 0;
    for ( USHORT nPortion = 0; nPortion < aTextPortionList.Count(); nPortion++  )
        nXLen += aTextPortionList[nPortion]->GetLen();
    return nXLen == pNode->Len() ? TRUE : FALSE;
}

BOOL CheckOrderedList( CharAttribArray& rAttribs, BOOL bStart )
{
    USHORT nPrev = 0;
    for ( USHORT nAttr = 0; nAttr < rAttribs.Count(); nAttr++ )
    {
        EditCharAttrib* pAttr = rAttribs[nAttr];
        USHORT nCur = bStart ? pAttr->GetStart() : pAttr->GetEnd();
        if ( nCur < nPrev )
            return FALSE;

        nPrev = nCur;
    }
    return TRUE;
}

#endif


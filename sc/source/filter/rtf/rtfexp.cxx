/*************************************************************************
 *
 *  $RCSfile: rtfexp.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:14 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop


#include "scitems.hxx"
#ifndef _SVX_ALGITEM_HXX //autogen
#include <svx/algitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _RTFOUT_HXX
#include <svtools/rtfout.hxx>
#endif
#ifndef _RTFKEYWD_HXX
#include <svtools/rtfkeywd.hxx>
#endif

#include "rtfexp.hxx"
#include "filter.hxx"
#include "flttools.hxx"
#include "document.hxx"
#include "patattr.hxx"
#include "attrib.hxx"
#include "cell.hxx"
#include "cellform.hxx"
#include "editutil.hxx"
#include "stlpool.hxx"


//------------------------------------------------------------------

FltError ScExportRTF( SvStream& rStrm, ScDocument* pDoc,
        const ScRange& rRange, const CharSet eNach )
{
    ScRTFExport aEx( rStrm, pDoc, rRange );
    return aEx.Write();
}


ScRTFExport::ScRTFExport( SvStream& rStrmP, ScDocument* pDocP, const ScRange& rRangeP )
            :
            ScExportBase( rStrmP, pDocP, rRangeP ),
            pCellX( new ULONG[ MAXCOL+2 ] )
{
}


ScRTFExport::~ScRTFExport()
{
    delete [] pCellX;
}


ULONG ScRTFExport::Write()
{
    rStrm << '{' << sRTF_RTF;
#ifdef MAC
    rStrm << sRTF_MAC << sNewLine;
#else
    rStrm << sRTF_ANSI << sNewLine;
#endif

#if 0
// das ist noch nicht ausgegoren
/*
    SfxStyleSheetBasePool*  pStylePool  = pDoc->GetStyleSheetPool();
    SfxStyleSheetBase*      pStyleSheet = NULL;
    DBG_ASSERT( pStylePool, "StylePool not found! :-(" );
    pStylePool->SetSearchMask( SFX_STYLE_FAMILY_PARA, SFXSTYLEBIT_ALL );
    pStyleSheet = pStylePool->Find( STRING_STANDARD, SFX_STYLE_FAMILY_PARA );
    DBG_ASSERT( pStyleSheet, "ParaStyle not found! :-(" );
    const SfxItemSet& rSetPara = pStyleSheet->GetItemSet();

    // fonttbl
    String aFontFamilyName(
        ((const SvxFontItem&)(rSetPara.Get( ATTR_FONT ))).GetFamilyName() );
    rStrm << sRTF_DEFF << '0'
        << '{' << sRTF_FONTTBL
        << '{' << sRTF_F << '0' << sRTF_FNIL << ' ' << aFontFamilyName.GetStr() << ";}"
        << '}' << sNewLine;

    // hier kaeme die colortbl

    // stylesheet
    UINT32 nFontHeight =
        ((const SvxFontHeightItem&)(rSetPara.Get( ATTR_FONT_HEIGHT ))).GetHeight();
    rStrm << '{' << sRTF_STYLESHEET
        << '{' << sRTF_FS << String( UINT32(nFontHeight / TWIPS_PER_POINT) ).GetStr()
            << ' ' << pStyleSheet->GetName().GetStr() << ";}"
        << '}' << sNewLine;
*/
#endif

    // Daten
    for ( USHORT nTab = aRange.aStart.Tab(); nTab <= aRange.aEnd.Tab(); nTab++ )
    {
        if ( nTab > aRange.aStart.Tab() )
            rStrm << sRTF_PAR;
        WriteTab( nTab );
    }

    rStrm << '}' << sNewLine;
    return rStrm.GetError();
}


void ScRTFExport::WriteTab( USHORT nTab )
{
    rStrm << '{' << sNewLine;
    if ( pDoc->HasTable( nTab ) )
    {
        memset( &pCellX[0], 0, (MAXCOL+2) * sizeof(ULONG) );
        USHORT nCol;
        USHORT nEndCol = aRange.aEnd.Col();
        for ( nCol = aRange.aStart.Col(); nCol <= nEndCol; nCol++ )
        {
            pCellX[nCol+1] = pCellX[nCol] + pDoc->GetColWidth( nCol, nTab );
        }

        USHORT nEndRow = aRange.aEnd.Row();
        for ( USHORT nRow = aRange.aStart.Row(); nRow <= nEndRow; nRow++ )
        {
            WriteRow( nTab, nRow );
        }
    }
    rStrm << '}' << sNewLine;
}


void ScRTFExport::WriteRow( USHORT nTab, USHORT nRow )
{
    rStrm << sRTF_TROWD << sRTF_TRGAPH << "30" << sRTF_TRLEFT << "-30";
    rStrm << sRTF_TRRH << ByteString::CreateFromInt32( pDoc->GetRowHeight( nRow, nTab ) ).GetBuffer();
    USHORT nCol;
    USHORT nEndCol = aRange.aEnd.Col();
    for ( nCol = aRange.aStart.Col(); nCol <= nEndCol; nCol++ )
    {
        const ScPatternAttr* pAttr = pDoc->GetPattern( nCol, nRow, nTab );
        const ScMergeAttr&      rMergeAttr      = (const ScMergeAttr&)      pAttr->GetItem( ATTR_MERGE );
        const SvxVerJustifyItem& rVerJustifyItem= (const SvxVerJustifyItem&)pAttr->GetItem( ATTR_VER_JUSTIFY );

        const sal_Char* pChar;

        if ( rMergeAttr.GetColMerge() )
            rStrm << sRTF_CLMGF;
        else
        {
            const ScMergeFlagAttr& rMergeFlagAttr = (const ScMergeFlagAttr&) pAttr->GetItem( ATTR_MERGE_FLAG );
            if ( rMergeFlagAttr.IsHorOverlapped() )
                rStrm << sRTF_CLMRG;
        }

        switch( rVerJustifyItem.GetValue() )
        {
            case SVX_VER_JUSTIFY_TOP:       pChar = sRTF_CLVERTALT; break;
            case SVX_VER_JUSTIFY_CENTER:    pChar = sRTF_CLVERTALC; break;
            case SVX_VER_JUSTIFY_BOTTOM:    pChar = sRTF_CLVERTALB; break;
            case SVX_VER_JUSTIFY_STANDARD:  pChar = sRTF_CLVERTALB; break;  //! Bottom
            default:                        pChar = NULL;           break;
        }
        if ( pChar )
            rStrm << pChar;

        rStrm << sRTF_CELLX << ByteString::CreateFromInt32( pCellX[nCol+1] ).GetBuffer();
        if ( (nCol & 0x0F) == 0x0F )
            rStrm << sNewLine;      // Zeilen nicht zu lang werden lassen
    }
    rStrm << sRTF_PARD << sRTF_PLAIN << sRTF_INTBL << sNewLine;

    ULONG nStrmPos = rStrm.Tell();
    for ( nCol = aRange.aStart.Col(); nCol <= nEndCol; nCol++ )
    {
        WriteCell( nTab, nRow, nCol );
        if ( rStrm.Tell() - nStrmPos > 255 )
        {   // Zeilen nicht zu lang werden lassen
            rStrm << sNewLine;
            nStrmPos = rStrm.Tell();
        }
    }
    rStrm << sRTF_ROW << sNewLine;
}


void ScRTFExport::WriteCell( USHORT nTab, USHORT nRow, USHORT nCol )
{
    const ScPatternAttr* pAttr = pDoc->GetPattern( nCol, nRow, nTab );

    const ScMergeFlagAttr& rMergeFlagAttr = (const ScMergeFlagAttr&) pAttr->GetItem( ATTR_MERGE_FLAG );
    if ( rMergeFlagAttr.IsHorOverlapped() )
    {
        rStrm << sRTF_CELL;
        return ;
    }

    ScBaseCell* pCell;
    pDoc->GetCell( nCol, nRow, nTab, pCell );
    BOOL bValueData;
    String aContent;
    if ( pCell )
    {
        switch ( pCell->GetCellType() )
        {
            case CELLTYPE_NOTE :
                bValueData = FALSE;
            break;      // nix
            case CELLTYPE_EDIT :
            {
                bValueData = FALSE;
                EditEngine& rEngine = GetEditEngine();
                const EditTextObject* pObj;
                ((const ScEditCell*)pCell)->GetData( pObj );
                if ( pObj )
                {
                    rEngine.SetText( *pObj );
                    aContent = rEngine.GetText( LINEEND_LF );   // LineFeed zwischen Absaetzen!
                }
            }
            break;
            default:
            {
                bValueData = pCell->HasValueData();
                ULONG nFormat = pAttr->GetNumberFormat( pFormatter );
                Color* pColor;
                ScCellFormat::GetString( pCell, nFormat, aContent, &pColor, *pFormatter );
            }
        }
    }
    else
        bValueData = FALSE;

    BOOL bResetPar, bResetAttr;
    bResetPar = bResetAttr = FALSE;

    const SvxHorJustifyItem&    rHorJustifyItem = (const SvxHorJustifyItem&)pAttr->GetItem( ATTR_HOR_JUSTIFY );
    const SvxWeightItem&        rWeightItem     = (const SvxWeightItem&)    pAttr->GetItem( ATTR_FONT_WEIGHT );
    const SvxPostureItem&       rPostureItem    = (const SvxPostureItem&)   pAttr->GetItem( ATTR_FONT_POSTURE );
    const SvxUnderlineItem&     rUnderlineItem  = (const SvxUnderlineItem&) pAttr->GetItem( ATTR_FONT_UNDERLINE );

    const sal_Char* pChar;

    switch( rHorJustifyItem.GetValue() )
    {
        case SVX_HOR_JUSTIFY_STANDARD:
            pChar = (bValueData ? sRTF_QR : sRTF_QL);
            break;
        case SVX_HOR_JUSTIFY_CENTER:    pChar = sRTF_QC;    break;
        case SVX_HOR_JUSTIFY_BLOCK:     pChar = sRTF_QJ;    break;
        case SVX_HOR_JUSTIFY_RIGHT:     pChar = sRTF_QR;    break;
        case SVX_HOR_JUSTIFY_LEFT:
        case SVX_HOR_JUSTIFY_REPEAT:
        default:                        pChar = sRTF_QL;    break;
    }
    rStrm << pChar;

    if ( rWeightItem.GetWeight() >= WEIGHT_BOLD )
    {   // bold
        bResetAttr = TRUE;
        rStrm << sRTF_B;
    }
    if ( rPostureItem.GetPosture() != ITALIC_NONE )
    {   // italic
        bResetAttr = TRUE;
        rStrm << sRTF_I;
    }
    if ( rUnderlineItem.GetUnderline() != UNDERLINE_NONE )
    {   // underline
        bResetAttr = TRUE;
        rStrm << sRTF_UL;
    }

    rStrm << ' ';
    RTFOutFuncs::Out_String( rStrm, aContent );
    rStrm << sRTF_CELL;

    if ( bResetPar )
        rStrm << sRTF_PARD << sRTF_INTBL;
    if ( bResetAttr )
        rStrm << sRTF_PLAIN;
}



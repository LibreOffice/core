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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include "scitems.hxx"
#include <editeng/eeitem.hxx>


#include <svx/algitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svl/style.hxx>
#include <svtools/rtfout.hxx>
#include <svtools/rtfkeywd.hxx>

#include "rtfexp.hxx"
#include "filter.hxx"
#include "document.hxx"
#include "patattr.hxx"
#include "attrib.hxx"
#include "cell.hxx"
#include "cellform.hxx"
#include "editutil.hxx"
#include "stlpool.hxx"
#include "ftools.hxx"

//------------------------------------------------------------------

FltError ScFormatFilterPluginImpl::ScExportRTF( SvStream& rStrm, ScDocument* pDoc,
        const ScRange& rRange, const CharSet /*eNach*/ )
{
    ScRTFExport aEx( rStrm, pDoc, rRange );
    return aEx.Write();
}


ScRTFExport::ScRTFExport( SvStream& rStrmP, ScDocument* pDocP, const ScRange& rRangeP )
            :
            ScExportBase( rStrmP, pDocP, rRangeP ),
            pCellX( new sal_uLong[ MAXCOL+2 ] )
{
}


ScRTFExport::~ScRTFExport()
{
    delete [] pCellX;
}


sal_uLong ScRTFExport::Write()
{
    rStrm << '{' << OOO_STRING_SVTOOLS_RTF_RTF;
    rStrm << OOO_STRING_SVTOOLS_RTF_ANSI << sNewLine;

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
    rStrm << OOO_STRING_SVTOOLS_RTF_DEFF << '0'
        << '{' << OOO_STRING_SVTOOLS_RTF_FONTTBL
        << '{' << OOO_STRING_SVTOOLS_RTF_F << '0' << OOO_STRING_SVTOOLS_RTF_FNIL << ' ' << aFontFamilyName.GetStr() << ";}"
        << '}' << sNewLine;

    // hier kaeme die colortbl

    // stylesheet
    sal_uInt32 nFontHeight =
        ((const SvxFontHeightItem&)(rSetPara.Get( ATTR_FONT_HEIGHT ))).GetHeight();
    rStrm << '{' << OOO_STRING_SVTOOLS_RTF_STYLESHEET
        << '{' << OOO_STRING_SVTOOLS_RTF_FS << String( sal_uInt32(nFontHeight / TWIPS_PER_POINT) ).GetStr()
            << ' ' << pStyleSheet->GetName().GetStr() << ";}"
        << '}' << sNewLine;
*/
#endif

    // Daten
    for ( SCTAB nTab = aRange.aStart.Tab(); nTab <= aRange.aEnd.Tab(); nTab++ )
    {
        if ( nTab > aRange.aStart.Tab() )
            rStrm << OOO_STRING_SVTOOLS_RTF_PAR;
        WriteTab( nTab );
    }

    rStrm << '}' << sNewLine;
    return rStrm.GetError();
}


void ScRTFExport::WriteTab( SCTAB nTab )
{
    rStrm << '{' << sNewLine;
    if ( pDoc->HasTable( nTab ) )
    {
        memset( &pCellX[0], 0, (MAXCOL+2) * sizeof(sal_uLong) );
        SCCOL nCol;
        SCCOL nEndCol = aRange.aEnd.Col();
        for ( nCol = aRange.aStart.Col(); nCol <= nEndCol; nCol++ )
        {
            pCellX[nCol+1] = pCellX[nCol] + pDoc->GetColWidth( nCol, nTab );
        }

        SCROW nEndRow = aRange.aEnd.Row();
        for ( SCROW nRow = aRange.aStart.Row(); nRow <= nEndRow; nRow++ )
        {
            WriteRow( nTab, nRow );
        }
    }
    rStrm << '}' << sNewLine;
}


void ScRTFExport::WriteRow( SCTAB nTab, SCROW nRow )
{
    rStrm << OOO_STRING_SVTOOLS_RTF_TROWD << OOO_STRING_SVTOOLS_RTF_TRGAPH << "30" << OOO_STRING_SVTOOLS_RTF_TRLEFT << "-30";
    rStrm << OOO_STRING_SVTOOLS_RTF_TRRH << ByteString::CreateFromInt32( pDoc->GetRowHeight( nRow, nTab ) ).GetBuffer();
    SCCOL nCol;
    SCCOL nEndCol = aRange.aEnd.Col();
    for ( nCol = aRange.aStart.Col(); nCol <= nEndCol; nCol++ )
    {
        const ScPatternAttr* pAttr = pDoc->GetPattern( nCol, nRow, nTab );
        const ScMergeAttr&      rMergeAttr      = (const ScMergeAttr&)      pAttr->GetItem( ATTR_MERGE );
        const SvxVerJustifyItem& rVerJustifyItem= (const SvxVerJustifyItem&)pAttr->GetItem( ATTR_VER_JUSTIFY );

        const sal_Char* pChar;

        if ( rMergeAttr.GetColMerge() != 0 )
            rStrm << OOO_STRING_SVTOOLS_RTF_CLMGF;
        else
        {
            const ScMergeFlagAttr& rMergeFlagAttr = (const ScMergeFlagAttr&) pAttr->GetItem( ATTR_MERGE_FLAG );
            if ( rMergeFlagAttr.IsHorOverlapped() )
                rStrm << OOO_STRING_SVTOOLS_RTF_CLMRG;
        }

        switch( rVerJustifyItem.GetValue() )
        {
            case SVX_VER_JUSTIFY_TOP:       pChar = OOO_STRING_SVTOOLS_RTF_CLVERTALT;   break;
            case SVX_VER_JUSTIFY_CENTER:    pChar = OOO_STRING_SVTOOLS_RTF_CLVERTALC;   break;
            case SVX_VER_JUSTIFY_BOTTOM:    pChar = OOO_STRING_SVTOOLS_RTF_CLVERTALB;   break;
            case SVX_VER_JUSTIFY_STANDARD:  pChar = OOO_STRING_SVTOOLS_RTF_CLVERTALB;   break;  //! Bottom
            default:                        pChar = NULL;           break;
        }
        if ( pChar )
            rStrm << pChar;

        rStrm << OOO_STRING_SVTOOLS_RTF_CELLX << ByteString::CreateFromInt32( pCellX[nCol+1] ).GetBuffer();
        if ( (nCol & 0x0F) == 0x0F )
            rStrm << sNewLine;      // Zeilen nicht zu lang werden lassen
    }
    rStrm << OOO_STRING_SVTOOLS_RTF_PARD << OOO_STRING_SVTOOLS_RTF_PLAIN << OOO_STRING_SVTOOLS_RTF_INTBL << sNewLine;

    sal_uLong nStrmPos = rStrm.Tell();
    for ( nCol = aRange.aStart.Col(); nCol <= nEndCol; nCol++ )
    {
        WriteCell( nTab, nRow, nCol );
        if ( rStrm.Tell() - nStrmPos > 255 )
        {   // Zeilen nicht zu lang werden lassen
            rStrm << sNewLine;
            nStrmPos = rStrm.Tell();
        }
    }
    rStrm << OOO_STRING_SVTOOLS_RTF_ROW << sNewLine;
}


void ScRTFExport::WriteCell( SCTAB nTab, SCROW nRow, SCCOL nCol )
{
    const ScPatternAttr* pAttr = pDoc->GetPattern( nCol, nRow, nTab );

    const ScMergeFlagAttr& rMergeFlagAttr = (const ScMergeFlagAttr&) pAttr->GetItem( ATTR_MERGE_FLAG );
    if ( rMergeFlagAttr.IsHorOverlapped() )
    {
        rStrm << OOO_STRING_SVTOOLS_RTF_CELL;
        return ;
    }

    ScBaseCell* pCell;
    pDoc->GetCell( nCol, nRow, nTab, pCell );
    sal_Bool bValueData;
    String aContent;
    if ( pCell )
    {
        switch ( pCell->GetCellType() )
        {
            case CELLTYPE_NOTE :
                bValueData = sal_False;
            break;      // nix
            case CELLTYPE_EDIT :
            {
                bValueData = sal_False;
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
                sal_uLong nFormat = pAttr->GetNumberFormat( pFormatter );
                Color* pColor;
                ScCellFormat::GetString( pCell, nFormat, aContent, &pColor, *pFormatter );
            }
        }
    }
    else
        bValueData = sal_False;

    sal_Bool bResetPar, bResetAttr;
    bResetPar = bResetAttr = sal_False;

    const SvxHorJustifyItem&    rHorJustifyItem = (const SvxHorJustifyItem&)pAttr->GetItem( ATTR_HOR_JUSTIFY );
    const SvxWeightItem&        rWeightItem     = (const SvxWeightItem&)    pAttr->GetItem( ATTR_FONT_WEIGHT );
    const SvxPostureItem&       rPostureItem    = (const SvxPostureItem&)   pAttr->GetItem( ATTR_FONT_POSTURE );
    const SvxUnderlineItem&     rUnderlineItem  = (const SvxUnderlineItem&) pAttr->GetItem( ATTR_FONT_UNDERLINE );

    const sal_Char* pChar;

    switch( rHorJustifyItem.GetValue() )
    {
        case SVX_HOR_JUSTIFY_STANDARD:
            pChar = (bValueData ? OOO_STRING_SVTOOLS_RTF_QR : OOO_STRING_SVTOOLS_RTF_QL);
            break;
        case SVX_HOR_JUSTIFY_CENTER:    pChar = OOO_STRING_SVTOOLS_RTF_QC;  break;
        case SVX_HOR_JUSTIFY_BLOCK:     pChar = OOO_STRING_SVTOOLS_RTF_QJ;  break;
        case SVX_HOR_JUSTIFY_RIGHT:     pChar = OOO_STRING_SVTOOLS_RTF_QR;  break;
        case SVX_HOR_JUSTIFY_LEFT:
        case SVX_HOR_JUSTIFY_REPEAT:
        default:                        pChar = OOO_STRING_SVTOOLS_RTF_QL;  break;
    }
    rStrm << pChar;

    if ( rWeightItem.GetWeight() >= WEIGHT_BOLD )
    {   // bold
        bResetAttr = sal_True;
        rStrm << OOO_STRING_SVTOOLS_RTF_B;
    }
    if ( rPostureItem.GetPosture() != ITALIC_NONE )
    {   // italic
        bResetAttr = sal_True;
        rStrm << OOO_STRING_SVTOOLS_RTF_I;
    }
    if ( rUnderlineItem.GetLineStyle() != UNDERLINE_NONE )
    {   // underline
        bResetAttr = sal_True;
        rStrm << OOO_STRING_SVTOOLS_RTF_UL;
    }

    rStrm << ' ';
    RTFOutFuncs::Out_String( rStrm, aContent );
    rStrm << OOO_STRING_SVTOOLS_RTF_CELL;

    if ( bResetPar )
        rStrm << OOO_STRING_SVTOOLS_RTF_PARD << OOO_STRING_SVTOOLS_RTF_INTBL;
    if ( bResetAttr )
        rStrm << OOO_STRING_SVTOOLS_RTF_PLAIN;
}



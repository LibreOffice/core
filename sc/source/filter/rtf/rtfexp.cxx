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

#include "scitems.hxx"
#include <editeng/eeitem.hxx>


#include <svx/algitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/justifyitem.hxx>
#include <svl/style.hxx>
#include <svtools/rtfout.hxx>
#include <svtools/rtfkeywd.hxx>

#include "rtfexp.hxx"
#include "filter.hxx"
#include "document.hxx"
#include "patattr.hxx"
#include "attrib.hxx"
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
    rStrm << OOO_STRING_SVTOOLS_RTF_TRRH << OString::number(pDoc->GetRowHeight(nRow, nTab)).getStr();
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

        rStrm << OOO_STRING_SVTOOLS_RTF_CELLX << OString::number(pCellX[nCol+1]).getStr();
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

    bool bValueData = false;
    OUString aContent;
    ScAddress aPos(nCol, nRow, nTab);
    switch (pDoc->GetCellType(aPos))
    {
        case CELLTYPE_NONE:
            bValueData = false;
        break;
        case CELLTYPE_EDIT:
        {
            bValueData = false;
            const EditTextObject* pObj = pDoc->GetEditText(aPos);
            if (pObj)
            {
                EditEngine& rEngine = GetEditEngine();
                rEngine.SetText(*pObj);
                aContent = rEngine.GetText(LINEEND_LF);   // LineFeed zwischen Absaetzen!
            }
        }
        break;
        default:
        {
            bValueData = pDoc->HasValueData(aPos);
            sal_uLong nFormat = pAttr->GetNumberFormat(pFormatter);
            Color* pColor;
            aContent = ScCellFormat::GetString(*pDoc, aPos, nFormat, &pColor, *pFormatter);
        }
    }

    sal_Bool bResetPar, bResetAttr;
    bResetPar = bResetAttr = false;

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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

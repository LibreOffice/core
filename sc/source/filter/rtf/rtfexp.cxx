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

#include <scitems.hxx>

#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/justifyitem.hxx>
#include <svtools/rtfout.hxx>
#include <svtools/rtfkeywd.hxx>

#include <rtfexp.hxx>
#include <cellvalue.hxx>
#include <document.hxx>
#include <patattr.hxx>
#include <attrib.hxx>
#include <cellform.hxx>
#include <editutil.hxx>
#include <ftools.hxx>

void ScFormatFilterPluginImpl::ScExportRTF( SvStream& rStrm, ScDocument* pDoc,
        const ScRange& rRange, const rtl_TextEncoding /*eNach*/ )
{
    ScRTFExport aEx( rStrm, pDoc, rRange );
    aEx.Write();
}

ScRTFExport::ScRTFExport( SvStream& rStrmP, ScDocument* pDocP, const ScRange& rRangeP )
            :
            ScExportBase( rStrmP, pDocP, rRangeP ),
            pCellX( new sal_uLong[ MAXCOL+2 ] )
{
}

ScRTFExport::~ScRTFExport()
{
}

void ScRTFExport::Write()
{
    rStrm.WriteChar( '{' ).WriteCharPtr( OOO_STRING_SVTOOLS_RTF_RTF );
    rStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_ANSI ).WriteCharPtr( SAL_NEWLINE_STRING );

    // Data
    for ( SCTAB nTab = aRange.aStart.Tab(); nTab <= aRange.aEnd.Tab(); nTab++ )
    {
        if ( nTab > aRange.aStart.Tab() )
            rStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_PAR );
        WriteTab( nTab );
    }

    rStrm.WriteChar( '}' ).WriteCharPtr( SAL_NEWLINE_STRING );
}

void ScRTFExport::WriteTab( SCTAB nTab )
{
    rStrm.WriteChar( '{' ).WriteCharPtr( SAL_NEWLINE_STRING );
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
    rStrm.WriteChar( '}' ).WriteCharPtr( SAL_NEWLINE_STRING );
}

void ScRTFExport::WriteRow( SCTAB nTab, SCROW nRow )
{
    rStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_TROWD ).WriteCharPtr( OOO_STRING_SVTOOLS_RTF_TRGAPH ).WriteCharPtr( "30" ).WriteCharPtr( OOO_STRING_SVTOOLS_RTF_TRLEFT ).WriteCharPtr( "-30" );
    rStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_TRRH ).WriteCharPtr( OString::number(pDoc->GetRowHeight(nRow, nTab)).getStr() );
    SCCOL nCol;
    SCCOL nEndCol = aRange.aEnd.Col();
    for ( nCol = aRange.aStart.Col(); nCol <= nEndCol; nCol++ )
    {
        const ScPatternAttr* pAttr = pDoc->GetPattern( nCol, nRow, nTab );
        const ScMergeAttr&      rMergeAttr      = pAttr->GetItem( ATTR_MERGE );
        const SvxVerJustifyItem& rVerJustifyItem= pAttr->GetItem( ATTR_VER_JUSTIFY );

        const sal_Char* pChar;

        if ( rMergeAttr.GetColMerge() != 0 )
            rStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_CLMGF );
        else
        {
            const ScMergeFlagAttr& rMergeFlagAttr = pAttr->GetItem( ATTR_MERGE_FLAG );
            if ( rMergeFlagAttr.IsHorOverlapped() )
                rStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_CLMRG );
        }

        switch( rVerJustifyItem.GetValue() )
        {
            case SvxCellVerJustify::Top:       pChar = OOO_STRING_SVTOOLS_RTF_CLVERTALT;   break;
            case SvxCellVerJustify::Center:    pChar = OOO_STRING_SVTOOLS_RTF_CLVERTALC;   break;
            case SvxCellVerJustify::Bottom:    pChar = OOO_STRING_SVTOOLS_RTF_CLVERTALB;   break;
            case SvxCellVerJustify::Standard:  pChar = OOO_STRING_SVTOOLS_RTF_CLVERTALB;   break;  //! Bottom
            default:                           pChar = nullptr;           break;
        }
        if ( pChar )
            rStrm.WriteCharPtr( pChar );

        rStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_CELLX ).WriteCharPtr( OString::number(pCellX[nCol+1]).getStr() );
        if ( (nCol & 0x0F) == 0x0F )
            rStrm.WriteCharPtr( SAL_NEWLINE_STRING ); // Do not let lines get too long
    }
    rStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_PARD ).WriteCharPtr( OOO_STRING_SVTOOLS_RTF_PLAIN ).WriteCharPtr( OOO_STRING_SVTOOLS_RTF_INTBL ).WriteCharPtr( SAL_NEWLINE_STRING );

    sal_uLong nStrmPos = rStrm.Tell();
    for ( nCol = aRange.aStart.Col(); nCol <= nEndCol; nCol++ )
    {
        WriteCell( nTab, nRow, nCol );
        if ( rStrm.Tell() - nStrmPos > 255 )
        {   // Do not let lines get too long
            rStrm.WriteCharPtr( SAL_NEWLINE_STRING );
            nStrmPos = rStrm.Tell();
        }
    }
    rStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_ROW ).WriteCharPtr( SAL_NEWLINE_STRING );
}

void ScRTFExport::WriteCell( SCTAB nTab, SCROW nRow, SCCOL nCol )
{
    const ScPatternAttr* pAttr = pDoc->GetPattern( nCol, nRow, nTab );

    const ScMergeFlagAttr& rMergeFlagAttr = pAttr->GetItem( ATTR_MERGE_FLAG );
    if ( rMergeFlagAttr.IsHorOverlapped() )
    {
        rStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_CELL );
        return ;
    }

    bool bValueData = false;
    OUString aContent;
    ScAddress aPos(nCol, nRow, nTab);
    ScRefCellValue aCell(*pDoc, aPos);
    switch (aCell.meType)
    {
        case CELLTYPE_NONE:
            bValueData = false;
        break;
        case CELLTYPE_EDIT:
        {
            bValueData = false;
            const EditTextObject* pObj = aCell.mpEditText;
            EditEngine& rEngine = GetEditEngine();
            rEngine.SetText(*pObj);
            aContent = rEngine.GetText(); // LineFeed in between paragraphs!
        }
        break;
        default:
        {
            bValueData = pDoc->HasValueData(aPos);
            sal_uInt32 nFormat = pAttr->GetNumberFormat(pFormatter);
            Color* pColor;
            aContent = ScCellFormat::GetString(*pDoc, aPos, nFormat, &pColor, *pFormatter);
        }
    }

    bool bResetAttr(false);

    const SvxHorJustifyItem&    rHorJustifyItem = pAttr->GetItem( ATTR_HOR_JUSTIFY );
    const SvxWeightItem&        rWeightItem     = pAttr->GetItem( ATTR_FONT_WEIGHT );
    const SvxPostureItem&       rPostureItem    = pAttr->GetItem( ATTR_FONT_POSTURE );
    const SvxUnderlineItem&     rUnderlineItem  = pAttr->GetItem( ATTR_FONT_UNDERLINE );

    const sal_Char* pChar;

    switch( rHorJustifyItem.GetValue() )
    {
        case SvxCellHorJustify::Standard:
            pChar = (bValueData ? OOO_STRING_SVTOOLS_RTF_QR : OOO_STRING_SVTOOLS_RTF_QL);
            break;
        case SvxCellHorJustify::Center:    pChar = OOO_STRING_SVTOOLS_RTF_QC;  break;
        case SvxCellHorJustify::Block:     pChar = OOO_STRING_SVTOOLS_RTF_QJ;  break;
        case SvxCellHorJustify::Right:     pChar = OOO_STRING_SVTOOLS_RTF_QR;  break;
        case SvxCellHorJustify::Left:
        case SvxCellHorJustify::Repeat:
        default:                        pChar = OOO_STRING_SVTOOLS_RTF_QL;  break;
    }
    rStrm.WriteCharPtr( pChar );

    if ( rWeightItem.GetWeight() >= WEIGHT_BOLD )
    {   // bold
        bResetAttr = true;
        rStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_B );
    }
    if ( rPostureItem.GetPosture() != ITALIC_NONE )
    {   // italic
        bResetAttr = true;
        rStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_I );
    }
    if ( rUnderlineItem.GetLineStyle() != LINESTYLE_NONE )
    {   // underline
        bResetAttr = true;
        rStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_UL );
    }

    rStrm.WriteChar( ' ' );
    RTFOutFuncs::Out_String( rStrm, aContent );
    rStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_CELL );

    if ( bResetAttr )
        rStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_PLAIN );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

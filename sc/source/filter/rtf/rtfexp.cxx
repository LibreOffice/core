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

#include <rtl/tencinfo.h>
#include <osl/thread.h>

#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/justifyitem.hxx>
#include <svtools/rtfout.hxx>
#include <svtools/rtfkeywd.hxx>
#include <tools/stream.hxx>

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
            m_pCellX( new sal_uLong[ pDoc->MaxCol()+2 ] )
{
}

ScRTFExport::~ScRTFExport()
{
}

void ScRTFExport::Write()
{
    rStrm.WriteChar( '{' ).WriteOString( OOO_STRING_SVTOOLS_RTF_RTF );
    rStrm.WriteOString( OOO_STRING_SVTOOLS_RTF_ANSI ).WriteOString( SAL_NEWLINE_STRING );

    m_aFontStrm.WriteChar( '{' ).WriteOString( OOO_STRING_SVTOOLS_RTF_FONTTBL );

    // Data
    for ( SCTAB nTab = aRange.aStart.Tab(); nTab <= aRange.aEnd.Tab(); nTab++ )
    {
        if ( nTab > aRange.aStart.Tab() )
            m_aDocStrm.WriteOString( OOO_STRING_SVTOOLS_RTF_PAR );
        WriteTab( nTab );
    }

    m_aFontStrm.WriteChar( '}' );
    m_aFontStrm.Seek(0);
    rStrm.WriteStream(m_aFontStrm);
    m_aDocStrm.Seek(0);
    rStrm.WriteStream(m_aDocStrm);
    rStrm.WriteChar( '}' ).WriteOString( SAL_NEWLINE_STRING );
}

void ScRTFExport::WriteTab( SCTAB nTab )
{
    m_aDocStrm.WriteChar( '{' ).WriteOString( SAL_NEWLINE_STRING );
    if ( pDoc->HasTable( nTab ) )
    {
        memset( &m_pCellX[0], 0, (pDoc->MaxCol()+2) * sizeof(sal_uLong) );
        SCCOL nCol;
        SCCOL nEndCol = aRange.aEnd.Col();
        for ( nCol = aRange.aStart.Col(); nCol <= nEndCol; nCol++ )
        {
            m_pCellX[nCol+1] = m_pCellX[nCol] + pDoc->GetColWidth( nCol, nTab );
        }

        SCROW nEndRow = aRange.aEnd.Row();
        for ( SCROW nRow = aRange.aStart.Row(); nRow <= nEndRow; nRow++ )
        {
            WriteRow( nTab, nRow );
        }
    }
    m_aDocStrm.WriteChar( '}' ).WriteOString( SAL_NEWLINE_STRING );
}

void ScRTFExport::WriteRow( SCTAB nTab, SCROW nRow )
{
    m_aDocStrm.WriteOString( OOO_STRING_SVTOOLS_RTF_TROWD ).WriteOString( OOO_STRING_SVTOOLS_RTF_TRGAPH ).WriteOString( "30" ).WriteOString( OOO_STRING_SVTOOLS_RTF_TRLEFT ).WriteOString( "-30" );
    m_aDocStrm.WriteOString( OOO_STRING_SVTOOLS_RTF_TRRH ).WriteOString( OString::number(pDoc->GetRowHeight(nRow, nTab)) );
    SCCOL nCol;
    SCCOL nEndCol = aRange.aEnd.Col();
    for ( nCol = aRange.aStart.Col(); nCol <= nEndCol; nCol++ )
    {
        const ScPatternAttr* pAttr = pDoc->GetPattern( nCol, nRow, nTab );
        const ScMergeAttr&      rMergeAttr      = pAttr->GetItem( ATTR_MERGE );
        const SvxVerJustifyItem& rVerJustifyItem= pAttr->GetItem( ATTR_VER_JUSTIFY );

        const char* pChar;

        if ( rMergeAttr.GetColMerge() != 0 )
            m_aDocStrm.WriteOString( OOO_STRING_SVTOOLS_RTF_CLMGF );
        else
        {
            const ScMergeFlagAttr& rMergeFlagAttr = pAttr->GetItem( ATTR_MERGE_FLAG );
            if ( rMergeFlagAttr.IsHorOverlapped() )
                m_aDocStrm.WriteOString( OOO_STRING_SVTOOLS_RTF_CLMRG );
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
            m_aDocStrm.WriteOString( pChar );

        m_aDocStrm.WriteOString( OOO_STRING_SVTOOLS_RTF_CELLX ).WriteOString( OString::number(m_pCellX[nCol+1]) );
        if ( (nCol & 0x0F) == 0x0F )
            m_aDocStrm.WriteOString( SAL_NEWLINE_STRING ); // Do not let lines get too long
    }
    m_aDocStrm.WriteOString( OOO_STRING_SVTOOLS_RTF_PARD ).WriteOString( OOO_STRING_SVTOOLS_RTF_PLAIN ).WriteOString( OOO_STRING_SVTOOLS_RTF_INTBL ).WriteOString( SAL_NEWLINE_STRING );

    sal_uInt64 nStrmPos = m_aDocStrm.Tell();
    for ( nCol = aRange.aStart.Col(); nCol <= nEndCol; nCol++ )
    {
        WriteCell( nTab, nRow, nCol );
        if ( m_aDocStrm.Tell() - nStrmPos > 255 )
        {   // Do not let lines get too long
            m_aDocStrm.WriteOString( SAL_NEWLINE_STRING );
            nStrmPos = m_aDocStrm.Tell();
        }
    }
    m_aDocStrm.WriteOString( OOO_STRING_SVTOOLS_RTF_ROW ).WriteOString( SAL_NEWLINE_STRING );
}

void ScRTFExport::WriteFontTable(const SvxFontItem& rFontItem, int nIndex)
{
    m_aFontStrm.WriteChar( '{' );
    m_aFontStrm.WriteOString( OOO_STRING_SVTOOLS_RTF_F );
    m_aFontStrm.WriteOString( OString::number(nIndex) );

    FontFamily eFamily = rFontItem.GetFamily();
    if (eFamily == FAMILY_DONTKNOW)
        m_aFontStrm.WriteOString( OOO_STRING_SVTOOLS_RTF_FNIL );
    else if (eFamily == FAMILY_DECORATIVE)
        m_aFontStrm.WriteOString( OOO_STRING_SVTOOLS_RTF_FDECOR );
    else if (eFamily == FAMILY_MODERN)
        m_aFontStrm.WriteOString( OOO_STRING_SVTOOLS_RTF_FMODERN );
    else if (eFamily == FAMILY_ROMAN)
        m_aFontStrm.WriteOString( OOO_STRING_SVTOOLS_RTF_FROMAN );
    else if (eFamily == FAMILY_SCRIPT)
        m_aFontStrm.WriteOString( OOO_STRING_SVTOOLS_RTF_FSCRIPT );
    else if (eFamily == FAMILY_SWISS)
        m_aFontStrm.WriteOString( OOO_STRING_SVTOOLS_RTF_FSWISS );

    m_aFontStrm.WriteOString( OOO_STRING_SVTOOLS_RTF_FPRQ );

    sal_uInt16 nVal = 0;
    FontPitch ePitch = rFontItem.GetPitch();
    if ( ePitch == PITCH_FIXED )
        nVal = 1;
    else if ( ePitch == PITCH_VARIABLE )
        nVal = 2;
    m_aFontStrm.WriteOString( OString::number(nVal) );

    rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252;
    rtl_TextEncoding eChrSet = rFontItem.GetCharSet();
    if (IsOpenSymbol(rFontItem.GetFamilyName()))
        eChrSet = RTL_TEXTENCODING_UTF8;
    else if( RTL_TEXTENCODING_DONTKNOW == eChrSet )
        eChrSet = osl_getThreadTextEncoding();

    m_aFontStrm.WriteOString( OOO_STRING_SVTOOLS_RTF_FCHARSET );
    m_aFontStrm.WriteOString( OString::number(rtl_getBestWindowsCharsetFromTextEncoding( eChrSet )) );

    m_aFontStrm.WriteChar( ' ' );
    RTFOutFuncs::Out_String( m_aFontStrm, rFontItem.GetFamilyName(), eDestEnc );
    m_aFontStrm.WriteOString( ";}" );
}

int ScRTFExport::AddFont(const SvxFontItem& rFontItem)
{
    auto nRet = m_pFontTable.size();
    auto itFont(m_pFontTable.find(rFontItem.GetFamilyName()));
    if (itFont == m_pFontTable.end())
    {
        m_pFontTable[rFontItem.GetFamilyName()] = nRet;
        WriteFontTable(rFontItem, nRet);
    }
    else
    {
        nRet = itFont->second;
    }

    return nRet;
}

void ScRTFExport::WriteCell( SCTAB nTab, SCROW nRow, SCCOL nCol )
{
    const ScPatternAttr* pAttr = pDoc->GetPattern( nCol, nRow, nTab );

    const ScMergeFlagAttr& rMergeFlagAttr = pAttr->GetItem( ATTR_MERGE_FLAG );
    if ( rMergeFlagAttr.IsHorOverlapped() )
    {
        m_aDocStrm.WriteOString( OOO_STRING_SVTOOLS_RTF_CELL );
        return ;
    }

    bool bValueData = false;
    OUString aContent;
    ScAddress aPos(nCol, nRow, nTab);
    ScRefCellValue aCell(*pDoc, aPos);
    switch (aCell.getType())
    {
        case CELLTYPE_NONE:
            bValueData = false;
        break;
        case CELLTYPE_EDIT:
        {
            bValueData = false;
            const EditTextObject* pObj = aCell.getEditText();
            EditEngine& rEngine = GetEditEngine();
            rEngine.SetText(*pObj);
            aContent = rEngine.GetText(); // LineFeed in between paragraphs!
        }
        break;
        default:
        {
            bValueData = pDoc->HasValueData(aPos);
            sal_uInt32 nFormat = pAttr->GetNumberFormat(pFormatter);
            const Color* pColor;
            aContent = ScCellFormat::GetString(*pDoc, aPos, nFormat, &pColor, nullptr);
        }
    }

    bool bResetAttr(false);

    const SvxFontItem&          rFontItem       = pAttr->GetItem( ATTR_FONT );
    const SvxHorJustifyItem&    rHorJustifyItem = pAttr->GetItem( ATTR_HOR_JUSTIFY );
    const SvxWeightItem&        rWeightItem     = pAttr->GetItem( ATTR_FONT_WEIGHT );
    const SvxPostureItem&       rPostureItem    = pAttr->GetItem( ATTR_FONT_POSTURE );
    const SvxUnderlineItem&     rUnderlineItem  = pAttr->GetItem( ATTR_FONT_UNDERLINE );

    m_aDocStrm.WriteOString( OOO_STRING_SVTOOLS_RTF_F )
        .WriteOString( OString::number(AddFont(rFontItem)) );

    const char* pChar;

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
    m_aDocStrm.WriteOString( pChar );

    if ( rWeightItem.GetWeight() >= WEIGHT_BOLD )
    {   // bold
        bResetAttr = true;
        m_aDocStrm.WriteOString( OOO_STRING_SVTOOLS_RTF_B );
    }
    if ( rPostureItem.GetPosture() != ITALIC_NONE )
    {   // italic
        bResetAttr = true;
        m_aDocStrm.WriteOString( OOO_STRING_SVTOOLS_RTF_I );
    }
    if ( rUnderlineItem.GetLineStyle() != LINESTYLE_NONE )
    {   // underline
        bResetAttr = true;
        m_aDocStrm.WriteOString( OOO_STRING_SVTOOLS_RTF_UL );
    }

    m_aDocStrm.WriteChar( ' ' );
    RTFOutFuncs::Out_String( m_aDocStrm, aContent );
    m_aDocStrm.WriteOString( OOO_STRING_SVTOOLS_RTF_CELL );

    if ( bResetAttr )
        m_aDocStrm.WriteOString( OOO_STRING_SVTOOLS_RTF_PLAIN );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

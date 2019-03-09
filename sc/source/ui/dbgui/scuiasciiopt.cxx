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

#undef SC_DLLIMPLEMENTATION

#include <svx/txencbox.hxx>
#include <vcl/button.hxx>
#include <vcl/combobox.hxx>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/ptrstyle.hxx>

#include <global.hxx>
#include <scresid.hxx>
#include <impex.hxx>
#include <scuiasciiopt.hxx>
#include <strings.hrc>
#include <strings.hxx>
#include <csvtablebox.hxx>
#include <osl/thread.h>
#include <unotools/transliterationwrapper.hxx>

#include <optutil.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <miscuno.hxx>
#include <osl/diagnose.h>

//! TODO make dynamic
const SCSIZE ASCIIDLG_MAXROWS                = MAXROWCOUNT;

using namespace com::sun::star::uno;

// Defines - CSV Import Preserve Options
enum CSVImportOptionsIndex
{
    CSVIO_MergeDelimiters = 0,
    CSVIO_Separators,
    CSVIO_TextSeparators,
    CSVIO_FixedWidth,
    CSVIO_RemoveSpace,
    CSVIO_FromRow,
    CSVIO_Text2ColSkipEmptyCells = CSVIO_FromRow,
    CSVIO_CharSet,
    CSVIO_QuotedAsText,
    CSVIO_DetectSpecialNum,
    CSVIO_Language,
    CSVIO_PasteSkipEmptyCells
};

const ::std::vector<OUString> CSVImportOptionNames =
{
    "MergeDelimiters",
    "Separators",
    "TextSeparators",
    "FixedWidth",
    "RemoveSpace",
    "FromRow",
    "CharSet",
    "QuotedFieldAsText",
    "DetectSpecialNumbers",
    "Language",
    "SkipEmptyCells"
};
const OUString aSep_Path =           "Office.Calc/Dialogs/CSVImport";
const OUString aSep_Path_Clpbrd =    "Office.Calc/Dialogs/ClipboardTextImport";
const OUString aSep_Path_Text2Col =  "Office.Calc/Dialogs/TextToColumnsImport";

namespace {
CSVImportOptionsIndex getSkipEmptyCellsIndex( ScImportAsciiCall eCall )
{
    return eCall == SC_TEXTTOCOLUMNS ? CSVIO_Text2ColSkipEmptyCells : CSVIO_PasteSkipEmptyCells;
}
}

static void lcl_FillCombo( ComboBox& rCombo, const OUString& rList, sal_Unicode cSelect )
{
    OUString aStr;
    if (!rList.isEmpty())
    {
        sal_Int32 nIdx {0};
        do
        {
            const OUString sEntry {rList.getToken(0, '\t', nIdx)};
            rCombo.InsertEntry( sEntry );
            if (nIdx>0 && static_cast<sal_Unicode>(rList.getToken(0, '\t', nIdx).toInt32()) == cSelect)
                aStr = sEntry;
        }
        while (nIdx>0);
    }

    if ( cSelect )
    {
        if (aStr.isEmpty())
            aStr = OUString(cSelect);         // Ascii

        rCombo.SetText(aStr);
    }
}

static sal_Unicode lcl_CharFromCombo( const ComboBox& rCombo, const OUString& rList )
{
    sal_Unicode c = 0;
    OUString aStr = rCombo.GetText();
    if ( !aStr.isEmpty() && !rList.isEmpty() )
    {
        sal_Int32 nIdx {0};
        OUString sToken {rList.getToken(0, '\t', nIdx)};
        while (nIdx>0)
        {
            if ( ScGlobal::GetpTransliteration()->isEqual( aStr, sToken ) )
            {
                sal_Int32 nTmpIdx {nIdx};
                c = static_cast<sal_Unicode>(rList.getToken(0, '\t', nTmpIdx).toInt32());
            }
            // Skip to next token at even position
            sToken = rList.getToken(1, '\t', nIdx);
        }
        if (!c)
        {
            sal_Unicode cFirst = aStr[0];
            // #i24235# first try the first character of the string directly
            if( (aStr.getLength() == 1) || (cFirst < '0') || (cFirst > '9') )
                c = cFirst;
            else    // keep old behaviour for compatibility (i.e. "39" -> "'")
                c = static_cast<sal_Unicode>(aStr.toInt32());       // Ascii
        }
    }
    return c;
}

static void lcl_CreatePropertiesNames ( OUString& rSepPath, Sequence<OUString>& rNames, ScImportAsciiCall eCall )
{
    sal_Int32 nProperties = 0;

    switch(eCall)
    {
        case SC_IMPORTFILE:
            rSepPath = aSep_Path;
            nProperties = 10;
            break;
        case SC_PASTETEXT:
            rSepPath = aSep_Path_Clpbrd;
            nProperties = 11;
            break;
        case SC_TEXTTOCOLUMNS:
        default:
            rSepPath = aSep_Path_Text2Col;
            nProperties = 6;
            break;
    }
    rNames.realloc( nProperties );
    OUString* pNames = rNames.getArray();
    pNames[ CSVIO_MergeDelimiters ] =   CSVImportOptionNames[ CSVIO_MergeDelimiters ];
    pNames[ CSVIO_Separators ] =        CSVImportOptionNames[ CSVIO_Separators ];
    pNames[ CSVIO_TextSeparators ] =    CSVImportOptionNames[ CSVIO_TextSeparators ];
    pNames[ CSVIO_FixedWidth ] =        CSVImportOptionNames[ CSVIO_FixedWidth ];
    pNames[ CSVIO_RemoveSpace ] =       CSVImportOptionNames[ CSVIO_RemoveSpace ];
    if (eCall != SC_TEXTTOCOLUMNS)
    {
        pNames[ CSVIO_FromRow ] =       CSVImportOptionNames[ CSVIO_FromRow ];
        pNames[ CSVIO_CharSet ] =       CSVImportOptionNames[ CSVIO_CharSet ];
        pNames[ CSVIO_QuotedAsText ] =  CSVImportOptionNames[ CSVIO_QuotedAsText ];
        pNames[ CSVIO_DetectSpecialNum ] = CSVImportOptionNames[ CSVIO_DetectSpecialNum ];
        pNames[ CSVIO_Language ] =      CSVImportOptionNames[ CSVIO_Language ];
    }
    if (eCall != SC_IMPORTFILE)
    {
        const sal_Int32 nSkipEmptyCells = getSkipEmptyCellsIndex(eCall);
        assert( nSkipEmptyCells < rNames.getLength());
        pNames[ nSkipEmptyCells ] = CSVImportOptionNames[ CSVIO_PasteSkipEmptyCells ];
    }
}

static void lcl_LoadSeparators( OUString& rFieldSeparators, OUString& rTextSeparators,
                             bool& rMergeDelimiters, bool& rQuotedAsText, bool& rDetectSpecialNum,
                             bool& rFixedWidth, sal_Int32& rFromRow, sal_Int32& rCharSet,
                             sal_Int32& rLanguage, bool& rSkipEmptyCells, bool& rRemoveSpace, ScImportAsciiCall eCall )
{
    Sequence<Any>aValues;
    const Any *pProperties;
    Sequence<OUString> aNames;
    OUString aSepPath;
    lcl_CreatePropertiesNames ( aSepPath, aNames, eCall);
    ScLinkConfigItem aItem( aSepPath );
    aValues = aItem.GetProperties( aNames );
    pProperties = aValues.getConstArray();

    if( pProperties[ CSVIO_MergeDelimiters ].hasValue() )
        rMergeDelimiters = ScUnoHelpFunctions::GetBoolFromAny( pProperties[ CSVIO_MergeDelimiters ] );

    if( pProperties[ CSVIO_RemoveSpace ].hasValue() )
        rRemoveSpace = ScUnoHelpFunctions::GetBoolFromAny( pProperties[ CSVIO_RemoveSpace ] );

    if( pProperties[ CSVIO_Separators ].hasValue() )
        pProperties[ CSVIO_Separators ] >>= rFieldSeparators;

    if( pProperties[ CSVIO_TextSeparators ].hasValue() )
        pProperties[ CSVIO_TextSeparators ] >>= rTextSeparators;

    if( pProperties[ CSVIO_FixedWidth ].hasValue() )
        rFixedWidth = ScUnoHelpFunctions::GetBoolFromAny( pProperties[ CSVIO_FixedWidth ] );

    if (eCall != SC_TEXTTOCOLUMNS)
    {
        if( pProperties[ CSVIO_FromRow ].hasValue() )
            pProperties[ CSVIO_FromRow ] >>= rFromRow;

        if( pProperties[ CSVIO_CharSet ].hasValue() )
            pProperties[ CSVIO_CharSet ] >>= rCharSet;

        if ( pProperties[ CSVIO_QuotedAsText ].hasValue() )
            pProperties[ CSVIO_QuotedAsText ] >>= rQuotedAsText;

        if ( pProperties[ CSVIO_DetectSpecialNum ].hasValue() )
            pProperties[ CSVIO_DetectSpecialNum ] >>= rDetectSpecialNum;

        if ( pProperties[ CSVIO_Language ].hasValue() )
            pProperties[ CSVIO_Language ] >>= rLanguage;
    }
    if (eCall != SC_IMPORTFILE)
    {
        const sal_Int32 nSkipEmptyCells = getSkipEmptyCellsIndex(eCall);
        assert( nSkipEmptyCells < aValues.getLength());
        if ( pProperties[nSkipEmptyCells].hasValue() )
            rSkipEmptyCells = ScUnoHelpFunctions::GetBoolFromAny( pProperties[nSkipEmptyCells] );
    }
}

static void lcl_SaveSeparators(
    const OUString& sFieldSeparators, const OUString& sTextSeparators, bool bMergeDelimiters, bool bQuotedAsText,
    bool bDetectSpecialNum, bool bFixedWidth, sal_Int32 nFromRow,
    sal_Int32 nCharSet, sal_Int32 nLanguage, bool bSkipEmptyCells, bool bRemoveSpace, ScImportAsciiCall eCall )
{
    Sequence<Any> aValues;
    Any *pProperties;
    Sequence<OUString> aNames;
    OUString aSepPath;
    lcl_CreatePropertiesNames ( aSepPath, aNames, eCall );
    ScLinkConfigItem aItem( aSepPath );
    aValues = aItem.GetProperties( aNames );
    pProperties = aValues.getArray();

    pProperties[ CSVIO_MergeDelimiters ] <<= bMergeDelimiters;
    pProperties[ CSVIO_RemoveSpace ] <<= bRemoveSpace;
    pProperties[ CSVIO_Separators ] <<= sFieldSeparators;
    pProperties[ CSVIO_TextSeparators ] <<= sTextSeparators;
    pProperties[ CSVIO_FixedWidth ] <<= bFixedWidth;
    if (eCall != SC_TEXTTOCOLUMNS)
    {
        pProperties[ CSVIO_FromRow ] <<= nFromRow;
        pProperties[ CSVIO_CharSet ] <<= nCharSet;
        pProperties[ CSVIO_QuotedAsText ] <<= bQuotedAsText;
        pProperties[ CSVIO_DetectSpecialNum ] <<= bDetectSpecialNum;
        pProperties[ CSVIO_Language ] <<= nLanguage;
    }
    if (eCall != SC_IMPORTFILE)
    {
        const sal_Int32 nSkipEmptyCells = getSkipEmptyCellsIndex(eCall);
        assert( nSkipEmptyCells < aValues.getLength());
        pProperties[ nSkipEmptyCells ] <<= bSkipEmptyCells;
    }

    aItem.PutProperties(aNames, aValues);
}

static constexpr OUStringLiteral gaTextSepList(SCSTR_TEXTSEP);

ScImportAsciiDlg::ScImportAsciiDlg( vcl::Window* pParent, const OUString& aDatName,
                                    SvStream* pInStream, ScImportAsciiCall eCall ) :
        ModalDialog (pParent, "TextImportCsvDialog",
            "modules/scalc/ui/textimportcsv.ui"),
        mpDatStream  ( pInStream ),
        mnStreamPos( pInStream ? pInStream->Tell() : 0 ),

        mnRowPosCount(0),

        mcTextSep   ( ScAsciiOptions::cDefaultTextSep ),
        meCall(eCall),
        mbDetectSpaceSep(eCall != SC_TEXTTOCOLUMNS)
{
    get(pFtCharSet, "textcharset");
    get(pLbCharSet, "charset");
    pLbCharSet->SetStyle(pLbCharSet->GetStyle() | WB_SORT);
    get(pFtCustomLang, "textlanguage");
    get(pLbCustomLang, "language");
    pLbCustomLang->SetStyle(pLbCustomLang->GetStyle() | WB_SORT);
    get(pFtRow, "textfromrow");
    get(pNfRow, "fromrow");
    get(pRbFixed, "tofixedwidth");
    get(pRbSeparated, "toseparatedby");
    get(pCkbTab, "tab");
    get(pCkbSemicolon, "semicolon");
    get(pCkbComma, "comma");
    get(pCkbSpace, "space");
    get(pCkbRemoveSpace, "removespace");
    get(pCkbOther, "other");
    get(pEdOther, "inputother");
    get(pCkbAsOnce, "mergedelimiters");
    get(pFtTextSep, "texttextdelimiter");
    get(pCbTextSep, "textdelimiter");
    get(pCkbQuotedAsText, "quotedfieldastext");
    get(pCkbDetectNumber, "detectspecialnumbers");
    get(pCkbSkipEmptyCells, "skipemptycells");
    get(pFtType, "textcolumntype");
    get(pLbType, "columntype");
    get(mpTableBox, "scrolledwindowcolumntype");

    OUString aName = GetText();
    switch (meCall)
    {
        case SC_TEXTTOCOLUMNS:
            SetText( get<FixedText>("textalttitle")->GetText() );
            break;
        case SC_IMPORTFILE:
            aName += " - [" + aDatName + "]";
            SetText( aName );
            break;
        default:
            break;
    }

    // To be able to prefill the correct values based on the file extension
    bool bIsTSV = (aDatName.endsWithIgnoreAsciiCase(".tsv") || aDatName.endsWithIgnoreAsciiCase(".tab"));

    // Default options are set in officecfg/registry/schema/org/openoffice/Office/Calc.xcs
    OUString sFieldSeparators(",;\t");
    OUString sTextSeparators(mcTextSep);
    bool bMergeDelimiters = false;
    bool bFixedWidth = false;
    bool bQuotedFieldAsText = false;
    bool bDetectSpecialNum = true;
    bool bSkipEmptyCells = true;
    bool bRemoveSpace = false;
    sal_Int32 nFromRow = 1;
    sal_Int32 nCharSet = -1;
    sal_Int32 nLanguage = 0;
    lcl_LoadSeparators (sFieldSeparators, sTextSeparators, bMergeDelimiters,
                         bQuotedFieldAsText, bDetectSpecialNum, bFixedWidth, nFromRow,
                         nCharSet, nLanguage, bSkipEmptyCells, bRemoveSpace, meCall);
    // load from saved settings
    maFieldSeparators = sFieldSeparators;

    if( bMergeDelimiters && !bIsTSV )
        pCkbAsOnce->Check();
    if (bQuotedFieldAsText)
        pCkbQuotedAsText->Check();
    if (bRemoveSpace)
        pCkbRemoveSpace->Check();
    if (bDetectSpecialNum)
        pCkbDetectNumber->Check();
    if (bSkipEmptyCells)
        pCkbSkipEmptyCells->Check();
    if( bFixedWidth && !bIsTSV )
        pRbFixed->Check();
    if( nFromRow != 1 )
        pNfRow->SetValue( nFromRow );

    if ( bIsTSV )
        pCkbTab->Check();
    else
        SetSeparators(); // Set Separators in the dialog from maFieldSeparators (empty are not set)

    // Get Separators from the dialog (empty are set from default)
    maFieldSeparators = GetSeparators();

    // Clipboard is always Unicode, else detect.
    rtl_TextEncoding ePreselectUnicode = (meCall == SC_IMPORTFILE ?
            RTL_TEXTENCODING_DONTKNOW : RTL_TEXTENCODING_UNICODE);
    // Sniff for Unicode / not
    if( ePreselectUnicode == RTL_TEXTENCODING_DONTKNOW && mpDatStream )
    {
        Seek( 0 );
        mpDatStream->StartReadingUnicodeText( RTL_TEXTENCODING_DONTKNOW );
        sal_uLong nUniPos = mpDatStream->Tell();
        switch (nUniPos)
        {
            case 2:
                ePreselectUnicode = RTL_TEXTENCODING_UNICODE;   // UTF-16
                break;
            case 3:
                ePreselectUnicode = RTL_TEXTENCODING_UTF8;      // UTF-8
                break;
            case 0:
                {
                    sal_uInt16 n;
                    mpDatStream->ReadUInt16( n );
                    // Assume that normal ASCII/ANSI/ISO/etc. text doesn't start with
                    // control characters except CR,LF,TAB
                    if ( (n & 0xff00) < 0x2000 )
                    {
                        switch ( n & 0xff00 )
                        {
                            case 0x0900 :
                            case 0x0a00 :
                            case 0x0d00 :
                                break;
                            default:
                                ePreselectUnicode = RTL_TEXTENCODING_UNICODE;   // UTF-16
                        }
                    }
                    mpDatStream->Seek(0);
                }
                break;
            default:
                ;   // nothing
        }
        mnStreamPos = mpDatStream->Tell();
    }

    pNfRow->SetModifyHdl( LINK( this, ScImportAsciiDlg, FirstRowHdl ) );

    // *** Separator characters ***
    lcl_FillCombo( *pCbTextSep, gaTextSepList, mcTextSep );
    pCbTextSep->SetText( sTextSeparators );

    Link<Edit&,void> aSeparatorHdl = LINK( this, ScImportAsciiDlg, SeparatorEditHdl );
    Link<Button*,void> aSeparatorClickHdl =LINK( this, ScImportAsciiDlg, SeparatorClickHdl );
    pCbTextSep->SetSelectHdl( LINK( this, ScImportAsciiDlg, SeparatorComboBoxHdl ) );
    pCbTextSep->SetModifyHdl( aSeparatorHdl );
    pCkbTab->SetClickHdl( aSeparatorClickHdl );
    pCkbSemicolon->SetClickHdl( aSeparatorClickHdl );
    pCkbComma->SetClickHdl( aSeparatorClickHdl );
    pCkbAsOnce->SetClickHdl( aSeparatorClickHdl );
    pCkbQuotedAsText->SetClickHdl( aSeparatorClickHdl );
    pCkbDetectNumber->SetClickHdl( aSeparatorClickHdl );
    pCkbSkipEmptyCells->SetClickHdl( aSeparatorClickHdl );
    pCkbSpace->SetClickHdl( aSeparatorClickHdl );
    pCkbRemoveSpace->SetClickHdl( aSeparatorClickHdl );
    pCkbOther->SetClickHdl( aSeparatorClickHdl );
    pEdOther->SetModifyHdl( aSeparatorHdl );

    // *** text encoding ListBox ***
    // all encodings allowed, including Unicode, but subsets are excluded
    pLbCharSet->FillFromTextEncodingTable( true );
    // Insert one "SYSTEM" entry for compatibility in AsciiOptions and system
    // independent document linkage.
    pLbCharSet->InsertTextEncoding( RTL_TEXTENCODING_DONTKNOW, ScResId( SCSTR_CHARSET_USER ) );
    if ( ePreselectUnicode == RTL_TEXTENCODING_DONTKNOW )
    {
        rtl_TextEncoding eSystemEncoding = osl_getThreadTextEncoding();
        // Prefer UTF-8, as UTF-16 would have already been detected from the stream.
        // This gives a better chance that the file is going to be opened correctly.
        if ( ( eSystemEncoding == RTL_TEXTENCODING_UNICODE ) && mpDatStream )
            eSystemEncoding = RTL_TEXTENCODING_UTF8;
        pLbCharSet->SelectTextEncoding( eSystemEncoding );
    }
    else
    {
        pLbCharSet->SelectTextEncoding( ePreselectUnicode );
    }

    if( nCharSet >= 0 && ePreselectUnicode == RTL_TEXTENCODING_DONTKNOW )
        pLbCharSet->SelectEntryPos( static_cast<sal_uInt16>(nCharSet) );

    SetSelectedCharSet();
    pLbCharSet->SetSelectHdl( LINK( this, ScImportAsciiDlg, CharSetHdl ) );

    pLbCustomLang->SetLanguageList(
        SvxLanguageListFlags::ALL | SvxLanguageListFlags::ONLY_KNOWN, false, false);
    pLbCustomLang->InsertLanguage(LANGUAGE_SYSTEM);
    pLbCustomLang->SelectLanguage(static_cast<LanguageType>(nLanguage));

    // *** column type ListBox ***
    OUString aColumnUser( ScResId( SCSTR_COLUMN_USER ) );
    for (sal_Int32 nIdx {0}; nIdx>=0; )
    {
        pLbType->InsertEntry( aColumnUser.getToken( 0, ';', nIdx ) );
    }

    pLbType->SetSelectHdl( LINK( this, ScImportAsciiDlg, LbColTypeHdl ) );
    pFtType->Disable();
    pLbType->Disable();

    // *** table box preview ***
    mpTableBox->Init();
    mpTableBox->SetUpdateTextHdl( LINK( this, ScImportAsciiDlg, UpdateTextHdl ) );
    mpTableBox->InitTypes( *pLbType );
    mpTableBox->SetColTypeHdl( LINK( this, ScImportAsciiDlg, ColTypeHdl ) );

    pRbSeparated->SetClickHdl( LINK( this, ScImportAsciiDlg, RbSepFixHdl ) );
    pRbFixed->SetClickHdl( LINK( this, ScImportAsciiDlg, RbSepFixHdl ) );

    SetupSeparatorCtrls();
    RbSepFixHdl( pRbFixed );

    UpdateVertical();

    mpTableBox->Execute( CSVCMD_NEWCELLTEXTS );

    if (meCall == SC_TEXTTOCOLUMNS)
    {
        pFtCharSet->Disable();
        pLbCharSet->Disable();
        pFtCustomLang->Disable();
        pLbCustomLang->SelectLanguage(LANGUAGE_SYSTEM);
        pLbCustomLang->Disable();
        pFtRow->Disable();
        pNfRow->Disable();

        // Quoted field as text option is not used for text-to-columns mode.
        pCkbQuotedAsText->Check(false);
        pCkbQuotedAsText->Disable();

        // Always detect special numbers for text-to-columns mode.
        pCkbDetectNumber->Check();
        pCkbDetectNumber->Disable();
    }
    if (meCall == SC_IMPORTFILE)
    {
        //Empty cells in imported file are empty
        pCkbSkipEmptyCells->Check(false);
        pCkbSkipEmptyCells->Hide();
    }
}

ScImportAsciiDlg::~ScImportAsciiDlg()
{
    disposeOnce();
}

void ScImportAsciiDlg::dispose()
{
    mpRowPosArray.reset();
    pFtCharSet.clear();
    pLbCharSet.clear();
    pFtCustomLang.clear();
    pLbCustomLang.clear();
    pFtRow.clear();
    pNfRow.clear();
    pRbFixed.clear();
    pRbSeparated.clear();
    pCkbTab.clear();
    pCkbSemicolon.clear();
    pCkbComma.clear();
    pCkbSpace.clear();
    pCkbRemoveSpace.clear();
    pCkbOther.clear();
    pEdOther.clear();
    pCkbAsOnce.clear();
    pFtTextSep.clear();
    pCbTextSep.clear();
    pCkbQuotedAsText.clear();
    pCkbDetectNumber.clear();
    pCkbSkipEmptyCells.clear();
    pFtType.clear();
    pLbType.clear();
    mpTableBox.clear();
    ModalDialog::dispose();
}

bool ScImportAsciiDlg::GetLine( sal_uLong nLine, OUString &rText, sal_Unicode& rcDetectSep )
{
    if (nLine >= ASCIIDLG_MAXROWS || !mpDatStream)
        return false;

    bool bRet = true;
    bool bFixed = pRbFixed->IsChecked();

    if (!mpRowPosArray)
        mpRowPosArray.reset( new sal_uLong[ASCIIDLG_MAXROWS + 2] );

    if (!mnRowPosCount) // complete re-fresh
    {
        memset( mpRowPosArray.get(), 0, sizeof(mpRowPosArray[0]) * (ASCIIDLG_MAXROWS+2));

        Seek(0);
        mpDatStream->StartReadingUnicodeText( mpDatStream->GetStreamCharSet() );

        mnStreamPos = mpDatStream->Tell();
        mpRowPosArray[mnRowPosCount] = mnStreamPos;
    }

    if (nLine >= mnRowPosCount)
    {
        // need to work out some more line information
        do
        {
            if (!Seek(mpRowPosArray[mnRowPosCount]) || !mpDatStream->good())
            {
                bRet = false;
                break;
            }
            rText = ReadCsvLine(*mpDatStream, !bFixed, maFieldSeparators,
                    mcTextSep, rcDetectSep);
            mnStreamPos = mpDatStream->Tell();
            mpRowPosArray[++mnRowPosCount] = mnStreamPos;
        } while (nLine >= mnRowPosCount && mpDatStream->good());
        if (mpDatStream->eof() &&
                mnStreamPos == mpRowPosArray[mnRowPosCount-1])
        {
            // the very end, not even an empty line read
            bRet = false;
            --mnRowPosCount;
        }
    }
    else
    {
        Seek( mpRowPosArray[nLine]);
        rText = ReadCsvLine(*mpDatStream, !bFixed, maFieldSeparators, mcTextSep, rcDetectSep);
        mnStreamPos = mpDatStream->Tell();
    }

    //  If the file content isn't unicode, ReadUniStringLine
    //  may try to seek beyond the file's end and cause a CANTSEEK error
    //  (depending on the stream type). The error code has to be cleared,
    //  or further read operations (including non-unicode) will fail.
    if ( mpDatStream->GetError() == ERRCODE_IO_CANTSEEK )
        mpDatStream->ResetError();

    ScImportExport::EmbeddedNullTreatment( rText);

    return bRet;
}

void ScImportAsciiDlg::GetOptions( ScAsciiOptions& rOpt )
{
    rOpt.SetCharSet( meCharSet );
    rOpt.SetCharSetSystem( mbCharSetSystem );
    rOpt.SetLanguage(pLbCustomLang->GetSelectedLanguage());
    rOpt.SetFixedLen( pRbFixed->IsChecked() );
    rOpt.SetStartRow( static_cast<long>(pNfRow->GetValue()) );
    mpTableBox->FillColumnData( rOpt );
    if( pRbSeparated->IsChecked() )
    {
        rOpt.SetFieldSeps( GetSeparators() );
        rOpt.SetMergeSeps( pCkbAsOnce->IsChecked() );
        rOpt.SetRemoveSpace( pCkbRemoveSpace->IsChecked() );
        rOpt.SetTextSep( lcl_CharFromCombo( *pCbTextSep, gaTextSepList ) );
    }

    rOpt.SetQuotedAsText(pCkbQuotedAsText->IsChecked());
    rOpt.SetDetectSpecialNumber(pCkbDetectNumber->IsChecked());
    rOpt.SetSkipEmptyCells(pCkbSkipEmptyCells->IsChecked());
}

void ScImportAsciiDlg::SaveParameters()
{
    lcl_SaveSeparators( maFieldSeparators, pCbTextSep->GetText(), pCkbAsOnce->IsChecked(),
                     pCkbQuotedAsText->IsChecked(), pCkbDetectNumber->IsChecked(),
                     pRbFixed->IsChecked(),
                     static_cast<sal_Int32>(pNfRow->GetValue()),
                     pLbCharSet->GetSelectedEntryPos(),
                     static_cast<sal_uInt16>(pLbCustomLang->GetSelectedLanguage()),
                     pCkbSkipEmptyCells->IsChecked(), pCkbRemoveSpace->IsChecked(), meCall );
}

void ScImportAsciiDlg::SetSeparators()
{
    OString sString(OUStringToOString(maFieldSeparators,
        RTL_TEXTENCODING_MS_1252));
    const sal_Char *aSep = sString.getStr();
    sal_Int32 len = maFieldSeparators.getLength();
    for (int i = 0; i < len; ++i)
    {
        switch( aSep[i] )
        {
            case '\t':  pCkbTab->Check();        break;
            case ';':   pCkbSemicolon->Check();  break;
            case ',':   pCkbComma->Check();      break;
            case ' ':   pCkbSpace->Check();      break;
            default:
                pCkbOther->Check();
                pEdOther->SetText( pEdOther->GetText() + OUString( aSep[i] ) );
        }
    }
}

void ScImportAsciiDlg::SetSelectedCharSet()
{
    meCharSet = pLbCharSet->GetSelectTextEncoding();
    mbCharSetSystem = (meCharSet == RTL_TEXTENCODING_DONTKNOW);
    if( mbCharSetSystem )
        meCharSet = osl_getThreadTextEncoding();
}

OUString ScImportAsciiDlg::GetSeparators() const
{
    OUString aSepChars;
    if( pCkbTab->IsChecked() )
        aSepChars += "\t";
    if( pCkbSemicolon->IsChecked() )
        aSepChars += ";";
    if( pCkbComma->IsChecked() )
        aSepChars += ",";
    if( pCkbSpace->IsChecked() )
        aSepChars += " ";
    if( pCkbOther->IsChecked() )
        aSepChars += pEdOther->GetText();
    return aSepChars;
}

void ScImportAsciiDlg::SetupSeparatorCtrls()
{
    bool bEnable = pRbSeparated->IsChecked();
    pCkbTab->Enable( bEnable );
    pCkbSemicolon->Enable( bEnable );
    pCkbComma->Enable( bEnable );
    pCkbSpace->Enable( bEnable );
    pCkbRemoveSpace->Enable( bEnable );
    pCkbOther->Enable( bEnable );
    pEdOther->Enable( bEnable );
    pCkbAsOnce->Enable( bEnable );
    pFtTextSep->Enable( bEnable );
    pCbTextSep->Enable( bEnable );
}

void ScImportAsciiDlg::UpdateVertical()
{
    mnRowPosCount = 0;
    if (mpDatStream)
        mpDatStream->SetStreamCharSet(meCharSet);
}

IMPL_LINK( ScImportAsciiDlg, RbSepFixHdl, Button*, pButton, void )
{
    OSL_ENSURE( pButton, "ScImportAsciiDlg::RbSepFixHdl - missing sender" );

    if( (pButton == pRbFixed) || (pButton == pRbSeparated) )
    {
        SetPointer( PointerStyle::Wait );
        if( pRbFixed->IsChecked() )
            mpTableBox->SetFixedWidthMode();
        else
            mpTableBox->SetSeparatorsMode();
        SetPointer( PointerStyle::Arrow );

        SetupSeparatorCtrls();
    }
}

IMPL_LINK( ScImportAsciiDlg, SeparatorClickHdl, Button*, pCtrl, void )
{
    SeparatorHdl(pCtrl);
}
IMPL_LINK( ScImportAsciiDlg, SeparatorComboBoxHdl, ComboBox&, rCtrl, void )
{
    SeparatorHdl(&rCtrl);
}
IMPL_LINK( ScImportAsciiDlg, SeparatorEditHdl, Edit&, rEdit, void )
{
    SeparatorHdl(&rEdit);
}
void ScImportAsciiDlg::SeparatorHdl( const Control* pCtrl )
{
    OSL_ENSURE( pCtrl, "ScImportAsciiDlg::SeparatorHdl - missing sender" );
    OSL_ENSURE( !pRbFixed->IsChecked(), "ScImportAsciiDlg::SeparatorHdl - not allowed in fixed width" );

    /*  #i41550# First update state of the controls. The GetSeparators()
        function needs final state of the check boxes. */
    if( (pCtrl == pCkbOther) && pCkbOther->IsChecked() )
        pEdOther->GrabFocus();
    else if( pCtrl == pEdOther )
        pCkbOther->Check( !pEdOther->GetText().isEmpty() );

    OUString aOldFldSeps( maFieldSeparators);
    maFieldSeparators = GetSeparators();
    sal_Unicode cOldSep = mcTextSep;
    mcTextSep = lcl_CharFromCombo( *pCbTextSep, gaTextSepList );
    // Any separator changed may result in completely different lines due to
    // embedded line breaks.
    if (cOldSep != mcTextSep || aOldFldSeps != maFieldSeparators)
        UpdateVertical();

    mpTableBox->Execute( CSVCMD_NEWCELLTEXTS );
}

IMPL_LINK( ScImportAsciiDlg, CharSetHdl, ListBox&, rListBox, void )
{
    SvxTextEncodingBox* pCharSetBox = static_cast<SvxTextEncodingBox*>(&rListBox);
    if( (pCharSetBox == pLbCharSet) && (pCharSetBox->GetSelectedEntryCount() == 1) )
    {
        SetPointer( PointerStyle::Wait );
        rtl_TextEncoding eOldCharSet = meCharSet;
        SetSelectedCharSet();
        // switching char-set invalidates 8bit -> String conversions
        if (eOldCharSet != meCharSet)
            UpdateVertical();

        mpTableBox->Execute( CSVCMD_NEWCELLTEXTS );
        SetPointer( PointerStyle::Arrow );
    }
}

IMPL_LINK( ScImportAsciiDlg, FirstRowHdl, Edit&, rEdit, void )
{
    NumericField& rNumField = static_cast<NumericField&>(rEdit);
    mpTableBox->Execute( CSVCMD_SETFIRSTIMPORTLINE, sal::static_int_cast<sal_Int32>( rNumField.GetValue() - 1 ) );
}

IMPL_LINK( ScImportAsciiDlg, LbColTypeHdl, ListBox&, rListBox, void )
{
    if( &rListBox == pLbType )
        mpTableBox->Execute( CSVCMD_SETCOLUMNTYPE, rListBox.GetSelectedEntryPos() );
}

IMPL_LINK_NOARG(ScImportAsciiDlg, UpdateTextHdl, ScCsvTableBox&, void)
{
    // Checking the separator can only be done once for the very first time
    // when the dialog wasn't already presented to the user.
    // As a side effect this has the benefit that the check is only done on the
    // first set of visible lines.
    sal_Unicode cDetectSep = (mbDetectSpaceSep && !pRbFixed->IsChecked() && !pCkbSpace->IsChecked() ? 0 : 0xffff);

    sal_Int32 nBaseLine = mpTableBox->GetFirstVisLine();
    sal_Int32 nRead = mpTableBox->GetVisLineCount();
    // If mnRowPosCount==0, this is an initializing call, read ahead for row
    // count and resulting scroll bar size and position to be able to scroll at
    // all. When adding lines, read only the amount of next lines to be
    // displayed.
    if (!mnRowPosCount || nRead > CSV_PREVIEW_LINES)
        nRead = CSV_PREVIEW_LINES;

    sal_Int32 i;
    for (i = 0; i < nRead; i++)
    {
        if (!GetLine( nBaseLine + i, maPreviewLine[i], cDetectSep))
            break;
    }
    for (; i < CSV_PREVIEW_LINES; i++)
        maPreviewLine[i].clear();

    if (mbDetectSpaceSep)
    {
        mbDetectSpaceSep = false;
        if (cDetectSep == ' ')
        {
            // Expect space to be appended by now so all subsequent
            // GetLine()/ReadCsvLine() actually used it.
            assert(maFieldSeparators.endsWith(" "));
            // Preselect Space in UI.
            pCkbSpace->Check();
        }
    }

    mpTableBox->Execute( CSVCMD_SETLINECOUNT, mnRowPosCount);
    bool bMergeSep = pCkbAsOnce->IsChecked();
    bool bRemoveSpace = pCkbRemoveSpace->IsChecked();
    mpTableBox->SetUniStrings( maPreviewLine, maFieldSeparators, mcTextSep, bMergeSep, bRemoveSpace );
}

IMPL_LINK( ScImportAsciiDlg, ColTypeHdl, ScCsvTableBox&, rTableBox, void )
{
    sal_Int32 nType = rTableBox.GetSelColumnType();
    sal_Int32 nTypeCount = pLbType->GetEntryCount();
    bool bEmpty = (nType == CSV_TYPE_MULTI);
    bool bEnable = ((0 <= nType) && (nType < nTypeCount)) || bEmpty;

    pFtType->Enable( bEnable );
    pLbType->Enable( bEnable );

    Link<ListBox&,void> aSelHdl = pLbType->GetSelectHdl();
    pLbType->SetSelectHdl( Link<ListBox&,void>() );
    if( bEmpty )
        pLbType->SetNoSelection();
    else if( bEnable )
        pLbType->SelectEntryPos( static_cast< sal_uInt16 >( nType ) );
    pLbType->SetSelectHdl( aSelHdl );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

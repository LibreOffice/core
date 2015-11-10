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

#include "global.hxx"
#include "scresid.hxx"
#include "impex.hxx"
#include "scuiasciiopt.hxx"
#include "asciiopt.hrc"
#include "csvtablebox.hxx"
#include <comphelper/string.hxx>
#include <osl/thread.h>
#include <rtl/tencinfo.h>
#include <unotools/transliterationwrapper.hxx>
#include "editutil.hxx"

#include <optutil.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include "miscuno.hxx"
#include <tools/urlobj.hxx>

//! TODO make dynamic
const SCSIZE ASCIIDLG_MAXROWS                = MAXROWCOUNT;

using namespace com::sun::star::uno;

// Defines - CSV Import Preserve Options
#define FIXED_WIDTH         "FixedWidth"
#define FROM_ROW            "FromRow"
#define CHAR_SET            "CharSet"
#define SEPARATORS          "Separators"
#define TEXT_SEPARATORS     "TextSeparators"
#define MERGE_DELIMITERS    "MergeDelimiters"
#define QUOTED_AS_TEXT      "QuotedFieldAsText"
#define DETECT_SPECIAL_NUM  "DetectSpecialNumbers"
#define LANGUAGE            "Language"
#define SEP_PATH            "Office.Calc/Dialogs/CSVImport"
#define SEP_PATH_CLPBRD     "Office.Calc/Dialogs/ClipboardTextImport"
#define SEP_PATH_TEXT2COL   "Office.Calc/Dialogs/TextToColumnsImport"

static void lcl_FillCombo( ComboBox& rCombo, const OUString& rList, sal_Unicode cSelect )
{
    sal_Int32 i;
    sal_Int32 nCount = comphelper::string::getTokenCount(rList, '\t');
    for ( i=0; i<nCount; i+=2 )
        rCombo.InsertEntry( rList.getToken(i,'\t') );

    if ( cSelect )
    {
        OUString aStr;
        for ( i=0; i<nCount; i+=2 )
            if ( (sal_Unicode)rList.getToken(i+1,'\t').toInt32() == cSelect )
                aStr = rList.getToken(i,'\t');
        if (aStr.isEmpty())
            aStr = OUString(cSelect);         // Ascii

        rCombo.SetText(aStr);
    }
}

static sal_Unicode lcl_CharFromCombo( ComboBox& rCombo, const OUString& rList )
{
    sal_Unicode c = 0;
    OUString aStr = rCombo.GetText();
    if ( !aStr.isEmpty() )
    {
        sal_Int32 nCount = comphelper::string::getTokenCount(rList, '\t');
        for ( sal_Int32 i=0; i<nCount; i+=2 )
        {
            if ( ScGlobal::GetpTransliteration()->isEqual( aStr, rList.getToken(i,'\t') ) )
                c = (sal_Unicode)rList.getToken(i+1,'\t').toInt32();
        }
        if (!c && !aStr.isEmpty())
        {
            sal_Unicode cFirst = aStr[0];
            // #i24235# first try the first character of the string directly
            if( (aStr.getLength() == 1) || (cFirst < '0') || (cFirst > '9') )
                c = cFirst;
            else    // keep old behaviour for compatibility (i.e. "39" -> "'")
                c = (sal_Unicode) aStr.toInt32();       // Ascii
        }
    }
    return c;
}

static void load_Separators( OUString &sFieldSeparators, OUString &sTextSeparators,
                             bool &bMergeDelimiters, bool& bQuotedAsText, bool& bDetectSpecialNum,
                             bool &bFixedWidth, sal_Int32 &nFromRow, sal_Int32 &nCharSet,
                             sal_Int32& nLanguage, ScImportAsciiCall eCall )
{
    Sequence<Any>aValues;
    const Any *pProperties;
    Sequence<OUString> aNames( eCall == SC_TEXTTOCOLUMNS ? 4 : 9 );
    OUString* pNames = aNames.getArray();
    OUString aSepPath;
    switch(eCall)
    {
        case SC_IMPORTFILE:
            aSepPath = SEP_PATH;
            break;
        case SC_PASTETEXT:
            aSepPath = SEP_PATH_CLPBRD;
            break;
        case SC_TEXTTOCOLUMNS:
        default:
            aSepPath = SEP_PATH_TEXT2COL;
            break;
    }
    ScLinkConfigItem aItem( aSepPath );

    pNames[0] = MERGE_DELIMITERS;
    pNames[1] = SEPARATORS;
    pNames[2] = TEXT_SEPARATORS;
    pNames[3] = FIXED_WIDTH;
    if (eCall != SC_TEXTTOCOLUMNS)
    {
        pNames[4] = FROM_ROW;
        pNames[5] = CHAR_SET;
        pNames[6] = QUOTED_AS_TEXT;
        pNames[7] = DETECT_SPECIAL_NUM;
        pNames[8] = LANGUAGE;
    }
    aValues = aItem.GetProperties( aNames );
    pProperties = aValues.getConstArray();

    if( pProperties[0].hasValue() )
        bMergeDelimiters = ScUnoHelpFunctions::GetBoolFromAny( pProperties[0] );

    if( pProperties[1].hasValue() )
        pProperties[1] >>= sFieldSeparators;

    if( pProperties[2].hasValue() )
        pProperties[2] >>= sTextSeparators;

    if( pProperties[3].hasValue() )
        bFixedWidth = ScUnoHelpFunctions::GetBoolFromAny( pProperties[3] );

    if (eCall != SC_TEXTTOCOLUMNS)
    {
        if( pProperties[4].hasValue() )
            pProperties[4] >>= nFromRow;

        if( pProperties[5].hasValue() )
            pProperties[5] >>= nCharSet;

        if ( pProperties[6].hasValue() )
            pProperties[6] >>= bQuotedAsText;

        if ( pProperties[7].hasValue() )
            pProperties[7] >>= bDetectSpecialNum;

        if ( pProperties[8].hasValue() )
            pProperties[8] >>= nLanguage;
    }
}

static void save_Separators(
    const OUString& maSeparators, const OUString& maTxtSep, bool bMergeDelimiters, bool bQuotedAsText,
    bool bDetectSpecialNum, bool bFixedWidth, sal_Int32 nFromRow,
    sal_Int32 nCharSet, sal_Int32 nLanguage, ScImportAsciiCall eCall )
{
    OUString sFieldSeparators = OUString( maSeparators );
    OUString sTextSeparators = OUString( maTxtSep );
    Sequence<Any> aValues;
    Any *pProperties;
    Sequence<OUString> aNames( eCall == SC_TEXTTOCOLUMNS ? 4 : 9 );
    OUString* pNames = aNames.getArray();
    OUString aSepPath;
    switch(eCall)
    {
        case SC_IMPORTFILE:
            aSepPath = SEP_PATH;
            break;
        case SC_PASTETEXT:
            aSepPath = SEP_PATH_CLPBRD;
            break;
        case SC_TEXTTOCOLUMNS:
        default:
            aSepPath = SEP_PATH_TEXT2COL;
            break;
    }
    ScLinkConfigItem aItem( aSepPath );

    pNames[0] = MERGE_DELIMITERS;
    pNames[1] = SEPARATORS;
    pNames[2] = TEXT_SEPARATORS;
    pNames[3] = FIXED_WIDTH;
    if (eCall != SC_TEXTTOCOLUMNS)
    {
        pNames[4] = FROM_ROW;
        pNames[5] = CHAR_SET;
        pNames[6] = QUOTED_AS_TEXT;
        pNames[7] = DETECT_SPECIAL_NUM;
        pNames[8] = LANGUAGE;
    }
    aValues = aItem.GetProperties( aNames );
    pProperties = aValues.getArray();
    ScUnoHelpFunctions::SetBoolInAny( pProperties[0], bMergeDelimiters );
    pProperties[1] <<= sFieldSeparators;
    pProperties[2] <<= sTextSeparators;
    ScUnoHelpFunctions::SetBoolInAny( pProperties[3], bFixedWidth );
    if (eCall != SC_TEXTTOCOLUMNS)
    {
        pProperties[4] <<= nFromRow;
        pProperties[5] <<= nCharSet;
        pProperties[6] <<= bQuotedAsText;
        pProperties[7] <<= bDetectSpecialNum;
        pProperties[8] <<= nLanguage;
    }

    aItem.PutProperties(aNames, aValues);
}

ScImportAsciiDlg::ScImportAsciiDlg( vcl::Window* pParent, const OUString& aDatName,
                                    SvStream* pInStream, ScImportAsciiCall eCall ) :
        ModalDialog (pParent, "TextImportCsvDialog",
            "modules/scalc/ui/textimportcsv.ui"),
        mpDatStream  ( pInStream ),
        mnStreamPos( pInStream ? pInStream->Tell() : 0 ),

        mpRowPosArray( nullptr ),
        mnRowPosCount(0),

        aCharSetUser( ScResId( SCSTR_CHARSET_USER ) ),
        aColumnUser ( ScResId( SCSTR_COLUMN_USER ) ),
        aTextSepList( ScResId( SCSTR_TEXTSEP ) ),
        mcTextSep   ( ScAsciiOptions::cDefaultTextSep ),
        meCall(eCall)
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
    get(pCkbOther, "other");
    get(pEdOther, "inputother");
    get(pCkbAsOnce, "mergedelimiters");
    get(pFtTextSep, "texttextdelimiter");
    get(pCbTextSep, "textdelimiter");
    get(pCkbQuotedAsText, "quotedfieldastext");
    get(pCkbDetectNumber, "detectspecialnumbers");
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
    sal_Int32 nFromRow = 1;
    sal_Int32 nCharSet = -1;
    sal_Int32 nLanguage = 0;
    load_Separators (sFieldSeparators, sTextSeparators, bMergeDelimiters,
                         bQuotedFieldAsText, bDetectSpecialNum, bFixedWidth, nFromRow, nCharSet, nLanguage, meCall);
    // load from saved settings
    maFieldSeparators = OUString(sFieldSeparators);

    if( bMergeDelimiters && !bIsTSV )
        pCkbAsOnce->Check();
    if (bQuotedFieldAsText)
        pCkbQuotedAsText->Check();
    if (bDetectSpecialNum)
        pCkbDetectNumber->Check();
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
    lcl_FillCombo( *pCbTextSep, aTextSepList, mcTextSep );
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
    pCkbSpace->SetClickHdl( aSeparatorClickHdl );
    pCkbOther->SetClickHdl( aSeparatorClickHdl );
    pEdOther->SetModifyHdl( aSeparatorHdl );

    // *** text encoding ListBox ***
    // all encodings allowed, including Unicode, but subsets are excluded
    pLbCharSet->FillFromTextEncodingTable( true );
    // Insert one "SYSTEM" entry for compatibility in AsciiOptions and system
    // independent document linkage.
    pLbCharSet->InsertTextEncoding( RTL_TEXTENCODING_DONTKNOW, aCharSetUser );
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
        SvxLanguageListFlags::ALL | SvxLanguageListFlags::ONLY_KNOWN, false);
    pLbCustomLang->InsertLanguage(LANGUAGE_SYSTEM);
    pLbCustomLang->SelectLanguage(static_cast<LanguageType>(nLanguage));

    // *** column type ListBox ***
    sal_Int32 nCount = comphelper::string::getTokenCount(aColumnUser, ';');
    for (sal_Int32 i=0; i<nCount; i++)
        pLbType->InsertEntry( aColumnUser.getToken( i, ';' ) );

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

    pEdOther->SetAccessibleName(pCkbOther->GetText());
    pEdOther->SetAccessibleRelationLabeledBy(pCkbOther);

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
}

ScImportAsciiDlg::~ScImportAsciiDlg()
{
    disposeOnce();
}

void ScImportAsciiDlg::dispose()
{
    delete[] mpRowPosArray;
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
    pCkbOther.clear();
    pEdOther.clear();
    pCkbAsOnce.clear();
    pFtTextSep.clear();
    pCbTextSep.clear();
    pCkbQuotedAsText.clear();
    pCkbDetectNumber.clear();
    pFtType.clear();
    pLbType.clear();
    mpTableBox.clear();
    ModalDialog::dispose();
}

bool ScImportAsciiDlg::GetLine( sal_uLong nLine, OUString &rText )
{
    if (nLine >= ASCIIDLG_MAXROWS || !mpDatStream)
        return false;

    bool bRet = true;
    bool bFixed = pRbFixed->IsChecked();

    if (!mpRowPosArray)
        mpRowPosArray = new sal_uLong[ASCIIDLG_MAXROWS + 2];

    if (!mnRowPosCount) // complete re-fresh
    {
        memset( mpRowPosArray, 0, sizeof(mpRowPosArray[0]) * (ASCIIDLG_MAXROWS+2));

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
            if (!Seek( mpRowPosArray[mnRowPosCount]) ||
                    mpDatStream->GetError() != ERRCODE_NONE ||
                    mpDatStream->IsEof())
            {
                bRet = false;
                break;
            }
            rText = ReadCsvLine(*mpDatStream, !bFixed, maFieldSeparators,
                    mcTextSep);
            mnStreamPos = mpDatStream->Tell();
            mpRowPosArray[++mnRowPosCount] = mnStreamPos;
        } while (nLine >= mnRowPosCount &&
                mpDatStream->GetError() == ERRCODE_NONE &&
                !mpDatStream->IsEof());
        if (mpDatStream->IsEof() &&
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
        rText = ReadCsvLine(*mpDatStream, !bFixed, maFieldSeparators, mcTextSep);
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
    rOpt.SetLanguage(pLbCustomLang->GetSelectLanguage());
    rOpt.SetFixedLen( pRbFixed->IsChecked() );
    rOpt.SetStartRow( (long)pNfRow->GetValue() );
    mpTableBox->FillColumnData( rOpt );
    if( pRbSeparated->IsChecked() )
    {
        rOpt.SetFieldSeps( GetSeparators() );
        rOpt.SetMergeSeps( pCkbAsOnce->IsChecked() );
        rOpt.SetTextSep( lcl_CharFromCombo( *pCbTextSep, aTextSepList ) );
    }

    rOpt.SetQuotedAsText(pCkbQuotedAsText->IsChecked());
    rOpt.SetDetectSpecialNumber(pCkbDetectNumber->IsChecked());
}

void ScImportAsciiDlg::SaveParameters()
{
    save_Separators( maFieldSeparators, pCbTextSep->GetText(), pCkbAsOnce->IsChecked(),
                     pCkbQuotedAsText->IsChecked(), pCkbDetectNumber->IsChecked(),
                     pRbFixed->IsChecked(),
                     static_cast<sal_Int32>(pNfRow->GetValue()),
                     static_cast<sal_Int32>(pLbCharSet->GetSelectEntryPos()),
                     static_cast<sal_Int32>(pLbCustomLang->GetSelectLanguage()), meCall );
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

IMPL_LINK_TYPED( ScImportAsciiDlg, RbSepFixHdl, Button*, pButton, void )
{
    OSL_ENSURE( pButton, "ScImportAsciiDlg::RbSepFixHdl - missing sender" );

    if( (pButton == pRbFixed) || (pButton == pRbSeparated) )
    {
        SetPointer( Pointer( PointerStyle::Wait ) );
        if( pRbFixed->IsChecked() )
            mpTableBox->SetFixedWidthMode();
        else
            mpTableBox->SetSeparatorsMode();
        SetPointer( Pointer( PointerStyle::Arrow ) );

        SetupSeparatorCtrls();
    }
}

IMPL_LINK_TYPED( ScImportAsciiDlg, SeparatorClickHdl, Button*, pCtrl, void )
{
    SeparatorHdl(pCtrl);
}
IMPL_LINK_TYPED( ScImportAsciiDlg, SeparatorComboBoxHdl, ComboBox&, rCtrl, void )
{
    SeparatorHdl(&rCtrl);
}
IMPL_LINK_TYPED( ScImportAsciiDlg, SeparatorEditHdl, Edit&, rEdit, void )
{
    SeparatorHdl(&rEdit);
}
void ScImportAsciiDlg::SeparatorHdl( Control* pCtrl )
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
    mcTextSep = lcl_CharFromCombo( *pCbTextSep, aTextSepList );
    // Any separator changed may result in completely different lines due to
    // embedded line breaks.
    if (cOldSep != mcTextSep || aOldFldSeps != maFieldSeparators)
        UpdateVertical();

    mpTableBox->Execute( CSVCMD_NEWCELLTEXTS );
}

IMPL_LINK_TYPED( ScImportAsciiDlg, CharSetHdl, ListBox&, rListBox, void )
{
    SvxTextEncodingBox* pCharSetBox = static_cast<SvxTextEncodingBox*>(&rListBox);
    if( (pCharSetBox == pLbCharSet) && (pCharSetBox->GetSelectEntryCount() == 1) )
    {
        SetPointer( Pointer( PointerStyle::Wait ) );
        rtl_TextEncoding eOldCharSet = meCharSet;
        SetSelectedCharSet();
        // switching char-set invalidates 8bit -> String conversions
        if (eOldCharSet != meCharSet)
            UpdateVertical();

        mpTableBox->Execute( CSVCMD_NEWCELLTEXTS );
        SetPointer( Pointer( PointerStyle::Arrow ) );
    }
}

IMPL_LINK_TYPED( ScImportAsciiDlg, FirstRowHdl, Edit&, rEdit, void )
{
    NumericField& rNumField = static_cast<NumericField&>(rEdit);
    mpTableBox->Execute( CSVCMD_SETFIRSTIMPORTLINE, sal::static_int_cast<sal_Int32>( rNumField.GetValue() - 1 ) );
}

IMPL_LINK_TYPED( ScImportAsciiDlg, LbColTypeHdl, ListBox&, rListBox, void )
{
    if( &rListBox == pLbType )
        mpTableBox->Execute( CSVCMD_SETCOLUMNTYPE, rListBox.GetSelectEntryPos() );
}

IMPL_LINK_NOARG_TYPED(ScImportAsciiDlg, UpdateTextHdl, ScCsvTableBox&, void)
{
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
        if (!GetLine( nBaseLine + i, maPreviewLine[i]))
            break;
    }
    for (; i < CSV_PREVIEW_LINES; i++)
        maPreviewLine[i].clear();

    mpTableBox->Execute( CSVCMD_SETLINECOUNT, mnRowPosCount);
    bool bMergeSep = pCkbAsOnce->IsChecked();
    mpTableBox->SetUniStrings( maPreviewLine, maFieldSeparators, mcTextSep, bMergeSep);
}

IMPL_LINK_TYPED( ScImportAsciiDlg, ColTypeHdl, ScCsvTableBox&, rTableBox, void )
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

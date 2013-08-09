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

#include "global.hxx"
#include "scresid.hxx"
#include "impex.hxx"
#include "scuiasciiopt.hxx"
#include "asciiopt.hrc"
#include <comphelper/string.hxx>
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

// ============================================================================

static void lcl_FillCombo( ComboBox& rCombo, const String& rList, sal_Unicode cSelect )
{
    xub_StrLen i;
    xub_StrLen nCount = comphelper::string::getTokenCount(rList, '\t');
    for ( i=0; i<nCount; i+=2 )
        rCombo.InsertEntry( rList.GetToken(i,'\t') );

    if ( cSelect )
    {
        String aStr;
        for ( i=0; i<nCount; i+=2 )
            if ( (sal_Unicode)rList.GetToken(i+1,'\t').ToInt32() == cSelect )
                aStr = rList.GetToken(i,'\t');
        if (!aStr.Len())
            aStr = cSelect;         // Ascii

        rCombo.SetText(aStr);
    }
}

static sal_Unicode lcl_CharFromCombo( ComboBox& rCombo, const String& rList )
{
    sal_Unicode c = 0;
    String aStr = rCombo.GetText();
    if ( aStr.Len() )
    {
        xub_StrLen nCount = comphelper::string::getTokenCount(rList, '\t');
        for ( xub_StrLen i=0; i<nCount; i+=2 )
        {
            if ( ScGlobal::GetpTransliteration()->isEqual( aStr, rList.GetToken(i,'\t') ) )
                c = (sal_Unicode)rList.GetToken(i+1,'\t').ToInt32();
        }
        if (!c && aStr.Len())
        {
            sal_Unicode cFirst = aStr.GetChar( 0 );
            // #i24235# first try the first character of the string directly
            if( (aStr.Len() == 1) || (cFirst < '0') || (cFirst > '9') )
                c = cFirst;
            else    // keep old behaviour for compatibility (i.e. "39" -> "'")
                c = (sal_Unicode) aStr.ToInt32();       // Ascii
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

    pNames[0] = OUString( MERGE_DELIMITERS );
    pNames[1] = OUString( SEPARATORS );
    pNames[2] = OUString( TEXT_SEPARATORS );
    pNames[3] = OUString( FIXED_WIDTH );
    if (eCall != SC_TEXTTOCOLUMNS)
    {
        pNames[4] = OUString( FROM_ROW );
        pNames[5] = OUString( CHAR_SET );
        pNames[6] = OUString( QUOTED_AS_TEXT );
        pNames[7] = OUString( DETECT_SPECIAL_NUM );
        pNames[8] = OUString( LANGUAGE );
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
    String maSeparators, String maTxtSep, bool bMergeDelimiters, bool bQuotedAsText,
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

    pNames[0] = OUString( MERGE_DELIMITERS );
    pNames[1] = OUString( SEPARATORS );
    pNames[2] = OUString( TEXT_SEPARATORS );
    pNames[3] = OUString( FIXED_WIDTH );
    if (eCall != SC_TEXTTOCOLUMNS)
    {
        pNames[4] = OUString( FROM_ROW );
        pNames[5] = OUString( CHAR_SET );
        pNames[6] = OUString( QUOTED_AS_TEXT );
        pNames[7] = OUString( DETECT_SPECIAL_NUM );
        pNames[8] = OUString( LANGUAGE );
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
        pProperties[6] <<= static_cast<sal_Bool>(bQuotedAsText);
        pProperties[7] <<= static_cast<sal_Bool>(bDetectSpecialNum);
        pProperties[8] <<= nLanguage;
    }

    aItem.PutProperties(aNames, aValues);
}

// ----------------------------------------------------------------------------

ScImportAsciiDlg::ScImportAsciiDlg( Window* pParent,String aDatName,
                                    SvStream* pInStream, ScImportAsciiCall eCall ) :
        ModalDialog (pParent, "TextImportCsvDialog",
            "modules/scalc/ui/textimportcsv.ui"),
        mpDatStream  ( pInStream ),
        mnStreamPos( pInStream ? pInStream->Tell() : 0 ),

        mpRowPosArray( NULL ),
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

    String aName = GetText();
    switch (meCall)
    {
        case SC_TEXTTOCOLUMNS:
            SetText( get<FixedText>("textalttitle")->GetText() );
            break;
        case SC_IMPORTFILE:
            aName.AppendAscii(RTL_CONSTASCII_STRINGPARAM(" - ["));
            aName += aDatName;
            aName += ']';
        default:
            SetText( aName );
    }

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
    maFieldSeparators = String(sFieldSeparators);

    if( bMergeDelimiters )
        pCkbAsOnce->Check();
    if (bQuotedFieldAsText)
        pCkbQuotedAsText->Check();
    if (bDetectSpecialNum)
        pCkbDetectNumber->Check();
    if( bFixedWidth )
        pRbFixed->Check();
    if( nFromRow != 1 )
        pNfRow->SetValue( nFromRow );

    // Set Separators in the dialog from maFieldSeparators (empty are not set)
    SetSeparators();

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
                    *mpDatStream >> n;
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

    Link aSeparatorHdl =LINK( this, ScImportAsciiDlg, SeparatorHdl );
    pCbTextSep->SetSelectHdl( aSeparatorHdl );
    pCbTextSep->SetModifyHdl( aSeparatorHdl );
    pCkbTab->SetClickHdl( aSeparatorHdl );
    pCkbSemicolon->SetClickHdl( aSeparatorHdl );
    pCkbComma->SetClickHdl( aSeparatorHdl );
    pCkbAsOnce->SetClickHdl( aSeparatorHdl );
    pCkbQuotedAsText->SetClickHdl( aSeparatorHdl );
    pCkbDetectNumber->SetClickHdl( aSeparatorHdl );
    pCkbSpace->SetClickHdl( aSeparatorHdl );
    pCkbOther->SetClickHdl( aSeparatorHdl );
    pEdOther->SetModifyHdl( aSeparatorHdl );

    // *** text encoding ListBox ***
    // all encodings allowed, including Unicode, but subsets are excluded
    pLbCharSet->FillFromTextEncodingTable( sal_True );
    // Insert one "SYSTEM" entry for compatibility in AsciiOptions and system
    // independent document linkage.
    pLbCharSet->InsertTextEncoding( RTL_TEXTENCODING_DONTKNOW, aCharSetUser );
    pLbCharSet->SelectTextEncoding( ePreselectUnicode == RTL_TEXTENCODING_DONTKNOW ?
            osl_getThreadTextEncoding() : ePreselectUnicode );

    if( nCharSet >= 0 && ePreselectUnicode == RTL_TEXTENCODING_DONTKNOW )
        pLbCharSet->SelectEntryPos( static_cast<sal_uInt16>(nCharSet) );

    SetSelectedCharSet();
    pLbCharSet->SetSelectHdl( LINK( this, ScImportAsciiDlg, CharSetHdl ) );

    pLbCustomLang->SetLanguageList(
        LANG_LIST_ALL | LANG_LIST_ONLY_KNOWN, false, false);
    pLbCustomLang->InsertLanguage(LANGUAGE_SYSTEM);
    pLbCustomLang->SelectLanguage(static_cast<LanguageType>(nLanguage), true);

    // *** column type ListBox ***
    xub_StrLen nCount = comphelper::string::getTokenCount(aColumnUser, ';');
    for (xub_StrLen i=0; i<nCount; i++)
        pLbType->InsertEntry( aColumnUser.GetToken( i ) );

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
    delete[] mpRowPosArray;
}


// ----------------------------------------------------------------------------

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
    int len = maFieldSeparators.Len();
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

String ScImportAsciiDlg::GetSeparators() const
{
    String aSepChars;
    if( pCkbTab->IsChecked() )
        aSepChars += '\t';
    if( pCkbSemicolon->IsChecked() )
        aSepChars += ';';
    if( pCkbComma->IsChecked() )
        aSepChars += ',';
    if( pCkbSpace->IsChecked() )
        aSepChars += ' ';
    if( pCkbOther->IsChecked() )
        aSepChars += pEdOther->GetText();
    return aSepChars;
}

void ScImportAsciiDlg::SetupSeparatorCtrls()
{
    sal_Bool bEnable = pRbSeparated->IsChecked();
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


// ----------------------------------------------------------------------------

IMPL_LINK( ScImportAsciiDlg, RbSepFixHdl, RadioButton*, pButton )
{
    OSL_ENSURE( pButton, "ScImportAsciiDlg::RbSepFixHdl - missing sender" );

    if( (pButton == pRbFixed) || (pButton == pRbSeparated) )
    {
        SetPointer( Pointer( POINTER_WAIT ) );
        if( pRbFixed->IsChecked() )
            mpTableBox->SetFixedWidthMode();
        else
            mpTableBox->SetSeparatorsMode();
        SetPointer( Pointer( POINTER_ARROW ) );

        SetupSeparatorCtrls();
    }
    return 0;
}

IMPL_LINK( ScImportAsciiDlg, SeparatorHdl, Control*, pCtrl )
{
    OSL_ENSURE( pCtrl, "ScImportAsciiDlg::SeparatorHdl - missing sender" );
    OSL_ENSURE( !pRbFixed->IsChecked(), "ScImportAsciiDlg::SeparatorHdl - not allowed in fixed width" );

    /*  #i41550# First update state of the controls. The GetSeparators()
        function needs final state of the check boxes. */
    if( (pCtrl == pCkbOther) && pCkbOther->IsChecked() )
        pEdOther->GrabFocus();
    else if( pCtrl == pEdOther )
        pCkbOther->Check( !pEdOther->GetText().isEmpty() );

    String aOldFldSeps( maFieldSeparators);
    maFieldSeparators = GetSeparators();
    sal_Unicode cOldSep = mcTextSep;
    mcTextSep = lcl_CharFromCombo( *pCbTextSep, aTextSepList );
    // Any separator changed may result in completely different lines due to
    // embedded line breaks.
    if (cOldSep != mcTextSep || aOldFldSeps != maFieldSeparators)
        UpdateVertical();

    mpTableBox->Execute( CSVCMD_NEWCELLTEXTS );
    return 0;
}

IMPL_LINK( ScImportAsciiDlg, CharSetHdl, SvxTextEncodingBox*, pCharSetBox )
{
    OSL_ENSURE( pCharSetBox, "ScImportAsciiDlg::CharSetHdl - missing sender" );

    if( (pCharSetBox == pLbCharSet) && (pCharSetBox->GetSelectEntryCount() == 1) )
    {
        SetPointer( Pointer( POINTER_WAIT ) );
        CharSet eOldCharSet = meCharSet;
        SetSelectedCharSet();
        // switching char-set invalidates 8bit -> String conversions
        if (eOldCharSet != meCharSet)
            UpdateVertical();

        mpTableBox->Execute( CSVCMD_NEWCELLTEXTS );
        SetPointer( Pointer( POINTER_ARROW ) );
    }
    return 0;
}

IMPL_LINK( ScImportAsciiDlg, FirstRowHdl, NumericField*, pNumField )
{
    OSL_ENSURE( pNumField, "ScImportAsciiDlg::FirstRowHdl - missing sender" );
    mpTableBox->Execute( CSVCMD_SETFIRSTIMPORTLINE, sal::static_int_cast<sal_Int32>( pNumField->GetValue() - 1 ) );
    return 0;
}

IMPL_LINK( ScImportAsciiDlg, LbColTypeHdl, ListBox*, pListBox )
{
    OSL_ENSURE( pListBox, "ScImportAsciiDlg::LbColTypeHdl - missing sender" );
    if( pListBox == pLbType )
        mpTableBox->Execute( CSVCMD_SETCOLUMNTYPE, pListBox->GetSelectEntryPos() );
    return 0;
}

IMPL_LINK_NOARG(ScImportAsciiDlg, UpdateTextHdl)
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
        maPreviewLine[i] = OUString();

    mpTableBox->Execute( CSVCMD_SETLINECOUNT, mnRowPosCount);
    bool bMergeSep = (pCkbAsOnce->IsChecked() == sal_True);
    mpTableBox->SetUniStrings( maPreviewLine, maFieldSeparators, mcTextSep, bMergeSep);

    return 0;
}

IMPL_LINK( ScImportAsciiDlg, ColTypeHdl, ScCsvTableBox*, pTableBox )
{
    OSL_ENSURE( pTableBox, "ScImportAsciiDlg::ColTypeHdl - missing sender" );

    sal_Int32 nType = pTableBox->GetSelColumnType();
    sal_Int32 nTypeCount = pLbType->GetEntryCount();
    bool bEmpty = (nType == CSV_TYPE_MULTI);
    bool bEnable = ((0 <= nType) && (nType < nTypeCount)) || bEmpty;

    pFtType->Enable( bEnable );
    pLbType->Enable( bEnable );

    Link aSelHdl = pLbType->GetSelectHdl();
    pLbType->SetSelectHdl( Link() );
    if( bEmpty )
        pLbType->SetNoSelection();
    else if( bEnable )
        pLbType->SelectEntryPos( static_cast< sal_uInt16 >( nType ) );
    pLbType->SetSelectHdl( aSelHdl );

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

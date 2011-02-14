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

#undef SC_DLLIMPLEMENTATION

#include "global.hxx"
#include "scresid.hxx"
#include "impex.hxx"
#include "scuiasciiopt.hxx"
#include "asciiopt.hrc"
#include <tools/debug.hxx>
#include <rtl/tencinfo.h>
#include <unotools/transliterationwrapper.hxx>
// ause
#include "editutil.hxx"

#include <optutil.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include "miscuno.hxx"

//! TODO make dynamic
const SCSIZE ASCIIDLG_MAXROWS                = MAXROWCOUNT;


using namespace rtl;
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

// ============================================================================

void lcl_FillCombo( ComboBox& rCombo, const String& rList, sal_Unicode cSelect )
{
    xub_StrLen i;
    xub_StrLen nCount = rList.GetTokenCount('\t');
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

sal_Unicode lcl_CharFromCombo( ComboBox& rCombo, const String& rList )
{
    sal_Unicode c = 0;
    String aStr = rCombo.GetText();
    if ( aStr.Len() )
    {
        xub_StrLen nCount = rList.GetTokenCount('\t');
        for ( xub_StrLen i=0; i<nCount; i+=2 )
        {
            if ( ScGlobal::GetpTransliteration()->isEqual( aStr, rList.GetToken(i,'\t') ) )//CHINA001 if ( ScGlobal::GetpTransliteration()->isEqual( aStr, rList.GetToken(i,'\t') ) )
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
                             sal_Int32& nLanguage )
{
    Sequence<Any>aValues;
    const Any *pProperties;
    Sequence<OUString> aNames(9);
    OUString* pNames = aNames.getArray();
    ScLinkConfigItem aItem( OUString::createFromAscii( SEP_PATH ) );

    pNames[0] = OUString::createFromAscii( MERGE_DELIMITERS );
    pNames[1] = OUString::createFromAscii( SEPARATORS );
    pNames[2] = OUString::createFromAscii( TEXT_SEPARATORS );
    pNames[3] = OUString::createFromAscii( FIXED_WIDTH );
    pNames[4] = OUString::createFromAscii( FROM_ROW );
    pNames[5] = OUString::createFromAscii( CHAR_SET );
    pNames[6] = OUString::createFromAscii( QUOTED_AS_TEXT );
    pNames[7] = OUString::createFromAscii( DETECT_SPECIAL_NUM );
    pNames[8] = OUString::createFromAscii( LANGUAGE );
    aValues = aItem.GetProperties( aNames );
    pProperties = aValues.getConstArray();
    if( pProperties[1].hasValue() )
        pProperties[1] >>= sFieldSeparators;

    if( pProperties[2].hasValue() )
        pProperties[2] >>= sTextSeparators;

    if( pProperties[0].hasValue() )
        bMergeDelimiters = ScUnoHelpFunctions::GetBoolFromAny( pProperties[0] );

    if( pProperties[3].hasValue() )
        bFixedWidth = ScUnoHelpFunctions::GetBoolFromAny( pProperties[3] );

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

static void save_Separators(
    String maSeparators, String maTxtSep, bool bMergeDelimiters, bool bQuotedAsText,
    bool bDetectSpecialNum, bool bFixedWidth, sal_Int32 nFromRow, sal_Int32 nCharSet, sal_Int32 nLanguage )
{
    OUString sFieldSeparators = OUString( maSeparators );
    OUString sTextSeparators = OUString( maTxtSep );
    Sequence<Any> aValues;
    Any *pProperties;
    Sequence<OUString> aNames(9);
    OUString* pNames = aNames.getArray();
    ScLinkConfigItem aItem( OUString::createFromAscii( SEP_PATH ) );

    pNames[0] = OUString::createFromAscii( MERGE_DELIMITERS );
    pNames[1] = OUString::createFromAscii( SEPARATORS );
    pNames[2] = OUString::createFromAscii( TEXT_SEPARATORS );
    pNames[3] = OUString::createFromAscii( FIXED_WIDTH );
    pNames[4] = OUString::createFromAscii( FROM_ROW );
    pNames[5] = OUString::createFromAscii( CHAR_SET );
    pNames[6] = OUString::createFromAscii( QUOTED_AS_TEXT );
    pNames[7] = OUString::createFromAscii( DETECT_SPECIAL_NUM );
    pNames[8] = OUString::createFromAscii( LANGUAGE );
    aValues = aItem.GetProperties( aNames );
    pProperties = aValues.getArray();
    pProperties[1] <<= sFieldSeparators;
    pProperties[2] <<= sTextSeparators;
    ScUnoHelpFunctions::SetBoolInAny( pProperties[0], bMergeDelimiters );
    ScUnoHelpFunctions::SetBoolInAny( pProperties[3], bFixedWidth );
    pProperties[4] <<= nFromRow;
    pProperties[5] <<= nCharSet;
    pProperties[6] <<= static_cast<sal_Bool>(bQuotedAsText);
    pProperties[7] <<= static_cast<sal_Bool>(bDetectSpecialNum);
    pProperties[8] <<= nLanguage;

    aItem.PutProperties(aNames, aValues);
}

// ----------------------------------------------------------------------------

ScImportAsciiDlg::ScImportAsciiDlg( Window* pParent,String aDatName,
                                    SvStream* pInStream, sal_Unicode /*cSep*/ ) :
        ModalDialog ( pParent, ScResId( RID_SCDLG_ASCII ) ),
        mpDatStream  ( pInStream ),
        mnStreamPos( pInStream ? pInStream->Tell() : 0 ),

        mpRowPosArray( NULL ),
        mnRowPosCount(0),

        aFlFieldOpt ( this, ScResId( FL_FIELDOPT ) ),
        aFtCharSet  ( this, ScResId( FT_CHARSET ) ),
        aLbCharSet  ( this, ScResId( LB_CHARSET ) ),
        aFtCustomLang( this, ScResId( FT_CUSTOMLANG ) ),
        aLbCustomLang( this, ScResId( LB_CUSTOMLANG ) ),

        aFtRow      ( this, ScResId( FT_AT_ROW  ) ),
        aNfRow      ( this, ScResId( NF_AT_ROW  ) ),

        aFlSepOpt   ( this, ScResId( FL_SEPOPT ) ),
        aRbFixed    ( this, ScResId( RB_FIXED ) ),
        aRbSeparated( this, ScResId( RB_SEPARATED ) ),

        aCkbTab     ( this, ScResId( CKB_TAB ) ),
        aCkbSemicolon(this, ScResId( CKB_SEMICOLON ) ),
        aCkbComma   ( this, ScResId( CKB_COMMA  ) ),
        aCkbSpace   ( this, ScResId( CKB_SPACE   ) ),
        aCkbOther   ( this, ScResId( CKB_OTHER ) ),
        aEdOther    ( this, ScResId( ED_OTHER ) ),
        aCkbAsOnce  ( this, ScResId( CB_ASONCE) ),
        aFlOtherOpt ( this, ScResId( FL_OTHER_OPTIONS ) ),

        aFtTextSep  ( this, ScResId( FT_TEXTSEP ) ),
        aCbTextSep  ( this, ScResId( CB_TEXTSEP ) ),

        aCkbQuotedAsText( this, ScResId(CB_QUOTED_AS_TEXT) ),
        aCkbDetectNumber( this, ScResId(CB_DETECT_SPECIAL_NUMBER) ),

        aFlWidth    ( this, ScResId( FL_WIDTH ) ),
        aFtType     ( this, ScResId( FT_TYPE ) ),
        aLbType     ( this, ScResId( LB_TYPE1 ) ),

        maTableBox  ( this, ScResId( CTR_TABLEBOX ) ),

        aBtnOk      ( this, ScResId( BTN_OK ) ),
        aBtnCancel  ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp    ( this, ScResId( BTN_HELP ) ),

        aCharSetUser( ScResId( SCSTR_CHARSET_USER ) ),
        aColumnUser ( ScResId( SCSTR_COLUMN_USER ) ),
        aFldSepList ( ScResId( SCSTR_FIELDSEP ) ),
        aTextSepList( ScResId( SCSTR_TEXTSEP ) ),
        mcTextSep   ( ScAsciiOptions::cDefaultTextSep ),
        maStrTextToColumns( ScResId( STR_TEXTTOCOLUMNS ) ),
        mbFileImport(true)
{
    FreeResource();
    mbFileImport = aDatName.Len() > 0;

    String aName = GetText();
    // aDatName is empty if invoked during paste from clipboard.
    if (mbFileImport)
    {
        aName.AppendAscii(RTL_CONSTASCII_STRINGPARAM(" - ["));
        aName += aDatName;
        aName += ']';
    }
    SetText( aName );


    OUString sFieldSeparators;
    OUString sTextSeparators;
    bool bMergeDelimiters = false;
    bool bFixedWidth = false;
    bool bQuotedFieldAsText = true;
    bool bDetectSpecialNum = false;
    sal_Int32 nFromRow = 1;
    sal_Int32 nCharSet = -1;
    sal_Int32 nLanguage = 0;
    if (mbFileImport)
        // load separators only when importing csv files.
        load_Separators (sFieldSeparators, sTextSeparators, bMergeDelimiters,
                         bQuotedFieldAsText, bDetectSpecialNum, bFixedWidth, nFromRow, nCharSet, nLanguage);
    maFieldSeparators = String(sFieldSeparators);

    if( bMergeDelimiters )
        aCkbAsOnce.Check();
    if (bQuotedFieldAsText)
        aCkbQuotedAsText.Check();
    if (bDetectSpecialNum)
        aCkbDetectNumber.Check();
    if( bFixedWidth )
        aRbFixed.Check();
    if( nFromRow != 1 )
        aNfRow.SetValue( nFromRow );

    ByteString bString(maFieldSeparators,RTL_TEXTENCODING_MS_1252);
    const sal_Char *aSep = bString.GetBuffer();
    int len = maFieldSeparators.Len();
    for (int i = 0; i < len; ++i)
    {
        switch( aSep[i] )
        {
            case '\t':  aCkbTab.Check();        break;
            case ';':   aCkbSemicolon.Check();  break;
            case ',':   aCkbComma.Check();      break;
            case ' ':   aCkbSpace.Check();      break;
            default:
                aCkbOther.Check();
                aEdOther.SetText( aEdOther.GetText() + OUString( aSep[i] ) );
        }
    }

    // Get Separators from the dialog
    maFieldSeparators = GetSeparators();

    // Clipboard is always Unicode, else detect.
    bool bPreselectUnicode = !mbFileImport;
    // Sniff for Unicode / not
    if( !bPreselectUnicode && mpDatStream )
    {
        Seek( 0 );
        mpDatStream->StartReadingUnicodeText();
        sal_uLong nUniPos = mpDatStream->Tell();
        if ( nUniPos > 0 )
            bPreselectUnicode = sal_True;   // read 0xfeff/0xfffe
        else
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
                        bPreselectUnicode = sal_True;
                }
            }
            mpDatStream->Seek(0);
        }
        mnStreamPos = mpDatStream->Tell();
    }

    aNfRow.SetModifyHdl( LINK( this, ScImportAsciiDlg, FirstRowHdl ) );

    // *** Separator characters ***
    lcl_FillCombo( aCbTextSep, aTextSepList, mcTextSep );
    aCbTextSep.SetText( sTextSeparators );

    Link aSeparatorHdl =LINK( this, ScImportAsciiDlg, SeparatorHdl );
    aCbTextSep.SetSelectHdl( aSeparatorHdl );
    aCbTextSep.SetModifyHdl( aSeparatorHdl );
    aCkbTab.SetClickHdl( aSeparatorHdl );
    aCkbSemicolon.SetClickHdl( aSeparatorHdl );
    aCkbComma.SetClickHdl( aSeparatorHdl );
    aCkbAsOnce.SetClickHdl( aSeparatorHdl );
    aCkbQuotedAsText.SetClickHdl( aSeparatorHdl );
    aCkbDetectNumber.SetClickHdl( aSeparatorHdl );
    aCkbSpace.SetClickHdl( aSeparatorHdl );
    aCkbOther.SetClickHdl( aSeparatorHdl );
    aEdOther.SetModifyHdl( aSeparatorHdl );

    // *** text encoding ListBox ***
    // all encodings allowed, including Unicode, but subsets are excluded
    aLbCharSet.FillFromTextEncodingTable( sal_True );
    // Insert one "SYSTEM" entry for compatibility in AsciiOptions and system
    // independent document linkage.
    aLbCharSet.InsertTextEncoding( RTL_TEXTENCODING_DONTKNOW, aCharSetUser );
    aLbCharSet.SelectTextEncoding( bPreselectUnicode ?
        RTL_TEXTENCODING_UNICODE : gsl_getSystemTextEncoding() );

    if( nCharSet >= 0 )
        aLbCharSet.SelectEntryPos( static_cast<sal_uInt16>(nCharSet) );

    SetSelectedCharSet();
    aLbCharSet.SetSelectHdl( LINK( this, ScImportAsciiDlg, CharSetHdl ) );

    aLbCustomLang.SetLanguageList(
        LANG_LIST_ALL | LANG_LIST_ONLY_KNOWN, false, false);
    aLbCustomLang.InsertLanguage(LANGUAGE_SYSTEM);
    aLbCustomLang.SelectLanguage(static_cast<LanguageType>(nLanguage), true);

    // *** column type ListBox ***
    xub_StrLen nCount = aColumnUser.GetTokenCount();
    for (xub_StrLen i=0; i<nCount; i++)
        aLbType.InsertEntry( aColumnUser.GetToken( i ) );

    aLbType.SetSelectHdl( LINK( this, ScImportAsciiDlg, LbColTypeHdl ) );
    aFtType.Disable();
    aLbType.Disable();

    // *** table box preview ***
    maTableBox.SetUpdateTextHdl( LINK( this, ScImportAsciiDlg, UpdateTextHdl ) );
    maTableBox.InitTypes( aLbType );
    maTableBox.SetColTypeHdl( LINK( this, ScImportAsciiDlg, ColTypeHdl ) );

    aRbSeparated.SetClickHdl( LINK( this, ScImportAsciiDlg, RbSepFixHdl ) );
    aRbFixed.SetClickHdl( LINK( this, ScImportAsciiDlg, RbSepFixHdl ) );

    SetupSeparatorCtrls();
    RbSepFixHdl( &aRbFixed );

    UpdateVertical();

    maTableBox.Execute( CSVCMD_NEWCELLTEXTS );
}


ScImportAsciiDlg::~ScImportAsciiDlg()
{
    delete[] mpRowPosArray;
}


// ----------------------------------------------------------------------------

bool ScImportAsciiDlg::GetLine( sal_uLong nLine, String &rText )
{
    if (nLine >= ASCIIDLG_MAXROWS || !mpDatStream)
        return false;

    bool bRet = true;
    bool bFixed = aRbFixed.IsChecked();

    if (!mpRowPosArray)
        mpRowPosArray = new sal_uLong[ASCIIDLG_MAXROWS + 2];

    if (!mnRowPosCount) // complete re-fresh
    {
        memset( mpRowPosArray, 0, sizeof(mpRowPosArray[0]) * (ASCIIDLG_MAXROWS+2));

        Seek(0);
        if ( mpDatStream->GetStreamCharSet() == RTL_TEXTENCODING_UNICODE )
            mpDatStream->StartReadingUnicodeText();

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
            mpDatStream->ReadCsvLine( rText, !bFixed, maFieldSeparators,
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
        mpDatStream->ReadCsvLine( rText, !bFixed, maFieldSeparators, mcTextSep);
        mnStreamPos = mpDatStream->Tell();
    }

    //  #107455# If the file content isn't unicode, ReadUniStringLine
    //  may try to seek beyond the file's end and cause a CANTSEEK error
    //  (depending on the stream type). The error code has to be cleared,
    //  or further read operations (including non-unicode) will fail.
    if ( mpDatStream->GetError() == ERRCODE_IO_CANTSEEK )
        mpDatStream->ResetError();

    return bRet;
}


void ScImportAsciiDlg::GetOptions( ScAsciiOptions& rOpt )
{
    rOpt.SetCharSet( meCharSet );
    rOpt.SetCharSetSystem( mbCharSetSystem );
    rOpt.SetLanguage(aLbCustomLang.GetSelectLanguage());
    rOpt.SetFixedLen( aRbFixed.IsChecked() );
    rOpt.SetStartRow( (long)aNfRow.GetValue() );
    maTableBox.FillColumnData( rOpt );
    if( aRbSeparated.IsChecked() )
    {
        rOpt.SetFieldSeps( GetSeparators() );
        rOpt.SetMergeSeps( aCkbAsOnce.IsChecked() );
        rOpt.SetTextSep( lcl_CharFromCombo( aCbTextSep, aTextSepList ) );
    }

    rOpt.SetQuotedAsText(aCkbQuotedAsText.IsChecked());
    rOpt.SetDetectSpecialNumber(aCkbDetectNumber.IsChecked());
}

void ScImportAsciiDlg::SetTextToColumnsMode()
{
    SetText( maStrTextToColumns );
    aFtCharSet.Disable();
    aLbCharSet.Disable();
    aFtCustomLang.Disable();
    aLbCustomLang.SelectLanguage(LANGUAGE_SYSTEM);
    aLbCustomLang.Disable();
    aFtRow.Disable();
    aNfRow.Disable();

    // Quoted field as text option is not used for text-to-columns mode.
    aCkbQuotedAsText.Check(false);
    aCkbQuotedAsText.Disable();

    // Always detect special numbers for text-to-columns mode.
    aCkbDetectNumber.Check();
    aCkbDetectNumber.Disable();
}

void ScImportAsciiDlg::SaveParameters()
{
    if (!mbFileImport)
        // We save parameters only for file import.
        return;

    save_Separators( maFieldSeparators, aCbTextSep.GetText(), aCkbAsOnce.IsChecked(),
                     aCkbQuotedAsText.IsChecked(), aCkbDetectNumber.IsChecked(),
                     aRbFixed.IsChecked(),
                     static_cast<sal_Int32>(aNfRow.GetValue()),
                     static_cast<sal_Int32>(aLbCharSet.GetSelectEntryPos()),
                     static_cast<sal_Int32>(aLbCustomLang.GetSelectLanguage()) );
}

void ScImportAsciiDlg::SetSelectedCharSet()
{
    meCharSet = aLbCharSet.GetSelectTextEncoding();
    mbCharSetSystem = (meCharSet == RTL_TEXTENCODING_DONTKNOW);
    if( mbCharSetSystem )
        meCharSet = gsl_getSystemTextEncoding();
}

String ScImportAsciiDlg::GetSeparators() const
{
    String aSepChars;
    if( aCkbTab.IsChecked() )
        aSepChars += '\t';
    if( aCkbSemicolon.IsChecked() )
        aSepChars += ';';
    if( aCkbComma.IsChecked() )
        aSepChars += ',';
    if( aCkbSpace.IsChecked() )
        aSepChars += ' ';
    if( aCkbOther.IsChecked() )
        aSepChars += aEdOther.GetText();
    return aSepChars;
}

void ScImportAsciiDlg::SetupSeparatorCtrls()
{
    sal_Bool bEnable = aRbSeparated.IsChecked();
    aCkbTab.Enable( bEnable );
    aCkbSemicolon.Enable( bEnable );
    aCkbComma.Enable( bEnable );
    aCkbSpace.Enable( bEnable );
    aCkbOther.Enable( bEnable );
    aEdOther.Enable( bEnable );
    aCkbAsOnce.Enable( bEnable );
    aFtTextSep.Enable( bEnable );
    aCbTextSep.Enable( bEnable );
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
    DBG_ASSERT( pButton, "ScImportAsciiDlg::RbSepFixHdl - missing sender" );

    if( (pButton == &aRbFixed) || (pButton == &aRbSeparated) )
    {
        SetPointer( Pointer( POINTER_WAIT ) );
        if( aRbFixed.IsChecked() )
            maTableBox.SetFixedWidthMode();
        else
            maTableBox.SetSeparatorsMode();
        SetPointer( Pointer( POINTER_ARROW ) );

        SetupSeparatorCtrls();
    }
    return 0;
}

IMPL_LINK( ScImportAsciiDlg, SeparatorHdl, Control*, pCtrl )
{
    DBG_ASSERT( pCtrl, "ScImportAsciiDlg::SeparatorHdl - missing sender" );
    DBG_ASSERT( !aRbFixed.IsChecked(), "ScImportAsciiDlg::SeparatorHdl - not allowed in fixed width" );

    /*  #i41550# First update state of the controls. The GetSeparators()
        function needs final state of the check boxes. */
    if( (pCtrl == &aCkbOther) && aCkbOther.IsChecked() )
        aEdOther.GrabFocus();
    else if( pCtrl == &aEdOther )
        aCkbOther.Check( aEdOther.GetText().Len() > 0 );

    String aOldFldSeps( maFieldSeparators);
    maFieldSeparators = GetSeparators();
    sal_Unicode cOldSep = mcTextSep;
    mcTextSep = lcl_CharFromCombo( aCbTextSep, aTextSepList );
    // Any separator changed may result in completely different lines due to
    // embedded line breaks.
    if (cOldSep != mcTextSep || aOldFldSeps != maFieldSeparators)
        UpdateVertical();

    maTableBox.Execute( CSVCMD_NEWCELLTEXTS );
    return 0;
}

IMPL_LINK( ScImportAsciiDlg, CharSetHdl, SvxTextEncodingBox*, pCharSetBox )
{
    DBG_ASSERT( pCharSetBox, "ScImportAsciiDlg::CharSetHdl - missing sender" );

    if( (pCharSetBox == &aLbCharSet) && (pCharSetBox->GetSelectEntryCount() == 1) )
    {
        SetPointer( Pointer( POINTER_WAIT ) );
        CharSet eOldCharSet = meCharSet;
        SetSelectedCharSet();
        // switching char-set invalidates 8bit -> String conversions
        if (eOldCharSet != meCharSet)
            UpdateVertical();

        maTableBox.Execute( CSVCMD_NEWCELLTEXTS );
        SetPointer( Pointer( POINTER_ARROW ) );
    }
    return 0;
}

IMPL_LINK( ScImportAsciiDlg, FirstRowHdl, NumericField*, pNumField )
{
    DBG_ASSERT( pNumField, "ScImportAsciiDlg::FirstRowHdl - missing sender" );
    maTableBox.Execute( CSVCMD_SETFIRSTIMPORTLINE, sal::static_int_cast<sal_Int32>( pNumField->GetValue() - 1 ) );
    return 0;
}

IMPL_LINK( ScImportAsciiDlg, LbColTypeHdl, ListBox*, pListBox )
{
    DBG_ASSERT( pListBox, "ScImportAsciiDlg::LbColTypeHdl - missing sender" );
    if( pListBox == &aLbType )
        maTableBox.Execute( CSVCMD_SETCOLUMNTYPE, pListBox->GetSelectEntryPos() );
    return 0;
}

IMPL_LINK( ScImportAsciiDlg, UpdateTextHdl, ScCsvTableBox*, EMPTYARG )
{
    sal_Int32 nBaseLine = maTableBox.GetFirstVisLine();
    sal_Int32 nRead = maTableBox.GetVisLineCount();
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
        maPreviewLine[i].Erase();

    maTableBox.Execute( CSVCMD_SETLINECOUNT, mnRowPosCount);
    bool bMergeSep = (aCkbAsOnce.IsChecked() == sal_True);
    maTableBox.SetUniStrings( maPreviewLine, maFieldSeparators, mcTextSep, bMergeSep);

    return 0;
}

IMPL_LINK( ScImportAsciiDlg, ColTypeHdl, ScCsvTableBox*, pTableBox )
{
    DBG_ASSERT( pTableBox, "ScImportAsciiDlg::ColTypeHdl - missing sender" );

    sal_Int32 nType = pTableBox->GetSelColumnType();
    sal_Int32 nTypeCount = aLbType.GetEntryCount();
    bool bEmpty = (nType == CSV_TYPE_MULTI);
    bool bEnable = ((0 <= nType) && (nType < nTypeCount)) || bEmpty;

    aFtType.Enable( bEnable );
    aLbType.Enable( bEnable );

    Link aSelHdl = aLbType.GetSelectHdl();
    aLbType.SetSelectHdl( Link() );
    if( bEmpty )
        aLbType.SetNoSelection();
    else if( bEnable )
        aLbType.SelectEntryPos( static_cast< sal_uInt16 >( nType ) );
    aLbType.SetSelectHdl( aSelHdl );

    return 0;
}

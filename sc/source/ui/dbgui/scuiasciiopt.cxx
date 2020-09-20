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

#include <unicode/uclean.h>
#include <unicode/ucsdet.h>

//! TODO make dynamic
const SCSIZE ASCIIDLG_MAXROWS                = MAXROWCOUNT;

using namespace com::sun::star::uno;

namespace {

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

}

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
constexpr OUStringLiteral aSep_Path =           u"Office.Calc/Dialogs/CSVImport";
constexpr OUStringLiteral aSep_Path_Clpbrd =    u"Office.Calc/Dialogs/ClipboardTextImport";
constexpr OUStringLiteral aSep_Path_Text2Col =  u"Office.Calc/Dialogs/TextToColumnsImport";

namespace {
CSVImportOptionsIndex getSkipEmptyCellsIndex( ScImportAsciiCall eCall )
{
    return eCall == SC_TEXTTOCOLUMNS ? CSVIO_Text2ColSkipEmptyCells : CSVIO_PasteSkipEmptyCells;
}
}

static void lcl_FillCombo(weld::ComboBox& rCombo, const OUString& rList, sal_Unicode cSelect)
{
    OUString aStr;
    if (!rList.isEmpty())
    {
        sal_Int32 nIdx {0};
        do
        {
            const OUString sEntry {rList.getToken(0, '\t', nIdx)};
            rCombo.append_text(sEntry);
            if (nIdx>0 && static_cast<sal_Unicode>(rList.getToken(0, '\t', nIdx).toInt32()) == cSelect)
                aStr = sEntry;
        }
        while (nIdx>0);
    }

    if ( cSelect )
    {
        if (aStr.isEmpty())
            aStr = OUString(cSelect);         // Ascii

        rCombo.set_entry_text(aStr);
    }
}

static sal_Unicode lcl_CharFromCombo(const weld::ComboBox& rCombo, const OUString& rList)
{
    sal_Unicode c = 0;
    OUString aStr = rCombo.get_active_text();
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

constexpr OUStringLiteral gaTextSepList(u"" SCSTR_TEXTSEP);

ScImportAsciiDlg::ScImportAsciiDlg(weld::Window* pParent, const OUString& aDatName,
                                   SvStream* pInStream, ScImportAsciiCall eCall)
    : GenericDialogController(pParent, "modules/scalc/ui/textimportcsv.ui", "TextImportCsvDialog")
    , mpDatStream(pInStream)
    , mnStreamPos(pInStream ? pInStream->Tell() : 0)
    , mnRowPosCount(0)
    , mcTextSep(ScAsciiOptions::cDefaultTextSep)
    , meCall(eCall)
    , mbDetectSpaceSep(eCall != SC_TEXTTOCOLUMNS)
    , mxFtCharSet(m_xBuilder->weld_label("textcharset"))
    , mxLbCharSet(new SvxTextEncodingBox(m_xBuilder->weld_combo_box("charset")))
    , mxFtCustomLang(m_xBuilder->weld_label("textlanguage"))
    , mxLbCustomLang(new SvxLanguageBox(m_xBuilder->weld_combo_box("language")))
    , mxFtRow(m_xBuilder->weld_label("textfromrow"))
    , mxNfRow(m_xBuilder->weld_spin_button("fromrow"))
    , mxRbFixed(m_xBuilder->weld_radio_button("tofixedwidth"))
    , mxRbSeparated(m_xBuilder->weld_radio_button("toseparatedby"))
    , mxCkbTab(m_xBuilder->weld_check_button("tab"))
    , mxCkbSemicolon(m_xBuilder->weld_check_button("semicolon"))
    , mxCkbComma(m_xBuilder->weld_check_button("comma"))
    , mxCkbRemoveSpace(m_xBuilder->weld_check_button("removespace"))
    , mxCkbSpace(m_xBuilder->weld_check_button("space"))
    , mxCkbOther(m_xBuilder->weld_check_button("other"))
    , mxEdOther(m_xBuilder->weld_entry("inputother"))
    , mxCkbAsOnce(m_xBuilder->weld_check_button("mergedelimiters"))
    , mxFtTextSep(m_xBuilder->weld_label("texttextdelimiter"))
    , mxCbTextSep(m_xBuilder->weld_combo_box("textdelimiter"))
    , mxCkbQuotedAsText(m_xBuilder->weld_check_button("quotedfieldastext"))
    , mxCkbDetectNumber(m_xBuilder->weld_check_button("detectspecialnumbers"))
    , mxCkbSkipEmptyCells(m_xBuilder->weld_check_button("skipemptycells"))
    , mxFtType(m_xBuilder->weld_label("textcolumntype"))
    , mxLbType(m_xBuilder->weld_combo_box("columntype"))
    , mxAltTitle(m_xBuilder->weld_label("textalttitle"))
    , mxTableBox(new ScCsvTableBox(*m_xBuilder))
{
    OUString aName = m_xDialog->get_title();
    switch (meCall)
    {
        case SC_TEXTTOCOLUMNS:
            m_xDialog->set_title(mxAltTitle->get_label());
            break;
        case SC_IMPORTFILE:
            aName += " - [" + aDatName + "]";
            m_xDialog->set_title(aName);
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
        mxCkbAsOnce->set_active(true);
    if (bQuotedFieldAsText)
        mxCkbQuotedAsText->set_active(true);
    if (bRemoveSpace)
        mxCkbRemoveSpace->set_active(true);
    if (bDetectSpecialNum)
        mxCkbDetectNumber->set_active(true);
    if (bSkipEmptyCells)
        mxCkbSkipEmptyCells->set_active(true);
    if (bFixedWidth && !bIsTSV)
        mxRbFixed->set_active(true);
    if (nFromRow != 1)
        mxNfRow->set_value(nFromRow);

    if ( bIsTSV )
        mxCkbTab->set_active(true);
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
        mpDatStream->Seek( 0 );
        constexpr size_t buffsize = 4096;
        sal_Int8 bytes[buffsize] = { 0 };
        sal_Int32 nRead = mpDatStream->ReadBytes( bytes, buffsize );
        mpDatStream->Seek( 0 );

        if ( nRead > 0 )
        {
            UErrorCode uerr = U_ZERO_ERROR;
            UCharsetDetector* ucd = ucsdet_open( &uerr );
            ucsdet_setText( ucd, reinterpret_cast<const char*>(bytes), nRead, &uerr );
            const UCharsetMatch* match = ucsdet_detect( ucd, &uerr );
            const char* pEncodingName = ucsdet_getName( match, &uerr );

            if ( U_SUCCESS(uerr) && !strcmp("UTF-8", pEncodingName) )
            {
                ePreselectUnicode = RTL_TEXTENCODING_UTF8; // UTF-8
                mpDatStream->StartReadingUnicodeText( RTL_TEXTENCODING_UTF8 );
            }
            else if ( U_SUCCESS(uerr) && !strcmp("UTF-16LE", pEncodingName) )
            {
                ePreselectUnicode = RTL_TEXTENCODING_UNICODE; // UTF-16LE
                mpDatStream->SetEndian( SvStreamEndian::LITTLE );
                mpDatStream->StartReadingUnicodeText( RTL_TEXTENCODING_UNICODE );
            }
            else if ( U_SUCCESS(uerr) && !strcmp("UTF-16BE", pEncodingName) )
            {
                ePreselectUnicode = RTL_TEXTENCODING_UNICODE; // UTF-16BE
                mpDatStream->SetEndian(SvStreamEndian::BIG);
                mpDatStream->StartReadingUnicodeText( RTL_TEXTENCODING_UNICODE );
            }
            else // other
                mpDatStream->StartReadingUnicodeText( RTL_TEXTENCODING_DONTKNOW );

            ucsdet_close( ucd );
        }

        mnStreamPos = mpDatStream->Tell();
    }

    mxNfRow->connect_value_changed( LINK( this, ScImportAsciiDlg, FirstRowHdl ) );

    // *** Separator characters ***
    lcl_FillCombo( *mxCbTextSep, gaTextSepList, mcTextSep );
    mxCbTextSep->set_entry_text(sTextSeparators);

    Link<weld::Button&,void> aSeparatorClickHdl =LINK( this, ScImportAsciiDlg, SeparatorClickHdl );
    mxCbTextSep->connect_changed( LINK( this, ScImportAsciiDlg, SeparatorComboBoxHdl ) );
    mxCkbTab->connect_clicked( aSeparatorClickHdl );
    mxCkbSemicolon->connect_clicked( aSeparatorClickHdl );
    mxCkbComma->connect_clicked( aSeparatorClickHdl );
    mxCkbAsOnce->connect_clicked( aSeparatorClickHdl );
    mxCkbQuotedAsText->connect_clicked( aSeparatorClickHdl );
    mxCkbDetectNumber->connect_clicked( aSeparatorClickHdl );
    mxCkbSkipEmptyCells->connect_clicked( aSeparatorClickHdl );
    mxCkbSpace->connect_clicked( aSeparatorClickHdl );
    mxCkbRemoveSpace->connect_clicked( aSeparatorClickHdl );
    mxCkbOther->connect_clicked( aSeparatorClickHdl );
    mxEdOther->connect_changed(LINK(this, ScImportAsciiDlg, SeparatorEditHdl));

    // *** text encoding ListBox ***
    // all encodings allowed, including Unicode, but subsets are excluded
    mxLbCharSet->FillFromTextEncodingTable( true );
    // Insert one "SYSTEM" entry for compatibility in AsciiOptions and system
    // independent document linkage.
    mxLbCharSet->InsertTextEncoding( RTL_TEXTENCODING_DONTKNOW, ScResId( SCSTR_CHARSET_USER ) );
    if ( ePreselectUnicode == RTL_TEXTENCODING_DONTKNOW )
    {
        rtl_TextEncoding eSystemEncoding = osl_getThreadTextEncoding();
        // Prefer UTF-8, as UTF-16 would have already been detected from the stream.
        // This gives a better chance that the file is going to be opened correctly.
        if ( ( eSystemEncoding == RTL_TEXTENCODING_UNICODE ) && mpDatStream )
            eSystemEncoding = RTL_TEXTENCODING_UTF8;
        mxLbCharSet->SelectTextEncoding( eSystemEncoding );
    }
    else
    {
        mxLbCharSet->SelectTextEncoding( ePreselectUnicode );
    }

    if (nCharSet >= 0 && ePreselectUnicode == RTL_TEXTENCODING_DONTKNOW)
        mxLbCharSet->set_active(nCharSet);

    SetSelectedCharSet();
    mxLbCharSet->connect_changed( LINK( this, ScImportAsciiDlg, CharSetHdl ) );

    mxLbCustomLang->SetLanguageList(
        SvxLanguageListFlags::ALL | SvxLanguageListFlags::ONLY_KNOWN, false, false);
    mxLbCustomLang->InsertLanguage(LANGUAGE_SYSTEM);
    mxLbCustomLang->set_active_id(static_cast<LanguageType>(nLanguage));

    // *** column type ListBox ***
    OUString aColumnUser( ScResId( SCSTR_COLUMN_USER ) );
    for (sal_Int32 nIdx {0}; nIdx>=0; )
    {
        mxLbType->append_text(aColumnUser.getToken(0, ';', nIdx));
    }

    mxLbType->connect_changed( LINK( this, ScImportAsciiDlg, LbColTypeHdl ) );
    mxFtType->set_sensitive(false);
    mxLbType->set_sensitive(false);

    // *** table box preview ***
    mxTableBox->Init();
    mxTableBox->SetUpdateTextHdl( LINK( this, ScImportAsciiDlg, UpdateTextHdl ) );
    mxTableBox->InitTypes( *mxLbType );
    mxTableBox->SetColTypeHdl( LINK( this, ScImportAsciiDlg, ColTypeHdl ) );

    mxRbSeparated->connect_clicked( LINK( this, ScImportAsciiDlg, RbSepFixHdl ) );
    mxRbFixed->connect_clicked( LINK( this, ScImportAsciiDlg, RbSepFixHdl ) );

    SetupSeparatorCtrls();
    RbSepFixHdl(*mxRbFixed);

    UpdateVertical();

    mxTableBox->GetGrid().Execute( CSVCMD_NEWCELLTEXTS );

    if (meCall == SC_TEXTTOCOLUMNS)
    {
        mxFtCharSet->set_sensitive(false);
        mxLbCharSet->set_sensitive(false);
        mxFtCustomLang->set_sensitive(false);
        mxLbCustomLang->set_active_id(LANGUAGE_SYSTEM);
        mxLbCustomLang->set_sensitive(false);
        mxFtRow->set_sensitive(false);
        mxNfRow->set_sensitive(false);

        // Quoted field as text option is not used for text-to-columns mode.
        mxCkbQuotedAsText->set_active(false);
        mxCkbQuotedAsText->set_sensitive(false);

        // Always detect special numbers for text-to-columns mode.
        mxCkbDetectNumber->set_active(true);
        mxCkbDetectNumber->set_sensitive(false);
    }
    if (meCall == SC_IMPORTFILE)
    {
        //Empty cells in imported file are empty
        mxCkbSkipEmptyCells->set_active(false);
        mxCkbSkipEmptyCells->hide();
    }
}

ScImportAsciiDlg::~ScImportAsciiDlg()
{
}

bool ScImportAsciiDlg::GetLine( sal_uLong nLine, OUString &rText, sal_Unicode& rcDetectSep )
{
    if (nLine >= ASCIIDLG_MAXROWS || !mpDatStream)
        return false;

    bool bRet = true;
    bool bFixed = mxRbFixed->get_active();

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
    rOpt.SetLanguage(mxLbCustomLang->get_active_id());
    rOpt.SetFixedLen( mxRbFixed->get_active() );
    rOpt.SetStartRow( mxNfRow->get_value() );
    mxTableBox->FillColumnData( rOpt );
    if( mxRbSeparated->get_active() )
    {
        rOpt.SetFieldSeps( GetSeparators() );
        rOpt.SetMergeSeps( mxCkbAsOnce->get_active() );
        rOpt.SetRemoveSpace( mxCkbRemoveSpace->get_active() );
        rOpt.SetTextSep( lcl_CharFromCombo( *mxCbTextSep, gaTextSepList ) );
    }

    rOpt.SetQuotedAsText(mxCkbQuotedAsText->get_active());
    rOpt.SetDetectSpecialNumber(mxCkbDetectNumber->get_active());
    rOpt.SetSkipEmptyCells(mxCkbSkipEmptyCells->get_active());
}

void ScImportAsciiDlg::SaveParameters()
{
    lcl_SaveSeparators( maFieldSeparators, mxCbTextSep->get_active_text(), mxCkbAsOnce->get_active(),
                     mxCkbQuotedAsText->get_active(), mxCkbDetectNumber->get_active(),
                     mxRbFixed->get_active(),
                     mxNfRow->get_value(),
                     mxLbCharSet->get_active(),
                     static_cast<sal_uInt16>(mxLbCustomLang->get_active_id()),
                     mxCkbSkipEmptyCells->get_active(), mxCkbRemoveSpace->get_active(), meCall );
}

void ScImportAsciiDlg::SetSeparators()
{
    OString sString(OUStringToOString(maFieldSeparators,
        RTL_TEXTENCODING_MS_1252));
    const char *aSep = sString.getStr();
    sal_Int32 len = maFieldSeparators.getLength();
    for (int i = 0; i < len; ++i)
    {
        switch( aSep[i] )
        {
            case '\t':  mxCkbTab->set_active(true);        break;
            case ';':   mxCkbSemicolon->set_active(true);  break;
            case ',':   mxCkbComma->set_active(true);      break;
            case ' ':   mxCkbSpace->set_active(true);      break;
            default:
                mxCkbOther->set_active(true);
                mxEdOther->set_text(mxEdOther->get_text() + OUStringChar(aSep[i]));
        }
    }
}

void ScImportAsciiDlg::SetSelectedCharSet()
{
    meCharSet = mxLbCharSet->GetSelectTextEncoding();
    mbCharSetSystem = (meCharSet == RTL_TEXTENCODING_DONTKNOW);
    if( mbCharSetSystem )
        meCharSet = osl_getThreadTextEncoding();
}

OUString ScImportAsciiDlg::GetSeparators() const
{
    OUString aSepChars;
    if( mxCkbTab->get_active() )
        aSepChars += "\t";
    if( mxCkbSemicolon->get_active() )
        aSepChars += ";";
    if( mxCkbComma->get_active() )
        aSepChars += ",";
    if( mxCkbSpace->get_active() )
        aSepChars += " ";
    if( mxCkbOther->get_active() )
        aSepChars += mxEdOther->get_text();
    return aSepChars;
}

void ScImportAsciiDlg::SetupSeparatorCtrls()
{
    bool bEnable = mxRbSeparated->get_active();
    mxCkbTab->set_sensitive( bEnable );
    mxCkbSemicolon->set_sensitive( bEnable );
    mxCkbComma->set_sensitive( bEnable );
    mxCkbSpace->set_sensitive( bEnable );
    mxCkbRemoveSpace->set_sensitive( bEnable );
    mxCkbOther->set_sensitive( bEnable );
    mxEdOther->set_sensitive( bEnable );
    mxCkbAsOnce->set_sensitive( bEnable );
    mxFtTextSep->set_sensitive( bEnable );
    mxCbTextSep->set_sensitive( bEnable );
}

void ScImportAsciiDlg::UpdateVertical()
{
    mnRowPosCount = 0;
    if (mpDatStream)
        mpDatStream->SetStreamCharSet(meCharSet);
}

IMPL_LINK(ScImportAsciiDlg, RbSepFixHdl, weld::Button&, rButton, void)
{
    if (&rButton == mxRbFixed.get() || &rButton == mxRbSeparated.get())
    {
        weld::WaitObject aWaitObj(m_xDialog.get());
        if( mxRbFixed->get_active() )
            mxTableBox->SetFixedWidthMode();
        else
            mxTableBox->SetSeparatorsMode();
        SetupSeparatorCtrls();
    }
}

IMPL_LINK(ScImportAsciiDlg, SeparatorClickHdl, weld::Button&, rCtrl, void)
{
    SeparatorHdl(&rCtrl);
}

IMPL_LINK( ScImportAsciiDlg, SeparatorComboBoxHdl, weld::ComboBox&, rCtrl, void )
{
    SeparatorHdl(&rCtrl);
}

IMPL_LINK( ScImportAsciiDlg, SeparatorEditHdl, weld::Entry&, rEdit, void )
{
    SeparatorHdl(&rEdit);
}

void ScImportAsciiDlg::SeparatorHdl(const weld::Widget* pCtrl)
{
    OSL_ENSURE( pCtrl, "ScImportAsciiDlg::SeparatorHdl - missing sender" );
    OSL_ENSURE( !mxRbFixed->get_active(), "ScImportAsciiDlg::SeparatorHdl - not allowed in fixed width" );

    /*  #i41550# First update state of the controls. The GetSeparators()
        function needs final state of the check boxes. */
    if (pCtrl == mxCkbOther.get() && mxCkbOther->get_active())
        mxEdOther->grab_focus();
    else if (pCtrl == mxEdOther.get())
        mxCkbOther->set_active(!mxEdOther->get_text().isEmpty());

    OUString aOldFldSeps( maFieldSeparators);
    maFieldSeparators = GetSeparators();
    sal_Unicode cOldSep = mcTextSep;
    mcTextSep = lcl_CharFromCombo( *mxCbTextSep, gaTextSepList );
    // Any separator changed may result in completely different lines due to
    // embedded line breaks.
    if (cOldSep != mcTextSep || aOldFldSeps != maFieldSeparators)
        UpdateVertical();

    mxTableBox->GetGrid().Execute( CSVCMD_NEWCELLTEXTS );
}

IMPL_LINK_NOARG(ScImportAsciiDlg, CharSetHdl, weld::ComboBox&, void)
{
    if (mxLbCharSet->get_active() != -1)
    {
        weld::WaitObject aWaitObj(m_xDialog.get());
        rtl_TextEncoding eOldCharSet = meCharSet;
        SetSelectedCharSet();
        // switching char-set invalidates 8bit -> String conversions
        if (eOldCharSet != meCharSet)
            UpdateVertical();

        mxTableBox->GetGrid().Execute( CSVCMD_NEWCELLTEXTS );
    }
}

IMPL_LINK(ScImportAsciiDlg, FirstRowHdl, weld::SpinButton&, rNumField, void)
{
    mxTableBox->GetGrid().Execute( CSVCMD_SETFIRSTIMPORTLINE, rNumField.get_value() - 1);
}

IMPL_LINK(ScImportAsciiDlg, LbColTypeHdl, weld::ComboBox&, rListBox, void)
{
    if (&rListBox == mxLbType.get())
        mxTableBox->GetGrid().Execute(CSVCMD_SETCOLUMNTYPE, rListBox.get_active());
}

IMPL_LINK_NOARG(ScImportAsciiDlg, UpdateTextHdl, ScCsvTableBox&, void)
{
    // Checking the separator can only be done once for the very first time
    // when the dialog wasn't already presented to the user.
    // As a side effect this has the benefit that the check is only done on the
    // first set of visible lines.
    sal_Unicode cDetectSep = (mbDetectSpaceSep && !mxRbFixed->get_active() && !mxCkbSpace->get_active() ? 0 : 0xffff);

    sal_Int32 nBaseLine = mxTableBox->GetGrid().GetFirstVisLine();
    sal_Int32 nRead = mxTableBox->GetGrid().GetVisLineCount();
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
            mxCkbSpace->set_active(true);
        }
    }

    mxTableBox->GetGrid().Execute( CSVCMD_SETLINECOUNT, mnRowPosCount);
    bool bMergeSep = mxCkbAsOnce->get_active();
    bool bRemoveSpace = mxCkbRemoveSpace->get_active();
    mxTableBox->SetUniStrings( maPreviewLine, maFieldSeparators, mcTextSep, bMergeSep, bRemoveSpace );
}

IMPL_LINK( ScImportAsciiDlg, ColTypeHdl, ScCsvTableBox&, rTableBox, void )
{
    sal_Int32 nType = rTableBox.GetSelColumnType();
    sal_Int32 nTypeCount = mxLbType->get_count();
    bool bEmpty = (nType == CSV_TYPE_MULTI);
    bool bEnable = ((0 <= nType) && (nType < nTypeCount)) || bEmpty;

    mxFtType->set_sensitive( bEnable );
    mxLbType->set_sensitive( bEnable );

    if (bEmpty)
        mxLbType->set_active(-1);
    else if (bEnable)
        mxLbType->set_active(nType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

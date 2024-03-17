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
#include <vcl/svapp.hxx>
#include <comphelper/lok.hxx>
#include <o3tl/string_view.hxx>

#include <unicode/ucsdet.h>

//! TODO make dynamic
const SCSIZE ASCIIDLG_MAXROWS                = MAXROWCOUNT;

// Maximum number of source lines to concatenate while generating the preview
// for one logical line. This may result in a wrong preview if the actual
// number of embedded line feeds is greater, but a number too high would take
// too much time (loop excessively if unlimited and large data) if none of the
// selected separators are actually used in data but a field at start of line
// is quoted.
constexpr sal_uInt32 kMaxEmbeddedLinefeeds = 500;

using namespace com::sun::star::uno;

namespace {

// Defines - CSV Import Preserve Options
// For usage of index order see lcl_CreatePropertiesNames() below.
enum CSVImportOptionsIndex
{
    CSVIO_MergeDelimiters = 0,
    CSVIO_Separators,
    CSVIO_TextSeparators,
    CSVIO_FixedWidth,
    CSVIO_RemoveSpace,
    CSVIO_EvaluateFormulas,
    // Settings for *all* dialog invocations above.
    // Settings not for SC_TEXTTOCOLUMNS below.
    CSVIO_FromRow,
    CSVIO_Text2ColSkipEmptyCells = CSVIO_FromRow,
    CSVIO_CharSet,
    CSVIO_QuotedAsText,
    CSVIO_DetectSpecialNum,
    CSVIO_DetectScientificNum,
    CSVIO_Language,
    // Plus one not for SC_IMPORTFILE.
    CSVIO_PasteSkipEmptyCells
};

}

// Config items for all three paths are defined in
// officecfg/registry/schema/org/openoffice/Office/Calc.xcs
// If not, options are neither loaded nor saved.
const ::std::vector<OUString> CSVImportOptionNames =
{
    "MergeDelimiters",
    "Separators",
    "TextSeparators",
    "FixedWidth",
    "RemoveSpace",
    "EvaluateFormulas",
    "FromRow",
    "CharSet",
    "QuotedFieldAsText",
    "DetectSpecialNumbers",
    "DetectScientificNumbers",
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

static void lcl_FillCombo(weld::ComboBox& rCombo, std::u16string_view rList, sal_Unicode cSelect)
{
    OUString aStr;
    if (!rList.empty())
    {
        sal_Int32 nIdx {0};
        do
        {
            const OUString sEntry {o3tl::getToken(rList, 0, '\t', nIdx)};
            rCombo.append_text(sEntry);
            if (nIdx>0 && static_cast<sal_Unicode>(o3tl::toInt32(o3tl::getToken(rList, 0, '\t', nIdx))) == cSelect)
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

static sal_Unicode lcl_CharFromCombo(const weld::ComboBox& rCombo, std::u16string_view rList)
{
    sal_Unicode c = 0;
    OUString aStr = rCombo.get_active_text();
    if ( !aStr.isEmpty() && !rList.empty() )
    {
        sal_Int32 nIdx {0};
        OUString sToken {o3tl::getToken(rList, 0, '\t', nIdx)};
        while (nIdx>0)
        {
            if ( ScGlobal::GetTransliteration().isEqual( aStr, sToken ) )
            {
                sal_Int32 nTmpIdx {nIdx};
                c = static_cast<sal_Unicode>(o3tl::toInt32(o3tl::getToken(rList, 0, '\t', nTmpIdx)));
            }
            // Skip to next token at even position
            sToken = o3tl::getToken(rList, 1, '\t', nIdx);
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
            nProperties = 12;
            break;
        case SC_PASTETEXT:
            rSepPath = aSep_Path_Clpbrd;
            nProperties = 13;
            break;
        case SC_TEXTTOCOLUMNS:
        default:
            rSepPath = aSep_Path_Text2Col;
            nProperties = 7;
            break;
    }
    rNames.realloc( nProperties );
    OUString* pNames = rNames.getArray();
    pNames[ CSVIO_MergeDelimiters ] =   CSVImportOptionNames[ CSVIO_MergeDelimiters ];
    pNames[ CSVIO_Separators ] =        CSVImportOptionNames[ CSVIO_Separators ];
    pNames[ CSVIO_TextSeparators ] =    CSVImportOptionNames[ CSVIO_TextSeparators ];
    pNames[ CSVIO_FixedWidth ] =        CSVImportOptionNames[ CSVIO_FixedWidth ];
    pNames[ CSVIO_RemoveSpace ] =       CSVImportOptionNames[ CSVIO_RemoveSpace ];
    pNames[ CSVIO_EvaluateFormulas ] =  CSVImportOptionNames[ CSVIO_EvaluateFormulas ];
    if (eCall != SC_TEXTTOCOLUMNS)
    {
        pNames[ CSVIO_FromRow ] =       CSVImportOptionNames[ CSVIO_FromRow ];
        pNames[ CSVIO_CharSet ] =       CSVImportOptionNames[ CSVIO_CharSet ];
        pNames[ CSVIO_QuotedAsText ] =  CSVImportOptionNames[ CSVIO_QuotedAsText ];
        pNames[ CSVIO_DetectSpecialNum ] = CSVImportOptionNames[ CSVIO_DetectSpecialNum ];
        pNames[ CSVIO_DetectScientificNum ] = CSVImportOptionNames[ CSVIO_DetectScientificNum ];
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
                             bool& rMergeDelimiters, bool& rQuotedAsText, bool& rDetectSpecialNum, bool& rDetectScientificNum,
                             bool& rFixedWidth, sal_Int32& rFromRow, sal_Int32& rCharSet,
                             sal_Int32& rLanguage, bool& rSkipEmptyCells, bool& rRemoveSpace,
                             bool& rEvaluateFormulas, ScImportAsciiCall eCall )
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

    if( pProperties[ CSVIO_EvaluateFormulas ].hasValue() )
        rEvaluateFormulas = ScUnoHelpFunctions::GetBoolFromAny( pProperties[ CSVIO_EvaluateFormulas ] );

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

        if ( pProperties[ CSVIO_DetectScientificNum ].hasValue() )
            pProperties[ CSVIO_DetectScientificNum ] >>= rDetectScientificNum;

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
    bool bDetectSpecialNum, bool bDetectScientificNum, bool bFixedWidth, sal_Int32 nFromRow,
    sal_Int32 nCharSet, sal_Int32 nLanguage, bool bSkipEmptyCells, bool bRemoveSpace, bool bEvaluateFormulas,
    ScImportAsciiCall eCall )
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
    pProperties[ CSVIO_EvaluateFormulas ] <<= bEvaluateFormulas;
    if (eCall != SC_TEXTTOCOLUMNS)
    {
        pProperties[ CSVIO_FromRow ] <<= nFromRow;
        pProperties[ CSVIO_CharSet ] <<= nCharSet;
        pProperties[ CSVIO_QuotedAsText ] <<= bQuotedAsText;
        pProperties[ CSVIO_DetectSpecialNum ] <<= bDetectSpecialNum;
        pProperties[ CSVIO_DetectScientificNum ] <<= bDetectScientificNum;
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

ScImportAsciiDlg::ScImportAsciiDlg(weld::Window* pParent, std::u16string_view aDatName,
                                   SvStream* pInStream, ScImportAsciiCall eCall,
                                   const ScAsciiOptions* aOptions)
    : GenericDialogController(pParent, "modules/scalc/ui/textimportcsv.ui", "TextImportCsvDialog")
    , mpDatStream(pInStream)
    , mnStreamPos(pInStream ? pInStream->Tell() : 0)
    , mnRowPosCount(0)
    , mcTextSep(ScAsciiOptions::cDefaultTextSep)
    , meCall(eCall)
    , mbDetectSep(eCall != SC_TEXTTOCOLUMNS)
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
    , mxCkbDetectScientificNumber(m_xBuilder->weld_check_button("detectscientificnumbers"))
    , mxCkbEvaluateFormulas(m_xBuilder->weld_check_button("evaluateformulas"))
    , mxCkbSkipEmptyCells(m_xBuilder->weld_check_button("skipemptycells"))
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
            if (!comphelper::LibreOfficeKit::isActive())
            {
                aName += OUString::Concat(" - [") + aDatName + "]";
                m_xDialog->set_title(aName);
            }
            break;
        default:
            break;
    }

    // To be able to prefill the correct values based on the file extension
    bool bIsTSV = (o3tl::endsWithIgnoreAsciiCase(aDatName, ".tsv") || o3tl::endsWithIgnoreAsciiCase(aDatName, ".tab"));

    // Default options are set in officecfg/registry/schema/org/openoffice/Office/Calc.xcs
    OUString sFieldSeparators(",;\t");
    OUString sTextSeparators(mcTextSep);
    bool bMergeDelimiters = false;
    bool bFixedWidth = false;
    bool bQuotedFieldAsText = false;
    bool bDetectSpecialNum = true;
    bool bDetectScientificNum = true;
    bool bEvaluateFormulas = (meCall != SC_IMPORTFILE);
    bool bSkipEmptyCells = true;
    bool bRemoveSpace = false;
    sal_Int32 nFromRow = 1;
    sal_Int32 nCharSet = -1;
    sal_Int32 nLanguage = 0;

    if (aOptions)
    {
        if (!aOptions->GetFieldSeps().isEmpty())
            sFieldSeparators = aOptions->GetFieldSeps();
        if (aOptions->GetTextSep())
            sTextSeparators = OUStringChar(aOptions->GetTextSep());
        bMergeDelimiters = aOptions->IsMergeSeps();
        bFixedWidth = aOptions->IsFixedLen();
        bQuotedFieldAsText = aOptions->IsQuotedAsText();
        bDetectSpecialNum = aOptions->IsDetectSpecialNumber();
        bDetectScientificNum = aOptions->IsDetectScientificNumber();
        bEvaluateFormulas = aOptions->IsEvaluateFormulas();
        bSkipEmptyCells = aOptions->IsSkipEmptyCells();
        bRemoveSpace = aOptions->IsRemoveSpace();
        nFromRow = aOptions->GetStartRow();
        nCharSet = aOptions->GetCharSet();
        nLanguage = static_cast<sal_uInt16>(aOptions->GetLanguage());
    }
    else
        lcl_LoadSeparators (sFieldSeparators, sTextSeparators, bMergeDelimiters,
                         bQuotedFieldAsText, bDetectSpecialNum, bDetectScientificNum, bFixedWidth, nFromRow,
                         nCharSet, nLanguage, bSkipEmptyCells, bRemoveSpace, bEvaluateFormulas, meCall);
    // load from saved settings
    maFieldSeparators = sFieldSeparators;

    if( bMergeDelimiters && !bIsTSV )
        mxCkbAsOnce->set_active(true);
    if (bQuotedFieldAsText)
        mxCkbQuotedAsText->set_active(true);
    if (bRemoveSpace)
        mxCkbRemoveSpace->set_active(true);
    if (bDetectSpecialNum)
    {
        mxCkbDetectNumber->set_active(true);
        bDetectScientificNum = true;
        mxCkbDetectScientificNumber->set_sensitive(false);
    }
    if (bDetectScientificNum)
        mxCkbDetectScientificNumber->set_active(true);
    if (bEvaluateFormulas)
        mxCkbEvaluateFormulas->set_active(true);
    if (bSkipEmptyCells)
        mxCkbSkipEmptyCells->set_active(true);
    if (bFixedWidth && !bIsTSV)
        mxRbFixed->set_active(true);
    if (nFromRow != 1)
        mxNfRow->set_value(nFromRow);

    // Clipboard is always Unicode, else detect.
    rtl_TextEncoding ePreselectUnicode = (aOptions ? aOptions->GetCharSet() : (meCall == SC_IMPORTFILE ?
            RTL_TEXTENCODING_DONTKNOW : RTL_TEXTENCODING_UNICODE));
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

            if ( const UCharsetMatch* match = ucsdet_detect(ucd, &uerr) )
            {
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
                    mpDatStream->SetEndian( SvStreamEndian::BIG );
                    mpDatStream->StartReadingUnicodeText( RTL_TEXTENCODING_UNICODE );
                }
                else // other
                    mpDatStream->StartReadingUnicodeText( RTL_TEXTENCODING_DONTKNOW );
            }

            ucsdet_close( ucd );
        }

        mnStreamPos = mpDatStream->Tell();
    }

    if (aOptions && !maFieldSeparators.isEmpty())
        SetSeparators(0);
    else if (bIsTSV)
        SetSeparators('\t');
    else
    {
        // Some MS-Excel convention is the first line containing the field
        // separator as "sep=|" (without quotes and any field separator
        // character). The second possibility seems to be it is present *with*
        // quotes so it shows up as cell content *including* the separator and
        // can be preserved during round trips. Check for an exact match of
        // any such and set separator.
        /* TODO: it is debatable whether the unquoted form should rather be
         * treated special to actually include the separator in the field data.
         * Currently it does not. */
        sal_Unicode cSep = 0;
        OUString aLine;
        // Try to read one more character, if more than 7 it can't be an exact
        // match of any.
        mpDatStream->ReadUniOrByteStringLine( aLine, mpDatStream->GetStreamCharSet(), 8);
        mpDatStream->Seek(mnStreamPos);
        if (aLine.getLength() == 8)
            ;   // nothing
        else if (aLine.getLength() == 5 && aLine.startsWithIgnoreAsciiCase("sep="))
            cSep = aLine[4];
        else if (aLine.getLength() == 7 && aLine[6] == '"' && aLine.startsWithIgnoreAsciiCase("\"sep="))
            cSep = aLine[5];

        // Set Separators in the dialog from maFieldSeparators (empty are not
        // set) or an optionally defined by file content field separator.
        SetSeparators(cSep);
    }

    // Get Separators from the dialog (empty are set from default)
    maFieldSeparators = GetSeparators();

    mxNfRow->connect_value_changed( LINK( this, ScImportAsciiDlg, FirstRowHdl ) );

    // *** Separator characters ***
    lcl_FillCombo( *mxCbTextSep, SCSTR_TEXTSEP, mcTextSep );
    mxCbTextSep->set_entry_text(sTextSeparators);
    // tdf#69207 - use selected text delimiter to parse the provided data
    mcTextSep = lcl_CharFromCombo(*mxCbTextSep, SCSTR_TEXTSEP);

    Link<weld::Toggleable&,void> aSeparatorClickHdl =LINK( this, ScImportAsciiDlg, SeparatorClickHdl );
    Link<weld::Toggleable&,void> aOtherOptionsClickHdl =LINK( this, ScImportAsciiDlg, OtherOptionsClickHdl );
    mxCbTextSep->connect_changed( LINK( this, ScImportAsciiDlg, SeparatorComboBoxHdl ) );
    mxCkbTab->connect_toggled( aSeparatorClickHdl );
    mxCkbSemicolon->connect_toggled( aSeparatorClickHdl );
    mxCkbComma->connect_toggled( aSeparatorClickHdl );
    mxCkbAsOnce->connect_toggled( aSeparatorClickHdl );
    mxCkbSpace->connect_toggled( aSeparatorClickHdl );
    mxCkbRemoveSpace->connect_toggled( aSeparatorClickHdl );
    mxCkbOther->connect_toggled( aSeparatorClickHdl );
    mxEdOther->connect_changed(LINK(this, ScImportAsciiDlg, SeparatorEditHdl));
    mxCkbQuotedAsText->connect_toggled( aOtherOptionsClickHdl );
    mxCkbDetectNumber->connect_toggled( aOtherOptionsClickHdl );
    mxCkbDetectScientificNumber->connect_toggled( aOtherOptionsClickHdl );
    mxCkbEvaluateFormulas->connect_toggled( aOtherOptionsClickHdl );
    mxCkbSkipEmptyCells->connect_toggled( aOtherOptionsClickHdl );

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
    mxLbType->set_sensitive(false);

    // *** table box preview ***
    mxTableBox->Init();
    mxTableBox->SetUpdateTextHdl( LINK( this, ScImportAsciiDlg, UpdateTextHdl ) );
    mxTableBox->InitTypes( *mxLbType );
    mxTableBox->SetColTypeHdl( LINK( this, ScImportAsciiDlg, ColTypeHdl ) );

    mxRbSeparated->connect_toggled( LINK( this, ScImportAsciiDlg, RbSepFixHdl ) );
    mxRbFixed->connect_toggled( LINK( this, ScImportAsciiDlg, RbSepFixHdl ) );

    SetupSeparatorCtrls();
    RbSepFix();

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
        mxCkbDetectScientificNumber->set_active(true);
        mxCkbDetectScientificNumber->set_sensitive(false);
    }
    if (meCall == SC_IMPORTFILE)
    {
        //Empty cells in imported file are empty
        mxCkbSkipEmptyCells->set_active(false);
        mxCkbSkipEmptyCells->hide();
    }
    m_xDialog->SetInstallLOKNotifierHdl(LINK(this, ScImportAsciiDlg, InstallLOKNotifierHdl));
}

IMPL_STATIC_LINK_NOARG(ScImportAsciiDlg, InstallLOKNotifierHdl, void*, vcl::ILibreOfficeKitNotifier*)
{
    return GetpApp();
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
                    mcTextSep, rcDetectSep, kMaxEmbeddedLinefeeds);
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
        rText = ReadCsvLine(*mpDatStream, !bFixed, maFieldSeparators, mcTextSep, rcDetectSep, kMaxEmbeddedLinefeeds);
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
        rOpt.SetTextSep( lcl_CharFromCombo( *mxCbTextSep, SCSTR_TEXTSEP ) );
    }

    rOpt.SetQuotedAsText(mxCkbQuotedAsText->get_active());
    rOpt.SetDetectSpecialNumber(mxCkbDetectNumber->get_active());
    rOpt.SetDetectScientificNumber(mxCkbDetectScientificNumber->get_active());
    rOpt.SetEvaluateFormulas(mxCkbEvaluateFormulas->get_active());
    rOpt.SetSkipEmptyCells(mxCkbSkipEmptyCells->get_active());
}

void ScImportAsciiDlg::SaveParameters()
{
    lcl_SaveSeparators( maFieldSeparators, mxCbTextSep->get_active_text(), mxCkbAsOnce->get_active(),
                     mxCkbQuotedAsText->get_active(), mxCkbDetectNumber->get_active(), mxCkbDetectScientificNumber->get_active(),
                     mxRbFixed->get_active(),
                     mxNfRow->get_value(),
                     mxLbCharSet->get_active(),
                     static_cast<sal_uInt16>(mxLbCustomLang->get_active_id()),
                     mxCkbSkipEmptyCells->get_active(), mxCkbRemoveSpace->get_active(),
                     mxCkbEvaluateFormulas->get_active(), meCall );
}

void ScImportAsciiDlg::SetSeparators( sal_Unicode cSep )
{
    if (cSep)
    {
        // Exclusively set a separator, maFieldSeparators needs not be
        // modified, it's obtained by GetSeparators() after this call.
        constexpr sal_Unicode aSeps[] = { '\t', ';', ',', ' ' };
        for (const sal_Unicode c : aSeps)
        {
            const bool bSet = (c == cSep);
            switch (c)
            {
                case '\t':  mxCkbTab->set_active(bSet);        break;
                case ';':   mxCkbSemicolon->set_active(bSet);  break;
                case ',':   mxCkbComma->set_active(bSet);      break;
                case ' ':   mxCkbSpace->set_active(bSet);      break;
            }
            if (bSet)
                cSep = 0;
        }
        if (cSep)
        {
            mxCkbOther->set_active(true);
            mxEdOther->set_text(OUStringChar(cSep));
        }
    }
    else
    {
        for (sal_Int32 i = 0; i < maFieldSeparators.getLength(); ++i)
        {
            switch (maFieldSeparators[i])
            {
                case '\t':  mxCkbTab->set_active(true);        break;
                case ';':   mxCkbSemicolon->set_active(true);  break;
                case ',':   mxCkbComma->set_active(true);      break;
                case ' ':   mxCkbSpace->set_active(true);      break;
                default:
                            mxCkbOther->set_active(true);
                            mxEdOther->set_text(mxEdOther->get_text() + OUStringChar(maFieldSeparators[i]));
            }
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

void ScImportAsciiDlg::RbSepFix()
{
    weld::WaitObject aWaitObj(m_xDialog.get());
    if( mxRbFixed->get_active() )
        mxTableBox->SetFixedWidthMode();
    else
        mxTableBox->SetSeparatorsMode();
    SetupSeparatorCtrls();
}

IMPL_LINK(ScImportAsciiDlg, RbSepFixHdl, weld::Toggleable&, rButton, void)
{
    if (!rButton.get_active())
        return;
    RbSepFix();
}

IMPL_LINK(ScImportAsciiDlg, SeparatorClickHdl, weld::Toggleable&, rCtrl, void)
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

IMPL_LINK(ScImportAsciiDlg, OtherOptionsClickHdl, weld::Toggleable&, rCtrl, void)
{
    if (&rCtrl == mxCkbDetectNumber.get())
    {
        if (mxCkbDetectNumber->get_active())
        {
            mxCkbDetectScientificNumber->set_active(true);
            mxCkbDetectScientificNumber->set_sensitive(false);
        }
        else
            mxCkbDetectScientificNumber->set_sensitive(true);
        return;
    }
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
    mcTextSep = lcl_CharFromCombo( *mxCbTextSep, SCSTR_TEXTSEP );
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
    mbDetectSep = (mbDetectSep && !mxRbFixed->get_active()
                && (!mxCkbTab->get_active() || !mxCkbSemicolon->get_active()
                    || !mxCkbComma->get_active() || !mxCkbSpace->get_active()));
    sal_Unicode cDetectSep = (mbDetectSep ? 0 : 0xffff);

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

    if (mbDetectSep)
    {
        mbDetectSep = false;
        if (cDetectSep)
        {
            // Expect separator to be appended by now so all subsequent
            // GetLine()/ReadCsvLine() actually used it.
            assert(maFieldSeparators.endsWith(OUStringChar(cDetectSep)));
            // Preselect separator in UI.
            switch (cDetectSep)
            {
                case '\t':  mxCkbTab->set_active(true);         break;
                case ';':   mxCkbSemicolon->set_active(true);   break;
                case ',':   mxCkbComma->set_active(true);       break;
                case ' ':   mxCkbSpace->set_active(true);       break;
            }
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

    mxLbType->set_sensitive( bEnable );

    if (bEmpty)
        mxLbType->set_active(-1);
    else if (bEnable)
        mxLbType->set_active(nType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#include <comphelper/sequence.hxx>
#include <o3tl/string_view.hxx>

#include <unicode/ucsdet.h>
#include <sfx2/objsh.hxx>
#include <svx/txenctab.hxx>

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

enum SeparatorType
{
    FIXED,
    SEPARATOR,
    DETECT_SEPARATOR
};

}

// Config items for all three paths are defined in
// officecfg/registry/schema/org/openoffice/Office/Calc.xcs
// If not, options are neither loaded nor saved.
constexpr OUString CSVIO_MergeDelimiters = u"MergeDelimiters"_ustr;
constexpr OUString CSVIO_Separators = u"Separators"_ustr;
constexpr OUString CSVIO_TextSeparators = u"TextSeparators"_ustr;
constexpr OUString CSVIO_RemoveSpace = u"RemoveSpace"_ustr;
constexpr OUString CSVIO_EvaluateFormulas = u"EvaluateFormulas"_ustr;
constexpr OUString CSVIO_SeparatorType = u"SeparatorType"_ustr;
constexpr OUString CSVIO_FromRow = u"FromRow"_ustr;
constexpr OUString CSVIO_Encoding = u"Encoding"_ustr;
constexpr OUString CSVIO_QuotedAsText = u"QuotedFieldAsText"_ustr;
constexpr OUString CSVIO_DetectSpecialNum = u"DetectSpecialNumbers"_ustr;
constexpr OUString CSVIO_DetectScientificNum = u"DetectScientificNumbers"_ustr;
constexpr OUString CSVIO_Language = u"Language"_ustr;
constexpr OUString CSVIO_SkipEmptyCells = u"SkipEmptyCells"_ustr;

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

static OUString lcl_GetConfigPath(ScImportAsciiCall eCall)
{
    switch(eCall)
    {
        case SC_IMPORTFILE:
            return u"Office.Calc/Dialogs/CSVImport"_ustr;
        case SC_PASTETEXT:
            return u"Office.Calc/Dialogs/ClipboardTextImport"_ustr;
        case SC_TEXTTOCOLUMNS:
        default:
            return u"Office.Calc/Dialogs/TextToColumnsImport"_ustr;
    }
}

static void lcl_LoadSeparators(ScImportAsciiCall eCall, OUString& rFieldSeparators, OUString& rTextSeparators,
                             bool& rMergeDelimiters, bool& rQuotedAsText, bool& rDetectSpecialNum, bool& rDetectScientificNum,
                             SeparatorType& rSepType, sal_Int32& rFromRow, rtl_TextEncoding& rEncoding,
                             sal_Int32& rLanguage, bool& rSkipEmptyCells, bool& rRemoveSpace,
                             bool& rEvaluateFormulas)
{
    ScLinkConfigItem aItem(lcl_GetConfigPath(eCall));
    const Sequence<OUString> aNames = aItem.GetNodeNames({});
    const Sequence<Any> aValues = aItem.GetProperties(aNames);

    rEncoding = RTL_TEXTENCODING_DONTKNOW;

    for (sal_Int32 i = 0; i < aNames.getLength(); ++i)
    {
        const OUString& name = aNames[i];
        const Any& value = aValues[i];
        if (!value.hasValue())
            continue;
        if (name == CSVIO_MergeDelimiters)
            rMergeDelimiters = ScUnoHelpFunctions::GetBoolFromAny(value);
        else if (name == CSVIO_RemoveSpace)
            rRemoveSpace = ScUnoHelpFunctions::GetBoolFromAny(value);
        else if (name == CSVIO_Separators)
            value >>= rFieldSeparators;
        else if (name == CSVIO_TextSeparators)
            value >>= rTextSeparators;
        else if (name == CSVIO_SeparatorType)
            rSepType = static_cast<SeparatorType>(ScUnoHelpFunctions::GetInt16FromAny(value));
        else if (name == CSVIO_EvaluateFormulas)
            rEvaluateFormulas = ScUnoHelpFunctions::GetBoolFromAny(value);
        else if (name == CSVIO_FromRow)
            value >>= rFromRow;
        else if (name == CSVIO_Encoding)
            value >>= rEncoding;
        else if (name == CSVIO_QuotedAsText)
            value >>= rQuotedAsText;
        else if (name == CSVIO_DetectSpecialNum)
            value >>= rDetectSpecialNum;
        else if (name == CSVIO_DetectScientificNum)
            value >>= rDetectScientificNum;
        else if (name == CSVIO_Language)
            value >>= rLanguage;
        else if (name == CSVIO_SkipEmptyCells)
            rSkipEmptyCells = ScUnoHelpFunctions::GetBoolFromAny(value);
    }
}

static void lcl_SaveSeparators(ScImportAsciiCall eCall,
    const OUString& sFieldSeparators, const OUString& sTextSeparators, bool bMergeDelimiters, bool bQuotedAsText,
    bool bDetectSpecialNum, bool bDetectScientificNum, SeparatorType rSepType, sal_Int32 nFromRow,
    rtl_TextEncoding eEncoding, sal_Int32 nLanguage, bool bSkipEmptyCells, bool bRemoveSpace, bool bEvaluateFormulas)
{
    ScLinkConfigItem aItem(lcl_GetConfigPath(eCall));
    std::unordered_map<OUString, Any> properties;

    for (const OUString& name : aItem.GetNodeNames({}))
    {
        if (name == CSVIO_MergeDelimiters)
            properties[name] <<= bMergeDelimiters;
        else if (name == CSVIO_RemoveSpace)
            properties[name] <<= bRemoveSpace;
        else if (name == CSVIO_Separators)
            properties[name] <<= sFieldSeparators;
        else if (name == CSVIO_TextSeparators)
            properties[name] <<= sTextSeparators;
        else if (name == CSVIO_EvaluateFormulas)
            properties[name] <<= bEvaluateFormulas;
        else if (name == CSVIO_SeparatorType)
            properties[name] <<= static_cast<sal_Int16>(rSepType);
        else if (name == CSVIO_FromRow)
            properties[name] <<= nFromRow;
        else if (name == CSVIO_Encoding)
            properties[name] <<= eEncoding;
        else if (name == CSVIO_QuotedAsText)
            properties[name] <<= bQuotedAsText;
        else if (name == CSVIO_DetectSpecialNum)
            properties[name] <<= bDetectSpecialNum;
        else if (name == CSVIO_DetectScientificNum)
            properties[name] <<= bDetectScientificNum;
        else if (name == CSVIO_Language)
            properties[name] <<= nLanguage;
        else if (name == CSVIO_SkipEmptyCells)
            properties[name] <<= bSkipEmptyCells;
    }

    aItem.PutProperties(comphelper::mapKeysToSequence(properties),
                        comphelper::mapValuesToSequence(properties));
}

ScImportAsciiDlg::ScImportAsciiDlg(weld::Window* pParent, std::u16string_view aDatName,
                                   SvStream* pInStream, ScImportAsciiCall eCall)
    : GenericDialogController(pParent, u"modules/scalc/ui/textimportcsv.ui"_ustr, u"TextImportCsvDialog"_ustr)
    , mpDatStream(pInStream)
    , mnStreamPos(pInStream ? pInStream->Tell() : 0)
    , mnStreamInitPos(mnStreamPos)
    , mnRowPosCount(0)
    , mcTextSep(ScAsciiOptions::cDefaultTextSep)
    , meDetectedCharSet(RTL_TEXTENCODING_DONTKNOW)
    , mbCharSetDetect(true)
    , meCall(eCall)
    , mxFtCharSet(m_xBuilder->weld_label(u"textcharset"_ustr))
    , mxLbCharSet(new SvxTextEncodingBox(m_xBuilder->weld_combo_box(u"charset"_ustr)))
    , mxFtDetectedCharSet(m_xBuilder->weld_label(u"textdetectedcharset"_ustr))
    , mxFtCustomLang(m_xBuilder->weld_label(u"textlanguage"_ustr))
    , mxLbCustomLang(new SvxLanguageBox(m_xBuilder->weld_combo_box(u"language"_ustr)))
    , mxFtRow(m_xBuilder->weld_label(u"textfromrow"_ustr))
    , mxNfRow(m_xBuilder->weld_spin_button(u"fromrow"_ustr))
    , mxRbDetectSep(m_xBuilder->weld_radio_button(u"todetectseparator"_ustr))
    , mxRbFixed(m_xBuilder->weld_radio_button(u"tofixedwidth"_ustr))
    , mxRbSeparated(m_xBuilder->weld_radio_button(u"toseparatedby"_ustr))
    , mxCkbTab(m_xBuilder->weld_check_button(u"tab"_ustr))
    , mxCkbSemicolon(m_xBuilder->weld_check_button(u"semicolon"_ustr))
    , mxCkbComma(m_xBuilder->weld_check_button(u"comma"_ustr))
    , mxCkbRemoveSpace(m_xBuilder->weld_check_button(u"removespace"_ustr))
    , mxCkbSpace(m_xBuilder->weld_check_button(u"space"_ustr))
    , mxCkbOther(m_xBuilder->weld_check_button(u"other"_ustr))
    , mxEdOther(m_xBuilder->weld_entry(u"inputother"_ustr))
    , mxCkbAsOnce(m_xBuilder->weld_check_button(u"mergedelimiters"_ustr))
    , mxFtTextSep(m_xBuilder->weld_label(u"texttextdelimiter"_ustr))
    , mxCbTextSep(m_xBuilder->weld_combo_box(u"textdelimiter"_ustr))
    , mxCkbQuotedAsText(m_xBuilder->weld_check_button(u"quotedfieldastext"_ustr))
    , mxCkbDetectNumber(m_xBuilder->weld_check_button(u"detectspecialnumbers"_ustr))
    , mxCkbDetectScientificNumber(m_xBuilder->weld_check_button(u"detectscientificnumbers"_ustr))
    , mxCkbEvaluateFormulas(m_xBuilder->weld_check_button(u"evaluateformulas"_ustr))
    , mxCkbSkipEmptyCells(m_xBuilder->weld_check_button(u"skipemptycells"_ustr))
    , mxLbType(m_xBuilder->weld_combo_box(u"columntype"_ustr))
    , mxAltTitle(m_xBuilder->weld_label(u"textalttitle"_ustr))
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
    OUString sFieldSeparators(u",;\t"_ustr);
    OUString sTextSeparators(mcTextSep);
    bool bMergeDelimiters = false;
    SeparatorType eSepType = DETECT_SEPARATOR;
    bool bQuotedFieldAsText = false;
    bool bDetectSpecialNum = true;
    bool bDetectScientificNum = true;
    bool bEvaluateFormulas = (meCall != SC_IMPORTFILE);
    bool bSkipEmptyCells = true;
    bool bRemoveSpace = false;
    sal_Int32 nFromRow = 1;
    rtl_TextEncoding eEncoding = RTL_TEXTENCODING_DONTKNOW;
    sal_Int32 nLanguage = 0;

    lcl_LoadSeparators ( meCall, sFieldSeparators, sTextSeparators, bMergeDelimiters,
                         bQuotedFieldAsText, bDetectSpecialNum, bDetectScientificNum, eSepType, nFromRow,
                         eEncoding, nLanguage, bSkipEmptyCells, bRemoveSpace, bEvaluateFormulas);

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
    if (eSepType == SeparatorType::FIXED)
    {
        if (bIsTSV)
        {
            eSepType = SeparatorType::SEPARATOR;
            mxRbSeparated->set_active(true);
        }
        else
            mxRbFixed->set_active(true);
    }
    else if (eSepType == SeparatorType::SEPARATOR)
        mxRbSeparated->set_active(true);
    else
        mxRbDetectSep->set_active(true);
    if (nFromRow != 1)
        mxNfRow->set_value(nFromRow);

    // Detect character set only once and then use it for "Detect" option.
    SvStreamEndian eEndian;
    SfxObjectShell::DetectCharSet(*mpDatStream, meDetectedCharSet, eEndian);
    if (meDetectedCharSet == RTL_TEXTENCODING_UNICODE)
        mpDatStream->SetEndian(eEndian);
    else if ( meDetectedCharSet == RTL_TEXTENCODING_DONTKNOW )
    {
        meDetectedCharSet = osl_getThreadTextEncoding();
        // Prefer UTF-8, as UTF-16 would have already been detected from the stream.
        // This gives a better chance that the file is going to be opened correctly.
        if ( meDetectedCharSet == RTL_TEXTENCODING_UNICODE && mpDatStream )
            meDetectedCharSet = RTL_TEXTENCODING_UTF8;
    }

    if (bIsTSV)
        SetSeparators('\t');
    else
        SetSeparators(0);

    // Get Separators from the dialog (empty are set from default)
    maFieldSeparators = GetActiveSeparators();

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
    // Insert one for detecting charset.
    mxLbCharSet->InsertTextEncoding( RTL_TEXTENCODING_USER_DETECTED, "- " + ScResId( SCSTR_AUTOMATIC ) + " -" );

    // Clipboard is always Unicode, and TextToColumns doesn't use encoding.
    if (meCall != SC_IMPORTFILE)
        eEncoding = RTL_TEXTENCODING_UNICODE;
    else if (eEncoding == RTL_TEXTENCODING_DONTKNOW)
        eEncoding = RTL_TEXTENCODING_USER_DETECTED;

    mxLbCharSet->SelectTextEncoding(eEncoding);

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

    mxRbDetectSep->connect_toggled( LINK( this, ScImportAsciiDlg, RbSepFixHdl ) );
    mxRbSeparated->connect_toggled( LINK( this, ScImportAsciiDlg, RbSepFixHdl ) );
    mxRbFixed->connect_toggled( LINK( this, ScImportAsciiDlg, RbSepFixHdl ) );

    RbSepFix();

    UpdateVertical();

    mxTableBox->GetGrid().Execute( CSVCMD_NEWCELLTEXTS );

    if (meCall == SC_TEXTTOCOLUMNS)
    {
        m_xBuilder->weld_frame(u"frame1"_ustr)->hide(); // the whole "Import" section

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

    if (comphelper::LibreOfficeKit::isActive())
        m_xBuilder->weld_button(u"cancel"_ustr)->hide();
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
        if (mpDatStream->eof() && mnRowPosCount &&
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
    if( mxRbSeparated->get_active() || mxRbDetectSep->get_active())
    {
        rOpt.SetFieldSeps( GetActiveSeparators() );
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
    lcl_SaveSeparators(meCall, GetSeparators(), mxCbTextSep->get_active_text(), mxCkbAsOnce->get_active(),
                     mxCkbQuotedAsText->get_active(), mxCkbDetectNumber->get_active(), mxCkbDetectScientificNumber->get_active(),
                     mxRbFixed->get_active() ? FIXED : (mxRbDetectSep->get_active() ? DETECT_SEPARATOR : SEPARATOR),
                     mxNfRow->get_value(),
                     mxLbCharSet->GetSelectTextEncoding(),
                     static_cast<sal_uInt16>(mxLbCustomLang->get_active_id()),
                     mxCkbSkipEmptyCells->get_active(), mxCkbRemoveSpace->get_active(),
                     mxCkbEvaluateFormulas->get_active());
}

void ScImportAsciiDlg::SetSeparators( sal_Unicode cSep )
{
    if (cSep)
    {
        // Exclusively set a separator, maFieldSeparators needs not be
        // modified, it's obtained by GetSeparators() after this call.
        static constexpr sal_Unicode aSeps[] = { '\t', ';', ',', ' ' };
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
    rtl_TextEncoding eOldCharSet = meCharSet;
    meCharSet = mxLbCharSet->GetSelectTextEncoding();
    mbCharSetDetect = (meCharSet == RTL_TEXTENCODING_USER_DETECTED);
    mbCharSetSystem = (meCharSet == RTL_TEXTENCODING_DONTKNOW);
    if (mbCharSetDetect)
    {
        meCharSet = meDetectedCharSet;
        mxFtDetectedCharSet->set_label(SvxTextEncodingTable::GetTextString(meCharSet));
    }
    else if( mbCharSetSystem )
    {
        meCharSet = osl_getThreadTextEncoding();
        mxFtDetectedCharSet->set_label(SvxTextEncodingTable::GetTextString(meCharSet));
    }
    else
        mxFtDetectedCharSet->set_label(SvxTextEncodingTable::GetTextString(meCharSet));

    if (eOldCharSet != meCharSet)
        DetectCsvSeparators();

    RbSepFix();
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

OUString ScImportAsciiDlg::GetActiveSeparators() const
{
    if (mxRbSeparated->get_active())
        return GetSeparators();

    if (mxRbDetectSep->get_active())
        return maDetectedFieldSeps;

    return OUString();
}

void ScImportAsciiDlg::SetupSeparatorCtrls()
{
    bool bEnable = mxRbSeparated->get_active();
    mxCkbTab->set_sensitive( bEnable );
    mxCkbSemicolon->set_sensitive( bEnable );
    mxCkbComma->set_sensitive( bEnable );
    mxCkbSpace->set_sensitive( bEnable );
    mxCkbOther->set_sensitive( bEnable );
    mxEdOther->set_sensitive( bEnable );

    bEnable = bEnable || mxRbDetectSep->get_active();
    mxCkbRemoveSpace->set_sensitive( bEnable );
    mxCkbAsOnce->set_sensitive( bEnable );
    mxFtTextSep->set_sensitive( bEnable );
    mxCbTextSep->set_sensitive( bEnable );

    OUString aSepName;
    if (maDetectedFieldSeps.isEmpty())
        aSepName += ScResId(SCSTR_NONE);
    else
    {
        for (int idx = 0; idx < maDetectedFieldSeps.getLength(); idx ++)
        {
            if (idx > 0)
                aSepName += u" ";

            if (maDetectedFieldSeps[idx] == u' ')
                aSepName += ScResId(SCSTR_FIELDSEP_SPACE);
            else if (maDetectedFieldSeps[idx] == u'\t')
                aSepName += ScResId(SCSTR_FIELDSEP_TAB);
            else
                aSepName += OUStringChar(maDetectedFieldSeps[idx]);
        }
    }
    mxRbDetectSep->set_label(ScResId(SCSTR_DETECTED).replaceFirst( "%1", aSepName));
}

void ScImportAsciiDlg::DetectCsvSeparators()
{
    mpDatStream->Seek(mnStreamInitPos);
    SfxObjectShell::DetectCsvSeparators(*mpDatStream, meCharSet, maDetectedFieldSeps, mcTextSep);
    mpDatStream->Seek(mnStreamPos);
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
    if (mxRbSeparated->get_active() || mxRbDetectSep->get_active())
    {
        maFieldSeparators = GetActiveSeparators();
        if (mxTableBox->IsFixedWidthMode())
            mxTableBox->SetSeparatorsMode();
        else
            mxTableBox->Refresh();
    }
    else
        mxTableBox->SetFixedWidthMode();

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
    sal_Unicode cOldSep = mcTextSep;
    mcTextSep = lcl_CharFromCombo( *mxCbTextSep, SCSTR_TEXTSEP );
    // Any separator changed may result in completely different lines due to
    // embedded line breaks.
    if (cOldSep != mcTextSep)
    {
        DetectCsvSeparators();

        SetupSeparatorCtrls();

        maFieldSeparators = GetActiveSeparators();
        if (aOldFldSeps != maFieldSeparators)
        {
            UpdateVertical();
            mxTableBox->Refresh();
            return;
        }
    }
    else
        maFieldSeparators = GetActiveSeparators();

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
    sal_Unicode cDetectSep = 0xffff;

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

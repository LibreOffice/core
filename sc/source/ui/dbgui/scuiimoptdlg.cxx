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

#include <scuiimoptdlg.hxx>
#include <scresid.hxx>
#include <strings.hrc>
#include <strings.hxx>
#include <officecfg/Office/Calc.hxx>
#include <osl/thread.h>
#include <rtl/tencinfo.h>
#include <imoptdlg.hxx>
#include <svx/txencbox.hxx>

// ScDelimiterTable

class ScDelimiterTable
{
public:
    explicit ScDelimiterTable( const OUString& rDelTab )
            :   theDelTab ( rDelTab ),
                nDelIdx   ( 0 )
            {}

    sal_uInt16  GetCode( const OUString& rDelimiter ) const;
    OUString  GetDelimiter( sal_Unicode nCode ) const;

    OUString  FirstDel()  { nDelIdx = 0; return theDelTab.getToken( 0, cSep, nDelIdx ); }
    OUString  NextDel()   { return theDelTab.getToken( 1, cSep, nDelIdx ); }

private:
    const OUString      theDelTab;
    static constexpr sal_Unicode cSep {'\t'};
    sal_Int32           nDelIdx;
};

sal_uInt16 ScDelimiterTable::GetCode( const OUString& rDel ) const
{
    if (!theDelTab.isEmpty())
    {
        sal_Int32 nIdx {0};

        // Check even tokens: start from 0 and then skip 1 token at each iteration
        if (rDel != theDelTab.getToken( 0, cSep, nIdx ))
            while (nIdx>0 && rDel != theDelTab.getToken( 1, cSep, nIdx ));

        if (nIdx>0)
            return static_cast<sal_Unicode>(theDelTab.getToken( 0, cSep, nIdx ).toInt32());
    }

    return 0;
}

OUString ScDelimiterTable::GetDelimiter( sal_Unicode nCode ) const
{
    if (!theDelTab.isEmpty())
    {
        sal_Int32 nIdx {0};
        // Check odd tokens: start from 1 and then skip 1 token at each iteration
        do
        {
            sal_Int32 nPrevIdx {nIdx};
            if (nCode == static_cast<sal_Unicode>(theDelTab.getToken( 1, cSep, nIdx ).toInt32()))
                return theDelTab.getToken( 0, cSep, nPrevIdx );
        }
        while (nIdx>0);
    }

    return OUString();
}

void ScImportOptionsDlg::FillFromTextEncodingTable(bool bExcludeImportSubsets, sal_uInt32 nExcludeInfoFlags)
{
    if (m_bIsAsciiImport)
        m_xLbCharset->FillFromTextEncodingTable(bExcludeImportSubsets, nExcludeInfoFlags);
    else
        m_xTvCharset->FillFromTextEncodingTable(bExcludeImportSubsets, nExcludeInfoFlags);
}

void ScImportOptionsDlg::FillFromDbTextEncodingMap(bool bExcludeImportSubsets, sal_uInt32 nExcludeInfoFlags)
{
    if (m_bIsAsciiImport)
        m_xLbCharset->FillFromDbTextEncodingMap(bExcludeImportSubsets, nExcludeInfoFlags);
    else
        m_xTvCharset->FillFromDbTextEncodingMap(bExcludeImportSubsets, nExcludeInfoFlags);
}

// ScImportOptionsDlg
ScImportOptionsDlg::ScImportOptionsDlg(weld::Window* pParent, bool bAscii,
                                       const ScImportOptions*  pOptions,
                                       const OUString* pStrTitle,
                                       bool bMultiByte, bool bOnlyDbtoolsEncodings,
                                       bool bImport)
    : GenericDialogController(pParent, "modules/scalc/ui/imoptdialog.ui", "ImOptDialog")
    , m_bIsAsciiImport(bAscii)
    , m_xFieldFrame(m_xBuilder->weld_frame("fieldframe"))
    , m_xFtCharset(m_xBuilder->weld_label("charsetft"))
    , m_xEncGrid(m_xBuilder->weld_widget("grid2"))
    , m_xFtFieldSep(m_xBuilder->weld_label("fieldft"))
    , m_xEdFieldSep(m_xBuilder->weld_combo_box("field"))
    , m_xFtTextSep(m_xBuilder->weld_label("textft"))
    , m_xEdTextSep(m_xBuilder->weld_combo_box("text"))
    , m_xCbShown(m_xBuilder->weld_check_button("asshown"))
    , m_xCbFormulas(m_xBuilder->weld_check_button("formulas"))
    , m_xCbQuoteAll(m_xBuilder->weld_check_button("quoteall"))
    , m_xCbFixed(m_xBuilder->weld_check_button("fixedwidth"))
    , m_xBtnOk(m_xBuilder->weld_button("ok"))
    , m_xLbCharset(new TextEncodingBox(m_xBuilder->weld_combo_box("charsetdropdown")))
    , m_xTvCharset(new TextEncodingTreeView(m_xBuilder->weld_tree_view("charsetlist")))
{
    if (bAscii)
    {
        m_xDialog->set_help_id(m_xDialog->get_help_id() + "?config=NonTextImport");
        m_xLbCharset->show();
    }
    else
    {
        m_xTvCharset->set_size_request(-1, m_xTvCharset->get_height_rows(6));
        m_xEncGrid->set_vexpand(true);
        m_xTvCharset->show();
    }

    OUString sFieldSep(SCSTR_FIELDSEP);
    sFieldSep = sFieldSep.replaceFirst( "%TAB",   ScResId(SCSTR_FIELDSEP_TAB) );
    sFieldSep = sFieldSep.replaceFirst( "%SPACE", ScResId(SCSTR_FIELDSEP_SPACE) );

    // not possible in the Ctor initializer (MSC cannot do that):
    pFieldSepTab.reset( new ScDelimiterTable(sFieldSep) );
    pTextSepTab.reset( new ScDelimiterTable(SCSTR_TEXTSEP) );

    OUString aStr = pFieldSepTab->FirstDel();
    sal_Unicode nCode;

    while (!aStr.isEmpty())
    {
        m_xEdFieldSep->append_text(aStr);
        aStr = pFieldSepTab->NextDel();
    }

    aStr = pTextSepTab->FirstDel();

    while (!aStr.isEmpty())
    {
        m_xEdTextSep->append_text(aStr);
        aStr = pTextSepTab->NextDel();
    }

    m_xEdFieldSep->set_active(0);
    m_xEdTextSep->set_active(0);

    if ( bOnlyDbtoolsEncodings )
    {
        // Even dBase export allows multibyte now
        if ( bMultiByte )
            FillFromDbTextEncodingMap( bImport );
        else
            FillFromDbTextEncodingMap( bImport, RTL_TEXTENCODING_INFO_MULTIBYTE );
    }
    else if ( !bAscii )
    {   //!TODO: Unicode would need work in each filter
        if ( bMultiByte )
            FillFromTextEncodingTable( bImport, RTL_TEXTENCODING_INFO_UNICODE );
        else
            FillFromTextEncodingTable( bImport, RTL_TEXTENCODING_INFO_UNICODE |
                RTL_TEXTENCODING_INFO_MULTIBYTE );
    }
    else
    {
        if ( pOptions )
        {
            nCode = pOptions->nFieldSepCode;
            aStr  = pFieldSepTab->GetDelimiter( nCode );

            if ( aStr.isEmpty() )
                m_xEdFieldSep->set_entry_text(OUString(nCode));
            else
                m_xEdFieldSep->set_entry_text(aStr);

            nCode = pOptions->nTextSepCode;
            aStr  = pTextSepTab->GetDelimiter( nCode );

            if ( aStr.isEmpty() )
                m_xEdTextSep->set_entry_text(OUString(nCode));
            else
                m_xEdTextSep->set_entry_text(aStr);
        }
        // all encodings allowed, even Unicode
        FillFromTextEncodingTable( bImport );
    }

    if( bAscii )
    {
        sal_Int32 nCharSet = officecfg::Office::Calc::Dialogs::CSVExport::CharSet::get();
        OUString strFieldSeparator = officecfg::Office::Calc::Dialogs::CSVExport::FieldSeparator::get();
        OUString strTextSeparator = officecfg::Office::Calc::Dialogs::CSVExport::TextSeparator::get();
        bool bSaveTrueCellContent = officecfg::Office::Calc::Dialogs::CSVExport::SaveTrueCellContent::get();
        bool bSaveCellFormulas = officecfg::Office::Calc::Dialogs::CSVExport::SaveCellFormulas::get();
        bool bQuoteAllTextCells = officecfg::Office::Calc::Dialogs::CSVExport::QuoteAllTextCells::get();
        bool bFixedWidth = officecfg::Office::Calc::Dialogs::CSVExport::FixedWidth::get();

        m_xCbFixed->show();
        m_xCbFixed->connect_toggled(LINK(this, ScImportOptionsDlg, FixedWidthHdl));
        m_xCbFixed->set_active( bFixedWidth );
        FixedWidthHdl(*m_xCbFixed);
        m_xCbShown->show();
        m_xCbShown->set_active( bSaveTrueCellContent );
        m_xCbQuoteAll->show();
        m_xCbQuoteAll->set_active( bQuoteAllTextCells );
        m_xCbFormulas->show();
        // default option for "save formulas" no longer taken from view shell but from persisted dialog settings
        m_xCbFormulas->set_active( bSaveCellFormulas );
        // if no charset, text separator or field separator exist, keep the values from dialog initialization
        if (strFieldSeparator.getLength() > 0)
            m_xEdFieldSep->set_entry_text(strFieldSeparator);
        if (strTextSeparator.getLength() > 0)
            m_xEdTextSep->set_entry_text(strTextSeparator);
        if (nCharSet < 0 || nCharSet == RTL_TEXTENCODING_DONTKNOW )
            m_xLbCharset->SelectTextEncoding(pOptions ? pOptions->eCharSet : osl_getThreadTextEncoding());
        else
            m_xLbCharset->SelectTextEncoding(nCharSet);
    }
    else
    {
        m_xFieldFrame->set_label(m_xFtCharset->get_label());
        m_xFtFieldSep->hide();
        m_xFtTextSep->hide();
        m_xFtCharset->hide();
        m_xEdFieldSep->hide();
        m_xEdTextSep->hide();
        m_xCbFixed->hide();
        m_xCbShown->hide();
        m_xCbQuoteAll->hide();
        m_xCbFormulas->hide();
        m_xTvCharset->grab_focus();
        m_xTvCharset->connect_row_activated(LINK(this, ScImportOptionsDlg, DoubleClickHdl));
        m_xTvCharset->SelectTextEncoding(pOptions ? pOptions->eCharSet : osl_getThreadTextEncoding());
    }

    // optional title:
    if (pStrTitle)
        m_xDialog->set_title(*pStrTitle);
}

ScImportOptionsDlg::~ScImportOptionsDlg()
{
}

void ScImportOptionsDlg::GetImportOptions( ScImportOptions& rOptions ) const
{
    auto nEncoding = m_bIsAsciiImport ? m_xLbCharset->GetSelectTextEncoding() : m_xTvCharset->GetSelectTextEncoding();
    rOptions.SetTextEncoding(nEncoding);

    if (m_xCbFixed->get_visible())
    {
        rOptions.nFieldSepCode = GetCodeFromCombo( *m_xEdFieldSep );
        rOptions.nTextSepCode  = GetCodeFromCombo( *m_xEdTextSep );
        rOptions.bFixedWidth = m_xCbFixed->get_active();
        rOptions.bSaveAsShown = m_xCbShown->get_active();
        rOptions.bQuoteAllText = m_xCbQuoteAll->get_active();
        rOptions.bSaveFormulas = m_xCbFormulas->get_active();
    }
}

sal_uInt16 ScImportOptionsDlg::GetCodeFromCombo(const weld::ComboBox& rEd) const
{
    ScDelimiterTable* pTab;
    OUString  aStr( rEd.get_active_text() );
    sal_uInt16  nCode;

    if (&rEd == m_xEdTextSep.get())
        pTab = pTextSepTab.get();
    else
        pTab = pFieldSepTab.get();

    if ( aStr.isEmpty() )
    {
        nCode = 0;          // no separator
    }
    else
    {
        nCode = pTab->GetCode( aStr );

        if ( nCode == 0 )
            nCode = static_cast<sal_uInt16>(aStr[0]);
    }

    return nCode;
}

IMPL_LINK_NOARG(ScImportOptionsDlg, FixedWidthHdl, weld::ToggleButton&, void)
{
    bool bEnable = !m_xCbFixed->get_active();
    m_xFtFieldSep->set_sensitive( bEnable );
    m_xEdFieldSep->set_sensitive( bEnable );
    m_xFtTextSep->set_sensitive( bEnable );
    m_xEdTextSep->set_sensitive( bEnable );
    m_xCbShown->set_sensitive( bEnable );
    m_xCbQuoteAll->set_sensitive( bEnable );
}

IMPL_LINK_NOARG(ScImportOptionsDlg, DoubleClickHdl, weld::TreeView&, void)
{
    m_xDialog->response(RET_OK);
}

void ScImportOptionsDlg::SaveImportOptions() const
{
    std::shared_ptr < comphelper::ConfigurationChanges > batch(comphelper::ConfigurationChanges::create());
    auto nEncoding = m_bIsAsciiImport ? m_xLbCharset->GetSelectTextEncoding() : m_xTvCharset->GetSelectTextEncoding();
    officecfg::Office::Calc::Dialogs::CSVExport::CharSet::set(nEncoding, batch);
    officecfg::Office::Calc::Dialogs::CSVExport::FieldSeparator::set(m_xEdFieldSep->get_active_text(), batch);
    officecfg::Office::Calc::Dialogs::CSVExport::TextSeparator::set(m_xEdTextSep->get_active_text(), batch);
    officecfg::Office::Calc::Dialogs::CSVExport::FixedWidth::set(m_xCbFixed->get_active(), batch);
    officecfg::Office::Calc::Dialogs::CSVExport::SaveCellFormulas::set(m_xCbFormulas->get_active(), batch);
    officecfg::Office::Calc::Dialogs::CSVExport::SaveTrueCellContent::set(m_xCbShown->get_active(), batch);
    officecfg::Office::Calc::Dialogs::CSVExport::QuoteAllTextCells::set(m_xCbQuoteAll->get_active(), batch);
    batch->commit();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

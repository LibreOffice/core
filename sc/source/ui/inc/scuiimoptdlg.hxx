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

#ifndef INCLUDED_SC_SOURCE_UI_INC_SCUIIMOPTDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_SCUIIMOPTDLG_HXX

#include <vcl/weld.hxx>

class ScDelimiterTable;
class ScImportOptions;
class TextEncodingBox;
class TextEncodingTreeView;

class ScImportOptionsDlg : public weld::GenericDialogController
{
public:
    ScImportOptionsDlg(weld::Window*           pParent,
                       bool                    bAscii,
                       const ScImportOptions*  pOptions,
                       const OUString*         pStrTitle,
                       bool                    bMultiByte,
                       bool                    bOnlyDbtoolsEncodings,
                       bool                    bImport);

    virtual ~ScImportOptionsDlg() override;

    void GetImportOptions( ScImportOptions& rOptions ) const;
    void SaveImportOptions() const;

private:
    std::unique_ptr<ScDelimiterTable> pFieldSepTab;
    std::unique_ptr<ScDelimiterTable> pTextSepTab;

    bool const m_bIsAsciiImport;

    std::unique_ptr<weld::Frame> m_xFieldFrame;
    std::unique_ptr<weld::Label> m_xFtCharset;
    std::unique_ptr<weld::Widget> m_xEncGrid;
    std::unique_ptr<weld::Label> m_xFtFieldSep;
    std::unique_ptr<weld::ComboBox> m_xEdFieldSep;
    std::unique_ptr<weld::Label> m_xFtTextSep;
    std::unique_ptr<weld::ComboBox> m_xEdTextSep;
    std::unique_ptr<weld::CheckButton> m_xCbShown;
    std::unique_ptr<weld::CheckButton> m_xCbFormulas;
    std::unique_ptr<weld::CheckButton> m_xCbQuoteAll;
    std::unique_ptr<weld::CheckButton> m_xCbFixed;
    std::unique_ptr<weld::Button> m_xBtnOk;
    std::unique_ptr<TextEncodingBox> m_xLbCharset;
    std::unique_ptr<TextEncodingTreeView> m_xTvCharset;

private:
    sal_uInt16 GetCodeFromCombo( const weld::ComboBox& rEd ) const;
    void FillFromTextEncodingTable(bool bExcludeImportSubsets, sal_uInt32 nExcludeInfoFlags = 0);
    void FillFromDbTextEncodingMap(bool bExcludeImportSubsets, sal_uInt32 nExcludeInfoFlags = 0);

    DECL_LINK(FixedWidthHdl, weld::ToggleButton&, void);
    DECL_LINK(DoubleClickHdl, weld::TreeView&, void);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

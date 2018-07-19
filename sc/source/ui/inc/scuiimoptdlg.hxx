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

#include <vcl/layout.hxx>
#include "imoptdlg.hxx"

class ScDelimiterTable;

class ScImportOptionsDlg : public ModalDialog
{
public:
    ScImportOptionsDlg( vcl::Window*            pParent,
                        bool                    bAscii,
                        const ScImportOptions*  pOptions,
                        const OUString*         pStrTitle,
                        bool                    bMultiByte,
                        bool                    bOnlyDbtoolsEncodings,
                        bool                    bImport );

    virtual ~ScImportOptionsDlg() override;
    virtual void dispose() override;

    void GetImportOptions( ScImportOptions& rOptions ) const;
    void SaveImportOptions() const;
    virtual OString GetScreenshotId() const override;

private:
    VclPtr<VclFrame>           m_pFieldFrame;
    VclPtr<FixedText>          m_pFtCharset;
    VclPtr<VclContainer>       m_pEncGrid;
    VclPtr<SvxTextEncodingBox> m_pLbCharset;
    VclPtr<FixedText>          m_pFtFieldSep;
    VclPtr<ComboBox>           m_pEdFieldSep;
    VclPtr<FixedText>          m_pFtTextSep;
    VclPtr<ComboBox>           m_pEdTextSep;
    VclPtr<CheckBox>           m_pCbShown;
    VclPtr<CheckBox>           m_pCbFormulas;
    VclPtr<CheckBox>           m_pCbQuoteAll;
    VclPtr<CheckBox>           m_pCbFixed;
    VclPtr<OKButton>           m_pBtnOk;


    std::unique_ptr<ScDelimiterTable> pFieldSepTab;
    std::unique_ptr<ScDelimiterTable> pTextSepTab;

    bool m_bIsAsciiImport;

private:
    sal_uInt16 GetCodeFromCombo( const ComboBox& rEd ) const;

    DECL_LINK( FixedWidthHdl, Button*, void );
    DECL_LINK( DoubleClickHdl, ListBox&, void );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

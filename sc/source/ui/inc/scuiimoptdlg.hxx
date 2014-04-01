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

#ifndef SCUI_IMOPTDLG_HXX
#define SCUI_IMOPTDLG_HXX

#include <vcl/layout.hxx>
#include "imoptdlg.hxx"




class ScDelimiterTable;

class ScImportOptionsDlg : public ModalDialog
{
public:
    ScImportOptionsDlg( Window*                 pParent,
                        bool                    bAscii = true,
                        const ScImportOptions*  pOptions = NULL,
                        const OUString*         pStrTitle = NULL,
                        bool                    bMultiByte = false,
                        bool                    bOnlyDbtoolsEncodings = false,
                        bool                    bImport = true );

    virtual ~ScImportOptionsDlg();

    void GetImportOptions( ScImportOptions& rOptions ) const;

private:
    VclFrame*           m_pFieldFrame;
    FixedText*          m_pFtCharset;
    SvxTextEncodingBox* m_pLbCharset;
    FixedText*          m_pFtFieldSep;
    ComboBox*           m_pEdFieldSep;
    FixedText*          m_pFtTextSep;
    ComboBox*           m_pEdTextSep;
    CheckBox*           m_pCbShown;
    CheckBox*           m_pCbFormulas;
    CheckBox*           m_pCbQuoteAll;
    CheckBox*           m_pCbFixed;
    OKButton*           m_pBtnOk;

    ScDelimiterTable*   pFieldSepTab;
    ScDelimiterTable*   pTextSepTab;

private:
    sal_uInt16 GetCodeFromCombo( const ComboBox& rEd ) const;

    DECL_LINK( FixedWidthHdl, CheckBox* );
    DECL_LINK( DoubleClickHdl, ListBox* );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

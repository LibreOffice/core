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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_ASCFLDLG_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_ASCFLDLG_HXX
#include <vcl/fixed.hxx>

#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <sfx2/basedlgs.hxx>
#include <svx/txencbox.hxx>
#include <svx/langbox.hxx>

class SwAsciiOptions;
class SvStream;
class SwDocShell;

class SwAsciiFilterDlg : public SfxModalDialog
{
    VclPtr<SvxTextEncodingBox> m_pCharSetLB;
    VclPtr<FixedText>          m_pFontFT;
    VclPtr<ListBox>            m_pFontLB;
    VclPtr<FixedText>          m_pLanguageFT;
    VclPtr<SvxLanguageBox>     m_pLanguageLB;
    VclPtr<RadioButton>        m_pCRLF_RB;
    VclPtr<RadioButton>        m_pCR_RB;
    VclPtr<RadioButton>        m_pLF_RB;
    bool                m_bSaveLineStatus;

    DECL_LINK_TYPED( CharSetSelHdl, ListBox&, void );
    DECL_LINK_TYPED( LineEndHdl, RadioButton&, void );
    void SetCRLF( LineEnd eEnd );
    LineEnd GetCRLF() const;

public:
    // CTOR:    for import - pStream is the inputstream
    //          for export - pStream must be 0
    SwAsciiFilterDlg( vcl::Window* pParent, SwDocShell& rDocSh,
                        SvStream* pStream );
    virtual ~SwAsciiFilterDlg();
    virtual void dispose() override;

    void FillOptions( SwAsciiOptions& rOptions );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

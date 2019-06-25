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

#include <sfx2/basedlgs.hxx>
#include <svx/txencbox.hxx>
#include <svx/langbox.hxx>
#include <tools/lineend.hxx>

class SwAsciiOptions;
class SvStream;
class SwDocShell;

class SwAsciiFilterDlg : public SfxDialogController
{
    bool m_bSaveLineStatus;
    OUString m_sExtraData;

    std::unique_ptr<TextEncodingBox> m_xCharSetLB;
    std::unique_ptr<weld::Label> m_xFontFT;
    std::unique_ptr<weld::ComboBox> m_xFontLB;
    std::unique_ptr<weld::Label> m_xLanguageFT;
    std::unique_ptr<LanguageBox>     m_xLanguageLB;
    std::unique_ptr<weld::RadioButton> m_xCRLF_RB;
    std::unique_ptr<weld::RadioButton> m_xCR_RB;
    std::unique_ptr<weld::RadioButton> m_xLF_RB;
    std::unique_ptr<weld::CheckButton> m_xIncludeBOM_CB;

    DECL_LINK(CharSetSelHdl, weld::ComboBox&, void);
    DECL_LINK(LineEndHdl, weld::ToggleButton&, void);
    void SetCRLF( LineEnd eEnd );
    LineEnd GetCRLF() const;
    void SetIncludeBOM( bool bIncludeBOM );
    bool GetIncludeBOM() const;
    void UpdateIncludeBOMSensitiveState();

public:
    // CTOR:    for import - pStream is the inputstream
    //          for export - pStream must be 0
    SwAsciiFilterDlg(weld::Window* pParent, SwDocShell& rDocSh, SvStream* pStream);
    virtual ~SwAsciiFilterDlg() override;

    void FillOptions( SwAsciiOptions& rOptions );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

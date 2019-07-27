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
#ifndef INCLUDED_CUI_SOURCE_OPTIONS_OPTSAVE_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_OPTSAVE_HXX

#include <memory>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <sfx2/tabdlg.hxx>

#define APP_WRITER              0
#define APP_WRITER_WEB          1
#define APP_WRITER_GLOBAL       2
#define APP_CALC                3
#define APP_IMPRESS             4
#define APP_DRAW                5
#define APP_MATH                6
#define APP_COUNT               7

namespace com { namespace sun { namespace star {
  namespace beans {
    struct PropertyValue;
}}}}

// class SvxSaveTabPage --------------------------------------------------

struct SvxSaveTabPage_Impl;

class SvxSaveTabPage : public SfxTabPage
{
private:
    std::unique_ptr<SvxSaveTabPage_Impl>    pImpl;

    std::unique_ptr<weld::CheckButton> m_xLoadUserSettingsCB;
    std::unique_ptr<weld::CheckButton> m_xLoadDocPrinterCB;
    std::unique_ptr<weld::CheckButton> m_xDocInfoCB;
    std::unique_ptr<weld::CheckButton> m_xBackupCB;
    std::unique_ptr<weld::CheckButton> m_xAutoSaveCB;
    std::unique_ptr<weld::SpinButton> m_xAutoSaveEdit;
    std::unique_ptr<weld::Label> m_xMinuteFT;
    std::unique_ptr<weld::CheckButton> m_xUserAutoSaveCB;
    std::unique_ptr<weld::CheckButton> m_xRelativeFsysCB;
    std::unique_ptr<weld::CheckButton> m_xRelativeInetCB;
    std::unique_ptr<weld::ComboBox> m_xODFVersionLB;
    std::unique_ptr<weld::CheckButton> m_xWarnAlienFormatCB;
    std::unique_ptr<weld::ComboBox> m_xDocTypeLB;
    std::unique_ptr<weld::Label> m_xSaveAsFT;
    std::unique_ptr<weld::ComboBox> m_xSaveAsLB;
    std::unique_ptr<weld::Widget> m_xODFWarningFI;
    std::unique_ptr<weld::Label> m_xODFWarningFT;

    DECL_LINK( AutoClickHdl_Impl, weld::Button&, void );
    DECL_LINK( FilterHdl_Impl, weld::ComboBox&, void );
    DECL_LINK(ODFVersionHdl_Impl, weld::ComboBox&, void );

    void    DetectHiddenControls();

public:
    SvxSaveTabPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~SvxSaveTabPage() override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent, const SfxItemSet* rAttrSet );

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

#endif // INCLUDED_CUI_SOURCE_OPTIONS_OPTSAVE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

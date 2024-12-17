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
#pragma once

#include <memory>
#include <sfx2/tabdlg.hxx>

enum AppType
{
    APP_WRITER            =  0,
    APP_WRITER_WEB        =  1,
    APP_WRITER_GLOBAL     =  2,
    APP_CALC              =  3,
    APP_IMPRESS           =  4,
    APP_DRAW              =  5,
    APP_MATH              =  6,
    APP_COUNT             =  7,
};

namespace com::sun::star::beans { struct PropertyValue; }

// class SvxSaveTabPage --------------------------------------------------

struct SvxSaveTabPage_Impl;

class SvxSaveTabPage : public SfxTabPage
{
private:
    std::unique_ptr<SvxSaveTabPage_Impl>    pImpl;

    std::unique_ptr<weld::CheckButton> m_xLoadViewPosAnyUserCB;
    std::unique_ptr<weld::Widget> m_xLoadViewPosAnyUserImg;
    std::unique_ptr<weld::CheckButton> m_xLoadUserSettingsCB;
    std::unique_ptr<weld::Widget> m_xLoadUserSettingsImg;
    std::unique_ptr<weld::CheckButton> m_xLoadDocPrinterCB;
    std::unique_ptr<weld::Widget> m_xLoadDocPrinterImg;
    std::unique_ptr<weld::CheckButton> m_xDocInfoCB;
    std::unique_ptr<weld::Widget> m_xDocInfoImg;
    std::unique_ptr<weld::CheckButton> m_xBackupCB;
    std::unique_ptr<weld::Widget> m_xBackupImg;
    std::unique_ptr<weld::CheckButton> m_xBackupIntoDocumentFolderCB;
    std::unique_ptr<weld::Widget> m_xBackupIntoDocumentFolderImg;
    std::unique_ptr<weld::CheckButton> m_xAutoSaveCB;
    std::unique_ptr<weld::Widget> m_xAutoSaveImg;
    std::unique_ptr<weld::SpinButton> m_xAutoSaveEdit;
    std::unique_ptr<weld::Label> m_xMinuteFT;
    std::unique_ptr<weld::CheckButton> m_xUserAutoSaveCB;
    std::unique_ptr<weld::Widget> m_xUserAutoSaveImg;
    std::unique_ptr<weld::CheckButton> m_xRelativeFsysCB;
    std::unique_ptr<weld::Widget> m_xRelativeFsysImg;
    std::unique_ptr<weld::CheckButton> m_xRelativeInetCB;
    std::unique_ptr<weld::Widget> m_xRelativeInetImg;
    std::unique_ptr<weld::ComboBox> m_xODFVersionLB;
    std::unique_ptr<weld::Label> m_xODFVersionFT;
    std::unique_ptr<weld::Widget> m_xODFVersionImg;
    std::unique_ptr<weld::CheckButton> m_xWarnAlienFormatCB;
    std::unique_ptr<weld::Widget> m_xWarnAlienFormatImg;
    std::unique_ptr<weld::ComboBox> m_xDocTypeLB;
    std::unique_ptr<weld::Label> m_xSaveAsFT;
    std::unique_ptr<weld::ComboBox> m_xSaveAsLB;
    std::unique_ptr<weld::Widget> m_xODFWarningFI;
    std::unique_ptr<weld::Label> m_xODFWarningFT;

    DECL_LINK( AutoClickHdl_Impl, weld::Toggleable&, void );
    DECL_LINK(BackupClickHdl_Impl, weld::Toggleable&, void);
    DECL_LINK( FilterHdl_Impl, weld::ComboBox&, void );
    DECL_LINK(ODFVersionHdl_Impl, weld::ComboBox&, void );

public:
    SvxSaveTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~SvxSaveTabPage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet );

    virtual OUString GetAllStrings() override;

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

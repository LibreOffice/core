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
#ifndef INCLUDED_CUI_SOURCE_OPTIONS_OPTINET2_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_OPTINET2_HXX

#include <memory>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <sfx2/tabdlg.hxx>

namespace svx {
    class SecurityOptionsDialog;
}

// class SvxProxyTabPage -------------------------------------------------
class SvxProxyTabPage : public SfxTabPage
{
private:

    std::unique_ptr<weld::ComboBox> m_xProxyModeLB;

    std::unique_ptr<weld::Label> m_xHttpProxyFT;
    std::unique_ptr<weld::Entry> m_xHttpProxyED;
    std::unique_ptr<weld::Label> m_xHttpPortFT;
    std::unique_ptr<weld::Entry> m_xHttpPortED;

    std::unique_ptr<weld::Label> m_xHttpsProxyFT;
    std::unique_ptr<weld::Entry> m_xHttpsProxyED;
    std::unique_ptr<weld::Label> m_xHttpsPortFT;
    std::unique_ptr<weld::Entry> m_xHttpsPortED;

    std::unique_ptr<weld::Label> m_xFtpProxyFT;
    std::unique_ptr<weld::Entry> m_xFtpProxyED;
    std::unique_ptr<weld::Label> m_xFtpPortFT;
    std::unique_ptr<weld::Entry> m_xFtpPortED;

    std::unique_ptr<weld::Label> m_xNoProxyForFT;
    std::unique_ptr<weld::Entry> m_xNoProxyForED;
    std::unique_ptr<weld::Label> m_xNoProxyDescFT;

    css::uno::Reference< css::uno::XInterface > m_xConfigurationUpdateAccess;

    void EnableControls_Impl();
    void ReadConfigData_Impl();
    void ReadConfigDefaults_Impl();
    void RestoreConfigDefaults_Impl();

    DECL_LINK(PortChangedHdl, weld::Entry&, void);
    DECL_STATIC_LINK(SvxProxyTabPage, NumberOnlyTextFilterHdl, OUString&, bool);
    DECL_STATIC_LINK(SvxProxyTabPage, NoSpaceTextFilterHdl, OUString&, bool);
    DECL_LINK(ProxyHdl_Impl, weld::ComboBox&, void);
    DECL_STATIC_LINK(SvxProxyTabPage, LoseFocusHdl_Impl, weld::Widget&, void);

public:
    SvxProxyTabPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~SvxProxyTabPage() override;
    static VclPtr<SfxTabPage>  Create( TabPageParent pParent, const SfxItemSet* rAttrSet );
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

// class SvxSecurityTabPage ---------------------------------------------

class SvtSecurityOptions;
class CertPathDialog;
class SvxSecurityTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
    friend class VclPtr<SvxSecurityTabPage>;
private:
    std::unique_ptr<SvtSecurityOptions>         mpSecOptions;
    std::unique_ptr<svx::SecurityOptionsDialog> m_xSecOptDlg;

    std::unique_ptr<CertPathDialog> mpCertPathDlg;

    OUString m_sPasswordStoringDeactivateStr;

    std::unique_ptr<weld::Button> m_xSecurityOptionsPB;

    std::unique_ptr<weld::CheckButton> m_xSavePasswordsCB;
    std::unique_ptr<weld::Button> m_xShowConnectionsPB;

    std::unique_ptr<weld::CheckButton> m_xMasterPasswordCB;
    std::unique_ptr<weld::Label> m_xMasterPasswordFT;
    std::unique_ptr<weld::Button> m_xMasterPasswordPB;

    std::unique_ptr<weld::Container> m_xMacroSecFrame;
    std::unique_ptr<weld::Button> m_xMacroSecPB;

    std::unique_ptr<weld::Container> m_xCertFrame;
    std::unique_ptr<weld::Button> m_xCertPathPB;

    std::unique_ptr<weld::Container> m_xTSAURLsFrame;
    std::unique_ptr<weld::Button> m_xTSAURLsPB;

    std::unique_ptr<weld::Label> m_xNoPasswordSaveFT;

    DECL_LINK(SecurityOptionsHdl, weld::Button&, void);
    DECL_LINK(SavePasswordHdl, weld::Button&, void);
    DECL_LINK(MasterPasswordHdl, weld::Button&, void);
    DECL_LINK(MasterPasswordCBHdl, weld::Button&, void);
    DECL_LINK(ShowPasswordsHdl, weld::Button&, void);
    DECL_LINK(MacroSecPBHdl, weld::Button&, void );
    DECL_LINK(CertPathPBHdl, weld::Button&, void );
    DECL_LINK(TSAURLsPBHdl, weld::Button&, void );

    void                InitControls();

    SvxSecurityTabPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~SvxSecurityTabPage() override;

protected:
    virtual void        ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

public:
    static VclPtr<SfxTabPage>  Create( TabPageParent pParent, const SfxItemSet* rAttrSet );
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

struct SvxEMailTabPage_Impl;
class SvxEMailTabPage : public SfxTabPage
{
    OUString      m_sDefaultFilterName;

    std::unique_ptr<SvxEMailTabPage_Impl> pImpl;

    std::unique_ptr<weld::Container> m_xMailContainer;
    std::unique_ptr<weld::Image> m_xMailerURLFI;
    std::unique_ptr<weld::Entry> m_xMailerURLED;
    std::unique_ptr<weld::Button> m_xMailerURLPB;
    std::unique_ptr<weld::Container> m_xSuppressHiddenContainer;
    std::unique_ptr<weld::Image> m_xSuppressHiddenFI;
    std::unique_ptr<weld::CheckButton> m_xSuppressHidden;
    std::unique_ptr<weld::Label> m_xDefaultFilterFT;

    DECL_LINK(FileDialogHdl_Impl, weld::Button&, void);

public:
    SvxEMailTabPage(TabPageParent pParent, const SfxItemSet& rSet );
    virtual ~SvxEMailTabPage() override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent, const SfxItemSet* rAttrSet );

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

#endif // INCLUDED_CUI_SOURCE_OPTIONS_OPTINET2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

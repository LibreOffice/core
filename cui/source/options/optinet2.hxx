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
    VclPtr<PushButton>         m_pSecurityOptionsPB;

    VclPtr<CheckBox>           m_pSavePasswordsCB;
    VclPtr<PushButton>         m_pShowConnectionsPB;

    VclPtr<CheckBox>           m_pMasterPasswordCB;
    VclPtr<FixedText>          m_pMasterPasswordFT;
    VclPtr<PushButton>         m_pMasterPasswordPB;

    VclPtr<VclContainer>       m_pMacroSecFrame;
    VclPtr<PushButton>         m_pMacroSecPB;

    VclPtr<VclContainer>       m_pCertFrame;
    VclPtr<PushButton>         m_pCertPathPB;

    VclPtr<VclContainer>       m_pTSAURLsFrame;
    VclPtr<PushButton>         m_pTSAURLsPB;

    std::unique_ptr<SvtSecurityOptions>         mpSecOptions;
    std::unique_ptr<svx::SecurityOptionsDialog> m_xSecOptDlg;

    std::unique_ptr<CertPathDialog> mpCertPathDlg;

    OUString            m_sPasswordStoringDeactivateStr;

    DECL_LINK(SecurityOptionsHdl, Button*, void);
    DECL_LINK(SavePasswordHdl, Button*, void);
    DECL_LINK(MasterPasswordHdl, Button*, void);
    DECL_LINK(MasterPasswordCBHdl, Button*, void);
    DECL_LINK(ShowPasswordsHdl, Button*, void);
    DECL_LINK(MacroSecPBHdl, Button*, void );
    DECL_LINK(CertPathPBHdl, Button*, void );
    DECL_LINK(TSAURLsPBHdl, Button*, void );

    void                InitControls();

                SvxSecurityTabPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual     ~SvxSecurityTabPage() override;
    virtual void dispose() override;

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

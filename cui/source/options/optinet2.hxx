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

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <vcl/field.hxx>
#include <vcl/group.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/stdctrl.hxx>
#include <svtools/svtabbx.hxx>
#include <sfx2/tabdlg.hxx>
#include <unotools/configitem.hxx>

#include <svtools/headbar.hxx>

namespace svx {
    class SecurityOptionsDialog;
}


// class SvxNoSpaceEdit --------------------------------------------------

class SvxNoSpaceEdit : public Edit
{
private:
    bool bOnlyNumeric;
public:
    SvxNoSpaceEdit(vcl::Window* pParent, WinBits nStyle)
        : Edit(pParent, nStyle)
        , bOnlyNumeric(false)
    {}
    virtual void KeyInput(const KeyEvent& rKEvent) override;
    virtual void Modify() override;
    virtual bool set_property(const OString &rKey, const OString &rValue) override;
};

// class SvxProxyTabPage -------------------------------------------------

class SvxProxyTabPage : public SfxTabPage
{
private:

    VclPtr<ListBox>        m_pProxyModeLB;

    VclPtr<FixedText>      m_pHttpProxyFT;
    VclPtr<SvxNoSpaceEdit> m_pHttpProxyED;
    VclPtr<FixedText>      m_pHttpPortFT;
    VclPtr<SvxNoSpaceEdit> m_pHttpPortED;

    VclPtr<FixedText>      m_pHttpsProxyFT;
    VclPtr<SvxNoSpaceEdit> m_pHttpsProxyED;
    VclPtr<FixedText>      m_pHttpsPortFT;
    VclPtr<SvxNoSpaceEdit> m_pHttpsPortED;

    VclPtr<FixedText>      m_pFtpProxyFT;
    VclPtr<SvxNoSpaceEdit> m_pFtpProxyED;
    VclPtr<FixedText>      m_pFtpPortFT;
    VclPtr<SvxNoSpaceEdit> m_pFtpPortED;

    VclPtr<FixedText>      m_pNoProxyForFT;
    VclPtr<Edit>           m_pNoProxyForED;
    VclPtr<FixedText>      m_pNoProxyDescFT;

    css::uno::Reference< css::uno::XInterface > m_xConfigurationUpdateAccess;

    void EnableControls_Impl(bool bEnable);
    void ReadConfigData_Impl();
    void ReadConfigDefaults_Impl();
    void RestoreConfigDefaults_Impl();

    DECL_LINK_TYPED( ProxyHdl_Impl, ListBox&, void );
    DECL_STATIC_LINK_TYPED( SvxProxyTabPage, LoseFocusHdl_Impl, Control&, void );

public:
    SvxProxyTabPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SvxProxyTabPage();
    virtual void dispose() override;
    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rAttrSet );
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

    SvtSecurityOptions*         mpSecOptions;
    VclPtr<svx::SecurityOptionsDialog> mpSecOptDlg;

    VclPtr<CertPathDialog> mpCertPathDlg;

    OUString            m_sPasswordStoringDeactivateStr;

    DECL_LINK_TYPED(SecurityOptionsHdl, Button*, void);
    DECL_LINK_TYPED(SavePasswordHdl, Button*, void);
    DECL_STATIC_LINK_TYPED(SvxSecurityTabPage, MasterPasswordHdl, Button*, void);
    DECL_LINK_TYPED(MasterPasswordCBHdl, Button*, void);
    DECL_LINK_TYPED(ShowPasswordsHdl, Button*, void);
    DECL_STATIC_LINK_TYPED(SvxSecurityTabPage, MacroSecPBHdl, Button*, void );
    DECL_LINK_TYPED(CertPathPBHdl, Button*, void );
    DECL_LINK_TYPED(TSAURLsPBHdl, Button*, void );

    void                InitControls();

                SvxSecurityTabPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual     ~SvxSecurityTabPage();
    virtual void dispose() override;

protected:
    virtual void        ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

public:
    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rAttrSet );
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

struct SvxEMailTabPage_Impl;
class SvxEMailTabPage : public SfxTabPage
{
    VclPtr<VclContainer> m_pMailContainer;
    VclPtr<FixedImage>   m_pMailerURLFI;
    VclPtr<Edit>         m_pMailerURLED;
    VclPtr<PushButton>   m_pMailerURLPB;
    VclPtr<VclContainer> m_pSuppressHiddenContainer;
    VclPtr<FixedImage>   m_pSuppressHiddenFI;
    VclPtr<CheckBox>     m_pSuppressHidden;

    OUString      m_sDefaultFilterName;

    std::unique_ptr<SvxEMailTabPage_Impl> pImpl;

    DECL_LINK_TYPED(  FileDialogHdl_Impl, Button*, void );

public:
    SvxEMailTabPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SvxEMailTabPage();
    virtual void        dispose() override;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rAttrSet );

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

#endif // INCLUDED_CUI_SOURCE_OPTIONS_OPTINET2_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

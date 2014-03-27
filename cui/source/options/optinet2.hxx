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

namespace lang = ::com::sun::star::lang;
namespace uno = ::com::sun::star::uno;

// class SvxNoSpaceEdit --------------------------------------------------

class SvxNoSpaceEdit : public Edit
{
private:
    bool bOnlyNumeric;
public:
    SvxNoSpaceEdit(Window* pParent, WinBits nStyle)
        : Edit(pParent, nStyle)
        , bOnlyNumeric(false)
    {}
    virtual void KeyInput(const KeyEvent& rKEvent) SAL_OVERRIDE;
    virtual void Modify() SAL_OVERRIDE;
    virtual bool set_property(const OString &rKey, const OString &rValue) SAL_OVERRIDE;
};

// class SvxProxyTabPage -------------------------------------------------

class SvxProxyTabPage : public SfxTabPage
{
private:

    ListBox*        m_pProxyModeLB;

    FixedText*      m_pHttpProxyFT;
    SvxNoSpaceEdit* m_pHttpProxyED;
    FixedText*      m_pHttpPortFT;
    SvxNoSpaceEdit* m_pHttpPortED;

    FixedText*      m_pHttpsProxyFT;
    SvxNoSpaceEdit* m_pHttpsProxyED;
    FixedText*      m_pHttpsPortFT;
    SvxNoSpaceEdit* m_pHttpsPortED;

    FixedText*      m_pFtpProxyFT;
    SvxNoSpaceEdit* m_pFtpProxyED;
    FixedText*      m_pFtpPortFT;
    SvxNoSpaceEdit* m_pFtpPortED;

    FixedText*      m_pNoProxyForFT;
    Edit*           m_pNoProxyForED;
    FixedText*      m_pNoProxyDescFT;

    const OUString aProxyModePN;
    const OUString aHttpProxyPN;
    const OUString aHttpPortPN;
    const OUString aHttpsProxyPN;
    const OUString aHttpsPortPN;
    const OUString aFtpProxyPN;
    const OUString aFtpPortPN;
    const OUString aNoProxyDescPN;

    uno::Reference< uno::XInterface > m_xConfigurationUpdateAccess;

    void EnableControls_Impl(sal_Bool bEnable);
    void ReadConfigData_Impl();
    void ReadConfigDefaults_Impl();
    void RestoreConfigDefaults_Impl();

    DECL_LINK( ProxyHdl_Impl, ListBox * );
    DECL_LINK( LoseFocusHdl_Impl, Edit * );

    SvxProxyTabPage( Window* pParent, const SfxItemSet& rSet );
    virtual ~SvxProxyTabPage();

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
    virtual bool        FillItemSet( SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet& rSet ) SAL_OVERRIDE;
};

// #98647# class SvxScriptExecListBox ------------------------------------
class SvxScriptExecListBox : public ListBox
{ // for adding tooltips to ListBox
public:
    SvxScriptExecListBox( Window* pParent, WinBits nStyle = WB_BORDER )
            :ListBox(pParent, nStyle) {}
    SvxScriptExecListBox( Window* pParent, const ResId& rResId )
            :ListBox(pParent, rResId) {}

protected:
    virtual void RequestHelp( const HelpEvent& rHEvt ) SAL_OVERRIDE;
};

// class SvxSecurityTabPage ---------------------------------------------

class SvtSecurityOptions;

class CertPathDialog;

class SvxSecurityTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:
    PushButton*         m_pSecurityOptionsPB;

    CheckBox*           m_pSavePasswordsCB;
    PushButton*         m_pShowConnectionsPB;

    CheckBox*           m_pMasterPasswordCB;
    FixedText*          m_pMasterPasswordFT;
    PushButton*         m_pMasterPasswordPB;

    VclContainer*       m_pMacroSecFrame;
    PushButton*         m_pMacroSecPB;

    VclContainer*       m_pCertFrame;
    PushButton*         m_pCertPathPB;

    SvtSecurityOptions*         mpSecOptions;
    svx::SecurityOptionsDialog* mpSecOptDlg;

    CertPathDialog* mpCertPathDlg;

    OUString            m_sPasswordStoringDeactivateStr;

    DECL_LINK(SecurityOptionsHdl, void *);
    DECL_LINK(SavePasswordHdl, void* );
    DECL_LINK(MasterPasswordHdl, void *);
    DECL_LINK(MasterPasswordCBHdl, void* );
    DECL_LINK(ShowPasswordsHdl, void *);
    DECL_LINK(MacroSecPBHdl, void* );
    DECL_LINK(CertPathPBHdl, void* );

    void                InitControls();

                SvxSecurityTabPage( Window* pParent, const SfxItemSet& rSet );
    virtual     ~SvxSecurityTabPage();

protected:
    virtual void        ActivatePage( const SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 ) SAL_OVERRIDE;

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
    virtual bool        FillItemSet( SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet& rSet ) SAL_OVERRIDE;
};

class MozPluginTabPage : public SfxTabPage
{
    CheckBox* m_pWBasicCodeCB;

    sal_Bool isInstalled(void);
    sal_Bool installPlugin(void);
    sal_Bool uninstallPlugin(void);

    MozPluginTabPage( Window* pParent, const SfxItemSet& rSet );
    virtual ~MozPluginTabPage();

public:

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet );

    virtual bool        FillItemSet( SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet& rSet ) SAL_OVERRIDE;

};

struct SvxEMailTabPage_Impl;
class SvxEMailTabPage : public SfxTabPage
{
    VclContainer* m_pMailContainer;
    FixedImage*   m_pMailerURLFI;
    Edit*         m_pMailerURLED;
    PushButton*   m_pMailerURLPB;

    OUString      m_sDefaultFilterName;

    SvxEMailTabPage_Impl* pImpl;

    DECL_LINK(  FileDialogHdl_Impl, PushButton* ) ;

public:
    SvxEMailTabPage( Window* pParent, const SfxItemSet& rSet );
    ~SvxEMailTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual bool        FillItemSet( SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet& rSet ) SAL_OVERRIDE;
};

#endif // INCLUDED_CUI_SOURCE_OPTIONS_OPTINET2_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

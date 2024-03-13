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

#include <vcl/weld.hxx>
#include <unotools/securityoptions.hxx>

namespace com::sun::star::xml::crypto { class XSecurityEnvironment; }
class MacroSecurityTP;

class MacroSecurity final : public weld::GenericDialogController
{
private:
    friend class MacroSecurityLevelTP;
    friend class MacroSecurityTrustedSourcesTP;

    css::uno::Reference<css::xml::crypto::XSecurityEnvironment> m_xSecurityEnvironment;

    std::unique_ptr<weld::Notebook> m_xTabCtrl;
    std::unique_ptr<weld::Button> m_xOkBtn;
    std::unique_ptr<weld::Button> m_xResetBtn;

    std::unique_ptr<MacroSecurityTP> m_xLevelTP;
    std::unique_ptr<MacroSecurityTP> m_xTrustSrcTP;

    DECL_LINK(ActivatePageHdl, const OUString&, void);
    DECL_LINK(OkBtnHdl, weld::Button&, void);
public:
    MacroSecurity(weld::Window* pParent,
        css::uno::Reference<css::xml::crypto::XSecurityEnvironment> xSecurityEnvironment);

    void EnableReset(bool bEnable = true)
    {
        m_xResetBtn->set_sensitive(bEnable);
    }
};

class MacroSecurityTP
{
protected:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;

    MacroSecurity* m_pDlg;
public:
    MacroSecurityTP(weld::Container* pParent, const OUString& rUIXMLDescription,
                    const OUString& rID, MacroSecurity* pDlg);
    virtual ~MacroSecurityTP();

    virtual void ActivatePage();
    virtual void ClosePage() = 0;
};

class MacroSecurityLevelTP : public MacroSecurityTP
{
private:
    sal_uInt16   mnCurLevel;
    // Stores the security level when the dialog opens. Used to check if the value changed
    sal_uInt16   mnInitialLevel;

    std::unique_ptr<weld::RadioButton> m_xVeryHighRB;
    std::unique_ptr<weld::RadioButton> m_xHighRB;
    std::unique_ptr<weld::RadioButton> m_xMediumRB;
    std::unique_ptr<weld::RadioButton> m_xLowRB;
    std::unique_ptr<weld::Widget> m_xVHighImg;
    std::unique_ptr<weld::Widget> m_xHighImg;
    std::unique_ptr<weld::Widget> m_xMedImg;
    std::unique_ptr<weld::Widget> m_xLowImg;
    std::unique_ptr<weld::Label> m_xWarningLb;
    std::unique_ptr<weld::Image> m_xWarningImg;
    std::unique_ptr<weld::Box> m_xWarningBox;

    void SetWarningLabel(const OUString& sMsg);

    DECL_LINK(RadioButtonHdl, weld::Toggleable&, void);
public:
    MacroSecurityLevelTP(weld::Container* pParent, MacroSecurity* pDlg);
    virtual void ClosePage() override;
};

class MacroSecurityTrustedSourcesTP : public MacroSecurityTP
{
private:
    std::vector< SvtSecurityOptions::Certificate > m_aTrustedAuthors;

    bool          mbAuthorsReadonly;
    bool          mbURLsReadonly;

    std::unique_ptr<weld::Image> m_xTrustCertROFI;
    std::unique_ptr<weld::TreeView> m_xTrustCertLB;
    std::unique_ptr<weld::Button> m_xViewCertPB;
    std::unique_ptr<weld::Button> m_xRemoveCertPB;
    std::unique_ptr<weld::Image> m_xTrustFileROFI;
    std::unique_ptr<weld::TreeView>  m_xTrustFileLocLB;
    std::unique_ptr<weld::Button> m_xAddLocPB;
    std::unique_ptr<weld::Button> m_xRemoveLocPB;

    DECL_LINK(ViewCertPBHdl, weld::Button&, void);
    DECL_LINK(RemoveCertPBHdl, weld::Button&, void);
    DECL_LINK(AddLocPBHdl, weld::Button&, void);
    DECL_LINK(RemoveLocPBHdl, weld::Button&, void);
    DECL_LINK(TrustCertLBSelectHdl, weld::TreeView&, void);
    DECL_LINK(TrustFileLocLBSelectHdl, weld::TreeView&, void);

    void FillCertLB(const bool bShowWarnings = false);
    void ImplCheckButtons();
    void ShowBrokenCertificateError(std::u16string_view rData);

public:
    MacroSecurityTrustedSourcesTP(weld::Container* pParent, MacroSecurity* pDlg);

    virtual void        ActivatePage() override;
    virtual void        ClosePage() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

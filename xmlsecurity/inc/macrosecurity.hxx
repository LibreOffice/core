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

#ifndef INCLUDED_XMLSECURITY_INC_MACROSECURITY_HXX
#define INCLUDED_XMLSECURITY_INC_MACROSECURITY_HXX

#include <vcl/weld.hxx>
#include <unotools/securityoptions.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>

namespace com {
namespace sun {
namespace star {
namespace xml { namespace crypto {
    class XSecurityEnvironment; }}
}}}

class MacroSecurityTP;

class MacroSecurity : public weld::GenericDialogController
{
private:
    friend class MacroSecurityLevelTP;
    friend class MacroSecurityTrustedSourcesTP;

    css::uno::Reference<css::xml::crypto::XSecurityEnvironment> m_xSecurityEnvironment;
    SvtSecurityOptions m_aSecOptions;

    std::unique_ptr<weld::Notebook> m_xTabCtrl;
    std::unique_ptr<weld::Button> m_xOkBtn;
    std::unique_ptr<weld::Button> m_xResetBtn;

    std::unique_ptr<MacroSecurityTP> m_xLevelTP;
    std::unique_ptr<MacroSecurityTP> m_xTrustSrcTP;

    DECL_LINK(ActivatePageHdl, const OString&, void);
    DECL_LINK(OkBtnHdl, weld::Button&, void);
public:
    MacroSecurity(weld::Window* pParent,
        const css::uno::Reference<css::xml::crypto::XSecurityEnvironment>& rxSecurityEnvironment);

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
                    const OString& rID, MacroSecurity* pDlg);
    virtual ~MacroSecurityTP();

    virtual void ActivatePage();
    virtual void ClosePage() = 0;
};

class MacroSecurityLevelTP : public MacroSecurityTP
{
private:
    sal_uInt16   mnCurLevel;

    std::unique_ptr<weld::RadioButton> m_xVeryHighRB;
    std::unique_ptr<weld::RadioButton> m_xHighRB;
    std::unique_ptr<weld::RadioButton> m_xMediumRB;
    std::unique_ptr<weld::RadioButton> m_xLowRB;
    std::unique_ptr<weld::Widget> m_xVHighImg;
    std::unique_ptr<weld::Widget> m_xHighImg;
    std::unique_ptr<weld::Widget> m_xMedImg;
    std::unique_ptr<weld::Widget> m_xLowImg;

    DECL_LINK(RadioButtonHdl, weld::ToggleButton&, void);
public:
    MacroSecurityLevelTP(weld::Container* pParent, MacroSecurity* pDlg);
    virtual void ClosePage() override;
};

class MacroSecurityTrustedSourcesTP : public MacroSecurityTP
{
private:
    css::uno::Sequence< SvtSecurityOptions::Certificate > m_aTrustedAuthors;

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

    void FillCertLB();
    void ImplCheckButtons();

public:
    MacroSecurityTrustedSourcesTP(weld::Container* pParent, MacroSecurity* pDlg);

    virtual void        ActivatePage() override;
    virtual void        ClosePage() override;
};


#endif // INCLUDED_XMLSECURITY_INC_MACROSECURITY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

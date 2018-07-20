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

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/tabdlg.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <svtools/simptabl.hxx>
#include <unotools/securityoptions.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>

namespace com {
namespace sun {
namespace star {
namespace xml { namespace crypto {
    class XSecurityEnvironment; }}
}}}

class MacroSecurityTP;

class MacroSecurity : public TabDialog
{
private:
    friend class MacroSecurityLevelTP;
    friend class MacroSecurityTrustedSourcesTP;

    VclPtr<TabControl>         m_pTabCtrl;
    VclPtr<OKButton>           m_pOkBtn;
    VclPtr<PushButton>         m_pResetBtn;

    css::uno::Reference< css::xml::crypto::XSecurityEnvironment >  mxSecurityEnvironment;
    SvtSecurityOptions                                          maSecOptions;

    sal_uInt16 m_nSecLevelId;
    sal_uInt16 m_nSecTrustId;

    VclPtr<MacroSecurityTP>    mpLevelTP;
    VclPtr<MacroSecurityTP>    mpTrustSrcTP;

    DECL_LINK(          OkBtnHdl, Button*, void );
public:
    MacroSecurity(vcl::Window* pParent,
        const css::uno::Reference< css::xml::crypto::XSecurityEnvironment >& rxSecurityEnvironment);
    virtual ~MacroSecurity() override;
    virtual void dispose() override;

    void EnableReset(bool _bEnable = true)
    {
        m_pResetBtn->Enable ( _bEnable );
    }
};

class MacroSecurityTP : public TabPage
{
protected:
    VclPtr<MacroSecurity>      mpDlg;
public:
    MacroSecurityTP(vcl::Window* _pParent, const OString& rID,
        const OUString& rUIXMLDescription, MacroSecurity* _pDlg);
    virtual ~MacroSecurityTP() override;
    virtual void dispose() override;

    virtual void        ClosePage() = 0;
};

class MacroSecurityLevelTP : public MacroSecurityTP
{
private:
    VclPtr<RadioButton> m_pVeryHighRB;
    VclPtr<RadioButton> m_pHighRB;
    VclPtr<RadioButton> m_pMediumRB;
    VclPtr<RadioButton> m_pLowRB;

    sal_uInt16   mnCurLevel;

    DECL_LINK(RadioButtonHdl, Button*, void);

public:
                        MacroSecurityLevelTP( vcl::Window* pParent, MacroSecurity* _pDlg );
    virtual             ~MacroSecurityLevelTP() override;
    virtual void        dispose() override;

    virtual void        ClosePage() override;
};


class MacroSecurityTrustedSourcesTP : public MacroSecurityTP
{
private:
    VclPtr<FixedImage>         m_pTrustCertROFI;
    VclPtr<SvSimpleTable>     m_pTrustCertLB;
    VclPtr<PushButton>         m_pViewCertPB;
    VclPtr<PushButton>         m_pRemoveCertPB;
    VclPtr<FixedImage>         m_pTrustFileROFI;
    VclPtr<ListBox>            m_pTrustFileLocLB;
    VclPtr<PushButton>         m_pAddLocPB;
    VclPtr<PushButton>         m_pRemoveLocPB;

    css::uno::Sequence< SvtSecurityOptions::Certificate > maTrustedAuthors;

    bool          mbAuthorsReadonly;
    bool          mbURLsReadonly;

    DECL_LINK(    ViewCertPBHdl, Button*, void );
    DECL_LINK(    RemoveCertPBHdl, Button*, void );
    DECL_LINK(    AddLocPBHdl, Button*, void );
    DECL_LINK(    RemoveLocPBHdl, Button*, void );
    DECL_LINK(    TrustCertLBSelectHdl, SvTreeListBox*, void );
    DECL_LINK(    TrustFileLocLBSelectHdl, ListBox&, void );

    void                FillCertLB();
    void                ImplCheckButtons();

public:
    MacroSecurityTrustedSourcesTP(vcl::Window* pParent, MacroSecurity* _pDlg);
    virtual ~MacroSecurityTrustedSourcesTP() override;
    virtual void        dispose() override;

    virtual void        ActivatePage() override;
    virtual void        ClosePage() override;
};


#endif // INCLUDED_XMLSECURITY_INC_MACROSECURITY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

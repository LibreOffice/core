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

#ifndef _XMLSECURITY_MACROSECURITY_HXX
#define _XMLSECURITY_MACROSECURITY_HXX

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/tabdlg.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <svtools/simptabl.hxx>
#include <svtools/stdctrl.hxx>
#include <unotools/securityoptions.hxx>

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

    TabControl*         m_pTabCtrl;
    OKButton*           m_pOkBtn;
    PushButton*         m_pResetBtn;

    css::uno::Reference< css::uno::XComponentContext >  mxCtx;
    css::uno::Reference< css::xml::crypto::XSecurityEnvironment >  mxSecurityEnvironment;
    SvtSecurityOptions                                          maSecOptions;

    sal_uInt16 m_nSecLevelId;
    sal_uInt16 m_nSecTrustId;

    MacroSecurityTP*    mpLevelTP;
    MacroSecurityTP*    mpTrustSrcTP;

    DECL_LINK(          OkBtnHdl, void* );
public:
    MacroSecurity(Window* pParent,
        const css::uno::Reference< css::uno::XComponentContext>& rxCtx,
        const css::uno::Reference< css::xml::crypto::XSecurityEnvironment >& rxSecurityEnvironment);
    virtual ~MacroSecurity();

    inline void EnableReset(bool _bEnable = true)
    {
        m_pResetBtn->Enable ( _bEnable );
    }
};

class MacroSecurityTP : public TabPage
{
protected:
    MacroSecurity*      mpDlg;
public:
    MacroSecurityTP(Window* _pParent, const OString& rID,
        const OUString& rUIXMLDescription, MacroSecurity* _pDlg);

    void SetTabDlg(MacroSecurity* pTabDlg)
    {
        mpDlg = pTabDlg;
    }

    virtual void        ClosePage( void ) = 0;
};

class MacroSecurityLevelTP : public MacroSecurityTP
{
private:
    RadioButton* m_pVeryHighRB;
    RadioButton* m_pHighRB;
    RadioButton* m_pMediumRB;
    RadioButton* m_pLowRB;

    sal_uInt16   mnCurLevel;

protected:
    DECL_LINK(RadioButtonHdl, void *);

public:
                        MacroSecurityLevelTP( Window* pParent, MacroSecurity* _pDlg );

    virtual void        ClosePage( void );
};


class MacroSecurityTrustedSourcesTP : public MacroSecurityTP
{
private:
    FixedImage*         m_pTrustCertROFI;
    SvSimpleTable*     m_pTrustCertLB;
    PushButton*         m_pViewCertPB;
    PushButton*         m_pRemoveCertPB;
    FixedImage*         m_pTrustFileROFI;
    ListBox*            m_pTrustFileLocLB;
    PushButton*         m_pAddLocPB;
    PushButton*         m_pRemoveLocPB;

    css::uno::Sequence< SvtSecurityOptions::Certificate > maTrustedAuthors;

    bool                mbAuthorsReadonly;
    bool                mbURLsReadonly;

    DECL_LINK(          ViewCertPBHdl, void* );
    DECL_LINK(          RemoveCertPBHdl, void* );
    DECL_LINK(          AddLocPBHdl, void* );
    DECL_LINK(          RemoveLocPBHdl, void* );
    DECL_LINK(          TrustCertLBSelectHdl, void* );
    DECL_LINK(          TrustFileLocLBSelectHdl, void* );

    void                FillCertLB( void );
    void                ImplCheckButtons();

public:
    MacroSecurityTrustedSourcesTP(Window* pParent, MacroSecurity* _pDlg);
    ~MacroSecurityTrustedSourcesTP();

    virtual void        ActivatePage();
    virtual void        ClosePage( void );
};


#endif // _XMLSECURITY_MACROSECURITY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

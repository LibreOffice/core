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

#ifndef _XMLSECURITY_CERTIFICATEVIEWER_HXX
#define _XMLSECURITY_CERTIFICATEVIEWER_HXX

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/tabdlg.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <svtools/simptabl.hxx>
#include <svtools/stdctrl.hxx>
#include <svtools/svmedit.hxx>

namespace com {
namespace sun {
namespace star {
namespace security {
    class XCertificate; }
namespace xml { namespace crypto {
    class XSecurityEnvironment; }}
}}}

class CertificateViewer : public TabDialog
{
private:
    friend class CertificateViewerGeneralTP;
    friend class CertificateViewerDetailsTP;
    friend class CertificateViewerCertPathTP;

    TabControl          maTabCtrl;
    OKButton            maOkBtn;
    HelpButton          maHelpBtn;

    bool                mbCheckForPrivateKey;

    css::uno::Reference< css::xml::crypto::XSecurityEnvironment > mxSecurityEnvironment;
    css::uno::Reference< css::security::XCertificate > mxCert;
public:
    CertificateViewer( Window* pParent, const css::uno::Reference< css::xml::crypto::XSecurityEnvironment >& rxSecurityEnvironment, const css::uno::Reference< css::security::XCertificate >& rXCert, bool bCheckForPrivateKey );
    virtual             ~CertificateViewer();
};


class CertificateViewerTP : public TabPage
{
protected:
    CertificateViewer*  mpDlg;
public:
    CertificateViewerTP( Window* _pParent, const ResId& _rResId, CertificateViewer* _pDlg );
    inline void         SetTabDlg( CertificateViewer* pTabDlg );
};

inline void CertificateViewerTP::SetTabDlg( CertificateViewer* _pTabDlg )
{
    mpDlg = _pTabDlg;
}


class CertificateViewerGeneralTP : public CertificateViewerTP
{
private:
    Window              maFrameWin;
    FixedImage          maCertImg;
    FixedInfo           maCertInfoFI;
    FixedLine           maSep1FL;
    FixedInfo           maHintNotTrustedFI;
    FixedLine           maSep2FL;
    FixedInfo           maIssuedToLabelFI;
    FixedInfo           maIssuedToFI;
    FixedInfo           maIssuedByLabelFI;
    FixedInfo           maIssuedByFI;
    FixedInfo           maValidDateFI;
    FixedImage          maKeyImg;
    FixedInfo           maHintCorrespPrivKeyFI;
public:
                        CertificateViewerGeneralTP( Window* pParent, CertificateViewer* _pDlg );

    virtual void        ActivatePage();
};


class CertificateViewerDetailsTP : public CertificateViewerTP
{
private:
    SvSimpleTableContainer m_aElementsLBContainer;
    SvSimpleTable      maElementsLB;
    MultiLineEdit       maElementML;
    Font                maStdFont;
    Font                maFixedWidthFont;

    DECL_LINK(          ElementSelectHdl, void* );
    void                Clear( void );
    void                InsertElement( const OUString& _rField, const OUString& _rValue,
                                       const OUString& _rDetails, bool _bFixedWidthFont = false );
public:
                        CertificateViewerDetailsTP( Window* pParent, CertificateViewer* _pDlg );
    virtual             ~CertificateViewerDetailsTP();

    virtual void        ActivatePage();
};


class CertificateViewerCertPathTP : public CertificateViewerTP
{
private:
    FixedText           maCertPathFT;
    SvTreeListBox       maCertPathLB;
    PushButton          maViewCertPB;
    FixedText           maCertStatusFT;
    MultiLineEdit       maCertStatusML;

    CertificateViewer*  mpParent;
    bool                mbFirstActivateDone;
    Image               maCertImage;
    Image               maCertNotValidatedImage;
    OUString            msCertOK;
    OUString            msCertNotValidated;

    DECL_LINK(          ViewCertHdl, void* );
    DECL_LINK(          CertSelectHdl, void* );
    void                Clear( void );
    SvTreeListEntry*        InsertCert( SvTreeListEntry* _pParent, const OUString& _rName,
                                    css::uno::Reference< css::security::XCertificate > rxCert,
                                    bool bValid);

public:
                        CertificateViewerCertPathTP( Window* pParent, CertificateViewer* _pDlg );
    virtual             ~CertificateViewerCertPathTP();

    virtual void        ActivatePage();
};


#endif // _XMLSECURITY_CERTIFICATEVIEWER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

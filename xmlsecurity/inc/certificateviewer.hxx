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

#ifndef INCLUDED_XMLSECURITY_INC_CERTIFICATEVIEWER_HXX
#define INCLUDED_XMLSECURITY_INC_CERTIFICATEVIEWER_HXX

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/tabdlg.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <svtools/simptabl.hxx>
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

    VclPtr<TabControl>         mpTabCtrl;
    sal_uInt16          mnGeneralId;
    sal_uInt16          mnDetailsId;
    sal_uInt16          mnPathId;

    bool const          mbCheckForPrivateKey;

    css::uno::Reference< css::xml::crypto::XSecurityEnvironment > mxSecurityEnvironment;
    css::uno::Reference< css::security::XCertificate > mxCert;
public:
    CertificateViewer( vcl::Window* pParent, const css::uno::Reference< css::xml::crypto::XSecurityEnvironment >& rxSecurityEnvironment, const css::uno::Reference< css::security::XCertificate >& rXCert, bool bCheckForPrivateKey );
    virtual             ~CertificateViewer() override;
    virtual void        dispose() override;
};


class CertificateViewerTP : public TabPage
{
protected:
    VclPtr<CertificateViewer>  mpDlg;
public:
    CertificateViewerTP( vcl::Window* _pParent, const OString& rID,
        const OUString& rUIXMLDescription, CertificateViewer* _pDlg );
    virtual ~CertificateViewerTP() override;
    virtual void dispose() override;
};

class CertificateViewerGeneralTP : public CertificateViewerTP
{
private:
    VclPtr<FixedImage>         m_pCertImg;
    VclPtr<FixedText>          m_pHintNotTrustedFT;
    VclPtr<FixedText>          m_pIssuedToLabelFT;
    VclPtr<FixedText>          m_pIssuedToFT;
    VclPtr<FixedText>          m_pIssuedByLabelFT;
    VclPtr<FixedText>          m_pIssuedByFT;
    VclPtr<FixedText>          m_pValidFromDateFT;
    VclPtr<FixedText>          m_pValidToDateFT;
    VclPtr<FixedImage>         m_pKeyImg;
    VclPtr<FixedText>          m_pHintCorrespPrivKeyFT;
public:
                        CertificateViewerGeneralTP( vcl::Window* pParent, CertificateViewer* _pDlg );
    virtual             ~CertificateViewerGeneralTP() override;
    virtual void        dispose() override;

    virtual void        ActivatePage() override;
};


class CertificateViewerDetailsTP : public CertificateViewerTP
{
private:
    VclPtr<SvSimpleTableContainer> m_pElementsLBContainer;
    VclPtr<SvSimpleTable>          m_pElementsLB;
    VclPtr<VclMultiLineEdit>       m_pValueDetails;
    vcl::Font                      m_aStdFont;
    vcl::Font                      m_aFixedWidthFont;

    DECL_LINK(    ElementSelectHdl, SvTreeListBox*, void );
    void                Clear();
    void                InsertElement( const OUString& _rField, const OUString& _rValue,
                                       const OUString& _rDetails, bool _bFixedWidthFont = false );
public:
                        CertificateViewerDetailsTP( vcl::Window* pParent, CertificateViewer* _pDlg );
    virtual             ~CertificateViewerDetailsTP() override;
    virtual void        dispose() override;

    virtual void        ActivatePage() override;
};


class CertificateViewerCertPathTP : public CertificateViewerTP
{
private:
    VclPtr<SvTreeListBox>      mpCertPathLB;
    VclPtr<PushButton>         mpViewCertPB;
    VclPtr<VclMultiLineEdit>   mpCertStatusML;

    VclPtr<CertificateViewer>  mpParent;
    bool                mbFirstActivateDone;
    Image               maCertImage;
    Image               maCertNotValidatedImage;
    OUString            msCertOK;
    OUString            msCertNotValidated;

    DECL_LINK(    ViewCertHdl, Button*, void );
    DECL_LINK(    CertSelectHdl, SvTreeListBox*, void );
    void                Clear();
    SvTreeListEntry*    InsertCert( SvTreeListEntry* _pParent, const OUString& _rName,
                                    const css::uno::Reference< css::security::XCertificate >& rxCert,
                                    bool bValid);

public:
                        CertificateViewerCertPathTP( vcl::Window* pParent, CertificateViewer* _pDlg );
    virtual             ~CertificateViewerCertPathTP() override;
    virtual void        dispose() override;

    virtual void        ActivatePage() override;
};


#endif // INCLUDED_XMLSECURITY_INC_CERTIFICATEVIEWER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <vcl/weld.hxx>

namespace com {
namespace sun {
namespace star {
namespace security {
    class XCertificate; }
namespace xml { namespace crypto {
    class XSecurityEnvironment; }}
}}}

class CertificateViewerGeneralTP;
class CertificateViewerDetailsTP;
class CertificateViewerCertPathTP;

class CertificateViewer : public weld::GenericDialogController
{
private:
    friend class CertificateViewerGeneralTP;
    friend class CertificateViewerDetailsTP;
    friend class CertificateViewerCertPathTP;

    bool const          mbCheckForPrivateKey;

    css::uno::Reference< css::xml::crypto::XSecurityEnvironment > mxSecurityEnvironment;
    css::uno::Reference< css::security::XCertificate > mxCert;

    CertificateChooser* mpParentChooser;

    std::unique_ptr<weld::Notebook> mxTabCtrl;

    std::unique_ptr<CertificateViewerGeneralTP> mxGeneralPage;
    std::unique_ptr<CertificateViewerDetailsTP> mxDetailsPage;
    std::unique_ptr<CertificateViewerCertPathTP> mxPathId;

    DECL_LINK(ActivatePageHdl, const OString&, void);

public:
    CertificateViewer(weld::Window* pParent, const css::uno::Reference< css::xml::crypto::XSecurityEnvironment >& rxSecurityEnvironment, const css::uno::Reference< css::security::XCertificate >& rXCert, bool bCheckForPrivateKey, CertificateChooser* pParentChooser);
    CertificateChooser* GetParentChooser() { return mpParentChooser; }
};

class CertificateViewerTP
{
protected:
    std::unique_ptr<weld::Builder> mxBuilder;
    std::unique_ptr<weld::Container> mxContainer;
    CertificateViewer* mpDlg;

public:
    CertificateViewerTP(weld::Container* pParent, const OUString& rUIXMLDescription,
                        const OString& rID, CertificateViewer* pDlg);
};

class CertificateViewerGeneralTP : public CertificateViewerTP
{
private:
    std::unique_ptr<weld::Image> m_xCertImg;
    std::unique_ptr<weld::Label> m_xHintNotTrustedFT;
    std::unique_ptr<weld::Label> m_xIssuedToLabelFT;
    std::unique_ptr<weld::Label> m_xIssuedToFT;
    std::unique_ptr<weld::Label> m_xIssuedByLabelFT;
    std::unique_ptr<weld::Label> m_xIssuedByFT;
    std::unique_ptr<weld::Label> m_xValidFromDateFT;
    std::unique_ptr<weld::Label> m_xValidToDateFT;
    std::unique_ptr<weld::Image> m_xKeyImg;
    std::unique_ptr<weld::Label> m_xHintCorrespPrivKeyFT;

public:
    CertificateViewerGeneralTP(weld::Container* pParent, CertificateViewer* pDlg);
};

struct Details_UserDatat
{
    OUString const  maTxt;
    bool const      mbFixedWidthFont;

    Details_UserDatat(const OUString& rTxt, bool bFixedWidthFont)
        : maTxt(rTxt)
        , mbFixedWidthFont(bFixedWidthFont)
    {
    }
};

class CertificateViewerDetailsTP : public CertificateViewerTP
{
private:
    std::vector<std::unique_ptr<Details_UserDatat>> m_aUserData;

    std::unique_ptr<weld::TreeView> m_xElementsLB;
    std::unique_ptr<weld::TextView> m_xValueDetails;

    DECL_LINK(ElementSelectHdl, weld::TreeView&, void);
    void                InsertElement(const OUString& rField, const OUString& rValue,
                                      const OUString& rDetails, bool bFixedWidthFont = false);
public:
    CertificateViewerDetailsTP(weld::Container* pParent, CertificateViewer* pDlg);
};

struct CertPath_UserData
{
    css::uno::Reference< css::security::XCertificate > mxCert;
    bool const mbValid;

    CertPath_UserData(css::uno::Reference<css::security::XCertificate> const & xCert, bool bValid)
        : mxCert(xCert)
        , mbValid(bValid)
    {
    }
};

class CertificateViewerCertPathTP : public CertificateViewerTP
{
private:
    CertificateViewer*  mpParent;
    bool                mbFirstActivateDone;

    std::vector<std::unique_ptr<CertPath_UserData>> maUserData;

    std::unique_ptr<weld::TreeView> mxCertPathLB;
    std::unique_ptr<weld::Button> mxViewCertPB;
    std::unique_ptr<weld::TextView> mxCertStatusML;
    std::unique_ptr<weld::Label> mxCertOK;
    std::unique_ptr<weld::Label> mxCertNotValidated;

    DECL_LINK(ViewCertHdl, weld::Button&, void);
    DECL_LINK(CertSelectHdl, weld::TreeView&, void);
    void                InsertCert(weld::TreeIter* pParent, const OUString& _rName,
                                   const css::uno::Reference< css::security::XCertificate >& rxCert,
                                   bool bValid);

public:
    CertificateViewerCertPathTP(weld::Container* pParent, CertificateViewer* pDlg);
    void ActivatePage();
};


#endif // INCLUDED_XMLSECURITY_INC_CERTIFICATEVIEWER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

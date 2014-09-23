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

#ifndef INCLUDED_XMLSECURITY_INC_XMLSECURITY_CERTIFICATEVIEWER_HXX
#define INCLUDED_XMLSECURITY_INC_XMLSECURITY_CERTIFICATEVIEWER_HXX

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

    TabControl*         mpTabCtrl;
    sal_uInt16          mnGeneralId;
    sal_uInt16          mnDetailsId;
    sal_uInt16          mnPathId;

    bool                mbCheckForPrivateKey;

    css::uno::Reference< css::xml::crypto::XSecurityEnvironment > mxSecurityEnvironment;
    css::uno::Reference< css::security::XCertificate > mxCert;
public:
    CertificateViewer( vcl::Window* pParent, const css::uno::Reference< css::xml::crypto::XSecurityEnvironment >& rxSecurityEnvironment, const css::uno::Reference< css::security::XCertificate >& rXCert, bool bCheckForPrivateKey );
    virtual             ~CertificateViewer();
};


class CertificateViewerTP : public TabPage
{
protected:
    CertificateViewer*  mpDlg;
public:
    CertificateViewerTP( vcl::Window* _pParent, const OString& rID,
        const OUString& rUIXMLDescription, CertificateViewer* _pDlg );
    void SetTabDlg( CertificateViewer* _pTabDlg )
    {
        mpDlg = _pTabDlg;
    }
};

class CertificateViewerGeneralTP : public CertificateViewerTP
{
private:
    FixedImage*         m_pCertImg;
    FixedText*          m_pHintNotTrustedFI;
    FixedText*          m_pIssuedToFI;
    FixedText*          m_pIssuedByFI;
    FixedText*          m_pValidFromDateFI;
    FixedText*          m_pValidToDateFI;
    FixedImage*         m_pKeyImg;
    FixedText*          m_pHintCorrespPrivKeyFI;
public:
                        CertificateViewerGeneralTP( vcl::Window* pParent, CertificateViewer* _pDlg );

    virtual void        ActivatePage() SAL_OVERRIDE;
};


class CertificateViewerDetailsTP : public CertificateViewerTP
{
private:
    SvSimpleTableContainer* m_pElementsLBContainer;
    SvSimpleTable*          m_pElementsLB;
    MultiLineEdit*          m_pValueDetails;
    vcl::Font               m_aStdFont;
    vcl::Font               m_aFixedWidthFont;

    DECL_LINK(          ElementSelectHdl, void* );
    void                Clear( void );
    void                InsertElement( const OUString& _rField, const OUString& _rValue,
                                       const OUString& _rDetails, bool _bFixedWidthFont = false );
public:
                        CertificateViewerDetailsTP( vcl::Window* pParent, CertificateViewer* _pDlg );
    virtual             ~CertificateViewerDetailsTP();

    virtual void        ActivatePage() SAL_OVERRIDE;
};


class CertificateViewerCertPathTP : public CertificateViewerTP
{
private:
    SvTreeListBox*      mpCertPathLB;
    PushButton*         mpViewCertPB;
    VclMultiLineEdit*   mpCertStatusML;

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
                        CertificateViewerCertPathTP( vcl::Window* pParent, CertificateViewer* _pDlg );
    virtual             ~CertificateViewerCertPathTP();

    virtual void        ActivatePage() SAL_OVERRIDE;
};


#endif // INCLUDED_XMLSECURITY_INC_XMLSECURITY_CERTIFICATEVIEWER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

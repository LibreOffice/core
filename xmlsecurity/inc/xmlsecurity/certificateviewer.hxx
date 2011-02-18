/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _XMLSECURITY_CERTIFICATEVIEWER_HXX
#define _XMLSECURITY_CERTIFICATEVIEWER_HXX

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/tabdlg.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <svtools/stdctrl.hxx>
#include <svx/simptabl.hxx>
#include <svtools/svmedit.hxx>

namespace com {
namespace sun {
namespace star {
namespace security {
    class XCertificate; }
namespace xml { namespace crypto {
    class XSecurityEnvironment; }}
}}}

namespace css = com::sun::star;
namespace cssu = com::sun::star::uno;
namespace dcss = ::com::sun::star;

class CertificateViewer : public TabDialog
{
private:
    friend class CertificateViewerGeneralTP;
    friend class CertificateViewerDetailsTP;
    friend class CertificateViewerCertPathTP;

    TabControl          maTabCtrl;
    OKButton            maOkBtn;
    HelpButton          maHelpBtn;

    sal_Bool                mbCheckForPrivateKey;

    cssu::Reference< dcss::xml::crypto::XSecurityEnvironment > mxSecurityEnvironment;
    cssu::Reference< dcss::security::XCertificate > mxCert;
public:
        CertificateViewer( Window* pParent, const cssu::Reference< dcss::xml::crypto::XSecurityEnvironment >& rxSecurityEnvironment, const cssu::Reference< dcss::security::XCertificate >& rXCert, sal_Bool bCheckForPrivateKey );
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
    SvxSimpleTable      maElementsLB;   // PB 2006/02/02 #i48648 now SvHeaderTabListBox
    MultiLineEdit       maElementML;
    Font                maStdFont;
    Font                maFixedWidthFont;

    DECL_LINK(          ElementSelectHdl, void* );
    void                Clear( void );
    void                InsertElement( const String& _rField, const String& _rValue,
                                        const String& _rDetails, bool _bFixedWidthFont = false );
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
    String              msCertOK;
    String              msCertNotValidated;

    DECL_LINK(          ViewCertHdl, void* );
    DECL_LINK(          CertSelectHdl, void* );
    void                Clear( void );
    SvLBoxEntry*        InsertCert( SvLBoxEntry* _pParent, const String& _rName,
                                    cssu::Reference< dcss::security::XCertificate > rxCert,
                                    bool bValid);

public:
                        CertificateViewerCertPathTP( Window* pParent, CertificateViewer* _pDlg );
    virtual             ~CertificateViewerCertPathTP();

    virtual void        ActivatePage();
};


#endif // _XMLSECURITY_CERTIFICATEVIEWER_HXX


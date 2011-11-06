/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


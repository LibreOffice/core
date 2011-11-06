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



#ifndef _XMLSECURITY_CERTIFICATECHOOSER_HXX
#define _XMLSECURITY_CERTIFICATECHOOSER_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <svx/simptabl.hxx>

namespace com {
namespace sun {
namespace star {
namespace security {
    class XCertificate; }
namespace xml { namespace crypto {
    class XSecurityEnvironment; }}
}}}

#include <com/sun/star/uno/Sequence.hxx>
#include <xmlsecurity/sigstruct.hxx>

namespace css = com::sun::star;
namespace cssu = com::sun::star::uno;
namespace dcss = ::com::sun::star;

class HeaderBar;

class CertificateChooser : public ModalDialog
{
private:
    cssu::Reference< cssu::XComponentContext > mxCtx;
    cssu::Reference< dcss::xml::crypto::XSecurityEnvironment > mxSecurityEnvironment;
    cssu::Sequence< cssu::Reference< dcss::security::XCertificate > > maCerts;
    SignatureInformations maCertsToIgnore;

    FixedText           maHintFT;
    SvxSimpleTable      maCertLB;   // PB 2006/02/02 #i48648 now SvHeaderTabListBox

    PushButton          maViewBtn;

    FixedLine           maBottomSepFL;
    OKButton            maOKBtn;
    CancelButton        maCancelBtn;
    HelpButton          maHelpBtn;

    sal_Bool                mbInitialized;

    sal_uInt16              GetSelectedEntryPos( void ) const;
//  DECL_LINK(          Initialize, void* );
    DECL_LINK(          ViewButtonHdl, Button* );
    DECL_LINK(          CertificateHighlightHdl, void* );
    DECL_LINK(          CertificateSelectHdl, void* );

    void ImplShowCertificateDetails();
    void ImplInitialize();

public:
    CertificateChooser( Window* pParent, cssu::Reference< cssu::XComponentContext>& rxCtx, cssu::Reference< dcss::xml::crypto::XSecurityEnvironment >& rxSecurityEnvironment, const SignatureInformations& rCertsToIgnore );
    ~CertificateChooser();

    short Execute();

    cssu::Reference< dcss::security::XCertificate > GetSelectedCertificate();

};

#endif // _XMLSECURITY_CERTIFICATECHOOSER_HXX


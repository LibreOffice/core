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



#ifndef _SECMACROWARNINGS_HXX
#define _SECMACROWARNINGS_HXX

#include <vcl/dialog.hxx>
#include <svtools/stdctrl.hxx>
#include <unotools/securityoptions.hxx>
#include <vcl/button.hxx>

namespace com {
namespace sun {
namespace star {
namespace security {
    class XCertificate; }
namespace xml { namespace crypto {
    class XSecurityEnvironment; }}
}}}

#include <com/sun/star/uno/Sequence.hxx>


namespace css = com::sun::star;
namespace cssu = com::sun::star::uno;
namespace css = ::com::sun::star;

class HeaderBar;

class MacroWarning : public ModalDialog
{
private:

    cssu::Reference< css::security::XCertificate >  mxCert;
    cssu::Reference< css::embed::XStorage >         mxStore;
    ::rtl::OUString                                 maODFVersion;
    const cssu::Sequence< css::security::DocumentSignatureInformation >*    mpInfos;

    FixedImage          maSymbolImg;
    FixedInfo           maDocNameFI;
    FixedInfo           maDescr1aFI;
    FixedInfo           maDescr1bFI;
    FixedInfo           maSignsFI;
    PushButton          maViewSignsBtn;
    FixedInfo           maDescr2FI;
    CheckBox            maAlwaysTrustCB;
    FixedLine           maBottomSepFL;
    OKButton            maEnableBtn;
    CancelButton        maDisableBtn;
    HelpButton          maHelpBtn;

    const bool          mbSignedMode;           // modus of dialog (signed / unsigned macros)
    const bool          mbShowSignatures;
    sal_Int32           mnActSecLevel;

    DECL_LINK(          ViewSignsBtnHdl, void* );
    DECL_LINK(          EnableBtnHdl, void* );
    DECL_LINK(          DisableBtnHdl, void* );
    DECL_LINK(          AlwaysTrustCheckHdl, void* );

    void                InitControls();
    void                FitControls();

public:
    MacroWarning( Window* pParent, bool _bShowSignatures, ResMgr& rResMgr );
    ~MacroWarning();

    virtual short   Execute();

    void    SetDocumentURL( const String& rDocURL );

    void    SetStorage( const cssu::Reference < css::embed::XStorage >& rxStore,
                        const ::rtl::OUString& aODFVersion,
                        const cssu::Sequence< css::security::DocumentSignatureInformation >& _rInfos );
    void    SetCertificate( const cssu::Reference< css::security::XCertificate >& _rxCert );
};

#endif


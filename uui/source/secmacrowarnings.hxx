/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: secmacrowarnings.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 16:24:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SECMACROWARNINGS_HXX
#define _SECMACROWARNINGS_HXX

#include <vcl/dialog.hxx>
#include <svtools/stdctrl.hxx>
#include <svtools/securityoptions.hxx>
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
                        const cssu::Sequence< css::security::DocumentSignatureInformation >& _rInfos );
    void    SetCertificate( const cssu::Reference< css::security::XCertificate >& _rxCert );
};

#endif


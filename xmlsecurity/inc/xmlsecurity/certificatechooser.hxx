/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: certificatechooser.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:06:16 $
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

#ifndef _SIGSTRUCT_HXX
#include <xmlsecurity/sigstruct.hxx>
#endif

namespace css = com::sun::star;
namespace cssu = com::sun::star::uno;
namespace dcss = ::com::sun::star;

class HeaderBar;

class CertificateChooser : public ModalDialog
{
private:
    cssu::Reference< dcss::xml::crypto::XSecurityEnvironment > mxSecurityEnvironment;
    cssu::Sequence< cssu::Reference< dcss::security::XCertificate > > maCerts;
    SignatureInformations maCertsToIgnore;

    FixedText           maHintFT;
    SvxSimpleTable      maCertLB;

    PushButton          maViewBtn;

    FixedLine           maBottomSepFL;
    OKButton            maOKBtn;
    CancelButton        maCancelBtn;
    HelpButton          maHelpBtn;

    BOOL                mbInitialized;

    USHORT              GetSelectedEntryPos( void ) const;
//  DECL_LINK(          Initialize, void* );
    DECL_LINK(          ViewButtonHdl, Button* );
    DECL_LINK(          CertificateHighlightHdl, void* );
    DECL_LINK(          CertificateSelectHdl, void* );

    void ImplShowCertificateDetails();
    void ImplInitialize();

public:
    CertificateChooser( Window* pParent, cssu::Reference< dcss::xml::crypto::XSecurityEnvironment >& rxSecurityEnvironment, const SignatureInformations& rCertsToIgnore );
    ~CertificateChooser();

    short Execute();

    cssu::Reference< dcss::security::XCertificate > GetSelectedCertificate();

};

#endif // _XMLSECURITY_CERTIFICATECHOOSER_HXX


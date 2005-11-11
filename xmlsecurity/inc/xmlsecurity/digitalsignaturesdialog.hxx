/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: digitalsignaturesdialog.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-11 09:17:24 $
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

#ifndef _XMLSECURITY_DIGITALSIGNATURESDIALOG_HXX
#define _XMLSECURITY_DIGITALSIGNATURESDIALOG_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <svtools/stdctrl.hxx>
#include <svx/simptabl.hxx>

#include <xmlsecurity/documentsignaturehelper.hxx>
#include <xmlsecurity/xmlsignaturehelper.hxx>

#ifndef _STLP_VECTOR
#include <vector>
#endif

namespace com {
namespace sun {
namespace star {
namespace lang {
    class XMultiServiceFactory; }
namespace io {
    class XStream; }
namespace embed {
    class XStorage; }
}}}

namespace css = com::sun::star;
namespace cssu = com::sun::star::uno;

class HeaderBar;

class DigitalSignaturesDialog : public ModalDialog
{
private:
    XMLSignatureHelper      maSignatureHelper;

    css::uno::Reference < css::embed::XStorage > mxStore;
    css::uno::Reference < css::io::XStream > mxSignatureStream;
    SignatureInformations   maCurrentSignatureInformations;
    bool                    mbVerifySignatures;
    bool                    mbSignaturesChanged;
    DocumentSignatureMode   meSignatureMode;

    FixedText           maHintDocFT;
    FixedText           maHintBasicFT;
    FixedText           maHintPackageFT;
    SvxSimpleTable      maSignaturesLB;
    FixedImage          maSigsValidImg;
    FixedInfo           maSigsValidFI;
    FixedImage          maSigsInvalidImg;
    FixedInfo           maSigsInvalidFI;
    FixedImage          maSigsNotvalidatedImg;
    FixedInfo           maSigsNotvalidatedFI;

    PushButton          maViewBtn;
    PushButton          maAddBtn;
    PushButton          maRemoveBtn;

    FixedLine           maBottomSepFL;
    OKButton            maOKBtn;
    CancelButton        maCancelBtn;
    HelpButton          maHelpBtn;

    DECL_LINK(          ViewButtonHdl, Button* );
    DECL_LINK(          AddButtonHdl, Button* );
    DECL_LINK(          RemoveButtonHdl, Button* );
    DECL_LINK(          SignatureHighlightHdl, void* );
    DECL_LINK(          SignatureSelectHdl, void* );
    DECL_LINK(          StartVerifySignatureHdl, void* );

    void                ImplGetSignatureInformations();
    void                ImplFillSignaturesBox();
    void                ImplShowSignaturesDetails();
    SignatureStreamHelper ImplOpenSignatureStream( sal_Int32 eStreamMode );

public:
    DigitalSignaturesDialog( Window* pParent, cssu::Reference< css::lang::XMultiServiceFactory >& rxMSF, DocumentSignatureMode eMode, sal_Bool bReadOnly );
    ~DigitalSignaturesDialog();

            // Initialize the dialog and the security environment, returns TRUE on success
    BOOL    Init( const rtl::OUString& rTokenName );

            // Set the storage which should be signed or verified
    void    SetStorage( const cssu::Reference < css::embed::XStorage >& rxStore );
    void    SetSignatureStream( const cssu::Reference < css::io::XStream >& rxStream );

                // Execute the dialog...
    short       Execute();

                // Did signatures change?
    sal_Bool    SignaturesChanged() const { return mbSignaturesChanged; }
};

#endif // _XMLSECURITY_DIGITALSIGNATURESDIALOG_HXX


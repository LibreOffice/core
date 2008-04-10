/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: digitalsignaturesdialog.hxx,v $
 * $Revision: 1.10 $
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
    SvxSimpleTable      maSignaturesLB; // PB 2006/02/02 #i48648 now SvHeaderTabListBox
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


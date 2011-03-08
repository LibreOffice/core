/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _XMLSECURITY_DIGITALSIGNATURESDIALOG_HXX
#define _XMLSECURITY_DIGITALSIGNATURESDIALOG_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <svtools/stdctrl.hxx>
#include <svx/simptabl.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

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
namespace xml { namespace dom {
    class XDocumentBuilder; } }
}}}

namespace css = com::sun::star;
namespace cssu = com::sun::star::uno;

class HeaderBar;

class DigitalSignaturesDialog : public ModalDialog
{
private:
    cssu::Reference< cssu::XComponentContext >& mxCtx;
    XMLSignatureHelper      maSignatureHelper;

    css::uno::Reference < css::embed::XStorage > mxStore;
    css::uno::Reference < css::io::XStream > mxSignatureStream;
    css::uno::Reference < css::io::XStream > mxTempSignatureStream;
    SignatureInformations   maCurrentSignatureInformations;
    bool                    mbVerifySignatures;
    bool                    mbSignaturesChanged;
    DocumentSignatureMode   meSignatureMode;
    css::uno::Sequence < css::uno::Sequence < css::beans::PropertyValue > > m_manifest;

    FixedText           maHintDocFT;
    FixedText           maHintBasicFT;
    FixedText           maHintPackageFT;
    SvxSimpleTable      maSignaturesLB; // #i48648 now SvHeaderTabListBox
    FixedImage          maSigsValidImg;
    FixedInfo           maSigsValidFI;
    FixedImage          maSigsInvalidImg;
    FixedInfo           maSigsInvalidFI;
    FixedImage          maSigsNotvalidatedImg;
    FixedInfo           maSigsNotvalidatedFI;
    FixedInfo           maSigsOldSignatureFI;

    PushButton          maViewBtn;
    PushButton          maAddBtn;
    PushButton          maRemoveBtn;

    FixedLine           maBottomSepFL;
    OKButton            maOKBtn;
    HelpButton          maHelpBtn;

    ::rtl::OUString m_sODFVersion;
    //Signals if the document contains already a document signature. This is only
    //importent when we are signing macros and if the value is true.
    bool m_bHasDocumentSignature;
    bool m_bWarningShowSignMacro;

    DECL_LINK(          ViewButtonHdl, Button* );
    DECL_LINK(          AddButtonHdl, Button* );
    DECL_LINK(          RemoveButtonHdl, Button* );
    DECL_LINK(          SignatureHighlightHdl, void* );
    DECL_LINK(          SignatureSelectHdl, void* );
    DECL_LINK(          StartVerifySignatureHdl, void* );
    DECL_LINK(          OKButtonHdl, void* );

    void                ImplGetSignatureInformations(bool bUseTempStream);
    void                ImplFillSignaturesBox();
    void                ImplShowSignaturesDetails();
    SignatureStreamHelper ImplOpenSignatureStream( sal_Int32 eStreamMode, bool bTempStream );

    //Checks if adding is allowed.
    //See the spec at specs/www/appwide/security/Electronic_Signatures_and_Security.sxw
    //(6.6.2)Behaviour with regard to ODF 1.2
    bool canAdd();
    bool canRemove();

    //Checks if a particular stream is a valid xml stream. Those are treated differently
    //when they are signed (c14n transformation)
    bool isXML(const ::rtl::OUString& rURI );
    bool canAddRemove();

public:
    DigitalSignaturesDialog( Window* pParent, cssu::Reference<
        cssu::XComponentContext >& rxCtx, DocumentSignatureMode eMode,
        sal_Bool bReadOnly, const ::rtl::OUString& sODFVersion, bool bHasDocumentSignature);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

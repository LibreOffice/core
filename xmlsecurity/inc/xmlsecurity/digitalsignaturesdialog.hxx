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

#ifndef INCLUDED_XMLSECURITY_INC_XMLSECURITY_DIGITALSIGNATURESDIALOG_HXX
#define INCLUDED_XMLSECURITY_INC_XMLSECURITY_DIGITALSIGNATURESDIALOG_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <svtools/simptabl.hxx>
#include <svtools/stdctrl.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <xmlsecurity/documentsignaturehelper.hxx>
#include <xmlsecurity/xmlsignaturehelper.hxx>

#include <vector>

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


class HeaderBar;

class DigitalSignaturesDialog : public ModalDialog
{
private:
    css::uno::Reference< css::uno::XComponentContext >& mxCtx;
    XMLSignatureHelper      maSignatureHelper;

    css::uno::Reference < css::embed::XStorage > mxStore;
    css::uno::Reference < css::io::XStream > mxSignatureStream;
    css::uno::Reference < css::io::XStream > mxTempSignatureStream;
    SignatureInformations   maCurrentSignatureInformations;
    bool                    mbVerifySignatures;
    bool                    mbSignaturesChanged;
    DocumentSignatureMode   meSignatureMode;
    css::uno::Sequence < css::uno::Sequence < css::beans::PropertyValue > > m_manifest;

    FixedText*          m_pHintDocFT;
    FixedText*          m_pHintBasicFT;
    FixedText*          m_pHintPackageFT;
    SvSimpleTable*     m_pSignaturesLB;
    FixedImage*         m_pSigsValidImg;
    FixedText*          m_pSigsValidFI;
    FixedImage*         m_pSigsInvalidImg;
    FixedText*          m_pSigsInvalidFI;
    FixedImage*         m_pSigsNotvalidatedImg;
    FixedText*          m_pSigsNotvalidatedFI;
    FixedImage*         m_pSigsOldSignatureImg;
    FixedText*          m_pSigsOldSignatureFI;

    PushButton*         m_pViewBtn;
    PushButton*         m_pAddBtn;
    PushButton*         m_pRemoveBtn;

    CloseButton*        m_pCloseBtn;

    OUString m_sODFVersion;
    //Signals if the document contains already a document signature. This is only
    //important when we are signing macros and if the value is true.
    bool m_bHasDocumentSignature;
    bool m_bWarningShowSignMacro;

    DECL_LINK(ViewButtonHdl, void *);
    DECL_LINK(AddButtonHdl, void *);
    DECL_LINK(RemoveButtonHdl, void *);
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
    bool isXML(const OUString& rURI );
    bool canAddRemove();

public:
    DigitalSignaturesDialog( vcl::Window* pParent, css::uno::Reference<
        css::uno::XComponentContext >& rxCtx, DocumentSignatureMode eMode,
        bool bReadOnly, const OUString& sODFVersion, bool bHasDocumentSignature);
    virtual ~DigitalSignaturesDialog();

    // Initialize the dialog and the security environment, returns TRUE on success
    bool    Init();

            // Set the storage which should be signed or verified
    void    SetStorage( const css::uno::Reference < css::embed::XStorage >& rxStore );
    void    SetSignatureStream( const css::uno::Reference < css::io::XStream >& rxStream );

                // Execute the dialog...
    short       Execute() SAL_OVERRIDE;

                // Did signatures change?
    bool    SignaturesChanged() const { return mbSignaturesChanged; }
};

#endif // INCLUDED_XMLSECURITY_INC_XMLSECURITY_DIGITALSIGNATURESDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

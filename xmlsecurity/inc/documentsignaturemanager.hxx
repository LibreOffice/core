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

#pragma once

#include "xmlsecuritydllapi.h"

#include <memory>

#include <svl/sigstruct.hxx>
#include "xmlsignaturehelper.hxx"
#include "documentsignaturehelper.hxx"

#include <com/sun/star/xml/crypto/XSEInitializer.hpp>

namespace com::sun::star
{
namespace beans
{
struct PropertyValue;
}
namespace embed
{
class XStorage;
}
namespace frame
{
class XModel;
}
namespace graphic
{
class XGraphic;
}
namespace uno
{
class XComponentContext;
}
}
class PDFSignatureHelper;

/// Manages signatures (addition, removal), used by DigitalSignaturesDialog.
class XMLSECURITY_DLLPUBLIC DocumentSignatureManager
{
private:
    css::uno::Reference<css::uno::XComponentContext> mxContext;
    css::uno::Reference<css::embed::XStorage> mxStore;
    XMLSignatureHelper maSignatureHelper;
    std::unique_ptr<PDFSignatureHelper> mpPDFSignatureHelper;
    SignatureInformations maCurrentSignatureInformations;
    DocumentSignatureMode const meSignatureMode;
    css::uno::Sequence<css::uno::Sequence<css::beans::PropertyValue>> m_manifest;
    css::uno::Reference<css::io::XStream> mxSignatureStream;
    css::uno::Reference<css::frame::XModel> mxModel;
    css::uno::Reference<css::io::XStream> mxTempSignatureStream;
    /// Storage containing all OOXML signatures, unused for ODF.
    css::uno::Reference<css::embed::XStorage> mxTempSignatureStorage;
    css::uno::Reference<css::xml::crypto::XSEInitializer> mxSEInitializer;
    css::uno::Reference<css::xml::crypto::XXMLSecurityContext> mxSecurityContext;
    css::uno::Reference<css::xml::crypto::XSEInitializer> mxGpgSEInitializer;
    css::uno::Reference<css::xml::crypto::XXMLSecurityContext> mxGpgSecurityContext;

public:
    DocumentSignatureManager(const css::uno::Reference<css::uno::XComponentContext>& xContext,
                             DocumentSignatureMode eMode);
    ~DocumentSignatureManager();

    /**
     * Checks if a particular stream is a valid xml stream. Those are treated
     * differently when they are signed (c14n transformation)
     */
    bool isXML(const OUString& rURI);
    bool readManifest();

    SignatureStreamHelper ImplOpenSignatureStream(sal_Int32 nStreamOpenMode, bool bTempStream);
    /// Add a new signature, using xCert as a signing certificate, and rDescription as description.
    bool add(const css::uno::Reference<css::security::XCertificate>& xCert,
             const css::uno::Reference<css::xml::crypto::XXMLSecurityContext>& xSecurityContext,
             const OUString& rDescription, sal_Int32& nSecurityId, bool bAdESCompliant,
             const OUString& rSignatureLineId = OUString(),
             const css::uno::Reference<css::graphic::XGraphic>& xValidGraphic
             = css::uno::Reference<css::graphic::XGraphic>(),
             const css::uno::Reference<css::graphic::XGraphic>& xInvalidGraphic
             = css::uno::Reference<css::graphic::XGraphic>());
    /// Remove signature at nPosition.
    void remove(sal_uInt16 nPosition);
    /// Read signatures from either a temp stream or the real storage.
    void read(bool bUseTempStream, bool bCacheLastSignature = true);
    /// Write signatures back to the persistent storage.
    void write(bool bXAdESCompliantIfODF);
    /// Lazy creation of PDF helper.
    PDFSignatureHelper& getPDFSignatureHelper();
#if 0
    // Checks if the document is a kind where it is relevant to distinguish between using XAdES or not
    bool IsXAdESRelevant();
#endif
    /// Attempts to initialize the platform-specific crypto.
    bool init();
    /// Get the security environment.
    css::uno::Reference<css::xml::crypto::XSecurityEnvironment> getSecurityEnvironment();
    css::uno::Reference<css::xml::crypto::XSecurityEnvironment> getGpgSecurityEnvironment();
    css::uno::Reference<css::xml::crypto::XXMLSecurityContext> const& getSecurityContext() const;
    css::uno::Reference<css::xml::crypto::XXMLSecurityContext> const& getGpgSecurityContext() const;
    void setStore(const css::uno::Reference<css::embed::XStorage>& xStore) { mxStore = xStore; }
    XMLSignatureHelper& getSignatureHelper() { return maSignatureHelper; }
    bool hasPDFSignatureHelper() const { return bool(mpPDFSignatureHelper); }
    void setSignatureStream(const css::uno::Reference<css::io::XStream>& xSignatureStream)
    {
        mxSignatureStream = xSignatureStream;
    }
    void setModel(const css::uno::Reference<css::frame::XModel>& xModel);
    const css::uno::Reference<css::embed::XStorage>& getStore() const { return mxStore; }
    DocumentSignatureMode getSignatureMode() const { return meSignatureMode; }
    SignatureInformations& getCurrentSignatureInformations()
    {
        return maCurrentSignatureInformations;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

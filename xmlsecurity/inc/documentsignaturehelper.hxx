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

#ifndef INCLUDED_XMLSECURITY_INC_DOCUMENTSIGNATUREHELPER_HXX
#define INCLUDED_XMLSECURITY_INC_DOCUMENTSIGNATUREHELPER_HXX

#include <com/sun/star/uno/Reference.h>
#include <rtl/ustring.hxx>
#include <svl/sigstruct.hxx>

#include <vector>

namespace com::sun::star {
    namespace io { class XStream; }
    namespace embed { class XStorage; }
}

namespace com::sun::star::xml::sax { class XDocumentHandler; }

enum class DocumentSignatureMode
{
    Content,
    Macros,
    Package
};

enum class DocumentSignatureAlgorithm
{
    OOo2,
    OOo3_0,
    OOo3_2
};

struct SignatureStreamHelper
{
    css::uno::Reference < css::embed::XStorage >    xSignatureStorage;
    css::uno::Reference < css::io::XStream >        xSignatureStream;
    /// If this is embed::StorageFormats::OFOPXML, then it's expected that xSignatureStream is an empty reference.
    sal_Int32 nStorageFormat;

    SignatureStreamHelper()
        : nStorageFormat(0)
    {
    }
};

namespace DocumentSignatureHelper
{
    SignatureStreamHelper OpenSignatureStream(
        const css::uno::Reference < css::embed::XStorage >& rxStore, sal_Int32 nOpenMode,
        DocumentSignatureMode eDocSigMode );

    std::vector< OUString > CreateElementList(
        const css::uno::Reference < css::embed::XStorage >& rxStore,
        DocumentSignatureMode eMode,
        const DocumentSignatureAlgorithm mode);

    bool isODFPre_1_2(const OUString & sODFVersion);
    bool isOOo3_2_Signature(const SignatureInformation & sigInfo);

    DocumentSignatureAlgorithm getDocumentAlgorithm(
        const OUString & sODFVersion, const SignatureInformation & sigInfo);

    bool CanSignWithGPG(const css::uno::Reference < css::embed::XStorage >& rxStore,
                        const OUString& sOdfVersion);

    bool checkIfAllFilesAreSigned( const ::std::vector< OUString > & sElementList,
        const SignatureInformation & sigInfo, const DocumentSignatureAlgorithm alg);

    bool equalsReferenceUriManifestPath(
        const OUString & rUri, const OUString & rPath);

    OUString GetDocumentContentSignatureDefaultStreamName();
    OUString GetScriptingContentSignatureDefaultStreamName();
    OUString GetPackageSignatureDefaultStreamName();

    /// In case the storage is OOXML, prepend a leading '/' and append content type to the element URIs.
    void AppendContentTypes(const css::uno::Reference<css::embed::XStorage>& xStorage, std::vector<OUString>& rElements);

    void writeDigestMethod(
        const css::uno::Reference<css::xml::sax::XDocumentHandler>& xDocumentHandler);
    void writeSignedProperties(
        const css::uno::Reference<css::xml::sax::XDocumentHandler>& xDocumentHandler,
        const SignatureInformation& signatureInfo,
        const OUString& sDate,
        const bool bWriteSignatureLineData);
};

#endif // INCLUDED_XMLSECURITY_INC_DOCUMENTSIGNATUREHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

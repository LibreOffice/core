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

#ifndef _XMLSECURITY_DOCUMENTSIGNATUREHELPER_HXX
#define _XMLSECURITY_DOCUMENTSIGNATUREHELPER_HXX

#include <com/sun/star/uno/Reference.h>
#include <rtl/ustring.hxx>
#include "xmlsecurity/sigstruct.hxx"

#ifndef _STLP_VECTOR
#include <vector>
#endif


namespace com {
namespace sun {
namespace star {
namespace io {
    class XStream; }
namespace embed {
    class XStorage; }
}}}


/**********************************************************
 DocumentSignatureHelper

 Helper class for signing and verifieng document signatures

 Functions:
 1. help to create a list of content to be signed/verified

 **********************************************************/

enum DocumentSignatureMode { SignatureModeDocumentContent, SignatureModeMacros, SignatureModePackage };

enum DocumentSignatureAlgorithm
{
    OOo2Document,
    OOo3_0Document,
    OOo3_2Document
};

struct SignatureStreamHelper
{
    css::uno::Reference < css::embed::XStorage >    xSignatureStorage;
    css::uno::Reference < css::io::XStream >        xSignatureStream;
};


class DocumentSignatureHelper
{
public:

    static SignatureStreamHelper OpenSignatureStream(
        const css::uno::Reference < css::embed::XStorage >& rxStore, sal_Int32 nOpenMode,
        DocumentSignatureMode eDocSigMode );
    static std::vector< OUString > CreateElementList(
        const css::uno::Reference < css::embed::XStorage >& rxStore,
        const OUString rRootStorageName, DocumentSignatureMode eMode,
        const DocumentSignatureAlgorithm mode);
    static bool isODFPre_1_2(const OUString & sODFVersion);
    static bool isOOo3_2_Signature(const SignatureInformation & sigInfo);
    static DocumentSignatureAlgorithm getDocumentAlgorithm(
        const OUString & sODFVersion, const SignatureInformation & sigInfo);
    static bool checkIfAllFilesAreSigned( const ::std::vector< OUString > & sElementList,
        const SignatureInformation & sigInfo, const DocumentSignatureAlgorithm alg);
    static bool equalsReferenceUriManifestPath(
        const OUString & rUri, const OUString & rPath);
    static OUString GetDocumentContentSignatureDefaultStreamName();
    static OUString GetScriptingContentSignatureDefaultStreamName();
    static OUString GetPackageSignatureDefaultStreamName();

};

#endif // _XMLSECURITY_XMLSIGNATUREHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

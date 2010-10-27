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

namespace css = com::sun::star;


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
    static std::vector< rtl::OUString > CreateElementList(
        const css::uno::Reference < css::embed::XStorage >& rxStore,
        const ::rtl::OUString rRootStorageName, DocumentSignatureMode eMode,
        const DocumentSignatureAlgorithm mode);
    static bool isODFPre_1_2(const ::rtl::OUString & sODFVersion);
    static bool isOOo3_2_Signature(const SignatureInformation & sigInfo);
    static DocumentSignatureAlgorithm getDocumentAlgorithm(
        const ::rtl::OUString & sODFVersion, const SignatureInformation & sigInfo);
    static bool checkIfAllFilesAreSigned( const ::std::vector< ::rtl::OUString > & sElementList,
        const SignatureInformation & sigInfo, const DocumentSignatureAlgorithm alg);
    static bool equalsReferenceUriManifestPath(
        const ::rtl::OUString & rUri, const ::rtl::OUString & rPath);
    static ::rtl::OUString GetDocumentContentSignatureDefaultStreamName();
    static ::rtl::OUString GetScriptingContentSignatureDefaultStreamName();
    static ::rtl::OUString GetPackageSignatureDefaultStreamName();

};

#endif // _XMLSECURITY_XMLSIGNATUREHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

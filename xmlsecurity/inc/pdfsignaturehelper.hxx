/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_XMLSECURITY_INC_PDFSIGNATUREHELPER_HXX
#define INCLUDED_XMLSECURITY_INC_PDFSIGNATUREHELPER_HXX

#include <xmlsecuritydllapi.h>

#include <vector>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/security/DocumentSignatureInformation.hpp>

/// Handles signatures of a PDF file.
class XMLSECURITY_DLLPUBLIC PDFSignatureHelper
{
    std::vector<css::security::DocumentSignatureInformation> m_aSignatureInfos;

public:
    bool ReadAndVerifySignature(const css::uno::Reference<css::io::XInputStream>& xInputStream);
    css::uno::Sequence<css::security::DocumentSignatureInformation> GetDocumentSignatureInformations();
};

#endif // INCLUDED_XMLSECURITY_INC_PDFSIGNATUREHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

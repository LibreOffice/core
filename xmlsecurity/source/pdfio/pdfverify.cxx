/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <pdfio/pdfdocument.hxx>

#include <sal/main.h>
#include <osl/file.hxx>
#include <iostream>

using namespace com::sun::star;

SAL_IMPLEMENT_MAIN_WITH_ARGS(nArgc, pArgv)
{
    if (nArgc < 2)
    {
        SAL_WARN("xmlsecurity.pdfio", "not enough parameters");
        return 1;
    }

    OUString aURL;
    osl::FileBase::getFileURLFromSystemPath(OUString::fromUtf8(pArgv[1]), aURL);

    SvFileStream aStream(aURL, StreamMode::READ);
    xmlsecurity::pdfio::PDFDocument aDocument;
    if (!aDocument.Read(aStream))
    {
        SAL_WARN("xmlsecurity.pdfio", "failed to read the document");
        return 1;
    }

    std::vector<xmlsecurity::pdfio::PDFObjectElement*> aSignatures = aDocument.GetSignatureWidgets();
    if (aSignatures.empty())
        std::cerr << "found no signatures" << std::endl;
    else
    {
        std::cerr << "found " << aSignatures.size() << " signatures" << std::endl;
        for (size_t i = 0; i < aSignatures.size(); ++i)
        {
            SignatureInformation aInfo(i);
            if (!xmlsecurity::pdfio::PDFDocument::ValidateSignature(aStream, aSignatures[i], aInfo))
            {
                SAL_WARN("xmlsecurity.pdfio", "failed to determine digest match");
                return 1;
            }

            bool bSuccess = aInfo.nStatus == xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED;
            std::cerr << "signature #" << i << ": digest match? " << bSuccess << std::endl;
        }
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

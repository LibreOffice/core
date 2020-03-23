/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/signaturestate.hxx>

#include <com/sun/star/security/CertificateValidity.hpp>
#include <com/sun/star/security/DocumentSignatureInformation.hpp>

using namespace css;

namespace DocumentSignatures
{
SignatureState
getSignatureState(const uno::Sequence<security::DocumentSignatureInformation>& aSigInfo)
{
    bool bCertValid = true;
    SignatureState nResult = SignatureState::NOSIGNATURES;
    bool bCompleteSignature = true;
    if (!aSigInfo.hasElements())
        return nResult;

    nResult = SignatureState::OK;
    for (const auto& rInfo : aSigInfo)
    {
        if (bCertValid)
        {
            sal_Int32 nCertStat = rInfo.CertificateStatus;
            bCertValid = nCertStat == security::CertificateValidity::VALID;
        }

        if (!rInfo.SignatureIsValid)
        {
            nResult = SignatureState::BROKEN;
            break;
        }
        bCompleteSignature &= !rInfo.PartialDocumentSignature;
    }

    if (nResult == SignatureState::OK && !bCertValid && !bCompleteSignature)
        nResult = SignatureState::NOTVALIDATED_PARTIAL_OK;
    else if (nResult == SignatureState::OK && !bCertValid)
        nResult = SignatureState::NOTVALIDATED;
    else if (nResult == SignatureState::OK && bCertValid && !bCompleteSignature)
        nResult = SignatureState::PARTIAL_OK;

    // this code must not check whether the document is modified
    // it should only check the provided info

    return nResult;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

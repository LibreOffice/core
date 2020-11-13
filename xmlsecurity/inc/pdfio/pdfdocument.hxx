/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_XMLSECURITY_INC_PDFIO_PDFDOCUMENT_HXX
#define INCLUDED_XMLSECURITY_INC_PDFIO_PDFDOCUMENT_HXX

#include <xmlsecuritydllapi.h>

namespace vcl
{
namespace filter
{
class PDFObjectElement;
class PDFDocument;
}
}
struct SignatureInformation;
class SvStream;

namespace xmlsecurity
{
namespace pdfio
{
/**
 * @param rInformation The actual result.
 * @param rDocument the parsed document to see if the signature is partial.
 * @return If we can determinate a result.
 */
XMLSECURITY_DLLPUBLIC bool ValidateSignature(SvStream& rStream,
                                             vcl::filter::PDFObjectElement* pSignature,
                                             SignatureInformation& rInformation,
                                             vcl::filter::PDFDocument& rDocument, int nMDPPerm);

} // namespace pdfio
} // namespace xmlsecurity

#endif // INCLUDED_XMLSECURITY_INC_PDFIO_PDFDOCUMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

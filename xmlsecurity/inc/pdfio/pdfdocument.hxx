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

#include <vcl/filter/pdfdocument.hxx>

#include <tools/stream.hxx>

#include <xmlsecuritydllapi.h>

struct SignatureInformation;

namespace xmlsecurity
{
namespace pdfio
{

/**
 * @param rInformation The actual result.
 * @param bLast If this is the last signature in the file, so it covers the whole file physically.
 * @return If we can determinate a result.
 */
XMLSECURITY_DLLPUBLIC bool ValidateSignature(SvStream& rStream, vcl::filter::PDFObjectElement* pSignature, SignatureInformation& rInformation, bool bLast);

} // namespace pdfio
} // namespace xmlsecurity

#endif // INCLUDED_XMLSECURITY_INC_PDFIO_PDFDOCUMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

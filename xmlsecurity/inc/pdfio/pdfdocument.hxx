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

#include <vector>

#include <com/sun/star/security/XCertificate.hpp>

#include <tools/stream.hxx>

#include <xmlsecuritydllapi.h>
#include <sigstruct.hxx>

namespace xmlsecurity
{
namespace pdfio
{

class PDFTrailerElement;
class PDFObjectElement;
class PDFHexStringElement;

/// A byte range in a PDF file.
class PDFElement
{
public:
    virtual bool Read(SvStream& rStream) = 0;
    virtual ~PDFElement() { }
};

/// In-memory representation of an on-disk PDF document.
class XMLSECURITY_DLLPUBLIC PDFDocument
{
    /// This vector owns all elements.
    std::vector< std::unique_ptr<PDFElement> > m_aElements;
    // List of object offsets we know.
    std::vector<size_t> m_aXRef;
    PDFTrailerElement* m_pTrailer;
    /// All editing takes place in this buffer, if it happens.
    SvMemoryStream m_aEditBuffer;

    static int AsHex(char ch);
    /// Decode a hex dump.
    static std::vector<unsigned char> DecodeHexString(PDFHexStringElement* pElement);

public:
    PDFDocument();
    PDFDocument& operator=(const PDFDocument&) = delete;
    PDFDocument(const PDFDocument&) = delete;
    static OString ReadKeyword(SvStream& rStream);
    static size_t FindStartXRef(SvStream& rStream);
    void ReadXRef(SvStream& rStream);
    static void SkipWhitespace(SvStream& rStream);
    size_t GetObjectOffset(size_t nIndex) const;
    const std::vector< std::unique_ptr<PDFElement> >& GetElements();
    std::vector<PDFObjectElement*> GetPages();

    bool Read(SvStream& rStream);
    /// Sign the read document with xCertificate in the edit buffer.
    bool Sign(const css::uno::Reference<css::security::XCertificate>& xCertificate, const OUString& rDescription);
    /// Serializes the contents of the edit buffer.
    bool Write(SvStream& rStream);
    std::vector<PDFObjectElement*> GetSignatureWidgets();
    /// Return value is about if we can determine a result, rInformation is about the actual result.
    static bool ValidateSignature(SvStream& rStream, PDFObjectElement* pSignature, SignatureInformation& rInformation);
};

} // namespace pdfio
} // namespace xmlsecurity

#endif // INCLUDED_XMLSECURITY_INC_PDFIO_PDFDOCUMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

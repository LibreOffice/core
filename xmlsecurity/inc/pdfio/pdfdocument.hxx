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

#include <tools/stream.hxx>

#include <xmlsecuritydllapi.h>

namespace xmlsecurity
{
namespace pdfio
{

class PDFTrailerElement;
class PDFObjectElement;

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

    static int AsHex(char ch);

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
    std::vector<PDFObjectElement*> GetSignatureWidgets();
    /// Return value is about if we can determine a result, bDigestMatch is about the actual result.
    static bool ValidateSignature(SvStream& rStream, PDFObjectElement* pSignature, bool& bDigestMatch);
};

} // namespace pdfio
} // namespace xmlsecurity

#endif // INCLUDED_XMLSECURITY_INC_PDFIO_PDFDOCUMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

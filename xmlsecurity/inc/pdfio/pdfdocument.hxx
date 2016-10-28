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

#include <map>
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

enum class TokenizeMode
{
    /// Full file.
    END_OF_STREAM,
    /// Till the first %%EOF token.
    EOF_TOKEN,
    /// Till the end of the current object.
    END_OF_OBJECT
};

/**
 * In-memory representation of an on-disk PDF document.
 *
 * The PDF element list is not meant to be saved back to disk, but some
 * elements remember their source offset / length, and based on that it's
 * possible to modify the input file.
 */
class XMLSECURITY_DLLPUBLIC PDFDocument
{
    /// This vector owns all elements.
    std::vector< std::unique_ptr<PDFElement> > m_aElements;
    /// Object ID <-> object offset map.
    std::map<size_t, size_t> m_aXRef;
    /// Object ID <-> "are changed as part of an incremental update?" map.
    std::map<size_t, bool> m_aXRefDirty;
    /// List of xref offsets we know.
    std::vector<size_t> m_aStartXRefs;
    /// List of EOF offsets we know.
    std::vector<size_t> m_aEOFs;
    PDFTrailerElement* m_pTrailer;
    /// All editing takes place in this buffer, if it happens.
    SvMemoryStream m_aEditBuffer;

    static int AsHex(char ch);
    /// Decode a hex dump.
    static std::vector<unsigned char> DecodeHexString(PDFHexStringElement* pElement);
    /// Tokenize elements from current offset.
    bool Tokenize(SvStream& rStream, TokenizeMode eMode);

public:
    PDFDocument();
    PDFDocument& operator=(const PDFDocument&) = delete;
    PDFDocument(const PDFDocument&) = delete;
    static OString ReadKeyword(SvStream& rStream);
    static size_t FindStartXRef(SvStream& rStream);
    void ReadXRef(SvStream& rStream);
    void ReadXRefStream(SvStream& rStream);
    static void SkipWhitespace(SvStream& rStream);
    /// Instead of all whitespace, just skip CR and NL characters.
    static void SkipLineBreaks(SvStream& rStream);
    size_t GetObjectOffset(size_t nIndex) const;
    const std::vector< std::unique_ptr<PDFElement> >& GetElements();
    std::vector<PDFObjectElement*> GetPages();
    /// Remember the end location of an EOF token.
    void PushBackEOF(size_t nOffset);

    /// Read elements from the start of the stream till its end.
    bool Read(SvStream& rStream);
    /// Sign the read document with xCertificate in the edit buffer.
    bool Sign(const css::uno::Reference<css::security::XCertificate>& xCertificate, const OUString& rDescription);
    /// Serializes the contents of the edit buffer.
    bool Write(SvStream& rStream);
    std::vector<PDFObjectElement*> GetSignatureWidgets();
    /**
     * @param rInformation The actual result.
     * @param bLast If this is the last signature in the file, so it covers the whole file physically.
     * @return If we can determinate a result.
     */
    static bool ValidateSignature(SvStream& rStream, PDFObjectElement* pSignature, SignatureInformation& rInformation, bool bLast);
    /// Remove the nth signature from read document in the edit buffer.
    bool RemoveSignature(size_t nPosition);
};

} // namespace pdfio
} // namespace xmlsecurity

#endif // INCLUDED_XMLSECURITY_INC_PDFIO_PDFDOCUMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

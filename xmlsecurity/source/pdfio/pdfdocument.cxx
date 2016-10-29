/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <pdfio/pdfdocument.hxx>

#include <map>
#include <memory>
#include <vector>

#include <com/sun/star/uno/Sequence.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/string.hxx>
#include <filter/msfilter/mscodec.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <sal/log.hxx>
#include <sal/types.h>
#include <sax/tools/converter.hxx>
#include <tools/zcodec.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/datetime.hxx>
#include <vcl/pdfwriter.hxx>
#include <xmloff/xmluconv.hxx>

#ifdef XMLSEC_CRYPTO_NSS
#include <cert.h>
#include <cms.h>
#include <nss.h>
#include <sechash.h>
#endif

using namespace com::sun::star;

namespace xmlsecurity
{
namespace pdfio
{

class PDFTrailerElement;
class PDFObjectElement;

/// A one-liner comment.
class PDFCommentElement : public PDFElement
{
    PDFDocument& m_rDoc;
    OString m_aComment;

public:
    PDFCommentElement(PDFDocument& rDoc);
    bool Read(SvStream& rStream) override;
};

/// Numbering object: an integer or a real.
class PDFNumberElement : public PDFElement
{
    /// Input file start location.
    sal_uInt64 m_nOffset;
    /// Input file token length.
    sal_uInt64 m_nLength;
    double m_fValue;

public:
    PDFNumberElement();
    bool Read(SvStream& rStream) override;
    double GetValue() const;
    sal_uInt64 GetLocation() const;
    sal_uInt64 GetLength() const;
};

class PDFReferenceElement;
class PDFDictionaryElement;
class PDFArrayElement;

/// Indirect object: something with a unique ID.
class PDFObjectElement : public PDFElement
{
    PDFDocument& m_rDoc;
    double m_fObjectValue;
    double m_fGenerationValue;
    std::map<OString, PDFElement*> m_aDictionary;
    /// Position after the '<<' token.
    sal_uInt64 m_nDictionaryOffset;
    /// Length of the dictionary buffer till (before) the '<<' token.
    sal_uInt64 m_nDictionaryLength;
    PDFDictionaryElement* m_pDictionaryElement;
    /// The contained direct array, if any.
    PDFArrayElement* m_pArrayElement;

public:
    PDFObjectElement(PDFDocument& rDoc, double fObjectValue, double fGenerationValue);
    bool Read(SvStream& rStream) override;
    PDFElement* Lookup(const OString& rDictionaryKey);
    PDFObjectElement* LookupObject(const OString& rDictionaryKey);
    double GetObjectValue() const;
    void SetDictionaryOffset(sal_uInt64 nDictionaryOffset);
    sal_uInt64 GetDictionaryOffset();
    void SetDictionaryLength(sal_uInt64 nDictionaryLength);
    sal_uInt64 GetDictionaryLength();
    PDFDictionaryElement* GetDictionary() const;
    void SetDictionary(PDFDictionaryElement* pDictionaryElement);
    void SetArray(PDFArrayElement* pArrayElement);
    PDFArrayElement* GetArray() const;
};

/// Dictionary object: a set key-value pairs.
class PDFDictionaryElement : public PDFElement
{
    /// Key-value pairs when the dictionary is a nested value.
    std::map<OString, PDFElement*> m_aItems;
    /// Offset after the '<<' token.
    sal_uInt64 m_nLocation;
    /// Position after the '/' token.
    std::map<OString, sal_uInt64> m_aDictionaryKeyOffset;
    /// Length of the dictionary key and value, till (before) the next token.
    std::map<OString, sal_uInt64> m_aDictionaryKeyValueLength;

public:
    PDFDictionaryElement();
    bool Read(SvStream& rStream) override;

    static size_t Parse(const std::vector< std::unique_ptr<PDFElement> >& rElements, PDFElement* pThis, std::map<OString, PDFElement*>& rDictionary);
    static PDFElement* Lookup(const std::map<OString, PDFElement*>& rDictionary, const OString& rKey);
    void SetKeyOffset(const OString& rKey, sal_uInt64 nOffset);
    sal_uInt64 GetKeyOffset(const OString& rKey) const;
    void SetKeyValueLength(const OString& rKey, sal_uInt64 nLength);
    sal_uInt64 GetKeyValueLength(const OString& rKey) const;
    const std::map<OString, PDFElement*>& GetItems() const;
};

/// End of a dictionary: '>>'.
class PDFEndDictionaryElement : public PDFElement
{
    /// Offset before the '>>' token.
    sal_uInt64 m_nLocation;
public:
    PDFEndDictionaryElement();
    bool Read(SvStream& rStream) override;
    sal_uInt64 GetLocation() const;
};

/// Name object: a key string.
class PDFNameElement : public PDFElement
{
    OString m_aValue;
    /// Offset after the '/' token.
    sal_uInt64 m_nLocation;
    /// Length till the next token start.
    sal_uInt64 m_nLength;
public:
    PDFNameElement();
    bool Read(SvStream& rStream) override;
    const OString& GetValue() const;
    sal_uInt64 GetLocation() const;
    sal_uInt64 GetLength() const;
};

/// Reference object: something with a unique ID.
class PDFReferenceElement : public PDFElement
{
    PDFDocument& m_rDoc;
    int m_fObjectValue;
    int m_fGenerationValue;

public:
    PDFReferenceElement(PDFDocument& rDoc, int fObjectValue, int fGenerationValue);
    bool Read(SvStream& rStream) override;
    /// Assuming the reference points to a number object, return its value.
    double LookupNumber(SvStream& rStream) const;
    /// Lookup referenced object, without assuming anything about its contents.
    PDFObjectElement* LookupObject() const;
    int GetObjectValue() const;
    int GetGenerationValue() const;
};

/// Stream object: a byte array with a known length.
class PDFStreamElement : public PDFElement
{
    size_t m_nLength;
    sal_uInt64 m_nOffset;

public:
    PDFStreamElement(size_t nLength);
    bool Read(SvStream& rStream) override;
    sal_uInt64 GetOffset() const;
};

/// End of a stream: 'endstream' keyword.
class PDFEndStreamElement : public PDFElement
{
public:
    bool Read(SvStream& rStream) override;
};

/// End of a object: 'endobj' keyword.
class PDFEndObjectElement : public PDFElement
{
public:
    bool Read(SvStream& rStream) override;
};

/// Array object: a list.
class PDFArrayElement : public PDFElement
{
    /// Location after the '[' token.
    sal_uInt64 m_nOffset;
    std::vector<PDFElement*> m_aElements;
public:
    PDFArrayElement();
    bool Read(SvStream& rStream) override;
    void PushBack(PDFElement* pElement);
    const std::vector<PDFElement*>& GetElements();
};

/// End of an array: ']'.
class PDFEndArrayElement : public PDFElement
{
    /// Location before the ']' token.
    sal_uInt64 m_nOffset;
public:
    PDFEndArrayElement();
    bool Read(SvStream& rStream) override;
    sal_uInt64 GetOffset() const;
};

/// Boolean object: a 'true' or a 'false'.
class PDFBooleanElement : public PDFElement
{
public:
    PDFBooleanElement(bool bValue);
    bool Read(SvStream& rStream) override;
};

/// Null object: the 'null' singleton.
class PDFNullElement : public PDFElement
{
public:
    bool Read(SvStream& rStream) override;
};

/// Hex string: in <AABB> form.
class PDFHexStringElement : public PDFElement
{
    OString m_aValue;
public:
    bool Read(SvStream& rStream) override;
    const OString& GetValue() const;
};

/// Literal string: in (asdf) form.
class PDFLiteralStringElement : public PDFElement
{
    OString m_aValue;
public:
    bool Read(SvStream& rStream) override;
    const OString& GetValue() const;
};

/// The trailer singleton is at the end of the doc.
class PDFTrailerElement : public PDFElement
{
    PDFDocument& m_rDoc;
    std::map<OString, PDFElement*> m_aDictionary;

public:
    PDFTrailerElement(PDFDocument& rDoc);
    bool Read(SvStream& rStream) override;
    PDFElement* Lookup(const OString& rDictionaryKey);
};

PDFDocument::PDFDocument()
    : m_pTrailer(nullptr),
      m_pXRefStream(nullptr)
{
}

bool PDFDocument::RemoveSignature(size_t nPosition)
{
    std::vector<PDFObjectElement*> aSignatures = GetSignatureWidgets();
    if (nPosition >= aSignatures.size())
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::RemoveSignature: invalid nPosition");
        return false;
    }

    if (aSignatures.size() != m_aEOFs.size() - 1)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::RemoveSignature: no 1:1 mapping between signatures and incremental updates");
        return false;
    }

    // The EOF offset is the end of the original file, without the signature at
    // nPosition.
    m_aEditBuffer.Seek(m_aEOFs[nPosition]);
    // Drop all bytes after the current position.
    m_aEditBuffer.SetStreamSize(m_aEditBuffer.Tell() + 1);

    return m_aEditBuffer.good();
}

bool PDFDocument::Sign(const uno::Reference<security::XCertificate>& xCertificate, const OUString& rDescription)
{
    // Decide what identifier to use for the new signature.
    std::vector<PDFObjectElement*> aSignatures = GetSignatureWidgets();
    sal_uInt32 nNextSignature = aSignatures.size() + 1;

    m_aEditBuffer.WriteCharPtr("\n");

    // Write signature object.
    sal_Int32 nSignatureId = m_aXRef.size();
    sal_uInt64 nSignatureOffset = m_aEditBuffer.Tell();
    m_aXRef[nSignatureId] = nSignatureOffset;
    m_aXRefDirty[nSignatureId] = true;
    OStringBuffer aSigBuffer;
    aSigBuffer.append(nSignatureId);
    aSigBuffer.append(" 0 obj\n");
    aSigBuffer.append("<</Contents <");
    sal_Int64 nSignatureContentOffset = nSignatureOffset + aSigBuffer.getLength();
    // Reserve space for the PKCS#7 object.
    const int MAX_SIGNATURE_CONTENT_LENGTH = 50000;
    OStringBuffer aContentFiller(MAX_SIGNATURE_CONTENT_LENGTH);
    comphelper::string::padToLength(aContentFiller, MAX_SIGNATURE_CONTENT_LENGTH, '0');
    aSigBuffer.append(aContentFiller.makeStringAndClear());
    aSigBuffer.append(">\n/Type/Sig/SubFilter/adbe.pkcs7.detached");
    // Byte range: we can write offset1-length1 and offset2 right now, will
    // write length2 later.
    aSigBuffer.append(" /ByteRange [ 0 ");
    // -1 and +1 is the leading "<" and the trailing ">" around the hex string.
    aSigBuffer.append(nSignatureContentOffset - 1);
    aSigBuffer.append(" ");
    aSigBuffer.append(nSignatureContentOffset + MAX_SIGNATURE_CONTENT_LENGTH + 1);
    aSigBuffer.append(" ");
    sal_uInt64 nSignatureLastByteRangeOffset = nSignatureOffset + aSigBuffer.getLength();
    // We don't know how many bytes we need for the last ByteRange value, this
    // should be enough.
    OStringBuffer aByteRangeFiller;
    comphelper::string::padToLength(aByteRangeFiller, 100, ' ');
    aSigBuffer.append(aByteRangeFiller.makeStringAndClear());
    // Finish the Sig obj.
    aSigBuffer.append(" /Filter/Adobe.PPKMS");

    if (!rDescription.isEmpty())
    {
        aSigBuffer.append("/Reason<");
        vcl::PDFWriter::AppendUnicodeTextString(rDescription, aSigBuffer);
        aSigBuffer.append(">");
    }

    aSigBuffer.append(" >>\nendobj\n\n");
    m_aEditBuffer.WriteOString(aSigBuffer.toString());

    // Write appearance object.
    sal_Int32 nAppearanceId = m_aXRef.size();
    m_aXRef[nAppearanceId] = m_aEditBuffer.Tell();
    m_aXRefDirty[nAppearanceId] = true;
    m_aEditBuffer.WriteUInt32AsString(nAppearanceId);
    m_aEditBuffer.WriteCharPtr(" 0 obj\n");
    m_aEditBuffer.WriteCharPtr("<</Type/XObject\n/Subtype/Form\n");
    m_aEditBuffer.WriteCharPtr("/BBox[0 0 0 0]\n/Length 0\n>>\n");
    m_aEditBuffer.WriteCharPtr("stream\n\nendstream\nendobj\n\n");

    // Write the Annot object, references nSignatureId and nAppearanceId.
    sal_Int32 nAnnotId = m_aXRef.size();
    m_aXRef[nAnnotId] = m_aEditBuffer.Tell();
    m_aXRefDirty[nAnnotId] = true;
    m_aEditBuffer.WriteUInt32AsString(nAnnotId);
    m_aEditBuffer.WriteCharPtr(" 0 obj\n");
    m_aEditBuffer.WriteCharPtr("<</Type/Annot/Subtype/Widget/F 132\n");
    m_aEditBuffer.WriteCharPtr("/Rect[0 0 0 0]\n");
    m_aEditBuffer.WriteCharPtr("/FT/Sig\n");
    std::vector<PDFObjectElement*> aPages = GetPages();
    if (aPages.empty())
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::Sign: found no pages");
        return false;
    }
    PDFObjectElement* pFirstPage = aPages[0];
    m_aEditBuffer.WriteCharPtr("/P ");
    m_aEditBuffer.WriteUInt32AsString(pFirstPage->GetObjectValue());
    m_aEditBuffer.WriteCharPtr(" 0 R\n");
    m_aEditBuffer.WriteCharPtr("/T(Signature");
    m_aEditBuffer.WriteUInt32AsString(nNextSignature);
    m_aEditBuffer.WriteCharPtr(")\n");
    m_aEditBuffer.WriteCharPtr("/V ");
    m_aEditBuffer.WriteUInt32AsString(nSignatureId);
    m_aEditBuffer.WriteCharPtr(" 0 R\n");
    m_aEditBuffer.WriteCharPtr("/DV ");
    m_aEditBuffer.WriteUInt32AsString(nSignatureId);
    m_aEditBuffer.WriteCharPtr(" 0 R\n");
    m_aEditBuffer.WriteCharPtr("/AP<<\n/N ");
    m_aEditBuffer.WriteUInt32AsString(nAppearanceId);
    m_aEditBuffer.WriteCharPtr(" 0 R\n>>\n");
    m_aEditBuffer.WriteCharPtr(">>\nendobj\n\n");

    // Write the updated first page object, references nAnnotId.
    sal_uInt32 nFirstPageId = pFirstPage->GetObjectValue();
    if (nFirstPageId >= m_aXRef.size())
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::Sign: invalid first page obj id");
        return false;
    }
    m_aXRef[nFirstPageId] = m_aEditBuffer.Tell();
    m_aXRefDirty[nFirstPageId] = true;
    m_aEditBuffer.WriteUInt32AsString(nFirstPageId);
    m_aEditBuffer.WriteCharPtr(" 0 obj\n");
    m_aEditBuffer.WriteCharPtr("<<");
    auto pAnnots = dynamic_cast<PDFArrayElement*>(pFirstPage->Lookup("Annots"));
    if (!pAnnots)
    {
        // No Annots key, just write the key with a single reference.
        m_aEditBuffer.WriteBytes(static_cast<const char*>(m_aEditBuffer.GetData()) + pFirstPage->GetDictionaryOffset(), pFirstPage->GetDictionaryLength());
        m_aEditBuffer.WriteCharPtr("/Annots[");
        m_aEditBuffer.WriteUInt32AsString(nAnnotId);
        m_aEditBuffer.WriteCharPtr(" 0 R]");
    }
    else
    {
        // Annots key is already there, insert our reference at the end.
        PDFDictionaryElement* pDictionary = pFirstPage->GetDictionary();

        // Offset right before the end of the Annots array.
        sal_uInt64 nAnnotsEndOffset = pDictionary->GetKeyOffset("Annots") + pDictionary->GetKeyValueLength("Annots") - 1;
        // Length of beginning of the dictionary -> Annots end.
        sal_uInt64 nAnnotsBeforeEndLength = nAnnotsEndOffset - pFirstPage->GetDictionaryOffset();
        m_aEditBuffer.WriteBytes(static_cast<const char*>(m_aEditBuffer.GetData()) + pFirstPage->GetDictionaryOffset(), nAnnotsBeforeEndLength);
        m_aEditBuffer.WriteCharPtr(" ");
        m_aEditBuffer.WriteUInt32AsString(nAnnotId);
        m_aEditBuffer.WriteCharPtr(" 0 R");
        // Length of Annots end -> end of the dictionary.
        sal_uInt64 nAnnotsAfterEndLength = pFirstPage->GetDictionaryOffset() + pFirstPage->GetDictionaryLength() - nAnnotsEndOffset;
        m_aEditBuffer.WriteBytes(static_cast<const char*>(m_aEditBuffer.GetData()) + nAnnotsEndOffset, nAnnotsAfterEndLength);
    }
    m_aEditBuffer.WriteCharPtr(">>");
    m_aEditBuffer.WriteCharPtr("\nendobj\n\n");

    // Write the updated Catalog object, references nAnnotId.
    auto pRoot = dynamic_cast<PDFReferenceElement*>(m_pTrailer->Lookup("Root"));
    if (!pRoot)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::Sign: trailer has no root reference");
        return false;
    }
    PDFObjectElement* pCatalog = pRoot->LookupObject();
    if (!pCatalog)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::Sign: invalid catalog reference");
        return false;
    }
    sal_uInt32 nCatalogId = pCatalog->GetObjectValue();
    if (nCatalogId >= m_aXRef.size())
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::Sign: invalid catalog obj id");
        return false;
    }
    m_aXRef[nCatalogId] = m_aEditBuffer.Tell();
    m_aXRefDirty[nCatalogId] = true;
    m_aEditBuffer.WriteUInt32AsString(nCatalogId);
    m_aEditBuffer.WriteCharPtr(" 0 obj\n");
    m_aEditBuffer.WriteCharPtr("<<");
    auto pAcroForm = dynamic_cast<PDFDictionaryElement*>(pCatalog->Lookup("AcroForm"));
    if (!pAcroForm)
    {
        // No AcroForm key, assume no signatures.
        m_aEditBuffer.WriteBytes(static_cast<const char*>(m_aEditBuffer.GetData()) + pCatalog->GetDictionaryOffset(), pCatalog->GetDictionaryLength());
        m_aEditBuffer.WriteCharPtr("/AcroForm<</Fields[\n");
        m_aEditBuffer.WriteUInt32AsString(nAnnotId);
        m_aEditBuffer.WriteCharPtr(" 0 R\n]/SigFlags 3>>\n");
    }
    else
    {
        // AcroForm key is already there, insert our reference at the Fields end.
        auto it = pAcroForm->GetItems().find("Fields");
        if (it == pAcroForm->GetItems().end())
        {
            SAL_WARN("xmlsecurity.pdfio", "PDFDocument::Sign: AcroForm without required Fields key");
            return false;
        }

        auto pFields = dynamic_cast<PDFArrayElement*>(it->second);
        if (!pFields)
        {
            SAL_WARN("xmlsecurity.pdfio", "PDFDocument::Sign: AcroForm Fields is not an array");
            return false;
        }

        // Offset right before the end of the Fields array.
        sal_uInt64 nFieldsEndOffset = pAcroForm->GetKeyOffset("Fields") + pAcroForm->GetKeyValueLength("Fields") - 1;
        // Length of beginning of the Catalog dictionary -> Fields end.
        sal_uInt64 nFieldsBeforeEndLength = nFieldsEndOffset - pCatalog->GetDictionaryOffset();
        m_aEditBuffer.WriteBytes(static_cast<const char*>(m_aEditBuffer.GetData()) + pCatalog->GetDictionaryOffset(), nFieldsBeforeEndLength);
        m_aEditBuffer.WriteCharPtr(" ");
        m_aEditBuffer.WriteUInt32AsString(nAnnotId);
        m_aEditBuffer.WriteCharPtr(" 0 R");
        // Length of Fields end -> end of the Catalog dictionary.
        sal_uInt64 nFieldsAfterEndLength = pCatalog->GetDictionaryOffset() + pCatalog->GetDictionaryLength() - nFieldsEndOffset;
        m_aEditBuffer.WriteBytes(static_cast<const char*>(m_aEditBuffer.GetData()) + nFieldsEndOffset, nFieldsAfterEndLength);
    }
    m_aEditBuffer.WriteCharPtr(">>\nendobj\n\n");

    // Write the xref table.
    sal_uInt64 nXRefOffset = m_aEditBuffer.Tell();
    m_aEditBuffer.WriteCharPtr("xref\n");
    for (const auto& rXRef : m_aXRef)
    {
        size_t nObject = rXRef.first;
        size_t nOffset = rXRef.second;
        if (!m_aXRefDirty[nObject])
            continue;

        m_aEditBuffer.WriteUInt32AsString(nObject);
        m_aEditBuffer.WriteCharPtr(" 1\n");
        OStringBuffer aBuffer;
        aBuffer.append(static_cast<sal_Int32>(nOffset));
        while (aBuffer.getLength() < 10)
            aBuffer.insert(0, "0");
        if (nObject == 0)
            aBuffer.append(" 65535 f \n");
        else
            aBuffer.append(" 00000 n \n");
        m_aEditBuffer.WriteOString(aBuffer.toString());
    }

    // Write the trailer.
    m_aEditBuffer.WriteCharPtr("trailer\n<</Size ");
    m_aEditBuffer.WriteUInt32AsString(m_aXRef.size());
    m_aEditBuffer.WriteCharPtr("/Root ");
    m_aEditBuffer.WriteUInt32AsString(pRoot->GetObjectValue());
    m_aEditBuffer.WriteCharPtr(" ");
    m_aEditBuffer.WriteUInt32AsString(pRoot->GetGenerationValue());
    m_aEditBuffer.WriteCharPtr(" R\n");
    if (auto pInfo = dynamic_cast<PDFReferenceElement*>(m_pTrailer->Lookup("Info")))
    {
        m_aEditBuffer.WriteCharPtr("/Info ");
        m_aEditBuffer.WriteUInt32AsString(pInfo->GetObjectValue());
        m_aEditBuffer.WriteCharPtr(" ");
        m_aEditBuffer.WriteUInt32AsString(pInfo->GetGenerationValue());
        m_aEditBuffer.WriteCharPtr(" R\n");
    }
    if (auto pID = dynamic_cast<PDFArrayElement*>(m_pTrailer->Lookup("ID")))
    {
        const std::vector<PDFElement*>& rElements = pID->GetElements();
        m_aEditBuffer.WriteCharPtr("/ID [ <");
        for (size_t i = 0; i < rElements.size(); ++i)
        {
            auto pIDString = dynamic_cast<PDFHexStringElement*>(rElements[i]);
            if (!pIDString)
                continue;

            m_aEditBuffer.WriteOString(pIDString->GetValue());
            if ((i + 1) < rElements.size())
                m_aEditBuffer.WriteCharPtr(">\n<");
        }
        m_aEditBuffer.WriteCharPtr("> ]\n");
    }

    if (!m_aStartXRefs.empty())
    {
        // Write location of the previous cross-reference section.
        m_aEditBuffer.WriteCharPtr("/Prev ");
        m_aEditBuffer.WriteUInt32AsString(m_aStartXRefs.back());
    }

    m_aEditBuffer.WriteCharPtr(">>\n");

    // Write startxref.
    m_aEditBuffer.WriteCharPtr("startxref\n");
    m_aEditBuffer.WriteUInt32AsString(nXRefOffset);
    m_aEditBuffer.WriteCharPtr("\n%%EOF\n");

    // Finalize the signature, now that we know the total file size.
    // Calculate the length of the last byte range.
    sal_uInt64 nFileEnd = m_aEditBuffer.Tell();
    sal_Int64 nLastByteRangeLength = nFileEnd - (nSignatureContentOffset + MAX_SIGNATURE_CONTENT_LENGTH + 1);
    // Write the length to the buffer.
    m_aEditBuffer.Seek(nSignatureLastByteRangeOffset);
    OStringBuffer aByteRangeBuffer;
    aByteRangeBuffer.append(nLastByteRangeLength);
    aByteRangeBuffer.append(" ]");
    m_aEditBuffer.WriteOString(aByteRangeBuffer.toString());

    // Create the PKCS#7 object.
    css::uno::Sequence<sal_Int8> aDerEncoded = xCertificate->getEncoded();
    if (!aDerEncoded.hasElements())
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::Sign: empty certificate");
        return false;
    }

    m_aEditBuffer.Seek(0);
    sal_uInt64 nBufferSize1 = nSignatureContentOffset - 1;
    std::unique_ptr<char[]> aBuffer1(new char[nBufferSize1]);
    m_aEditBuffer.ReadBytes(aBuffer1.get(), nBufferSize1);

    m_aEditBuffer.Seek(nSignatureContentOffset + MAX_SIGNATURE_CONTENT_LENGTH + 1);
    sal_uInt64 nBufferSize2 = nLastByteRangeLength;
    std::unique_ptr<char[]> aBuffer2(new char[nBufferSize2]);
    m_aEditBuffer.ReadBytes(aBuffer2.get(), nBufferSize2);

    OStringBuffer aCMSHexBuffer;
    vcl::PDFWriter::PDFSignContext aSignContext(aCMSHexBuffer);
    aSignContext.m_pDerEncoded = aDerEncoded.getArray();
    aSignContext.m_nDerEncoded = aDerEncoded.getLength();
    aSignContext.m_pByteRange1 = aBuffer1.get();
    aSignContext.m_nByteRange1 = nBufferSize1;
    aSignContext.m_pByteRange2 = aBuffer2.get();
    aSignContext.m_nByteRange2 = nBufferSize2;
    if (!vcl::PDFWriter::Sign(aSignContext))
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::Sign: PDFWriter::Sign() failed");
        return false;
    }

    assert(aCMSHexBuffer.getLength() <= MAX_SIGNATURE_CONTENT_LENGTH);

    m_aEditBuffer.Seek(nSignatureContentOffset);
    m_aEditBuffer.WriteOString(aCMSHexBuffer.toString());

    return true;
}

bool PDFDocument::Write(SvStream& rStream)
{
    m_aEditBuffer.Seek(0);
    rStream.WriteStream(m_aEditBuffer);
    return rStream.good();
}

bool PDFDocument::Tokenize(SvStream& rStream, TokenizeMode eMode)
{
    bool bInXRef = false;
    // The next number will be an xref offset.
    bool bInStartXRef = false;
    // Last seen object token.
    PDFObjectElement* pObject = nullptr;
    // Dictionary depth, so we know when we're outside any dictionaries.
    int nDictionaryDepth = 0;
    // Last seen array token that's outside any dictionaries.
    PDFArrayElement* pArray = nullptr;
    while (true)
    {
        char ch;
        rStream.ReadChar(ch);
        if (rStream.IsEof())
            break;

        switch (ch)
        {
        case '%':
        {
            auto pComment = new PDFCommentElement(*this);
            m_aElements.push_back(std::unique_ptr<PDFElement>(pComment));
            rStream.SeekRel(-1);
            if (!m_aElements.back()->Read(rStream))
                return false;
            if (eMode == TokenizeMode::EOF_TOKEN && !m_aEOFs.empty() && m_aEOFs.back() == rStream.Tell())
            {
                // Found EOF and partial parsing requested, we're done.
                return true;
            }
            break;
        }
        case '<':
        {
            // Dictionary or hex string.
            rStream.ReadChar(ch);
            rStream.SeekRel(-2);
            if (ch == '<')
            {
                m_aElements.push_back(std::unique_ptr<PDFElement>(new PDFDictionaryElement()));
                ++nDictionaryDepth;
            }
            else
                m_aElements.push_back(std::unique_ptr<PDFElement>(new PDFHexStringElement()));
            if (!m_aElements.back()->Read(rStream))
                return false;
            break;
        }
        case '>':
        {
            m_aElements.push_back(std::unique_ptr<PDFElement>(new PDFEndDictionaryElement()));
            --nDictionaryDepth;
            rStream.SeekRel(-1);
            if (!m_aElements.back()->Read(rStream))
                return false;
            break;
        }
        case '[':
        {
            auto pArr = new PDFArrayElement();
            m_aElements.push_back(std::unique_ptr<PDFElement>(pArr));
            if (nDictionaryDepth == 0)
            {
                // The array is attached directly, inform the object.
                pArray = pArr;
                if (pObject)
                    pObject->SetArray(pArray);
            }
            rStream.SeekRel(-1);
            if (!m_aElements.back()->Read(rStream))
                return false;
            break;
        }
        case ']':
        {
            m_aElements.push_back(std::unique_ptr<PDFElement>(new PDFEndArrayElement()));
            pArray = nullptr;
            rStream.SeekRel(-1);
            if (!m_aElements.back()->Read(rStream))
                return false;
            break;
        }
        case '/':
        {
            m_aElements.push_back(std::unique_ptr<PDFElement>(new PDFNameElement()));
            rStream.SeekRel(-1);
            if (!m_aElements.back()->Read(rStream))
                return false;
            break;
        }
        case '(':
        {
            m_aElements.push_back(std::unique_ptr<PDFElement>(new PDFLiteralStringElement()));
            rStream.SeekRel(-1);
            if (!m_aElements.back()->Read(rStream))
                return false;
            break;
        }
        default:
        {
            if (isdigit(ch) || ch == '-')
            {
                // Numbering object: an integer or a real.
                PDFNumberElement* pNumberElement = new PDFNumberElement();
                m_aElements.push_back(std::unique_ptr<PDFElement>(pNumberElement));
                rStream.SeekRel(-1);
                if (!pNumberElement->Read(rStream))
                    return false;
                if (bInStartXRef)
                {
                    bInStartXRef = false;
                    m_aStartXRefs.push_back(pNumberElement->GetValue());

                    auto it = m_aOffsetObjects.find(pNumberElement->GetValue());
                    if (it != m_aOffsetObjects.end())
                        m_pXRefStream = it->second;
                }
            }
            else if (isalpha(ch))
            {
                // Possible keyword, like "obj".
                rStream.SeekRel(-1);
                OString aKeyword = ReadKeyword(rStream);

                bool bObj = aKeyword == "obj";
                if (bObj || aKeyword == "R")
                {
                    size_t nElements = m_aElements.size();
                    if (nElements < 2)
                    {
                        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::Tokenize: expected at least two tokens before 'obj' or 'R' keyword");
                        return false;
                    }

                    auto pObjectNumber = dynamic_cast<PDFNumberElement*>(m_aElements[nElements - 2].get());
                    auto pGenerationNumber = dynamic_cast<PDFNumberElement*>(m_aElements[nElements - 1].get());
                    if (!pObjectNumber || !pGenerationNumber)
                    {
                        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::Tokenize: missing object or generation number before 'obj' or 'R' keyword");
                        return false;
                    }

                    if (bObj)
                    {
                        pObject = new PDFObjectElement(*this, pObjectNumber->GetValue(), pGenerationNumber->GetValue());
                        m_aElements.push_back(std::unique_ptr<PDFElement>(pObject));
                        m_aOffsetObjects[pObjectNumber->GetLocation()] = pObject;
                        m_aIDObjects[pObjectNumber->GetValue()] = pObject;
                    }
                    else
                    {
                        m_aElements.push_back(std::unique_ptr<PDFElement>(new PDFReferenceElement(*this, pObjectNumber->GetValue(), pGenerationNumber->GetValue())));
                        if (pArray)
                            // Reference is part of a direct (non-dictionary) array, inform the array.
                            pArray->PushBack(m_aElements.back().get());
                    }
                    if (!m_aElements.back()->Read(rStream))
                        return false;
                }
                else if (aKeyword == "stream")
                {
                    // Look up the length of the stream from the parent object's dictionary.
                    size_t nLength = 0;
                    for (size_t nElement = 0; nElement < m_aElements.size(); ++nElement)
                    {
                        // Iterate in reverse order.
                        size_t nIndex = m_aElements.size() - nElement - 1;
                        PDFElement* pElement = m_aElements[nIndex].get();
                        auto pObjectElement = dynamic_cast<PDFObjectElement*>(pElement);
                        if (!pObjectElement)
                            continue;

                        PDFElement* pLookup = pObjectElement->Lookup("Length");
                        auto pReference = dynamic_cast<PDFReferenceElement*>(pLookup);
                        if (pReference)
                        {
                            // Length is provided as a reference.
                            nLength = pReference->LookupNumber(rStream);
                            break;
                        }

                        auto pNumber = dynamic_cast<PDFNumberElement*>(pLookup);
                        if (pNumber)
                        {
                            // Length is provided directly.
                            nLength = pNumber->GetValue();
                            break;
                        }

                        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::Tokenize: found no Length key for stream keyword");
                        return false;
                    }

                    PDFDocument::SkipLineBreaks(rStream);
                    m_aElements.push_back(std::unique_ptr<PDFElement>(new PDFStreamElement(nLength)));
                    if (!m_aElements.back()->Read(rStream))
                        return false;
                }
                else if (aKeyword == "endstream")
                {
                    m_aElements.push_back(std::unique_ptr<PDFElement>(new PDFEndStreamElement()));
                    if (!m_aElements.back()->Read(rStream))
                        return false;
                }
                else if (aKeyword == "endobj")
                {
                    m_aElements.push_back(std::unique_ptr<PDFElement>(new PDFEndObjectElement()));
                    if (!m_aElements.back()->Read(rStream))
                        return false;
                    if (eMode == TokenizeMode::END_OF_OBJECT)
                    {
                        // Found endobj and only object parsing was requested, we're done.
                        return true;
                    }
                }
                else if (aKeyword == "true" || aKeyword == "false")
                    m_aElements.push_back(std::unique_ptr<PDFElement>(new PDFBooleanElement(aKeyword.toBoolean())));
                else if (aKeyword == "null")
                    m_aElements.push_back(std::unique_ptr<PDFElement>(new PDFNullElement()));
                else if (aKeyword == "xref")
                    // Allow 'f' and 'n' keywords.
                    bInXRef = true;
                else if (bInXRef && (aKeyword == "f" || aKeyword == "n"))
                {
                }
                else if (aKeyword == "trailer")
                {
                    m_pTrailer = new PDFTrailerElement(*this);
                    m_aElements.push_back(std::unique_ptr<PDFElement>(m_pTrailer));
                }
                else if (aKeyword == "startxref")
                {
                    bInStartXRef = true;
                }
                else
                {
                    SAL_WARN("xmlsecurity.pdfio", "PDFDocument::Tokenize: unexpected '" << aKeyword << "' keyword at byte position " << rStream.Tell());
                    return false;
                }
            }
            else
            {
                if (!isspace(ch))
                {
                    SAL_WARN("xmlsecurity.pdfio", "PDFDocument::Tokenize: unexpected character: " << ch << " at byte position " << rStream.Tell());
                    return false;
                }
            }
            break;
        }
        }
    }

    return true;
}

bool PDFDocument::Read(SvStream& rStream)
{
    // Check file magic.
    std::vector<sal_Int8> aHeader(5);
    rStream.Seek(0);
    rStream.ReadBytes(aHeader.data(), aHeader.size());
    if (aHeader[0] != '%' || aHeader[1] != 'P' || aHeader[2] != 'D' || aHeader[3] != 'F' || aHeader[4] != '-')
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::Read: header mismatch");
        return false;
    }

    // Allow later editing of the contents in-memory.
    rStream.Seek(0);
    m_aEditBuffer.WriteStream(rStream);

    // Look up the offset of the xref table.
    size_t nStartXRef = FindStartXRef(rStream);
    SAL_INFO("xmlsecurity.pdfio", "PDFDocument::Read: nStartXRef is " << nStartXRef);
    if (nStartXRef == 0)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::Read: found no xref statrt offset");
        return false;
    }
    while (true)
    {
        rStream.Seek(nStartXRef);
        ReadXRef(rStream);
        if (!Tokenize(rStream, TokenizeMode::EOF_TOKEN))
        {
            SAL_WARN("xmlsecurity.pdfio", "PDFDocument::Read: failed to tokenizer trailer after xref");
            return false;
        }

        PDFNumberElement* pPrev = nullptr;
        if (m_pTrailer)
            pPrev = dynamic_cast<PDFNumberElement*>(m_pTrailer->Lookup("Prev"));
        if (pPrev)
            nStartXRef = pPrev->GetValue();

        // Reset state, except object offsets and the edit buffer.
        m_aElements.clear();
        m_aStartXRefs.clear();
        m_aEOFs.clear();
        m_pTrailer = nullptr;
        m_pXRefStream = nullptr;
        if (!pPrev)
            break;
    }

    // Then we can tokenize the stream.
    rStream.Seek(0);
    return Tokenize(rStream, TokenizeMode::END_OF_STREAM);
}

OString PDFDocument::ReadKeyword(SvStream& rStream)
{
    OStringBuffer aBuf;
    char ch;
    rStream.ReadChar(ch);
    while (isalpha(ch))
    {
        aBuf.append(ch);
        rStream.ReadChar(ch);
        if (rStream.IsEof())
            break;
    }
    rStream.SeekRel(-1);
    return aBuf.toString();
}

size_t PDFDocument::FindStartXRef(SvStream& rStream)
{
    // Find the "startxref" token, somewhere near the end of the document.
    std::vector<char> aBuf(1024);
    rStream.Seek(STREAM_SEEK_TO_END);
    rStream.SeekRel(static_cast<sal_Int64>(-1) * aBuf.size());
    size_t nBeforePeek = rStream.Tell();
    size_t nSize = rStream.ReadBytes(aBuf.data(), aBuf.size());
    rStream.Seek(nBeforePeek);
    if (nSize != aBuf.size())
        aBuf.resize(nSize);
    OString aPrefix("startxref");
    auto it = std::search(aBuf.begin(), aBuf.end(), aPrefix.getStr(), aPrefix.getStr() + aPrefix.getLength());
    if (it == aBuf.end())
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::FindStartXRef: found no startxref");
        return 0;
    }

    rStream.SeekRel(it - aBuf.begin() + aPrefix.getLength());
    if (rStream.IsEof())
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::FindStartXRef: unexpected end of stream after startxref");
        return 0;
    }

    PDFDocument::SkipWhitespace(rStream);
    PDFNumberElement aNumber;
    if (!aNumber.Read(rStream))
        return 0;
    return aNumber.GetValue();
}

void PDFDocument::ReadXRefStream(SvStream& rStream)
{
    // Look up the stream length in the object dictionary.
    if (!Tokenize(rStream, TokenizeMode::END_OF_OBJECT))
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ReadXRefStream: failed to read object");
        return;
    }

    if (m_aElements.empty())
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ReadXRefStream: no tokens found");
        return;
    }

    PDFObjectElement* pObject = nullptr;
    for (const auto& pElement : m_aElements)
    {
        if (auto pObj = dynamic_cast<PDFObjectElement*>(pElement.get()))
        {
            pObject = pObj;
            break;
        }
    }
    if (!pObject)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ReadXRefStream: no object token found");
        return;
    }

    PDFElement* pLookup = pObject->Lookup("Length");
    auto pNumber = dynamic_cast<PDFNumberElement*>(pLookup);
    if (!pNumber)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ReadXRefStream: stream length is not provided");
        return;
    }
    sal_uInt64 nLength = pNumber->GetValue();

    // Look up the stream offset.
    PDFStreamElement* pStream = nullptr;
    for (const auto& pElement : m_aElements)
    {
        if (auto pS = dynamic_cast<PDFStreamElement*>(pElement.get()))
        {
            pStream = pS;
            break;
        }
    }
    if (!pStream)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ReadXRefStream: no stream token found");
        return;
    }

    // Read and decompress it.
    rStream.Seek(pStream->GetOffset());
    std::vector<char> aBuf(nLength);
    rStream.ReadBytes(aBuf.data(), aBuf.size());

    auto pFilter = dynamic_cast<PDFNameElement*>(pObject->Lookup("Filter"));
    if (!pFilter)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ReadXRefStream: no Filter found");
        return;
    }

    if (pFilter->GetValue() != "FlateDecode")
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ReadXRefStream: unexpected filter: " << pFilter->GetValue());
        return;
    }

    int nColumns = 1;
    int nPredictor = 1;
    if (auto pDecodeParams = dynamic_cast<PDFDictionaryElement*>(pObject->Lookup("DecodeParms")))
    {
        const std::map<OString, PDFElement*>& rItems = pDecodeParams->GetItems();
        auto it = rItems.find("Columns");
        if (it != rItems.end())
            if (auto pColumns = dynamic_cast<PDFNumberElement*>(it->second))
                nColumns = pColumns->GetValue();
        it = rItems.find("Predictor");
        if (it != rItems.end())
            if (auto pPredictor = dynamic_cast<PDFNumberElement*>(it->second))
                nPredictor = pPredictor->GetValue();
    }

    SvMemoryStream aSource(aBuf.data(), aBuf.size(), StreamMode::READ);
    SvMemoryStream aStream;
    ZCodec aZCodec;
    aZCodec.BeginCompression();
    aZCodec.Decompress(aSource, aStream);
    if (!aZCodec.EndCompression())
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ReadXRefStream: decompression failed");
        return;
    }

    // Look up the first and the last entry we need to read.
    auto pIndex = dynamic_cast<PDFArrayElement*>(pObject->Lookup("Index"));
    if (!pIndex || pIndex->GetElements().size() < 2)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ReadXRefStream: Index not found or has < 2 elements");
        return;
    }

    const std::vector<PDFElement*>& rIndexElements = pIndex->GetElements();
    auto pFirstObject = dynamic_cast<PDFNumberElement*>(rIndexElements[0]);
    if (!pFirstObject)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ReadXRefStream: Index has no first object");
        return;
    }

    auto pNumberOfObjects = dynamic_cast<PDFNumberElement*>(rIndexElements[1]);
    if (!pNumberOfObjects)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ReadXRefStream: Index has no number of objects");
        return;
    }

    // Look up the format of a single entry.
    const int nWSize = 3;
    auto pW = dynamic_cast<PDFArrayElement*>(pObject->Lookup("W"));
    if (!pW || pW->GetElements().size() < nWSize)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ReadXRefStream: W not found or has < 3 elements");
        return;
    }
    int aW[nWSize];
    // First character is the (kind of) repeated predictor.
    int nLineLength = 1;
    for (size_t i = 0; i < nWSize; ++i)
    {
        auto pI = dynamic_cast<PDFNumberElement*>(pW->GetElements()[i]);
        if (!pI)
        {
            SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ReadXRefStream: W contains non-number");
            return;
        }
        aW[i] = pI->GetValue();
        nLineLength += aW[i];
    }

    if (nLineLength - 1 != nColumns)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ReadXRefStream: /DecodeParms/Columns is inconsitent with /W");
        return;
    }

    size_t nSize = pNumberOfObjects->GetValue();
    aStream.Seek(0);
    // This is the line as read from the stream.
    std::vector<unsigned char> aOrigLine(nLineLength);
    // This is the line as it appears after tweaking according to nPredictor.
    std::vector<unsigned char> aFilteredLine(nLineLength);
    for (size_t nEntry = 0; nEntry < nSize; ++nEntry)
    {
        size_t nIndex = pFirstObject->GetValue() + nEntry;

        aStream.ReadBytes(aOrigLine.data(), aOrigLine.size());
        if (aOrigLine[0] + 10 != nPredictor)
        {
            SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ReadXRefStream: in-stream predictor is inconsistent with /DecodeParms/Predictor for object #" << nIndex);
            return;
        }

        for (int i = 0; i < nLineLength; ++i)
        {
            switch (nPredictor)
            {
            case 1:
                // No prediction.
                break;
            case 12:
                // PNG prediction: up (on all rows).
                aFilteredLine[i] = aFilteredLine[i] + aOrigLine[i];
                break;
            default:
                SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ReadXRefStream: unexpected predictor: " << nPredictor);
                return;
                break;
            }
        }

        // First character is already handled above.
        int nPos = 1;
        size_t nType = 0;
        // Start of the current field in the stream data.
        int nOffset = nPos;
        for (; nPos < nOffset + aW[0]; ++nPos)
        {
            unsigned char nCh = aFilteredLine[nPos];
            nType = (nType << 8) + nCh;
        }

        // Start of the object in the file stream.
        size_t nStreamOffset = 0;
        nOffset = nPos;
        for (; nPos < nOffset + aW[1]; ++nPos)
        {
            unsigned char nCh = aFilteredLine[nPos];
            nStreamOffset = (nStreamOffset << 8) + nCh;
        }

        // Generation number of the object.
        size_t nGenerationNumber = 0;
        nOffset = nPos;
        for (; nPos < nOffset + aW[2]; ++nPos)
        {
            unsigned char nCh = aFilteredLine[nPos];
            nGenerationNumber = (nGenerationNumber << 8) + nCh;
        }

        // "n" entry of the xref table
        if (nType == 1)
        {
            if (m_aXRef.find(nIndex) == m_aXRef.end())
            {
                m_aXRef[nIndex] = nStreamOffset;
                m_aXRefDirty[nIndex] = false;
            }
        }
    }
}

void PDFDocument::ReadXRef(SvStream& rStream)
{
    OString aKeyword = ReadKeyword(rStream);
    if (aKeyword.isEmpty())
    {
        ReadXRefStream(rStream);
        return;
    }

    if (aKeyword != "xref")
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ReadXRef: xref is not the first keyword");
        return;
    }

    PDFDocument::SkipWhitespace(rStream);

    while (true)
    {
        PDFNumberElement aFirstObject;
        if (!aFirstObject.Read(rStream))
        {
            // Next token is not a number, it'll be the trailer.
            return;
        }

        if (aFirstObject.GetValue() < 0)
        {
            SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ReadXRef: expected first object number >= 0");
            return;
        }

        PDFDocument::SkipWhitespace(rStream);
        PDFNumberElement aNumberOfEntries;
        if (!aNumberOfEntries.Read(rStream))
        {
            SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ReadXRef: failed to read number of entries");
            return;
        }

        if (aNumberOfEntries.GetValue() <= 0)
        {
            SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ReadXRef: expected one or more entries");
            return;
        }

        size_t nSize = aNumberOfEntries.GetValue();
        for (size_t nEntry = 0; nEntry < nSize; ++nEntry)
        {
            size_t nIndex = aFirstObject.GetValue() + nEntry;
            PDFDocument::SkipWhitespace(rStream);
            PDFNumberElement aOffset;
            if (!aOffset.Read(rStream))
            {
                SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ReadXRef: failed to read offset");
                return;
            }

            PDFDocument::SkipWhitespace(rStream);
            PDFNumberElement aGenerationNumber;
            if (!aGenerationNumber.Read(rStream))
            {
                SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ReadXRef: failed to read generation number");
                return;
            }

            PDFDocument::SkipWhitespace(rStream);
            aKeyword = ReadKeyword(rStream);
            if (aKeyword != "f" && aKeyword != "n")
            {
                SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ReadXRef: unexpected keyword");
                return;
            }
            // xrefs are read in reverse order, so never update an existing
            // offset with an older one.
            if (m_aXRef.find(nIndex) == m_aXRef.end())
            {
                m_aXRef[nIndex] = aOffset.GetValue();
                // Initially only the first entry is dirty.
                m_aXRefDirty[nIndex] = nIndex == 0;
            }
            PDFDocument::SkipWhitespace(rStream);
        }
    }
}

void PDFDocument::SkipWhitespace(SvStream& rStream)
{
    char ch = 0;

    while (true)
    {
        rStream.ReadChar(ch);
        if (rStream.IsEof())
            break;

        if (!isspace(ch))
        {
            rStream.SeekRel(-1);
            return;
        }
    }
}

void PDFDocument::SkipLineBreaks(SvStream& rStream)
{
    char ch = 0;

    while (true)
    {
        rStream.ReadChar(ch);
        if (rStream.IsEof())
            break;

        if (ch != '\n' && ch != '\r')
        {
            rStream.SeekRel(-1);
            return;
        }
    }
}

size_t PDFDocument::GetObjectOffset(size_t nIndex) const
{
    auto it = m_aXRef.find(nIndex);
    if (it == m_aXRef.end())
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::GetObjectOffset: wanted to look up index #" << nIndex << ", but failed");
        return 0;
    }

    return it->second;
}

const std::vector< std::unique_ptr<PDFElement> >& PDFDocument::GetElements()
{
    return m_aElements;
}

const std::map<size_t, PDFObjectElement*>& PDFDocument::GetIDObjects() const
{
    return m_aIDObjects;
}

std::vector<PDFObjectElement*> PDFDocument::GetPages()
{
    std::vector<PDFObjectElement*> aRet;

    PDFReferenceElement* pRoot = nullptr;
    if (m_pTrailer)
        pRoot = dynamic_cast<PDFReferenceElement*>(m_pTrailer->Lookup("Root"));
    else if (m_pXRefStream)
        pRoot = dynamic_cast<PDFReferenceElement*>(m_pXRefStream->Lookup("Root"));

    if (!pRoot)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::GetPages: trailer has no Root key");
        return aRet;
    }

    PDFObjectElement* pCatalog = pRoot->LookupObject();
    if (!pCatalog)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::GetPages: trailer has no catalog");
        return aRet;
    }

    PDFObjectElement* pPages = pCatalog->LookupObject("Pages");
    if (!pPages)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::GetPages: catalog (obj " << pCatalog->GetObjectValue() << ") has no pages");
        return aRet;
    }

    auto pKids = dynamic_cast<PDFArrayElement*>(pPages->Lookup("Kids"));
    if (!pKids)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::GetPages: pages has no kids");
        return aRet;
    }

    for (const auto& pKid : pKids->GetElements())
    {
        auto pReference = dynamic_cast<PDFReferenceElement*>(pKid);
        if (!pReference)
            continue;

        aRet.push_back(pReference->LookupObject());
    }

    return aRet;
}

void PDFDocument::PushBackEOF(size_t nOffset)
{
    m_aEOFs.push_back(nOffset);
}

std::vector<PDFObjectElement*> PDFDocument::GetSignatureWidgets()
{
    std::vector<PDFObjectElement*> aRet;

    std::vector<PDFObjectElement*> aPages = GetPages();

    for (const auto& pPage : aPages)
    {
        if (!pPage)
            continue;

        PDFElement* pAnnotsElement = pPage->Lookup("Annots");
        auto pAnnots = dynamic_cast<PDFArrayElement*>(pAnnotsElement);
        if (!pAnnots)
        {
            // Annots is not an array, see if it's a reference to an object
            // with a direct array.
            auto pAnnotsRef = dynamic_cast<PDFReferenceElement*>(pAnnotsElement);
            if (pAnnotsRef)
            {
                if (PDFObjectElement* pAnnotsObject = pAnnotsRef->LookupObject())
                {
                    pAnnots = pAnnotsObject->GetArray();
                }
            }
        }

        if (!pAnnots)
            continue;

        for (const auto& pAnnot : pAnnots->GetElements())
        {
            auto pReference = dynamic_cast<PDFReferenceElement*>(pAnnot);
            if (!pReference)
                continue;

            PDFObjectElement* pAnnotObject = pReference->LookupObject();
            if (!pAnnotObject)
                continue;

            auto pFT = dynamic_cast<PDFNameElement*>(pAnnotObject->Lookup("FT"));
            if (!pFT || pFT->GetValue() != "Sig")
                continue;

            aRet.push_back(pAnnotObject);
        }
    }

    return aRet;
}

int PDFDocument::AsHex(char ch)
{
    int nRet = 0;
    if (isdigit(ch))
        nRet = ch - '0';
    else
    {
        if (ch >= 'a' && ch <= 'f')
            nRet = ch - 'a';
        else if (ch >= 'A' && ch <= 'F')
            nRet = ch - 'A';
        else
            return -1;
        nRet += 10;
    }
    return nRet;
}

std::vector<unsigned char> PDFDocument::DecodeHexString(PDFHexStringElement* pElement)
{
    std::vector<unsigned char> aRet;
    const OString& rHex = pElement->GetValue();
    size_t nHexLen = rHex.getLength();
    {
        int nByte = 0;
        int nCount = 2;
        for (size_t i = 0; i < nHexLen; ++i)
        {
            nByte = nByte << 4;
            sal_Int8 nParsed = AsHex(rHex[i]);
            if (nParsed == -1)
            {
                SAL_WARN("xmlsecurity.pdfio", "PDFDocument::DecodeHexString: invalid hex value");
                return aRet;
            }
            nByte += nParsed;
            --nCount;
            if (!nCount)
            {
                aRet.push_back(nByte);
                nCount = 2;
                nByte = 0;
            }
        }
    }

    return aRet;
}

bool PDFDocument::ValidateSignature(SvStream& rStream, PDFObjectElement* pSignature, SignatureInformation& rInformation, bool bLast)
{
    PDFObjectElement* pValue = pSignature->LookupObject("V");
    if (!pValue)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ValidateSignature: no value");
        return false;
    }

    auto pContents = dynamic_cast<PDFHexStringElement*>(pValue->Lookup("Contents"));
    if (!pContents)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ValidateSignature: no contents");
        return false;
    }

    auto pByteRange = dynamic_cast<PDFArrayElement*>(pValue->Lookup("ByteRange"));
    if (!pByteRange || pByteRange->GetElements().size() < 2)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ValidateSignature: no byte range or too few elements");
        return false;
    }

    auto pSubFilter = dynamic_cast<PDFNameElement*>(pValue->Lookup("SubFilter"));
    if (!pSubFilter || pSubFilter->GetValue() != "adbe.pkcs7.detached")
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ValidateSignature: no or unsupported sub-filter");
        return false;
    }

    // Reason / comment / description is optional.
    auto pReason = dynamic_cast<PDFHexStringElement*>(pValue->Lookup("Reason"));
    if (pReason)
    {
        // See appendUnicodeTextString() for the export equivalent of this.
        std::vector<unsigned char> aReason = PDFDocument::DecodeHexString(pReason);
        OUStringBuffer aBuffer;
        sal_uInt16 nByte = 0;
        for (size_t i = 0; i < aReason.size(); ++i)
        {
            if (i % 2 == 0)
                nByte = aReason[i];
            else
            {
                sal_Unicode nUnicode;
                nUnicode = (nByte << 8);
                nUnicode |= aReason[i];
                aBuffer.append(nUnicode);
            }
        }

        if (!aBuffer.isEmpty())
            rInformation.ouDescription = aBuffer.makeStringAndClear();
    }

    // Date: used only when the time of signing is not available in the
    // signature.
    auto pM = dynamic_cast<PDFLiteralStringElement*>(pValue->Lookup("M"));
    if (pM)
    {
        // Example: "D:20161027100104".
        const OString& rM = pM->GetValue();
        if (rM.startsWith("D:") && rM.getLength() >= 16)
        {
            rInformation.stDateTime.Year = rM.copy(2, 4).toInt32();
            rInformation.stDateTime.Month = rM.copy(6, 2).toInt32();
            rInformation.stDateTime.Day = rM.copy(8, 2).toInt32();
            rInformation.stDateTime.Hours = rM.copy(10, 2).toInt32();
            rInformation.stDateTime.Minutes = rM.copy(12, 2).toInt32();
            rInformation.stDateTime.Seconds = rM.copy(14, 2).toInt32();
        }
    }

    // Build a list of offset-length pairs, representing the signed bytes.
    std::vector<std::pair<size_t, size_t>> aByteRanges;
    size_t nByteRangeOffset = 0;
    const std::vector<PDFElement*>& rByteRangeElements = pByteRange->GetElements();
    for (size_t i = 0; i < rByteRangeElements.size(); ++i)
    {
        auto pNumber = dynamic_cast<PDFNumberElement*>(rByteRangeElements[i]);
        if (!pNumber)
        {
            SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ValidateSignature: signature offset and length has to be a number");
            return false;
        }

        if (i % 2 == 0)
        {
            nByteRangeOffset = pNumber->GetValue();
            continue;
        }
        size_t nByteRangeLength = pNumber->GetValue();
        aByteRanges.push_back(std::make_pair(nByteRangeOffset, nByteRangeLength));
    }

    // Detect if the byte ranges don't cover everything, but the signature itself.
    if (aByteRanges.size() < 2)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ValidateSignature: expected 2 byte ranges");
        return false;
    }
    if (aByteRanges[0].first != 0)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ValidateSignature: first range start is not 0");
        return false;
    }
    // 2 is the leading "<" and the trailing ">" around the hex string.
    size_t nSignatureLength = pContents->GetValue().getLength() + 2;
    if (aByteRanges[1].first != (aByteRanges[0].second + nSignatureLength))
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ValidateSignature: second range start is not the end of the signature");
        return false;
    }
    rStream.Seek(STREAM_SEEK_TO_END);
    size_t nFileEnd = rStream.Tell();
    if (bLast && (aByteRanges[1].first + aByteRanges[1].second) != nFileEnd)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ValidateSignature: second range end is not the end of the file");
        return false;
    }

    // At this point there is no obviously missing info to validate the
    // signature.
    std::vector<unsigned char> aSignature = PDFDocument::DecodeHexString(pContents);
    if (aSignature.empty())
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ValidateSignature: empty contents");
        return false;
    }

#ifdef XMLSEC_CRYPTO_NSS
    // Validate the signature. No need to call NSS_Init() here, assume that the
    // caller did that already.

    SECItem aSignatureItem;
    aSignatureItem.data = aSignature.data();
    aSignatureItem.len = aSignature.size();
    NSSCMSMessage* pCMSMessage = NSS_CMSMessage_CreateFromDER(&aSignatureItem,
                                 /*cb=*/nullptr,
                                 /*cb_arg=*/nullptr,
                                 /*pwfn=*/nullptr,
                                 /*pwfn_arg=*/nullptr,
                                 /*decrypt_key_cb=*/nullptr,
                                 /*decrypt_key_cb_arg=*/nullptr);
    if (!NSS_CMSMessage_IsSigned(pCMSMessage))
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ValidateSignature: message is not signed");
        return false;
    }

    NSSCMSContentInfo* pCMSContentInfo = NSS_CMSMessage_ContentLevel(pCMSMessage, 0);
    if (!pCMSContentInfo)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ValidateSignature: NSS_CMSMessage_ContentLevel() failed");
        return false;
    }

    NSSCMSSignedData* pCMSSignedData = static_cast<NSSCMSSignedData*>(NSS_CMSContentInfo_GetContent(pCMSContentInfo));
    if (!pCMSSignedData)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ValidateSignature: NSS_CMSContentInfo_GetContent() failed");
        return false;
    }

    // Import certificates from the signed data temporarily, so it'll be
    // possible to verify the signature, even if we didn't have the certificate
    // perviously.
    std::vector<CERTCertificate*> aDocumentCertificates;
    for (size_t i = 0; pCMSSignedData->rawCerts[i]; ++i)
        aDocumentCertificates.push_back(CERT_NewTempCertificate(CERT_GetDefaultCertDB(), pCMSSignedData->rawCerts[i], nullptr, 0, 0));

    NSSCMSSignerInfo* pCMSSignerInfo = NSS_CMSSignedData_GetSignerInfo(pCMSSignedData, 0);
    if (!pCMSSignerInfo)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ValidateSignature: NSS_CMSSignedData_GetSignerInfo() failed");
        return false;
    }

    SECItem aAlgorithm = NSS_CMSSignedData_GetDigestAlgs(pCMSSignedData)[0]->algorithm;
    HASH_HashType eHashType = HASH_GetHashTypeByOidTag(SECOID_FindOIDTag(&aAlgorithm));
    HASHContext* pHASHContext = HASH_Create(eHashType);
    if (!pHASHContext)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ValidateSignature: HASH_Create() failed");
        return false;
    }

    // We have a hash, update it with the byte ranges.
    for (const auto& rByteRange : aByteRanges)
    {
        rStream.Seek(rByteRange.first);

        // And now hash this byte range.
        const int nChunkLen = 4096;
        std::vector<unsigned char> aBuffer(nChunkLen);
        for (size_t nByte = 0; nByte < rByteRange.second;)
        {
            size_t nRemainingSize = rByteRange.second - nByte;
            if (nRemainingSize < nChunkLen)
            {
                rStream.ReadBytes(aBuffer.data(), nRemainingSize);
                HASH_Update(pHASHContext, aBuffer.data(), nRemainingSize);
                nByte = rByteRange.second;
            }
            else
            {
                rStream.ReadBytes(aBuffer.data(), nChunkLen);
                HASH_Update(pHASHContext, aBuffer.data(), nChunkLen);
                nByte += nChunkLen;
            }
        }
    }

    // Find out what is the expected length of the hash.
    unsigned int nMaxResultLen = 0;
    switch (SECOID_FindOIDTag(&aAlgorithm))
    {
    case SEC_OID_SHA1:
        nMaxResultLen = msfilter::SHA1_HASH_LENGTH;
        break;
    case SEC_OID_SHA256:
        nMaxResultLen = msfilter::SHA256_HASH_LENGTH;
        break;
    default:
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ValidateSignature: unrecognized algorithm");
        return false;
    }

    auto pActualResultBuffer = static_cast<unsigned char*>(PORT_Alloc(nMaxResultLen));
    unsigned int nActualResultLen;
    HASH_End(pHASHContext, pActualResultBuffer, &nActualResultLen, nMaxResultLen);

    CERTCertificate* pCertificate = NSS_CMSSignerInfo_GetSigningCertificate(pCMSSignerInfo, CERT_GetDefaultCertDB());
    if (!pCertificate)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ValidateSignature: NSS_CMSSignerInfo_GetSigningCertificate() failed");
        return false;
    }
    else
    {
        uno::Sequence<sal_Int8> aDerCert(pCertificate->derCert.len);
        for (size_t i = 0; i < pCertificate->derCert.len; ++i)
            aDerCert[i] = pCertificate->derCert.data[i];
        OUStringBuffer aBuffer;
        sax::Converter::encodeBase64(aBuffer, aDerCert);
        rInformation.ouX509Certificate = aBuffer.makeStringAndClear();
    }

    PRTime nSigningTime;
    // This may fail, in which case the date should be taken from the dictionary's "M" key.
    if (NSS_CMSSignerInfo_GetSigningTime(pCMSSignerInfo, &nSigningTime) == SECSuccess)
    {
        // First convert the UNIX timestamp to an ISO8601 string.
        OUStringBuffer aBuffer;
        uno::Reference<uno::XComponentContext> xComponentContext = comphelper::getProcessComponentContext();
        CalendarWrapper aCalendarWrapper(xComponentContext);
        // nSigningTime is in microseconds.
        SvXMLUnitConverter::convertDateTime(aBuffer, static_cast<double>(nSigningTime) / 1000000 / tools::Time::secondPerDay, aCalendarWrapper.getEpochStart().GetUNODate());

        // Then convert this string to a local UNO DateTime.
        util::DateTime aUNODateTime;
        try
        {
            utl::ISO8601parseDateTime(aBuffer.toString(), aUNODateTime);
        }
        catch (const std::length_error&)
        {
            SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ValidateSignature: failed to parse signature date string");
            return false;
        }
        DateTime aDateTime(aUNODateTime);
        aDateTime.ConvertToLocalTime();
        rInformation.stDateTime = aDateTime.GetUNODateTime();
    }

    SECItem* pContentInfoContentData = pCMSSignedData->contentInfo.content.data;
    if (pContentInfoContentData && pContentInfoContentData->data)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDocument::ValidateSignature: expected nullptr content info");
        return false;
    }

    SECItem aActualResultItem;
    aActualResultItem.data = pActualResultBuffer;
    aActualResultItem.len = nActualResultLen;
    if (NSS_CMSSignerInfo_Verify(pCMSSignerInfo, &aActualResultItem, nullptr) == SECSuccess)
        rInformation.nStatus = xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED;

    // Everything went fine
    PORT_Free(pActualResultBuffer);
    HASH_Destroy(pHASHContext);
    NSS_CMSSignerInfo_Destroy(pCMSSignerInfo);
    for (auto pDocumentCertificate : aDocumentCertificates)
        CERT_DestroyCertificate(pDocumentCertificate);

    return true;
#else
    // Not implemented.
    (void)rStream;
    (void)rInformation;

    return false;
#endif
}

PDFCommentElement::PDFCommentElement(PDFDocument& rDoc)
    : m_rDoc(rDoc)
{
}

bool PDFCommentElement::Read(SvStream& rStream)
{
    // Read from (including) the % char till (excluding) the end of the line.
    OStringBuffer aBuf;
    char ch;
    rStream.ReadChar(ch);
    while (!rStream.IsEof())
    {
        if (ch == 0x0a)
        {
            m_aComment = aBuf.makeStringAndClear();

            if (m_aComment.startsWith("%%EOF"))
                m_rDoc.PushBackEOF(rStream.Tell());

            SAL_INFO("xmlsecurity.pdfio", "PDFCommentElement::Read: m_aComment is '" << m_aComment << "'");
            return true;
        }
        aBuf.append(ch);
        rStream.ReadChar(ch);
    }

    return false;
}

PDFNumberElement::PDFNumberElement()
    : m_nOffset(0),
      m_nLength(0),
      m_fValue(0)
{
}

bool PDFNumberElement::Read(SvStream& rStream)
{
    OStringBuffer aBuf;
    m_nOffset = rStream.Tell();
    char ch;
    rStream.ReadChar(ch);
    if (!isdigit(ch) && ch != '-' && ch != '.')
    {
        rStream.SeekRel(-1);
        return false;
    }
    while (!rStream.IsEof())
    {
        if (!isdigit(ch) && ch != '-' && ch != '.')
        {
            rStream.SeekRel(-1);
            m_nLength = rStream.Tell() - m_nOffset;
            m_fValue = aBuf.makeStringAndClear().toDouble();
            SAL_INFO("xmlsecurity.pdfio", "PDFNumberElement::Read: m_fValue is '" << m_fValue << "'");
            return true;
        }
        aBuf.append(ch);
        rStream.ReadChar(ch);
    }

    return false;
}

sal_uInt64 PDFNumberElement::GetLocation() const
{
    return m_nOffset;
}

sal_uInt64 PDFNumberElement::GetLength() const
{
    return m_nLength;
}

PDFBooleanElement::PDFBooleanElement(bool /*bValue*/)
{
}

bool PDFBooleanElement::Read(SvStream& /*rStream*/)
{
    return true;
}

bool PDFNullElement::Read(SvStream& /*rStream*/)
{
    return true;
}

bool PDFHexStringElement::Read(SvStream& rStream)
{
    char ch;
    rStream.ReadChar(ch);
    if (ch != '<')
    {
        SAL_INFO("xmlsecurity.pdfio", "PDFHexStringElement::Read: expected '<' as first character");
        return false;
    }
    rStream.ReadChar(ch);

    OStringBuffer aBuf;
    while (!rStream.IsEof())
    {
        if (ch == '>')
        {
            m_aValue = aBuf.makeStringAndClear();
            SAL_INFO("xmlsecurity.pdfio", "PDFHexStringElement::Read: m_aValue length is " << m_aValue.getLength());
            return true;
        }
        aBuf.append(ch);
        rStream.ReadChar(ch);
    }

    return false;
}

const OString& PDFHexStringElement::GetValue() const
{
    return m_aValue;
}

bool PDFLiteralStringElement::Read(SvStream& rStream)
{
    char nPrevCh = 0;
    char ch = 0;
    rStream.ReadChar(ch);
    if (ch != '(')
    {
        SAL_INFO("xmlsecurity.pdfio", "PDFHexStringElement::Read: expected '(' as first character");
        return false;
    }
    nPrevCh = ch;
    rStream.ReadChar(ch);

    OStringBuffer aBuf;
    while (!rStream.IsEof())
    {
        if (ch == ')' && nPrevCh != '\\')
        {
            m_aValue = aBuf.makeStringAndClear();
            SAL_INFO("xmlsecurity.pdfio", "PDFLiteralStringElement::Read: m_aValue is '" << m_aValue << "'");
            return true;
        }
        aBuf.append(ch);
        nPrevCh = ch;
        rStream.ReadChar(ch);
    }

    return false;
}

const OString& PDFLiteralStringElement::GetValue() const
{
    return m_aValue;
}

PDFTrailerElement::PDFTrailerElement(PDFDocument& rDoc)
    : m_rDoc(rDoc)
{
}

bool PDFTrailerElement::Read(SvStream& /*rStream*/)
{
    return true;
}

PDFElement* PDFTrailerElement::Lookup(const OString& rDictionaryKey)
{
    if (m_aDictionary.empty())
        PDFDictionaryElement::Parse(m_rDoc.GetElements(), this, m_aDictionary);

    return PDFDictionaryElement::Lookup(m_aDictionary, rDictionaryKey);
}


double PDFNumberElement::GetValue() const
{
    return m_fValue;
}

PDFObjectElement::PDFObjectElement(PDFDocument& rDoc, double fObjectValue, double fGenerationValue)
    : m_rDoc(rDoc),
      m_fObjectValue(fObjectValue),
      m_fGenerationValue(fGenerationValue),
      m_nDictionaryOffset(0),
      m_nDictionaryLength(0),
      m_pDictionaryElement(nullptr),
      m_pArrayElement(nullptr)
{
}

bool PDFObjectElement::Read(SvStream& /*rStream*/)
{
    SAL_INFO("xmlsecurity.pdfio", "PDFObjectElement::Read: " << m_fObjectValue << " " << m_fGenerationValue << " obj");
    return true;
}

PDFDictionaryElement::PDFDictionaryElement()
    : m_nLocation(0)
{
}

size_t PDFDictionaryElement::Parse(const std::vector< std::unique_ptr<PDFElement> >& rElements, PDFElement* pThis, std::map<OString, PDFElement*>& rDictionary)
{
    // The index of last parsed element, in case of nested dictionaries.
    size_t nRet = 0;

    if (!rDictionary.empty())
        return nRet;

    auto pThisObject = dynamic_cast<PDFObjectElement*>(pThis);
    // This is set to non-nullptr here for nested dictionaries only.
    auto pThisDictionary = dynamic_cast<PDFDictionaryElement*>(pThis);

    // Find out where the dictionary for this object starts.
    size_t nIndex = 0;
    for (size_t i = 0; i < rElements.size(); ++i)
    {
        if (rElements[i].get() == pThis)
        {
            nIndex = i;
            break;
        }
    }

    OString aName;
    sal_uInt64 nNameOffset = 0;
    std::vector<PDFNumberElement*> aNumbers;
    // The array value we're in -- if any.
    PDFArrayElement* pArray = nullptr;
    sal_uInt64 nDictionaryOffset = 0;
    int nDictionaryDepth = 0;
    for (size_t i = nIndex; i < rElements.size(); ++i)
    {
        // Dictionary tokens can be nested, track enter/leave.
        if (auto pDictionary = dynamic_cast<PDFDictionaryElement*>(rElements[i].get()))
        {
            if (++nDictionaryDepth == 1)
            {
                // First dictionary start, track start offset.
                nDictionaryOffset = pDictionary->m_nLocation;
                if (pThisObject)
                {
                    pThisObject->SetDictionary(pDictionary);
                    pThisDictionary = pDictionary;
                    pThisObject->SetDictionaryOffset(nDictionaryOffset);
                }
            }
            else
            {
                // Nested dictionary.
                nIndex = PDFDictionaryElement::Parse(rElements, pDictionary, pDictionary->m_aItems);
                rDictionary[aName] = pDictionary;
                aName.clear();
            }
        }

        if (auto pEndDictionary = dynamic_cast<PDFEndDictionaryElement*>(rElements[i].get()))
        {
            if (--nDictionaryDepth == 0)
            {
                // Last dictionary end, track length and stop parsing.
                if (pThisObject)
                    pThisObject->SetDictionaryLength(pEndDictionary->GetLocation() - nDictionaryOffset);
                nRet = nIndex;
                break;
            }
        }

        auto pName = dynamic_cast<PDFNameElement*>(rElements[i].get());
        if (pName)
        {
            if (!aNumbers.empty())
            {
                PDFNumberElement* pNumber = aNumbers.back();
                rDictionary[aName] = pNumber;
                if (pThisDictionary)
                {
                    pThisDictionary->SetKeyOffset(aName, nNameOffset);
                    pThisDictionary->SetKeyValueLength(aName, pNumber->GetLocation() + pNumber->GetLength() - nNameOffset);
                }
                aName.clear();
                aNumbers.clear();
            }

            if (aName.isEmpty())
            {
                // Remember key.
                aName = pName->GetValue();
                nNameOffset = pName->GetLocation();
            }
            else
            {
                // Name-name key-value.
                rDictionary[aName] = pName;
                if (pThisDictionary)
                {
                    pThisDictionary->SetKeyOffset(aName, nNameOffset);
                    pThisDictionary->SetKeyValueLength(aName, pName->GetLocation() + pName->GetLength() - nNameOffset);
                }
                aName.clear();
            }
            continue;
        }

        auto pArr = dynamic_cast<PDFArrayElement*>(rElements[i].get());
        if (pArr)
        {
            pArray = pArr;
            continue;
        }

        auto pEndArr = dynamic_cast<PDFEndArrayElement*>(rElements[i].get());
        if (pArray && pEndArr)
        {
            if (!aNumbers.empty())
            {
                for (auto& pNumber : aNumbers)
                    pArray->PushBack(pNumber);
                aNumbers.clear();
            }
            rDictionary[aName] = pArray;
            if (pThisDictionary)
            {
                pThisDictionary->SetKeyOffset(aName, nNameOffset);
                // Include the ending ']' in the length of the key - (array)value pair length.
                pThisDictionary->SetKeyValueLength(aName, pEndArr->GetOffset() - nNameOffset + 1);
            }
            aName.clear();
            pArray = nullptr;
            continue;
        }

        auto pReference = dynamic_cast<PDFReferenceElement*>(rElements[i].get());
        if (pReference)
        {
            if (!pArray)
            {
                rDictionary[aName] = pReference;
                if (pThisDictionary)
                    pThisDictionary->SetKeyOffset(aName, nNameOffset);
                aName.clear();
            }
            else
            {
                pArray->PushBack(pReference);
            }
            aNumbers.clear();
            continue;
        }

        auto pLiteralString = dynamic_cast<PDFLiteralStringElement*>(rElements[i].get());
        if (pLiteralString)
        {
            rDictionary[aName] = pLiteralString;
            if (pThisDictionary)
                pThisDictionary->SetKeyOffset(aName, nNameOffset);
            aName.clear();
            continue;
        }

        auto pHexString = dynamic_cast<PDFHexStringElement*>(rElements[i].get());
        if (pHexString)
        {
            if (!pArray)
            {
                rDictionary[aName] = pHexString;
                if (pThisDictionary)
                    pThisDictionary->SetKeyOffset(aName, nNameOffset);
                aName.clear();
            }
            else
            {
                pArray->PushBack(pHexString);
            }
            continue;
        }

        if (dynamic_cast<PDFEndObjectElement*>(rElements[i].get()))
            break;

        // Just remember this, so that in case it's not a reference parameter,
        // we can handle it later.
        auto pNumber = dynamic_cast<PDFNumberElement*>(rElements[i].get());
        if (pNumber)
            aNumbers.push_back(pNumber);
    }

    if (!aNumbers.empty())
    {
        rDictionary[aName] = aNumbers.back();
        if (pThisDictionary)
            pThisDictionary->SetKeyOffset(aName, nNameOffset);
        aName.clear();
        aNumbers.clear();
    }

    return nRet;
}

PDFElement* PDFDictionaryElement::Lookup(const std::map<OString, PDFElement*>& rDictionary, const OString& rKey)
{
    auto it = rDictionary.find(rKey);
    if (it == rDictionary.end())
        return nullptr;

    return it->second;
}

PDFElement* PDFObjectElement::Lookup(const OString& rDictionaryKey)
{
    if (m_aDictionary.empty())
        PDFDictionaryElement::Parse(m_rDoc.GetElements(), this, m_aDictionary);

    return PDFDictionaryElement::Lookup(m_aDictionary, rDictionaryKey);
}

PDFObjectElement* PDFObjectElement::LookupObject(const OString& rDictionaryKey)
{
    auto pKey = dynamic_cast<PDFReferenceElement*>(Lookup(rDictionaryKey));
    if (!pKey)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFObjectElement::LookupObject: no such key with reference value: " << rDictionaryKey);
        return nullptr;
    }

    return pKey->LookupObject();
}

double PDFObjectElement::GetObjectValue() const
{
    return m_fObjectValue;
}

void PDFObjectElement::SetDictionaryOffset(sal_uInt64 nDictionaryOffset)
{
    m_nDictionaryOffset = nDictionaryOffset;
}

sal_uInt64 PDFObjectElement::GetDictionaryOffset()
{
    if (m_aDictionary.empty())
        PDFDictionaryElement::Parse(m_rDoc.GetElements(), this, m_aDictionary);

    return m_nDictionaryOffset;
}

void PDFDictionaryElement::SetKeyOffset(const OString& rKey, sal_uInt64 nOffset)
{
    m_aDictionaryKeyOffset[rKey] = nOffset;
}

void PDFDictionaryElement::SetKeyValueLength(const OString& rKey, sal_uInt64 nLength)
{
    m_aDictionaryKeyValueLength[rKey] = nLength;
}

sal_uInt64 PDFDictionaryElement::GetKeyOffset(const OString& rKey) const
{
    auto it = m_aDictionaryKeyOffset.find(rKey);
    if (it == m_aDictionaryKeyOffset.end())
        return 0;

    return it->second;
}

sal_uInt64 PDFDictionaryElement::GetKeyValueLength(const OString& rKey) const
{
    auto it = m_aDictionaryKeyValueLength.find(rKey);
    if (it == m_aDictionaryKeyValueLength.end())
        return 0;

    return it->second;
}

const std::map<OString, PDFElement*>& PDFDictionaryElement::GetItems() const
{
    return m_aItems;
}

void PDFObjectElement::SetDictionaryLength(sal_uInt64 nDictionaryLength)
{
    m_nDictionaryLength = nDictionaryLength;
}

sal_uInt64 PDFObjectElement::GetDictionaryLength()
{
    if (m_aDictionary.empty())
        PDFDictionaryElement::Parse(m_rDoc.GetElements(), this, m_aDictionary);

    return m_nDictionaryLength;
}

PDFDictionaryElement* PDFObjectElement::GetDictionary() const
{
    return m_pDictionaryElement;
}

void PDFObjectElement::SetDictionary(PDFDictionaryElement* pDictionaryElement)
{
    m_pDictionaryElement = pDictionaryElement;
}

void PDFObjectElement::SetArray(PDFArrayElement* pArrayElement)
{
    m_pArrayElement = pArrayElement;
}

PDFArrayElement* PDFObjectElement::GetArray() const
{
    return m_pArrayElement;
}

PDFReferenceElement::PDFReferenceElement(PDFDocument& rDoc, int fObjectValue, int fGenerationValue)
    : m_rDoc(rDoc),
      m_fObjectValue(fObjectValue),
      m_fGenerationValue(fGenerationValue)
{
}

bool PDFReferenceElement::Read(SvStream& /*rStream*/)
{
    SAL_INFO("xmlsecurity.pdfio", "PDFReferenceElement::Read: " << m_fObjectValue << " " << m_fGenerationValue << " R");
    return true;
}

double PDFReferenceElement::LookupNumber(SvStream& rStream) const
{
    size_t nOffset = m_rDoc.GetObjectOffset(m_fObjectValue);
    if (nOffset == 0)
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFReferenceElement::LookupNumber: found no offset for object #" << m_fObjectValue);
        return 0;
    }

    sal_uInt64 nOrigPos = rStream.Tell();
    comphelper::ScopeGuard g([&]()
    {
        rStream.Seek(nOrigPos);
    });

    rStream.Seek(nOffset);
    {
        PDFDocument::SkipWhitespace(rStream);
        PDFNumberElement aNumber;
        bool bRet = aNumber.Read(rStream);
        if (!bRet || aNumber.GetValue() != m_fObjectValue)
        {
            SAL_WARN("xmlsecurity.pdfio", "PDFReferenceElement::LookupNumber: offset points to not matching object");
            return 0;
        }
    }

    {
        PDFDocument::SkipWhitespace(rStream);
        PDFNumberElement aNumber;
        bool bRet = aNumber.Read(rStream);
        if (!bRet || aNumber.GetValue() != m_fGenerationValue)
        {
            SAL_WARN("xmlsecurity.pdfio", "PDFReferenceElement::LookupNumber: offset points to not matching generation");
            return 0;
        }
    }

    {
        PDFDocument::SkipWhitespace(rStream);
        OString aKeyword = PDFDocument::ReadKeyword(rStream);
        if (aKeyword != "obj")
        {
            SAL_WARN("xmlsecurity.pdfio", "PDFReferenceElement::LookupNumber: offset doesn't point to an obj keyword");
            return 0;
        }
    }

    PDFDocument::SkipWhitespace(rStream);
    PDFNumberElement aNumber;
    if (!aNumber.Read(rStream))
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFReferenceElement::LookupNumber: failed to read referenced number");
        return 0;
    }

    return aNumber.GetValue();
}

PDFObjectElement* PDFReferenceElement::LookupObject() const
{
    const std::map<size_t, PDFObjectElement*>& rIDObjects = m_rDoc.GetIDObjects();
    auto it = rIDObjects.find(m_fObjectValue);
    if (it != rIDObjects.end())
        return it->second;

    SAL_WARN("xmlsecurity.pdfio", "PDFReferenceElement::LookupObject: can't find obj " << m_fObjectValue);
    return nullptr;
}

int PDFReferenceElement::GetObjectValue() const
{
    return m_fObjectValue;
}

int PDFReferenceElement::GetGenerationValue() const
{
    return m_fGenerationValue;
}

bool PDFDictionaryElement::Read(SvStream& rStream)
{
    char ch;
    rStream.ReadChar(ch);
    if (ch != '<')
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDictionaryElement::Read: unexpected character: " << ch);
        return false;
    }

    if (rStream.IsEof())
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDictionaryElement::Read: unexpected end of file");
        return false;
    }

    rStream.ReadChar(ch);
    if (ch != '<')
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFDictionaryElement::Read: unexpected character: " << ch);
        return false;
    }

    m_nLocation = rStream.Tell();

    SAL_INFO("xmlsecurity.pdfio", "PDFDictionaryElement::Read: '<<'");

    return true;
}

PDFEndDictionaryElement::PDFEndDictionaryElement()
    : m_nLocation(0)
{
}

sal_uInt64 PDFEndDictionaryElement::GetLocation() const
{
    return m_nLocation;
}

bool PDFEndDictionaryElement::Read(SvStream& rStream)
{
    m_nLocation = rStream.Tell();
    char ch;
    rStream.ReadChar(ch);
    if (ch != '>')
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFEndDictionaryElement::Read: unexpected character: " << ch);
        return false;
    }

    if (rStream.IsEof())
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFEndDictionaryElement::Read: unexpected end of file");
        return false;
    }

    rStream.ReadChar(ch);
    if (ch != '>')
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFEndDictionaryElement::Read: unexpected character: " << ch);
        return false;
    }

    SAL_INFO("xmlsecurity.pdfio", "PDFEndDictionaryElement::Read: '>>'");

    return true;
}

PDFNameElement::PDFNameElement()
    : m_nLocation(0),
      m_nLength(0)
{
}

bool PDFNameElement::Read(SvStream& rStream)
{
    char ch;
    rStream.ReadChar(ch);
    if (ch != '/')
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFNameElement::Read: unexpected character: " << ch);
        return false;
    }
    m_nLocation = rStream.Tell();

    if (rStream.IsEof())
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFNameElement::Read: unexpected end of file");
        return false;
    }

    // Read till the first white-space.
    OStringBuffer aBuf;
    rStream.ReadChar(ch);
    while (!rStream.IsEof())
    {
        if (isspace(ch) || ch == '/' || ch == '[' || ch == '<' || ch == '>' || ch == '(')
        {
            rStream.SeekRel(-1);
            m_aValue = aBuf.makeStringAndClear();
            SAL_INFO("xmlsecurity.pdfio", "PDFNameElement::Read: m_aValue is '" << m_aValue << "'");
            return true;
        }
        aBuf.append(ch);
        rStream.ReadChar(ch);
    }

    return false;
}

const OString& PDFNameElement::GetValue() const
{
    return m_aValue;
}

sal_uInt64 PDFNameElement::GetLocation() const
{
    return m_nLocation;
}

sal_uInt64 PDFNameElement::GetLength() const
{
    return m_nLength;
}

PDFStreamElement::PDFStreamElement(size_t nLength)
    : m_nLength(nLength),
      m_nOffset(0)
{
}

bool PDFStreamElement::Read(SvStream& rStream)
{
    SAL_INFO("xmlsecurity.pdfio", "PDFStreamElement::Read: length is " << m_nLength);
    m_nOffset = rStream.Tell();
    rStream.SeekRel(m_nLength);

    return rStream.good();
}

sal_uInt64 PDFStreamElement::GetOffset() const
{
    return m_nOffset;
}

bool PDFEndStreamElement::Read(SvStream& /*rStream*/)
{
    return true;
}

bool PDFEndObjectElement::Read(SvStream& /*rStream*/)
{
    return true;
}

PDFArrayElement::PDFArrayElement()
    : m_nOffset(0)
{
}

bool PDFArrayElement::Read(SvStream& rStream)
{
    char ch;
    rStream.ReadChar(ch);
    if (ch != '[')
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFArrayElement::Read: unexpected character: " << ch);
        return false;
    }
    m_nOffset = rStream.Tell();

    SAL_INFO("xmlsecurity.pdfio", "PDFArrayElement::Read: '['");

    return true;
}

void PDFArrayElement::PushBack(PDFElement* pElement)
{
    m_aElements.push_back(pElement);
}

const std::vector<PDFElement*>& PDFArrayElement::GetElements()
{
    return m_aElements;
}

PDFEndArrayElement::PDFEndArrayElement()
    : m_nOffset(0)
{
}

bool PDFEndArrayElement::Read(SvStream& rStream)
{
    m_nOffset = rStream.Tell();
    char ch;
    rStream.ReadChar(ch);
    if (ch != ']')
    {
        SAL_WARN("xmlsecurity.pdfio", "PDFEndArrayElement::Read: unexpected character: " << ch);
        return false;
    }

    SAL_INFO("xmlsecurity.pdfio", "PDFEndArrayElement::Read: ']'");

    return true;
}

sal_uInt64 PDFEndArrayElement::GetOffset() const
{
    return m_nOffset;
}

} // namespace pdfio
} // namespace xmlsecurity

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

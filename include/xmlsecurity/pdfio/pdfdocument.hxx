/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_XMLSECURITY_PDFIO_PDFDOCUMENT_HXX
#define INCLUDED_XMLSECURITY_PDFIO_PDFDOCUMENT_HXX

#include <map>
#include <vector>

#include <com/sun/star/security/XCertificate.hpp>

#include <tools/stream.hxx>

#include <xmlsecurity/xmlsecuritydllapi.h>

struct SignatureInformation;

namespace xmlsecurity
{
namespace pdfio
{

class PDFTrailerElement;
class PDFHexStringElement;
class PDFReferenceElement;
class PDFDocument;
class PDFDictionaryElement;
class PDFArrayElement;
class PDFStreamElement;

/// A byte range in a PDF file.
class XMLSECURITY_DLLPUBLIC PDFElement
{
public:
    virtual bool Read(SvStream& rStream) = 0;
    virtual ~PDFElement() { }
};

/// Indirect object: something with a unique ID.
class XMLSECURITY_DLLPUBLIC PDFObjectElement : public PDFElement
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
    /// The stream of this object, used when this is an object stream.
    PDFStreamElement* m_pStreamElement;
    /// Objects of an object stream.
    std::vector< std::unique_ptr<PDFObjectElement> > m_aStoredElements;
    /// Elements of an object in an object stream.
    std::vector< std::unique_ptr<PDFElement> > m_aElements;
    /// Uncompressed buffer of an object in an object stream.
    std::unique_ptr<SvMemoryStream> m_pStreamBuffer;

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
    void SetStream(PDFStreamElement* pStreamElement);
    /// Access to the stream of the object, if it has any.
    PDFStreamElement* GetStream() const;
    PDFArrayElement* GetArray() const;
    /// Parse objects stored in this object stream.
    void ParseStoredObjects();
    std::vector< std::unique_ptr<PDFElement> >& GetStoredElements();
    SvMemoryStream* GetStreamBuffer() const;
    void SetStreamBuffer(std::unique_ptr<SvMemoryStream>& pStreamBuffer);
};

/// Array object: a list.
class XMLSECURITY_DLLPUBLIC PDFArrayElement : public PDFElement
{
    /// Location after the '[' token.
    sal_uInt64 m_nOffset = 0;
    std::vector<PDFElement*> m_aElements;
public:
    PDFArrayElement();
    bool Read(SvStream& rStream) override;
    void PushBack(PDFElement* pElement);
    const std::vector<PDFElement*>& GetElements();
};

/// Reference object: something with a unique ID.
class XMLSECURITY_DLLPUBLIC PDFReferenceElement : public PDFElement
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
    PDFObjectElement* LookupObject();
    int GetObjectValue() const;
    int GetGenerationValue() const;
};

/// Stream object: a byte array with a known length.
class XMLSECURITY_DLLPUBLIC PDFStreamElement : public PDFElement
{
    size_t m_nLength;
    sal_uInt64 m_nOffset;
    /// The byte array itself.
    SvMemoryStream m_aMemory;

public:
    explicit PDFStreamElement(size_t nLength);
    bool Read(SvStream& rStream) override;
    sal_uInt64 GetOffset() const;
    SvMemoryStream& GetMemory();
};

/// Name object: a key string.
class XMLSECURITY_DLLPUBLIC PDFNameElement : public PDFElement
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

/// Dictionary object: a set key-value pairs.
class XMLSECURITY_DLLPUBLIC PDFDictionaryElement : public PDFElement
{
    /// Key-value pairs when the dictionary is a nested value.
    std::map<OString, PDFElement*> m_aItems;
    /// Offset after the '<<' token.
    sal_uInt64 m_nLocation = 0;
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
    /// Looks up an object which is only referenced in this dictionary.
    PDFObjectElement* LookupObject(const OString& rDictionaryKey);
    /// Looks up an element which is contained in this dictionary.
    PDFElement* LookupElement(const OString& rDictionaryKey);
};

enum class TokenizeMode
{
    /// Full file.
    END_OF_STREAM,
    /// Till the first %%EOF token.
    EOF_TOKEN,
    /// Till the end of the current object.
    END_OF_OBJECT,
    /// Same as END_OF_OBJECT, but for object streams (no endobj keyword).
    STORED_OBJECT
};

/// The type column of an entry in a cross-reference stream.
enum class XRefEntryType
{
    /// xref "f" or xref stream "0".
    FREE,
    /// xref "n" or xref stream "1".
    NOT_COMPRESSED,
    /// xref stream "2.
    COMPRESSED
};

/// An entry in a cross-reference stream.
struct XRefEntry
{
    XRefEntryType m_eType;
    /**
     * Non-compressed: The byte offset of the object, starting from the
     * beginning of the file.
     * Compressed: The object number of the object stream in which this object is
     * stored.
     */
    sal_uInt64 m_nOffset;
    /**
     * Non-compressed: The generation number of the object.
     * Compressed: The index of this object within the object stream.
     */
    sal_uInt64 m_nGenerationNumber;
    /// Are changed as part of an incremental update?.
    bool m_bDirty;

    XRefEntry();
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
    std::map<size_t, XRefEntry> m_aXRef;
    /// Object offset <-> Object pointer map.
    std::map<size_t, PDFObjectElement*> m_aOffsetObjects;
    /// Object ID <-> Object pointer map.
    std::map<size_t, PDFObjectElement*> m_aIDObjects;
    /// List of xref offsets we know.
    std::vector<size_t> m_aStartXRefs;
    /// List of EOF offsets we know.
    std::vector<size_t> m_aEOFs;
    PDFTrailerElement* m_pTrailer;
    /// When m_pTrailer is nullptr, this can still have a dictionary.
    PDFObjectElement* m_pXRefStream;
    /// All editing takes place in this buffer, if it happens.
    SvMemoryStream m_aEditBuffer;

    static int AsHex(char ch);
    /// Suggest a minimal, yet free signature ID to use for the next signature.
    sal_uInt32 GetNextSignature();
    /// Write the signature object as part of signing.
    sal_Int32 WriteSignatureObject(const OUString& rDescription, bool bAdES, sal_uInt64& rLastByteRangeOffset, sal_Int64& rSignatureContentOffset);
    /// Write the appearance object as part of signing.
    sal_Int32 WriteAppearanceObject();
    /// Write the annot object as part of signing.
    sal_Int32 WriteAnnotObject(PDFObjectElement& rFirstPage, sal_Int32 nSignatureId, sal_Int32 nAppearanceId);
    /// Write the updated Page object as part of signing.
    bool WritePageObject(PDFObjectElement& rFirstPage, sal_Int32 nAnnotId);
    /// Write the updated Catalog object as part of signing.
    bool WriteCatalogObject(sal_Int32 nAnnotId, PDFReferenceElement*& pRoot);
    /// Write the updated cross-references as part of signing.
    void WriteXRef(sal_uInt64 nXRefOffset, PDFReferenceElement* pRoot);

public:
    PDFDocument();
    PDFDocument& operator=(const PDFDocument&) = delete;
    PDFDocument(const PDFDocument&) = delete;
    /// @name Low-level functions, to be used by PDFElement subclasses.
    //@{
    /// Decode a hex dump.
    static std::vector<unsigned char> DecodeHexString(PDFHexStringElement* pElement);
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
    /// Look up object based on object number, possibly by parsing object streams.
    PDFObjectElement* LookupObject(size_t nObjectNumber);
    /// Access to the input document, even after the input stream is gone.
    SvMemoryStream& GetEditBuffer();
    /// Tokenize elements from current offset.
    bool Tokenize(SvStream& rStream, TokenizeMode eMode, std::vector< std::unique_ptr<PDFElement> >& rElements, PDFObjectElement* pObject);
    /// Register an object (owned directly or indirectly by m_aElements) as a provider for a given ID.
    void SetIDObject(size_t nID, PDFObjectElement* pObject);
    //@}

    /// @name High-level functions, to be used by others.
    //@{
    /// Read elements from the start of the stream till its end.
    bool Read(SvStream& rStream);
    /// Sign the read document with xCertificate in the edit buffer.
    bool Sign(const css::uno::Reference<css::security::XCertificate>& xCertificate, const OUString& rDescription, bool bAdES);
    /// Serializes the contents of the edit buffer.
    bool Write(SvStream& rStream);
    /// Get a list of signatures embedded into this document.
    std::vector<PDFObjectElement*> GetSignatureWidgets();
    /// Remove the nth signature from read document in the edit buffer.
    bool RemoveSignature(size_t nPosition);
    //@}
};

/**
 * @param rInformation The actual result.
 * @param bLast If this is the last signature in the file, so it covers the whole file physically.
 * @return If we can determinate a result.
 */
XMLSECURITY_DLLPUBLIC bool ValidateSignature(SvStream& rStream, PDFObjectElement* pSignature, SignatureInformation& rInformation, bool bLast);

} // namespace pdfio
} // namespace xmlsecurity

#endif // INCLUDED_XMLSECURITY_PDFIO_PDFDOCUMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_FILTER_PDFDOCUMENT_HXX
#define INCLUDED_VCL_FILTER_PDFDOCUMENT_HXX

#include <memory>
#include <map>
#include <vector>

#include <tools/stream.hxx>

#include <vcl/dllapi.h>

namespace com
{
namespace sun
{
namespace star
{
namespace security
{
class XCertificate;
}
}
}
}

namespace com
{
namespace sun
{
namespace star
{
namespace uno
{
template <class interface_type> class Reference;
}
}
}
}

namespace vcl
{
namespace filter
{
class PDFTrailerElement;
class PDFReferenceElement;
class PDFDocument;
class PDFDictionaryElement;
class PDFArrayElement;
class PDFStreamElement;
class PDFNumberElement;

/// A byte range in a PDF file.
class VCL_DLLPUBLIC PDFElement
{
    bool m_bVisiting = false;
    bool m_bParsing = false;

public:
    PDFElement() = default;
    virtual bool Read(SvStream& rStream) = 0;
    virtual ~PDFElement() = default;
    void setVisiting(bool bVisiting) { m_bVisiting = bVisiting; }
    bool alreadyVisiting() const { return m_bVisiting; }
    void setParsing(bool bParsing) { m_bParsing = bParsing; }
    bool alreadyParsing() const { return m_bParsing; }
};

/// Indirect object: something with a unique ID.
class VCL_DLLPUBLIC PDFObjectElement : public PDFElement
{
    /// The document owning this element.
    PDFDocument& m_rDoc;
    double m_fObjectValue;
    double m_fGenerationValue;
    std::map<OString, PDFElement*> m_aDictionary;
    /// If set, the object contains this number element (outside any dictionary/array).
    PDFNumberElement* m_pNumberElement;
    /// Position after the '<<' token.
    sal_uInt64 m_nDictionaryOffset;
    /// Length of the dictionary buffer till (before) the '>>' token.
    sal_uInt64 m_nDictionaryLength;
    PDFDictionaryElement* m_pDictionaryElement;
    /// Position after the '[' token, if m_pArrayElement is set.
    sal_uInt64 m_nArrayOffset;
    /// Length of the array buffer till (before) the ']' token.
    sal_uInt64 m_nArrayLength;
    /// The contained direct array, if any.
    PDFArrayElement* m_pArrayElement;
    /// The stream of this object, used when this is an object stream.
    PDFStreamElement* m_pStreamElement;
    /// Objects of an object stream.
    std::vector<std::unique_ptr<PDFObjectElement>> m_aStoredElements;
    /// Elements of an object in an object stream.
    std::vector<std::unique_ptr<PDFElement>> m_aElements;
    /// Uncompressed buffer of an object in an object stream.
    std::unique_ptr<SvMemoryStream> m_pStreamBuffer;
    /// List of all reference elements inside this object's dictionary and
    /// nested dictionaries.
    std::vector<PDFReferenceElement*> m_aDictionaryReferences;

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
    PDFDictionaryElement* GetDictionary();
    void SetDictionary(PDFDictionaryElement* pDictionaryElement);
    void SetNumberElement(PDFNumberElement* pNumberElement);
    PDFNumberElement* GetNumberElement() const;
    /// Get access to the parsed key-value items from the object dictionary.
    const std::map<OString, PDFElement*>& GetDictionaryItems();
    const std::vector<PDFReferenceElement*>& GetDictionaryReferences() const;
    void AddDictionaryReference(PDFReferenceElement* pReference);
    void SetArray(PDFArrayElement* pArrayElement);
    void SetStream(PDFStreamElement* pStreamElement);
    /// Access to the stream of the object, if it has any.
    PDFStreamElement* GetStream() const;
    void SetArrayOffset(sal_uInt64 nArrayOffset);
    sal_uInt64 GetArrayOffset();
    void SetArrayLength(sal_uInt64 nArrayLength);
    sal_uInt64 GetArrayLength();
    PDFArrayElement* GetArray() const;
    /// Parse objects stored in this object stream.
    void ParseStoredObjects();
    std::vector<std::unique_ptr<PDFElement>>& GetStoredElements();
    SvMemoryStream* GetStreamBuffer() const;
    void SetStreamBuffer(std::unique_ptr<SvMemoryStream>& pStreamBuffer);
    PDFDocument& GetDocument();
};

/// Array object: a list.
class VCL_DLLPUBLIC PDFArrayElement : public PDFElement
{
    std::vector<PDFElement*> m_aElements;
    /// The object that contains this array.
    PDFObjectElement* const m_pObject;

public:
    PDFArrayElement(PDFObjectElement* pObject);
    bool Read(SvStream& rStream) override;
    void PushBack(PDFElement* pElement);
    const std::vector<PDFElement*>& GetElements();
};

/// Reference object: something with a unique ID.
class VCL_DLLPUBLIC PDFReferenceElement : public PDFElement
{
    PDFDocument& m_rDoc;
    int m_fObjectValue;
    int m_fGenerationValue;
    /// Location after the 'R' token.
    sal_uInt64 m_nOffset = 0;
    /// The element providing the object number.
    PDFNumberElement& m_rObject;

public:
    PDFReferenceElement(PDFDocument& rDoc, PDFNumberElement& rObject,
                        PDFNumberElement const& rGeneration);
    bool Read(SvStream& rStream) override;
    /// Assuming the reference points to a number object, return its value.
    double LookupNumber(SvStream& rStream) const;
    /// Lookup referenced object, without assuming anything about its contents.
    PDFObjectElement* LookupObject();
    int GetObjectValue() const;
    int GetGenerationValue() const;
    sal_uInt64 GetOffset() const;
    PDFNumberElement& GetObjectElement() const;
};

/// Stream object: a byte array with a known length.
class VCL_DLLPUBLIC PDFStreamElement : public PDFElement
{
    size_t const m_nLength;
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
class VCL_DLLPUBLIC PDFNameElement : public PDFElement
{
    OString m_aValue;
    /// Offset after the '/' token.
    sal_uInt64 m_nLocation = 0;

public:
    PDFNameElement();
    bool Read(SvStream& rStream) override;
    const OString& GetValue() const;
    sal_uInt64 GetLocation() const;
    static sal_uInt64 GetLength() { return 0; }
};

/// Dictionary object: a set key-value pairs.
class VCL_DLLPUBLIC PDFDictionaryElement : public PDFElement
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

    static size_t Parse(const std::vector<std::unique_ptr<PDFElement>>& rElements,
                        PDFElement* pThis, std::map<OString, PDFElement*>& rDictionary);
    static PDFElement* Lookup(const std::map<OString, PDFElement*>& rDictionary,
                              const OString& rKey);
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
    /// xref stream "2".
    COMPRESSED
};

/// An entry in a cross-reference stream.
class XRefEntry
{
    XRefEntryType m_eType = XRefEntryType::NOT_COMPRESSED;
    /**
     * Non-compressed: The byte offset of the object, starting from the
     * beginning of the file.
     * Compressed: The object number of the object stream in which this object is
     * stored.
     */
    sal_uInt64 m_nOffset = 0;
    /// Are changed as part of an incremental update?.
    bool m_bDirty = false;

public:
    XRefEntry();

    void SetType(XRefEntryType eType) { m_eType = eType; }

    XRefEntryType GetType() const { return m_eType; }

    void SetOffset(sal_uInt64 nOffset) { m_nOffset = nOffset; }

    sal_uInt64 GetOffset() const { return m_nOffset; }

    void SetDirty(bool bDirty) { m_bDirty = bDirty; }

    bool GetDirty() const { return m_bDirty; }
};

/// Hex string: in <AABB> form.
class VCL_DLLPUBLIC PDFHexStringElement : public PDFElement
{
    OString m_aValue;

public:
    bool Read(SvStream& rStream) override;
    const OString& GetValue() const;
};

/// Literal string: in (asdf) form.
class VCL_DLLPUBLIC PDFLiteralStringElement : public PDFElement
{
    OString m_aValue;

public:
    bool Read(SvStream& rStream) override;
    const OString& GetValue() const;
};

/// Numbering object: an integer or a real.
class VCL_DLLPUBLIC PDFNumberElement : public PDFElement
{
    /// Input file start location.
    sal_uInt64 m_nOffset = 0;
    /// Input file token length.
    sal_uInt64 m_nLength = 0;
    double m_fValue = 0;

public:
    PDFNumberElement();
    bool Read(SvStream& rStream) override;
    double GetValue() const;
    sal_uInt64 GetLocation() const;
    sal_uInt64 GetLength() const;
};

/**
 * In-memory representation of an on-disk PDF document.
 *
 * The PDF element list is not meant to be saved back to disk, but some
 * elements remember their source offset / length, and based on that it's
 * possible to modify the input file.
 */
class VCL_DLLPUBLIC PDFDocument
{
    /// This vector owns all elements.
    std::vector<std::unique_ptr<PDFElement>> m_aElements;
    /// Object ID <-> object offset map.
    std::map<size_t, XRefEntry> m_aXRef;
    /// Object offset <-> Object pointer map.
    std::map<size_t, PDFObjectElement*> m_aOffsetObjects;
    /// Object ID <-> Object pointer map.
    std::map<size_t, PDFObjectElement*> m_aIDObjects;
    /// List of xref offsets we know.
    std::vector<size_t> m_aStartXRefs;
    /// Offsets of trailers, from latest to oldest.
    std::vector<size_t> m_aTrailerOffsets;
    /// Trailer offset <-> Trailer pointer map.
    std::map<size_t, PDFTrailerElement*> m_aOffsetTrailers;
    /// List of EOF offsets we know.
    std::vector<size_t> m_aEOFs;
    PDFTrailerElement* m_pTrailer = nullptr;
    /// When m_pTrailer is nullptr, this can still have a dictionary.
    PDFObjectElement* m_pXRefStream = nullptr;
    /// All editing takes place in this buffer, if it happens.
    SvMemoryStream m_aEditBuffer;

    /// Suggest a minimal, yet free signature ID to use for the next signature.
    sal_uInt32 GetNextSignature();
    /// Write the signature object as part of signing.
    sal_Int32 WriteSignatureObject(const OUString& rDescription, bool bAdES,
                                   sal_uInt64& rLastByteRangeOffset, sal_Int64& rContentOffset);
    /// Write the appearance object as part of signing.
    sal_Int32 WriteAppearanceObject();
    /// Write the annot object as part of signing.
    sal_Int32 WriteAnnotObject(PDFObjectElement const& rFirstPage, sal_Int32 nSignatureId,
                               sal_Int32 nAppearanceId);
    /// Write the updated Page object as part of signing.
    bool WritePageObject(PDFObjectElement& rFirstPage, sal_Int32 nAnnotId);
    /// Write the updated Catalog object as part of signing.
    bool WriteCatalogObject(sal_Int32 nAnnotId, PDFReferenceElement*& pRoot);
    /// Write the updated cross-references as part of signing.
    void WriteXRef(sal_uInt64 nXRefOffset, PDFReferenceElement const* pRoot);

public:
    PDFDocument();
    PDFDocument& operator=(const PDFDocument&) = delete;
    PDFDocument(const PDFDocument&) = delete;
    /// @name Low-level functions, to be used by PDFElement subclasses.
    //@{
    /// Decode a hex dump.
    static std::vector<unsigned char> DecodeHexString(PDFHexStringElement const* pElement);
    static OString ReadKeyword(SvStream& rStream);
    static size_t FindStartXRef(SvStream& rStream);
    void ReadXRef(SvStream& rStream);
    void ReadXRefStream(SvStream& rStream);
    static void SkipWhitespace(SvStream& rStream);
    /// Instead of all whitespace, just skip CR and NL characters.
    static void SkipLineBreaks(SvStream& rStream);
    size_t GetObjectOffset(size_t nIndex) const;
    const std::vector<std::unique_ptr<PDFElement>>& GetElements();
    std::vector<PDFObjectElement*> GetPages();
    /// Remember the end location of an EOF token.
    void PushBackEOF(size_t nOffset);
    /// Look up object based on object number, possibly by parsing object streams.
    PDFObjectElement* LookupObject(size_t nObjectNumber);
    /// Access to the input document, even after the input stream is gone.
    SvMemoryStream& GetEditBuffer();
    /// Tokenize elements from current offset.
    bool Tokenize(SvStream& rStream, TokenizeMode eMode,
                  std::vector<std::unique_ptr<PDFElement>>& rElements,
                  PDFObjectElement* pObjectElement);
    /// Register an object (owned directly or indirectly by m_aElements) as a provider for a given ID.
    void SetIDObject(size_t nID, PDFObjectElement* pObject);
    //@}

    /// @name High-level functions, to be used by others.
    //@{
    /// Read elements from the start of the stream till its end.
    bool Read(SvStream& rStream);
    /// Sign the read document with xCertificate in the edit buffer.
    bool Sign(const css::uno::Reference<css::security::XCertificate>& xCertificate,
              const OUString& rDescription, bool bAdES);
    /// Serializes the contents of the edit buffer.
    bool Write(SvStream& rStream);
    /// Get a list of signatures embedded into this document.
    std::vector<PDFObjectElement*> GetSignatureWidgets();
    /// Remove the nth signature from read document in the edit buffer.
    bool RemoveSignature(size_t nPosition);
    //@}
};

} // namespace pdfio
} // namespace xmlsecurity

#endif // INCLUDED_VCL_FILTER_PDFDOCUMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

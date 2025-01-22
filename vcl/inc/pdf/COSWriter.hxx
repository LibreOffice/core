/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <pdf/IPDFEncryptor.hxx>
#include <vcl/pdfwriter.hxx>

namespace vcl::pdf
{
/** Writes the "Carousel" object structure (COS) to the string buffer.
 *
 * "Carousel" object structure (COS) is used by PDF.
 *
 * Structure elements like: objects, IDs, dictionaries, key/values, ...
 */
class COSWriter
{
    std::shared_ptr<IPDFEncryptor> mpPDFEncryptor;
    EncryptionParams maParams;
    OStringBuffer maLine;
    OStringBuffer& mrBuffer;

    void appendLiteralString(const char* pStr, sal_Int32 nLength);

public:
    COSWriter(EncryptionParams aParams = EncryptionParams(),
              std::shared_ptr<IPDFEncryptor> const& pPDFEncryptor = nullptr)
        : mpPDFEncryptor(pPDFEncryptor)
        , maParams(aParams)
        , maLine(1024)
        , mrBuffer(maLine)
    {
    }

    COSWriter(OStringBuffer& rBuffer, EncryptionParams aParams = EncryptionParams(),
              std::shared_ptr<IPDFEncryptor> const& pPDFEncryptor = nullptr)
        : mpPDFEncryptor(pPDFEncryptor)
        , maParams(aParams)
        , mrBuffer(rBuffer)
    {
    }

    void startObject(sal_Int32 nObjectID)
    {
        mrBuffer.append(nObjectID);
        mrBuffer.append(" 0 obj\n");
    }

    void endObject() { mrBuffer.append("endobj\n\n"); }

    OStringBuffer& getLine() { return mrBuffer; }

    void startDict() { mrBuffer.append("<<"); }
    void startDictWithKey(std::string_view key)
    {
        mrBuffer.append(key);
        mrBuffer.append("<<");
    }
    void endDict() { mrBuffer.append(">>\n"); }

    void startStream() { mrBuffer.append("stream\n"); }
    void endStream() { mrBuffer.append("\nendstream\n"); }

    void write(std::string_view key, std::string_view value)
    {
        mrBuffer.append(key);
        mrBuffer.append(value);
    }

    void write(std::string_view key, sal_Int32 value)
    {
        mrBuffer.append(key);
        mrBuffer.append(" ");
        mrBuffer.append(value);
    }

    void writeReference(sal_Int32 nObjectID)
    {
        mrBuffer.append(nObjectID);
        mrBuffer.append(" 0 R");
    }

    void writeKeyAndReference(std::string_view key, sal_Int32 nObjectID)
    {
        mrBuffer.append(key);
        mrBuffer.append(" ");
        writeReference(nObjectID);
    }

    void writeKeyAndUnicode(std::string_view key, OUString const& rString)
    {
        mrBuffer.append(key);
        writeUnicode(rString);
    }

    void writeUnicode(OUString const& rString);

    void writeKeyAndUnicodeEncrypt(std::string_view key, OUString const& rString, sal_Int32 nObject)
    {
        mrBuffer.append(key);
        writeUnicodeEncrypt(rString, nObject);
    }

    void writeUnicodeEncrypt(OUString const& rString, sal_Int32 nObject);

    void writeLiteral(std::string_view value)
    {
        mrBuffer.append("(");
        appendLiteralString(value.data(), value.size());
        mrBuffer.append(")");
    }

    void writeKeyAndLiteral(std::string_view key, std::string_view value)
    {
        mrBuffer.append(key);
        writeLiteral(value);
    }

    void writeLiteralEncrypt(std::u16string_view value, sal_Int32 nObject,
                             rtl_TextEncoding nEncoding = RTL_TEXTENCODING_ASCII_US);

    void writeKeyAndLiteralEncrypt(std::string_view key, std::u16string_view value,
                                   sal_Int32 nObject,
                                   rtl_TextEncoding nEncoding = RTL_TEXTENCODING_ASCII_US)
    {
        mrBuffer.append(key);
        writeLiteralEncrypt(value, nObject, nEncoding);
    }

    void writeLiteralEncrypt(std::string_view value, sal_Int32 nObject);

    void writeKeyAndLiteralEncrypt(std::string_view key, std::string_view value, sal_Int32 nObject)
    {
        mrBuffer.append(key);
        writeLiteralEncrypt(value, nObject);
    }

    void writeHexArray(std::string_view key, sal_uInt8* pData, size_t nSize)
    {
        mrBuffer.append(key);
        mrBuffer.append("<");
        COSWriter::appendHexArray(pData, nSize, mrBuffer);
        mrBuffer.append(">");
    }

    template <typename T> static void appendHex(T nValue, OStringBuffer& rBuffer)
    {
        static constexpr const auto constHexDigits = std::to_array<char>(
            { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' });
        rBuffer.append(constHexDigits[(nValue >> 4) & 15]);
        rBuffer.append(constHexDigits[nValue & 15]);
    }

    static void appendHexArray(sal_uInt8* pArray, size_t nSize, OStringBuffer& rBuffer)
    {
        for (size_t i = 0; i < nSize; i++)
            appendHex(pArray[i], rBuffer);
    }

    static void appendUnicodeTextString(const OUString& rString, OStringBuffer& rBuffer);
    static void appendName(std::u16string_view rString, OStringBuffer& rBuffer);
    static void appendName(const char* pString, OStringBuffer& rBuffer);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

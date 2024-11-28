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

    void appendLiteralString(const char* pStr, sal_Int32 nLength)
    {
        while (nLength)
        {
            switch (*pStr)
            {
                case '\n':
                    mrBuffer.append("\\n");
                    break;
                case '\r':
                    mrBuffer.append("\\r");
                    break;
                case '\t':
                    mrBuffer.append("\\t");
                    break;
                case '\b':
                    mrBuffer.append("\\b");
                    break;
                case '\f':
                    mrBuffer.append("\\f");
                    break;
                case '(':
                case ')':
                case '\\':
                    mrBuffer.append("\\");
                    mrBuffer.append(static_cast<char>(*pStr));
                    break;
                default:
                    mrBuffer.append(static_cast<char>(*pStr));
                    break;
            }
            pStr++;
            nLength--;
        }
    }

    template <typename T> static void appendHex(T nValue, OStringBuffer& rBuffer)
    {
        static constexpr const auto constHexDigits = std::to_array<char>(
            { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' });
        rBuffer.append(constHexDigits[(nValue >> 4) & 15]);
        rBuffer.append(constHexDigits[nValue & 15]);
    }

    void appendHexArray(sal_uInt8* pArray, size_t nSize)
    {
        for (size_t i = 0; i < nSize; i++)
            appendHex(pArray[i], mrBuffer);
    }

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

    void writeUnicode(OUString const& rString)
    {
        mrBuffer.append("<");

        mrBuffer.append("FEFF");
        const sal_Unicode* pString = rString.getStr();
        size_t nLength = rString.getLength();
        for (size_t i = 0; i < nLength; i++)
        {
            sal_Unicode aChar = pString[i];
            appendHex(sal_Int8(aChar >> 8), mrBuffer);
            appendHex(sal_Int8(aChar & 255), mrBuffer);
        }

        mrBuffer.append(">");
    }

    void writeKeyAndUnicodeEncrypt(std::string_view key, OUString const& rString, sal_Int32 nObject)
    {
        mrBuffer.append(key);
        writeUnicodeEncrypt(rString, nObject);
    }

    void writeUnicodeEncrypt(OUString const& rString, sal_Int32 nObject)
    {
        if (maParams.mbCanEncrypt && mpPDFEncryptor)
        {
            mrBuffer.append("<");
            const sal_Unicode* pString = rString.getStr();
            size_t nLength = rString.getLength();
            //prepare a unicode string, encrypt it
            mpPDFEncryptor->setupEncryption(maParams.maKey, nObject);
            sal_Int32 nChars = 2 + (nLength * 2);
            std::vector<sal_uInt8> aEncryptionBuffer(nChars);
            sal_uInt8* pCopy = aEncryptionBuffer.data();
            *pCopy++ = 0xFE;
            *pCopy++ = 0xFF;
            // we need to prepare a byte stream from the unicode string buffer
            for (size_t i = 0; i < nLength; i++)
            {
                sal_Unicode aUnicodeChar = pString[i];
                *pCopy++ = sal_uInt8(aUnicodeChar >> 8);
                *pCopy++ = sal_uInt8(aUnicodeChar & 255);
            }
            std::vector<sal_uInt8> aNewBuffer(nChars);
            mpPDFEncryptor->encrypt(aEncryptionBuffer.data(), nChars, aNewBuffer, nChars);
            //now append, hexadecimal (appendHex), the encrypted result
            appendHexArray(aNewBuffer.data(), aNewBuffer.size());
            mrBuffer.append(">");
        }
        else
        {
            writeUnicode(rString);
        }
    }

    void writeLiteral(std::string_view value)
    {
        mrBuffer.append("(");
        appendLiteralString(value.data(), value.size());
        mrBuffer.append(")");
    }

    void writeLiteralEncrypt(std::u16string_view value, sal_Int32 nObject,
                             rtl_TextEncoding nEncoding = RTL_TEXTENCODING_ASCII_US)
    {
        OString aBufferString(OUStringToOString(value, nEncoding));
        sal_Int32 nLength = aBufferString.getLength();
        OStringBuffer aBuffer(nLength);
        const char* pT = aBufferString.getStr();

        for (sal_Int32 i = 0; i < nLength; i++, pT++)
        {
            if ((*pT & 0x80) == 0)
                aBuffer.append(*pT);
            else
            {
                aBuffer.append('<');
                appendHex(*pT, aBuffer);
                aBuffer.append('>');
            }
        }
        writeLiteralEncrypt(aBuffer.makeStringAndClear(), nObject);
    }

    void writeLiteralEncrypt(std::string_view value, sal_Int32 nObject)
    {
        if (maParams.mbCanEncrypt && mpPDFEncryptor)
        {
            mrBuffer.append("(");
            size_t nChars = value.size();
            std::vector<sal_uInt8> aEncryptionBuffer(nChars);
            mpPDFEncryptor->setupEncryption(maParams.maKey, nObject);
            mpPDFEncryptor->encrypt(value.data(), nChars, aEncryptionBuffer, nChars);
            appendLiteralString(reinterpret_cast<char*>(aEncryptionBuffer.data()),
                                aEncryptionBuffer.size());
            mrBuffer.append(")");
        }
        else
        {
            writeLiteral(value);
        }
    }

    void writeKeyAndLiteralEncrypt(std::string_view key, std::string_view value, sal_Int32 nObject)
    {
        mrBuffer.append(key);
        mrBuffer.append(" ");
        writeLiteralEncrypt(value, nObject);
    }

    void writeHexArray(std::string_view key, sal_uInt8* pData, size_t nSize)
    {
        mrBuffer.append(key);
        mrBuffer.append(" <");
        appendHexArray(pData, nSize);
        mrBuffer.append(">");
    }

    static void appendUnicodeTextString(const OUString& rString, OStringBuffer& rBuffer)
    {
        rBuffer.append("FEFF");
        const sal_Unicode* pString = rString.getStr();
        size_t nLength = rString.getLength();
        for (size_t i = 0; i < nLength; i++)
        {
            sal_Unicode aChar = pString[i];
            COSWriter::appendHex(sal_Int8(aChar >> 8), rBuffer);
            COSWriter::appendHex(sal_Int8(aChar & 255), rBuffer);
        }
    }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

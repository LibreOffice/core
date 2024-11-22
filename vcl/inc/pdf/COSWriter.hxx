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
    OStringBuffer maLine;

    void appendLiteralString(const char* pStr, sal_Int32 nLength)
    {
        while (nLength)
        {
            switch (*pStr)
            {
                case '\n':
                    maLine.append("\\n");
                    break;
                case '\r':
                    maLine.append("\\r");
                    break;
                case '\t':
                    maLine.append("\\t");
                    break;
                case '\b':
                    maLine.append("\\b");
                    break;
                case '\f':
                    maLine.append("\\f");
                    break;
                case '(':
                case ')':
                case '\\':
                    maLine.append("\\");
                    maLine.append(static_cast<char>(*pStr));
                    break;
                default:
                    maLine.append(static_cast<char>(*pStr));
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
            appendHex(pArray[i], maLine);
    }

public:
    COSWriter(std::shared_ptr<IPDFEncryptor> const& pPDFEncryptor = nullptr)
        : mpPDFEncryptor(pPDFEncryptor)
        , maLine(1024)
    {
    }

    void startObject(sal_Int32 nObjectID)
    {
        maLine.append(nObjectID);
        maLine.append(" 0 obj\n");
    }

    void endObject() { maLine.append("endobj\n\n"); }

    OStringBuffer& getLine() { return maLine; }

    void startDict() { maLine.append("<<"); }
    void endDict() { maLine.append(">>\n"); }

    void startStream() { maLine.append("stream\n"); }
    void endStream() { maLine.append("\nendstream\n"); }

    void write(std::string_view key, std::string_view value)
    {
        maLine.append(key);
        maLine.append(value);
    }

    void write(std::string_view key, sal_Int32 value)
    {
        maLine.append(key);
        maLine.append(" ");
        maLine.append(value);
    }

    void writeReference(std::string_view key, sal_Int32 nObjectID)
    {
        maLine.append(key);
        maLine.append(" ");
        maLine.append(nObjectID);
        maLine.append(" 0 R");
    }

    void writeKeyAndUnicode(std::string_view key, OUString const& rString)
    {
        maLine.append(key);
        maLine.append("<");
        appendUnicodeTextString(rString, maLine);
        maLine.append(">");
    }

    void writeKeyAndUnicodeEncrypt(std::string_view key, OUString const& rString, sal_Int32 nObject,
                                   bool bEncrypt, std::vector<sal_uInt8>& rKey)
    {
        if (bEncrypt && mpPDFEncryptor)
        {
            maLine.append(key);
            maLine.append("<");
            const sal_Unicode* pString = rString.getStr();
            size_t nLength = rString.getLength();
            //prepare a unicode string, encrypt it
            mpPDFEncryptor->setupEncryption(rKey, nObject);
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
            maLine.append(">");
        }
        else
        {
            writeKeyAndUnicode(key, rString);
        }
    }

    void writeKeyAndLiteral(std::string_view key, std::string_view value)
    {
        maLine.append(key);
        maLine.append("(");
        appendLiteralString(value.data(), value.size());
        maLine.append(")");
    }

    void writeKeyAndLiteralEncrypt(std::string_view key, std::string_view value, sal_Int32 nObject,
                                   bool bEncrypt, std::vector<sal_uInt8>& rKey)
    {
        if (bEncrypt && mpPDFEncryptor)
        {
            maLine.append(key);
            maLine.append("(");
            size_t nChars = value.size();
            std::vector<sal_uInt8> aEncryptionBuffer(nChars);
            mpPDFEncryptor->setupEncryption(rKey, nObject);
            mpPDFEncryptor->encrypt(value.data(), nChars, aEncryptionBuffer, nChars);
            appendLiteralString(reinterpret_cast<char*>(aEncryptionBuffer.data()),
                                aEncryptionBuffer.size());
            maLine.append(")");
        }
        else
        {
            writeKeyAndLiteral(key, value);
        }
    }

    void writeHexArray(std::string_view key, sal_uInt8* pData, size_t nSize)
    {
        maLine.append(key);
        maLine.append(" <");
        appendHexArray(pData, nSize);
        maLine.append(">");
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

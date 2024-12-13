/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <pdf/COSWriter.hxx>
#include <rtl/ustring.hxx>

namespace vcl::pdf
{
void COSWriter::appendLiteralString(const char* pStr, sal_Int32 nLength)
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

void COSWriter::writeUnicode(OUString const& rString)
{
    mrBuffer.append("<");
    COSWriter::appendUnicodeTextString(rString, mrBuffer);
    mrBuffer.append(">");
}

void COSWriter::writeUnicodeEncrypt(OUString const& rString, sal_Int32 nObject)
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

void COSWriter::writeLiteralEncrypt(std::u16string_view value, sal_Int32 nObject,
                                    rtl_TextEncoding nEncoding)
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

void COSWriter::writeLiteralEncrypt(std::string_view value, sal_Int32 nObject)
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

void COSWriter::appendUnicodeTextString(const OUString& rString, OStringBuffer& rBuffer)
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

} //end vcl::pdf

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

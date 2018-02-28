/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "rowinputbinary.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/io/WrongFormatException.hpp>
#include <com/sun/star/io/XConnectable.hpp>

#include <unotools/ucbstreamhelper.hxx>
#include <tools/stream.hxx>

namespace dbahsql
{
using namespace css::uno;
using namespace css::sdbc;
using namespace css::io;

typedef std::vector<sal_Int32> ColumnTypeVector;

HsqlRowInputStream::HsqlRowInputStream() {}

void HsqlRowInputStream::setInputStream(Reference<XInputStream>& rStream)
{
    m_pStream.reset(utl::UcbStreamHelper::CreateStream(rStream, true));
    m_pStream->SetEndian(SvStreamEndian::BIG);
}

SvStream* HsqlRowInputStream::getInputStream() const { return m_pStream.get(); }

void HsqlRowInputStream::seek(sal_Int32 nPos) { m_pStream->Seek(nPos); }

OUString HsqlRowInputStream::readString()
{
    sal_Int32 nLen = 0;
    m_pStream->ReadInt32(nLen);
    return readUTF(nLen);
}

OUString HsqlRowInputStream::readUTF(sal_Int32 nUTFLen)
{
    Sequence<sal_Unicode> aBuffer(nUTFLen);
    sal_Unicode* pStr = aBuffer.getArray();

    sal_Int32 nCount = 0;
    sal_Int32 nStrLen = 0;
    while (nCount < nUTFLen)
    {
        unsigned char cIn = 0;
        m_pStream->ReadUChar(cIn);
        sal_uInt8 c = reinterpret_cast<sal_uInt8&>(cIn);
        sal_uInt8 char2, char3;
        switch (c >> 4)
        {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                // 0xxxxxxx
                nCount++;
                pStr[nStrLen++] = c;
                break;

            case 12:
            case 13:
                // 110x xxxx   10xx xxxx
                nCount += 2;
                if (nCount > nUTFLen)
                {
                    throw WrongFormatException();
                }

                m_pStream->ReadUChar(cIn);
                char2 = reinterpret_cast<sal_uInt8&>(cIn);
                if ((char2 & 0xC0) != 0x80)
                {
                    throw WrongFormatException();
                }

                pStr[nStrLen++] = (sal_Unicode(c & 0x1F) << 6) | (char2 & 0x3F);
                break;

            case 14:
                // 1110 xxxx  10xx xxxx  10xx xxxx
                nCount += 3;
                if (nCount > nUTFLen)
                {
                    throw WrongFormatException();
                }

                m_pStream->ReadUChar(cIn);
                char2 = reinterpret_cast<sal_uInt8&>(cIn);
                m_pStream->ReadUChar(cIn);
                char3 = reinterpret_cast<sal_uInt8&>(cIn);

                if (((char2 & 0xC0) != 0x80) || ((char3 & 0xC0) != 0x80))
                {
                    throw WrongFormatException();
                }
                pStr[nStrLen++] = (sal_Unicode(c & 0x0F) << 12) | (sal_Unicode(char2 & 0x3F) << 6)
                                  | (char3 & 0x3F);
                break;

            default:
                // 10xx xxxx,  1111 xxxx
                throw WrongFormatException();
        }
    }
    return OUString(pStr, nStrLen);
}

bool HsqlRowInputStream::checkNull()
{
    unsigned char cIn = 0;
    m_pStream->ReadUChar(cIn);
    sal_uInt8 nNull = reinterpret_cast<sal_uInt8&>(cIn);
    return nNull == 0;
}

std::vector<Any> HsqlRowInputStream::readOneRow(const ColumnTypeVector& nColTypes)
{
    auto nLen = nColTypes.size();
    std::vector<Any> aData;

    for (size_t i = 0; i < nLen; ++i)
    {
        if (checkNull())
        {
            aData.push_back(Any());
            continue;
        }

        sal_Int32 nType = nColTypes[i];

        // TODO throw error on EoF

        switch (nType)
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::LONGVARCHAR:
                aData.push_back(makeAny(readString()));
                break;
            case DataType::TINYINT:
            case DataType::SMALLINT:
            {
                sal_Int16 value = 0;
                m_pStream->ReadInt16(value);
                aData.push_back(makeAny(value));
            }
            break;
            case DataType::INTEGER:
            {
                sal_Int32 value = 0;
                m_pStream->ReadInt32(value);
                aData.push_back(makeAny(value));
            }
            break;
            case DataType::BIGINT:
            {
                sal_Int64 value = 0;
                m_pStream->ReadInt64(value);
                aData.push_back(makeAny(value));
            }
            break;
            case DataType::REAL:
            case DataType::FLOAT:
            case DataType::DOUBLE:
            {
                double value = 0;
                m_pStream->ReadDouble(value);
                // FIXME double is not necessarily 4 bytes
                aData.push_back(makeAny(value));
            }
            break;
            case DataType::NUMERIC:
            case DataType::DECIMAL:
            {
                sal_Int32 nSize = 0;
                m_pStream->ReadInt32(nSize);

                std::vector<sal_uInt8> aBytes(nSize);
                m_pStream->ReadBytes(aBytes.data(), nSize);

                // TODO make a numeric out of this.
            }
            break;
            case DataType::DATE:
                break;
            case DataType::TIME:
                break;
            case DataType::TIMESTAMP:
                break;
            case DataType::BOOLEAN:
            {
                sal_uInt8 nBool = 0;
                m_pStream->ReadUChar(nBool);
                aData.push_back(makeAny(static_cast<bool>(nBool)));
            }
            break;
            case DataType::OTHER:
                // TODO
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
            {
                sal_Int32 nSize = 0;
                m_pStream->ReadInt32(nSize);

                Sequence<sal_uInt8> aBytes(nSize);
                m_pStream->ReadBytes(aBytes.getArray(), nSize);
                aData.push_back(makeAny(aBytes));
            }
            break;

            default:
                // TODO other exception
                throw WrongFormatException();
        }
    }
    return aData;
}

} // namespace dbahsql

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

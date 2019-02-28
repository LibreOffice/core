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
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/DateTime.hpp>

#include <unotools/ucbstreamhelper.hxx>
#include <tools/stream.hxx>
#include <rtl/ustrbuf.hxx>

#include <boost/date_time/posix_time/posix_time.hpp>

namespace
{
/**
     * Converts binary represented big integer value to BCD (Binary Coded
     * Decimal), and returns a string representation of the number.
     *
     * Bytes[0] is the most significant part of the number.
     */
OUString lcl_double_dabble(const std::vector<sal_uInt8>& bytes)
{
    size_t nbits = 8 * bytes.size(); // length of array in bits
    size_t nscratch = nbits / 2; // length of scratch in bytes
    std::vector<char> scratch(nscratch, 0);

    for (size_t i = 0; i < bytes.size(); ++i)
    {
        for (size_t j = 0; j < 8; ++j)
        {
            /* This bit will be shifted in on the right. */
            int shifted_in = (bytes[i] & (1 << (7 - j))) ? 1 : 0;

            /* Add 3 everywhere that scratch[k] >= 5. */
            for (size_t k = 0; k < nscratch; ++k)
                scratch[k] += (scratch[k] >= 5) ? 3 : 0;

            /* Shift scratch to the left by one position. */
            for (size_t k = 0; k < nscratch - 1; ++k)
            {
                scratch[k] <<= 1;
                scratch[k] &= 0xF;
                scratch[k] |= (scratch[k + 1] >= 8) ? 1 : 0;
            }

            /* Shift in the new bit from arr. */
            scratch[nscratch - 1] <<= 1;
            scratch[nscratch - 1] &= 0xF;
            scratch[nscratch - 1] |= shifted_in;
        }
    }

    auto it = scratch.begin();
    /* Remove leading zeros from the scratch space. */
    while (*it == 0 && scratch.size() > 1)
    {
        it = scratch.erase(it);
    }

    /* Convert the scratch space from BCD digits to ASCII. */
    for (auto& digit : scratch)
        digit += '0';

    /* Resize and return the resulting string. */
    return OStringToOUString(OString(scratch.data(), scratch.size()), RTL_TEXTENCODING_UTF8);
}

OUString lcl_makeStringFromBigint(const std::vector<sal_uInt8>& bytes)
{
    std::vector<sal_uInt8> aBytes{ bytes };

    OUStringBuffer sRet;
    // two's complement
    if (aBytes[0] == 1)
    {
        sRet.append("-");
        for (auto& byte : aBytes)
            byte = ~byte;
        // add 1 to byte array
        // FIXME e.g. 10000 valid ?
        for (size_t i = aBytes.size() - 1; i != 0; --i)
        {
            aBytes[i] += 1;
            if (aBytes[i] != 0)
                break;
        }
    }
    // convert binary to BCD
    OUString sNum = lcl_double_dabble(aBytes);
    sRet.append(sNum);
    return sRet.makeStringAndClear();
}

OUString lcl_putDot(const OUString& sNum, sal_Int32 nScale)
{
    // e.g. sNum = "0", nScale = 2 -> "0.00"
    OUStringBuffer sBuf{ sNum };
    sal_Int32 nNullsToAppend = nScale - sNum.getLength() + 1;
    for (sal_Int32 i = 0; i < nNullsToAppend; ++i)
        sBuf.insert(0, "0");

    if (nScale > 0)
        sBuf.insert(sBuf.getLength() - 1 - nScale, ".");
    return sBuf.makeStringAndClear();
}
}

namespace dbahsql
{
using namespace css::uno;
using namespace css::sdbc;
using namespace css::io;
using namespace boost::posix_time;
using namespace boost::gregorian;

HsqlRowInputStream::HsqlRowInputStream() {}

void HsqlRowInputStream::setInputStream(Reference<XInputStream> const& rStream)
{
    m_pStream = utl::UcbStreamHelper::CreateStream(rStream, true);
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

std::vector<Any> HsqlRowInputStream::readOneRow(const std::vector<ColumnDefinition>& nColTypes)
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

        sal_Int32 nType = nColTypes[i].getDataType();

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
                assert(aBytes.size() > 0);

                sal_Int32 nScale = 0;
                m_pStream->ReadInt32(nScale);

                Sequence<Any> result(2);
                OUString sNum = lcl_makeStringFromBigint(aBytes);
                result[0] <<= lcl_putDot(sNum, nScale);
                result[1] <<= nScale;
                aData.push_back(makeAny(result));
            }
            break;
            case DataType::DATE:
            {
                sal_Int64 value = 0;
                m_pStream->ReadInt64(value); // in millisec, from 1970
                ptime epoch = time_from_string("1970-01-01 00:00:00.000");
                ptime time = epoch + milliseconds(value);
                date asDate = time.date();

                css::util::Date loDate(asDate.day(), asDate.month(),
                                       asDate.year()); // day, month, year
                aData.push_back(makeAny(loDate));
            }
            break;
            case DataType::TIME:
            {
                sal_Int64 value = 0;
                m_pStream->ReadInt64(value);
                auto valueInSecs = value / 1000;
                /* Observed valueInSecs fall in the range from
                   negative one day to positive two days.  Coerce
                   valueInSecs between zero and positive one day.*/
                const int secPerDay = 24 * 60 * 60;
                valueInSecs = (valueInSecs + secPerDay) % secPerDay;

                auto nHours = valueInSecs / (60 * 60);
                valueInSecs = valueInSecs % 3600;
                const sal_uInt16 nMins = valueInSecs / 60;
                const sal_uInt16 nSecs = valueInSecs % 60;
                css::util::Time time((value % 1000) * 1000000, nSecs, nMins, nHours, true);
                aData.push_back(makeAny(time));
            }
            break;
            case DataType::TIMESTAMP:
            {
                sal_Int64 nEpochMillis = 0;
                m_pStream->ReadInt64(nEpochMillis);
                ptime epoch = time_from_string("1970-01-01 00:00:00.000");
                ptime time = epoch + milliseconds(nEpochMillis);
                date asDate = time.date();

                sal_Int32 nNanos = 0;
                m_pStream->ReadInt32(nNanos);

                // convert into LO internal representation of dateTime
                css::util::DateTime dateTime;
                dateTime.NanoSeconds = nNanos;
                dateTime.Seconds = time.time_of_day().seconds();
                dateTime.Minutes = time.time_of_day().minutes();
                dateTime.Hours = time.time_of_day().hours();
                dateTime.Day = asDate.day();
                dateTime.Month = asDate.month();
                dateTime.Year = asDate.year();
                aData.push_back(makeAny(dateTime));
            }
            break;
            case DataType::BOOLEAN:
            {
                sal_uInt8 nBool = 0;
                m_pStream->ReadUChar(nBool);
                aData.push_back(makeAny(static_cast<bool>(nBool)));
            }
            break;
            case DataType::OTHER:
                aData.push_back(Any{}); // TODO
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
            {
                sal_Int32 nSize = 0;
                m_pStream->ReadInt32(nSize);

                Sequence<sal_Int8> aBytes(nSize);
                m_pStream->ReadBytes(aBytes.getArray(), nSize);
                aData.push_back(makeAny(aBytes));
            }
            break;

            default:
                throw WrongFormatException();
        }
    }
    return aData;
}

} // namespace dbahsql

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

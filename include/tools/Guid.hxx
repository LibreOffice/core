/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <array>
#include <rtl/uuid.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <algorithm>
#include <stdio.h>
#include <cctype>
#include <o3tl/numeric.hxx>

namespace tools
{
class Guid
{
private:
    typedef std::array<sal_uInt8, 16> GuidArrayType;

    GuidArrayType maGuidArray;

    void parse(std::string_view rString)
    {
        if (rString.size() != 38)
            return;

        if (rString[0] != '{' || rString[37] != '}' || rString[9] != '-' || rString[14] != '-'
            || rString[19] != '-' || rString[24] != '-')
            return;

        for (size_t x = 1; x <= 8; x++)
            if (!std::isxdigit(rString[x]))
                return;
        for (size_t x = 10; x <= 13; x++)
            if (!std::isxdigit(rString[x]))
                return;
        for (size_t x = 15; x <= 18; x++)
            if (!std::isxdigit(rString[x]))
                return;
        for (size_t x = 20; x <= 23; x++)
            if (!std::isxdigit(rString[x]))
                return;
        for (size_t x = 25; x <= 36; x++)
            if (!std::isxdigit(rString[x]))
                return;

        maGuidArray[0] = o3tl::convertToHex<sal_uInt8>(rString[1], rString[2]);
        maGuidArray[1] = o3tl::convertToHex<sal_uInt8>(rString[3], rString[4]);
        maGuidArray[2] = o3tl::convertToHex<sal_uInt8>(rString[5], rString[6]);
        maGuidArray[3] = o3tl::convertToHex<sal_uInt8>(rString[7], rString[8]);

        maGuidArray[4] = o3tl::convertToHex<sal_uInt8>(rString[10], rString[11]);
        maGuidArray[5] = o3tl::convertToHex<sal_uInt8>(rString[12], rString[13]);

        maGuidArray[6] = o3tl::convertToHex<sal_uInt8>(rString[15], rString[16]);
        maGuidArray[7] = o3tl::convertToHex<sal_uInt8>(rString[17], rString[18]);

        maGuidArray[8] = o3tl::convertToHex<sal_uInt8>(rString[20], rString[21]);
        maGuidArray[9] = o3tl::convertToHex<sal_uInt8>(rString[22], rString[23]);

        maGuidArray[10] = o3tl::convertToHex<sal_uInt8>(rString[25], rString[26]);
        maGuidArray[11] = o3tl::convertToHex<sal_uInt8>(rString[27], rString[28]);
        maGuidArray[12] = o3tl::convertToHex<sal_uInt8>(rString[29], rString[30]);
        maGuidArray[13] = o3tl::convertToHex<sal_uInt8>(rString[31], rString[32]);
        maGuidArray[14] = o3tl::convertToHex<sal_uInt8>(rString[33], rString[34]);
        maGuidArray[15] = o3tl::convertToHex<sal_uInt8>(rString[35], rString[36]);
    }

public:
    /// GenerateTag is used as a flag for generating the GUID
    enum GenerateTag
    {
        Generate = 0
    };

    /// Constructor which generates the GUID
    Guid(enum GenerateTag) { rtl_createUuid(maGuidArray.data(), nullptr, false); }

    /// Default constructor which initializes the values to 0 (empty GUID)
    Guid() { maGuidArray.fill(0); }

    /// parse the GUID from the string
    Guid(std::string_view rString)
    {
        maGuidArray.fill(0);
        parse(rString);
    }

    /// set the GUID from an array
    Guid(const sal_uInt8 aGuidArray[16])
    {
        std::copy(aGuidArray, aGuidArray + 16, maGuidArray.begin());
    }

    Guid(Guid&&) = delete;

    Guid(Guid const& rOther) { *this = rOther; }

    void operator=(Guid const& rOther) { std::copy(rOther.cbegin(), rOther.cend(), begin()); }

    bool isEmpty() { return *std::max_element(maGuidArray.begin(), maGuidArray.end()) == 0; }

    GuidArrayType::iterator begin() { return maGuidArray.begin(); }
    GuidArrayType::iterator end() { return maGuidArray.end(); }
    GuidArrayType::const_iterator cbegin() const { return maGuidArray.cbegin(); }
    GuidArrayType::const_iterator cend() const { return maGuidArray.cend(); }

    OString getString() const
    {
        char sBuffer[40];
        snprintf(sBuffer, sizeof(sBuffer),
                 "{%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                 maGuidArray[0], maGuidArray[1], maGuidArray[2], maGuidArray[3], maGuidArray[4],
                 maGuidArray[5], maGuidArray[6], maGuidArray[7], maGuidArray[8], maGuidArray[9],
                 maGuidArray[10], maGuidArray[11], maGuidArray[12], maGuidArray[13],
                 maGuidArray[14], maGuidArray[15]);

        return OString(sBuffer);
    }

    OUString getOUString() { return OStringToOUString(getString(), RTL_TEXTENCODING_ASCII_US); }

    bool operator==(Guid const& rCompare) const
    {
        return rtl_compareUuid(maGuidArray.data(), rCompare.maGuidArray.data()) == 0;
    }

    bool operator!=(Guid const& rCompare) const { return !(*this == rCompare); }
};

template <typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& rStream,
                                                     tools::Guid const& rGuid)
{
    OString aString = rGuid.getString();
    rStream << "GUID" << aString.getStr();
    return rStream;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

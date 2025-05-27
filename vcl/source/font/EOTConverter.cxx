/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string.h>
#include <vector>
#include <tools/vcompat.hxx>
#include <cstdio>
#include <tools/stream.hxx>
#include <vcl/font/EOTConverter.hxx>
#include <osl/endian.h>
#include <font/TTFStructure.hxx>
#include <font/TTFReader.hxx>

namespace font
{
namespace
{
// Writes padding, length and string data to font output
void writeNameTableString(font::TTFFont& rFont,
                          std::unique_ptr<NameTableHandler>& pNameTableHandler,
                          font::NameID eNameID, std::vector<sal_uInt8>& rEotOutput)
{
    sal_uInt64 nOffset = 0;
    sal_uInt16 nLength = 0;

    // Padding
    rEotOutput.push_back(0);
    rEotOutput.push_back(0);

    if (pNameTableHandler
        && pNameTableHandler->findEnglishUnicodeNameOffset(eNameID, nOffset, nLength))
    {
        // Length
        rEotOutput.push_back(sal_uInt8((nLength + 2) & 0xff));
        rEotOutput.push_back(sal_uInt8((nLength + 2) >> 8));

        OUString aString = rFont.getNameTableString(nOffset, nLength);
        for (sal_Int32 i = 0; i < aString.getLength(); i++)
        {
            sal_Unicode nUniChar = aString[i];
            rEotOutput.push_back(sal_uInt8(nUniChar & 0xff));
            rEotOutput.push_back(sal_uInt8(nUniChar >> 8));
        }
        // null terminated
        rEotOutput.push_back(sal_uInt8(0));
        rEotOutput.push_back(sal_uInt8(0));
    }
    else
    {
        // Length 0
        rEotOutput.push_back(sal_uInt8(0));
        rEotOutput.push_back(sal_uInt8(0));
    }
}
}

bool EOTConverter::convert(std::vector<sal_uInt8>& rEotOutput)
{
    font::TTFFont aFont(mrFontDataContainer);

    rEotOutput.clear();
    rEotOutput.resize(sizeof(EOTHeader));

    EOTHeader* pEot = reinterpret_cast<EOTHeader*>(rEotOutput.data());
    pEot->nFontDataSize = mrFontDataContainer.size();
    pEot->nVersion = 0x00020002;
    pEot->nFlags = 0;
    pEot->nCharset = 0;
    pEot->nMagicNumber = 0x504c;
    pEot->nReserved1 = 0;
    pEot->nReserved2 = 0;
    pEot->nReserved3 = 0;
    pEot->nReserved4 = 0;

    auto pHandler = aFont.getTableEntriesHandler();
    if (!pHandler)
        return false;

    const font::OS2Table* pOS2 = pHandler->getOS2Table();
    if (pOS2)
    {
        for (sal_uInt32 n = 0; n < 10; n++)
            pEot->nFontPANOSE[n] = pOS2->nPanose[n];

        pEot->nItalic = pOS2->nFsSelection & 0x01;
        pEot->nWeight = pOS2->nWeightClass;
        // FIXME: Should use OS2->fsType, but some TrueType fonts set it to an over-restrictive value.
        // Since ATS does not enforce this on Mac OS X, we do not enforce it either.
        pEot->nFsType = 0x0000;
        pEot->nUnicodeRange1 = pOS2->nUnicodeRange1;
        pEot->nUnicodeRange2 = pOS2->nUnicodeRange2;
        pEot->nUnicodeRange3 = pOS2->nUnicodeRange3;
        pEot->nUnicodeRange4 = pOS2->nUnicodeRange4;
        pEot->nCodePageRange1 = pOS2->nCodePageRange1;
        pEot->nCodePageRange2 = pOS2->nCodePageRange2;
    }

    const font::HeadTable* pHeadTable = pHandler->getHeadTable();
    if (pHeadTable)
    {
        pEot->nCheckSumAdjustment = pHeadTable->nCheckSumAdjustment;
    }

    auto pNameTableHandler = pHandler->getNameTableHandler();

    writeNameTableString(aFont, pNameTableHandler, font::NameID::FamilyName, rEotOutput);
    writeNameTableString(aFont, pNameTableHandler, font::NameID::SubfamilyName, rEotOutput);
    writeNameTableString(aFont, pNameTableHandler, font::NameID::Version, rEotOutput);
    writeNameTableString(aFont, pNameTableHandler, font::NameID::FullFontName, rEotOutput);

    // Padding5
    rEotOutput.push_back(0);
    rEotOutput.push_back(0);

    // Root String Size
    rEotOutput.push_back(0);
    rEotOutput.push_back(0);

    // Root String CheckSum (for size 0)
    rEotOutput.push_back(0x42);
    rEotOutput.push_back(0x53);
    rEotOutput.push_back(0x47);
    rEotOutput.push_back(0x50);

    // EUDC CodePage
    rEotOutput.push_back(0xE4);
    rEotOutput.push_back(0x04);
    rEotOutput.push_back(0x00);
    rEotOutput.push_back(0x00);

    // Padding6
    rEotOutput.push_back(0);
    rEotOutput.push_back(0);

    // Signature Size = should be 0x0000
    rEotOutput.push_back(0);
    rEotOutput.push_back(0);

    // EUDC Flags
    rEotOutput.push_back(0);
    rEotOutput.push_back(0);
    rEotOutput.push_back(0);
    rEotOutput.push_back(0);

    // EUDC Font Size = 0
    rEotOutput.push_back(0);
    rEotOutput.push_back(0);
    rEotOutput.push_back(0);
    rEotOutput.push_back(0);

    // rEOTOutput could've been reallocated - need to reinterpret that.
    pEot = reinterpret_cast<EOTHeader*>(rEotOutput.data());
    pEot->nEotSize = rEotOutput.size() + mrFontDataContainer.size();

    rEotOutput.insert(rEotOutput.end(), mrFontDataContainer.begin(), mrFontDataContainer.end());

    return true;
}

} // end font namespace

int TestEOT(const void* data, sal_uInt32 size)
{
    const sal_uInt8* bytes = static_cast<const sal_uInt8*>(data);
    std::vector<sal_uInt8> input(bytes, bytes + size);

    std::vector<sal_uInt8> aEOT;
    font::FontDataContainer aContainer(input);
    font::EOTConverter aConverter(aContainer);
    return aConverter.convert(aEOT) ? 1 : 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

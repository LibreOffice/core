/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/dllapi.h>
#include <vcl/font/FontDataContainer.hxx>

namespace font
{
#pragma pack(push, 1)

/** Main EOT Header
 *
 * See: https://www.w3.org/submissions/EOT/ */
struct EOTHeader
{
    sal_uInt32 nEotSize;
    sal_uInt32 nFontDataSize;
    sal_uInt32 nVersion;
    sal_uInt32 nFlags;
    sal_uInt8 nFontPANOSE[10];
    sal_uInt8 nCharset;
    sal_uInt8 nItalic;
    sal_uInt32 nWeight;
    sal_uInt16 nFsType;
    sal_uInt16 nMagicNumber;
    sal_uInt32 nUnicodeRange1;
    sal_uInt32 nUnicodeRange2;
    sal_uInt32 nUnicodeRange3;
    sal_uInt32 nUnicodeRange4;
    sal_uInt32 nCodePageRange1;
    sal_uInt32 nCodePageRange2;
    sal_uInt32 nCheckSumAdjustment;
    sal_uInt32 nReserved1;
    sal_uInt32 nReserved2;
    sal_uInt32 nReserved3;
    sal_uInt32 nReserved4;
    // variable length types come after this
};

#pragma pack(pop)

/** Converts TTF Font wrapped in a FontDataContainer to EOT type */
class VCL_DLLPUBLIC EOTConverter
{
private:
    font::FontDataContainer const& mrFontDataContainer;

public:
    explicit EOTConverter(font::FontDataContainer const& rFontDataContainer)
        : mrFontDataContainer(rFontDataContainer)
    {
    }

    bool convert(std::vector<sal_uInt8>& rEmbeddedOutput);
};

} // end font namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

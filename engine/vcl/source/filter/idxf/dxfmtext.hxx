/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <rtl/textenc.h>
#include <rtl/ustring.hxx>

#include <string_view>
#include <vector>

// The character format of a piece of DXF text
struct DXFTextFormat
{
    // empty for the default font
    OUString maFontName;
    // height of the capital letters in drawing units
    double mfHeight = 1.0;
    double mfWidthFactor = 1.0;
    // slant to the right in degrees
    double mfOblique = 0.0;
    bool mbBold = false;
    bool mbItalic = false;
    bool mbUnderline = false;
    bool mbOverline = false;
    bool mbStrikeout = false;
    // colour index, 0 is BYBLOCK, 256 is BYLAYER, -1 for the colour of the entity
    sal_Int32 mnColorIndex = -1;
    // 0xRRGGBB, wins over the index if not -1
    sal_Int32 mnTrueColor = -1;

    bool operator==(const DXFTextFormat&) const = default;
};

struct DXFTextRun
{
    OUString maText;
    DXFTextFormat maFormat;
};

struct DXFTextParagraph
{
    std::vector<DXFTextRun> maRuns;
};

namespace dxf
{
// Splits the text of an MTEXT entity into paragraphs of runs with their character formats,
// interpreting the inline formatting codes. rBase is the format at the start of the text,
// eEncoding is the one of the drawing, for the %%nnn codes.
std::vector<DXFTextParagraph> parseMText(std::u16string_view aText, const DXFTextFormat& rBase,
                                         rtl_TextEncoding eEncoding);

// Replaces the special character codes of DXF text: %%c, %%d, %%p, %%%, %%nnn and, with
// bEscapes, \U+XXXX and \M+nXXXX. The underline, overline and strikeout toggles %%u, %%o
// and %%k are removed.
OUString decodeSpecialCharacters(std::u16string_view aText, rtl_TextEncoding eEncoding,
                                 bool bEscapes);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

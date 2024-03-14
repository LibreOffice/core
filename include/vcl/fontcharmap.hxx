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

#ifndef INCLUDED_FONTCHARMAP_HXX
#define INCLUDED_FONTCHARMAP_HXX

#include <vcl/dllapi.h>
#include <vcl/vclenum.hxx>
#include <tools/ref.hxx>
#include <vector>

class ImplFontCharMap;
class FontCharMap;
class OutputDevice;

typedef tools::SvRef<ImplFontCharMap> ImplFontCharMapRef;
typedef tools::SvRef<FontCharMap> FontCharMapRef;

class VCL_DLLPUBLIC FontCharMap final : public SvRefBase
{
public:
    /** A new FontCharMap is created based on a "default" map, which includes
        all codepoints in the Unicode BMP range, including surrogates.
     **/
    FontCharMap();

    /** A new FontCharMap is created based on passed arguments.
     */
    SAL_DLLPRIVATE FontCharMap(bool bMicrosoftSymbolMap, std::vector<sal_UCS4> aRangeCodes);

    SAL_DLLPRIVATE virtual ~FontCharMap() override;

    /** Get the default font character map

        @returns the default font character map.
     */
    static FontCharMapRef GetDefaultMap(bool bMicrosoftSymbolMap);

    /** Determines if the font character map is the "default". The default map
        includes all codepoints in the Unicode BMP range, including surrogates.

        @returns true if default map, false if not default map.
     */
    bool                IsDefaultMap() const;

    /** Does the font character map include the UCS4 character?

        @returns true if character exists in font character map, false is not.
     */
    bool                HasChar( sal_UCS4 ) const;

    /** Returns the number of chars supported by the font, which
        are inside the unicode range from cMin to cMax (inclusive).

        @param  cMin        Lowest codepoint in range to be counted
        @param  cMax        Highest codepoint in range to be counted

        @returns number of characters in the font charmap between the two
                 codepoints.
     */
    int                 CountCharsInRange( sal_UCS4 cMin, sal_UCS4 cMax ) const;

    /** Get the number of characters in the font character map.

        @returns number of characters in the font character map.
     */
    int                 GetCharCount() const;

    /** Get the first character in the font character map.

        @returns first character in the font character map.
     */
    sal_UCS4            GetFirstChar() const;

    /** Get the last character in the font character map.

        @returns last character in the font character map.
     */
    sal_UCS4            GetLastChar() const;

    /** Get the next character in the font character map. This is important
        because character maps (e.g. the default map which holds the characters
        in the BMP plane) can have discontiguous ranges.

        @param  cChar       Character from which to find next character

        @returns next character in the font character map.
     */
    sal_UCS4            GetNextChar( sal_UCS4 cChar ) const;

    /** Get the previous character in the font character map. This is important
        because character maps (e.g. the default map which holds the characters
        in the BMP plane) can have discontiguous ranges.

        @param  cChar       Character from which to find previous character

        @returns previous character in the font character map.
     */
    sal_UCS4            GetPrevChar( sal_UCS4 cChar ) const;

    /** Get the index of a particular character in the font character map. The
        index is different from the codepoint, because font character maps can

        determine the index.

        @param  cChar       Character used to find index number

        @returns Index of character in font character map.
     */
    int                 GetIndexFromChar( sal_UCS4 cChar ) const;

    /** Get the character at a particular index in the font character map. The
        index is different from the codepoint, because font character maps can

        determine the character.

        @param  nCharIndex  Index used to find the character

        @returns Character in font character map.
     */
    sal_UCS4            GetCharFromIndex( int nCharIndex ) const;

    SAL_DLLPRIVATE bool isMicrosoftSymbolMap() const;

private:
    ImplFontCharMapRef mpImplFontCharMap;

    friend class ::OutputDevice;

    SAL_DLLPRIVATE int  findRangeIndex( sal_UCS4 ) const;

    SAL_DLLPRIVATE      FontCharMap( ImplFontCharMapRef pIFCMap );

    // prevent assignment and copy construction
                        FontCharMap( const FontCharMap& ) = delete;
    void                operator=( const FontCharMap& ) = delete;
};

#endif // INCLUDED_FONTCHARMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

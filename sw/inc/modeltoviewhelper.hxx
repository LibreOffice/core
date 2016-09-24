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

#ifndef INCLUDED_SW_INC_MODELTOVIEWHELPER_HXX
#define INCLUDED_SW_INC_MODELTOVIEWHELPER_HXX

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <o3tl/typed_flags_set.hxx>
#include <vector>

class SwTextNode;

/** Some helpers for converting model strings to view strings.

    A paragraph string does not have its fields expanded, i.e., they are
    represented by a special character inside the string with an additional
    attribute assigned to it. For some tasks (e.g., SmartTags) it is required
    to expand the fields to get the string as it appears in the view. Two
    helper functions are provided to convert model positions to view positions
    and vice versa.

           CH_TXTATR_BREAKWORD -> SwTextNode will have field attributes associated with these
                .       .
                .       .
                .       .
    AAAAA BBBBB # CCCCC # DDDDD
        | |  |    |
        | |  |    |
        | ---------
        |    |  .
        |    |  .
        |    |  .......... bounds of a hidden text character attribute
        ------
           .
           .
           .............. a range of text defined in redline region as deleted

    0000: pass through gives:                                        AAAAA BBBBB # CCCCC # DDDDD
    0001: only expanding fields gives:                               AAAAA BBBBB foo CCCCC foo DDDDD
    0010: only hiding hiddens gives:                                 AAAAA CCCCC # DDDDD
    0100: only hiding redlines gives:                                AAAABB # CCCCC # DDDDD
    0011: expanding fields + hiding hiddens gives:                   AAAAA CCCC foo DDDDD
    0101: expanding fields + hiding redlines gives:                  AAAA B foo CCCCC foo DDDDD
    0110: hiding hiddens + hiding redlines gives:                    AAAACCCC # DDDDD
    0111: expanding fields + hiding hiddens + hiding redlines gives: AAAABB foo CCCCC foo DDDDD
*/

enum class ExpandMode
{
    PassThrough    = 0x0000,
    ExpandFields   = 0x0001,
    ExpandFootnote = 0x0002,
    HideInvisible  = 0x0004,
    HideDeletions  = 0x0008,
    /// do not expand to content, but replace with zwsp
    ReplaceMode    = 0x0010,
};

namespace o3tl
{
    template<> struct typed_flags<ExpandMode> : is_typed_flags<ExpandMode, 0x001f> {};
}

class ModelToViewHelper
{
    /** For each expanded/hidden portion in the model string, there is an entry in
        the conversion map. The first value of the ConversionMapEntry points to
        the start position in the model string, the second value points to the
        associated start position in the view string. The last entry in the
        conversion map denotes the lengths of the model resp. view string.
    */
    struct ConversionMapEntry
    {
        ConversionMapEntry(sal_Int32 nModelPos, sal_Int32 nViewPos, bool bVisible)
            : m_nModelPos(nModelPos)
            , m_nViewPos(nViewPos)
            , m_bVisible(bVisible)
        {
        }
        sal_Int32 m_nModelPos;
        sal_Int32 m_nViewPos;
        bool m_bVisible;
    };
    typedef std::vector< ConversionMapEntry > ConversionMap;
    typedef std::vector<sal_Int32> Positions;

    ConversionMap m_aMap;
    /// store positions of fields and footnotes for grammar checkers
    Positions m_FieldPositions;
    Positions m_FootnotePositions;

    OUString m_aRetText;

public:

    /** This struct defines a position in the model string.

        The 'main' position is given by mnPos. If there's a field located at
        this position, mbIsField is set and mnSubPos denotes the position inside
        that field.
    */
    struct ModelPosition
    {
        sal_Int32 mnPos;
        sal_Int32 mnSubPos;
        bool mbIsField;

        ModelPosition() : mnPos(0), mnSubPos(0), mbIsField(false) {}
    };

    ModelToViewHelper(const SwTextNode &rNode,
            // defaults are appropriate for spell/grammar checking
            ExpandMode eMode = ExpandMode::ExpandFields | ExpandMode::ExpandFootnote | ExpandMode::ReplaceMode);
    ModelToViewHelper() //pass through filter, view == model
    {
    }

    /** Converts a model position into a view position

        @param nPos
            nPos denotes a position in the model string which should be
            converted. Note that converting model positions inside fields is
            not supported, therefore nPos is not of type ModelPosition.

        @return
            the position of nPos in the view string. In case the conversion
            could not be performed (e.g., because there is not ConversionMap or
            nPos is behind the last entry in the conversion map) nPos will
            be returned.
    */
    sal_Int32 ConvertToViewPosition( sal_Int32 nModelPos ) const;

    /** Converts a view position into a model position

        @param nPos
            nPos denotes a position in the view string which should be
            converted.

        @return
            the position of nPos in the model string. In case the conversion
            could not be performed (e.g., because there is not ConversionMap or
            nPos is behind the last entry in the conversion map) a model
            model position with mnPos = nPos and mnIsField = false will be
            returned.
    */
    ModelPosition ConvertToModelPosition( sal_Int32 nViewPos ) const;

    const OUString& getViewText() const { return m_aRetText; }
    Positions const& getFieldPositions() const { return m_FieldPositions; }
    Positions const& getFootnotePositions() const { return m_FootnotePositions;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

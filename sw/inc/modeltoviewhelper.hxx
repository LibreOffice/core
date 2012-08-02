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

#ifndef _MODELTOVIEWHELPER_HXX
#define _MODELTOVIEWHELPER_HXX

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <vector>

class SwTxtNode;

/** Some helpers for converting model strings to view strings.

    A paragraph string does not have its fields expanded, i.e., they are
    represented by a special character inside the string with an additional
    attribute assigned to it. For some tasks (e.g., SmartTags) it is required
    to expand the fields to get the string as it appears in the view. Two
    helper functions are provided to convert model positions to view positions
    and vice versa.

           CH_TXTATR_BREAKWORD -> SwTxtNode will have field attributes associated with these
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

#define PASSTHROUGH   0x0000
#define EXPANDFIELDS  0x0001
#define HIDEINVISIBLE 0x0002
#define HIDEREDLINED  0x0004

class ModelToViewHelper
{
    /** For each field in the model string, there is an entry in the conversion
        map. The first value of the ConversionMapEntry points to the field
        position in the model string, the second value points to the associated
        position in the view string. The last entry in the conversion map
        denotes the lengths of the model resp. view string.
    */
    typedef std::pair< sal_uInt32 , sal_uInt32 > ConversionMapEntry;
    typedef std::vector< ConversionMapEntry > ConversionMap;

    ConversionMap m_aMap;

    rtl::OUString m_aRetText;

public:

    /** This struct defines a position in the model string.

        The 'main' position is given by mnPos. If there's a field located at
        this position, mbIsField is set and mnSubPos denotes the position inside
        that field.
    */
    struct ModelPosition
    {
        sal_uInt32 mnPos;
        sal_uInt32 mnSubPos;
        bool mbIsField;

        ModelPosition() : mnPos(0), mnSubPos(0), mbIsField(false) {}
    };

    ModelToViewHelper(const SwTxtNode &rNode, int eMode = EXPANDFIELDS);
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
    sal_uInt32 ConvertToViewPosition( sal_uInt32 nModelPos ) const;

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
    ModelPosition ConvertToModelPosition( sal_uInt32 nViewPos ) const;

    rtl::OUString getViewText() const { return m_aRetText; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

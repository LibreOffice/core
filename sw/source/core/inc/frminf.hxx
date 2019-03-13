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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_FRMINF_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_FRMINF_HXX

#include <swtypes.hxx>

#include "TextFrameIndex.hxx"

#include <vector>

class SwTextFrame;
class SwTextCursor;

class SwTextFrameInfo
{
    const SwTextFrame *pFrame;

    // Where does the text (w/o whitespaces) start (document is global!)?
    static SwTwips GetLineStart( const SwTextCursor &rLine );

public:
    SwTextFrameInfo( const SwTextFrame *pTextFrame ) : pFrame(pTextFrame) { }

    // Does the paragraph fit into a single line?
    bool IsOneLine() const;

    // Is the line filled to X%?
    bool IsFilled( const sal_uInt8 nPercent ) const;

    // Where does the text (w/o whitespaces) start (rel. in frame)?
    SwTwips GetLineStart() const;

    // return center position of the next character
    SwTwips GetCharPos(TextFrameIndex nChar, bool bCenter = true) const;

    // collect all whitespaces at the beginning and end of a line in Pam
    void GetSpaces(std::vector<std::pair<TextFrameIndex, TextFrameIndex>> &,
            bool bWithLineBreak) const;

    // Is a bullet point/symbol/etc. at the first text position?
    bool IsBullet(TextFrameIndex nTextPos) const;

    // determine indentation for first line
    SwTwips GetFirstIndent() const;

    const SwTextFrame* GetFrame() const { return pFrame; }
    SwTextFrameInfo& SetFrame( const SwTextFrame* pNew )
        { pFrame = pNew; return *this; }

    // Is it a comparison? Returns position in frame.
    sal_Int32 GetBigIndent( TextFrameIndex & rFndPos,
                        const SwTextFrame *pNextFrame ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

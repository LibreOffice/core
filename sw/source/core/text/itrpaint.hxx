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

#pragma once

#include "itrtxt.hxx"

class SwSaveClip;          // SwTextPainter
class SwMultiPortion;

class SwTextPainter : public SwTextCursor
{
    bool bPaintDrop;

    SwLinePortion *CalcPaintOfst( const SwRect &rPaint );
    void CheckSpecialUnderline( const SwLinePortion* pPor,
                                long nAdjustBaseLine = 0 );
protected:
    void CtorInitTextPainter( SwTextFrame *pFrame, SwTextPaintInfo *pInf );
    explicit SwTextPainter(SwTextNode const * pTextNode)
        : SwTextCursor(pTextNode)
        , bPaintDrop(false)
    {
    }

public:
    SwTextPainter(SwTextFrame *pTextFrame, SwTextPaintInfo *pTextPaintInf)
        : SwTextCursor(pTextFrame->GetTextNodeFirst())
    {
        CtorInitTextPainter( pTextFrame, pTextPaintInf );
    }
    void DrawTextLine( const SwRect &rPaint, SwSaveClip &rClip,
                       const bool bUnderSz );
    void PaintDropPortion();
    // if PaintMultiPortion is called recursively, we have to pass the
    // surrounding SwBidiPortion
    void PaintMultiPortion( const SwRect &rPaint, SwMultiPortion& rMulti,
                            const SwMultiPortion* pEnvPor = nullptr );
    void SetPaintDrop( const bool bNew ) { bPaintDrop = bNew; }
    bool IsPaintDrop() const { return bPaintDrop; }
    SwTextPaintInfo &GetInfo()
        { return static_cast<SwTextPaintInfo&>(SwTextIter::GetInfo()); }
    const SwTextPaintInfo &GetInfo() const
        { return static_cast<const SwTextPaintInfo&>(SwTextIter::GetInfo()); }
};

bool IsUnderlineBreak( const SwLinePortion& rPor, const SwFont& rFnt );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_WIDORP_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_WIDORP_HXX
class SwTextFrame;

#include <swtypes.hxx>
#include "itrtxt.hxx"

class SwTextFrameBreak
{
private:
    SwTwips  m_nRstHeight;
    SwTwips  m_nOrigin;
protected:
    SwTextFrame *m_pFrame;
    bool     m_bBreak;
    bool     m_bKeep;
public:
    SwTextFrameBreak( SwTextFrame *pFrame, const SwTwips nRst = 0  );
    bool IsBreakNow( SwTextMargin &rLine );
    bool IsKeepAlways() const { return m_bKeep; }

    void SetKeep( const bool bNew ) { m_bKeep = bNew; }

    bool IsInside( SwTextMargin const &rLine ) const;

    // In order to be able to handle special cases with Footnote.
    // SetRstHeight sets the rest height for SwTextFrameBreak. This is needed
    // to call TruncLines() without IsBreakNow() returning another value.
    // We assume that rLine is pointing to the last non-fitting line.

    void SetRstHeight( const SwTextMargin &rLine );
};

class WidowsAndOrphans : public SwTextFrameBreak
{
private:
    sal_uInt16   m_nWidLines, m_nOrphLines;

public:
    WidowsAndOrphans( SwTextFrame *pFrame, const SwTwips nRst = 0,
        bool bCheckKeep = true );
    bool FindWidows( SwTextFrame *pFrame, SwTextMargin &rLine );
    sal_uInt16 GetOrphansLines() const
    { return m_nOrphLines; }
    void ClrOrphLines(){ m_nOrphLines = 0; }

    bool FindBreak( SwTextFrame *pFrame, SwTextMargin &rLine, bool bHasToFit );
    bool WouldFit( SwTextMargin &rLine, SwTwips &rMaxHeight, bool bTest );
    // i#16128 - This method is named this way to avoid confusion with
    // base class method <SwTextFrameBreak::IsBreakNow>, which isn't virtual.
    bool IsBreakNowWidAndOrp( SwTextMargin &rLine )
    {
        bool isOnFirstLine = (rLine.GetLineNr() == 1 && !rLine.GetPrev());
        if ( isOnFirstLine && rLine.GetCurr()->IsDummy()) {
            return IsBreakNow( rLine );
        }
        if ( rLine.GetLineNr() > m_nOrphLines ) {
            return IsBreakNow( rLine );
        }
        return false;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

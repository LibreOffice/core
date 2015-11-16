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
class SwTextFrm;

#include "swtypes.hxx"
#include "itrtxt.hxx"

class SwTextFrmBreak
{
private:
    SwTwips  m_nRstHeight;
    SwTwips  m_nOrigin;
protected:
    SwTextFrm *m_pFrm;
    bool     m_bBreak;
    bool     m_bKeep;
public:
    SwTextFrmBreak( SwTextFrm *pFrm, const SwTwips nRst = 0  );
    bool IsBreakNow( SwTextMargin &rLine );
    bool IsKeepAlways() const { return m_bKeep; }

    inline void SetKeep( const bool bNew ) { m_bKeep = bNew; }

    bool IsInside( SwTextMargin &rLine ) const;

    // In order to be able to handle special cases with Footnote.
    // SetRstHeight sets the rest height for SwTextFrmBreak. This is needed
    // to call TruncLines() without IsBreakNow() returning another value.
    // We assume that rLine is pointing to the last non-fitting line.

    // OD 2004-02-27 #106629# - no longer inline
    void SetRstHeight( const SwTextMargin &rLine );
};

class WidowsAndOrphans : public SwTextFrmBreak
{
private:
    sal_uInt16   nWidLines, nOrphLines;

public:
    WidowsAndOrphans( SwTextFrm *pFrm, const SwTwips nRst = 0,
        bool bCheckKeep = true );
    bool FindWidows( SwTextFrm *pFrm, SwTextMargin &rLine );
    sal_uInt16 GetWidowsLines() const
    { return nWidLines; }
    sal_uInt16 GetOrphansLines() const
    { return nOrphLines; }
    void ClrOrphLines(){ nOrphLines = 0; }

    bool FindBreak( SwTextFrm *pFrm, SwTextMargin &rLine, bool bHasToFit );
    bool WouldFit( SwTextMargin &rLine, SwTwips &rMaxHeight, bool bTest );
    // OD 2004-02-25 #i16128# - rename method to avoid confusion with base class
    // method <SwTextFrmBreak::IsBreakNow>, which isn't virtual.
    bool IsBreakNowWidAndOrp( SwTextMargin &rLine )
    {
        bool isOnFirstLine = (rLine.GetLineNr() == 1 && !rLine.GetPrev());
        if ( isOnFirstLine && rLine.GetCurr()->IsDummy()) {
            return IsBreakNow( rLine );
        }
        if ( rLine.GetLineNr() > nOrphLines ) {
            return IsBreakNow( rLine );
        }
        return false;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

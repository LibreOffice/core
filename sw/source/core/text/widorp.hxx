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
class SwTxtFrm;

#include "swtypes.hxx"
#include "itrtxt.hxx"

class SwTxtFrmBreak
{
private:
    SwTwips  nRstHeight;
    SwTwips  nOrigin;
protected:
    SwTxtFrm *pFrm;
    bool     bBreak;
    bool     bKeep;
public:
    SwTxtFrmBreak( SwTxtFrm *pFrm, const SwTwips nRst = 0  );
    bool IsBreakNow( SwTxtMargin &rLine );

    bool IsBroken() const     { return bBreak; }
    bool IsKeepAlways() const { return bKeep; }
    void Keep()               { bKeep = true; }
    void Break()              { bKeep = false; bBreak = true; }

    inline bool GetKeep() const { return bKeep; }
    inline void SetKeep( const bool bNew ) { bKeep = bNew; }

    bool IsInside( SwTxtMargin &rLine ) const;

    // In order to be able to handle special cases with Ftn.
    // SetRstHeight sets the rest height for SwTxtFrmBreak. This is needed
    // to call TruncLines() without IsBreakNow() returning another value.
    // We assume that rLine is pointing to the last non-fitting line.

    // OD 2004-02-27 #106629# - no longer inline
    void SetRstHeight( const SwTxtMargin &rLine );
    SwTwips GetRstHeight() const { return nRstHeight; }
};

class WidowsAndOrphans : public SwTxtFrmBreak
{
private:
    MSHORT   nWidLines, nOrphLines;

public:
    WidowsAndOrphans( SwTxtFrm *pFrm, const SwTwips nRst = 0,
        bool bCheckKeep = true );
    bool FindWidows( SwTxtFrm *pFrm, SwTxtMargin &rLine );
    MSHORT GetWidowsLines() const
    { return nWidLines; }
    MSHORT GetOrphansLines() const
    { return nOrphLines; }
    void ClrOrphLines(){ nOrphLines = 0; }

    bool FindBreak( SwTxtFrm *pFrm, SwTxtMargin &rLine, bool bHasToFit );
    bool WouldFit( SwTxtMargin &rLine, SwTwips &rMaxHeight, bool bTest );
    // OD 2004-02-25 #i16128# - rename method to avoid confusion with base class
    // method <SwTxtFrmBreak::IsBreakNow>, which isn't virtual.
    bool IsBreakNowWidAndOrp( SwTxtMargin &rLine )
    {
        return ( rLine.GetLineNr() > nOrphLines ) && IsBreakNow( rLine );
    }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

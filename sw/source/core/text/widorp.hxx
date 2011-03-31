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
#ifndef _WIDORP_HXX
#define _WIDORP_HXX
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
    sal_Bool     bBreak;
    sal_Bool     bKeep;
public:
    SwTxtFrmBreak( SwTxtFrm *pFrm, const SwTwips nRst = 0  );
    sal_Bool IsBreakNow( SwTxtMargin &rLine );

    sal_Bool IsBroken() const   { return bBreak; }
    sal_Bool IsKeepAlways() const { return bKeep; }
    void Keep()             { bKeep = sal_True; }
    void Break()                { bKeep = sal_False; bBreak = sal_True; }

    inline sal_Bool GetKeep() const { return bKeep; }
    inline void SetKeep( const sal_Bool bNew ) { bKeep = bNew; }

    sal_Bool IsInside( SwTxtMargin &rLine ) const;

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
        sal_Bool bCheckKeep = sal_True );
    sal_Bool FindWidows( SwTxtFrm *pFrm, SwTxtMargin &rLine );
    MSHORT GetWidowsLines() const
    { return nWidLines; }
    MSHORT GetOrphansLines() const
    { return nOrphLines; }
    void ClrOrphLines(){ nOrphLines = 0; }

    sal_Bool FindBreak( SwTxtFrm *pFrm, SwTxtMargin &rLine, sal_Bool bHasToFit );
    sal_Bool WouldFit( SwTxtMargin &rLine, SwTwips &rMaxHeight, sal_Bool bTest );
    // OD 2004-02-25 #i16128# - rename method to avoid confusion with base class
    // method <SwTxtFrmBreak::IsBreakNow>, which isn't virtual.
    sal_Bool IsBreakNowWidAndOrp( SwTxtMargin &rLine )
    {
        return ( rLine.GetLineNr() > nOrphLines ) && IsBreakNow( rLine );
    }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

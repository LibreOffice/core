/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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

    // Um Sonderfaelle mit Ftn behandeln zu koennen.
    // Mit SetRstHeight wird dem SwTxtFrmBreak die Resthoehe eingestellt,
    // Um TruncLines() rufen zu koennen, ohne dass IsBreakNow() einen
    // anderen Wert zurueckliefert.
    // Es wird dabei davon ausgegangen, dass rLine auf der letzten Zeile
    // steht, die nicht mehr passt.

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

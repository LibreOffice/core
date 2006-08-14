/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: widorp.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 16:45:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

    const sal_Bool IsBroken() const     { return bBreak; }
    const sal_Bool IsKeepAlways() const { return bKeep; }
          void Keep()               { bKeep = sal_True; }
          void Break()              { bKeep = sal_False; bBreak = sal_True; }

    inline sal_Bool GetKeep() const { return bKeep; }
    inline void SetKeep( const sal_Bool bNew ) { bKeep = bNew; }

    const sal_Bool IsInside( SwTxtMargin &rLine ) const;

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
    const MSHORT GetWidowsLines() const
    { return nWidLines; }
    const MSHORT GetOrphansLines() const
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

/*************************************************************************
 *
 *  $RCSfile: widorp.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    const SwTwips  nOrigin;
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
    void    SetRstHeight( const SwTxtMargin &rLine ) { nRstHeight = rLine.Y() - nOrigin; }
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
    sal_Bool WouldFit( SwTxtMargin &rLine, SwTwips &rMaxHeight );
    sal_Bool IsBreakNow( SwTxtMargin &rLine )
    {   return ( rLine.GetLineNr() > nOrphLines ) &&
                 SwTxtFrmBreak::IsBreakNow( rLine ); }
};


#endif

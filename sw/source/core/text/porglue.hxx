/*************************************************************************
 *
 *  $RCSfile: porglue.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:25 $
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
#ifndef _PORGLUE_HXX
#define _PORGLUE_HXX


//#include <stdlib.h>

#include "porlin.hxx"

class SwRect;
struct SwPosition;

/*************************************************************************
 *                      class SwGluePortion
 *************************************************************************/

class SwGluePortion : public SwLinePortion
{
private:
    KSHORT nFixWidth;
public:
                SwGluePortion( const KSHORT nInitFixWidth );

    void Join( SwGluePortion *pVictim );

    inline short GetPrtGlue() const;
    inline KSHORT GetFixWidth() const { return nFixWidth; }
    inline void SetFixWidth( const KSHORT nNew ) { nFixWidth = nNew; }
    void MoveGlue( SwGluePortion *pTarget, const short nPrtGlue );
    inline void MoveAllGlue( SwGluePortion *pTarget );
    inline void MoveHalfGlue( SwGluePortion *pTarget );
    inline void AdjFixWidth();
#ifdef OLDRECYCLE
    virtual sal_Bool MayRecycle() const;
#endif
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual xub_StrLen GetCrsrOfst( const KSHORT nOfst ) const;
    virtual SwPosSize GetTxtSize( const SwTxtSizeInfo &rInfo ) const;
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwFixPortion
 *************************************************************************/

class SwFixPortion : public SwGluePortion
{
    KSHORT nFix;        // der Width-Offset in der Zeile
public:
        SwFixPortion( const SwRect &rFlyRect );
        SwFixPortion( const KSHORT nFixWidth, const KSHORT nFixPos );
    inline void   Fix( const KSHORT nNewFix ) { nFix = nNewFix; }
    inline KSHORT Fix() const { return nFix; }
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                class SwMarginPortion
 *************************************************************************/

class SwMarginPortion : public SwGluePortion
{
public:
        SwMarginPortion( const KSHORT nFixWidth );
        void AdjustRight();
#ifdef OLDRECYCLE
    virtual sal_Bool MayRecycle() const;
#endif
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                inline SwGluePortion::GetPrtGlue()
 *************************************************************************/

inline short SwGluePortion::GetPrtGlue() const
{ return Width() - nFixWidth; }

/*************************************************************************
 *              inline SwGluePortion::AdjFixWidth()
 * Die FixWidth darf niemals groesser sein als die Gesamtbreite !
 *************************************************************************/

inline void SwGluePortion::AdjFixWidth()
{
    if( nFixWidth > PrtWidth() )
        nFixWidth = PrtWidth();
}

/*************************************************************************
 *                 inline SwGluePortion::MoveGlue()
 *************************************************************************/

inline void SwGluePortion::MoveAllGlue( SwGluePortion *pTarget )
{
    MoveGlue( pTarget, GetPrtGlue() );
}

/*************************************************************************
 *                inline SwGluePortion::MoveHalfGlue()
 *************************************************************************/

inline void SwGluePortion::MoveHalfGlue( SwGluePortion *pTarget )
{
    MoveGlue( pTarget, GetPrtGlue() / 2 );
}

CLASSIO( SwGluePortion )
CLASSIO( SwFixPortion )
CLASSIO( SwMarginPortion )


#endif


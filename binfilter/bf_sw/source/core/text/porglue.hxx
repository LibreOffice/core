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
#ifndef _PORGLUE_HXX
#define _PORGLUE_HXX


//#include <stdlib.h>

#include "porlin.hxx"
namespace binfilter {

class SwRect;
class SwLineLayout;
struct SwPosition;

/*************************************************************************
 *						class SwGluePortion
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
    virtual void Paint( const SwTxtPaintInfo &rInf ) const{DBG_BF_ASSERT(0, "STRIP");};//STRIP001 	virtual void Paint( const SwTxtPaintInfo &rInf ) const;

    OUTPUT_OPERATOR
};

/*************************************************************************
 *						class SwFixPortion
 *************************************************************************/

class SwFixPortion : public SwGluePortion
{
    KSHORT nFix;		// der Width-Offset in der Zeile
public:
        SwFixPortion( const SwRect &rFlyRect );
        SwFixPortion( const KSHORT nFixWidth, const KSHORT nFixPos );
    inline void   Fix( const KSHORT nNewFix ) { nFix = nNewFix; }
    inline KSHORT Fix() const { return nFix; }
    OUTPUT_OPERATOR
};

/*************************************************************************
 *				  class SwMarginPortion
 *************************************************************************/

class SwMarginPortion : public SwGluePortion
{
public:
        SwMarginPortion( const KSHORT nFixWidth );
        void AdjustRight( const SwLineLayout* pCurr );
    OUTPUT_OPERATOR
};

/*************************************************************************
 *				  inline SwGluePortion::GetPrtGlue()
 *************************************************************************/

inline short SwGluePortion::GetPrtGlue() const
{ return Width() - nFixWidth; }

/*************************************************************************
 *				inline SwGluePortion::AdjFixWidth()
 * Die FixWidth darf niemals groesser sein als die Gesamtbreite !
 *************************************************************************/

inline void SwGluePortion::AdjFixWidth()
{
    if( nFixWidth > PrtWidth() )
        nFixWidth = PrtWidth();
}

/*************************************************************************
 *				   inline SwGluePortion::MoveGlue()
 *************************************************************************/

inline void SwGluePortion::MoveAllGlue( SwGluePortion *pTarget )
{
    MoveGlue( pTarget, GetPrtGlue() );
}

/*************************************************************************
 *				  inline SwGluePortion::MoveHalfGlue()
 *************************************************************************/

inline void SwGluePortion::MoveHalfGlue( SwGluePortion *pTarget )
{
    MoveGlue( pTarget, GetPrtGlue() / 2 );
}

CLASSIO( SwGluePortion )
CLASSIO( SwFixPortion )
CLASSIO( SwMarginPortion )


} //namespace binfilter
#endif


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
#ifndef _PORFLY_HXX
#define _PORFLY_HXX
// OD 28.10.2003 #113049#
#include <ascharanchoredobjectposition.hxx>

#include "porglue.hxx"

class SwDrawContact;
class SwFlyInCntFrm;
class SwTxtFrm;
struct SwCrsrMoveState;

/*************************************************************************
 *                class SwFlyPortion
 *************************************************************************/

class SwFlyPortion : public SwFixPortion
{
    KSHORT nBlankWidth;
public:
    inline  SwFlyPortion( const SwRect &rFlyRect )
        : SwFixPortion(rFlyRect), nBlankWidth( 0 ) { SetWhichPor( POR_FLY ); }
    inline KSHORT GetBlankWidth( ) const { return nBlankWidth; }
    inline void SetBlankWidth( const KSHORT nNew ) { nBlankWidth = nNew; }
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                class SwFlyCntPortion
 *************************************************************************/

class SwFlyCntPortion : public SwLinePortion
{
    void *pContact; // bDraw ? DrawContact : FlyInCntFrm
    Point aRef;     // Relativ zu diesem Point wird die AbsPos berechnet.
    sal_Bool bDraw : 1;  // DrawContact?
    sal_Bool bMax : 1;   // Zeilenausrichtung und Hoehe == Zeilenhoehe
    sal_uInt8 nAlign : 3; // Zeilenausrichtung? Nein, oben, mitte, unten
    virtual xub_StrLen GetCrsrOfst( const KSHORT nOfst ) const;

public:
    // OD 29.07.2003 #110978# - use new datatype for parameter <nFlags>
    SwFlyCntPortion( const SwTxtFrm& rFrm, SwFlyInCntFrm *pFly,
                     const Point &rBase,
                     long nAscent, long nDescent, long nFlyAsc, long nFlyDesc,
                     objectpositioning::AsCharFlags nFlags );
    // OD 29.07.2003 #110978# - use new datatype for parameter <nFlags>
    SwFlyCntPortion( const SwTxtFrm& rFrm, SwDrawContact *pDrawContact,
                     const Point &rBase,
                     long nAscent, long nDescent, long nFlyAsc, long nFlyDesc,
                     objectpositioning::AsCharFlags nFlags );
    inline const Point& GetRefPoint() const { return aRef; }
    inline SwFlyInCntFrm *GetFlyFrm() { return (SwFlyInCntFrm*)pContact; }
    inline const SwFlyInCntFrm *GetFlyFrm() const
        { return (SwFlyInCntFrm*)pContact; }
    inline SwDrawContact *GetDrawContact() { return (SwDrawContact*)pContact; }
    inline const SwDrawContact* GetDrawContact() const
        { return (SwDrawContact*)pContact; }
    inline sal_Bool IsDraw() const { return bDraw; }
    inline sal_Bool IsMax() const { return bMax; }
    inline sal_uInt8 GetAlign() const { return nAlign; }
    inline void SetAlign( sal_uInt8 nNew ) { nAlign = nNew; }
    inline void SetMax( sal_Bool bNew ) { bMax = bNew; }
    // OD 29.07.2003 #110978# - use new datatype for parameter <nFlags>
    void SetBase( const SwTxtFrm& rFrm, const Point &rBase,
                  long nLnAscent, long nLnDescent,
                  long nFlyAscent, long nFlyDescent,
                  objectpositioning::AsCharFlags nFlags );
    xub_StrLen GetFlyCrsrOfst( const KSHORT nOfst, const Point &rPoint,
                        SwPosition *pPos, SwCrsrMoveState* pCMS ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    OUTPUT_OPERATOR
};

CLASSIO( SwFlyPortion )
CLASSIO( SwFlyCntPortion )


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

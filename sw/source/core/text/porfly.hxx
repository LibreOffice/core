/*************************************************************************
 *
 *  $RCSfile: porfly.hxx,v $
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
#ifndef _PORFLY_HXX
#define _PORFLY_HXX

#include "porglue.hxx"

class SwDrawContact;
class SwFrmFmt;
class SwFlyInCntFrm;

/*************************************************************************
 *                class SwFlyPortion
 *************************************************************************/

class SwFlyPortion : public SwFixPortion
{
    KSHORT nBlankWidth;
public:
    inline  SwFlyPortion( const SwRect &rFlyRect )
        : SwFixPortion(rFlyRect), nBlankWidth( 0 ) { SetWhichPor( POR_FLY ); }
    inline const KSHORT GetBlankWidth( ) const { return nBlankWidth; }
    inline void SetBlankWidth( const KSHORT nNew ) { nBlankWidth = nNew; }
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
#ifdef OLDRECYCLE
    virtual sal_Bool MayRecycle() const;
#endif
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                class SwFlyCntPortion
 *************************************************************************/

#define SETBASE_NOFLAG  0
#define SETBASE_QUICK   1
#define SETBASE_ULSPACE 2
#define SETBASE_INIT    4

class SwFlyCntPortion : public SwLinePortion
{
    void *pContact; // bDraw ? DrawContact : FlyInCntFrm
    Point aRef;     // Relativ zu diesem Point wird die AbsPos berechnet.
    sal_Bool bDraw : 1;  // DrawContact?
    sal_Bool bMax : 1;   // Zeilenausrichtung und Hoehe == Zeilenhoehe
    sal_uInt8 nAlign : 3; // Zeilenausrichtung? Nein, oben, mitte, unten
    virtual xub_StrLen GetCrsrOfst( const KSHORT nOfst ) const;

public:
    SwFlyCntPortion( SwFlyInCntFrm *pFly, const Point &rBase, long nAscent,
        long nDescent, long nFlyAsc, long nFlyDesc, sal_Bool bQuick = sal_False );
    SwFlyCntPortion( SwDrawContact *pDrawContact, const Point &rBase,
        long nAscent, long nDescent, long nFlyAsc, long nFlyDesc,
        sal_Bool bQuick = sal_False );
    inline const Point& GetRefPoint() const { return aRef; }
    inline SwFlyInCntFrm *GetFlyFrm() { return (SwFlyInCntFrm*)pContact; }
    inline const SwFlyInCntFrm *GetFlyFrm() const
        { return (SwFlyInCntFrm*)pContact; }
    inline SwDrawContact *GetDrawContact() { return (SwDrawContact*)pContact; }
    inline const SwDrawContact* GetDrawContact() const
        { return (SwDrawContact*)pContact; }
    inline const sal_Bool IsDraw() const { return bDraw; }
    inline const sal_Bool IsMax() const { return bMax; }
    inline const sal_uInt8 GetAlign() const { return nAlign; }
    inline void SetAlign( sal_uInt8 nNew ) { nAlign = nNew; }
    inline void SetMax( sal_Bool bNew ) { bMax = bNew; }
    void SetBase( const Point &rBase, long nLnAscent, long nLnDescent,
        long nFlyAscent, long nFlyDescent, sal_uInt8 nFlags );
    const SwFrmFmt *GetFrmFmt() const;
    xub_StrLen GetFlyCrsrOfst( const KSHORT nOfst, const Point &rPoint,
                        SwPosition *pPos, const SwCrsrMoveState* pCMS ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    OUTPUT_OPERATOR
};

CLASSIO( SwFlyPortion )
CLASSIO( SwFlyCntPortion )


#endif

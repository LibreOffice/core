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
#ifndef _NOTXTFRM_HXX
#define _NOTXTFRM_HXX

#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
class OutputDevice;
class Bitmap;
namespace binfilter {

class SwNoTxtNode;

class SwBorderAttrs;

struct SwCrsrMoveState;

class SwNoTxtFrm: public SwCntntFrm
{
    friend void _FrmFinit();	//erase pErrorBmp and pReplaceBmp
    static Bitmap *pErrorBmp, *pReplaceBmp;

    short	 nWeight;					// "Wichtigkeit" der Grafik

    const Size& GetSize() const;

    void InitCtor();

    void Format ( const SwBorderAttrs *pAttrs = 0 );
    void PaintCntnt  ( OutputDevice*, const SwRect&, const SwRect& ) const;
    /// OD 25.09.2002 #99739# - delete unused 3rd parameter
    void PaintPicture( OutputDevice*, const SwRect& ) const;
protected:
    virtual void MakeAll();
public:
    SwNoTxtFrm( SwNoTxtNode * const );
    ~SwNoTxtFrm();

    virtual void Modify( SfxPoolItem*, SfxPoolItem* );
    virtual void Paint(const SwRect& ) const;
    virtual BOOL GetCharRect( SwRect &, const SwPosition&,
                              SwCrsrMoveState* = 0) const;
    BOOL GetCrsrOfst(SwPosition* pPos, Point& aPoint,
                            const SwCrsrMoveState* = 0) const;

    const Size &GetGrfSize() const	{ return GetSize(); }
    void GetGrfArea( SwRect &rRect, SwRect * = 0, BOOL bMirror = TRUE ) const;

    void StopAnimation( OutputDevice* = 0 ) const;
    BOOL HasAnimation()  const;

    // Routinen fuer den Grafik-Cache
    USHORT GetWeight() { return nWeight; }

    static const Bitmap& GetBitmap( BOOL bError );
};


} //namespace binfilter
#endif

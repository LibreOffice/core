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
#ifndef _BLINK_HXX
#define _BLINK_HXX

class SwLinePortion;
class SwRootFrm;
class SwTxtFrm;

#include <vcl/timer.hxx>
#include <tools/gen.hxx>

#include <svl/svarray.hxx>

class SwBlinkPortion
{
    Point               aPos;
    const SwLinePortion *pPor;
    const SwRootFrm     *pFrm;
    sal_uInt16              nDir;
public:
    SwBlinkPortion( const SwLinePortion* pPortion, sal_uInt16 nDirection )
            { pPor = pPortion; nDir = nDirection; }
    SwBlinkPortion( const SwBlinkPortion* pBlink, const SwLinePortion* pPort )
    {   pPor = pPort; pFrm = pBlink->pFrm; aPos = pBlink->aPos; nDir = pBlink->nDir; }
    void SetPos( const Point& aNew ){ aPos = aNew; }
    const Point& GetPos() const{ return aPos; }
    void SetRootFrm( const SwRootFrm* pNew ){ pFrm = pNew; }
    const SwRootFrm* GetRootFrm() const{ return pFrm; }
    const SwLinePortion *GetPortion() const{ return pPor; }
    sal_uInt16 GetDirection() const { return nDir; }
    sal_Bool operator<( const SwBlinkPortion& rBlinkPortion ) const
    { return (long)pPor < (long)rBlinkPortion.pPor; }
    sal_Bool operator==( const SwBlinkPortion& rBlinkPortion ) const
    { return (long)pPor == (long)rBlinkPortion.pPor; }
};

typedef SwBlinkPortion* SwBlinkPortionPtr;
SV_DECL_PTRARR_SORT_DEL(SwBlinkList, SwBlinkPortionPtr, 0, 10)

class SwBlink
{
    SwBlinkList     aList;
    AutoTimer       aTimer;
    sal_Bool            bVisible;
public:
    SwBlink();
    ~SwBlink();

    DECL_LINK( Blinker, Timer * );

    void Insert( const Point& rPoint, const SwLinePortion* pPor,
                 const SwTxtFrm *pTxtFrm, sal_uInt16 nDir );
    void Replace( const SwLinePortion* pOld, const SwLinePortion* pNew );
    void Delete( const SwLinePortion* pPor );
    void FrmDelete( const SwRootFrm* pRoot );
    inline sal_Bool IsVisible() const { return bVisible ; }
};

// Blink-Manager, globale Variable, in Blink.Cxx angelegt
extern SwBlink *pBlink;


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

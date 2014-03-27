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
#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_PORFLY_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_PORFLY_HXX
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
    virtual void Paint( const SwTxtPaintInfo &rInf ) const SAL_OVERRIDE;
    virtual bool Format( SwTxtFormatInfo &rInf ) SAL_OVERRIDE;
    OUTPUT_OPERATOR_OVERRIDE
};

/*************************************************************************
 *                class SwFlyCntPortion
 *************************************************************************/

/// This portion represents an as-character anchored fly (shape, frame, etc.)
class SwFlyCntPortion : public SwLinePortion
{
    void *pContact; // bDraw ? DrawContact : FlyInCntFrm
    Point aRef;     // Relatively to this point we calculate the AbsPos
    bool bDraw : 1;  // DrawContact?
    bool bMax : 1;   // Line adjustment and height == line height
    sal_uInt8 nAlign : 3; // Line adjustment? No, above, middle, bottom
    virtual sal_Int32 GetCrsrOfst( const KSHORT nOfst ) const SAL_OVERRIDE;

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
    inline bool IsDraw() const { return bDraw; }
    inline bool IsMax() const { return bMax; }
    inline sal_uInt8 GetAlign() const { return nAlign; }
    inline void SetAlign( sal_uInt8 nNew ) { nAlign = nNew; }
    inline void SetMax( bool bNew ) { bMax = bNew; }
    // OD 29.07.2003 #110978# - use new datatype for parameter <nFlags>
    void SetBase( const SwTxtFrm& rFrm, const Point &rBase,
                  long nLnAscent, long nLnDescent,
                  long nFlyAscent, long nFlyDescent,
                  objectpositioning::AsCharFlags nFlags );
    sal_Int32 GetFlyCrsrOfst( const KSHORT nOfst, const Point &rPoint,
                        SwPosition *pPos, SwCrsrMoveState* pCMS ) const;
    virtual bool Format( SwTxtFormatInfo &rInf ) SAL_OVERRIDE;
    virtual void Paint( const SwTxtPaintInfo &rInf ) const SAL_OVERRIDE;
    OUTPUT_OPERATOR_OVERRIDE
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

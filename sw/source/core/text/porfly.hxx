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

#include <ascharanchoredobjectposition.hxx>

#include "porglue.hxx"

class SwDrawContact;
class SwFlyInContentFrame;
class SwTextFrame;
struct SwCursorMoveState;

class SwFlyPortion : public SwFixPortion
{
    sal_uInt16 nBlankWidth;
public:
    explicit SwFlyPortion( const SwRect &rFlyRect )
        : SwFixPortion(rFlyRect), nBlankWidth( 0 ) { SetWhichPor( POR_FLY ); }
    inline sal_uInt16 GetBlankWidth( ) const { return nBlankWidth; }
    inline void SetBlankWidth( const sal_uInt16 nNew ) { nBlankWidth = nNew; }
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;
    OUTPUT_OPERATOR_OVERRIDE
};

/// This portion represents an as-character anchored fly (shape, frame, etc.)
class SwFlyCntPortion : public SwLinePortion
{
    void *pContact; // bDraw ? DrawContact : FlyInContentFrame
    Point aRef;     // Relatively to this point we calculate the AbsPos
    bool bDraw : 1;  // DrawContact?
    bool bMax : 1;   // Line adjustment and height == line height
    sal_uInt8 nAlign : 3; // Line adjustment? No, above, middle, bottom
    virtual sal_Int32 GetCursorOfst( const sal_uInt16 nOfst ) const override;

public:
    // Use new datatype for parameter <nFlags>
    SwFlyCntPortion( const SwTextFrame& rFrame, SwFlyInContentFrame *pFly,
                     const Point &rBase,
                     long nAscent, long nDescent, long nFlyAsc, long nFlyDesc,
                     objectpositioning::AsCharFlags nFlags );
    // Use new datatype for parameter <nFlags>
    SwFlyCntPortion( const SwTextFrame& rFrame, SwDrawContact *pDrawContact,
                     const Point &rBase,
                     long nAscent, long nDescent, long nFlyAsc, long nFlyDesc,
                     objectpositioning::AsCharFlags nFlags );
    inline const Point& GetRefPoint() const { return aRef; }
    inline SwFlyInContentFrame *GetFlyFrame() { return static_cast<SwFlyInContentFrame*>(pContact); }
    inline const SwFlyInContentFrame *GetFlyFrame() const
        { return static_cast<SwFlyInContentFrame*>(pContact); }
    inline SwDrawContact *GetDrawContact() { return static_cast<SwDrawContact*>(pContact); }
    inline const SwDrawContact* GetDrawContact() const
        { return static_cast<SwDrawContact*>(pContact); }
    inline bool IsDraw() const { return bDraw; }
    inline bool IsMax() const { return bMax; }
    inline sal_uInt8 GetAlign() const { return nAlign; }
    inline void SetAlign( sal_uInt8 nNew ) { nAlign = nNew; }
    inline void SetMax( bool bNew ) { bMax = bNew; }
    // Use new datatype for parameter <nFlags>
    void SetBase( const SwTextFrame& rFrame, const Point &rBase,
                  long nLnAscent, long nLnDescent,
                  long nFlyAscent, long nFlyDescent,
                  objectpositioning::AsCharFlags nFlags );
    sal_Int32 GetFlyCursorOfst( const sal_uInt16 nOfst, const Point &rPoint,
                        SwPosition *pPos, SwCursorMoveState* pCMS ) const;
    virtual bool Format( SwTextFormatInfo &rInf ) override;
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    OUTPUT_OPERATOR_OVERRIDE
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_PORGLUE_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_PORGLUE_HXX

#include "porlin.hxx"

class SwRect;
class SwLineLayout;

class SwGluePortion : public SwLinePortion
{
private:
    sal_uInt16 nFixWidth;
public:
    explicit SwGluePortion( const sal_uInt16 nInitFixWidth );

    void Join( SwGluePortion *pVictim );

    inline short GetPrtGlue() const;
    inline sal_uInt16 GetFixWidth() const { return nFixWidth; }
    inline void SetFixWidth( const sal_uInt16 nNew ) { nFixWidth = nNew; }
    void MoveGlue( SwGluePortion *pTarget, const short nPrtGlue );
    inline void MoveAllGlue( SwGluePortion *pTarget );
    inline void MoveHalfGlue( SwGluePortion *pTarget );
    inline void AdjFixWidth();
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual sal_Int32 GetCursorOfst( const sal_uInt16 nOfst ) const override;
    virtual SwPosSize GetTextSize( const SwTextSizeInfo &rInfo ) const override;
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const override;

    OUTPUT_OPERATOR_OVERRIDE
};

class SwFixPortion : public SwGluePortion
{
    sal_uInt16 nFix;        // The width offset in the line
public:
    explicit SwFixPortion( const SwRect &rFlyRect );
    SwFixPortion( const sal_uInt16 nFixWidth, const sal_uInt16 nFixPos );
    inline void   Fix( const sal_uInt16 nNewFix ) { nFix = nNewFix; }
    inline sal_uInt16 Fix() const { return nFix; }
    OUTPUT_OPERATOR_OVERRIDE
};

class SwMarginPortion : public SwGluePortion
{
public:
    explicit SwMarginPortion( const sal_uInt16 nFixWidth );
    void AdjustRight( const SwLineLayout* pCurr );
    OUTPUT_OPERATOR_OVERRIDE
};

inline short SwGluePortion::GetPrtGlue() const
{ return Width() - nFixWidth; }

// The FixWidth MUST NEVER be larger than the accumulated width!
inline void SwGluePortion::AdjFixWidth()
{
    if( nFixWidth > PrtWidth() )
        nFixWidth = PrtWidth();
}

inline void SwGluePortion::MoveAllGlue( SwGluePortion *pTarget )
{
    MoveGlue( pTarget, GetPrtGlue() );
}

inline void SwGluePortion::MoveHalfGlue( SwGluePortion *pTarget )
{
    MoveGlue( pTarget, GetPrtGlue() / 2 );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

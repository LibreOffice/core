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
    virtual void Paint( const SwTxtPaintInfo &rInf ) const SAL_OVERRIDE;
    virtual sal_Int32 GetCrsrOfst( const KSHORT nOfst ) const SAL_OVERRIDE;
    virtual SwPosSize GetTxtSize( const SwTxtSizeInfo &rInfo ) const SAL_OVERRIDE;
    virtual bool GetExpTxt( const SwTxtSizeInfo &rInf, OUString &rTxt ) const SAL_OVERRIDE;

    OUTPUT_OPERATOR_OVERRIDE
};

/*************************************************************************
 *                      class SwFixPortion
 *************************************************************************/

class SwFixPortion : public SwGluePortion
{
    KSHORT nFix;        // The width offset in the line
public:
        SwFixPortion( const SwRect &rFlyRect );
        SwFixPortion( const KSHORT nFixWidth, const KSHORT nFixPos );
    inline void   Fix( const KSHORT nNewFix ) { nFix = nNewFix; }
    inline KSHORT Fix() const { return nFix; }
    OUTPUT_OPERATOR_OVERRIDE
};

/*************************************************************************
 *                class SwMarginPortion
 *************************************************************************/

class SwMarginPortion : public SwGluePortion
{
public:
        SwMarginPortion( const KSHORT nFixWidth );
        void AdjustRight( const SwLineLayout* pCurr );
    OUTPUT_OPERATOR_OVERRIDE
};

/*************************************************************************
 *                inline SwGluePortion::GetPrtGlue()
 *************************************************************************/

inline short SwGluePortion::GetPrtGlue() const
{ return Width() - nFixWidth; }

/*************************************************************************
 *              inline SwGluePortion::AdjFixWidth()
 * The FixWidth MUST NEVER be larger than the accumulated width!
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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

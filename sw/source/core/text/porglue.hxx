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

#pragma once

#include "porlin.hxx"

class SwRect;
class SwLineLayout;

class SwGluePortion : public SwLinePortion
{
private:
    sal_uInt16 m_nFixWidth;
public:
    explicit SwGluePortion( const sal_uInt16 nInitFixWidth );

    void Join( SwGluePortion *pVictim );

    inline tools::Long GetPrtGlue() const;
    sal_uInt16 GetFixWidth() const { return m_nFixWidth; }
    void SetFixWidth( const sal_uInt16 nNew ) { m_nFixWidth = nNew; }
    void MoveGlue( SwGluePortion *pTarget, const tools::Long nPrtGlue );
    inline void MoveAllGlue( SwGluePortion *pTarget );
    inline void MoveHalfGlue( SwGluePortion *pTarget );
    inline void AdjFixWidth();
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual TextFrameIndex GetModelPositionForViewPoint(sal_uInt16 nOfst) const override;
    virtual SwPosSize GetTextSize( const SwTextSizeInfo &rInfo ) const override;
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const override;

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

class SwFixPortion : public SwGluePortion
{
    sal_uInt16 m_nFix;        // The width offset in the line
public:
    explicit SwFixPortion( const SwRect &rFlyRect );
    SwFixPortion();
    void   SetFix( const sal_uInt16 nNewFix ) { m_nFix = nNewFix; }
    sal_uInt16 GetFix() const { return m_nFix; }
};

class SwMarginPortion : public SwGluePortion
{
public:
    explicit SwMarginPortion();
    void AdjustRight( const SwLineLayout* pCurr );
};

inline tools::Long SwGluePortion::GetPrtGlue() const
{ return Width() - m_nFixWidth; }

// The FixWidth MUST NEVER be larger than the accumulated width!
inline void SwGluePortion::AdjFixWidth()
{
    if( m_nFixWidth > PrtWidth() )
        m_nFixWidth = PrtWidth();
}

inline void SwGluePortion::MoveAllGlue( SwGluePortion *pTarget )
{
    MoveGlue( pTarget, GetPrtGlue() );
}

inline void SwGluePortion::MoveHalfGlue( SwGluePortion *pTarget )
{
    MoveGlue( pTarget, GetPrtGlue() / 2 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

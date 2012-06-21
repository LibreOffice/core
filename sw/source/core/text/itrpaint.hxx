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
#ifndef _ITRPAINT_HXX
#define _ITRPAINT_HXX
#include "itrtxt.hxx"

class SwSaveClip;          // SwTxtPainter
class SwMultiPortion;

/*************************************************************************
 *                      class SwTxtPainter
 *************************************************************************/

class SwTxtPainter : public SwTxtCursor
{
    sal_Bool bPaintDrop;

    SwLinePortion *CalcPaintOfst( const SwRect &rPaint );
    void CheckSpecialUnderline( const SwLinePortion* pPor,
                                long nAdjustBaseLine = 0 );
protected:
    void CtorInitTxtPainter( SwTxtFrm *pFrm, SwTxtPaintInfo *pInf );
    inline SwTxtPainter(SwTxtNode* pTxtNode) : SwTxtCursor(pTxtNode) { }

public:
    inline SwTxtPainter( SwTxtFrm *pTxtFrm, SwTxtPaintInfo *pTxtPaintInf ) : SwTxtCursor(pTxtFrm!=NULL?pTxtFrm->GetTxtNode():NULL)
           { CtorInitTxtPainter( pTxtFrm, pTxtPaintInf ); }
    void DrawTextLine( const SwRect &rPaint, SwSaveClip &rClip,
                       const sal_Bool bUnderSz );
    void PaintDropPortion();
    // if PaintMultiPortion is called recursively, we have to pass the
    // surrounding SwBidiPortion
    void PaintMultiPortion( const SwRect &rPaint, SwMultiPortion& rMulti,
                            const SwMultiPortion* pEnvPor = 0 );
    inline void SetPaintDrop( const sal_Bool bNew ) { bPaintDrop = bNew; }
    inline sal_Bool IsPaintDrop() const { return bPaintDrop; }
    inline SwTxtPaintInfo &GetInfo()
        { return (SwTxtPaintInfo&)SwTxtIter::GetInfo(); }
    inline const SwTxtPaintInfo &GetInfo() const
        { return (const SwTxtPaintInfo&)SwTxtIter::GetInfo(); }
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

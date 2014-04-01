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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_NOTXTFRM_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_NOTXTFRM_HXX

#include <cntfrm.hxx>

class SwNoTxtNode;
class OutputDevice;
class SwBorderAttrs;
struct SwCrsrMoveState;

class SwNoTxtFrm: public SwCntntFrm
{
    friend void _FrmFinit();

    short    nWeight;                   // importance of the graphic

    const Size& GetSize() const;

    void InitCtor();

    void Format ( const SwBorderAttrs *pAttrs = 0 ) SAL_OVERRIDE;
    void PaintCntnt  ( OutputDevice*, const SwRect&, const SwRect& ) const;
    void PaintPicture( OutputDevice*, const SwRect& ) const;
protected:
    virtual void MakeAll() SAL_OVERRIDE;
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) SAL_OVERRIDE;
public:
    SwNoTxtFrm( SwNoTxtNode * const, SwFrm* );
    virtual ~SwNoTxtFrm();

    virtual void Paint( SwRect const&,
                        SwPrintData const*const pPrintData = NULL ) const SAL_OVERRIDE;
    virtual bool GetCharRect( SwRect &, const SwPosition&,
                              SwCrsrMoveState* = 0) const SAL_OVERRIDE;
    virtual bool GetCrsrOfst(SwPosition* pPos, Point& aPoint,
                     SwCrsrMoveState* = 0, bool bTestBackground = false) const SAL_OVERRIDE;

    const Size &GetGrfSize() const  { return GetSize(); }
    void GetGrfArea( SwRect &rRect, SwRect * = 0, bool bMirror = true ) const;

    sal_Bool IsTransparent() const;

    void StopAnimation( OutputDevice* = 0 ) const;
    sal_Bool HasAnimation()  const;

    // Routine for the graphics cache
    sal_uInt16 GetWeight() { return nWeight; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

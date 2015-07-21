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

class SwNoTextNode;
class OutputDevice;
class SwBorderAttrs;
struct SwCrsrMoveState;

class SwNoTextFrm: public SwContentFrm
{
    friend void _FrmFinit();

    const Size& GetSize() const;

    void InitCtor();

    void Format ( vcl::RenderContext* pRenderContext, const SwBorderAttrs *pAttrs = 0 ) SAL_OVERRIDE;
    void PaintPicture( vcl::RenderContext*, const SwRect& ) const;

    virtual void DestroyImpl() SAL_OVERRIDE;
    virtual ~SwNoTextFrm();

protected:
    virtual void MakeAll(vcl::RenderContext* pRenderContext) SAL_OVERRIDE;
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) SAL_OVERRIDE;
public:
    SwNoTextFrm( SwNoTextNode * const, SwFrm* );

    virtual void Paint( vcl::RenderContext& rRenderContext, SwRect const&,
                        SwPrintData const*const pPrintData = NULL ) const SAL_OVERRIDE;
    virtual bool GetCharRect( SwRect &, const SwPosition&,
                              SwCrsrMoveState* = 0) const SAL_OVERRIDE;
    virtual bool GetCrsrOfst(SwPosition* pPos, Point& aPoint,
                     SwCrsrMoveState* = 0, bool bTestBackground = false) const SAL_OVERRIDE;

    void GetGrfArea( SwRect &rRect, SwRect * = 0, bool bMirror = true ) const;

    bool IsTransparent() const;

    void StopAnimation( OutputDevice* = 0 ) const;
    bool HasAnimation()  const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

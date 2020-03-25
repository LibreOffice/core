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
#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_TXTPAINT_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_TXTPAINT_HXX
#include <vcl/outdev.hxx>

class SwRect;               // SwSaveClip
class SwTextFrame;

class SwSaveClip final
{
    vcl::Region   aClip;
    const bool     bOn;
          bool     bChg;

    VclPtr<OutputDevice> pOut;
    void ChgClip_( const SwRect &rRect, const SwTextFrame* pFrame,
                   bool bEnlargeRect,
                   sal_Int32 nEnlargeTop,
                   sal_Int32 nEnlargeBottom );
public:
    explicit SwSaveClip(OutputDevice* pOutDev)
        : bOn(pOutDev && pOutDev->IsClipRegion())
        , bChg(false)
        , pOut(pOutDev)
    {
    }

    ~SwSaveClip();
    void ChgClip( const SwRect &rRect, const SwTextFrame* pFrame = nullptr,
                         bool bEnlargeRect = false,
                         sal_Int32 nEnlargeTop = 0,
                         sal_Int32 nEnlargeBottom = 0)
             { if( pOut ) ChgClip_( rRect, pFrame,
                         bEnlargeRect, nEnlargeTop, nEnlargeBottom ); }
    bool IsOn()  const { return bOn; }
    bool IsChg() const { return bChg; }
};


#ifdef DBG_UTIL

class SwDbgOut
{
protected:
        VclPtr<OutputDevice> pOut;
public:
        inline SwDbgOut( OutputDevice* pOutDev, const bool bOn );
};

class DbgBackColor : public SwDbgOut
{
        Color   aOldFillColor;
public:
        DbgBackColor( OutputDevice* pOut, const bool bOn );
       ~DbgBackColor();
};

class DbgRect : public SwDbgOut
{
public:
        DbgRect( OutputDevice* pOut, const tools::Rectangle &rRect,
                 const bool bOn,
                 Color eColor );
};

inline SwDbgOut::SwDbgOut( OutputDevice* pOutDev, const bool bOn )
               :pOut( bOn ? pOutDev : nullptr )
{ }

inline DbgBackColor::DbgBackColor( OutputDevice* pOutDev, const bool bOn )
    :SwDbgOut( pOutDev, bOn )
{
    if( pOut )
    {
        aOldFillColor = pOut->GetFillColor();
        pOut->SetFillColor( COL_RED );
    }
}

inline DbgBackColor::~DbgBackColor()
{
    if( pOut )
    {
        pOut->SetFillColor( aOldFillColor );
    }
}

inline DbgRect::DbgRect( OutputDevice* pOutDev, const tools::Rectangle &rRect,
                         const bool bOn,
                         Color eColor )
    : SwDbgOut( pOutDev, bOn )
{
    if( pOut )
    {
        const Color aColor( eColor );
        Color aLineColor = pOut->GetLineColor();
        pOut->SetLineColor( aColor );
        Color aFillColor = pOut->GetFillColor();
        pOut->SetFillColor( COL_TRANSPARENT );
        pOut->DrawRect( rRect );
        pOut->SetLineColor( aLineColor );
        pOut->SetFillColor( aFillColor );
    }
}

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

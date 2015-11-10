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
#include <txtfrm.hxx>

class SwSaveClip
{
    vcl::Region   aClip;
    const bool     bOn;
          bool     bChg;
protected:
    VclPtr<OutputDevice> pOut;
    void _ChgClip( const SwRect &rRect, const SwTextFrm* pFrm,
                   bool bEnlargeRect );
public:
    explicit SwSaveClip(OutputDevice* pOutDev)
        : bOn(pOutDev && pOutDev->IsClipRegion())
        , bChg(false)
        , pOut(pOutDev)
    {
    }

    inline ~SwSaveClip();
    inline void ChgClip( const SwRect &rRect, const SwTextFrm* pFrm = nullptr,
                         bool bEnlargeRect = false)
             { if( pOut ) _ChgClip( rRect, pFrm, bEnlargeRect ); }
           void Reset();
    inline bool IsOn()  const { return bOn; }
    inline bool IsChg() const { return bChg; }
};

inline SwSaveClip::~SwSaveClip()
{
    Reset();
}

#ifdef DBG_UTIL

class SwDbgOut
{
protected:
        VclPtr<OutputDevice> pOut;
public:
        inline SwDbgOut( OutputDevice* pOutDev, const bool bOn = true );
};

class DbgBackColor : public SwDbgOut
{
        Color   aOldFillColor;
public:
        DbgBackColor( OutputDevice* pOut, const bool bOn = true,
                  ColorData nColor = COL_YELLOW );
       ~DbgBackColor();
};

class DbgRect : public SwDbgOut
{
public:
        DbgRect( OutputDevice* pOut, const Rectangle &rRect,
                 const bool bOn = true,
                 ColorData eColor = COL_LIGHTBLUE );
};

inline SwDbgOut::SwDbgOut( OutputDevice* pOutDev, const bool bOn )
               :pOut( bOn ? pOutDev : nullptr )
{ }

inline DbgBackColor::DbgBackColor( OutputDevice* pOutDev, const bool bOn,
                           ColorData eColor )
    :SwDbgOut( pOutDev, bOn )
{
    if( pOut )
    {
        aOldFillColor = pOut->GetFillColor();
        pOut->SetFillColor( Color(eColor) );
    }
}

inline DbgBackColor::~DbgBackColor()
{
    if( pOut )
    {
        pOut->SetFillColor( aOldFillColor );
    }
}

inline DbgRect::DbgRect( OutputDevice* pOutDev, const Rectangle &rRect,
                         const bool bOn,
                         ColorData eColor )
    : SwDbgOut( pOutDev, bOn )
{
    if( pOut )
    {
        const Color aColor( eColor );
        Color aLineColor = pOut->GetLineColor();
        pOut->SetLineColor( aColor );
        Color aFillColor = pOut->GetFillColor();
        pOut->SetFillColor( Color(COL_TRANSPARENT) );
        pOut->DrawRect( rRect );
        pOut->SetLineColor( aLineColor );
        pOut->SetFillColor( aFillColor );
    }
}

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <svx/svdomeas.hxx>
#include <svx/svdmodel.hxx>
#include <vcl/builder.hxx>

#include "svx/measctrl.hxx"
#include <svx/dialmgr.hxx>
#include "svx/dlgutil.hxx"

SvxXMeasurePreview::SvxXMeasurePreview( Window* pParent,const ResId& rResId,const SfxItemSet& rInAttrs)
:    Control ( pParent, rResId )

{
    SetMapMode( MAP_100TH_MM );

    Size aSize = GetOutputSize();

    // Scale: 1:2
    MapMode aMapMode = GetMapMode();
    aMapMode.SetScaleX( Fraction( 1, 2 ) );
    aMapMode.SetScaleY( Fraction( 1, 2 ) );
    SetMapMode( aMapMode );

    aSize = GetOutputSize();
    Point aPt1 = Point( aSize.Width() / 5, (long) ( aSize.Height() / 2 ) );
    Point aPt2 = Point( aSize.Width() * 4 / 5, (long) ( aSize.Height() / 2 ) );

    pMeasureObj = new SdrMeasureObj( aPt1, aPt2 );
    pModel = new SdrModel();
    pMeasureObj->SetModel( pModel );

    pMeasureObj->SetMergedItemSetAndBroadcast(rInAttrs);

    SetDrawMode( GetSettings().GetStyleSettings().GetHighContrastMode() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );

    Invalidate();
}

SvxXMeasurePreview::SvxXMeasurePreview( Window* pParent, WinBits nStyle)
    : Control(pParent, nStyle)
{
    SetMapMode( MAP_100TH_MM );

    // Scale: 1:2
    MapMode aMapMode = GetMapMode();
    aMapMode.SetScaleX( Fraction( 1, 2 ) );
    aMapMode.SetScaleY( Fraction( 1, 2 ) );
    SetMapMode( aMapMode );

    Size aSize = GetOutputSize();
    Point aPt1 = Point( aSize.Width() / 5, (long) ( aSize.Height() / 2 ) );
    Point aPt2 = Point( aSize.Width() * 4 / 5, (long) ( aSize.Height() / 2 ) );

    pMeasureObj = new SdrMeasureObj( aPt1, aPt2 );
    pModel = new SdrModel();
    pMeasureObj->SetModel( pModel );

    SetDrawMode( GetSettings().GetStyleSettings().GetHighContrastMode() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );

    Invalidate();
}

void SvxXMeasurePreview::Resize()
{
    Control::Resize();

    Size aSize = GetOutputSize();
    Point aPt1 = Point( aSize.Width() / 5, (long) ( aSize.Height() / 2 ) );
    pMeasureObj->SetPoint(aPt1, 0);
    Point aPt2 = Point( aSize.Width() * 4 / 5, (long) ( aSize.Height() / 2 ) );
    pMeasureObj->SetPoint(aPt2, 1);
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSvxXMeasurePreview(Window *pParent, VclBuilder::stringmap &rMap)
{
    WinBits nWinStyle = 0;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    return new SvxXMeasurePreview(pParent, nWinStyle);
}


SvxXMeasurePreview::~SvxXMeasurePreview()
{
    // No one is deleting the MeasureObj? This is not only an error but also
    // a memory leak (!). Main problem is that this object is still listening to
    // a StyleSheet of the model which was set. Thus, if You want to keep the obnject,
    // set the modfel to 0L, if object is not needed (seems to be the case here),
    // delete it.
    delete pMeasureObj;

    delete pModel;
}

void SvxXMeasurePreview::Paint( const Rectangle&  )
{
    pMeasureObj->SingleObjectPainter(*this);
}

void SvxXMeasurePreview::SetAttributes( const SfxItemSet& rInAttrs )
{
    pMeasureObj->SetMergedItemSetAndBroadcast(rInAttrs);

    Invalidate();
}

void SvxXMeasurePreview::MouseButtonDown( const MouseEvent& rMEvt )
{
    bool bZoomIn  = rMEvt.IsLeft() && !rMEvt.IsShift();
    bool bZoomOut = rMEvt.IsRight() || rMEvt.IsShift();
    sal_Bool bCtrl    = rMEvt.IsMod1();

    if( bZoomIn || bZoomOut )
    {
        MapMode aMapMode = GetMapMode();
        Fraction aXFrac = aMapMode.GetScaleX();
        Fraction aYFrac = aMapMode.GetScaleY();
        Fraction* pMultFrac;

        if( bZoomIn )
        {
            if( bCtrl )
                pMultFrac = new Fraction( 3, 2 );
            else
                pMultFrac = new Fraction( 11, 10 );
        }
        else
        {
            if( bCtrl )
                pMultFrac = new Fraction( 2, 3 );
            else
                pMultFrac = new Fraction( 10, 11 );
        }

        aXFrac *= *pMultFrac;
        aYFrac *= *pMultFrac;
        if( (double)aXFrac > 0.001 && (double)aXFrac < 1000.0 &&
            (double)aYFrac > 0.001 && (double)aYFrac < 1000.0 )
        {
            aMapMode.SetScaleX( aXFrac );
            aMapMode.SetScaleY( aYFrac );
            SetMapMode( aMapMode );

            Size aOutSize( GetOutputSize() );

            Point aPt( aMapMode.GetOrigin() );
            long nX = (long)( ( (double)aOutSize.Width() - ( (double)aOutSize.Width() * (double)*pMultFrac  ) ) / 2.0 + 0.5 );
            long nY = (long)( ( (double)aOutSize.Height() - ( (double)aOutSize.Height() * (double)*pMultFrac  ) ) / 2.0 + 0.5 );
            aPt.X() +=  nX;
            aPt.Y() +=  nY;

            aMapMode.SetOrigin( aPt );
            SetMapMode( aMapMode );

            Invalidate();
        }
        delete pMultFrac;
    }
}

// -----------------------------------------------------------------------

void SvxXMeasurePreview::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        SetDrawMode( GetSettings().GetStyleSettings().GetHighContrastMode() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

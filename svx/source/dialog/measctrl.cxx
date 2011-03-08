/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#include <svx/svdomeas.hxx>
#include <svx/svdmodel.hxx>

#include "measctrl.hxx"
#include <svx/dialmgr.hxx>
#include "dlgutil.hxx"

SvxXMeasurePreview::SvxXMeasurePreview
(
    Window* pParent,
    const ResId& rResId,
    const SfxItemSet& rInAttrs
) :

    Control ( pParent, rResId ),
    rAttrs  ( rInAttrs )

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
    BOOL bZoomIn  = rMEvt.IsLeft() && !rMEvt.IsShift();
    BOOL bZoomOut = rMEvt.IsRight() || rMEvt.IsShift();
    BOOL bCtrl    = rMEvt.IsMod1();

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

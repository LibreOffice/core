/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#include <svx/svdomeas.hxx>
#include <svx/svdmodel.hxx>

#include "svx/measctrl.hxx"
#include <svx/dialmgr.hxx>
#include "svx/dlgutil.hxx"

/*************************************************************************
|*
|* Ctor SvxXMeasurePreview
|*
*************************************************************************/

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

    // Massstab: 1:2
    MapMode aMapMode = GetMapMode();
    aMapMode.SetScaleX( Fraction( 1, 2 ) );
    aMapMode.SetScaleY( Fraction( 1, 2 ) );
    SetMapMode( aMapMode );

    aSize = GetOutputSize();
    Rectangle aRect = Rectangle( Point(), aSize );
    Point aPt1 = Point( aSize.Width() / 5, (long) ( aSize.Height() / 2 ) );
    Point aPt2 = Point( aSize.Width() * 4 / 5, (long) ( aSize.Height() / 2 ) );

    pModel = new SdrModel();
    pMeasureObj = new SdrMeasureObj( *pModel, aPt1, aPt2 );
    // pMeasureObj->SetModel( pModel );

    //pMeasureObj->SetItemSetAndBroadcast(rInAttrs);
    pMeasureObj->SetMergedItemSetAndBroadcast(rInAttrs);

    SetDrawMode( GetSettings().GetStyleSettings().GetHighContrastMode() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );

    Invalidate();
}

/*************************************************************************
|*
|* Dtor SvxXMeasurePreview
|*
*************************************************************************/

SvxXMeasurePreview::~SvxXMeasurePreview()
{
    // #111111#
    // No one is deleting the MeasureObj? This is not only an error but also
    // a memory leak (!). Main problem is that this object is still listening to
    // a StyleSheet of the model which was set. Thus, if You want to keep the obnject,
    // set the modfel to 0L, if object is not needed (seems to be the case here),
    // delete it.
    deleteSdrObjectSafeAndClearPointer(pMeasureObj);

    delete pModel;
}

/*************************************************************************
|*
|* SvxXMeasurePreview: Paint()
|*
*************************************************************************/

void SvxXMeasurePreview::Paint( const Rectangle&  )
{
    pMeasureObj->SingleObjectPainter(*this); // #110094#-17
}

/*************************************************************************
|*
|* SvxXMeasurePreview: SetAttributes()
|*
*************************************************************************/

void SvxXMeasurePreview::SetAttributes( const SfxItemSet& rInAttrs )
{
    //pMeasureObj->SetItemSetAndBroadcast(rInAttrs);
    pMeasureObj->SetMergedItemSetAndBroadcast(rInAttrs);

    Invalidate();
}

/*************************************************************************
|*
|* SvxXMeasurePreview: SetAttributes()
|*
*************************************************************************/

void SvxXMeasurePreview::MouseButtonDown( const MouseEvent& rMEvt )
{
    sal_Bool bZoomIn  = rMEvt.IsLeft() && !rMEvt.IsShift();
    sal_Bool bZoomOut = rMEvt.IsRight() || rMEvt.IsShift();
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


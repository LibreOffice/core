/*************************************************************************
 *
 *  $RCSfile: measctrl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// include ---------------------------------------------------------------

#pragma hdrstop

#include "xoutx.hxx"
#include "svdomeas.hxx"
#include "svdmodel.hxx"

//#include "svdrwobj.hxx" // SdrPaintInfoRec

#include "measctrl.hxx"
#include "dialmgr.hxx"

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
    pExtOutDev = new ExtOutputDevice( this );

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

    pMeasureObj = new SdrMeasureObj( aPt1, aPt2 );

    pModel = new SdrModel();
    pMeasureObj->SetModel( pModel );

    pMeasureObj->SetAttributes( rInAttrs, FALSE );

    Invalidate();
}

/*************************************************************************
|*
|* Dtor SvxXMeasurePreview
|*
*************************************************************************/

SvxXMeasurePreview::~SvxXMeasurePreview()
{
    delete pExtOutDev;
    delete pModel;
}

/*************************************************************************
|*
|* SvxXMeasurePreview: Paint()
|*
*************************************************************************/

void SvxXMeasurePreview::Paint( const Rectangle& rRect )
{
    SdrPaintInfoRec aInfoRec;

    pMeasureObj->Paint( *pExtOutDev, aInfoRec );
}

/*************************************************************************
|*
|* SvxXMeasurePreview: SetAttributes()
|*
*************************************************************************/

void SvxXMeasurePreview::SetAttributes( const SfxItemSet& rInAttrs )
{
    pMeasureObj->SetAttributes( rInAttrs, FALSE );
    Invalidate();
}

/*************************************************************************
|*
|* SvxXMeasurePreview: SetAttributes()
|*
*************************************************************************/

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


/*************************************************************************
 *
 *  $RCSfile: dlgctl3d.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:07 $
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

#pragma hdrstop

#include "dialogs.hrc"

#include <math.h>

#ifndef _SVX_XFLCLIT_HXX //autogen
#include <xflclit.hxx>
#endif
#ifndef _SVX_FMMODEL_HXX
#include <fmmodel.hxx>
#endif
#ifndef _SVX_FMPAGE_HXX
#include <fmpage.hxx>
#endif
#ifndef _E3D_VIEW3D_HXX //autogen
#include <view3d.hxx>
#endif
#ifndef _E3D_POLYSC3D_HXX //autogen
#include <polysc3d.hxx>
#endif
#ifndef _E3D_OBJ3D_HXX //autogen
#include <obj3d.hxx>
#endif
#ifndef _CAMERA3D_HXX //autogen
#include <camera3d.hxx>
#endif
#ifndef _VOLUME3D_HXX //autogen
#include <volume3d.hxx>
#endif
#ifndef _E3D_SPHERE3D_HXX //autogen
#include <sphere3d.hxx>
#endif
#ifndef _E3D_CUBE3D_HXX //autogen
#include <cube3d.hxx>
#endif
#ifndef _SV_EVENT_HXX //autogen
#include <vcl/event.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif
#ifndef _SFXSTYLE_HXX
#include <svtools/style.hxx>
#endif

#include "dlgctl3d.hxx"
#include <goodies/base3d.hxx>
#include <tools/link.hxx>

/*************************************************************************
|*  3D Preview Control
|*  Ctor
\************************************************************************/
Svx3DPreviewControl::Svx3DPreviewControl( Window* pParent, const ResId& rResId ) :
            Control     ( pParent, rResId ),
            pModel      ( NULL ),
            pFmPage     ( NULL ),
            pScene      ( NULL ),
            p3DView     ( NULL ),
            p3DObj      ( NULL ),
            nObjectType ( PREVIEW_OBJECTTYPE_SPHERE )
{
    Construct();
}

/*************************************************************************
|* Ctor
\************************************************************************/
Svx3DPreviewControl::Svx3DPreviewControl( Window* pParent, WinBits nStyle ) :
            Control     ( pParent, nStyle ),
            pModel      ( NULL ),
            pFmPage     ( NULL ),
            pScene      ( NULL ),
            p3DView     ( NULL ),
            p3DObj      ( NULL ),
            nObjectType ( PREVIEW_OBJECTTYPE_SPHERE )
{
    Construct();
}

/*************************************************************************
|* Dtor
\************************************************************************/
Svx3DPreviewControl::~Svx3DPreviewControl()
{
    delete p3DView;
    delete pModel;
}

/*************************************************************************
|* Svx3DPreviewControl::Construct
\************************************************************************/
void Svx3DPreviewControl::Construct()
{
    SetMapMode( MAP_100TH_MM );

    // Model
    pModel = new FmFormModel();
    pModel->GetItemPool().FreezeIdRanges();

    // Page
    pFmPage = new FmFormPage( *pModel, NULL );
    pModel->InsertPage( pFmPage, 0 );

    // 3D View
    p3DView = new E3dView( pModel, this );

    // 3D Scene
    pScene = new E3dPolyScene(p3DView->Get3DDefaultAttributes());

    // Objekt
    SetObjectType(PREVIEW_OBJECTTYPE_SPHERE);

    // Kameraeinstellungen, Perspektive ...
    Camera3D& rCamera  = (Camera3D&) pScene->GetCamera();
    const Volume3D& rVolume = pScene->GetBoundVolume();
    double fW = rVolume.GetWidth();
    double fH = rVolume.GetHeight();
    double fCamZ = rVolume.MaxVec().Z() + ((fW + fH) / 2.0);

    rCamera.SetAutoAdjustProjection(FALSE);
    rCamera.SetViewWindow(- fW / 2, - fH / 2, fW, fH);
    Vector3D aLookAt;
    Vector3D aCamPos(p3DView->DefaultCamPos().X (), p3DView->DefaultCamPos().Y (),
        fCamZ < p3DView->DefaultCamPos().Z () ? p3DView->DefaultCamPos().Z () : fCamZ);
    rCamera.SetPosAndLookAt(aCamPos, aLookAt);
    rCamera.SetFocalLength(p3DView->DefaultCamFocal());
    rCamera.SetDefaults(p3DView->DefaultCamPos(), aLookAt, p3DView->DefaultCamFocal());

    pScene->SetCamera( rCamera );
    pFmPage->InsertObject( pScene );

    pScene->RotateX( DEG2RAD( 25 ) );
    pScene->RotateY( DEG2RAD( 40 ) ); // Weil es auch ein Wuerfel sein kann

    // SnapRects der Objekte ungueltig
    pScene->SetRectsDirty();

    // Transformationen initialisieren, damit bei RecalcSnapRect()
    // richtig gerechnet wird
    pScene->InitTransformationSet();

    SfxItemSet aSet( pModel->GetItemPool(),
        XATTR_LINESTYLE, XATTR_LINESTYLE,
        XATTR_FILL_FIRST, XATTR_FILLBITMAP, 0 );
    aSet.Put( XLineStyleItem( XLINE_NONE ) );
    aSet.Put( XFillStyleItem( XFILL_SOLID ) );
    aSet.Put( XFillColorItem( String(), Color( COL_WHITE ) ) );
    pScene->NbcSetAttributes( aSet, FALSE );

    // Default-Attribute holen (ohne markiertes Objekt)
//  SfxItemSet aDefaultSet = p3DView->Get3DAttributes();

    // PageView
    SdrPageView* pPageView = p3DView->ShowPage( pFmPage, Point() );
    p3DView->SetMarkHdlHidden( TRUE );

    // Szene markieren
    p3DView->MarkObj( pScene, pPageView );

    // Initiale Groesse
    pScene->FitSnapRectToBoundVol();
//  Set3DAttributes(aDefaultSet);
}

/*************************************************************************
|* Svx3DPreviewControl::Resize
\************************************************************************/
void Svx3DPreviewControl::Resize()
{
    // Seite der Page
    Size aSize( GetSizePixel() );
    aSize = PixelToLogic( aSize );
    pFmPage->SetSize( aSize );

    // Groesse setzen
    Size aObjSize( aSize.Width()*5/6, aSize.Height()*5/6 );
    Point aObjPoint( (aSize.Width() - aObjSize.Width()) / 2,
        (aSize.Height() - aObjSize.Height()) / 2);
    Rectangle aRect( aObjPoint, aObjSize);
    pScene->SetSnapRect( aRect );
}

/*************************************************************************
|* Svx3DPreviewControl::Paint
\************************************************************************/
void Svx3DPreviewControl::Paint( const Rectangle& rRect )
{
    p3DView->InitRedraw( this, Region( rRect ) );
}

/*************************************************************************
|* Svx3DPreviewControl::MouseButtonDown
\************************************************************************/
void Svx3DPreviewControl::MouseButtonDown( const MouseEvent& rMEvt )
{
    Control::MouseButtonDown( rMEvt );
    if( rMEvt.IsShift() && rMEvt.IsMod1() )
    {
        SetObjectType( (nObjectType+1) % 2 );
    }
}

/*************************************************************************
|* Svx3DPreviewControl::SetObjectType
\************************************************************************/
void Svx3DPreviewControl::SetObjectType( UINT16 nType )
{
    if( nObjectType != nType || !p3DObj)
    {
        SfxItemSet aSet(pModel->GetItemPool(),
            SDRATTR_START,  SDRATTR_END,
            SID_ATTR_3D_START, SID_ATTR_3D_END,
            0);
        nObjectType = nType;

        if( p3DObj )
        {
            p3DObj->TakeAttributes( aSet, FALSE, FALSE );
            pScene->Remove3DObj( p3DObj );
            delete p3DObj;
            p3DObj = NULL;
        }

        switch( nType )
        {
            case PREVIEW_OBJECTTYPE_SPHERE:
            {
                // Kugel erzeugen
                p3DObj = new E3dSphereObj(
                    p3DView->Get3DDefaultAttributes(),
                    Vector3D( 0, 0, 0 ),
                    Vector3D( 1000, 1000, 1000 ));
            }
            break;

            case PREVIEW_OBJECTTYPE_CUBE:
            {
                // Wuerfel erzeugen
                p3DObj = new E3dCubeObj(
                    p3DView->Get3DDefaultAttributes(),
                    Vector3D( -500, -500, -500 ),
                    Vector3D( 1000, 1000, 1000 ));
            }
            break;
        }

        // Objekte groesser machen, da meisst nur 1x1x1 cm gross nach
        // dem Erzeugen
        Matrix4D aScaleMat;
        aScaleMat.Scale(Vector3D(5.0, 5.0, 5.0));
        p3DObj->ApplyTransform(aScaleMat);

        // Rein in die Szene
        pScene->Insert3DObj( p3DObj );
        p3DObj->NbcSetAttributes( aSet, FALSE );

        // Refresh
        Resize();
    }
}

/*************************************************************************
|* Svx3DPreviewControl::Get3DAttributes
\************************************************************************/
SfxItemSet Svx3DPreviewControl::Get3DAttributes() const
{
    return( p3DView->Get3DAttributes( pScene ) );
}
/*************************************************************************
|* Svx3DPreviewControl::Set3DAttributes
\************************************************************************/
void Svx3DPreviewControl::Set3DAttributes( const SfxItemSet& rAttr )
{
    p3DView->Set3DAttributes( rAttr, pScene, TRUE );
    Resize();
}

/*************************************************************************
|* Svx3DPreviewControl::SetObjectType
\************************************************************************/
void Svx3DPreviewControl::Set3DObject( const E3dObject* pObj )
{
    if( pObj->ISA( E3dCompoundObject ) )
    {
        pScene->Remove3DObj( p3DObj );
        delete p3DObj;
        p3DObj = (E3dCompoundObject*)pObj->Clone();
        pScene->Insert3DObj( p3DObj );
        Resize();
    }
    else if( pObj->ISA( E3dPolyScene ) )
    {
        delete pFmPage->RemoveObject( pScene->GetOrdNum() );
        p3DObj = NULL;
        pScene = (E3dPolyScene*)pObj->Clone();
        pFmPage->InsertObject( pScene );
        Resize();
    }
}

/*************************************************************************
|*
|*  Control zur Darstellung und Auswahl der Eckpunkte (und Mittelpunkt)
|*  eines 3D-Objekts
|*
\************************************************************************/

SvxRectCtl3D::SvxRectCtl3D( Window* pParent,
                            const ResId& rResId,
                            USHORT nBorderWidth,
                            USHORT nBorderHeight,
                            USHORT nCircle ) :
                        Control( pParent, rResId ),
                        nBW( nBorderWidth ),
                        nBH( nBorderHeight ),
                        nRadius( nCircle)

{
    SetMapMode( MAP_100TH_MM );
    SetBackground( Wallpaper( Color( COL_LIGHTGRAY ) ) );

    aSize = GetOutputSize();
    long nW = aSize.Width() - nBW;
    long nH = aSize.Height() - nBH;
    long nBWh = nBW / 2; // BorderWidthHalf
    long nBHh = nBH / 2; // BorderHeightHalf

    // PointArray wird mit Koordinaten des Controls gefuellt,
    // um schneller painten zu k”nnen

    aPointArray[0]  = Point(); // Leer -> dummy
    aPointArray[1]  = Point( nW*1/2 + nBWh, 0      + nBHh );
    aPointArray[2]  = Point( nW*1/4 + nBWh, nH*1/8 + nBHh );
    aPointArray[3]  = Point( nW*3/4 + nBWh, nH*1/8 + nBHh );
    aPointArray[4]  = Point( 0      + nBWh, nH*2/8 + nBHh );
    aPointArray[5]  = Point( nW*1/2 + nBWh, nH*2/8 + nBHh );
    aPointArray[6]  = Point( nW     + nBWh, nH*2/8 + nBHh );
    aPointArray[7]  = Point( nW*1/4 + nBWh, nH*3/8 + nBHh );
    aPointArray[8]  = Point( nW*3/4 + nBWh, nH*3/8 + nBHh );
    aPointArray[9]  = Point( 0      + nBWh, nH*4/8 + nBHh );
    aPointArray[10] = Point( nW*1/2 + nBWh, nH*4/8 + nBHh );
    aPointArray[11] = Point( nW     + nBWh, nH*4/8 + nBHh );
    aPointArray[12] = Point( nW*1/4 + nBWh, nH*5/8 + nBHh );
    aPointArray[13] = Point( nW*3/4 + nBWh, nH*5/8 + nBHh );
    aPointArray[14] = Point( 0      + nBWh, nH*6/8 + nBHh );
    aPointArray[15] = Point( nW*1/2 + nBWh, nH*6/8 + nBHh );
    aPointArray[16] = Point( nW     + nBWh, nH*6/8 + nBHh );
    aPointArray[17] = Point( nW*1/4 + nBWh, nH*7/8 + nBHh );
    aPointArray[18] = Point( nW*3/4 + nBWh, nH*7/8 + nBHh );
    aPointArray[19] = Point( nW*1/2 + nBWh, nH     + nBHh );

    // Distanz -> halbe Rahmenbreite
    nDist = (short) nBWh;

    aPolyPoints1[0] = Point( aPointArray[19].X(),
                         aPointArray[19].Y() - nDist );
    aPolyPoints1[1] = Point( ( aPointArray[14].X() + aPointArray[17].X() ) / 2,
                         ( aPointArray[14].Y() + aPointArray[17].Y() ) / 2 - nDist );
    aPolyPoints1[2] = Point( ( aPointArray[4].X() + aPointArray[7].X() ) / 2,
                         ( aPointArray[4].Y() + aPointArray[7].Y() ) / 2 + nDist );
    aPolyPoints1[3] = Point( aPointArray[10].X(),
                         aPointArray[10].Y() + nDist );

    aPolyPoints2[0] = aPolyPoints1[0];
    aPolyPoints2[1] = Point( ( aPointArray[16].X() + aPointArray[18].X() ) / 2,
                         ( aPointArray[16].Y() + aPointArray[18].Y() ) / 2 - nDist);
    aPolyPoints2[2] = Point( ( aPointArray[6].X() + aPointArray[8].X() ) / 2,
                         ( aPointArray[6].Y() + aPointArray[8].Y() ) / 2 + nDist );
    aPolyPoints2[3] = aPolyPoints1[3];

    aPolyPoints3[0] = aPolyPoints1[3];
    aPolyPoints3[1] = aPolyPoints1[2];
    aPolyPoints3[2] = Point( ( aPointArray[2].X() + aPointArray[3].X() ) / 2,
                         ( aPointArray[2].Y() + aPointArray[3].Y() ) / 2 + nDist );
    aPolyPoints3[3] = aPolyPoints2[2];

    aPoly1 = Polygon( 4, aPolyPoints1 );
    aPoly2 = Polygon( 4, aPolyPoints2 );
    aPoly3 = Polygon( 4, aPolyPoints3 );

    Reset();
}

/*************************************************************************
|*
|*  Dtor
|*
\************************************************************************/

SvxRectCtl3D::~SvxRectCtl3D()
{
}

/*************************************************************************
|*
|*  Zeichnet das Control (Rechteck mit 9 Kreisen)
|*
\************************************************************************/

void SvxRectCtl3D::Paint( const Rectangle& rRect )
{

    if( IsEnabled() )
        SetLineColor( Color( COL_BLACK ) );
    else
        SetLineColor( Color( COL_GRAY ) );

    // Zeichnen des Polygons ( Darstellungsobjekt )
    SetFillColor( Color( COL_GRAY ) );
    DrawPolygon( aPoly1 );
    SetFillColor( Color( COL_CYAN ) );
    DrawPolygon( aPoly2 );
    SetFillColor( Color( COL_LIGHTGRAY ) );
    DrawPolygon( aPoly3 );

    // Zeichnen des Drahtgeruestes
    DrawLine( aPointArray[ 1], aPointArray[ 4] );
    DrawLine( aPointArray[ 1], aPointArray[ 6] );
    DrawLine( aPointArray[ 4], aPointArray[10] );
    DrawLine( aPointArray[ 6], aPointArray[10] );
    DrawLine( aPointArray[ 4], aPointArray[14] );
    DrawLine( aPointArray[10], aPointArray[19] );
    DrawLine( aPointArray[ 6], aPointArray[16] );
    DrawLine( aPointArray[14], aPointArray[19] );
    DrawLine( aPointArray[16], aPointArray[19] );

    // Zeichnen der Kreise
    if( IsEnabled() )
    {
        SetLineColor( Color( COL_LIGHTBLUE ) );
        SetFillColor( Color( COL_WHITE ) );
    }
    else
    {
        SetLineColor( Color( COL_GRAY ) );
        SetFillColor( Color( COL_LIGHTGRAY ) );
    }
    for( int i = 1; i < 20; i++ )
    {
        if( nActPoint == i )
        {
            const Color& rOldLineColor = GetLineColor();
            const Color& rOldFillColor = GetFillColor();

            SetLineColor( Color( COL_YELLOW ) );
            SetFillColor( Color( COL_WHITE ) );

            DrawEllipse( Rectangle(
                    aPointArray[i] - Point( nRadius+100, nRadius+100 ),
                    aPointArray[i] + Point( nRadius+100, nRadius+100 ) ) );

            DrawEllipse( Rectangle(
                    aPointArray[i] - Point( nRadius+50, nRadius+50 ),
                    aPointArray[i] + Point( nRadius+50, nRadius+50 ) ) );

            SetFillColor( Color( COL_YELLOW ) );

            DrawEllipse( Rectangle(
                    aPointArray[i] - Point( nRadius, nRadius ),
                    aPointArray[i] + Point( nRadius, nRadius ) ) );

            SetLineColor( rOldLineColor );
            SetFillColor( rOldFillColor );
        }
        else
            DrawEllipse( Rectangle(
                aPointArray[i] - Point( nRadius, nRadius ),
                aPointArray[i] + Point( nRadius, nRadius ) ) );
    }
}

/*************************************************************************
|*
|*  Das angeklickte Rechteck wird ermittelt um die Farbe zu wechseln
|*
\************************************************************************/

void SvxRectCtl3D::MouseButtonDown( const MouseEvent& rMEvt )
{
    Point aPt = PixelToLogic( rMEvt.GetPosPixel() );

    for( int i = 1; i < 20; i++ )
    {
        Rectangle aRect( aPointArray[i] - Point( nRadius, nRadius ),
                         aPointArray[i] + Point( nRadius, nRadius ) );
        if( aRect.IsInside( aPt ) )
        {
            nOldPoint = nActPoint;
            nActPoint = i;

            // Neuen Kreis neu Zeichnen
            aRect = Rectangle( aPointArray[i] - Point( nRadius+100, nRadius+100 ),
                        aPointArray[i] + Point( nRadius+100, nRadius+100 ) );
            Invalidate( aRect );

            // Alten Kreis neu Zeichnen
            aRect = Rectangle( aPointArray[nOldPoint] - Point( nRadius+100, nRadius+100 ),
                        aPointArray[nOldPoint] + Point( nRadius+100, nRadius+100 ) );
            Invalidate( aRect );

            break;
        }
    }
}

/*************************************************************************
|*
|*  Bewirkt den Ursprungszustand des Controls
|*
\************************************************************************/

void SvxRectCtl3D::Reset()
{
    nActPoint = 10;
    nOldPoint = 0;
    Invalidate();
}

/*************************************************************************
|*
|*  Gibt den aktuell ausgewaehlten Point als Vector zurueck
|*
\************************************************************************/

Vector3D SvxRectCtl3D::GetVector()
{
    return( PointNumToVector( nActPoint ) );
}

/*************************************************************************
|*
|*  Setzt den uebergebenen Vector als Point
|*
\************************************************************************/

void SvxRectCtl3D::SetPoint( Vector3D nVect )
{
    nActPoint = VectorToPointNum( nVect );
    if( nActPoint )
        Invalidate();
}

/*************************************************************************
|*
|*  Konvertiert den uebergebenen Punkt in einen Vector3D
|*
\************************************************************************/

Vector3D SvxRectCtl3D::PointNumToVector( short nPoint )
{
    Vector3D aVect;

    switch( nPoint )
    {
        case  1: aVect.X() =  0.0;  aVect.Y() =  1.0; aVect.Z() = -1.0; break;
        case  2: aVect.X() = -1.0;  aVect.Y() =  1.0; aVect.Z() = -1.0; break;
        case  3: aVect.X() =  1.0;  aVect.Y() =  1.0; aVect.Z() = -1.0; break;
        case  4: aVect.X() = -1.0;  aVect.Y() =  1.0; aVect.Z() =  0.0; break;
        case  5: aVect.X() =  0.0;  aVect.Y() =  1.0; aVect.Z() =  0.0; break;
        case  6: aVect.X() =  1.0;  aVect.Y() =  1.0; aVect.Z() =  0.0; break;
        case  7: aVect.X() = -1.0;  aVect.Y() =  1.0; aVect.Z() =  1.0; break;
        case  8: aVect.X() =  1.0;  aVect.Y() =  1.0; aVect.Z() =  1.0; break;
        case  9: aVect.X() = -1.0;  aVect.Y() =  0.0; aVect.Z() =  0.0; break;
        case 10: aVect.X() =  0.0;  aVect.Y() =  1.0; aVect.Z() =  1.0; break;
        case 11: aVect.X() =  1.0;  aVect.Y() =  0.0; aVect.Z() =  0.0; break;
        case 12: aVect.X() = -1.0;  aVect.Y() =  0.0; aVect.Z() =  1.0; break;
        case 13: aVect.X() =  1.0;  aVect.Y() =  0.0; aVect.Z() =  1.0; break;
        case 14: aVect.X() = -1.0;  aVect.Y() = -1.0; aVect.Z() =  0.0; break;
        case 15: aVect.X() =  0.0;  aVect.Y() =  0.0; aVect.Z() =  1.0; break;
        case 16: aVect.X() =  1.0;  aVect.Y() = -1.0; aVect.Z() =  0.0; break;
        case 17: aVect.X() = -1.0;  aVect.Y() = -1.0; aVect.Z() =  1.0; break;
        case 18: aVect.X() =  1.0;  aVect.Y() = -1.0; aVect.Z() =  1.0; break;
        case 19: aVect.X() =  0.0;  aVect.Y() = -1.0; aVect.Z() =  1.0; break;
        default: aVect.X() = -1.0;  aVect.Y() = -1.0; aVect.Z() = -1.0; break;
    }

    return( aVect );
}

/*************************************************************************
|*
|*  Konvertiert den uebergebenen Vector3D in einen Punkt
|*
\************************************************************************/

short SvxRectCtl3D::VectorToPointNum( Vector3D aVect )
{
    short nPoint = 0;

    if     ( fabs (aVect.X()          ) < 1e-3 && fabs (aVect.Y() - 0.70711) < 1e-3 && fabs (aVect.Z() + 0.70711) < 1e-3 ) nPoint =  1;
    else if( fabs (aVect.X() + 0.57735) < 1e-3 && fabs (aVect.Y() - 0.57735) < 1e-3 && fabs (aVect.Z() + 0.57735) < 1e-3 ) nPoint =  2;
    else if( fabs (aVect.X() - 0.57735) < 1e-3 && fabs (aVect.Y() - 0.57735) < 1e-3 && fabs (aVect.Z() + 0.57735) < 1e-3 ) nPoint =  3;
    else if( fabs (aVect.X() + 0.70711) < 1e-3 && fabs (aVect.Y() - 0.70711) < 1e-3 && fabs (aVect.Z()          ) < 1e-3 ) nPoint =  4;
    else if( fabs (aVect.X()          ) < 1e-3 && fabs (aVect.Y() - 1.0    ) < 1e-3 && fabs (aVect.Z()          ) < 1e-3 ) nPoint =  5;
    else if( fabs (aVect.X() - 0.70711) < 1e-3 && fabs (aVect.Y() - 0.70711) < 1e-3 && fabs (aVect.Z()          ) < 1e-3 ) nPoint =  6;
    else if( fabs (aVect.X() + 0.57735) < 1e-3 && fabs (aVect.Y() - 0.57735) < 1e-3 && fabs (aVect.Z() - 0.57735) < 1e-3 ) nPoint =  7;
    else if( fabs (aVect.X() - 0.57735) < 1e-3 && fabs (aVect.Y() - 0.57735) < 1e-3 && fabs (aVect.Z() - 0.57735) < 1e-3 ) nPoint =  8;
    else if( fabs (aVect.X() + 1.0    ) < 1e-3 && fabs (aVect.Y()          ) < 1e-3 && fabs (aVect.Z()          ) < 1e-3 ) nPoint =  9;
    else if( fabs (aVect.X()          ) < 1e-3 && fabs (aVect.Y() - 0.70711) < 1e-3 && fabs (aVect.Z() - 0.70711) < 1e-3 ) nPoint = 10;
    else if( fabs (aVect.X() - 1.0    ) < 1e-3 && fabs (aVect.Y()          ) < 1e-3 && fabs (aVect.Z()          ) < 1e-3 ) nPoint = 11;
    else if( fabs (aVect.X() + 0.70711) < 1e-3 && fabs (aVect.Y()          ) < 1e-3 && fabs (aVect.Z() - 0.70711) < 1e-3 ) nPoint = 12;
    else if( fabs (aVect.X() - 0.70711) < 1e-3 && fabs (aVect.Y()          ) < 1e-3 && fabs (aVect.Z() - 0.70711) < 1e-3 ) nPoint = 13;
    else if( fabs (aVect.X() + 0.70711) < 1e-3 && fabs (aVect.Y() + 0.70711) < 1e-3 && fabs (aVect.Z()          ) < 1e-3 ) nPoint = 14;
    else if( fabs (aVect.X()          ) < 1e-3 && fabs (aVect.Y()          ) < 1e-3 && fabs (aVect.Z() - 1.0    ) < 1e-3 ) nPoint = 15;
    else if( fabs (aVect.X() - 0.70711) < 1e-3 && fabs (aVect.Y() + 0.70711) < 1e-3 && fabs (aVect.Z()          ) < 1e-3 ) nPoint = 16;
    else if( fabs (aVect.X() + 0.57735) < 1e-3 && fabs (aVect.Y() + 0.57735) < 1e-3 && fabs (aVect.Z() - 0.57735) < 1e-3 ) nPoint = 17;
    else if( fabs (aVect.X() - 0.57735) < 1e-3 && fabs (aVect.Y() + 0.57735) < 1e-3 && fabs (aVect.Z() - 0.57735) < 1e-3 ) nPoint = 18;
    else if( fabs (aVect.X()          ) < 1e-3 && fabs (aVect.Y() + 0.70711) < 1e-3 && fabs (aVect.Z() - 0.70711) < 1e-3 ) nPoint = 19;

    return( nPoint );
}





/*************************************************************************
|*
|*  3D Preview Control
|*
\************************************************************************/

SvxPreviewCtl3D::SvxPreviewCtl3D( Window* pParent, const ResId& rResId)
:   Control( pParent, rResId )
{
    // Members initialisieren
    Init();
}

SvxPreviewCtl3D::SvxPreviewCtl3D( Window* pParent, WinBits nStyle)
:   Control( pParent, nStyle)
{
    // Members initialisieren
    Init();
}

void SvxPreviewCtl3D::Init()
{
    // Members mit Defaults fuellen
    bGeometryCube=FALSE;
    fRotateX=-20.0;
    fRotateY=45.0;
    fRotateZ=0.0;
    fDistance=10.0;
    fDeviceSize=1.5;

    // MapMode waehlen
    SetMapMode( MAP_100TH_MM );

    // Hintergrund in einem schoenen neutralen Grau
    SetBackground( Wallpaper( Color( COL_GRAY ) ) );

    // Segmente
    nHorSegs = 24;
    nVerSegs = 12;

    // Normalenmodus
    nNormalMode = PREVIEW_NORMAL_MODE_OBJECT;

    // ShadeMode
    nShadeMode = PREVIEW_SHADEMODE_GOURAUD;

    // Geometrie erzeugen
    CreateGeometry();

    // Material initialisieren
    Color aColWhite(COL_WHITE);
    Color aColBlack(COL_BLACK);

    aObjectMaterial.SetMaterial(aColWhite, Base3DMaterialAmbient);
    aObjectMaterial.SetMaterial(aColWhite, Base3DMaterialDiffuse);
    aObjectMaterial.SetMaterial(aColWhite, Base3DMaterialSpecular);
    aObjectMaterial.SetMaterial(aColBlack, Base3DMaterialEmission);
    aObjectMaterial.SetShininess(32);
}

SvxPreviewCtl3D::~SvxPreviewCtl3D()
{
}

void SvxPreviewCtl3D::Paint( const Rectangle& rRect )
{
    // Base3D anfordern
    Base3D* pBase3D = Base3D::Create(this, nShadeMode == PREVIEW_SHADEMODE_DRAFT);

    Rectangle aVisible(Point(0,0), GetOutputSizePixel());
    aVisible = PixelToLogic(aVisible);

    // Orientierung
    Matrix4D mOrient;
    aCameraSet.SetObjectTrans(mOrient);
    mOrient.Orientation(
        Point4D(0.0, 0.0, fDistance, 1.0),
        Vector3D(0.0, 0.0, 1.0),
        Vector3D(0.0, 1.0, 0.0));
    aCameraSet.SetOrientation(mOrient);

    // Matritzen setzen
    pBase3D->SetTransformationSet(&aCameraSet);

    // Licht setzen
    pBase3D->SetLightGroup(&aLights);

    // ShadeMode setzen
    if(nShadeMode == PREVIEW_SHADEMODE_FLAT || nShadeMode == PREVIEW_SHADEMODE_DRAFT)
        pBase3D->SetShadeModel(Base3DFlat);
    else if(nShadeMode == PREVIEW_SHADEMODE_GOURAUD)
        pBase3D->SetShadeModel(Base3DSmooth);
    else
        pBase3D->SetShadeModel(Base3DPhong);

    // Ausgaberechteck setzen
    aCameraSet.SetDeviceRectangle(-fDeviceSize, fDeviceSize, -fDeviceSize, fDeviceSize, FALSE);
    aCameraSet.SetFrontClippingPlane(fDistance - fDeviceSize);
    aCameraSet.SetBackClippingPlane(fDistance + fDeviceSize);
    aCameraSet.SetViewportRectangle(aVisible);

    // Matritzen setzen
    pBase3D->SetTransformationSet(&aCameraSet);

    // Werte fuer Objekt setzen
    pBase3D->SetActiveTexture();
    pBase3D->SetMaterial(aObjectMaterial.GetMaterial(Base3DMaterialAmbient), Base3DMaterialAmbient);
    pBase3D->SetMaterial(aObjectMaterial.GetMaterial(Base3DMaterialDiffuse), Base3DMaterialDiffuse);
    pBase3D->SetMaterial(aObjectMaterial.GetMaterial(Base3DMaterialSpecular), Base3DMaterialSpecular);
    pBase3D->SetMaterial(aObjectMaterial.GetMaterial(Base3DMaterialEmission), Base3DMaterialEmission);
    pBase3D->SetShininess(aObjectMaterial.GetShininess());

    pBase3D->SetRenderMode(Base3DRenderFill);
    pBase3D->SetCullMode(Base3DCullBack);

    // ScissorRegion defaultmaessig disablen
    pBase3D->ActivateScissorRegion(FALSE);

    // Nicht flach
    pBase3D->SetForceFlat(FALSE);

    // Geometrie ausgeben
    DrawGeometryClip(pBase3D);
}

void SvxPreviewCtl3D::DrawGeometryClip(Base3D *pBase3D)
{
    // spezielles Clipping fuer OpenGL, um keine floating windows ueberzumalen
    if(pBase3D->GetBase3DType() == BASE3D_TYPE_OPENGL
        && GetOutDevType() == OUTDEV_WINDOW
        && pBase3D->GetTransformationSet())
    {
        Window* pWin = (Window*)this;
        Region aClipRegion = pWin->GetActiveClipRegion();

        // ClipRegion ist gesetzt, benutze diese
        RegionHandle aRegionHandle = aClipRegion.BeginEnumRects();
        Rectangle aClipRect;

        while(aClipRegion.GetEnumRects(aRegionHandle, aClipRect))
        {
            if(aClipRect.IsOver(pBase3D->GetTransformationSet()->GetLogicalViewportBounds()))
            {
                // Viewport setzen
                pBase3D->SetScissorRegion(aClipRect, TRUE);

                // Zeichne alle Objekte
                pBase3D->StartScene();
                DrawGeometry(pBase3D);
                pBase3D->EndScene();
            }
        }
        aClipRegion.EndEnumRects(aRegionHandle);
    }
    else
    {
        // Zeichne alle Objekte
        pBase3D->StartScene();
        DrawGeometry(pBase3D);
        pBase3D->EndScene();
    }
}

void SvxPreviewCtl3D::DrawGeometry(Base3D *pBase3D)
{
    pBase3D->DrawPolygonGeometry(aGeometry);
}

void SvxPreviewCtl3D::SetGeometry(BOOL bGeomCube)
{
    if(bGeometryCube != bGeomCube)
    {
        bGeometryCube = bGeomCube;
        CreateGeometry();
    }
    Invalidate();
}

void SvxPreviewCtl3D::SetRotation(double fRotX, double fRotY, double fRotZ)
{
    if(fRotX != fRotateX || fRotY != fRotateY || fRotZ != fRotateZ)
    {
        fRotateX = fRotX;
        fRotateY = fRotY;
        fRotateZ = fRotZ;
        CreateGeometry();
    }
    Invalidate();
}

void SvxPreviewCtl3D::GetRotation(double& rRotX, double& rRotY, double& rRotZ)
{
    rRotX = fRotateX;
    rRotY = fRotateY;
    rRotZ = fRotateZ;
}

// Zugriffsfunktionen Materialien
void SvxPreviewCtl3D::SetMaterial(Color rNew, Base3DMaterialValue eVal)
{
    if(aObjectMaterial.GetMaterial(eVal) != rNew)
    {
        aObjectMaterial.SetMaterial(rNew, eVal);
        Invalidate();
    }
}

Color SvxPreviewCtl3D::GetMaterial(Base3DMaterialValue eVal)
{
    return aObjectMaterial.GetMaterial(eVal);
}

void SvxPreviewCtl3D::SetShininess(UINT16 nNew)
{
    if(aObjectMaterial.GetShininess() != nNew)
    {
        aObjectMaterial.SetShininess(nNew);
        Invalidate();
    }
}

UINT16 SvxPreviewCtl3D::GetShininess()
{
    return aObjectMaterial.GetShininess();
}

// Lichtquellen setzen
void SvxPreviewCtl3D::SetLightGroup(B3dLightGroup* pNew)
{
    if(pNew)
    {
        aLights = *pNew;
        Invalidate();
    }
}

// View-Einstellungen
void SvxPreviewCtl3D::SetUserDistance(double fNew)
{
    if(fNew != fDistance)
    {
        fDistance = fNew;
        Invalidate();
    }
}

void SvxPreviewCtl3D::SetDeviceSize(double fNew)
{
    if(fNew != fDeviceSize)
    {
        fDeviceSize = fNew;
        Invalidate();
    }
}

// Zugriffsfunktionen Segmentierung
void SvxPreviewCtl3D::SetHorizontalSegments(UINT16 nNew)
{
    if(nNew != nHorSegs)
    {
        nHorSegs = nNew;
        CreateGeometry();
        Invalidate();
    }
}

void SvxPreviewCtl3D::SetVerticalSegments(UINT16 nNew)
{
    if(nNew != nVerSegs)
    {
        nVerSegs = nNew;
        CreateGeometry();
        Invalidate();
    }
}

void SvxPreviewCtl3D::SetSegments(UINT16 nNewHor, UINT16 nNewVer)
{
    if(nNewHor != nHorSegs || nNewVer != nVerSegs)
    {
        nHorSegs = nNewHor;
        nVerSegs = nNewVer;
        CreateGeometry();
        Invalidate();
    }
}

// Zugriff Normalenmodus
void SvxPreviewCtl3D::SetNormalMode(UINT16 nNew)
{
    if(nNew != nNormalMode)
    {
        nNormalMode = nNew;
        CreateGeometry();
        Invalidate();
    }
}

// Zugriff auf ShadeMode
void SvxPreviewCtl3D::SetShadeMode(UINT16 nNew)
{
    if(nNew != nShadeMode)
    {
        nShadeMode = nNew;
        Invalidate();
    }
}

void SvxPreviewCtl3D::CreateGeometry()
{
    // Wuerfel erzeugen fuer Objektgroesse
    B3dVolume aVolume;
    aVolume.MinVec() = Vector3D(-1.0, -1.0, -1.0);
    aVolume.MaxVec() = Vector3D( 1.0,  1.0,  1.0);

    if(bGeometryCube)
    {
        // Wuerfel erzeugen
        aGeometry.CreateCube(aVolume);
    }
    else
    {
        // AHCTUNG: Das PreviewControl hat bis zu dieser Stelle KEINE
        // Begrenzung in der Anzahl der Hor/Ver Segmente. Diese wird hier nun
        // explizit eingeschraenkt.
        double fHSegs = (nHorSegs > 50) ? 50.0 : (double)nHorSegs;
        double fVSegs = (nVerSegs > 50) ? 50.0 : (double)nVerSegs;

        // Kugel erzeugen
        aGeometry.CreateSphere(aVolume, fHSegs, fVSegs);
    }

    if(nNormalMode != PREVIEW_NORMAL_MODE_OBJECT)
    {
        if(!(nNormalMode == PREVIEW_NORMAL_MODE_FLAT))
        {
            aGeometry.CreateDefaultNormalsSphere();
        }
    }

    // Gesetzte Rotation ausfuehren
    if(fRotateX != 0.0 || fRotateY != 0.0 || fRotateZ != 0.0)
    {
        Matrix4D aRotMat;
        if(fRotateY != 0.0)
            aRotMat.RotateY(fRotateY * F_PI180);
        if(fRotateX != 0.0)
            aRotMat.RotateX(-fRotateX * F_PI180);
        if(fRotateZ != 0.0)
            aRotMat.RotateZ(fRotateZ * F_PI180);
        aGeometry.Transform(aRotMat);
    }
}

/*************************************************************************
|*
|*  3D Light Control
|*
\************************************************************************/

SvxLightPrevievCtl3D::SvxLightPrevievCtl3D( Window* pParent, const ResId& rResId)
:   SvxPreviewCtl3D(pParent, rResId)
{
    // Members initialisieren
    Init();
}

SvxLightPrevievCtl3D::SvxLightPrevievCtl3D( Window* pParent, WinBits nStyle )
:   SvxPreviewCtl3D(pParent, nStyle)
{
    // Members initialisieren
    Init();
}

void SvxLightPrevievCtl3D::Init()
{
    // Lokale Parameter fuellen
    eSelectedLight = Base3DLightNone;
    fObjectRadius = 1.414;
    fDistanceToObject = 0.4;
    fScaleSizeSelected = 1.8;
    fLampSize = 0.1;
    nInteractionStartDistance = 5 * 5 * 2;
    bMouseMoved = FALSE;
    bGeometrySelected = FALSE;

    // Device groesser, da Lampen angezeigt werden
    SetDeviceSize(2.0);

    // Geometrie fuer Lampenobjekt erzeugen
    CreateLightGeometry();
}

SvxLightPrevievCtl3D::~SvxLightPrevievCtl3D()
{
}

void SvxLightPrevievCtl3D::SelectLight(Base3DLightNumber eNew)
{
    if(eNew != eSelectedLight)
    {
        eSelectedLight = eNew;
        bGeometrySelected = FALSE;
        Invalidate();
    }
}

void SvxLightPrevievCtl3D::SelectGeometry()
{
    if(!bGeometrySelected)
    {
        bGeometrySelected = TRUE;
        eSelectedLight = Base3DLightNone;
        Invalidate();
    }
}

void SvxLightPrevievCtl3D::SetObjectRadius(double fNew)
{
    if(fObjectRadius != fNew)
    {
        fObjectRadius = fNew;
        Invalidate();
    }
}

void SvxLightPrevievCtl3D::SetDistanceToObject(double fNew)
{
    if(fDistanceToObject != fNew)
    {
        fDistanceToObject = fNew;
        Invalidate();
    }
}

void SvxLightPrevievCtl3D::SetScaleSizeSelected(double fNew)
{
    if(fScaleSizeSelected != fNew)
    {
        fScaleSizeSelected = fNew;
        Invalidate();
    }
}

void SvxLightPrevievCtl3D::SetLampSize(double fNew)
{
    if(fLampSize != fNew)
    {
        fLampSize = fNew;
        CreateLightGeometry();
        Invalidate();
    }
}

void SvxLightPrevievCtl3D::DrawGeometry(Base3D *pBase3D)
{
    // call parent; zeichnet das Objekt selbst
    SvxPreviewCtl3D::DrawGeometry(pBase3D);

    // Lichter zeichnen
    for(UINT16 a=0;a<BASE3D_MAX_NUMBER_LIGHTS;a++)
    {
        Base3DLightNumber eLightNum = (Base3DLightNumber)(Base3DLight0 + a);
        if(aLights.IsEnabled(eLightNum))
            DrawLightGeometry(eLightNum, pBase3D);
    }
}

void SvxLightPrevievCtl3D::DrawLightGeometry(Base3DLightNumber eLightNum,
    Base3D* pBase3D)
{
    // Geometrie bereitstellen
    B3dGeometry aNew;
    Matrix4D aTrans;
    double fRadius = fObjectRadius + fDistanceToObject;
    Color aLineColor(COL_YELLOW);
    aNew = aLightGeometry;

    if(eLightNum == eSelectedLight)
        aTrans.Scale(fScaleSizeSelected, fScaleSizeSelected, fScaleSizeSelected);

    Vector3D aDirection = aLights.GetDirection(eLightNum);
    aDirection.Normalize();
    aDirection *= fRadius;
    aTrans.Translate(aDirection);

    aNew.Transform(aTrans);

    // Material setzen
    Color aZwi;
    aZwi = aLights.GetIntensity(Base3DMaterialDiffuse, eLightNum);
    pBase3D->SetMaterial(aZwi, Base3DMaterialAmbient);
//  pBase3D->SetMaterial(aZwi, Base3DMaterialDiffuse);
    pBase3D->SetMaterial(aZwi, Base3DMaterialEmission);
    aZwi = aLights.GetIntensity(Base3DMaterialSpecular, eLightNum);
    pBase3D->SetMaterial(aZwi, Base3DMaterialSpecular);

    // Lampe Zeichnen
    pBase3D->SetRenderMode(Base3DRenderLine);
    pBase3D->DrawPolygonGeometry(aNew);

    if(eLightNum == eSelectedLight)
    {
        // Beleuchtung aus und Linienfarbe setzen
        BOOL bLightingWasEnabled = aLights.IsLightingEnabled();
        aLights.EnableLighting(FALSE);
        pBase3D->SetLightGroup(&aLights);
        pBase3D->SetLineWidth();

        // Kreis am Boden zeichnen
        Vector3D aPoint(0.0, -fRadius, fRadius);
        pBase3D->StartPrimitive(Base3DLineLoop);
        pBase3D->SetColor(aLineColor);

        double fWink;
        for(fWink=-F_PI;fWink < F_PI; fWink += F_2PI/24.0)
        {
            aPoint.Z() = -cos(fWink) * fRadius;
            aPoint.X() = -sin(fWink) * fRadius;
            pBase3D->AddVertex(aPoint);
        }
        pBase3D->EndPrimitive();

        // Kreisbogen zeichnen
        double fBodenWinkel = atan2(-aDirection.X(), -aDirection.Z());
        double fSinBoden = sin(fBodenWinkel) * fRadius;
        double fCosBoden = cos(fBodenWinkel) * fRadius;
        pBase3D->StartPrimitive(Base3DLineStrip);
        pBase3D->SetColor(aLineColor);

        for(fWink=-F_PI2;fWink < F_PI2; fWink += F_PI/12.0)
        {
            aPoint.X() = cos(fWink) * -fSinBoden;
            aPoint.Y() = sin(fWink) * fRadius;
            aPoint.Z() = cos(fWink) * -fCosBoden;
            pBase3D->AddVertex(aPoint);
        }
        pBase3D->EndPrimitive();

        // Verbindung zeichnen
        pBase3D->StartPrimitive(Base3DLineStrip);
        pBase3D->SetColor(aLineColor);
        aPoint = Vector3D(0.0, -fRadius, 0.0);
        pBase3D->AddVertex(aPoint);
        aPoint.X() = -fSinBoden;
        aPoint.Z() = -fCosBoden;
        pBase3D->AddVertex(aPoint);
        aPoint.Y() = 0.0;
        pBase3D->AddVertex(aPoint);
        pBase3D->EndPrimitive();

        // Beleuchtung wieder eischalten
        aLights.EnableLighting(bLightingWasEnabled);
        pBase3D->SetLightGroup(&aLights);
    }
}

void SvxLightPrevievCtl3D::CreateLightGeometry()
{
    // Wuerfel erzeugen fuer Objektgroesse
    B3dVolume aVolume;
    aVolume.MinVec() = Vector3D(-fLampSize, -fLampSize, -fLampSize);
    aVolume.MaxVec() = Vector3D( fLampSize,  fLampSize,  fLampSize);

    // Kugel erzeugen
    aLightGeometry.CreateSphere(aVolume, 4.0, 3.0);
}

// Selektion gueltig? D.h.: Lampe ist Selektiert un auch EINGESCHALTET
BOOL SvxLightPrevievCtl3D::IsSelectionValid()
{
    if((eSelectedLight != Base3DLightNone)
        && (aLights.GetLightObject(eSelectedLight).IsEnabled()))
    {
        return TRUE;
    }
    return FALSE;
}

// Selektierte Lampe Position in Polarkoordinaten holen/setzen
// dabei geht Hor:[0..360.0[ und Ver:[-90..90] Grad
void SvxLightPrevievCtl3D::GetPosition(double& rHor, double& rVer)
{
    if(IsSelectionValid())
    {
        Vector3D aDirection = aLights.GetDirection(eSelectedLight);
        aDirection.Normalize();
        rHor = atan2(-aDirection.X(), -aDirection.Z()) + F_PI; // 0..2PI
        rVer = atan2(aDirection.Y(), aDirection.GetXZLength()); // -PI2..PI2
        rHor /= F_PI180; // 0..360.0
        rVer /= F_PI180; // -90.0..90.0
    }
    if(IsGeometrySelected())
    {
        rHor = fRotateY;
        rVer = fRotateX;
    }
}

void SvxLightPrevievCtl3D::SetPosition(double fHor, double fVer)
{
    if(IsSelectionValid())
    {
        Vector3D aDirection;
        fHor = (fHor * F_PI180) - F_PI; // -PI..PI
        fVer *= F_PI180; // -PI2..PI2
        aDirection.X() = cos(fVer) * -sin(fHor);
        aDirection.Y() = sin(fVer);
        aDirection.Z() = cos(fVer) * -cos(fHor);
        aDirection.Normalize();
        aLights.SetDirection(aDirection, eSelectedLight);
        Invalidate();
    }
    if(IsGeometrySelected())
    {
        SetRotation(fVer, fHor, fRotateZ);
    }
}

// Interaktion
void SvxLightPrevievCtl3D::MouseButtonDown( const MouseEvent& rMEvt )
{
    BOOL bCallParent = TRUE;

    // Status switchen
    if(rMEvt.IsLeft())
    {
        if(IsSelectionValid() || bGeometrySelected)
        {
            bMouseMoved = FALSE;
            bCallParent = FALSE;
            aActionStartPoint = rMEvt.GetPosPixel();
            StartTracking();
        }
        else
        {
            // Einfacher Click ohne viel Bewegen, versuche eine
            // Selektion
            TrySelection(rMEvt.GetPosPixel());
            bCallParent = FALSE;
        }
    }

    // call parent
    if(bCallParent)
        SvxPreviewCtl3D::MouseButtonDown(rMEvt);
}

void SvxLightPrevievCtl3D::Tracking( const TrackingEvent& rTEvt )
{
    if(rTEvt.IsTrackingEnded())
    {
        if(rTEvt.IsTrackingCanceled())
        {
            if(bMouseMoved)
            {
                // Interaktion abbrechen
                bMouseMoved = FALSE;
                if(bGeometrySelected)
                {
                    SetRotation(fSaveActionStartVer, fSaveActionStartHor, fSaveActionStartRotZ);
                }
                else
                {
                    SetPosition(fSaveActionStartHor, fSaveActionStartVer);
                }
                if(aChangeCallback.IsSet())
                    aChangeCallback.Call(this);
            }
        }
        else
        {
            const MouseEvent& rMEvt = rTEvt.GetMouseEvent();
            if(bMouseMoved)
            {
                // Wurde interaktiv veraendert
            }
            else
            {
                // Einfacher Click ohne viel Bewegen, versuche eine
                // Selektion
                TrySelection(rMEvt.GetPosPixel());
            }
        }
    }
    else
    {
        const MouseEvent& rMEvt = rTEvt.GetMouseEvent();
        Point aDeltaPos = rMEvt.GetPosPixel() - aActionStartPoint;

        if(!bMouseMoved)
        {
            if(INT32(aDeltaPos.X() * aDeltaPos.X() + aDeltaPos.Y() * aDeltaPos.Y())
                > nInteractionStartDistance)
            {
                if(bGeometrySelected)
                {
                    GetRotation(fSaveActionStartVer, fSaveActionStartHor, fSaveActionStartRotZ);
                }
                else
                {
                    // Start der Interaktion, Werte Sichern
                    GetPosition(fSaveActionStartHor, fSaveActionStartVer);
                }
                bMouseMoved = TRUE;
            }
        }

        if(bMouseMoved)
        {
            if(bGeometrySelected)
            {
                double fNewRotX = fSaveActionStartVer - ((double)aDeltaPos.Y());
                double fNewRotY = fSaveActionStartHor + ((double)aDeltaPos.X());

                // Horizontal abgleichen
                while(fNewRotY < 0.0)
                    fNewRotY += 360.0;
                while(fNewRotY >= 360.0)
                    fNewRotY -= 360.0;

                // Vertikal cutten
                if(fNewRotX < -90.0)
                    fNewRotX = -90.0;
                if(fNewRotX > 90.0)
                    fNewRotX = 90.0;

                SetRotation(fNewRotX, fNewRotY, fSaveActionStartRotZ);

                if(aChangeCallback.IsSet())
                    aChangeCallback.Call(this);
            }
            else
            {
                // Interaktion im vollen Gange
                double fNewPosHor = fSaveActionStartHor + ((double)aDeltaPos.X());
                double fNewPosVer = fSaveActionStartVer - ((double)aDeltaPos.Y());

                // Horizontal abgleichen
                while(fNewPosHor < 0.0)
                    fNewPosHor += 360.0;
                while(fNewPosHor >= 360.0)
                    fNewPosHor -= 360.0;

                // Vertikal cutten
                if(fNewPosVer < -90.0)
                    fNewPosVer = -90.0;
                if(fNewPosVer > 90.0)
                    fNewPosVer = 90.0;

                SetPosition(fNewPosHor, fNewPosVer);

                if(aChangeCallback.IsSet())
                    aChangeCallback.Call(this);
            }
        }
    }
}

// Selektion einer Lampe
void SvxLightPrevievCtl3D::TrySelection(Point aPosPixel)
{
    BOOL bNewSelection(FALSE);
    Base3DLightNumber eNew = Base3DLightNone;

    for(UINT16 a=0;a<BASE3D_MAX_NUMBER_LIGHTS;a++)
    {
        Base3DLightNumber eActualLight = (Base3DLightNumber)(Base3DLight0 + a);
        if(aLights.IsEnabled(eActualLight))
        {
            Vector3D aLightPos = aLights.GetDirection(eActualLight);
            aLightPos.Normalize();
            aLightPos *= GetObjectRadius() + GetDistanceToObject();
            Vector3D aScreenPos = aCameraSet.ObjectToViewCoor(aLightPos);
            Point aScreenPosPixel((long)(aScreenPos.X() + 0.5), (long)(aScreenPos.Y() + 0.5));
            aScreenPosPixel = LogicToPixel(aScreenPosPixel);
            aScreenPosPixel -= aPosPixel;
            INT32 nDistance = (aScreenPosPixel.X() * aScreenPosPixel.X())
                + (aScreenPosPixel.Y() * aScreenPosPixel.Y());
            if(nDistance < nInteractionStartDistance)
            {
                eNew = eActualLight;
                bNewSelection = TRUE;
            }
        }
    }

    if(bNewSelection && eSelectedLight != eNew)
    {
        // Auswaehlen
        SelectLight(eNew);

        // Falls tatsaechlich eine andere Lampe selektiert
        // wurde, rufe den entsprechenden Callback
        if(aSelectionChangeCallback.IsSet())
            aSelectionChangeCallback.Call(this);
    }
    else
    {
        // Punkt in logische Koordinaten umrechnen
        Point aPosLogic = PixelToLogic(aPosPixel);

        // Punkte generieren
        Vector3D aHitFront(aPosLogic.X(), aPosLogic.Y(), 0.0);
        Vector3D aHitBack(aPosLogic.X(), aPosLogic.Y(), ZBUFFER_DEPTH_RANGE);

        // Umrechnen
        aHitFront = aCameraSet.ViewToObjectCoor(aHitFront);
        aHitBack = aCameraSet.ViewToObjectCoor(aHitBack);

        // Eventuell die Geometrie des Beispielobjektes waehlen
        if(aGeometry.CheckHit(aHitFront, aHitBack, 0))
        {
            // Auswaehlen
            SelectGeometry();

            // Falls tatsaechlich eine andere Lampe selektiert
            // wurde, rufe den entsprechenden Callback
            if(aSelectionChangeCallback.IsSet())
                aSelectionChangeCallback.Call(this);
        }
    }
}

/*************************************************************************
|*
|*  3D Light Control Konstruktor
|*
\************************************************************************/

SvxLightCtl3D::SvxLightCtl3D( Window* pParent, const ResId& rResId)
:   Control(pParent, rResId),
    aLightControl(this, 0),
    aHorScroller(this, WB_HORZ | WB_DRAG),
    aVerScroller(this, WB_VERT | WB_DRAG),
    aSwitcher(this, 0)
{
    // Members initialisieren
    Init();
}

SvxLightCtl3D::SvxLightCtl3D( Window* pParent, WinBits nStyle )
:   Control(pParent, nStyle),
    aLightControl(this, 0),
    aHorScroller(this, WB_HORZ | WB_DRAG),
    aVerScroller(this, WB_VERT | WB_DRAG),
    aSwitcher(this, 0)
{
    // Members initialisieren
    Init();
}

void SvxLightCtl3D::Init()
{
    // Lokale Parameter setzen
    bSphereUsed = TRUE;
    bVectorValid = FALSE;

    // Light preview
    aLightControl.Show();
    aLightControl.SetChangeCallback( LINK(this, SvxLightCtl3D, InternalInteractiveChange) );
    aLightControl.SetSelectionChangeCallback( LINK(this, SvxLightCtl3D, InternalSelectionChange) );

    // Horiz Scrollbar
    aHorScroller.Show();
    aHorScroller.SetRange(Range(0, 36000));
    aHorScroller.SetLineSize(100);
    aHorScroller.SetPageSize(1000);
    aHorScroller.SetScrollHdl( LINK(this, SvxLightCtl3D, ScrollBarMove) );

    // Vert Scrollbar
    aVerScroller.Show();
    aVerScroller.SetRange(Range(0, 18000));
    aVerScroller.SetLineSize(100);
    aVerScroller.SetPageSize(1000);
    aVerScroller.SetScrollHdl( LINK(this, SvxLightCtl3D, ScrollBarMove) );

    // Switch Button
    aSwitcher.Show();
    aSwitcher.SetClickHdl( LINK(this, SvxLightCtl3D, ButtonPress) );

    // Selektion klaeren
    CheckSelection();

    // Neues Layout
    NewLayout();
}

SvxLightCtl3D::~SvxLightCtl3D()
{
}

void SvxLightCtl3D::SetVector(const Vector3D& rNew)
{
    aVector = rNew;
    aVector.Normalize();
    bVectorValid = TRUE;
}

const Vector3D& SvxLightCtl3D::GetVector()
{
    // Grobe Anbindung an altes Verhalten, um eine Reaktion zu haben
    aVector = aLightControl.GetLightGroup()->GetDirection(aLightControl.GetSelectedLight());
    aVector.Normalize();
    return aVector;
}

void SvxLightCtl3D::Resize()
{
    // call parent
    Control::Resize();

    // Neues Layout
    NewLayout();
}

void SvxLightCtl3D::NewLayout()
{
    // Layout members
    Size aSize = GetOutputSizePixel();
    long nScrollSize = aHorScroller.GetSizePixel().Height();

    // Preview Fenster
    Point aPoint(0, 0);
    Size aDestSize(aSize.Width() - nScrollSize, aSize.Height() - nScrollSize);
    aLightControl.SetPosSizePixel(aPoint, aDestSize);

    // Horizontaler Scrollbar
    aPoint.Y() = aSize.Height() - nScrollSize;
    aDestSize.Height() = nScrollSize;
    aHorScroller.SetPosSizePixel(aPoint, aDestSize);

    // Vertikaler Scrollbar
    aPoint.X() = aSize.Width() - nScrollSize;
    aPoint.Y() = 0;
    aDestSize.Width() = nScrollSize;
    aDestSize.Height() = aSize.Height() - nScrollSize;
    aVerScroller.SetPosSizePixel(aPoint, aDestSize);

    // Button
    aPoint.Y() = aSize.Height() - nScrollSize;
    aDestSize.Height() = nScrollSize;
    aSwitcher.SetPosSizePixel(aPoint, aDestSize);
}

// Selektion auf Gueltigkeit pruefen
void SvxLightCtl3D::CheckSelection()
{
    BOOL bSelectionValid = (aLightControl.IsSelectionValid()
        || aLightControl.IsGeometrySelected());
    aHorScroller.Enable(bSelectionValid);
    aVerScroller.Enable(bSelectionValid);

    if(bSelectionValid)
    {
        double fHor, fVer;
        aLightControl.GetPosition(fHor, fVer);
        aHorScroller.SetThumbPos( INT32(fHor * 100.0) );
        aVerScroller.SetThumbPos( 18000 - INT32((fVer + 90.0) * 100.0) );
    }
}

IMPL_LINK( SvxLightCtl3D, ScrollBarMove, void*, pNil)
{
    INT32 nHor = aHorScroller.GetThumbPos();
    INT32 nVer = aVerScroller.GetThumbPos();

    aLightControl.SetPosition(
        ((double)nHor) / 100.0,
        ((double)((18000 - nVer) - 9000)) / 100.0);

    if(aUserInteractiveChangeCallback.IsSet())
        aUserInteractiveChangeCallback.Call(this);

    // ...um Kompatibel zu bleiben, kann spaeter wieder raus
    //InteractiveChange(NULL);

    return NULL;
}

IMPL_LINK( SvxLightCtl3D, ButtonPress, void*, pNil)
{
    aLightControl.SetGeometry(bSphereUsed);
    bSphereUsed = !bSphereUsed;
    return NULL;
}

IMPL_LINK( SvxLightCtl3D, InternalInteractiveChange, void*, pNil)
{
    double fHor, fVer;

    aLightControl.GetPosition(fHor, fVer);
    aHorScroller.SetThumbPos( INT32(fHor * 100.0) );
    aVerScroller.SetThumbPos( 18000 - INT32((fVer + 90.0) * 100.0) );

    if(aUserInteractiveChangeCallback.IsSet())
        aUserInteractiveChangeCallback.Call(this);

    // ...um Kompatibel zu bleiben, kann spaeter wieder raus
    //InteractiveChange(NULL);

    return NULL;
}

IMPL_LINK( SvxLightCtl3D, InternalSelectionChange, void*, pNil)
{
    CheckSelection();

    if(aUserSelectionChangeCallback.IsSet())
        aUserSelectionChangeCallback.Call(this);

    // ...um Kompatibel zu bleiben, kann spaeter wieder raus
    //SelectionChange(NULL);

    return NULL;
}

// ...um Kompatibel zu bleiben, kann spaeter wieder raus
/*
IMPL_LINK( SvxLightCtl3D, InteractiveChange, void*, pNil)
{
    return NULL;
} */

/*
IMPL_LINK( SvxLightCtl3D, SelectionChange, void*, pNil)
{
    return NULL;
}*/



/*************************************************************************
 *
 *  $RCSfile: msashape3d.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:03:40 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _MSASHAPE3D_HXX
#include <msashape3d.hxx>
#endif

#ifndef _SV_POLY_HXX
#include <vcl/poly.hxx>
#endif
#ifndef _SVDITER_HXX
#include <svditer.hxx>
#endif
#ifndef _SVDOBJ_HXX
#include <svdobj.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef SVX_XFILLIT0_HXX
#include <xfillit0.hxx>
#endif
#ifndef _SVX_XIT_HXX
#include <xit.hxx>
#endif
#ifndef _SVX_XBTMPIT_HXX
#include <xbtmpit.hxx>
#endif
#ifndef _SVX_XFLCLIT_HXX
#include <xflclit.hxx>
#endif
#ifndef _SVDOPATH_HXX
#include <svdopath.hxx>
#endif
#ifndef _SVDOGRP_HXX
#include <svdogrp.hxx>
#endif
#ifndef _SVDPAGE_HXX
#include <svdpage.hxx>
#endif
#ifndef _E3D_POLYSC3D_HXX
#include "polysc3d.hxx"
#endif
#ifndef _SVDDEF_HXX
#include "svddef.hxx"
#endif
#ifndef _SVX3DITEMS_HXX
#include "svx3ditems.hxx"
#endif
#ifndef _E3D_EXTRUD3D_HXX
#include "extrud3d.hxx"
#endif
#ifndef _SVX_XFLBMTIT_HXX
#include <xflbmtit.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SVX_XLNCLIT_HXX
#include <xlnclit.hxx>
#endif

#define ITEMVALUE(ItemSet,Id,Cast)  ((const Cast&)(ItemSet).Get(Id)).GetValue()

double SvxMSDffAutoShape3D::Fix16ToAngle( sal_Int32 nFixAngle )
{
    double fAngle = 0.0;
    if ( nFixAngle )
    {
        fAngle = nFixAngle;
        fAngle /= 65536.0;
    }
    return fAngle;
}

SvxMSDffAutoShape3D::Transformation2D::Transformation2D( const DffPropSet& rPropSet, const Rectangle& rSnapRect ) :
    aCenter( rSnapRect.Center() )
{
    bParallel = ( rPropSet.GetPropertyValue( DFF_Prop_fc3DFillHarsh, 0 ) & 4 ) != 0;
    if ( bParallel )
    {
        fSkewAngle = Fix16ToAngle( rPropSet.GetPropertyValue( DFF_Prop_c3DSkewAngle, -135 * 65536 ) ) * F_PI180;
        nSkewAmount = rPropSet.GetPropertyValue( DFF_Prop_c3DSkewAmount, 50 );
    }
    else
    {
        fZScreen = 0.0;
        fViewPointOriginX = ((double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DOriginX, 32768 )) * rSnapRect.GetWidth()) / 65536.0;
        fViewPointOriginY = ((double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DOriginY,-32768 )) * rSnapRect.GetHeight()) / 65536.0;
        fViewPoint.X() = (double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DXViewpoint, 1250000 )) / 360;   // 360 emu = 0,01 mm
        fViewPoint.Y() = (double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DYViewpoint,-1250000 )) / 360;
        fViewPoint.W() = (double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DZViewpoint,-9000000 )) / 360;
    }
}

void SvxMSDffAutoShape3D::Transformation2D::ApplySkewSettings( Polygon3D& rPoly3D )
{
    sal_uInt16 j;
    for ( j = 0; j < rPoly3D.GetPointCount(); j++ )
    {
        Vector3D& rPoint = rPoly3D[ j ];
        double fDepth = -( rPoint.Z() * nSkewAmount ) / 100.0;
        rPoint.X() += fDepth * cos( fSkewAngle );
        rPoint.Y() += -( fDepth * sin( fSkewAngle ) );
    }
}

Point SvxMSDffAutoShape3D::Transformation2D::Transform2D( const Vector3D& rPoint3D )
{
    Point aPoint2D;
    if ( bParallel )
    {
        aPoint2D.X() = (sal_Int32)rPoint3D.X();
        aPoint2D.Y() = (sal_Int32)rPoint3D.Y();
    }
    else
    {
        double fX = rPoint3D.X() - fViewPointOriginX;
        double fY = rPoint3D.Y() - fViewPointOriginY;
        double f = ( fZScreen - fViewPoint.W() ) / ( rPoint3D.Z() - fViewPoint.W() );
        aPoint2D.X() = (sal_Int32)(( fX - fViewPoint.X() ) * f + fViewPoint.X() + fViewPointOriginX );
        aPoint2D.Y() = (sal_Int32)(( fY - fViewPoint.Y() ) * f + fViewPoint.Y() + fViewPointOriginY );
    }
    aPoint2D.Move( aCenter.X(), aCenter.Y() );
    return aPoint2D;
}

void SvxMSDffAutoShape3D::Rotate( Vector3D& rPoint, const double x, const double y, const double z )
{
    // rotation z axis
     Vector3D aPoint( rPoint );
    rPoint.X() = aPoint.X() * cos( z ) + aPoint.Y() * sin( z );
    rPoint.Y() = -( aPoint.X() * sin( z ) ) + aPoint.Y() * cos( z );

    // rotation y axis
    aPoint = rPoint;
    rPoint.X() = aPoint.X() * cos( y ) + aPoint.Z() * sin( y );
    rPoint.Z() = -( aPoint.X() * sin( y ) ) + aPoint.Z() * cos( y );

    // rotation x axis
    aPoint = rPoint;
    rPoint.Y() = aPoint.Y() * cos( x ) + aPoint.Z() * sin( x );
    rPoint.Z() = -( aPoint.Y() * sin( x ) ) + aPoint.Z() * cos( x );
}

void SvxMSDffAutoShape3D::Rotate( PolyPolygon3D& rPolyPoly3D, const Point3D& rOrigin, const double x, const double y, const double z )
{
    sal_uInt16 i, j;
    for( i = 0; i < rPolyPoly3D.Count(); i++ )
    {
        Polygon3D& rPoly3D = rPolyPoly3D[ i ];
        for( j = 0; j < rPoly3D.GetPointCount(); j++ )
        {
            Vector3D& rPoint = rPoly3D[ j ];
            rPoint.X() -= rOrigin.X();
            rPoint.Y() -= rOrigin.Y();
            rPoint.Z() -= rOrigin.W();
            Rotate( rPoint, x, y, z );
            rPoint.X() += rOrigin.X();
            rPoint.Y() += rOrigin.Y();
            rPoint.Z() += rOrigin.W();
        }
    }
}

SdrObject* SvxMSDffAutoShape3D::Create3DObject( const SdrObject* pObj, const DffPropSet& rPropSet,
                SfxItemSet& aSet, Rectangle& rSnapRect, sal_uInt32 nSpFlags )
{
    SdrObject* pRet = NULL;
    std::vector< E3dCompoundObject* > aPlaceholderObjectList;

    double fExtrusionBackward = (double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DExtrudeBackward, 457200 )) / 360.0;  // in emus
    double fExtrusionForward  = (double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DExtrudeForward, 0 )) / 360.0;
    double fDepth = fExtrusionBackward - fExtrusionForward;
    if ( fDepth < 1.0 )
        fDepth = 1.0;
    ProjectionType eProjectionType( rPropSet.GetPropertyValue( DFF_Prop_fc3DFillHarsh, 0 ) & 4 ? PR_PARALLEL : PR_PERSPECTIVE );

    // pObj Umwandeln in Szene mit 3D Objekt
    E3dDefaultAttributes a3DDefaultAttr;
    a3DDefaultAttr.SetDefaultLatheCharacterMode( TRUE );
    a3DDefaultAttr.SetDefaultExtrudeCharacterMode( TRUE );

    E3dScene* pScene = new E3dPolyScene( a3DDefaultAttr );

    sal_Bool bSceneHasObjects ( sal_False );
    sal_Bool bUseTwoFillStyles( sal_False );

    MSO_3DRenderMode eRenderMode( (MSO_3DRenderMode)rPropSet.GetPropertyValue( DFF_Prop_c3DRenderMode, mso_FullRender ) );
    XFillStyle eFillStyle( ITEMVALUE( aSet, XATTR_FILLSTYLE, XFillStyleItem ) );
    pScene->SetItem( Svx3DShadeModeItem( 0 ) );
    aSet.Put( Svx3DPercentDiagonalItem( 0 ) );
    aSet.Put( Svx3DTextureModeItem( 1 ) );
    if ( eRenderMode == mso_Wireframe )
    {
        aSet.Put( XLineStyleItem( XLINE_SOLID ) );
        aSet.Put( XFillStyleItem ( XFILL_NONE ) );
        aSet.Put( Svx3DDoubleSidedItem( TRUE ) );
    }
    else
    {
        aSet.Put( XLineStyleItem( XLINE_NONE ) );
        if ( eFillStyle == XFILL_NONE )
            aSet.Put( XFillStyleItem( XFILL_SOLID ) );
        else if ( ( eFillStyle == XFILL_BITMAP ) || ( eFillStyle == XFILL_GRADIENT ) )
            bUseTwoFillStyles = sal_True;
    }
    const SdrObject* pNext = NULL;
    SdrObjListIter* pIter = NULL;
    if ( pObj->IsGroupObject() )
    {
        pIter = new SdrObjListIter( *pObj, IM_DEEPWITHGROUPS );
        if ( pIter->IsMore() )
            pNext = pIter->Next();
    }
    else
        pNext = pObj;
    while ( pNext )
    {
        sal_Bool bIsPlaceholderObject = (((XFillStyleItem&)pNext->GetItem( XATTR_FILLSTYLE )).GetValue() == XFILL_NONE )
                                     && (((XLineStyleItem&)pNext->GetItem( XATTR_LINESTYLE )).GetValue() == XLINE_NONE );

        SdrObject* pNewObj = pNext->ConvertToPolyObj( FALSE, FALSE );
        SdrPathObj* pPath = PTR_CAST( SdrPathObj, pNewObj );
        if( pPath )
        {
            const XPolyPolygon& rPolyPolygon = pPath->GetPathPoly();
            E3dCompoundObject* p3DObj = new E3dExtrudeObj( a3DDefaultAttr,
                rPolyPolygon, bUseTwoFillStyles ? 1 : fDepth );
            p3DObj->NbcSetLayer( pObj->GetLayer() );
            p3DObj->SetItemSet( aSet );
            if ( bIsPlaceholderObject )
                aPlaceholderObjectList.push_back( p3DObj );
            else if ( bUseTwoFillStyles )
            {
                Bitmap aFillBmp;
                sal_Bool bFillBmpTile = ((XFillBmpTileItem&)p3DObj->GetItem( XATTR_FILLBMP_TILE )).GetValue();
                if ( bFillBmpTile )
                {
                    const XFillBitmapItem& rBmpItm = (XFillBitmapItem&)p3DObj->GetItem( XATTR_FILLBITMAP );
                    const XOBitmap& rXOBmp = rBmpItm.GetValue();
                    aFillBmp = rXOBmp.GetBitmap();
                    Size aLogicalSize = aFillBmp.GetPrefSize();
                    if ( aFillBmp.GetPrefMapMode() == MAP_PIXEL )
                        aLogicalSize = Application::GetDefaultDevice()->PixelToLogic( aLogicalSize, MAP_100TH_MM );
                    else
                        aLogicalSize = OutputDevice::LogicToLogic( aLogicalSize, aFillBmp.GetPrefMapMode(), MAP_100TH_MM );
                    aLogicalSize.Width()  *= 5;         ;//             :-(     nice scaling, look at engine3d/obj3d.cxx
                    aLogicalSize.Height() *= 5;
                    aFillBmp.SetPrefSize( aLogicalSize );
                    aFillBmp.SetPrefMapMode( MAP_100TH_MM );
                    p3DObj->SetItem( XFillBitmapItem( String(), aFillBmp ) );
                }
                else
                {
                    Rectangle aBoundRect( rPolyPolygon.GetBoundRect() );
                    if ( rSnapRect != aBoundRect )
                    {
                        const XFillBitmapItem& rBmpItm = (XFillBitmapItem&)p3DObj->GetItem( XATTR_FILLBITMAP );
                        const XOBitmap& rXOBmp = rBmpItm.GetValue();
                        aFillBmp = rXOBmp.GetBitmap();
                        Size aBmpSize( aFillBmp.GetSizePixel() );
                        double fXScale = (double)aBoundRect.GetWidth() / (double)rSnapRect.GetWidth();
                        double fYScale = (double)aBoundRect.GetHeight() / (double)rSnapRect.GetHeight();

                        Point aPt( (sal_Int32)( (double)( aBoundRect.Left() - rSnapRect.Left() )* (double)aBmpSize.Width() / (double)rSnapRect.GetWidth() ),
                                            (sal_Int32)( (double)( aBoundRect.Top() - rSnapRect.Top() ) * (double)aBmpSize.Height() / (double)rSnapRect.GetHeight() ) );
                        Size aSize( (sal_Int32)( aBmpSize.Width() * fXScale ),
                                                (sal_Int32)( aBmpSize.Height() * fYScale ) );
                        Rectangle aCropRect( aPt, aSize );
                         aFillBmp.Crop( aCropRect );
                        p3DObj->SetItem( XFillBitmapItem( String(), aFillBmp ) );
                    }
                }
                pScene->Insert3DObj( p3DObj );
                p3DObj = new E3dExtrudeObj( a3DDefaultAttr, pPath->GetPathPoly(), fDepth );
                p3DObj->NbcSetLayer( pObj->GetLayer() );
                p3DObj->SetItemSet( aSet );
                p3DObj->SetItem( XFillStyleItem( XFILL_SOLID ) );
                p3DObj->SetItem( Svx3DCloseFrontItem( sal_False ) );
                p3DObj->SetItem( Svx3DCloseBackItem( sal_False ) );
                pScene->Insert3DObj( p3DObj );
                p3DObj = new E3dExtrudeObj( a3DDefaultAttr, pPath->GetPathPoly(), 1 );
                p3DObj->NbcSetLayer( pObj->GetLayer() );
                p3DObj->SetItemSet( aSet );
                Matrix4D aFrontTransform( p3DObj->GetTransform() );
                aFrontTransform.Translate( 0, 0, fDepth );
                p3DObj->NbcSetTransform( aFrontTransform );
                if ( ( eFillStyle == XFILL_BITMAP ) && !aFillBmp.IsEmpty() )
                    p3DObj->SetItem( XFillBitmapItem( String(), aFillBmp ) );
            }
            else if ( eFillStyle == XFILL_NONE )
            {
                XLineColorItem& rLineColor = (XLineColorItem&)p3DObj->GetItem( XATTR_LINECOLOR );
                p3DObj->SetItem( XFillColorItem( String(), rLineColor.GetValue() ) );
                p3DObj->SetItem( Svx3DDoubleSidedItem( sal_True ) );
                p3DObj->SetItem( Svx3DCloseFrontItem( sal_False ) );
                p3DObj->SetItem( Svx3DCloseBackItem( sal_False ) );
            }
            pScene->Insert3DObj( p3DObj );
            bSceneHasObjects = sal_True;
        }
        delete pNewObj;
        pNext = NULL;
        if ( pIter && pIter->IsMore() )
            pNext = pIter->Next();
    }
    delete pIter;

    if ( bSceneHasObjects ) // is the SdrObject properly converted
    {
        // then we can change the return value
        pRet = pScene;

        // Kameraeinstellungen, Perspektive ...
        Camera3D& rCamera = (Camera3D&)pScene->GetCamera();
        const Volume3D& rVolume = pScene->GetBoundVolume();
        pScene->CorrectSceneDimensions();
        pScene->NbcSetSnapRect( rSnapRect );

        // InitScene replacement
        double fW = rVolume.GetWidth();
        double fH = rVolume.GetHeight();
        double fCamZ = rVolume.MaxVec().Z() + ( ( fW + fH ) / 2.0 );

        rCamera.SetAutoAdjustProjection( FALSE );
        rCamera.SetViewWindow( -fW / 2, - fH / 2, fW, fH);
        Vector3D aLookAt( 0.0, 0.0, 0.0 );
        Vector3D aCamPos( 0.0, 0.0, 100.0 );

        rCamera.SetDefaults( Vector3D( 0.0, 0.0, 100.0 ), aLookAt, 100.0 );
        rCamera.SetPosAndLookAt( aCamPos, aLookAt );
        rCamera.SetFocalLength( 1.0 );
        rCamera.SetProjection( eProjectionType );
        pScene->SetCamera( rCamera );
        pScene->SetRectsDirty();

        double fViewPointOriginX = ((double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DOriginX, 32768 )) * rSnapRect.GetWidth()) / 65536.0;
        double fViewPointOriginY = ((double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DOriginY,-32768 )) * rSnapRect.GetHeight()) / 65536.0;

        Matrix4D aNewTransform( pScene->GetTransform() );
        Point aCenter( rSnapRect.Center() );
        aNewTransform.Translate( -aCenter.X(), aCenter.Y(), -pScene->GetBoundVolume().GetDepth() );
        double fXRotate = Fix16ToAngle( rPropSet.GetPropertyValue( DFF_Prop_c3DXRotationAngle, 0 ) );
        double fYRotate = Fix16ToAngle( rPropSet.GetPropertyValue( DFF_Prop_c3DYRotationAngle, 0 ) );
        double fZRotate = Fix16ToAngle( rPropSet.GetPropertyValue( DFF_Prop_Rotation, 0 ) );
        if ( fZRotate != 0.0 )
            aNewTransform.RotateZ( -fZRotate * F_PI180 );
        if ( nSpFlags & SP_FFLIPH )
            aNewTransform.ScaleX( -1 );
        if ( nSpFlags & SP_FFLIPV )
            aNewTransform.ScaleY( -1 );
        if( fYRotate != 0.0 )
            aNewTransform.RotateY( -fYRotate * F_PI180 );
        if( fXRotate != 0.0 )
            aNewTransform.RotateX( -fXRotate * F_PI180 );
        if ( eProjectionType == PR_PARALLEL )
        {
            sal_Int32 nSkewAmount = rPropSet.GetPropertyValue( DFF_Prop_c3DSkewAmount, 50 );
            if ( nSkewAmount )
            {
                sal_Int32 nSkewAngle = ((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DSkewAngle,  -135 * 65536 ) ) >> 16;

                double fAlpha( (double)nSkewAngle * F_PI180 );
                double fInvTanBeta( (double)nSkewAmount / 100.0 );
                if(fInvTanBeta)
                {
                    aNewTransform.ShearXY(
                        fInvTanBeta * cos(fAlpha),
                        fInvTanBeta * sin(fAlpha));
                }
            }
        }
        else
        {
            aNewTransform.Translate( -fViewPointOriginX, fViewPointOriginY, 0 );
            // now set correct camera position
            double fViewPointOriginX = ((double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DOriginX, 32768 )) * rSnapRect.GetWidth()) / 65536.0;
            double fViewPointOriginY = ((double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DOriginY,-32768 )) * rSnapRect.GetHeight()) / 65536.0;
            double fViewPointX = (double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DXViewpoint, 1250000 )) / 360;
            double fViewPointY = (double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DYViewpoint,-1250000 )) / 360;
            double fViewPointZ = (double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DZViewpoint,-9000000 )) / 360;

            Vector3D aLookAt( fViewPointX, -fViewPointY, 0 );
            Vector3D aNewCamPos( fViewPointX, -fViewPointY, -fViewPointZ );
            rCamera.SetPosAndLookAt( aNewCamPos, aLookAt );
            pScene->SetCamera( rCamera );
        }
        pScene->NbcSetTransform( aNewTransform );
        pScene->FitSnapRectToBoundVol();

        ///////////
        // light //
        ///////////
        double fAmbientIntensity = ((double)rPropSet.GetPropertyValue( DFF_Prop_c3DAmbientIntensity, 22178 )) / 65536.0;

        sal_Int32 nLightX = ((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DKeyX, 50000 )) / 360;
        sal_Int32 nLightY = - ((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DKeyY, 0 )) / 360;
        sal_Int32 nLightZ = - ((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DKeyZ, 10000 )) / 360;
        if ( !nLightZ )
            nLightZ = 1;
        double fLightIntensity = ((double)rPropSet.GetPropertyValue( DFF_Prop_c3DKeyIntensity, 43712 )) / 65536.0;
        sal_Bool bLightHarsh = ( rPropSet.GetPropertyValue( DFF_Prop_fc3DFillHarsh, 0 ) & 2 ) != 0;

        sal_Int32 nLight2X = ((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DFillX, -50000 )) / 360;
        sal_Int32 nLight2Y = - ((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DFillY, 0 )) / 360;
        sal_Int32 nLight2Z = - ((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DFillZ, 10000 )) / 360;
        if ( !nLight2Z )
            nLight2Z = -1;
        double fLight2Intensity = ((double)rPropSet.GetPropertyValue( DFF_Prop_c3DFillIntensity, 43712 )) / 65536.0;
        sal_Bool bLight2Harsh = ( rPropSet.GetPropertyValue( DFF_Prop_fc3DFillHarsh, 0 ) & 1 ) != 0;
        sal_Bool bLightFace = ( rPropSet.GetPropertyValue( DFF_Prop_fc3DLightFace, 0 ) & 1 ) != 0;

        sal_uInt16 nAmbientColor = (sal_uInt16)( fAmbientIntensity * 255.0 );
        if ( nAmbientColor > 255 )
            nAmbientColor = 255;
        Color aGlobalAmbientColor( (sal_uInt8)nAmbientColor, (sal_uInt8)nAmbientColor, (sal_uInt8)nAmbientColor );
        pScene->SetItem( Svx3DAmbientcolorItem( aGlobalAmbientColor ) );

        sal_uInt8 nSpotLight1 = (sal_uInt8)( fLightIntensity * 255.0 );
        Vector3D aSpotLight1( (double)nLightX, (double)nLightY, (double)nLightZ );
        aSpotLight1.Normalize();
        pScene->SetItem( Svx3DLightOnOff1Item( sal_True ) );
        Color aAmbientSpot1Color( nSpotLight1, nSpotLight1, nSpotLight1 );
        pScene->SetItem( Svx3DLightcolor1Item( aAmbientSpot1Color ) );
        pScene->SetItem( Svx3DLightDirection1Item( aSpotLight1 ) );

        sal_uInt8 nSpotLight2 = (sal_uInt8)( fLight2Intensity * 255.0 );
        Vector3D aSpotLight2( (double)nLight2X, (double)nLight2Y, (double)nLight2Z );
        aSpotLight2.Normalize();
        pScene->SetItem( Svx3DLightOnOff2Item( sal_True ) );
        Color aAmbientSpot2Color( nSpotLight2, nSpotLight2, nSpotLight2 );
        pScene->SetItem( Svx3DLightcolor2Item( aAmbientSpot2Color ) );
        pScene->SetItem( Svx3DLightDirection2Item( aSpotLight2 ) );

        if ( nLightX || nLightY )
        {
            sal_uInt8 nSpotLight3 = 70;
            Vector3D aSpotLight3( 0, 0, 1 );
            pScene->SetItem( Svx3DLightOnOff3Item( sal_True ) );
            Color aAmbientSpot3Color( nSpotLight3, nSpotLight3, nSpotLight3 );
            pScene->SetItem( Svx3DLightcolor3Item( aAmbientSpot3Color ) );
            pScene->SetItem( Svx3DLightDirection3Item( aSpotLight3 ) );
        }

        double fSpecular = ((double)rPropSet.GetPropertyValue( DFF_Prop_c3DSpecularAmt, 0 )) / 65536.0;
        double fDiffuse = ((double)rPropSet.GetPropertyValue( DFF_Prop_c3DDiffuseAmt, 0 )) / 65536.0;
        double fShininess = ((double)rPropSet.GetPropertyValue( DFF_Prop_c3DShininess, 0 )) / 65536.0;

        sal_uInt16 nItensity = 25;
        Color aSpecularCol( COL_BLACK );
        if ( fSpecular == 0.0 )
        {
            aSpecularCol = Color( 51, 51, 51 );
        }
        else if ( fSpecular > 1.2 )
        {
            aSpecularCol = Color( 128, 128, 128 );
        }
        pScene->SetItem( Svx3DMaterialSpecularItem( aSpecularCol ) );
        pScene->SetItem( Svx3DMaterialSpecularIntensityItem( nItensity ) );

        pScene->SetModel( pObj->GetModel() );
        pScene->InitTransformationSet();
        pRet->SetSnapRect( CalculateNewSnapRect( rSnapRect, rPropSet ) );

        // removing placeholder objects
        std::vector< E3dCompoundObject* >::iterator aObjectListIter( aPlaceholderObjectList.begin() );
        while ( aObjectListIter != aPlaceholderObjectList.end() )
        {
            pScene->Remove3DObj( *aObjectListIter );
            delete *aObjectListIter++;
        }
    }
    else
        delete pScene;
    return pRet;
}

Rectangle SvxMSDffAutoShape3D::CalculateNewSnapRect( const Rectangle& rOriginalSnapRect, const DffPropSet& rPropSet )
{
    const Point aCenter( rOriginalSnapRect.Center() );

    double fExtrusionBackward = (double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DExtrudeBackward, 457200 )) / 360.0;
    double fExtrusionForward  = (double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DExtrudeForward, 0 )) / 360.0;

    sal_uInt16 i;

    // creating initial bound volume ( without rotation. skewing.and camera )
    Polygon3D aBoundVolume( 8 );
    const Polygon aPolygon( rOriginalSnapRect );
    for ( i = 0; i < 4; i++ )
    {
        aBoundVolume[ i ].X() = aPolygon[ i ].X() - aCenter.X();
        aBoundVolume[ i ].Y() = aPolygon[ i ].Y() - aCenter.Y();
        aBoundVolume[ i ].Z()  = fExtrusionForward;
        aBoundVolume[ i + 4 ].X()  = aPolygon[ i ].X() - aCenter.X();
        aBoundVolume[ i + 4 ].Y()  = aPolygon[ i ].Y() - aCenter.Y();
        aBoundVolume[ i + 4 ].Z()  = fExtrusionBackward;
    }

    Point3D aRotateCenter;
    aRotateCenter.X() = 0.0;
    aRotateCenter.Y() = 0.0;
    aRotateCenter.W() = rPropSet.GetPropertyValue( DFF_Prop_c3DRotationCenterZ, 0 ) / 360;

    // double XCenterInGUnits = rPropSet.GetPropertyValue( DFF_Prop_c3DRotationCenterX, 0 );
    // double YCenterInGUnits = rPropSet.GetPropertyValue( DFF_Prop_c3DRotationCenterY, 0 );

    // sal_Int32 nRotationXAxisInProz = rPropSet.GetPropertyValue( DFF_Prop_c3DRotationAxisX, 100 );
    // sal_Int32 nRotationYAxisInProz = rPropSet.GetPropertyValue( DFF_Prop_c3DRotationAxisY, 0 );
    // sal_Int32 nRotationZAxisInProz = rPropSet.GetPropertyValue( DFF_Prop_c3DRotationAxisZ, 0 );

    double fXRotate = Fix16ToAngle( rPropSet.GetPropertyValue( DFF_Prop_c3DXRotationAngle, 0 ) ) * F_PI180;
    double fYRotate = Fix16ToAngle( rPropSet.GetPropertyValue( DFF_Prop_c3DYRotationAngle, 0 ) ) * F_PI180;
    double fZRotate = -Fix16ToAngle( rPropSet.GetPropertyValue( DFF_Prop_Rotation, 0 ) ) * F_PI180;
    // double fRotationAngle = Fix16ToAngle( rPropSet.GetPropertyValue( DFF_Prop_c3DRotationAngle, 0 ) );       // * F_PI180;

    for( i = 0; i < 8; i++ )        // rotating bound volume
    {
        Vector3D& rPoint = aBoundVolume[ i ];
        rPoint.X() -= aRotateCenter.X();
        rPoint.Y() -= aRotateCenter.Y();
        rPoint.Z() -= aRotateCenter.W();
        Rotate( rPoint, fXRotate, fYRotate, fZRotate );
        rPoint.X() += aRotateCenter.X();
        rPoint.Y() += aRotateCenter.Y();
        rPoint.Z() += aRotateCenter.W();
    }
    Transformation2D aTransformation2D( rPropSet, rOriginalSnapRect );
    if ( aTransformation2D.IsParallel() )
        aTransformation2D.ApplySkewSettings( aBoundVolume );

    Polygon aTransformed( 8 );
    for ( i = 0; i < 8; i++ )
        aTransformed[ i ] = aTransformation2D.Transform2D( aBoundVolume[ i ] );
    return aTransformed.GetBoundRect();
}

/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: msashape3d.cxx,v $
 * $Revision: 1.13.226.1 $
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
#include <msashape3d.hxx>
#include <tools/poly.hxx>
#include <svditer.hxx>
#include <svx/svdobj.hxx>
#include <svtools/poolitem.hxx>
#include <svtools/itemset.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xflclit.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdpage.hxx>
#include <svx/polysc3d.hxx>
#include <svx/svddef.hxx>
#include <svx/svx3ditems.hxx>
#include <svx/extrud3d.hxx>
#include <svx/xflbmtit.hxx>
#include <vcl/svapp.hxx>
#include <svx/xlnclit.hxx>

#define ITEMVALUE(ItemSet,Id,Cast)  ((const Cast&)(ItemSet).Get(Id)).GetValue()

double SvxMSDffCustomShape3D::Fix16ToAngle( sal_Int32 nFixAngle )
{
    double fAngle = 0.0;
    if ( nFixAngle )
    {
        fAngle = nFixAngle;
        fAngle /= 65536.0;
    }
    return fAngle;
}

SvxMSDffCustomShape3D::Transformation2D::Transformation2D( const DffPropSet& rPropSet, const Rectangle& rSnapRect ) :
    aCenter( rSnapRect.Center() )
{
    bParallel = ( rPropSet.GetPropertyValue( DFF_Prop_fc3DFillHarsh, 0 ) & 4 ) != 0;
    if ( bParallel )
    {
        fSkewAngle = Fix16ToAngle( rPropSet.GetPropertyValue( DFF_Prop_c3DSkewAngle, sal::static_int_cast< UINT32 >( -135 * 65536 ) ) ) * F_PI180;
        nSkewAmount = rPropSet.GetPropertyValue( DFF_Prop_c3DSkewAmount, 50 );
    }
    else
    {
        fZScreen = 0.0;
        fViewPointOriginX = ((double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DOriginX, 32768 )) * rSnapRect.GetWidth()) / 65536.0;
        fViewPointOriginY = ((double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DOriginY, (sal_uInt32)-32768 )) * rSnapRect.GetHeight()) / 65536.0;
        fViewPoint.setX((double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DXViewpoint, 1250000 )) / 360);   // 360 emu = 0,01 mm
        fViewPoint.setY((double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DYViewpoint, (sal_uInt32)-1250000 )) / 360);
        fViewPoint.setZ((double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DZViewpoint, (sal_uInt32)-9000000 )) / 360);
    }
}

basegfx::B3DPolygon SvxMSDffCustomShape3D::Transformation2D::ApplySkewSettings( const basegfx::B3DPolygon& rPoly3D )
{
    basegfx::B3DPolygon aRetval;
    sal_uInt32 j;

    for ( j = 0L; j < rPoly3D.count(); j++ )
    {
        const basegfx::B3DPoint aPoint(rPoly3D.getB3DPoint(j));
        const double fDepth(-( aPoint.getZ() * nSkewAmount ) / 100.0);
        const basegfx::B3DPoint aNewPoint(
            aPoint.getX() + (fDepth * cos( fSkewAngle )),
            aPoint.getY() - ( fDepth * sin( fSkewAngle )),
            aPoint.getZ());
        aRetval.append(aNewPoint);
    }

    return aRetval;
}

Point SvxMSDffCustomShape3D::Transformation2D::Transform2D( const basegfx::B3DPoint& rPoint3D )
{
    Point aPoint2D;
    if ( bParallel )
    {
        aPoint2D.X() = (sal_Int32)rPoint3D.getX();
        aPoint2D.Y() = (sal_Int32)rPoint3D.getY();
    }
    else
    {
        double fX = rPoint3D.getX() - fViewPointOriginX;
        double fY = rPoint3D.getY() - fViewPointOriginY;
        double f = ( fZScreen - fViewPoint.getZ() ) / ( rPoint3D.getZ() - fViewPoint.getZ() );
        aPoint2D.X() = (sal_Int32)(( fX - fViewPoint.getX() ) * f + fViewPoint.getX() + fViewPointOriginX );
        aPoint2D.Y() = (sal_Int32)(( fY - fViewPoint.getY() ) * f + fViewPoint.getY() + fViewPointOriginY );
    }
    aPoint2D.Move( aCenter.X(), aCenter.Y() );
    return aPoint2D;
}

basegfx::B3DPoint SvxMSDffCustomShape3D::Rotate( const basegfx::B3DPoint& rPoint, const double x, const double y, const double z )
{
    basegfx::B3DPoint aRetval(rPoint);
     basegfx::B3DPoint aPoint( rPoint );

    // rotation z axis
    aRetval.setX(aPoint.getX() * cos( z ) + aPoint.getY() * sin( z ));
    aRetval.setY(-( aPoint.getX() * sin( z ) ) + aPoint.getY() * cos( z ));

    // rotation y axis
    aPoint = aRetval;
    aRetval.setX(aPoint.getX() * cos( y ) + aPoint.getZ() * sin( y ));
    aRetval.setZ(-( aPoint.getX() * sin( y ) ) + aPoint.getZ() * cos( y ));

    // rotation x axis
    aPoint = aRetval;
    aRetval.setY(aPoint.getY() * cos( x ) + aPoint.getZ() * sin( x ));
    aRetval.setZ(-( aPoint.getY() * sin( x ) ) + aPoint.getZ() * cos( x ));

    return aRetval;
}

//basegfx::B3DPolyPolygon SvxMSDffCustomShape3D::Rotate( const basegfx::B3DPolyPolygon& rPolyPoly3D, const basegfx::B3DPoint& rOrigin, const double x, const double y, const double z )
//{
//  basegfx::B3DPolyPolygon aRetval;
//  sal_uInt32 i, j;
//
//  for( i = 0L; i < rPolyPoly3D.count(); i++ )
//  {
//      const basegfx::B3DPolygon aPoly3D(rPolyPoly3D.getB3DPolygon(i));
//      basegfx::B3DPolygon rNewPoly;
//
//      for( j = 0L; j < aPoly3D.count(); j++ )
//      {
//          basegfx::B3DPoint aPoint(aPoly3D.getB3DPoint(j));
//
//          aPoint -= rOrigin;
//          aPoint = Rotate( aPoint, x, y, z );
//          aPoint += rOrigin;
//
//          rNewPoly.append(aPoint);
//      }
//
//      aRetval.append(rNewPoly);
//  }
//
//  return aRetval;
//}

SdrObject* SvxMSDffCustomShape3D::Create3DObject( const SdrObject* pObj, const DffPropSet& rPropSet,
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
    pScene->SetMergedItem( Svx3DShadeModeItem( 0 ) );
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
        const SfxItemSet& rNextItemSet = pNext->GetMergedItemSet();

        sal_Bool bIsPlaceholderObject = (((XFillStyleItem&)rNextItemSet.Get( XATTR_FILLSTYLE )).GetValue() == XFILL_NONE )
                                     && (((XLineStyleItem&)rNextItemSet.Get( XATTR_LINESTYLE )).GetValue() == XLINE_NONE );

        SdrObject* pNewObj = pNext->ConvertToPolyObj( FALSE, FALSE );
        SdrPathObj* pPath = PTR_CAST( SdrPathObj, pNewObj );
        if( pPath )
        {
            const basegfx::B2DPolyPolygon aPolyPolygon(pPath->GetPathPoly());
            E3dCompoundObject* p3DObj = new E3dExtrudeObj( a3DDefaultAttr, aPolyPolygon, bUseTwoFillStyles ? 1 : fDepth );
            p3DObj->NbcSetLayer( pObj->GetLayer() );
            p3DObj->SetMergedItemSet( aSet );
            if ( bIsPlaceholderObject )
                aPlaceholderObjectList.push_back( p3DObj );
            else if ( bUseTwoFillStyles )
            {
                Bitmap aFillBmp;
                sal_Bool bFillBmpTile = ((XFillBmpTileItem&)p3DObj->GetMergedItem( XATTR_FILLBMP_TILE )).GetValue();
                if ( bFillBmpTile )
                {
                    const XFillBitmapItem& rBmpItm = (XFillBitmapItem&)p3DObj->GetMergedItem( XATTR_FILLBITMAP );
                    const XOBitmap& rXOBmp = rBmpItm.GetBitmapValue();
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
                    p3DObj->SetMergedItem( XFillBitmapItem( String(), aFillBmp ) );
                }
                else
                {
                    Rectangle aBoundRect( PolyPolygon(aPolyPolygon).GetBoundRect() );
                    if ( rSnapRect != aBoundRect )
                    {
                        const XFillBitmapItem& rBmpItm = (XFillBitmapItem&)p3DObj->GetMergedItem( XATTR_FILLBITMAP );
                        const XOBitmap& rXOBmp = rBmpItm.GetBitmapValue();
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
                        p3DObj->SetMergedItem( XFillBitmapItem( String(), aFillBmp ) );
                    }
                }
                pScene->Insert3DObj( p3DObj );
                p3DObj = new E3dExtrudeObj( a3DDefaultAttr, pPath->GetPathPoly(), fDepth );
                p3DObj->NbcSetLayer( pObj->GetLayer() );
                p3DObj->SetMergedItemSet( aSet );
                p3DObj->SetMergedItem( XFillStyleItem( XFILL_SOLID ) );
                p3DObj->SetMergedItem( Svx3DCloseFrontItem( sal_False ) );
                p3DObj->SetMergedItem( Svx3DCloseBackItem( sal_False ) );
                pScene->Insert3DObj( p3DObj );
                p3DObj = new E3dExtrudeObj( a3DDefaultAttr, pPath->GetPathPoly(), 1 );
                p3DObj->NbcSetLayer( pObj->GetLayer() );
                p3DObj->SetMergedItemSet( aSet );
                basegfx::B3DHomMatrix aFrontTransform( p3DObj->GetTransform() );
                aFrontTransform.translate( 0.0, 0.0, fDepth );
                p3DObj->NbcSetTransform( aFrontTransform );
                if ( ( eFillStyle == XFILL_BITMAP ) && !aFillBmp.IsEmpty() )
                    p3DObj->SetMergedItem( XFillBitmapItem( String(), aFillBmp ) );
            }
            else if ( eFillStyle == XFILL_NONE )
            {
                XLineColorItem& rLineColor = (XLineColorItem&)p3DObj->GetMergedItem( XATTR_LINECOLOR );
                p3DObj->SetMergedItem( XFillColorItem( String(), rLineColor.GetColorValue() ) );
                p3DObj->SetMergedItem( Svx3DDoubleSidedItem( sal_True ) );
                p3DObj->SetMergedItem( Svx3DCloseFrontItem( sal_False ) );
                p3DObj->SetMergedItem( Svx3DCloseBackItem( sal_False ) );
            }
            pScene->Insert3DObj( p3DObj );
            bSceneHasObjects = sal_True;
        }
        SdrObject::Free( pNewObj );
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
        const basegfx::B3DRange& rVolume = pScene->GetBoundVolume();
        pScene->NbcSetSnapRect( rSnapRect );

        // InitScene replacement
        double fW(rVolume.getWidth());
        double fH(rVolume.getHeight());

        rCamera.SetAutoAdjustProjection( FALSE );
        rCamera.SetViewWindow( -fW / 2, - fH / 2, fW, fH);
        basegfx::B3DPoint aLookAt( 0.0, 0.0, 0.0 );
        basegfx::B3DPoint aCamPos( 0.0, 0.0, 100.0 );

        rCamera.SetDefaults( basegfx::B3DPoint( 0.0, 0.0, 100.0 ), aLookAt, 100.0 );
        rCamera.SetPosAndLookAt( aCamPos, aLookAt );
        rCamera.SetFocalLength( 1.0 );
        rCamera.SetProjection( eProjectionType );
        pScene->SetCamera( rCamera );
        pScene->SetRectsDirty();

        double fViewPointOriginX = ((double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DOriginX, 32768 )) * rSnapRect.GetWidth()) / 65536.0;
        double fViewPointOriginY = ((double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DOriginY, (sal_uInt32)-32768 )) * rSnapRect.GetHeight()) / 65536.0;

        basegfx::B3DHomMatrix aNewTransform( pScene->GetTransform() );
        Point aCenter( rSnapRect.Center() );
        aNewTransform.translate( -aCenter.X(), aCenter.Y(), -pScene->GetBoundVolume().getDepth() );
        double fXRotate = Fix16ToAngle( rPropSet.GetPropertyValue( DFF_Prop_c3DXRotationAngle, 0 ) );
        double fYRotate = Fix16ToAngle( rPropSet.GetPropertyValue( DFF_Prop_c3DYRotationAngle, 0 ) );
        double fZRotate = Fix16ToAngle( rPropSet.GetPropertyValue( DFF_Prop_Rotation, 0 ) );
        if ( fZRotate != 0.0 )
        {
            aNewTransform.rotate(0.0, 0.0, -fZRotate * F_PI180 );
        }
        if ( nSpFlags & SP_FFLIPH )
        {
            aNewTransform.scale( -1.0, 0.0, 0.0 );
        }
        if ( nSpFlags & SP_FFLIPV )
        {
            aNewTransform.scale(0.0, -1.0, 0.0 );
        }
        if( fYRotate != 0.0 )
        {
            aNewTransform.rotate( 0.0, -fYRotate * F_PI180, 0.0 );
        }
        if( fXRotate != 0.0 )
        {
            aNewTransform.rotate( -fXRotate * F_PI180, 0.0, 0.0 );
        }
        if ( eProjectionType == PR_PARALLEL )
        {
            sal_Int32 nSkewAmount = rPropSet.GetPropertyValue( DFF_Prop_c3DSkewAmount, 50 );
            if ( nSkewAmount )
            {
                sal_Int32 nSkewAngle = ((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DSkewAngle, sal::static_int_cast< UINT32 >( -135 * 65536 ) ) ) >> 16;

                double fAlpha( (double)nSkewAngle * F_PI180 );
                double fInvTanBeta( (double)nSkewAmount / 100.0 );
                if(fInvTanBeta)
                {
                    aNewTransform.shearXY(
                        fInvTanBeta * cos(fAlpha),
                        fInvTanBeta * sin(fAlpha));
                }
            }
        }
        else
        {
            aNewTransform.translate( -fViewPointOriginX, fViewPointOriginY, 0.0 );
            // now set correct camera position
//          double fViewPointOriginX = ((double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DOriginX, 32768 )) * rSnapRect.GetWidth()) / 65536.0;
//          double fViewPointOriginY = ((double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DOriginY, (sal_uInt32)-32768 )) * rSnapRect.GetHeight()) / 65536.0;
            double fViewPointX = (double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DXViewpoint, 1250000 )) / 360;
            double fViewPointY = (double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DYViewpoint,(sal_uInt32)-1250000 )) / 360;
            double fViewPointZ = (double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DZViewpoint,(sal_uInt32)-9000000 )) / 360;

            basegfx::B3DPoint aNewLookAt( fViewPointX, -fViewPointY, 0 );
            basegfx::B3DPoint aNewCamPos( fViewPointX, -fViewPointY, -fViewPointZ );
            rCamera.SetPosAndLookAt( aNewCamPos, aNewLookAt );
            pScene->SetCamera( rCamera );
        }
        pScene->NbcSetTransform( aNewTransform );

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
//      sal_Bool bLightHarsh = ( rPropSet.GetPropertyValue( DFF_Prop_fc3DFillHarsh, 0 ) & 2 ) != 0;

        sal_Int32 nLight2X = ((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DFillX, (sal_uInt32)-50000 )) / 360;
        sal_Int32 nLight2Y = - ((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DFillY, 0 )) / 360;
        sal_Int32 nLight2Z = - ((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DFillZ, 10000 )) / 360;
        if ( !nLight2Z )
            nLight2Z = -1;
        double fLight2Intensity = ((double)rPropSet.GetPropertyValue( DFF_Prop_c3DFillIntensity, 43712 )) / 65536.0;
//      sal_Bool bLight2Harsh = ( rPropSet.GetPropertyValue( DFF_Prop_fc3DFillHarsh, 0 ) & 1 ) != 0;
//      sal_Bool bLightFace = ( rPropSet.GetPropertyValue( DFF_Prop_fc3DLightFace, 0 ) & 1 ) != 0;

        sal_uInt16 nAmbientColor = (sal_uInt16)( fAmbientIntensity * 255.0 );
        if ( nAmbientColor > 255 )
            nAmbientColor = 255;
        Color aGlobalAmbientColor( (sal_uInt8)nAmbientColor, (sal_uInt8)nAmbientColor, (sal_uInt8)nAmbientColor );
        pScene->SetMergedItem( Svx3DAmbientcolorItem( aGlobalAmbientColor ) );

        sal_uInt8 nSpotLight1 = (sal_uInt8)( fLightIntensity * 255.0 );
        basegfx::B3DVector aSpotLight1( (double)nLightX, (double)nLightY, (double)nLightZ );
        aSpotLight1.normalize();
        pScene->SetMergedItem( Svx3DLightOnOff1Item( sal_True ) );
        Color aAmbientSpot1Color( nSpotLight1, nSpotLight1, nSpotLight1 );
        pScene->SetMergedItem( Svx3DLightcolor1Item( aAmbientSpot1Color ) );
        pScene->SetMergedItem( Svx3DLightDirection1Item( aSpotLight1 ) );

        sal_uInt8 nSpotLight2 = (sal_uInt8)( fLight2Intensity * 255.0 );
        basegfx::B3DVector aSpotLight2( (double)nLight2X, (double)nLight2Y, (double)nLight2Z );
        aSpotLight2.normalize();
        pScene->SetMergedItem( Svx3DLightOnOff2Item( sal_True ) );
        Color aAmbientSpot2Color( nSpotLight2, nSpotLight2, nSpotLight2 );
        pScene->SetMergedItem( Svx3DLightcolor2Item( aAmbientSpot2Color ) );
        pScene->SetMergedItem( Svx3DLightDirection2Item( aSpotLight2 ) );

        if ( nLightX || nLightY )
        {
            sal_uInt8 nSpotLight3 = 70;
            basegfx::B3DVector aSpotLight3( 0.0, 0.0, 1.0 );
            pScene->SetMergedItem( Svx3DLightOnOff3Item( sal_True ) );
            Color aAmbientSpot3Color( nSpotLight3, nSpotLight3, nSpotLight3 );
            pScene->SetMergedItem( Svx3DLightcolor3Item( aAmbientSpot3Color ) );
            pScene->SetMergedItem( Svx3DLightDirection3Item( aSpotLight3 ) );
        }

        double fSpecular = ((double)rPropSet.GetPropertyValue( DFF_Prop_c3DSpecularAmt, 0 )) / 65536.0;
//      double fDiffuse = ((double)rPropSet.GetPropertyValue( DFF_Prop_c3DDiffuseAmt, 0 )) / 65536.0;
//      double fShininess = ((double)rPropSet.GetPropertyValue( DFF_Prop_c3DShininess, 0 )) / 65536.0;

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
        pScene->SetMergedItem( Svx3DMaterialSpecularItem( aSpecularCol ) );
        pScene->SetMergedItem( Svx3DMaterialSpecularIntensityItem( nItensity ) );

        pScene->SetModel( pObj->GetModel() );
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

Rectangle SvxMSDffCustomShape3D::CalculateNewSnapRect( const Rectangle& rOriginalSnapRect, const DffPropSet& rPropSet )
{
    const Point aCenter( rOriginalSnapRect.Center() );
    //double fExtrusionBackward = (double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DExtrudeBackward, 457200 )) / 360.0;
    double fExtrusionForward  = (double)((sal_Int32)rPropSet.GetPropertyValue( DFF_Prop_c3DExtrudeForward, 0 )) / 360.0;
    sal_uInt32 i;

    // creating initial bound volume ( without rotation. skewing.and camera )
    basegfx::B3DPolygon aBoundVolume;
    const Polygon aPolygon( rOriginalSnapRect );

    for ( i = 0L; i < 4L; i++ )
    {
        aBoundVolume.append(basegfx::B3DPoint(
            aPolygon[ (sal_uInt16)i ].X() - aCenter.X(),
            aPolygon[ (sal_uInt16)i ].Y() - aCenter.Y(),
            fExtrusionForward));
    }

    for ( ; i < 8L; i++ )
    {
        aBoundVolume.append(basegfx::B3DPoint(
            aPolygon[ (sal_uInt16)i ].X() - aCenter.X(),
            aPolygon[ (sal_uInt16)i ].Y() - aCenter.Y(),
            fExtrusionForward));
    }

    basegfx::B3DPoint aRotateCenter;
    aRotateCenter.setX(0.0);
    aRotateCenter.setY(0.0);
    aRotateCenter.setZ(rPropSet.GetPropertyValue( DFF_Prop_c3DRotationCenterZ, 0 ) / 360);

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
        basegfx::B3DPoint aPoint(aBoundVolume.getB3DPoint(i));
        aPoint -= aRotateCenter;
        aPoint = Rotate( aPoint, fXRotate, fYRotate, fZRotate );
        aPoint += aRotateCenter;
        aBoundVolume.setB3DPoint(i, aPoint);
    }

    Transformation2D aTransformation2D( rPropSet, rOriginalSnapRect );
    if ( aTransformation2D.IsParallel() )
    {
        aBoundVolume = aTransformation2D.ApplySkewSettings( aBoundVolume );
    }

    Polygon aTransformed( 8 );
    for ( i = 0L; i < 8L; i++ )
    {
        aTransformed[ (sal_uInt16)i ] = aTransformation2D.Transform2D( aBoundVolume.getB3DPoint(i) );
    }

    return aTransformed.GetBoundRect();
}

// eof

/*************************************************************************
 *
 *  $RCSfile: EnhancedCustomShape3d.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2004-06-10 11:31:57 $
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

#ifndef _ENHANCEDCUSTOMSHAPE3D_HXX
#include "EnhancedCustomShape3d.hxx"
#endif
#ifndef _SVDETC_HXX
#include <svdetc.hxx>
#endif
#ifndef _SVDMODEL_HXX
#include <svdmodel.hxx>
#endif
#ifndef _XOUTX_HXX
#include <xoutx.hxx>
#endif
#ifndef _SV_POLY_HXX
#include <tools/poly.hxx>
#endif
#ifndef _SVDITER_HXX
#include <svditer.hxx>
#endif
#ifndef _SVDOBJ_HXX
#include <svdobj.hxx>
#endif
#ifndef _SVDOASHP_HXX
#include <svdoashp.hxx>
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
#ifndef SVX_XSFLCLIT_HXX
#include <xsflclit.hxx>
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
#ifndef _SDASITM_HXX
#include <sdasitm.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_POINT_HPP_
#include <com/sun/star/awt/Point.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POSITION3D_HPP_
#include <com/sun/star/drawing/Position3D.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_DIRECTION3D_HPP_
#include <com/sun/star/drawing/Direction3D.hpp>
#endif
#ifndef _SDR_PROPERTIES_PROPERTIES_HXX
#include <svx/sdr/properties/properties.hxx>
#endif

#define ITEMVALUE(ItemSet,Id,Cast)  ((const Cast&)(ItemSet).Get(Id)).GetValue()
using namespace com::sun::star;
using namespace com::sun::star::uno;

const rtl::OUString sExtrusion( RTL_CONSTASCII_USTRINGPARAM ( "Extrusion" ) );
double GetDouble( SdrCustomShapeGeometryItem& rItem, const rtl::OUString& rPropertyName, double fDefault, const double* pMap )
{
    double fRetValue = fDefault;
    Any* pAny = rItem.GetPropertyValueByName( sExtrusion, rPropertyName );
    if ( pAny )
        *pAny >>= fRetValue;
    if ( pMap )
        fRetValue *= *pMap;
    return fRetValue;
}
sal_Int32 GetInt32( SdrCustomShapeGeometryItem& rItem, const rtl::OUString& rPropertyName, sal_Int32 nDefault )
{
    sal_Int32 nRetValue = nDefault;
    Any* pAny = rItem.GetPropertyValueByName( sExtrusion, rPropertyName );
    if ( pAny )
        *pAny >>= nRetValue;
    return nRetValue;

}
sal_Bool GetBool( SdrCustomShapeGeometryItem& rItem, const rtl::OUString& rPropertyName, sal_Bool bDefault )
{
    sal_Bool bRetValue = bDefault;
    const Any* pAny = rItem.GetPropertyValueByName( sExtrusion, rPropertyName );
    if ( pAny )
        *pAny >>= bRetValue;
    return bRetValue;
}
awt::Point GetPoint( SdrCustomShapeGeometryItem& rItem, const rtl::OUString& rPropertyName, const awt::Point& rDefault )
{
    awt::Point aRetValue( rDefault );
    const Any* pAny = rItem.GetPropertyValueByName( sExtrusion, rPropertyName );
    if ( pAny )
        *pAny >>= aRetValue;
    return aRetValue;
}
drawing::Position3D GetPosition3D( SdrCustomShapeGeometryItem& rItem, const rtl::OUString& rPropertyName,
                                    const drawing::Position3D& rDefault, const double* pMap )
{
    drawing::Position3D aRetValue( rDefault );
    const Any* pAny = rItem.GetPropertyValueByName( sExtrusion, rPropertyName );
    if ( pAny )
        *pAny >>= aRetValue;
    if ( pMap )
    {
        aRetValue.PositionX *= *pMap;
        aRetValue.PositionY *= *pMap;
        aRetValue.PositionZ *= *pMap;
    }
    return aRetValue;
}

drawing::Direction3D GetDirection3D( SdrCustomShapeGeometryItem& rItem, const rtl::OUString& rPropertyName, const drawing::Direction3D& rDefault )
{
    drawing::Direction3D aRetValue( rDefault );
    const Any* pAny = rItem.GetPropertyValueByName( sExtrusion, rPropertyName );
    if ( pAny )
        *pAny >>= aRetValue;
    return aRetValue;
}

EnhancedCustomShape3d::Transformation2D::Transformation2D( const SdrObject* pCustomShape, const Rectangle& rBoundRect, const double *pM ) :
    aCenter( pCustomShape->GetSnapRect().Center() ),
    pMap( pM )
{
    SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)pCustomShape->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );

    const rtl::OUString sParallel( RTL_CONSTASCII_USTRINGPARAM ( "Parallel" ) );
    bParallel = GetBool( rGeometryItem, sParallel, sal_True );
    if ( bParallel )
    {
        const rtl::OUString sSkewAngle( RTL_CONSTASCII_USTRINGPARAM ( "SkewAngle" ) );
        const rtl::OUString sSkew     ( RTL_CONSTASCII_USTRINGPARAM ( "Skew" ) );
        fSkewAngle = GetDouble( rGeometryItem, sSkewAngle, -135, NULL ) * F_PI180;
        fSkew = GetDouble( rGeometryItem, sSkew, 50, NULL );
    }
    else
    {
        fZScreen = 0.0;
        const rtl::OUString sOriginX( RTL_CONSTASCII_USTRINGPARAM ( "OriginX" ) );
        const rtl::OUString sOriginY( RTL_CONSTASCII_USTRINGPARAM ( "OriginY" ) );
        fOriginX = GetDouble( rGeometryItem, sOriginX,  50.0, NULL );
        fOriginY = GetDouble( rGeometryItem, sOriginY, -50.0, NULL );
        fOriginX = ( fOriginX * pCustomShape->GetLogicRect().GetWidth() ) / 100;
        fOriginY = ( fOriginY * pCustomShape->GetLogicRect().GetHeight()) / 100;

        const rtl::OUString sViewPoint( RTL_CONSTASCII_USTRINGPARAM ( "ViewPoint" ) );
        drawing::Position3D aViewPointDefault( 3472, -3472, 25000 );
        drawing::Position3D aViewPoint( GetPosition3D( rGeometryItem, sViewPoint, aViewPointDefault, pMap ) );
        fViewPoint.X() = aViewPoint.PositionX;
        fViewPoint.Y() = aViewPoint.PositionY;
        fViewPoint.W() = -aViewPoint.PositionZ;
    }
}

void EnhancedCustomShape3d::Transformation2D::ApplySkewSettings( Polygon3D& rPoly3D )
{
    sal_uInt16 j;
    for ( j = 0; j < rPoly3D.GetPointCount(); j++ )
    {
        Vector3D& rPoint = rPoly3D[ j ];
        double fDepth = -( rPoint.Z() * fSkew ) / 100.0;
        rPoint.X() += fDepth * cos( fSkewAngle );
        rPoint.Y() += -( fDepth * sin( fSkewAngle ) );
    }
}

Point EnhancedCustomShape3d::Transformation2D::Transform2D( const Vector3D& rPoint3D )
{
    Point aPoint2D;
    if ( bParallel )
    {
        aPoint2D.X() = (sal_Int32)rPoint3D.X();
        aPoint2D.Y() = (sal_Int32)rPoint3D.Y();
    }
    else
    {
        double fX = rPoint3D.X() - fOriginX;
        double fY = rPoint3D.Y() - fOriginY;
        double f = ( fZScreen - fViewPoint.W() ) / ( rPoint3D.Z() - fViewPoint.W() );
        aPoint2D.X() = (sal_Int32)(( fX - fViewPoint.X() ) * f + fViewPoint.X() + fOriginX );
        aPoint2D.Y() = (sal_Int32)(( fY - fViewPoint.Y() ) * f + fViewPoint.Y() + fOriginY );
    }
    aPoint2D.Move( aCenter.X(), aCenter.Y() );
    return aPoint2D;
}

void EnhancedCustomShape3d::Rotate( Vector3D& rPoint, const double x, const double y, const double z )
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

void EnhancedCustomShape3d::Rotate( PolyPolygon3D& rPolyPoly3D, const Point3D& rOrigin, const double x, const double y, const double z )
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

sal_Bool IsClockwise( const XPolygon& rPoly )
{
    sal_Bool    bIsClockwise = sal_True;
    sal_uInt16  i, nPoints = rPoly.GetPointCount();
    if ( nPoints > 1 )
    {
        double fArea = rPoly[ nPoints - 1 ].X() * rPoly[ 0 ].Y() - rPoly[ 0 ].X() * rPoly[ nPoints - 1 ].Y();
        for ( i = 0; i < nPoints - 1; i++ )
            fArea += rPoly[ i ].X() * rPoly[ i + 1 ].Y() - rPoly[ i + 1 ].X() * rPoly[ i ].Y();
        bIsClockwise = fArea < 0.0;
    }
    return bIsClockwise;
}

SdrObject* EnhancedCustomShape3d::Create3DObject( const SdrObject* pShape2d, const SdrObject* pCustomShape )
{
    SdrObject*  pRet = NULL;
    SdrModel*   pModel = pCustomShape->GetModel();
    SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)pCustomShape->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );

    double      fMap, *pMap = NULL;
    if ( pModel )
    {
        fMap = 1.0;
        Fraction aFraction( pModel->GetScaleFraction() );
        if ( ( aFraction.GetNumerator() ) != 1 || ( aFraction.GetDenominator() != 1 ) )
        {
            fMap *= aFraction.GetNumerator();
            fMap /= aFraction.GetDenominator();
            pMap = &fMap;
        }
        if ( pModel->GetScaleUnit() != MAP_100TH_MM )
        {
            DBG_ASSERT( pModel->GetScaleUnit() == MAP_TWIP, "EnhancedCustomShape3d::Current MapMode is Unsupported" );
            fMap *= 1440.0 / 2540.0;
            pMap = &fMap;
        }
    }
    const rtl::OUString sOn( RTL_CONSTASCII_USTRINGPARAM ( "On" ) );
    if ( GetBool( rGeometryItem, sOn, sal_False ) )
    {
        sal_Bool bIsMirroredX = ((SdrObjCustomShape*)pCustomShape)->IsMirroredX();
        sal_Bool bIsMirroredY = ((SdrObjCustomShape*)pCustomShape)->IsMirroredY();
        Rectangle aSnapRect( pCustomShape->GetLogicRect() );
        long nObjectRotation = pCustomShape->GetRotateAngle();
        if ( nObjectRotation )
        {
            double a = ( 36000 - nObjectRotation ) * nPi180;
            long dx = aSnapRect.Right() - aSnapRect.Left();
            long dy = aSnapRect.Bottom()- aSnapRect.Top();
            Point aP( aSnapRect.TopLeft() );
            RotatePoint( aP, pCustomShape->GetSnapRect().Center(), sin( a ), cos( a ) );
            aSnapRect.Left() = aP.X();
            aSnapRect.Top() = aP.Y();
            aSnapRect.Right() = aSnapRect.Left() + dx;
            aSnapRect.Bottom() = aSnapRect.Top() + dy;
        }
        Point aCenter( aSnapRect.Center() );

        SfxItemSet aSet( pCustomShape->GetMergedItemSet() );
        std::vector< E3dCompoundObject* > aPlaceholderObjectList;

        const rtl::OUString sBackwardDepth( RTL_CONSTASCII_USTRINGPARAM ( "BackwardDepth" ) );
        const rtl::OUString sForewardDepth( RTL_CONSTASCII_USTRINGPARAM ( "ForewardDepth" ) );
        double fExtrusionBackward = GetDouble( rGeometryItem, sBackwardDepth, 1270, pMap );
        double fExtrusionForward  = GetDouble( rGeometryItem, sForewardDepth, 0, pMap );

        double fDepth = fExtrusionBackward - fExtrusionForward;
        if ( fDepth < 1.0 )
            fDepth = 1.0;
        const rtl::OUString sParallel( RTL_CONSTASCII_USTRINGPARAM ( "Parallel" ) );
        ProjectionType eProjectionType( GetBool( rGeometryItem, sParallel, sal_True ) ? PR_PARALLEL : PR_PERSPECTIVE );

        // pShape2d Umwandeln in Szene mit 3D Objekt
        E3dDefaultAttributes a3DDefaultAttr;
        a3DDefaultAttr.SetDefaultLatheCharacterMode( TRUE );
        a3DDefaultAttr.SetDefaultExtrudeCharacterMode( TRUE );

        E3dScene* pScene = new E3dPolyScene( a3DDefaultAttr );

        sal_Bool bSceneHasObjects ( sal_False );
        sal_Bool bUseTwoFillStyles( sal_False );

        const rtl::OUString sRenderMode( RTL_CONSTASCII_USTRINGPARAM ( "RenderMode" ) );
        MSO_3DRenderMode eRenderMode( (MSO_3DRenderMode)GetInt32( rGeometryItem, sRenderMode, mso_FullRender ) );
        const rtl::OUString sExtrusionColor( RTL_CONSTASCII_USTRINGPARAM ( "Color" ) );
        sal_Bool bUseExtrusionColor = GetBool( rGeometryItem, sExtrusionColor, sal_False );

        XFillStyle eFillStyle( ITEMVALUE( aSet, XATTR_FILLSTYLE, XFillStyleItem ) );
        pScene->GetProperties().SetObjectItem( Svx3DShadeModeItem( 0 ) );
        aSet.Put( Svx3DPercentDiagonalItem( 0 ) );
        aSet.Put( Svx3DTextureModeItem( 1 ) );
        aSet.Put( Svx3DNormalsKindItem( 1 ) );

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
            else if ( ( eFillStyle == XFILL_BITMAP ) || ( eFillStyle == XFILL_GRADIENT ) || bUseExtrusionColor )
                bUseTwoFillStyles = sal_True;

            // #116336#
            // If shapes are mirrored once (mirroring two times correct geometry again)
            // double-sided at the object and two-sided-lighting at the scene need to be set.
            if((bIsMirroredX && !bIsMirroredY) || (!bIsMirroredX && bIsMirroredY))
            {
                aSet.Put( Svx3DDoubleSidedItem( sal_True ) );
                pScene->GetProperties().SetObjectItem( Svx3DTwoSidedLightingItem( sal_True ) );
            }
        }

        Rectangle aBoundRect2d;
        SdrObjListIter aIter( *pShape2d, IM_DEEPWITHGROUPS );
        while( aIter.IsMore() )
        {
            const SdrObject* pNext = aIter.Next();
            sal_Bool bIsPlaceholderObject = (((XFillStyleItem&)pNext->GetMergedItem( XATTR_FILLSTYLE )).GetValue() == XFILL_NONE )
                                        && (((XLineStyleItem&)pNext->GetMergedItem( XATTR_LINESTYLE )).GetValue() == XLINE_NONE );
            XPolyPolygon aXPolyPoly;
            if ( pNext->ISA( SdrPathObj ) )
            {
                sal_uInt16 i;
                const XPolyPolygon& rPolyPolygon = ((SdrPathObj*)pNext)->GetPathPoly();
                for ( i = 0; i < rPolyPolygon.Count(); i++ )
                    aXPolyPoly.Insert( XOutCreatePolygon( rPolyPolygon.GetObject( i ), NULL, 100 ), XPOLYPOLY_APPEND );
            }
            else
            {
                SdrObject* pNewObj = pNext->ConvertToPolyObj( FALSE, FALSE );
                SdrPathObj* pPath = PTR_CAST( SdrPathObj, pNewObj );
                if ( pPath )
                    aXPolyPoly = pPath->GetPathPoly();
                delete pNewObj;
            }
            if( aXPolyPoly.Count() )
            {
                Rectangle aBoundRect( aXPolyPoly.GetBoundRect() );
                aBoundRect2d.Union( aBoundRect );

                E3dCompoundObject* p3DObj = new E3dExtrudeObj( a3DDefaultAttr,
                    aXPolyPoly, bUseTwoFillStyles ? 0 : fDepth );
                p3DObj->NbcSetLayer( pShape2d->GetLayer() );
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
                        p3DObj->SetMergedItem( XFillBitmapItem( String(), aFillBmp ) );
                    }
                    else
                    {
                        if ( aSnapRect != aBoundRect )
                        {
                            const XFillBitmapItem& rBmpItm = (XFillBitmapItem&)p3DObj->GetMergedItem( XATTR_FILLBITMAP );
                            const XOBitmap& rXOBmp = rBmpItm.GetValue();
                            aFillBmp = rXOBmp.GetBitmap();
                            Size aBmpSize( aFillBmp.GetSizePixel() );
                            double fXScale = (double)aBoundRect.GetWidth() / (double)aSnapRect.GetWidth();
                            double fYScale = (double)aBoundRect.GetHeight() / (double)aSnapRect.GetHeight();

                            Point aPt( (sal_Int32)( (double)( aBoundRect.Left() - aSnapRect.Left() )* (double)aBmpSize.Width() / (double)aSnapRect.GetWidth() ),
                                                (sal_Int32)( (double)( aBoundRect.Top() - aSnapRect.Top() ) * (double)aBmpSize.Height() / (double)aSnapRect.GetHeight() ) );
                            Size aSize( (sal_Int32)( aBmpSize.Width() * fXScale ),
                                                    (sal_Int32)( aBmpSize.Height() * fYScale ) );
                            Rectangle aCropRect( aPt, aSize );
                             aFillBmp.Crop( aCropRect );
                            p3DObj->SetMergedItem( XFillBitmapItem( String(), aFillBmp ) );
                        }
                    }
                    pScene->Insert3DObj( p3DObj );
                    p3DObj = new E3dExtrudeObj( a3DDefaultAttr, aXPolyPoly, fDepth );
                    p3DObj->NbcSetLayer( pShape2d->GetLayer() );
                    p3DObj->SetMergedItemSet( aSet );
                    if ( bUseExtrusionColor )
                        p3DObj->SetMergedItem( XFillColorItem( String(), ((XSecondaryFillColorItem&)p3DObj->GetMergedItem( XATTR_SECONDARYFILLCOLOR )).GetValue() ) );
                    p3DObj->SetMergedItem( XFillStyleItem( XFILL_SOLID ) );
                    p3DObj->SetMergedItem( Svx3DCloseFrontItem( sal_False ) );
                    p3DObj->SetMergedItem( Svx3DCloseBackItem( sal_False ) );
                    pScene->Insert3DObj( p3DObj );
                    p3DObj = new E3dExtrudeObj( a3DDefaultAttr, aXPolyPoly, 0 );
                    p3DObj->NbcSetLayer( pShape2d->GetLayer() );
                    p3DObj->SetMergedItemSet( aSet );
                    Matrix4D aFrontTransform( p3DObj->GetTransform() );
                    aFrontTransform.Translate( 0, 0, fDepth );
                    p3DObj->NbcSetTransform( aFrontTransform );
                    if ( ( eFillStyle == XFILL_BITMAP ) && !aFillBmp.IsEmpty() )
                        p3DObj->SetMergedItem( XFillBitmapItem( String(), aFillBmp ) );
                }
                else if ( eFillStyle == XFILL_NONE )
                {
                    XLineColorItem& rLineColor = (XLineColorItem&)p3DObj->GetMergedItem( XATTR_LINECOLOR );
                    p3DObj->SetMergedItem( XFillColorItem( String(), rLineColor.GetValue() ) );
                    p3DObj->SetMergedItem( Svx3DDoubleSidedItem( sal_True ) );
                    p3DObj->SetMergedItem( Svx3DCloseFrontItem( sal_False ) );
                    p3DObj->SetMergedItem( Svx3DCloseBackItem( sal_False ) );
                }
                pScene->Insert3DObj( p3DObj );
                bSceneHasObjects = sal_True;
            }
        }

        if ( bSceneHasObjects ) // is the SdrObject properly converted
        {
            // then we can change the return value
            pRet = pScene;

            // Kameraeinstellungen, Perspektive ...
            Camera3D& rCamera = (Camera3D&)pScene->GetCamera();
            const Volume3D& rVolume = pScene->GetBoundVolume();
            pScene->CorrectSceneDimensions();
            pScene->NbcSetSnapRect( aSnapRect );

            // InitScene replacement
            double fW = rVolume.GetWidth();
            double fH = rVolume.GetHeight();

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

            const rtl::OUString sOriginX( RTL_CONSTASCII_USTRINGPARAM ( "OriginX" ) );
            const rtl::OUString sOriginY( RTL_CONSTASCII_USTRINGPARAM ( "OriginY" ) );
            double fOriginX = GetDouble( rGeometryItem, sOriginX,  50.0, NULL );
            double fOriginY = GetDouble( rGeometryItem, sOriginY, -50.0, NULL );
            fOriginX = ( fOriginX * aSnapRect.GetWidth())  / 100;
            fOriginY = ( fOriginY * aSnapRect.GetHeight()) / 100;

            Matrix4D aNewTransform( pScene->GetTransform() );
            aNewTransform.Translate( -aCenter.X(), aCenter.Y(), -pScene->GetBoundVolume().GetDepth() );
            const rtl::OUString sAngleX( RTL_CONSTASCII_USTRINGPARAM ( "AngleX" ) );
            const rtl::OUString sAngleY( RTL_CONSTASCII_USTRINGPARAM ( "AngleY" ) );
            double fXRotate = GetDouble( rGeometryItem, sAngleX, 0, NULL ) * F_PI180 ;
            double fYRotate = GetDouble( rGeometryItem, sAngleY, 0, NULL ) * F_PI180;
            double fZRotate = ((SdrObjCustomShape*)pCustomShape)->GetObjectRotation() * F_PI180;
            if ( fZRotate != 0.0 )
                aNewTransform.RotateZ( fZRotate );
            if ( bIsMirroredX )
                aNewTransform.ScaleX( -1 );
            if ( bIsMirroredY )
                aNewTransform.ScaleY( -1 );
            if( fYRotate != 0.0 )
                aNewTransform.RotateY( -fYRotate );
            if( fXRotate != 0.0 )
                aNewTransform.RotateX( -fXRotate );
            if ( eProjectionType == PR_PARALLEL )
            {
                const rtl::OUString sSkew( RTL_CONSTASCII_USTRINGPARAM ( "Skew" ) );
                double fSkew = GetDouble( rGeometryItem, sSkew, 50, NULL );
                if ( fSkew != 0.0 )
                {
                    const rtl::OUString sSkewAngle( RTL_CONSTASCII_USTRINGPARAM ( "SkewAngle" ) );
                    double fAlpha = GetDouble( rGeometryItem, sSkewAngle, -135, NULL ) * F_PI180;
                    double fInvTanBeta( fSkew / 100.0 );
                    if(fInvTanBeta)
                    {
                        aNewTransform.ShearXY(
                            fInvTanBeta * cos(fAlpha),
                            fInvTanBeta * sin(fAlpha));
                    }
                }
                Vector3D aLookAt( 0, 0, 0 );
                Vector3D aNewCamPos( 0, 0, 25000 );
                rCamera.SetPosAndLookAt( aNewCamPos, aLookAt );
                pScene->SetCamera( rCamera );
            }
            else
            {
                aNewTransform.Translate( -fOriginX, fOriginY, 0 );
                // now set correct camera position
                const rtl::OUString sViewPoint( RTL_CONSTASCII_USTRINGPARAM ( "ViewPoint" ) );
                drawing::Position3D aViewPointDefault( 3472, -3472, 25000 );
                drawing::Position3D aViewPoint( GetPosition3D( rGeometryItem, sViewPoint, aViewPointDefault, pMap ) );
                double fViewPointX = aViewPoint.PositionX;
                double fViewPointY = aViewPoint.PositionY;
                double fViewPointZ = aViewPoint.PositionZ;
                Vector3D aLookAt( fViewPointX, -fViewPointY, 0 );
                Vector3D aNewCamPos( fViewPointX, -fViewPointY, fViewPointZ );
                rCamera.SetPosAndLookAt( aNewCamPos, aLookAt );
                pScene->SetCamera( rCamera );
            }
            pScene->NbcSetTransform( aNewTransform );
            pScene->FitSnapRectToBoundVol();

            ///////////
            // light //
            ///////////

            const rtl::OUString sBrightness( RTL_CONSTASCII_USTRINGPARAM ( "Brightness" ) );
            double fAmbientIntensity = GetDouble( rGeometryItem, sBrightness, 22178.0 / 655.36, NULL ) / 100.0;


            const rtl::OUString sLightDirection1( RTL_CONSTASCII_USTRINGPARAM ( "LightDirection1" ) );
            drawing::Direction3D aLightDirection1Default( 50000, 0, 10000 );
            drawing::Direction3D aLightDirection1( GetDirection3D( rGeometryItem, sLightDirection1, aLightDirection1Default ) );
            if ( aLightDirection1.DirectionZ == 0.0 )
                aLightDirection1.DirectionZ = 1.0;

            const rtl::OUString sLightLevel1( RTL_CONSTASCII_USTRINGPARAM ( "LightLevel1" ) );
            double fLightIntensity = GetDouble( rGeometryItem, sLightLevel1, 43712.0 / 655.36, NULL ) / 100.0;

            const rtl::OUString sLightHarsh1( RTL_CONSTASCII_USTRINGPARAM ( "LightHarsh1" ) );
            sal_Bool bLightHarsh1 = GetBool( rGeometryItem, sLightHarsh1, sal_False );

            const rtl::OUString sLightDirection2( RTL_CONSTASCII_USTRINGPARAM ( "LightDirection2" ) );
            drawing::Direction3D aLightDirection2Default( -50000, 0, 10000 );
            drawing::Direction3D aLightDirection2( GetDirection3D( rGeometryItem, sLightDirection2, aLightDirection2Default ) );
            if ( aLightDirection2.DirectionZ == 0.0 )
                aLightDirection2.DirectionZ = -1;

            const rtl::OUString sLightLevel2( RTL_CONSTASCII_USTRINGPARAM ( "LightLevel2" ) );
            double fLight2Intensity = GetDouble( rGeometryItem, sLightLevel2, 43712.0 / 655.36, NULL ) / 100.0;

            const rtl::OUString sLightHarsh2( RTL_CONSTASCII_USTRINGPARAM ( "LightHarsh2" ) );
            const rtl::OUString sLightFace( RTL_CONSTASCII_USTRINGPARAM ( "LightFace" ) );
            sal_Bool bLight2Harsh = GetBool( rGeometryItem, sLightHarsh2, sal_False );
            sal_Bool bLightFace = GetBool( rGeometryItem, sLightFace, sal_False );

            sal_uInt16 nAmbientColor = (sal_uInt16)( fAmbientIntensity * 255.0 );
            if ( nAmbientColor > 255 )
                nAmbientColor = 255;
            Color aGlobalAmbientColor( (sal_uInt8)nAmbientColor, (sal_uInt8)nAmbientColor, (sal_uInt8)nAmbientColor );
            pScene->GetProperties().SetObjectItem( Svx3DAmbientcolorItem( aGlobalAmbientColor ) );

            sal_uInt8 nSpotLight1 = (sal_uInt8)( fLightIntensity * 255.0 );
            Vector3D aSpotLight1( aLightDirection1.DirectionX, - ( aLightDirection1.DirectionY ), -( aLightDirection1.DirectionZ ) );
            aSpotLight1.Normalize();
            pScene->GetProperties().SetObjectItem( Svx3DLightOnOff1Item( sal_True ) );
            Color aAmbientSpot1Color( nSpotLight1, nSpotLight1, nSpotLight1 );
            pScene->GetProperties().SetObjectItem( Svx3DLightcolor1Item( aAmbientSpot1Color ) );
            pScene->GetProperties().SetObjectItem( Svx3DLightDirection1Item( aSpotLight1 ) );

            sal_uInt8 nSpotLight2 = (sal_uInt8)( fLight2Intensity * 255.0 );
            Vector3D aSpotLight2( aLightDirection2.DirectionX, -aLightDirection2.DirectionY, -aLightDirection2.DirectionZ );
            aSpotLight2.Normalize();
            pScene->GetProperties().SetObjectItem( Svx3DLightOnOff2Item( sal_True ) );
            Color aAmbientSpot2Color( nSpotLight2, nSpotLight2, nSpotLight2 );
            pScene->GetProperties().SetObjectItem( Svx3DLightcolor2Item( aAmbientSpot2Color ) );
            pScene->GetProperties().SetObjectItem( Svx3DLightDirection2Item( aSpotLight2 ) );

                sal_uInt8 nSpotLight3 = 70;
                Vector3D aSpotLight3( 0, 0, 1 );
                pScene->GetProperties().SetObjectItem( Svx3DLightOnOff3Item( sal_True ) );
                Color aAmbientSpot3Color( nSpotLight3, nSpotLight3, nSpotLight3 );
                pScene->GetProperties().SetObjectItem( Svx3DLightcolor3Item( aAmbientSpot3Color ) );
                pScene->GetProperties().SetObjectItem( Svx3DLightDirection3Item( aSpotLight3 ) );

            const rtl::OUString sSpecularity( RTL_CONSTASCII_USTRINGPARAM ( "Specularity" ) );
            const rtl::OUString sDiffusity( RTL_CONSTASCII_USTRINGPARAM ( "Diffusity" ) );
            const rtl::OUString sShininess( RTL_CONSTASCII_USTRINGPARAM ( "Shininess" ) );
            const rtl::OUString sMetal( RTL_CONSTASCII_USTRINGPARAM ( "Metal" ) );
            double fSpecular = GetDouble( rGeometryItem, sSpecularity, 0, NULL ) / 100;
            double fDiffuse = GetDouble( rGeometryItem, sDiffusity, 0, NULL ) / 100;
            double fShininess = GetDouble( rGeometryItem, sShininess, 0, NULL ) / 100;
            sal_Bool bMetal = GetBool( rGeometryItem, sMetal, sal_False );

            Color aSpecularCol( 225,225,225 );
            if ( bMetal )
            {
                aSpecularCol = Color( 200, 200, 200 );
                fSpecular += 0.15;
            }
            sal_Int32 nIntensity = (sal_Int32)fSpecular * 100;
            if ( nIntensity > 100 )
                nIntensity = 100;
            else if ( nIntensity < 0 )
                nIntensity = 0;
            nIntensity = 100 - nIntensity;
            pScene->GetProperties().SetObjectItem( Svx3DMaterialSpecularItem( aSpecularCol ) );
            pScene->GetProperties().SetObjectItem( Svx3DMaterialSpecularIntensityItem( (sal_uInt16)nIntensity ) );

            pScene->SetModel( pModel );
            pScene->InitTransformationSet();
            pRet->SetSnapRect( CalculateNewSnapRect( pCustomShape, aBoundRect2d, pMap ) );

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
    }
    return pRet;
}

Rectangle EnhancedCustomShape3d::CalculateNewSnapRect( const SdrObject* pCustomShape, const Rectangle& rBoundRect, const double* pMap )
{
    SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)pCustomShape->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
    const Point aCenter( pCustomShape->GetSnapRect().Center() );

    const rtl::OUString sBackwardDepth( RTL_CONSTASCII_USTRINGPARAM ( "BackwardDepth" ) );
    const rtl::OUString sForewardDepth( RTL_CONSTASCII_USTRINGPARAM ( "ForewardDepth" ) );
    double fExtrusionBackward = GetDouble( rGeometryItem, sBackwardDepth, 1270, pMap );
    double fExtrusionForward  = GetDouble( rGeometryItem, sForewardDepth, 0, pMap );
    sal_uInt16 i;

    // creating initial bound volume ( without rotation. skewing.and camera )
    Polygon3D aBoundVolume( 8 );
    const Polygon aPolygon( rBoundRect );
    for ( i = 0; i < 4; i++ )
    {
        aBoundVolume[ i ].X() = aPolygon[ i ].X() - aCenter.X();
        aBoundVolume[ i ].Y() = aPolygon[ i ].Y() - aCenter.Y();
        aBoundVolume[ i ].Z()  = fExtrusionForward;
        aBoundVolume[ i + 4 ].X()  = aPolygon[ i ].X() - aCenter.X();
        aBoundVolume[ i + 4 ].Y()  = aPolygon[ i ].Y() - aCenter.Y();
        aBoundVolume[ i + 4 ].Z()  = fExtrusionBackward;
    }

    const rtl::OUString sRotationCenterX( RTL_CONSTASCII_USTRINGPARAM ( "RotationCenterX" ) );
    const rtl::OUString sRotationCenterY( RTL_CONSTASCII_USTRINGPARAM ( "RotationCenterY" ) );
    const rtl::OUString sRotationCenterZ( RTL_CONSTASCII_USTRINGPARAM ( "RotationCenterZ" ) );

    drawing::Position3D aRotationCenter( 0, 0, 0 ); // default seems to be wrong, a fractional size of shape has to be used!!
    aRotationCenter.PositionX = GetDouble( rGeometryItem, sRotationCenterX, 0.0, NULL );
    aRotationCenter.PositionY = GetDouble( rGeometryItem, sRotationCenterY, 0.0, NULL );
    aRotationCenter.PositionZ = GetDouble( rGeometryItem, sRotationCenterZ, 0.0, NULL );

    // double XCenterInGUnits = rPropSet.GetPropertyValue( DFF_Prop_c3DRotationCenterX, 0 );
    // double YCenterInGUnits = rPropSet.GetPropertyValue( DFF_Prop_c3DRotationCenterY, 0 );

    // sal_Int32 nRotationXAxisInProz = rPropSet.GetPropertyValue( DFF_Prop_c3DRotationAxisX, 100 );
    // sal_Int32 nRotationYAxisInProz = rPropSet.GetPropertyValue( DFF_Prop_c3DRotationAxisY, 0 );
    // sal_Int32 nRotationZAxisInProz = rPropSet.GetPropertyValue( DFF_Prop_c3DRotationAxisZ, 0 );


    const rtl::OUString sAngleX( RTL_CONSTASCII_USTRINGPARAM ( "AngleX" ) );
    const rtl::OUString sAngleY( RTL_CONSTASCII_USTRINGPARAM ( "AngleY" ) );
    double fXRotate = GetDouble( rGeometryItem, sAngleX, 0, NULL ) * F_PI180;
    double fYRotate = GetDouble( rGeometryItem, sAngleY, 0, NULL ) * F_PI180;
    double fZRotate = -(360.0-(double)pCustomShape->GetRotateAngle()/100.0) * F_PI180;
    // double fRotationAngle = Fix16ToAngle( rPropSet.GetPropertyValue( DFF_Prop_c3DRotationAngle, 0 ) );       // * F_PI180;

    for( i = 0; i < 8; i++ )        // rotating bound volume
    {
        Vector3D& rPoint = aBoundVolume[ i ];
        rPoint.X() -= aRotationCenter.PositionX;
        rPoint.Y() -= aRotationCenter.PositionY;
        rPoint.Z() -= aRotationCenter.PositionZ;
        Rotate( rPoint, fXRotate, fYRotate, fZRotate );
        rPoint.X() += aRotationCenter.PositionX;
        rPoint.Y() += aRotationCenter.PositionY;
        rPoint.Z() += aRotationCenter.PositionZ;
    }
    Transformation2D aTransformation2D( pCustomShape, rBoundRect, pMap );
    if ( aTransformation2D.IsParallel() )
        aTransformation2D.ApplySkewSettings( aBoundVolume );

    Polygon aTransformed( 8 );
    for ( i = 0; i < 8; i++ )
        aTransformed[ i ] = aTransformation2D.Transform2D( aBoundVolume[ i ] );
    return aTransformed.GetBoundRect();
}

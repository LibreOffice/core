/*************************************************************************
 *
 *  $RCSfile: EnhancedCustomShapeEngine.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: pjunck $ $Date: 2004-11-03 10:34:02 $
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

#ifndef _ENHANCED_CUSTOMSHAPE_ENGINE_HXX
#include "EnhancedCustomShapeEngine.hxx"
#endif
#ifndef _ENHANCEDCUSTOMSHAPE2D_HXX
#include "EnhancedCustomShape2d.hxx"
#endif
#ifndef _ENHANCEDCUSTOMSHAPE3D_HXX
#include "EnhancedCustomShape3d.hxx"
#endif
#ifndef _ENHANCEDCUSTOMSHAPEFONTWORK_HXX
#include "EnhancedCustomShapeFontWork.hxx"
#endif
#ifndef _ENHANCED_CUSTOMSHAPE_HANDLE_HXX
#include "EnhancedCustomShapeHandle.hxx"
#endif
#ifndef _ENHANCEDCUSTOMSHAPEGEOMETRY_HXX
#include "EnhancedCustomShapeGeometry.hxx"
#endif
#ifndef _SVX_UNOSHAPE_HXX
#include "unoshape.hxx"
#endif
#ifndef _SVX_UNOPAGE_HXX
#include "unopage.hxx"
#endif
#ifndef _SVX_UNOAPI_HXX_
#include "unoapi.hxx"
#endif
#ifndef _SVDOBJ_HXX
#include "svdobj.hxx"
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef _SVDOPATH_HXX
#include "svdopath.hxx"
#endif
#ifndef _XOUTX_HXX
#include "xoutx.hxx"
#endif
#ifndef _SVDITER_HXX
#include "svditer.hxx"
#endif
#ifndef _SVX_UNOPOLYHELPER_HXX
#include "unopolyhelper.hxx"
#endif
#include <uno/mapping.hxx>

// ---------------------------
// - EnhancedCustomShapeEngine -
// ---------------------------

rtl::OUString EnhancedCustomShapeEngine_getImplementationName()
    throw( NMSP_UNO::RuntimeException )
{
    return B2UCONST( "com.sun.star.drawing.EnhancedCustomShapeEngine" );
}
sal_Bool SAL_CALL EnhancedCustomShapeEngine_supportsService( const rtl::OUString& ServiceName )
    throw( NMSP_UNO::RuntimeException )
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "drafts.com.sun.star.drawing.CustomShapeEngine" ) );
}
SEQ( rtl::OUString ) SAL_CALL EnhancedCustomShapeEngine_getSupportedServiceNames()
    throw( NMSP_UNO::RuntimeException )
{
    SEQ( rtl::OUString ) aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = B2UCONST( "drafts.com.sun.star.drawing.CustomShapeEngine" );
    return aRet;
}

// -----------------------------------------------------------------------------

EnhancedCustomShapeEngine::EnhancedCustomShapeEngine( const REF( NMSP_LANG::XMultiServiceFactory )& rxMgr ) :
    mxFact      ( rxMgr )
{
}
EnhancedCustomShapeEngine::~EnhancedCustomShapeEngine()
{
}

// XInterface -----------------------------------------------------------------

void SAL_CALL EnhancedCustomShapeEngine::acquire() throw()
{
    OWeakObject::acquire();
}
void SAL_CALL EnhancedCustomShapeEngine::release() throw()
{
    OWeakObject::release();
}

// XInitialization ------------------------------------------------------------

void SAL_CALL EnhancedCustomShapeEngine::initialize( const SEQ( NMSP_UNO::Any )& aArguments )
    throw ( NMSP_UNO::Exception, NMSP_UNO::RuntimeException )
{
    sal_Int32 i;
    SEQ( NMSP_BEANS::PropertyValue ) aParameter;
    for ( i = 0; i < aArguments.getLength(); i++ )
    {
        if ( aArguments[ i ] >>= aParameter )
            break;
    }
    for ( i = 0; i < aParameter.getLength(); i++ )
    {
        const NMSP_BEANS::PropertyValue& rProp = aParameter[ i ];
        if ( rProp.Name.equalsAscii( "CustomShape" ) )
            rProp.Value >>= mxShape;
    }
}

// XServiceInfo ---------------------------------------------------------------

rtl::OUString SAL_CALL EnhancedCustomShapeEngine::getImplementationName()
    throw( NMSP_UNO::RuntimeException )
{
    return EnhancedCustomShapeEngine_getImplementationName();
}
sal_Bool SAL_CALL EnhancedCustomShapeEngine::supportsService( const rtl::OUString& rServiceName )
    throw( NMSP_UNO::RuntimeException )
{
    return EnhancedCustomShapeEngine_supportsService( rServiceName );
}
SEQ( rtl::OUString ) SAL_CALL EnhancedCustomShapeEngine::getSupportedServiceNames()
    throw ( NMSP_UNO::RuntimeException )
{
    return EnhancedCustomShapeEngine_getSupportedServiceNames();
}

// XCustomShapeEngine -----------------------------------------------------------

REF( com::sun::star::drawing::XShape ) SAL_CALL EnhancedCustomShapeEngine::render()
    throw ( NMSP_UNO::RuntimeException )
{
    REF( com::sun::star::drawing::XShape ) xShape;
    SdrObject* pSdrObjCustomShape( GetSdrObjectFromXShape( mxShape ) );
    if ( pSdrObjCustomShape )
    {
        // retrieving the TextPath property to check if feature is enabled
        SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)
            pSdrObjCustomShape->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
        sal_Bool bTextPathOn = sal_False;
        const rtl::OUString sTextPath( RTL_CONSTASCII_USTRINGPARAM ( "TextPath" ) );
        com::sun::star::uno::Any* pAny = rGeometryItem.GetPropertyValueByName( sTextPath, sTextPath );
        if ( pAny )
            *pAny >>= bTextPathOn;

        EnhancedCustomShape2d aCustomShape2d( pSdrObjCustomShape );
        sal_Int32 nRotateAngle = aCustomShape2d.GetRotateAngle();

        sal_Bool bFlipV = aCustomShape2d.IsFlipVert();
        sal_Bool bFlipH = aCustomShape2d.IsFlipHorz();
        sal_Bool bLineGeometryNeededOnly = bTextPathOn;

        SdrObject* pRenderedShape = aCustomShape2d.CreateObject( bLineGeometryNeededOnly );
        if ( pRenderedShape )
        {
            if ( bTextPathOn )
            {
                SdrObject* pRenderedFontWork = EnhancedCustomShapeFontWork::CreateFontWork( pRenderedShape, pSdrObjCustomShape );
                if ( pRenderedFontWork )
                    delete pRenderedShape, pRenderedShape = pRenderedFontWork;
            }
            SdrObject* pRenderedShape3d = EnhancedCustomShape3d::Create3DObject( pRenderedShape, pSdrObjCustomShape );
            if ( pRenderedShape3d )
            {
                bFlipV = bFlipH = sal_False;
                nRotateAngle = 0;
                delete pRenderedShape;
                pRenderedShape = pRenderedShape3d;
            }
            Rectangle aRect( pSdrObjCustomShape->GetSnapRect() );
            if( nRotateAngle )
            {
                double a = nRotateAngle * F_PI18000;
                pRenderedShape->NbcRotate( pSdrObjCustomShape->GetSnapRect().Center(), nRotateAngle, sin( a ), cos( a ) );
            }
            if ( bFlipV )
            {
                Point aLeft( aRect.Left(), ( aRect.Top() + aRect.Bottom() ) >> 1 );
                Point aRight( aLeft.X() + 1000, aLeft.Y() );
                pRenderedShape->NbcMirror( aLeft, aRight );
            }
            if ( bFlipH )
            {
                Point aTop( ( aRect.Left() + aRect.Right() ) >> 1, aRect.Top() );
                Point aBottom( aTop.X(), aTop.Y() + 1000 );
                pRenderedShape->NbcMirror( aTop, aBottom );
            }
            pRenderedShape->NbcSetStyleSheet( pSdrObjCustomShape->GetStyleSheet(), sal_True );
            pRenderedShape->RecalcSnapRect();
            xShape = SvxDrawPage::CreateShapeByTypeAndInventor( pRenderedShape->GetObjIdentifier(),
                pRenderedShape->GetObjInventor(), pRenderedShape, NULL );
        }
    }
    return xShape;
}

com::sun::star::awt::Rectangle SAL_CALL EnhancedCustomShapeEngine::getTextBounds()
    throw ( NMSP_UNO::RuntimeException )
{
    com::sun::star::awt::Rectangle aTextRect;
    SdrObject* pSdrObjCustomShape( GetSdrObjectFromXShape( mxShape ) );
    if ( pSdrObjCustomShape )
    {
        EnhancedCustomShape2d aCustomShape2d( pSdrObjCustomShape );
        Rectangle aRect( aCustomShape2d.GetTextRect() );
        aTextRect.X = aRect.Left();
        aTextRect.Y = aRect.Top();
        aTextRect.Width = aRect.GetWidth();
        aTextRect.Height = aRect.GetHeight();
    }
    return aTextRect;
}

com::sun::star::drawing::PolyPolygonBezierCoords SAL_CALL EnhancedCustomShapeEngine::getLineGeometry()
    throw ( NMSP_UNO::RuntimeException )
{
    com::sun::star::drawing::PolyPolygonBezierCoords aPolyPolygonBezierCoords;
    SdrObject* pSdrObjCustomShape( GetSdrObjectFromXShape( mxShape ) );
    if ( pSdrObjCustomShape )
    {
        EnhancedCustomShape2d aCustomShape2d( pSdrObjCustomShape );
        SdrObject* pObj = aCustomShape2d.CreateLineGeometry();
        if ( pObj )
        {
            Rectangle aRect( pSdrObjCustomShape->GetSnapRect() );
            sal_Int32 nRotateAngle = aCustomShape2d.GetRotateAngle();
            if( nRotateAngle )
            {
                double a = nRotateAngle * F_PI18000;
                pObj->NbcRotate( aRect.Center(), nRotateAngle, sin( a ), cos( a ) );
            }
            if ( aCustomShape2d.IsFlipHorz() )
            {
                Point aTop( ( aRect.Left() + aRect.Right() ) >> 1, aRect.Top() );
                Point aBottom( aTop.X(), aTop.Y() + 1000 );
                pObj->NbcMirror( aTop, aBottom );
            }
            if ( aCustomShape2d.IsFlipVert() )
            {
                Point aLeft( aRect.Left(), ( aRect.Top() + aRect.Bottom() ) >> 1 );
                Point aRight( aLeft.X() + 1000, aLeft.Y() );
                pObj->NbcMirror( aLeft, aRight );
            }

            XPolyPolygon aXPolyPoly;
            SdrObjListIter aIter( *pObj, IM_DEEPWITHGROUPS );
            while ( aIter.IsMore() )
            {
                SdrObject* pNewObj = NULL;
                const XPolyPolygon* pPP = NULL;
                const SdrObject* pNext = aIter.Next();

                if ( pNext->ISA( SdrPathObj ) )
                    pPP = &((SdrPathObj*)pNext)->GetPathPoly();
                else
                {
                    pNewObj = pNext->ConvertToPolyObj( FALSE, FALSE );
                    SdrPathObj* pPath = PTR_CAST( SdrPathObj, pNewObj );
                    if ( pPath )
                        pPP = &pPath->GetPathPoly();
                }
                if ( pPP )
                {
                    sal_uInt16 i;
                    for ( i = 0; i < pPP->Count(); i++ )
                        aXPolyPoly.Insert( XOutCreatePolygon( pPP->GetObject( i )), XPOLYPOLY_APPEND );
//BFS09                     aXPolyPoly.Insert( XOutCreatePolygon( pPP->GetObject( i ), NULL, 100 ), XPOLYPOLY_APPEND );
                }
                delete pNewObj;
            }
            delete pObj;
            SvxPolyPolygonToPolyPolygonBezierCoords( aXPolyPoly, aPolyPolygonBezierCoords );
        }
    }
    return aPolyPolygonBezierCoords;
}

SEQ( REF( drafts::com::sun::star::drawing::XCustomShapeHandle ) ) SAL_CALL EnhancedCustomShapeEngine::getInteraction()
    throw ( NMSP_UNO::RuntimeException )
{
    sal_uInt32 i, nHdlCount = 0;
    SdrObject* pSdrObjCustomShape = GetSdrObjectFromXShape( mxShape );
    if ( pSdrObjCustomShape )
    {
        EnhancedCustomShape2d aCustomShape2d( pSdrObjCustomShape );
        nHdlCount = aCustomShape2d.GetHdlCount();
    }
    SEQ( REF( drafts::com::sun::star::drawing::XCustomShapeHandle ) ) aSeq( nHdlCount );
    for ( i = 0; i < nHdlCount; i++ )
        aSeq[ i ] = new EnhancedCustomShapeHandle( mxShape, i );
    return aSeq;
}

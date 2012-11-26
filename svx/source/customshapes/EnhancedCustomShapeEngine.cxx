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
#include "EnhancedCustomShapeEngine.hxx"
#include "svx/EnhancedCustomShape2d.hxx"
#include "EnhancedCustomShape3d.hxx"
#include "EnhancedCustomShapeFontWork.hxx"
#include "EnhancedCustomShapeHandle.hxx"
#include "svx/EnhancedCustomShapeGeometry.hxx"
#include <svx/unoshape.hxx>
#include <svx/unopage.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdorect.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/outliner.hxx>
#include <svx/svdoutl.hxx>
#include <svl/itemset.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svditer.hxx>
#include <unopolyhelper.hxx>
#include <uno/mapping.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <com/sun/star/document/XActionLockable.hpp>
#include <svx/svdlegacy.hxx>
#include <svx/sdrobjectfactory.hxx>

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
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.drawing.CustomShapeEngine" ) );
}
SEQ( rtl::OUString ) SAL_CALL EnhancedCustomShapeEngine_getSupportedServiceNames()
    throw( NMSP_UNO::RuntimeException )
{
    SEQ( rtl::OUString ) aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = B2UCONST( "com.sun.star.drawing.CustomShapeEngine" );
    return aRet;
}

// -----------------------------------------------------------------------------

EnhancedCustomShapeEngine::EnhancedCustomShapeEngine( const REF( NMSP_LANG::XMultiServiceFactory )& rxMgr ) :
    mxFact                  ( rxMgr ),
    mbForceGroupWithText    ( sal_False )
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
        else if ( rProp.Name.equalsAscii( "ForceGroupWithText" ) )
            rProp.Value >>= mbForceGroupWithText;
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

SdrObject* EnhancedCustomShapeEngine::ImplForceGroupWithText( const SdrObjCustomShape* pCustoObj, SdrObject* pRenderedShape )
{
    bool bHasText = pCustoObj->HasText();
    if ( pRenderedShape || bHasText )
    {
        // applying shadow
        const SdrObject* pShadowGeometry = pCustoObj->GetSdrObjectShadowFromCustomShape();
        if ( pShadowGeometry )
        {
            if ( pRenderedShape )
            {
                SdrObjGroup* pSdrObjGroup = dynamic_cast< SdrObjGroup* >(pRenderedShape);

                if ( !pSdrObjGroup )
                {
                    pSdrObjGroup = new SdrObjGroup(pCustoObj->getSdrModelFromSdrObject());
                    pSdrObjGroup->InsertObjectToSdrObjList(*pRenderedShape);
                    pRenderedShape = pSdrObjGroup;
                }

                pSdrObjGroup->InsertObjectToSdrObjList(*pShadowGeometry->CloneSdrObject(), 0);
            }
            else
            {
                pRenderedShape = pShadowGeometry->CloneSdrObject();
            }
        }

        // apply text
        if ( bHasText )
        {
            // #i37011# also create a text object and add at rPos + 1
            SdrTextObj* pTextObj = (SdrTextObj*)SdrObjFactory::MakeNewObject(
                pCustoObj->getSdrModelFromSdrObject(),
                SdrObjectCreationInfo(OBJ_TEXT, pCustoObj->GetObjInventor()));

            // Copy text content
            OutlinerParaObject* pParaObj = pCustoObj->GetOutlinerParaObject();
            if( pParaObj )
                pTextObj->SetOutlinerParaObject( new OutlinerParaObject(*pParaObj) );

            // copy all attributes
            SfxItemSet aTargetItemSet( pCustoObj->GetMergedItemSet() );

            // clear fill and line style
            aTargetItemSet.Put(XLineStyleItem(XLINE_NONE));
            aTargetItemSet.Put(XFillStyleItem(XFILL_NONE));

            // get the text bounds and set at text object
            basegfx::B2DRange aTextBounds(sdr::legacy::GetLogicRange(*pCustoObj));
            SdrObject* pSdrObjCustomShape( GetSdrObjectFromXShape( mxShape ) );
            if ( pSdrObjCustomShape )
            {
                EnhancedCustomShape2d aCustomShape2d( pSdrObjCustomShape );
                aTextBounds = aCustomShape2d.GetTextRange();
            }
            sdr::legacy::SetLogicRange(*pTextObj, aTextBounds );

            // if rotated, copy rotation, too.
            const long aOldRotation(sdr::legacy::GetRotateAngle(*pCustoObj));
            if ( aOldRotation )
            {
                sdr::legacy::RotateSdrObject(*pTextObj,
                    sdr::legacy::GetSnapRect(*pCustoObj).Center(),
                    aOldRotation);
            }

            // set modified ItemSet at text object
            pTextObj->SetMergedItemSet(aTargetItemSet);

            if ( pRenderedShape )
            {
                SdrObjGroup* pSdrObjGroup = dynamic_cast< SdrObjGroup* >(pRenderedShape);

                if ( !pSdrObjGroup )
                {
                    pSdrObjGroup = new SdrObjGroup(pCustoObj->getSdrModelFromSdrObject());
                    pSdrObjGroup->InsertObjectToSdrObjList(*pRenderedShape);
                    pRenderedShape = pSdrObjGroup;
                }

                pSdrObjGroup->InsertObjectToSdrObjList(*pTextObj);
            }
            else
            {
                pRenderedShape = pTextObj;
        }
        }

        // force group
        if ( pRenderedShape )
        {
            SdrObjGroup* pSdrObjGroup = dynamic_cast< SdrObjGroup* >(pRenderedShape);

            if ( !pSdrObjGroup )
            {
                pSdrObjGroup = new SdrObjGroup(pCustoObj->getSdrModelFromSdrObject());
                pSdrObjGroup->InsertObjectToSdrObjList(*pRenderedShape);
                pRenderedShape = pSdrObjGroup;
            }

            //pRenderedShape->SetPage( pCustoObj->GetPage() );
            //pRenderedShape->SetModel( pCustoObj->GetModel() );
        }
    }
    return pRenderedShape;
}

void SetTemporary( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape )
{
    if ( xShape.is() )
    {
        SvxShape* pShape = SvxShape::getImplementation( xShape );
        if ( pShape )
            pShape->TakeSdrObjectOwnership();
/*
        ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XShapes > xShapes( xShape, ::com::sun::star::uno::UNO_QUERY );
        if ( xShapes.is() )
        {
            sal_Int32 i;
            for ( i = 0; i < xShapes->getCount(); i++ )
            {
                ::com::sun::star::uno::Any aAny( xShapes->getByIndex( i ) );
                ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape;
                if ( aAny >>= xShape )
                    SetTemporary( xShape );
            }
        }
*/
    }
}

REF( com::sun::star::drawing::XShape ) SAL_CALL EnhancedCustomShapeEngine::render()
    throw ( NMSP_UNO::RuntimeException )
{
    REF( com::sun::star::drawing::XShape ) xShape;
    SdrObjCustomShape* pSdrObjCustomShape(dynamic_cast< SdrObjCustomShape* >(GetSdrObjectFromXShape(mxShape)));

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
//      sal_Int32 nRotateAngle = aCustomShape2d.GetRotateAngle();

//      sal_Bool bFlipV = aCustomShape2d.IsFlipVert();
//      sal_Bool bFlipH = aCustomShape2d.IsFlipHorz();
        sal_Bool bLineGeometryNeededOnly = bTextPathOn;

        SdrObject* pRenderedShape = aCustomShape2d.CreateObject( bLineGeometryNeededOnly );
        if ( pRenderedShape )
        {
            if ( bTextPathOn )
            {
                SdrObject* pRenderedFontWork = EnhancedCustomShapeFontWork::CreateFontWork( pRenderedShape, pSdrObjCustomShape );
                if ( pRenderedFontWork )
                {
                    deleteSdrObjectSafeAndClearPointer( pRenderedShape );
                    pRenderedShape = pRenderedFontWork;
                }
            }
            SdrObject* pRenderedShape3d = EnhancedCustomShape3d::Create3DObject(*pRenderedShape, *pSdrObjCustomShape);
            if ( pRenderedShape3d )
            {
//              bFlipV = bFlipH = sal_False;
//              nRotateAngle = 0;
                deleteSdrObjectSafeAndClearPointer( pRenderedShape );
                pRenderedShape = pRenderedShape3d;
            }

            // copy original transformation
            pRenderedShape->setSdrObjectTransformation(pSdrObjCustomShape->getSdrObjectTransformation());

//          const Rectangle aRect( sdr::legacy::GetSnapRect(*pSdrObjCustomShape) );
//          long aOldShear(sdr::legacy::GetShearAngleX(*pSdrObjCustomShape));
//
//          if ( aOldShear )
//          {
//              if ((bFlipV&&!bFlipH )||(bFlipH&&!bFlipV))
//              {
//                  aOldShear = -aOldShear;
//              }
//
//              sdr::legacy::ShearSdrObject(*pRenderedShape, aRect.Center(), aOldShear, false);
//          }
//
//          if( nRotateAngle )
//          {
//              sdr::legacy::RotateSdrObject(*pRenderedShape, aRect.Center(), nRotateAngle);
//          }
//
//          if ( bFlipV )
//          {
//              Point aLeft( aRect.Left(), ( aRect.Top() + aRect.Bottom() ) >> 1 );
//              Point aRight( aLeft.X() + 1000, aLeft.Y() );
//              sdr::legacy::MirrorSdrObject(*pRenderedShape, aLeft, aRight );
//          }
//
//          if ( bFlipH )
//          {
//              Point aTop( ( aRect.Left() + aRect.Right() ) >> 1, aRect.Top() );
//              Point aBottom( aTop.X(), aTop.Y() + 1000 );
//              sdr::legacy::MirrorSdrObject(*pRenderedShape, aTop, aBottom );
//          }

            pRenderedShape->SetStyleSheet(pSdrObjCustomShape->GetStyleSheet(), true);
        }

        if ( mbForceGroupWithText )
            pRenderedShape = ImplForceGroupWithText( (SdrObjCustomShape*)pSdrObjCustomShape, pRenderedShape );

        if ( pRenderedShape )
        {
            aCustomShape2d.ApplyGluePoints( pRenderedShape );
            xShape = SvxDrawPage::CreateShapeBySvxShapeKind(
                SdrObjectCreatorInventorToSvxShapeKind(pRenderedShape->GetObjIdentifier(), pRenderedShape->GetObjInventor()),
                pRenderedShape,
                0);
        }
        SetTemporary( xShape );
    }
    return xShape;
}

com::sun::star::awt::Rectangle SAL_CALL EnhancedCustomShapeEngine::getTextBounds()
    throw ( NMSP_UNO::RuntimeException )
{
    com::sun::star::awt::Rectangle aTextRect;
    SdrObject* pSdrObjCustomShape( GetSdrObjectFromXShape( mxShape ) );
    ::com::sun::star::uno::Reference< ::com::sun::star::document::XActionLockable > xLockable( mxShape, ::com::sun::star::uno::UNO_QUERY );
    if ( pSdrObjCustomShape && xLockable.is() && !xLockable->isActionLocked() )
    {
        if ( pSdrObjCustomShape )
        {
            EnhancedCustomShape2d aCustomShape2d( pSdrObjCustomShape );
            const basegfx::B2DRange aRange( aCustomShape2d.GetTextRange() );
            aTextRect.X = basegfx::fround(aRange.getMinX());
            aTextRect.Y = basegfx::fround(aRange.getMinY());
            aTextRect.Width = basegfx::fround(aRange.getWidth());
            aTextRect.Height = basegfx::fround(aRange.getHeight());
        }
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
//          const Rectangle aRect( sdr::legacy::GetSnapRect(*pSdrObjCustomShape) );
//          const sal_Bool bFlipV = aCustomShape2d.IsFlipVert();
//          const sal_Bool bFlipH = aCustomShape2d.IsFlipHorz();

            // copy original transformation
            pObj->setSdrObjectTransformation(pSdrObjCustomShape->getSdrObjectTransformation());

//          long aOldShear(sdr::legacy::GetShearAngleX(*pSdrObjCustomShape));
//          if ( aOldShear )
//          {
//              if ((bFlipV&&!bFlipH )||(bFlipH&&!bFlipV))
//              {
//                  aOldShear = -aOldShear;
//              }
//
//              sdr::legacy::ShearSdrObject(*pObj, aRect.Center(), aOldShear, false);
//          }
//          sal_Int32 nRotateAngle = aCustomShape2d.GetRotateAngle();
//          if( nRotateAngle )
//          {
//              sdr::legacy::RotateSdrObject(*pObj, aRect.Center(), nRotateAngle );
//          }
//          if ( bFlipH )
//          {
//              Point aTop( ( aRect.Left() + aRect.Right() ) >> 1, aRect.Top() );
//              Point aBottom( aTop.X(), aTop.Y() + 1000 );
//              sdr::legacy::MirrorSdrObject(*pObj, aTop, aBottom );
//          }
//          if ( bFlipV )
//          {
//              Point aLeft( aRect.Left(), ( aRect.Top() + aRect.Bottom() ) >> 1 );
//              Point aRight( aLeft.X() + 1000, aLeft.Y() );
//              sdr::legacy::MirrorSdrObject(*pObj, aLeft, aRight );
//          }

            basegfx::B2DPolyPolygon aPolyPolygon;
            SdrObjListIter aIter( *pObj, IM_DEEPWITHGROUPS );

            while ( aIter.IsMore() )
            {
                SdrObject* pNewObj = NULL;
                basegfx::B2DPolyPolygon aPP;
                const SdrObject* pNext = aIter.Next();
                const SdrPathObj* pSdrPathObj = dynamic_cast< const SdrPathObj* >(pNext);

                if ( pSdrPathObj )
                {
                    aPP = pSdrPathObj->getB2DPolyPolygonInObjectCoordinates();
                }
                else
                {
                    pNewObj = pNext->ConvertToPolyObj( false, false);
                    SdrPathObj* pPath = dynamic_cast< SdrPathObj* >(pNewObj);
                    if ( pPath )
                        aPP = pPath->getB2DPolyPolygonInObjectCoordinates();
                }

                if ( aPP.count() )
                    aPolyPolygon.append(aPP);

                deleteSdrObjectSafeAndClearPointer( pNewObj );
            }
            deleteSdrObjectSafeAndClearPointer( pObj );
            basegfx::tools::B2DPolyPolygonToUnoPolyPolygonBezierCoords(aPolyPolygon, aPolyPolygonBezierCoords);
        }
    }

    return aPolyPolygonBezierCoords;
}

SEQ( REF( com::sun::star::drawing::XCustomShapeHandle ) ) SAL_CALL EnhancedCustomShapeEngine::getInteraction()
    throw ( NMSP_UNO::RuntimeException )
{
    sal_uInt32 i, nHdlCount = 0;
    SdrObject* pSdrObjCustomShape = GetSdrObjectFromXShape( mxShape );
    if ( pSdrObjCustomShape )
    {
        EnhancedCustomShape2d aCustomShape2d( pSdrObjCustomShape );
        nHdlCount = aCustomShape2d.GetHdlCount();
    }
    SEQ( REF( com::sun::star::drawing::XCustomShapeHandle ) ) aSeq( nHdlCount );
    for ( i = 0; i < nHdlCount; i++ )
        aSeq[ i ] = new EnhancedCustomShapeHandle( mxShape, i );
    return aSeq;
}

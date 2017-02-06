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

#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/DoubleSequence.hpp>
#include <com/sun/star/drawing/CameraGeometry.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/serviceinfohelper.hxx>

#include <svx/svdpool.hxx>
#include <svx/svditer.hxx>
#include <svx/unoshape.hxx>
#include <svx/unopage.hxx>
#include <editeng/unoprnms.hxx>
#include <svx/polysc3d.hxx>
#include "svx/globl3d.hxx"
#include <svx/cube3d.hxx>
#include <svx/sphere3d.hxx>
#include <svx/lathe3d.hxx>
#include <svx/extrud3d.hxx>
#include <svx/polygn3d.hxx>
#include "svx/unoshprp.hxx"
#include "svx/svdmodel.hxx"
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/polygon/b3dpolygontools.hxx>
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include "shapeimpl.hxx"

using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

#define QUERYINT( xint ) \
    if( rType == cppu::UnoType<xint>::get() ) \
        aAny <<= Reference< xint >(this)

Svx3DSceneObject::Svx3DSceneObject( SdrObject* pObj, SvxDrawPage* pDrawPage ) throw()
:   SvxShape( pObj, getSvxMapProvider().GetMap(SVXMAP_3DSCENEOBJECT), getSvxMapProvider().GetPropertySet(SVXMAP_3DSCENEOBJECT, SdrObject::GetGlobalDrawObjectItemPool()) )
,   mxPage( pDrawPage )
{
}


Svx3DSceneObject::~Svx3DSceneObject() throw()
{
}


void Svx3DSceneObject::Create( SdrObject* pNewObj, SvxDrawPage* pNewPage )
{
    SvxShape::Create( pNewObj, pNewPage );
    mxPage = pNewPage;
}


uno::Any SAL_CALL Svx3DSceneObject::queryAggregation( const uno::Type & rType )
{
    uno::Any aAny;

    QUERYINT( drawing::XShapes );
    else QUERYINT( container::XIndexAccess );
    else QUERYINT( container::XElementAccess );
    else
        return SvxShape::queryAggregation( rType );

    return aAny;
}

uno::Any SAL_CALL Svx3DSceneObject::queryInterface( const uno::Type & rType )
{
    return SvxShape::queryInterface( rType );
}

void SAL_CALL Svx3DSceneObject::acquire() throw ( )
{
    SvxShape::acquire();
}

void SAL_CALL Svx3DSceneObject::release() throw ( )
{
    SvxShape::release();
}

// XTypeProvider

uno::Sequence< sal_Int8 > SAL_CALL Svx3DSceneObject::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}


void SAL_CALL Svx3DSceneObject::add( const Reference< drawing::XShape >& xShape )
{
    SolarMutexGuard aGuard;

    SvxShape* pShape = SvxShape::getImplementation( xShape );

    if(!mpObj.is() || !mxPage.is() || pShape == nullptr || nullptr != pShape->GetSdrObject() )
        throw uno::RuntimeException();

    SdrObject* pSdrShape = mxPage->CreateSdrObject_( xShape );
    if( dynamic_cast<const E3dObject* >(pSdrShape) !=  nullptr )
    {
        mpObj->GetSubList()->NbcInsertObject( pSdrShape );

        if(pShape)
            pShape->Create( pSdrShape, mxPage.get()  );
    }
    else
    {
        SdrObject::Free( pSdrShape );
        throw uno::RuntimeException();
    }

    if( mpModel )
        mpModel->SetChanged();
}


void SAL_CALL Svx3DSceneObject::remove( const Reference< drawing::XShape >& xShape )
{
    SolarMutexGuard aGuard;

    SvxShape* pShape = SvxShape::getImplementation( xShape );

    if(!mpObj.is() || pShape == nullptr)
        throw uno::RuntimeException();

    SdrObject* pSdrShape = pShape->GetSdrObject();
    if(pSdrShape == nullptr || pSdrShape->GetObjList()->GetOwnerObj() != mpObj.get())
    {
        throw uno::RuntimeException();
    }
    else
    {
        SdrObjList& rList = *pSdrShape->GetObjList();

        const size_t nObjCount = rList.GetObjCount();
        size_t nObjNum = 0;
        while( nObjNum < nObjCount )
        {
            if(rList.GetObj( nObjNum ) == pSdrShape )
                break;
            nObjNum++;
        }

        if( nObjNum < nObjCount )
        {
            SdrObject* pObject = rList.NbcRemoveObject( nObjNum );
            SdrObject::Free( pObject );
        }
        else
        {
            SAL_WARN( "svx", "Fatality! SdrObject is not belonging to its SdrObjList! [CL]" );
        }
    }
}


sal_Int32 SAL_CALL Svx3DSceneObject::getCount()
{
    SolarMutexGuard aGuard;

    sal_Int32 nRetval = 0;

    if(mpObj.is() && dynamic_cast<const E3dPolyScene* >(mpObj.get()) != nullptr && mpObj->GetSubList())
        nRetval = mpObj->GetSubList()->GetObjCount();
    return nRetval;
}


uno::Any SAL_CALL Svx3DSceneObject::getByIndex( sal_Int32 Index )
{
    SolarMutexGuard aGuard;

    if( !mpObj.is() || mpObj->GetSubList() == nullptr )
        throw uno::RuntimeException();

    if( Index<0 || mpObj->GetSubList()->GetObjCount() <= static_cast<size_t>(Index) )
        throw lang::IndexOutOfBoundsException();

    SdrObject* pDestObj = mpObj->GetSubList()->GetObj( Index );
    if(pDestObj == nullptr)
        throw lang::IndexOutOfBoundsException();

    Reference< drawing::XShape > xShape( pDestObj->getUnoShape(), uno::UNO_QUERY );
    return uno::Any(xShape);
}


// css::container::XElementAccess

uno::Type SAL_CALL Svx3DSceneObject::getElementType()
{
    return cppu::UnoType<drawing::XShape>::get();
}


sal_Bool SAL_CALL Svx3DSceneObject::hasElements()
{
    SolarMutexGuard aGuard;

    return mpObj.is() && mpObj->GetSubList() && (mpObj->GetSubList()->GetObjCount() > 0);
}


static bool ConvertHomogenMatrixToObject( E3dObject* pObject, const Any& rValue )
{
    drawing::HomogenMatrix m;
    if( rValue >>= m )
    {
        basegfx::B3DHomMatrix aMat;
        aMat.set(0, 0, m.Line1.Column1);
        aMat.set(0, 1, m.Line1.Column2);
        aMat.set(0, 2, m.Line1.Column3);
        aMat.set(0, 3, m.Line1.Column4);
        aMat.set(1, 0, m.Line2.Column1);
        aMat.set(1, 1, m.Line2.Column2);
        aMat.set(1, 2, m.Line2.Column3);
        aMat.set(1, 3, m.Line2.Column4);
        aMat.set(2, 0, m.Line3.Column1);
        aMat.set(2, 1, m.Line3.Column2);
        aMat.set(2, 2, m.Line3.Column3);
        aMat.set(2, 3, m.Line3.Column4);
        aMat.set(3, 0, m.Line4.Column1);
        aMat.set(3, 1, m.Line4.Column2);
        aMat.set(3, 2, m.Line4.Column3);
        aMat.set(3, 3, m.Line4.Column4);
        pObject->SetTransform(aMat);
        return true;
    }
    return false;
}

static void ConvertObjectToHomogenMatric( E3dObject* pObject, Any& rValue )
{
    drawing::HomogenMatrix aHomMat;
    const basegfx::B3DHomMatrix& rMat = pObject->GetTransform();
    aHomMat.Line1.Column1 = rMat.get(0, 0);
    aHomMat.Line1.Column2 = rMat.get(0, 1);
    aHomMat.Line1.Column3 = rMat.get(0, 2);
    aHomMat.Line1.Column4 = rMat.get(0, 3);
    aHomMat.Line2.Column1 = rMat.get(1, 0);
    aHomMat.Line2.Column2 = rMat.get(1, 1);
    aHomMat.Line2.Column3 = rMat.get(1, 2);
    aHomMat.Line2.Column4 = rMat.get(1, 3);
    aHomMat.Line3.Column1 = rMat.get(2, 0);
    aHomMat.Line3.Column2 = rMat.get(2, 1);
    aHomMat.Line3.Column3 = rMat.get(2, 2);
    aHomMat.Line3.Column4 = rMat.get(2, 3);
    aHomMat.Line4.Column1 = rMat.get(3, 0);
    aHomMat.Line4.Column2 = rMat.get(3, 1);
    aHomMat.Line4.Column3 = rMat.get(3, 2);
    aHomMat.Line4.Column4 = rMat.get(3, 3);
    rValue <<= aHomMat;
}


struct ImpRememberTransAndRect
{
    basegfx::B3DHomMatrix                   maMat;
    Rectangle                   maRect;
};

bool Svx3DSceneObject::setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue )
{
    switch( pProperty->nWID )
    {
    case OWN_ATTR_3D_VALUE_TRANSFORM_MATRIX:
    {
        // pach transformation matrix to the object
        if( ConvertHomogenMatrixToObject( static_cast< E3dObject* >( mpObj.get() ), rValue ) )
            return true;
        break;
    }
    case OWN_ATTR_3D_VALUE_CAMERA_GEOMETRY:
    {
        // set CameraGeometry at scene
        E3dScene* pScene = static_cast< E3dScene* >( mpObj.get() );
        drawing::CameraGeometry aCamGeo;

        if(rValue >>= aCamGeo)
        {
            basegfx::B3DPoint aVRP(aCamGeo.vrp.PositionX, aCamGeo.vrp.PositionY, aCamGeo.vrp.PositionZ);
            basegfx::B3DVector aVPN(aCamGeo.vpn.DirectionX, aCamGeo.vpn.DirectionY, aCamGeo.vpn.DirectionZ);
            basegfx::B3DVector aVUP(aCamGeo.vup.DirectionX, aCamGeo.vup.DirectionY, aCamGeo.vup.DirectionZ);

            // rescue scene transformation
            ImpRememberTransAndRect aSceneTAR;
            aSceneTAR.maMat = pScene->GetTransform();
            aSceneTAR.maRect = pScene->GetSnapRect();

            // rescue object transformations
            SdrObjListIter aIter(*pScene->GetSubList(), SdrIterMode::DeepWithGroups);
            std::vector<basegfx::B3DHomMatrix*> aObjTrans;
            while(aIter.IsMore())
            {
                E3dObject* p3DObj = static_cast<E3dObject*>(aIter.Next());
                basegfx::B3DHomMatrix* pNew = new basegfx::B3DHomMatrix;
                *pNew = p3DObj->GetTransform();
                aObjTrans.push_back(pNew);
            }

            // reset object transformations
            aIter.Reset();
            while(aIter.IsMore())
            {
                E3dObject* p3DObj = static_cast<E3dObject*>(aIter.Next());
                p3DObj->NbcSetTransform(basegfx::B3DHomMatrix());
            }

            // reset scene transformation and make a complete recalc
            pScene->NbcSetTransform(basegfx::B3DHomMatrix());

            // fill old camera from new parameters
            Camera3D aCam(pScene->GetCamera());
            const basegfx::B3DRange& rVolume = pScene->GetBoundVolume();
            double fW = rVolume.getWidth();
            double fH = rVolume.getHeight();

            const SfxItemSet& rSceneSet = pScene->GetMergedItemSet();
            double fCamPosZ =
                (double)static_cast<const SfxUInt32Item&>(rSceneSet.Get(SDRATTR_3DSCENE_DISTANCE)).GetValue();
            double fCamFocal =
                (double)static_cast<const SfxUInt32Item&>(rSceneSet.Get(SDRATTR_3DSCENE_FOCAL_LENGTH)).GetValue();

            aCam.SetAutoAdjustProjection(false);
            aCam.SetViewWindow(- fW / 2, - fH / 2, fW, fH);
            basegfx::B3DPoint aLookAt;
            basegfx::B3DPoint aCamPos(0.0, 0.0, fCamPosZ);
            aCam.SetPosAndLookAt(aCamPos, aLookAt);
            aCam.SetFocalLength(fCamFocal / 100.0);
            aCam.SetDefaults(basegfx::B3DPoint(0.0, 0.0, fCamPosZ), aLookAt, fCamFocal / 100.0);
            aCam.SetDeviceWindow(Rectangle(0, 0, (long)fW, (long)fH));

            // set at scene
            pScene->SetCamera(aCam);

            // #91047# use imported VRP, VPN and VUP (if used)
            bool bVRPUsed(!aVRP.equal(basegfx::B3DPoint(0.0, 0.0, 1.0)));
            bool bVPNUsed(!aVPN.equal(basegfx::B3DVector(0.0, 0.0, 1.0)));
            bool bVUPUsed(!aVUP.equal(basegfx::B3DVector(0.0, 1.0, 0.0)));

            if(bVRPUsed || bVPNUsed || bVUPUsed)
            {
                pScene->GetCameraSet().SetViewportValues(aVRP, aVPN, aVUP);
            }

            // set object transformations again at objects
            aIter.Reset();
            sal_uInt32 nIndex(0L);
            while(aIter.IsMore())
            {
                E3dObject* p3DObj = static_cast<E3dObject*>(aIter.Next());
                basegfx::B3DHomMatrix* pMat = aObjTrans[nIndex++];
                p3DObj->NbcSetTransform(*pMat);
                delete pMat;
            }

            // set scene transformation again at scene
            pScene->NbcSetTransform(aSceneTAR.maMat);
            pScene->NbcSetSnapRect(aSceneTAR.maRect);

            return true;
        }
        break;
    }
    default:
        return SvxShape::setPropertyValueImpl(rName, pProperty, rValue);
    }

    throw IllegalArgumentException();
}


bool Svx3DSceneObject::getPropertyValueImpl(const OUString& rName, const SfxItemPropertySimpleEntry* pProperty,
    css::uno::Any& rValue)
{
    switch( pProperty->nWID )
    {
    case OWN_ATTR_3D_VALUE_TRANSFORM_MATRIX:
    {
        // pach object to a homogeneous 4x4 matrix
        ConvertObjectToHomogenMatric( static_cast< E3dObject* >( mpObj.get() ), rValue );
        break;
    }
    case OWN_ATTR_3D_VALUE_CAMERA_GEOMETRY:
    {
        // get CameraGeometry from scene
        E3dScene* pScene = static_cast< E3dScene* >( mpObj.get() );
        drawing::CameraGeometry aCamGeo;

        // fill Vectors from scene camera
        B3dCamera& aCameraSet = pScene->GetCameraSet();
        basegfx::B3DPoint aVRP(aCameraSet.GetVRP());
        basegfx::B3DVector aVPN(aCameraSet.GetVPN());
        basegfx::B3DVector aVUP(aCameraSet.GetVUV());

        // transfer to structure
        aCamGeo.vrp.PositionX = aVRP.getX();
        aCamGeo.vrp.PositionY = aVRP.getY();
        aCamGeo.vrp.PositionZ = aVRP.getZ();
        aCamGeo.vpn.DirectionX = aVPN.getX();
        aCamGeo.vpn.DirectionY = aVPN.getY();
        aCamGeo.vpn.DirectionZ = aVPN.getZ();
        aCamGeo.vup.DirectionX = aVUP.getX();
        aCamGeo.vup.DirectionY = aVUP.getY();
        aCamGeo.vup.DirectionZ = aVUP.getZ();

        rValue <<= aCamGeo;
        break;
    }
    default:
        return SvxShape::getPropertyValueImpl( rName, pProperty, rValue );
    }

    return true;
}

// css::lang::XServiceInfo
uno::Sequence< OUString > SAL_CALL Svx3DSceneObject::getSupportedServiceNames()
{
    uno::Sequence< OUString > aSeq( SvxShape::getSupportedServiceNames() );
    comphelper::ServiceInfoHelper::addToSequence( aSeq, {"com.sun.star.drawing.Shape3DScene"} );
    return aSeq;
}

Svx3DCubeObject::Svx3DCubeObject( SdrObject* pObj ) throw()
:   SvxShape( pObj, getSvxMapProvider().GetMap(SVXMAP_3DCUBEOBJEKT), getSvxMapProvider().GetPropertySet(SVXMAP_3DCUBEOBJEKT, SdrObject::GetGlobalDrawObjectItemPool()) )
{
}

Svx3DCubeObject::~Svx3DCubeObject() throw()
{
}

bool Svx3DCubeObject::setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue )
{
    SolarMutexGuard aGuard;

    switch( pProperty->nWID )
    {
    case OWN_ATTR_3D_VALUE_TRANSFORM_MATRIX:
    {
        // pack transformationmatrix to the object
        if( ConvertHomogenMatrixToObject( static_cast< E3dObject* >( mpObj.get() ), rValue ) )
            return true;
        break;
    }
    case OWN_ATTR_3D_VALUE_POSITION:
    {
        // pack position to the object
        drawing::Position3D aUnoPos;
        if( rValue >>= aUnoPos )
        {
            basegfx::B3DPoint aPos(aUnoPos.PositionX, aUnoPos.PositionY, aUnoPos.PositionZ);
            static_cast< E3dCubeObj* >( mpObj.get() )->SetCubePos(aPos);
            return true;
        }
        break;
    }
    case OWN_ATTR_3D_VALUE_SIZE:
    {
        // pack size to the object
        drawing::Direction3D aDirection;
        if( rValue >>= aDirection )
        {
            basegfx::B3DVector aSize(aDirection.DirectionX, aDirection.DirectionY, aDirection.DirectionZ);
            static_cast< E3dCubeObj* >( mpObj.get() )->SetCubeSize(aSize);
            return true;
        }
        break;
    }
    case OWN_ATTR_3D_VALUE_POS_IS_CENTER:
    {
        bool bNew = false;
        // pack sal_Bool bPosIsCenter to the object
        if( rValue >>= bNew )
        {
            static_cast< E3dCubeObj* >( mpObj.get() )->SetPosIsCenter(bNew);
            return true;
        }
        break;
    }
    default:
        return SvxShape::setPropertyValueImpl( rName, pProperty, rValue );
    }

    throw IllegalArgumentException();
}

bool Svx3DCubeObject::getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue )
{
    switch( pProperty->nWID )
    {
    case OWN_ATTR_3D_VALUE_TRANSFORM_MATRIX:
    {
        // pack transformation to a homogeneous matrix
        ConvertObjectToHomogenMatric( static_cast< E3dObject* >( mpObj.get() ), rValue );
        break;
    }
    case OWN_ATTR_3D_VALUE_POSITION:
    {
        // pack position
        const basegfx::B3DPoint& rPos = static_cast<E3dCubeObj*>(mpObj.get())->GetCubePos();
        drawing::Position3D aPos;

        aPos.PositionX = rPos.getX();
        aPos.PositionY = rPos.getY();
        aPos.PositionZ = rPos.getZ();

        rValue <<= aPos;
        break;
    }
    case OWN_ATTR_3D_VALUE_SIZE:
    {
        // pack size
        const basegfx::B3DVector& rSize = static_cast<E3dCubeObj*>(mpObj.get())->GetCubeSize();
        drawing::Direction3D aDir;

        aDir.DirectionX = rSize.getX();
        aDir.DirectionY = rSize.getY();
        aDir.DirectionZ = rSize.getZ();

        rValue <<= aDir;
        break;
    }
    case OWN_ATTR_3D_VALUE_POS_IS_CENTER:
    {
        rValue <<= static_cast<E3dCubeObj*>(mpObj.get())->GetPosIsCenter();
        break;
    }
    default:
        return SvxShape::getPropertyValueImpl( rName, pProperty, rValue );
    }

    return true;
}

// css::lang::XServiceInfo
uno::Sequence< OUString > SAL_CALL Svx3DCubeObject::getSupportedServiceNames()
{
    uno::Sequence< OUString > aSeq( SvxShape::getSupportedServiceNames() );
    comphelper::ServiceInfoHelper::addToSequence( aSeq, {"com.sun.star.drawing.Shape3D",
                            "com.sun.star.drawing.Shape3DCube"});
    return aSeq;
}

Svx3DSphereObject::Svx3DSphereObject( SdrObject* pObj ) throw()
:   SvxShape( pObj, getSvxMapProvider().GetMap(SVXMAP_3DSPHEREOBJECT), getSvxMapProvider().GetPropertySet(SVXMAP_3DSPHEREOBJECT, SdrObject::GetGlobalDrawObjectItemPool()) )
{
}

Svx3DSphereObject::~Svx3DSphereObject() throw()
{
}

bool Svx3DSphereObject::setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue )
{
    switch( pProperty->nWID )
    {
    case OWN_ATTR_3D_VALUE_TRANSFORM_MATRIX:
    {
        // pack transformation matrix to the object
        if( ConvertHomogenMatrixToObject( static_cast< E3dObject* >( mpObj.get() ), rValue ) )
            return true;
        break;
    }

    case OWN_ATTR_3D_VALUE_POSITION:
    {
        // pack position to the object
        drawing::Position3D aUnoPos;
        if( rValue >>= aUnoPos )
        {
            basegfx::B3DPoint aPos(aUnoPos.PositionX, aUnoPos.PositionY, aUnoPos.PositionZ);
            static_cast<E3dSphereObj*>(mpObj.get())->SetCenter(aPos);
            return true;
        }
        break;
    }

    case OWN_ATTR_3D_VALUE_SIZE:
    {
        // pack size to the object
        drawing::Direction3D aDir;
        if( rValue >>= aDir )
        {
            basegfx::B3DVector aPos(aDir.DirectionX, aDir.DirectionY, aDir.DirectionZ);
            static_cast<E3dSphereObj*>(mpObj.get())->SetSize(aPos);
            return true;
        }
        break;
    }
    default:
        return SvxShape::setPropertyValueImpl( rName, pProperty, rValue );
    }

    throw IllegalArgumentException();
}

bool Svx3DSphereObject::getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue )
{
    switch( pProperty->nWID )
    {
    case OWN_ATTR_3D_VALUE_TRANSFORM_MATRIX:
    {
        // pack transformation to a homogeneous matrix
        ConvertObjectToHomogenMatric( static_cast< E3dObject* >( mpObj.get() ), rValue );
        break;
    }
    case OWN_ATTR_3D_VALUE_POSITION:
    {
        // pack position
        const basegfx::B3DPoint& rPos = static_cast<E3dSphereObj*>(mpObj.get())->Center();
        drawing::Position3D aPos;

        aPos.PositionX = rPos.getX();
        aPos.PositionY = rPos.getY();
        aPos.PositionZ = rPos.getZ();

        rValue <<= aPos;
        break;
    }
    case OWN_ATTR_3D_VALUE_SIZE:
    {
        // pack size
        const basegfx::B3DVector& rSize = static_cast<E3dSphereObj*>(mpObj.get())->Size();
        drawing::Direction3D aDir;

        aDir.DirectionX = rSize.getX();
        aDir.DirectionY = rSize.getY();
        aDir.DirectionZ = rSize.getZ();

        rValue <<= aDir;
        break;
    }
    default:
        return SvxShape::getPropertyValueImpl( rName, pProperty, rValue );
    }

    return true;
}

// css::lang::XServiceInfo
uno::Sequence< OUString > SAL_CALL Svx3DSphereObject::getSupportedServiceNames()
{
    uno::Sequence< OUString > aSeq( SvxShape::getSupportedServiceNames() );
    comphelper::ServiceInfoHelper::addToSequence( aSeq, {"com.sun.star.drawing.Shape3D",
                            "com.sun.star.drawing.Shape3DSphere"});
    return aSeq;
}
Svx3DLatheObject::Svx3DLatheObject( SdrObject* pObj ) throw()
:   SvxShape( pObj, getSvxMapProvider().GetMap(SVXMAP_3DLATHEOBJECT), getSvxMapProvider().GetPropertySet(SVXMAP_3DLATHEOBJECT, SdrObject::GetGlobalDrawObjectItemPool()) )
{
}


Svx3DLatheObject::~Svx3DLatheObject() throw()
{
}

bool PolyPolygonShape3D_to_B3dPolyPolygon(
    const Any& rValue,
    basegfx::B3DPolyPolygon& rResultPolygon,
    bool bCorrectPolygon)
{
    drawing::PolyPolygonShape3D aSourcePolyPolygon;
    if( !(rValue >>= aSourcePolyPolygon) )
        return false;

    sal_Int32 nOuterSequenceCount = aSourcePolyPolygon.SequenceX.getLength();
    if(nOuterSequenceCount != aSourcePolyPolygon.SequenceY.getLength() || nOuterSequenceCount != aSourcePolyPolygon.SequenceZ.getLength())
        return false;

    drawing::DoubleSequence* pInnerSequenceX = aSourcePolyPolygon.SequenceX.getArray();
    drawing::DoubleSequence* pInnerSequenceY = aSourcePolyPolygon.SequenceY.getArray();
    drawing::DoubleSequence* pInnerSequenceZ = aSourcePolyPolygon.SequenceZ.getArray();
    for(sal_Int32 a(0L);a<nOuterSequenceCount;a++)
    {
        sal_Int32 nInnerSequenceCount = pInnerSequenceX->getLength();
        if(nInnerSequenceCount != pInnerSequenceY->getLength() || nInnerSequenceCount != pInnerSequenceZ->getLength())
        {
            return false;
        }
        basegfx::B3DPolygon aNewPolygon;
        double* pArrayX = pInnerSequenceX->getArray();
        double* pArrayY = pInnerSequenceY->getArray();
        double* pArrayZ = pInnerSequenceZ->getArray();
        for(sal_Int32 b(0L);b<nInnerSequenceCount;b++)
        {
            aNewPolygon.append(basegfx::B3DPoint(*pArrayX++,*pArrayY++,*pArrayZ++));
        }
        pInnerSequenceX++;
        pInnerSequenceY++;
        pInnerSequenceZ++;

        // #i101520# correction is needed for imported polygons of old format,
        // see callers
        if(bCorrectPolygon)
        {
            basegfx::tools::checkClosed(aNewPolygon);
        }

        rResultPolygon.append(aNewPolygon);
    }
    return true;
}

static void B3dPolyPolygon_to_PolyPolygonShape3D( const basegfx::B3DPolyPolygon& rSourcePolyPolygon, Any& rValue )
{
    drawing::PolyPolygonShape3D aRetval;
    aRetval.SequenceX.realloc(rSourcePolyPolygon.count());
    aRetval.SequenceY.realloc(rSourcePolyPolygon.count());
    aRetval.SequenceZ.realloc(rSourcePolyPolygon.count());
    drawing::DoubleSequence* pOuterSequenceX = aRetval.SequenceX.getArray();
    drawing::DoubleSequence* pOuterSequenceY = aRetval.SequenceY.getArray();
    drawing::DoubleSequence* pOuterSequenceZ = aRetval.SequenceZ.getArray();
    for(sal_uInt32 a(0L);a<rSourcePolyPolygon.count();a++)
    {
        const basegfx::B3DPolygon aPoly(rSourcePolyPolygon.getB3DPolygon(a));
        sal_Int32 nPointCount(aPoly.count());
        if(aPoly.isClosed()) nPointCount++;
        pOuterSequenceX->realloc(nPointCount);
        pOuterSequenceY->realloc(nPointCount);
        pOuterSequenceZ->realloc(nPointCount);
        double* pInnerSequenceX = pOuterSequenceX->getArray();
        double* pInnerSequenceY = pOuterSequenceY->getArray();
        double* pInnerSequenceZ = pOuterSequenceZ->getArray();
        for(sal_uInt32 b(0L);b<aPoly.count();b++)
        {
            const basegfx::B3DPoint aPoint(aPoly.getB3DPoint(b));
            *pInnerSequenceX++ = aPoint.getX();
            *pInnerSequenceY++ = aPoint.getY();
            *pInnerSequenceZ++ = aPoint.getZ();
        }
        if(aPoly.isClosed())
        {
            const basegfx::B3DPoint aPoint(aPoly.getB3DPoint(0L));
            *pInnerSequenceX++ = aPoint.getX();
            *pInnerSequenceY++ = aPoint.getY();
            *pInnerSequenceZ++ = aPoint.getZ();
        }
        pOuterSequenceX++;
        pOuterSequenceY++;
        pOuterSequenceZ++;
    }
    rValue <<= aRetval;
}

bool Svx3DLatheObject::setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue )
{
    switch( pProperty->nWID )
    {
    case OWN_ATTR_3D_VALUE_TRANSFORM_MATRIX:
    {
        // pack transformation matrix to the object
        if( ConvertHomogenMatrixToObject( static_cast< E3dObject* >( mpObj.get() ), rValue ) )
            return true;
        break;
    }
    case OWN_ATTR_3D_VALUE_POLYPOLYGON3D:
    {
        // pack polygon definition to the object
        basegfx::B3DPolyPolygon aNewB3DPolyPolygon;

        // #i101520# Probably imported
        if( PolyPolygonShape3D_to_B3dPolyPolygon( rValue, aNewB3DPolyPolygon, true ) )
        {
            // #105127# SetPolyPoly3D sets the Svx3DVerticalSegmentsItem to the number
            // of points of the polygon. Thus, value gets lost. To avoid this, rescue
            // item here and re-set after setting the polygon.
            const sal_uInt32 nPrevVerticalSegs(static_cast<E3dLatheObj*>(mpObj.get())->GetVerticalSegments());

            // set polygon
            const basegfx::B3DHomMatrix aIdentity;
            const basegfx::B2DPolyPolygon aB2DPolyPolygon(basegfx::tools::createB2DPolyPolygonFromB3DPolyPolygon(aNewB3DPolyPolygon, aIdentity));
            static_cast<E3dLatheObj*>(mpObj.get())->SetPolyPoly2D(aB2DPolyPolygon);
            const sal_uInt32 nPostVerticalSegs(static_cast<E3dLatheObj*>(mpObj.get())->GetVerticalSegments());

            if(nPrevVerticalSegs != nPostVerticalSegs)
            {
                // restore the vertical segment count
                static_cast<E3dLatheObj*>(mpObj.get())->SetMergedItem(makeSvx3DVerticalSegmentsItem(nPrevVerticalSegs));
            }
            return true;
        }
        break;
    }
    default:
        return SvxShape::setPropertyValueImpl( rName, pProperty, rValue );
    }

    throw IllegalArgumentException();
}

bool Svx3DLatheObject::getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue )
{
    switch( pProperty->nWID )
    {
    case OWN_ATTR_3D_VALUE_TRANSFORM_MATRIX:
    {
        // pack transformation to a homogeneous matrix
        drawing::HomogenMatrix aHomMat;
        basegfx::B3DHomMatrix aMat = static_cast<E3dObject*>(mpObj.get())->GetTransform();

        // pack evtl. transformed matrix to output
        aHomMat.Line1.Column1 = aMat.get(0, 0);
        aHomMat.Line1.Column2 = aMat.get(0, 1);
        aHomMat.Line1.Column3 = aMat.get(0, 2);
        aHomMat.Line1.Column4 = aMat.get(0, 3);
        aHomMat.Line2.Column1 = aMat.get(1, 0);
        aHomMat.Line2.Column2 = aMat.get(1, 1);
        aHomMat.Line2.Column3 = aMat.get(1, 2);
        aHomMat.Line2.Column4 = aMat.get(1, 3);
        aHomMat.Line3.Column1 = aMat.get(2, 0);
        aHomMat.Line3.Column2 = aMat.get(2, 1);
        aHomMat.Line3.Column3 = aMat.get(2, 2);
        aHomMat.Line3.Column4 = aMat.get(2, 3);
        aHomMat.Line4.Column1 = aMat.get(3, 0);
        aHomMat.Line4.Column2 = aMat.get(3, 1);
        aHomMat.Line4.Column3 = aMat.get(3, 2);
        aHomMat.Line4.Column4 = aMat.get(3, 3);

        rValue <<= aHomMat;
        break;
    }
    case OWN_ATTR_3D_VALUE_POLYPOLYGON3D:
    {
        const basegfx::B2DPolyPolygon& rPolyPoly = static_cast<E3dLatheObj*>(mpObj.get())->GetPolyPoly2D();
        const basegfx::B3DPolyPolygon aB3DPolyPolygon(basegfx::tools::createB3DPolyPolygonFromB2DPolyPolygon(rPolyPoly));

        B3dPolyPolygon_to_PolyPolygonShape3D(aB3DPolyPolygon, rValue);
        break;
    }
    default:
        return SvxShape::getPropertyValueImpl( rName, pProperty, rValue );
    }

    return true;
}

// css::lang::XServiceInfo
uno::Sequence< OUString > SAL_CALL Svx3DLatheObject::getSupportedServiceNames()
{
    uno::Sequence< OUString > aSeq( SvxShape::getSupportedServiceNames() );
    comphelper::ServiceInfoHelper::addToSequence( aSeq, {"com.sun.star.drawing.Shape3D",
                            "com.sun.star.drawing.Shape3DLathe"});
    return aSeq;
}

Svx3DExtrudeObject::Svx3DExtrudeObject( SdrObject* pObj ) throw()
:   SvxShape( pObj, getSvxMapProvider().GetMap(SVXMAP_3DEXTRUDEOBJECT), getSvxMapProvider().GetPropertySet(SVXMAP_3DEXTRUDEOBJECT, SdrObject::GetGlobalDrawObjectItemPool()) )
{
}

Svx3DExtrudeObject::~Svx3DExtrudeObject() throw()
{
}

bool Svx3DExtrudeObject::setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue )
{
    switch( pProperty->nWID )
    {
    case OWN_ATTR_3D_VALUE_TRANSFORM_MATRIX:
    {
        // pack transformation matrix to the object
        if( ConvertHomogenMatrixToObject( static_cast< E3dObject* >( mpObj.get() ), rValue ) )
            return true;
        break;
    }

    case OWN_ATTR_3D_VALUE_POLYPOLYGON3D:
    {
        // pack polygon definition to the object
        basegfx::B3DPolyPolygon aNewB3DPolyPolygon;

        // #i101520# Probably imported
        if( PolyPolygonShape3D_to_B3dPolyPolygon( rValue, aNewB3DPolyPolygon, true ) )
        {
            // set polygon
            const basegfx::B3DHomMatrix aIdentity;
            const basegfx::B2DPolyPolygon aB2DPolyPolygon(basegfx::tools::createB2DPolyPolygonFromB3DPolyPolygon(aNewB3DPolyPolygon, aIdentity));
            static_cast<E3dExtrudeObj*>(mpObj.get())->SetExtrudePolygon(aB2DPolyPolygon);
            return true;
        }
        break;
    }
    default:
        return SvxShape::setPropertyValueImpl( rName, pProperty, rValue );
    }

    throw IllegalArgumentException();
}

bool Svx3DExtrudeObject::getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue )
{
    switch( pProperty->nWID )
    {
    case OWN_ATTR_3D_VALUE_TRANSFORM_MATRIX:
    {
        // pack transformation to a homogeneous matrix
        drawing::HomogenMatrix aHomMat;
        basegfx::B3DHomMatrix aMat = static_cast<E3dObject*>(mpObj.get())->GetTransform();

        // pack evtl. transformed matrix to output
        aHomMat.Line1.Column1 = aMat.get(0, 0);
        aHomMat.Line1.Column2 = aMat.get(0, 1);
        aHomMat.Line1.Column3 = aMat.get(0, 2);
        aHomMat.Line1.Column4 = aMat.get(0, 3);
        aHomMat.Line2.Column1 = aMat.get(1, 0);
        aHomMat.Line2.Column2 = aMat.get(1, 1);
        aHomMat.Line2.Column3 = aMat.get(1, 2);
        aHomMat.Line2.Column4 = aMat.get(1, 3);
        aHomMat.Line3.Column1 = aMat.get(2, 0);
        aHomMat.Line3.Column2 = aMat.get(2, 1);
        aHomMat.Line3.Column3 = aMat.get(2, 2);
        aHomMat.Line3.Column4 = aMat.get(2, 3);
        aHomMat.Line4.Column1 = aMat.get(3, 0);
        aHomMat.Line4.Column2 = aMat.get(3, 1);
        aHomMat.Line4.Column3 = aMat.get(3, 2);
        aHomMat.Line4.Column4 = aMat.get(3, 3);

        rValue <<= aHomMat;
        break;
    }

    case OWN_ATTR_3D_VALUE_POLYPOLYGON3D:
    {
        // pack polygon definition
        const basegfx::B2DPolyPolygon& rPolyPoly = static_cast<E3dExtrudeObj*>(mpObj.get())->GetExtrudePolygon();
        const basegfx::B3DPolyPolygon aB3DPolyPolygon(basegfx::tools::createB3DPolyPolygonFromB2DPolyPolygon(rPolyPoly));

        B3dPolyPolygon_to_PolyPolygonShape3D(aB3DPolyPolygon, rValue);
        break;
    }
    default:
        return SvxShape::getPropertyValueImpl( rName, pProperty, rValue );
    }

    return true;
}

// css::lang::XServiceInfo
uno::Sequence< OUString > SAL_CALL Svx3DExtrudeObject::getSupportedServiceNames()
{
    uno::Sequence< OUString > aSeq( SvxShape::getSupportedServiceNames() );
    comphelper::ServiceInfoHelper::addToSequence( aSeq, {"com.sun.star.drawing.Shape3D",
                            "com.sun.star.drawing.Shape3DExtrude"});
    return aSeq;
}

Svx3DPolygonObject::Svx3DPolygonObject( SdrObject* pObj ) throw()
:   SvxShape( pObj, getSvxMapProvider().GetMap(SVXMAP_3DPOLYGONOBJECT), getSvxMapProvider().GetPropertySet(SVXMAP_3DPOLYGONOBJECT, SdrObject::GetGlobalDrawObjectItemPool()) )
{
}

Svx3DPolygonObject::~Svx3DPolygonObject() throw()
{
}

bool Svx3DPolygonObject::setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue )
{
    switch( pProperty->nWID )
    {
    case OWN_ATTR_3D_VALUE_TRANSFORM_MATRIX:
    {
        // pack transformation matrix to the object
        if( ConvertHomogenMatrixToObject( static_cast< E3dObject* >( mpObj.get() ), rValue ) )
            return true;
        break;
    }

    case OWN_ATTR_3D_VALUE_POLYPOLYGON3D:
    {
        // pack polygon definition to the object
        basegfx::B3DPolyPolygon aNewB3DPolyPolygon;

        // #i101520# Direct API data (e.g. from chart)
        if( PolyPolygonShape3D_to_B3dPolyPolygon( rValue, aNewB3DPolyPolygon, false ) )
        {
            // set polygon
            static_cast<E3dPolygonObj*>(mpObj.get())->SetPolyPolygon3D(aNewB3DPolyPolygon);
            return true;
        }
        break;
    }
    case OWN_ATTR_3D_VALUE_NORMALSPOLYGON3D:
    {
        // pack perpendicular definition to the object
        basegfx::B3DPolyPolygon aNewB3DPolyPolygon;

        // #i101520# Direct API data (e.g. from chart)
        if( PolyPolygonShape3D_to_B3dPolyPolygon( rValue, aNewB3DPolyPolygon, false ) )
        {
            // set polygon
            static_cast<E3dPolygonObj*>(mpObj.get())->SetPolyNormals3D(aNewB3DPolyPolygon);
            return true;
        }
        break;
    }
    case OWN_ATTR_3D_VALUE_TEXTUREPOLYGON3D:
    {
        // pack texture definition to the object
        basegfx::B3DPolyPolygon aNewB3DPolyPolygon;

        // #i101520# Direct API data (e.g. from chart)
        if( PolyPolygonShape3D_to_B3dPolyPolygon( rValue, aNewB3DPolyPolygon, false ) )
        {
            // set polygon
            const basegfx::B3DHomMatrix aIdentity;
            const basegfx::B2DPolyPolygon aB2DPolyPolygon(basegfx::tools::createB2DPolyPolygonFromB3DPolyPolygon(aNewB3DPolyPolygon, aIdentity));
            static_cast<E3dPolygonObj*>(mpObj.get())->SetPolyTexture2D(aB2DPolyPolygon);
            return true;
        }
        break;
    }
    case OWN_ATTR_3D_VALUE_LINEONLY:
    {
        bool bNew = false;
        if( rValue >>= bNew )
        {
            static_cast<E3dPolygonObj*>(mpObj.get())->SetLineOnly(bNew);
            return true;
        }
        break;
    }
    default:
        return SvxShape::setPropertyValueImpl( rName, pProperty, rValue );
    }

    throw IllegalArgumentException();
}

bool Svx3DPolygonObject::getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue )
{
    switch( pProperty->nWID )
    {
    case OWN_ATTR_3D_VALUE_TRANSFORM_MATRIX:
    {
        ConvertObjectToHomogenMatric( static_cast< E3dObject* >( mpObj.get() ), rValue );
        break;
    }

    case OWN_ATTR_3D_VALUE_POLYPOLYGON3D:
    {
        B3dPolyPolygon_to_PolyPolygonShape3D(static_cast<E3dPolygonObj*>(mpObj.get())->GetPolyPolygon3D(),rValue);
        break;
    }

    case OWN_ATTR_3D_VALUE_NORMALSPOLYGON3D:
    {
        B3dPolyPolygon_to_PolyPolygonShape3D(static_cast<E3dPolygonObj*>(mpObj.get())->GetPolyNormals3D(),rValue);
        break;
    }

    case OWN_ATTR_3D_VALUE_TEXTUREPOLYGON3D:
    {
        // pack texture definition
        const basegfx::B2DPolyPolygon& rPolyPoly = static_cast<E3dPolygonObj*>(mpObj.get())->GetPolyTexture2D();
        const basegfx::B3DPolyPolygon aB3DPolyPolygon(basegfx::tools::createB3DPolyPolygonFromB2DPolyPolygon(rPolyPoly));

        B3dPolyPolygon_to_PolyPolygonShape3D(aB3DPolyPolygon,rValue);
        break;
    }

    case OWN_ATTR_3D_VALUE_LINEONLY:
    {
        rValue <<= static_cast<E3dPolygonObj*>(mpObj.get())->GetLineOnly();
        break;
    }

    default:
        return SvxShape::getPropertyValueImpl( rName, pProperty, rValue );
    }

    return true;
}

// css::lang::XServiceInfo
uno::Sequence< OUString > SAL_CALL Svx3DPolygonObject::getSupportedServiceNames()
{
    Sequence< OUString > aSeq( SvxShape::getSupportedServiceNames() );
    comphelper::ServiceInfoHelper::addToSequence( aSeq, {"com.sun.star.drawing.Shape3D",
                            "com.sun.star.drawing.Shape3DPolygon"});
    return aSeq;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

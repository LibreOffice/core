/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoshap3.cxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:54:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#define _SVX_USE_UNOGLOBALS_

#ifndef _COM_SUN_STAR_DRAWING_HOMOGENMATRIX_HPP_
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POSITION3D_HPP_
#include <com/sun/star/drawing/Position3D.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_DIRECTION3D_HPP_
#include <com/sun/star/drawing/Direction3D.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_DOUBLESEQUENCE_HPP_
#include <com/sun/star/drawing/DoubleSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_CAMERAGEOMETRY_HPP_
#include <com/sun/star/drawing/CameraGeometry.hpp>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#include <rtl/uuid.h>
#include <rtl/memory.h>

#include "unoshape.hxx"
#include "unopage.hxx"
#include "unoprnms.hxx"
#include "polysc3d.hxx"
#include "globl3d.hxx"
#include "cube3d.hxx"
#include "sphere3d.hxx"
#include "lathe3d.hxx"
#include "extrud3d.hxx"
#include "polygn3d.hxx"
//#include "poly3d.hxx"
#include "svdmodel.hxx"

#ifndef _BGFX_POLYGON_B3DPOLYGON_HXX
#include <basegfx/polygon/b3dpolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolygontools.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONSHAPE3D_HPP_
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

using ::rtl::OUString;
using namespace ::vos;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

#define INTERFACE_TYPE( xint ) \
    ::getCppuType((const Reference< xint >*)0)

#define QUERYINT( xint ) \
    if( rType == ::getCppuType((const Reference< xint >*)0) ) \
        aAny <<= Reference< xint >(this)

/***********************************************************************
* class Svx3DSceneObject                                               *
***********************************************************************/

//----------------------------------------------------------------------
Svx3DSceneObject::Svx3DSceneObject( SdrObject* pObj, SvxDrawPage* pDrawPage ) throw()
:   SvxShape( pObj, aSvxMapProvider.GetMap(SVXMAP_3DSCENEOBJECT) )
,   mxPage( pDrawPage )
{
}

//----------------------------------------------------------------------
Svx3DSceneObject::~Svx3DSceneObject() throw()
{
}

//----------------------------------------------------------------------
void Svx3DSceneObject::Create( SdrObject* pNewObj, SvxDrawPage* pNewPage ) throw()
{
    SvxShape::Create( pNewObj, pNewPage );
    mxPage = pNewPage;
}

//----------------------------------------------------------------------
uno::Any SAL_CALL Svx3DSceneObject::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aAny;

    QUERYINT( drawing::XShapes );
    else QUERYINT( container::XIndexAccess );
    else QUERYINT( container::XElementAccess );
    else
        return SvxShape::queryAggregation( rType );

    return aAny;
}

uno::Any SAL_CALL Svx3DSceneObject::queryInterface( const uno::Type & rType ) throw( uno::RuntimeException )
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

uno::Sequence< uno::Type > SAL_CALL Svx3DSceneObject::getTypes()
    throw (uno::RuntimeException)
{

    return SvxShape::getTypes();
}

uno::Sequence< sal_Int8 > SAL_CALL Svx3DSceneObject::getImplementationId()
    throw (uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

//----------------------------------------------------------------------
void SAL_CALL Svx3DSceneObject::add( const Reference< drawing::XShape >& xShape )
    throw( uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    SvxShape* pShape = SvxShape::getImplementation( xShape );

    if(!mpObj.is() || !mxPage.is() || pShape == NULL || NULL != pShape->GetSdrObject() )
        throw uno::RuntimeException();

    SdrObject* pSdrShape = mxPage->_CreateSdrObject( xShape );
    if( pSdrShape->ISA(E3dObject) )
    {
        mpObj->GetSubList()->NbcInsertObject( pSdrShape );

        if(pShape)
            pShape->Create( pSdrShape, mxPage.get()  );
    }
    else
    {
        delete pSdrShape;
        pShape->InvalidateSdrObject();
        throw uno::RuntimeException();
    }

    if( mpModel )
        mpModel->SetChanged();
}

//----------------------------------------------------------------------
void SAL_CALL Svx3DSceneObject::remove( const Reference< drawing::XShape >& xShape )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    SvxShape* pShape = SvxShape::getImplementation( xShape );

    if(!mpObj.is() || pShape == NULL)
        throw uno::RuntimeException();

    SdrObject* pSdrShape = pShape->GetSdrObject();
    if(pSdrShape == NULL || pSdrShape->GetObjList()->GetOwnerObj() != mpObj.get())
    {
        throw uno::RuntimeException();
    }
    else
    {
        SdrObjList& rList = *pSdrShape->GetObjList();

        const sal_uInt32 nObjCount = rList.GetObjCount();
        sal_uInt32 nObjNum = 0;
        while( nObjNum < nObjCount )
        {
            if(rList.GetObj( nObjNum ) == pSdrShape )
                break;
            nObjNum++;
        }

        if( nObjNum < nObjCount )
        {
            delete rList.NbcRemoveObject( nObjNum );
            pShape->InvalidateSdrObject();
        }
        else
        {
            DBG_ASSERT( 0, "Fatality! SdrObject is not belonging to its SdrObjList! [CL]" );
        }
    }
}

//----------------------------------------------------------------------
sal_Int32 SAL_CALL Svx3DSceneObject::getCount()
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    sal_Int32 nRetval = 0;

    if(mpObj.is() && mpObj->ISA(E3dPolyScene) && mpObj->GetSubList())
        nRetval = mpObj->GetSubList()->GetObjCount();
    return nRetval;
}

//----------------------------------------------------------------------

uno::Any SAL_CALL Svx3DSceneObject::getByIndex( sal_Int32 Index )
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( !mpObj.is() || mpObj->GetSubList() == NULL )
        throw uno::RuntimeException();

    if( mpObj->GetSubList()->GetObjCount() <= (sal_uInt32)Index )
        throw lang::IndexOutOfBoundsException();

    SdrObject* pDestObj = mpObj->GetSubList()->GetObj( Index );
    if(pDestObj == NULL)
        throw lang::IndexOutOfBoundsException();

    Reference< drawing::XShape > xShape( pDestObj->getUnoShape(), uno::UNO_QUERY );
    uno::Any aAny;
    aAny <<= xShape;
    return aAny;
}

//----------------------------------------------------------------------
// ::com::sun::star::container::XElementAccess

uno::Type SAL_CALL Svx3DSceneObject::getElementType()
    throw( uno::RuntimeException )
{
    return ::getCppuType(( const Reference< drawing::XShape>*)0);
}

//----------------------------------------------------------------------
sal_Bool SAL_CALL Svx3DSceneObject::hasElements()
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    return mpObj.is() && mpObj->GetSubList() && (mpObj->GetSubList()->GetObjCount() > 0);
}

//----------------------------------------------------------------------
#define HOMOGEN_MATRIX_TO_OBJECT \
    drawing::HomogenMatrix m; \
    if( aValue >>= m ) { \
        basegfx::B3DHomMatrix aMat; \
        aMat.set(0, 0, m.Line1.Column1); \
        aMat.set(0, 1, m.Line1.Column2); \
        aMat.set(0, 2, m.Line1.Column3); \
        aMat.set(0, 3, m.Line1.Column4); \
        aMat.set(1, 0, m.Line2.Column1); \
        aMat.set(1, 1, m.Line2.Column2); \
        aMat.set(1, 2, m.Line2.Column3); \
        aMat.set(1, 3, m.Line2.Column4); \
        aMat.set(2, 0, m.Line3.Column1); \
        aMat.set(2, 1, m.Line3.Column2); \
        aMat.set(2, 2, m.Line3.Column3); \
        aMat.set(2, 3, m.Line3.Column4); \
        aMat.set(3, 0, m.Line4.Column1); \
        aMat.set(3, 1, m.Line4.Column2); \
        aMat.set(3, 2, m.Line4.Column3); \
        aMat.set(3, 3, m.Line4.Column4); \
        ((E3dObject*)mpObj.get())->SetTransform(aMat); \
    }

#define OBJECT_TO_HOMOGEN_MATRIX \
    drawing::HomogenMatrix aHomMat; \
    const basegfx::B3DHomMatrix& rMat = ((E3dObject*)mpObj.get())->GetTransform(); \
    aHomMat.Line1.Column1 = rMat.get(0, 0); \
    aHomMat.Line1.Column2 = rMat.get(0, 1); \
    aHomMat.Line1.Column3 = rMat.get(0, 2); \
    aHomMat.Line1.Column4 = rMat.get(0, 3); \
    aHomMat.Line2.Column1 = rMat.get(1, 0); \
    aHomMat.Line2.Column2 = rMat.get(1, 1); \
    aHomMat.Line2.Column3 = rMat.get(1, 2); \
    aHomMat.Line2.Column4 = rMat.get(1, 3); \
    aHomMat.Line3.Column1 = rMat.get(2, 0); \
    aHomMat.Line3.Column2 = rMat.get(2, 1); \
    aHomMat.Line3.Column3 = rMat.get(2, 2); \
    aHomMat.Line3.Column4 = rMat.get(2, 3); \
    aHomMat.Line4.Column1 = rMat.get(3, 0); \
    aHomMat.Line4.Column2 = rMat.get(3, 1); \
    aHomMat.Line4.Column3 = rMat.get(3, 2); \
    aHomMat.Line4.Column4 = rMat.get(3, 3); \
    return uno::Any( &aHomMat, ::getCppuType((const drawing::HomogenMatrix*)0) );

//----------------------------------------------------------------------
#ifndef _SVDITER_HXX
#include <svditer.hxx>
#endif

struct ImpRememberTransAndRect
{
    basegfx::B3DHomMatrix                   maMat;
    Rectangle                   maRect;
};

void SAL_CALL Svx3DSceneObject::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TRANSFORM_MATRIX)) )
    {
        // Transformationsmatrix in das Objekt packen
        HOMOGEN_MATRIX_TO_OBJECT
    }
    else if(mpObj.is() && mpObj->ISA(E3dScene) && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_CAMERA_GEOMETRY)) )
    {
        // set CameraGeometry at scene
        E3dScene* pScene = (E3dScene*)mpObj.get();
        drawing::CameraGeometry aCamGeo;

        if(aValue >>= aCamGeo)
        {
            basegfx::B3DPoint aVRP(aCamGeo.vrp.PositionX, aCamGeo.vrp.PositionY, aCamGeo.vrp.PositionZ);
            basegfx::B3DVector aVPN(aCamGeo.vpn.DirectionX, aCamGeo.vpn.DirectionY, aCamGeo.vpn.DirectionZ);
            basegfx::B3DVector aVUP(aCamGeo.vup.DirectionX, aCamGeo.vup.DirectionY, aCamGeo.vup.DirectionZ);

            // rescue scene transformation
            ImpRememberTransAndRect aSceneTAR;
            aSceneTAR.maMat = pScene->GetTransform();
            aSceneTAR.maRect = pScene->GetSnapRect();

            // rescue object transformations
            SdrObjListIter aIter(*pScene->GetSubList(), IM_DEEPWITHGROUPS);
            List aObjTrans;
            while(aIter.IsMore())
            {
                E3dObject* p3DObj = (E3dObject*)aIter.Next();
                basegfx::B3DHomMatrix* pNew = new basegfx::B3DHomMatrix;
                *pNew = p3DObj->GetTransform();
                aObjTrans.Insert(pNew, LIST_APPEND);
            }

            // reset object transformations
            aIter.Reset();
            while(aIter.IsMore())
            {
                E3dObject* p3DObj = (E3dObject*)aIter.Next();
                p3DObj->NbcResetTransform();
            }

            // reset scene transformation and make a complete recalc
            pScene->NbcResetTransform();

            // fill old camera from new parameters
            Camera3D aCam(pScene->GetCamera());
            const Volume3D& rVolume = pScene->GetBoundVolume();
            double fW = rVolume.getWidth();
            double fH = rVolume.getHeight();

            const SfxItemSet& rSceneSet = pScene->GetMergedItemSet();
            double fCamPosZ =
                (double)((const SfxUInt32Item&)rSceneSet.Get(SDRATTR_3DSCENE_DISTANCE)).GetValue();
            double fCamFocal =
                (double)((const SfxUInt32Item&)rSceneSet.Get(SDRATTR_3DSCENE_FOCAL_LENGTH)).GetValue();

            aCam.SetAutoAdjustProjection(FALSE);
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
            sal_Bool bVRPUsed(!aVRP.equal(basegfx::B3DPoint(0.0, 0.0, 1.0)));
            sal_Bool bVPNUsed(!aVPN.equal(basegfx::B3DVector(0.0, 0.0, 1.0)));
            sal_Bool bVUPUsed(!aVUP.equal(basegfx::B3DVector(0.0, 1.0, 0.0)));

            if(bVRPUsed || bVPNUsed || bVUPUsed)
            {
                pScene->GetCameraSet().SetViewportValues(aVRP, aVPN, aVUP);
            }

            // set object transformations again at objects
            aIter.Reset();
            sal_uInt32 nIndex(0L);
            while(aIter.IsMore())
            {
                E3dObject* p3DObj = (E3dObject*)aIter.Next();
                basegfx::B3DHomMatrix* pMat = (basegfx::B3DHomMatrix*)aObjTrans.GetObject(nIndex++);
                p3DObj->NbcSetTransform(*pMat);
                delete pMat;
            }

            // set scene transformation again at scene
            pScene->NbcSetTransform(aSceneTAR.maMat);
            pScene->FitSnapRectToBoundVol();
            pScene->NbcSetSnapRect(aSceneTAR.maRect);

            // #86559# init transformation set to allow correct
            // calculation of BoundRect
            pScene->InitTransformationSet();
        }
    }
    else
    {
        SvxShape::setPropertyValue(aPropertyName, aValue);
    }
}

//----------------------------------------------------------------------
uno::Any SAL_CALL Svx3DSceneObject::getPropertyValue( const OUString& PropertyName )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(mpObj.is() && PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TRANSFORM_MATRIX)) )
    {
        // Objekt in eine homogene 4x4 Matrix packen
        OBJECT_TO_HOMOGEN_MATRIX
    }
    else if(mpObj.is() && mpObj->ISA(E3dScene) && PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_CAMERA_GEOMETRY)) )
    {
        // get CameraGeometry from scene
        E3dScene* pScene = (E3dScene*)mpObj.get();
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

        return uno::Any(&aCamGeo, ::getCppuType((const drawing::CameraGeometry*)0) );
    }
    else
    {
        return SvxShape::getPropertyValue(PropertyName);
    }
}

// ::com::sun::star::lang::XServiceInfo
uno::Sequence< OUString > SAL_CALL Svx3DSceneObject::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSeq( SvxShape::getSupportedServiceNames() );
    SvxServiceInfoHelper::addToSequence( aSeq, 1, "com.sun.star.drawing.Shape3DScene" );
    return aSeq;
}

/***********************************************************************
*                                                                      *
***********************************************************************/

//----------------------------------------------------------------------
Svx3DCubeObject::Svx3DCubeObject( SdrObject* pObj ) throw()
:   SvxShape( pObj, aSvxMapProvider.GetMap(SVXMAP_3DCUBEOBJEKT) )
{
}

//----------------------------------------------------------------------
Svx3DCubeObject::~Svx3DCubeObject() throw()
{
}

//----------------------------------------------------------------------
void SAL_CALL Svx3DCubeObject::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TRANSFORM_MATRIX)) )
    {
        // Transformationsmatrix in das Objekt packen
        HOMOGEN_MATRIX_TO_OBJECT
    }
    else if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_POS)) )
    {
        // Position in das Objekt packen
        drawing::Position3D aUnoPos;
        if( aValue >>= aUnoPos )
        {
            basegfx::B3DPoint aPos(aUnoPos.PositionX, aUnoPos.PositionY, aUnoPos.PositionZ);
            ((E3dCubeObj*)mpObj.get())->SetCubePos(aPos);
        }
    }
    else if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_SIZE)) )
    {
        // Groesse in das Objekt packen
        drawing::Direction3D aDirection;
        if( aValue >>= aDirection )
        {
            basegfx::B3DVector aSize(aDirection.DirectionX, aDirection.DirectionY, aDirection.DirectionZ);
            ((E3dCubeObj*)mpObj.get())->SetCubeSize(aSize);
        }
    }
    else if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_POS_IS_CENTER)) )
    {
        // sal_Bool bPosIsCenter in das Objekt packen
        if( aValue.getValueType() == ::getCppuBooleanType() )
        {
            sal_Bool bNew = *(sal_Bool*)aValue.getValue();
            ((E3dCubeObj*)mpObj.get())->SetPosIsCenter(bNew);
        }
    }
    else
    {
        SvxShape::setPropertyValue( aPropertyName, aValue );
    }
}

//----------------------------------------------------------------------
uno::Any SAL_CALL Svx3DCubeObject::getPropertyValue( const OUString& aPropertyName )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TRANSFORM_MATRIX)) )
    {
        // Transformation in eine homogene Matrix packen
        OBJECT_TO_HOMOGEN_MATRIX
    }
    else if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_POS)) )
    {
        // Position packen
        const basegfx::B3DPoint& rPos = ((E3dCubeObj*)mpObj.get())->GetCubePos();
        drawing::Position3D aPos;

        aPos.PositionX = rPos.getX();
        aPos.PositionY = rPos.getY();
        aPos.PositionZ = rPos.getZ();

        return uno::Any( &aPos, ::getCppuType((const drawing::Position3D*)0) );
    }
    else if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_SIZE)) )
    {
        // Groesse packen
        const basegfx::B3DVector& rSize = ((E3dCubeObj*)mpObj.get())->GetCubeSize();
        drawing::Direction3D aDir;

        aDir.DirectionX = rSize.getX();
        aDir.DirectionY = rSize.getY();
        aDir.DirectionZ = rSize.getZ();

        return uno::Any( &aDir, ::getCppuType((const drawing::Direction3D*)0) );
    }
    else if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_POS_IS_CENTER)) )
    {
        // sal_Bool bPosIsCenter packen
        sal_Bool bIsCenter = ((E3dCubeObj*)mpObj.get())->GetPosIsCenter();
        return uno::Any( &bIsCenter, ::getCppuBooleanType() );
    }
    else
    {
        return SvxShape::getPropertyValue(aPropertyName);
    }
}

// ::com::sun::star::lang::XServiceInfo
uno::Sequence< OUString > SAL_CALL Svx3DCubeObject::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSeq( SvxShape::getSupportedServiceNames() );
    SvxServiceInfoHelper::addToSequence( aSeq, 2, "com.sun.star.drawing.Shape3D",
                            "com.sun.star.drawing.Shape3DCube");
    return aSeq;
}

/***********************************************************************
*                                                                      *
***********************************************************************/

//----------------------------------------------------------------------
Svx3DSphereObject::Svx3DSphereObject( SdrObject* pObj ) throw()
:   SvxShape( pObj, aSvxMapProvider.GetMap(SVXMAP_3DSPHEREOBJECT) )
{
}

//----------------------------------------------------------------------
Svx3DSphereObject::~Svx3DSphereObject() throw()
{
}

//----------------------------------------------------------------------
void SAL_CALL Svx3DSphereObject::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TRANSFORM_MATRIX)))
    {
        // Transformationsmatrix in das Objekt packen
        HOMOGEN_MATRIX_TO_OBJECT
    }
    else if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_POS)))
    {
        // Position in das Objekt packen
        drawing::Position3D aUnoPos;
        if( aValue >>= aUnoPos )
        {
            basegfx::B3DPoint aPos(aUnoPos.PositionX, aUnoPos.PositionY, aUnoPos.PositionZ);
            ((E3dSphereObj*)mpObj.get())->SetCenter(aPos);
        }
    }
    else if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_SIZE)))
    {
        // Groesse in das Objekt packen
        drawing::Direction3D aDir;
        if( aValue >>= aDir )
        {
            basegfx::B3DVector aPos(aDir.DirectionX, aDir.DirectionY, aDir.DirectionZ);
            ((E3dSphereObj*)mpObj.get())->SetSize(aPos);
        }
    }
    else
    {
        SvxShape::setPropertyValue(aPropertyName, aValue);
    }
}

//----------------------------------------------------------------------
uno::Any SAL_CALL Svx3DSphereObject::getPropertyValue( const OUString& aPropertyName )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TRANSFORM_MATRIX)))
    {
        // Transformation in eine homogene Matrix packen
        OBJECT_TO_HOMOGEN_MATRIX
    }
    else if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_POS)))
    {
        // Position packen
        const basegfx::B3DPoint& rPos = ((E3dSphereObj*)mpObj.get())->Center();
        drawing::Position3D aPos;

        aPos.PositionX = rPos.getX();
        aPos.PositionY = rPos.getY();
        aPos.PositionZ = rPos.getZ();

        return uno::Any( &aPos, ::getCppuType((const drawing::Position3D*)0) );
    }
    else if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_SIZE)))
    {
        // Groesse packen
        const basegfx::B3DVector& rSize = ((E3dSphereObj*)mpObj.get())->Size();
        drawing::Direction3D aDir;

        aDir.DirectionX = rSize.getX();
        aDir.DirectionY = rSize.getY();
        aDir.DirectionZ = rSize.getZ();

        return uno::Any( &aDir, ::getCppuType((const drawing::Direction3D*)0) );
    }
    else
    {
        return SvxShape::getPropertyValue(aPropertyName);
    }
}

// ::com::sun::star::lang::XServiceInfo
uno::Sequence< OUString > SAL_CALL Svx3DSphereObject::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSeq( SvxShape::getSupportedServiceNames() );
    SvxServiceInfoHelper::addToSequence( aSeq, 2, "com.sun.star.drawing.Shape3D",
                            "com.sun.star.drawing.Shape3DSphere");
    return aSeq;
}

/***********************************************************************
*                                                                      *
***********************************************************************/

//----------------------------------------------------------------------
Svx3DLatheObject::Svx3DLatheObject( SdrObject* pObj ) throw()
:   SvxShape( pObj, aSvxMapProvider.GetMap(SVXMAP_3DLATHEOBJECT) )
{
}

//----------------------------------------------------------------------
Svx3DLatheObject::~Svx3DLatheObject() throw()
{
}

#define POLYPOLYGONSHAPE3D_TO_B3DPOLYPOLYGON(aResultPolygon) \
    drawing::PolyPolygonShape3D aSourcePolyPolygon; \
    if( !(aValue >>= aSourcePolyPolygon) ) \
        throw lang::IllegalArgumentException(); \
    sal_Int32 nOuterSequenceCount = aSourcePolyPolygon.SequenceX.getLength(); \
    if(nOuterSequenceCount != aSourcePolyPolygon.SequenceY.getLength() \
        || nOuterSequenceCount != aSourcePolyPolygon.SequenceZ.getLength()) \
    { \
        throw lang::IllegalArgumentException(); \
    } \
    basegfx::B3DPolyPolygon aResultPolygon; \
    drawing::DoubleSequence* pInnerSequenceX = aSourcePolyPolygon.SequenceX.getArray(); \
    drawing::DoubleSequence* pInnerSequenceY = aSourcePolyPolygon.SequenceY.getArray(); \
    drawing::DoubleSequence* pInnerSequenceZ = aSourcePolyPolygon.SequenceZ.getArray(); \
    for(sal_Int32 a(0L);a<nOuterSequenceCount;a++) \
    { \
        sal_Int32 nInnerSequenceCount = pInnerSequenceX->getLength(); \
        if(nInnerSequenceCount != pInnerSequenceY->getLength() \
            || nInnerSequenceCount != pInnerSequenceZ->getLength()) \
        { \
            throw lang::IllegalArgumentException(); \
        } \
        basegfx::B3DPolygon aNewPolygon; \
        double* pArrayX = pInnerSequenceX->getArray(); \
        double* pArrayY = pInnerSequenceY->getArray(); \
        double* pArrayZ = pInnerSequenceZ->getArray(); \
        for(sal_Int32 b(0L);b<nInnerSequenceCount;b++) \
        { \
            aNewPolygon.append(basegfx::B3DPoint(*pArrayX++,*pArrayY++,*pArrayZ++)); \
        } \
        pInnerSequenceX++; \
        pInnerSequenceY++; \
        pInnerSequenceZ++; \
        basegfx::tools::checkClosed(aNewPolygon); \
        aResultPolygon.append(aNewPolygon); \
    } \

#define B3DPOLYPOLYGON_TO_POLYPOLYGONSHAPE3D(rSourcePolyPolygon) \
    drawing::PolyPolygonShape3D aRetval; \
    aRetval.SequenceX.realloc(rSourcePolyPolygon.count()); \
    aRetval.SequenceY.realloc(rSourcePolyPolygon.count()); \
    aRetval.SequenceZ.realloc(rSourcePolyPolygon.count()); \
    drawing::DoubleSequence* pOuterSequenceX = aRetval.SequenceX.getArray(); \
    drawing::DoubleSequence* pOuterSequenceY = aRetval.SequenceY.getArray(); \
    drawing::DoubleSequence* pOuterSequenceZ = aRetval.SequenceZ.getArray(); \
    for(sal_uInt32 a(0L);a<rSourcePolyPolygon.count();a++) \
    { \
        const basegfx::B3DPolygon aPoly(rSourcePolyPolygon.getB3DPolygon(a)); \
        sal_Int32 nPointCount(aPoly.count()); \
        if(aPoly.isClosed()) nPointCount++; \
        pOuterSequenceX->realloc(nPointCount); \
        pOuterSequenceY->realloc(nPointCount); \
        pOuterSequenceZ->realloc(nPointCount); \
        double* pInnerSequenceX = pOuterSequenceX->getArray(); \
        double* pInnerSequenceY = pOuterSequenceY->getArray(); \
        double* pInnerSequenceZ = pOuterSequenceZ->getArray(); \
        for(sal_uInt32 b(0L);b<aPoly.count();b++) \
        { \
            const basegfx::B3DPoint aPoint(aPoly.getB3DPoint(b)); \
            *pInnerSequenceX++ = aPoint.getX(); \
            *pInnerSequenceY++ = aPoint.getY(); \
            *pInnerSequenceZ++ = aPoint.getZ(); \
        } \
        if(aPoly.isClosed()) \
        { \
            const basegfx::B3DPoint aPoint(aPoly.getB3DPoint(0L)); \
            *pInnerSequenceX++ = aPoint.getX(); \
            *pInnerSequenceY++ = aPoint.getY(); \
            *pInnerSequenceZ++ = aPoint.getZ(); \
        } \
        pOuterSequenceX++; \
        pOuterSequenceY++; \
        pOuterSequenceZ++; \
    } \
    return uno::Any( &aRetval, ::getCppuType((const drawing::PolyPolygonShape3D*)0) ); \

//----------------------------------------------------------------------
void SAL_CALL Svx3DLatheObject::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TRANSFORM_MATRIX)) )
    {
        // Transformationsmatrix in das Objekt packen
        HOMOGEN_MATRIX_TO_OBJECT
    }
    else if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_POLYPOLYGON3D)) )
    {
        // Polygondefinition in das Objekt packen
        POLYPOLYGONSHAPE3D_TO_B3DPOLYPOLYGON(aNewB3DPolyPolygon)

        // #105127# SetPolyPoly3D sets the Svx3DVerticalSegmentsItem to the number
        // of points of the polygon. Thus, value gets lost. To avoid this, rescue
        // item here and re-set after setting the polygon.
        const sal_uInt32 nPrevVerticalSegs(((E3dLatheObj*)mpObj.get())->GetVerticalSegments());

        // Polygon setzen
        const basegfx::B3DHomMatrix aIdentity;
        const basegfx::B2DPolyPolygon aB2DPolyPolygon(basegfx::tools::createB2DPolyPolygonFromB3DPolyPolygon(aNewB3DPolyPolygon, aIdentity));
        ((E3dLatheObj*)mpObj.get())->SetPolyPoly2D(aB2DPolyPolygon);
        const sal_uInt32 nPostVerticalSegs(((E3dLatheObj*)mpObj.get())->GetVerticalSegments());

        if(nPrevVerticalSegs != nPostVerticalSegs)
        {
            // restore the vertical segment count
            ((E3dLatheObj*)mpObj.get())->SetMergedItem(Svx3DVerticalSegmentsItem(nPrevVerticalSegs));
        }
    }
    else
    {
        SvxShape::setPropertyValue(aPropertyName, aValue);
    }
}

//----------------------------------------------------------------------
uno::Any SAL_CALL Svx3DLatheObject::getPropertyValue( const OUString& aPropertyName )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TRANSFORM_MATRIX)) )
    {
        // Transformation in eine homogene Matrix packen
        drawing::HomogenMatrix aHomMat;
        basegfx::B3DHomMatrix aMat = ((E3dObject*)mpObj.get())->GetTransform();

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

        return uno::Any( &aHomMat, ::getCppuType((const drawing::HomogenMatrix*)0) );
    }
    else if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_POLYPOLYGON3D)) )
    {
        const basegfx::B2DPolyPolygon& rPolyPoly = ((E3dLatheObj*)mpObj.get())->GetPolyPoly2D();
        const basegfx::B3DPolyPolygon aB3DPolyPolygon(basegfx::tools::createB3DPolyPolygonFromB2DPolyPolygon(rPolyPoly));

        B3DPOLYPOLYGON_TO_POLYPOLYGONSHAPE3D(aB3DPolyPolygon)
    }
    else
    {
        return SvxShape::getPropertyValue(aPropertyName);
    }
}

// ::com::sun::star::lang::XServiceInfo
uno::Sequence< OUString > SAL_CALL Svx3DLatheObject::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSeq( SvxShape::getSupportedServiceNames() );
    SvxServiceInfoHelper::addToSequence( aSeq, 2, "com.sun.star.drawing.Shape3D",
                            "com.sun.star.drawing.Shape3DLathe");
    return aSeq;
}

/***********************************************************************
*                                                                      *
***********************************************************************/

Svx3DExtrudeObject::Svx3DExtrudeObject( SdrObject* pObj ) throw()
:   SvxShape( pObj, aSvxMapProvider.GetMap(SVXMAP_3DEXTRUDEOBJECT) )
{
}

//----------------------------------------------------------------------
Svx3DExtrudeObject::~Svx3DExtrudeObject() throw()
{
}

//----------------------------------------------------------------------
void SAL_CALL Svx3DExtrudeObject::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TRANSFORM_MATRIX)) )
    {
        // Transformationsmatrix in das Objekt packen
        HOMOGEN_MATRIX_TO_OBJECT
    }
    else if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_POLYPOLYGON3D)) )
    {
        // Polygondefinition in das Objekt packen
        POLYPOLYGONSHAPE3D_TO_B3DPOLYPOLYGON(aNewB3DPolyPolygon)

        // Polygon setzen
        const basegfx::B3DHomMatrix aIdentity;
        const basegfx::B2DPolyPolygon aB2DPolyPolygon(basegfx::tools::createB2DPolyPolygonFromB3DPolyPolygon(aNewB3DPolyPolygon, aIdentity));
        ((E3dExtrudeObj*)mpObj.get())->SetExtrudePolygon(aB2DPolyPolygon);
    }
    else
    {
        SvxShape::setPropertyValue(aPropertyName, aValue);
    }
}

//----------------------------------------------------------------------
uno::Any SAL_CALL Svx3DExtrudeObject::getPropertyValue( const OUString& aPropertyName )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TRANSFORM_MATRIX)) )
    {
        // Transformation in eine homogene Matrix packen
        drawing::HomogenMatrix aHomMat;
        basegfx::B3DHomMatrix aMat = ((E3dObject*)mpObj.get())->GetTransform();

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

        return uno::Any( &aHomMat, ::getCppuType((const drawing::HomogenMatrix*)0) );
    }
    else if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_POLYPOLYGON3D)) )
    {
        // Polygondefinition packen
        const basegfx::B2DPolyPolygon& rPolyPoly = ((E3dExtrudeObj*)mpObj.get())->GetExtrudePolygon();
        const basegfx::B3DPolyPolygon aB3DPolyPolygon(basegfx::tools::createB3DPolyPolygonFromB2DPolyPolygon(rPolyPoly));

        B3DPOLYPOLYGON_TO_POLYPOLYGONSHAPE3D(aB3DPolyPolygon)
    }
    else
    {
        return SvxShape::getPropertyValue(aPropertyName);
    }
}

// ::com::sun::star::lang::XServiceInfo
uno::Sequence< OUString > SAL_CALL Svx3DExtrudeObject::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSeq( SvxShape::getSupportedServiceNames() );
    SvxServiceInfoHelper::addToSequence( aSeq, 2, "com.sun.star.drawing.Shape3D",
                            "com.sun.star.drawing.Shape3DExtrude");
    return aSeq;
}

/***********************************************************************
*                                                                      *
***********************************************************************/

//----------------------------------------------------------------------
Svx3DPolygonObject::Svx3DPolygonObject( SdrObject* pObj ) throw()
:   SvxShape( pObj, aSvxMapProvider.GetMap(SVXMAP_3DPOLYGONOBJECT) )
{
}

//----------------------------------------------------------------------
Svx3DPolygonObject::~Svx3DPolygonObject() throw()
{
}

//----------------------------------------------------------------------
void SAL_CALL Svx3DPolygonObject::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TRANSFORM_MATRIX)) )
    {
        // Transformationsmatrix in das Objekt packen
        HOMOGEN_MATRIX_TO_OBJECT
    }
    else if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_POLYPOLYGON3D)) )
    {
        // Polygondefinition in das Objekt packen
        POLYPOLYGONSHAPE3D_TO_B3DPOLYPOLYGON(aNewB3DPolyPolygon)

        // Polygon setzen
        ((E3dPolygonObj*)mpObj.get())->SetPolyPolygon3D(aNewB3DPolyPolygon);
    }
    else if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_NORMALSPOLYGON3D)) )
    {
        // Normalendefinition in das Objekt packen
        POLYPOLYGONSHAPE3D_TO_B3DPOLYPOLYGON(aNewB3DPolyPolygon)

        // Polygon setzen
        ((E3dPolygonObj*)mpObj.get())->SetPolyNormals3D(aNewB3DPolyPolygon);
    }
    else if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TEXTUREPOLYGON3D)) )
    {
        // Texturdefinition in das Objekt packen
        POLYPOLYGONSHAPE3D_TO_B3DPOLYPOLYGON(aNewB3DPolyPolygon)

        // Polygon setzen
        const basegfx::B3DHomMatrix aIdentity;
        const basegfx::B2DPolyPolygon aB2DPolyPolygon(basegfx::tools::createB2DPolyPolygonFromB3DPolyPolygon(aNewB3DPolyPolygon, aIdentity));
        ((E3dPolygonObj*)mpObj.get())->SetPolyTexture2D(aB2DPolyPolygon);
    }
    else if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_LINEONLY)) )
    {
        // sal_Bool bLineOnly in das Objekt packen
        if( aValue.getValueType() == ::getCppuBooleanType() )
        {
            sal_Bool bNew = *(sal_Bool*)aValue.getValue();
            ((E3dPolygonObj*)mpObj.get())->SetLineOnly(bNew);
        }
    }
    else
    {
        SvxShape::setPropertyValue(aPropertyName, aValue);
    }
}

//----------------------------------------------------------------------
uno::Any SAL_CALL Svx3DPolygonObject::getPropertyValue( const OUString& aPropertyName )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TRANSFORM_MATRIX)) )
    {
        // Transformation in eine homogene Matrix packen
        OBJECT_TO_HOMOGEN_MATRIX
    }
    else if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_POLYPOLYGON3D)) )
    {
        // Polygondefinition packen
        const basegfx::B3DPolyPolygon& rPolyPoly = ((E3dPolygonObj*)mpObj.get())->GetPolyPolygon3D();

        B3DPOLYPOLYGON_TO_POLYPOLYGONSHAPE3D(rPolyPoly)
    }
    else if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_NORMALSPOLYGON3D)) )
    {
        // Normalendefinition packen
        const basegfx::B3DPolyPolygon& rPolyPoly = ((E3dPolygonObj*)mpObj.get())->GetPolyNormals3D();

        B3DPOLYPOLYGON_TO_POLYPOLYGONSHAPE3D(rPolyPoly)
    }
    else if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TEXTUREPOLYGON3D)) )
    {
        // Texturdefinition packen
        const basegfx::B2DPolyPolygon& rPolyPoly = ((E3dPolygonObj*)mpObj.get())->GetPolyTexture2D();
        const basegfx::B3DPolyPolygon aB3DPolyPolygon(basegfx::tools::createB3DPolyPolygonFromB2DPolyPolygon(rPolyPoly));

        B3DPOLYPOLYGON_TO_POLYPOLYGONSHAPE3D(aB3DPolyPolygon)
    }
    else if(mpObj.is() && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_LINEONLY)) )
    {
        // sal_Bool bLineOnly packen
        sal_Bool bNew = ((E3dPolygonObj*)mpObj.get())->GetLineOnly();
        return uno::Any( &bNew, ::getCppuBooleanType() );
    }
    else
    {
        return SvxShape::getPropertyValue(aPropertyName);
    }
}

// ::com::sun::star::lang::XServiceInfo
uno::Sequence< OUString > SAL_CALL Svx3DPolygonObject::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    Sequence< OUString > aSeq( SvxShape::getSupportedServiceNames() );
    SvxServiceInfoHelper::addToSequence( aSeq, 2, "com.sun.star.drawing.Shape3D",
                            "com.sun.star.drawing.Shape3DPolygon");
    return aSeq;
}

// eof

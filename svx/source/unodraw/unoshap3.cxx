/*************************************************************************
 *
 *  $RCSfile: unoshap3.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:27 $
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
#include "poly3d.hxx"

using namespace ::vos;
using namespace ::rtl;
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
uno::Sequence< uno::Type > Svx3DSceneObject::maTypeSequence;

//----------------------------------------------------------------------
Svx3DSceneObject::Svx3DSceneObject( SdrObject* pObj, SvxDrawPage* pDrawPage ) throw()
:   SvxShape( pObj, aSvxMapProvider.GetMap(SVXMAP_3DSCENEOBJECT) ), pPage( pDrawPage )
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
    pPage = pNewPage;
}

//----------------------------------------------------------------------
uno::Any SAL_CALL Svx3DSceneObject::queryAggregation( const uno::Type & rType )
    throw(uno::RuntimeException)
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

void SAL_CALL Svx3DSceneObject::acquire() throw( uno::RuntimeException )
{
    SvxShape::acquire();
}

void SAL_CALL Svx3DSceneObject::release() throw( uno::RuntimeException )
{
    SvxShape::release();
}

// XTypeProvider

uno::Sequence< uno::Type > SAL_CALL Svx3DSceneObject::getTypes()
    throw (uno::RuntimeException)
{
    if( maTypeSequence.getLength() == 0 )
    {
        const uno::Sequence< uno::Type > aBaseTypes( SvxShape::getTypes() );
        const uno::Type* pBaseTypes = aBaseTypes.getConstArray();
        const sal_Int32 nBaseTypes = aBaseTypes.getLength();
        const sal_Int32 nOwnTypes = 1;      // !DANGER! Keep this updated!

        maTypeSequence.realloc( nBaseTypes  + nOwnTypes );
        uno::Type* pTypes = maTypeSequence.getArray();

        *pTypes++ = ::getCppuType((const uno::Reference< drawing::XShapes>*)0);

        for( sal_Int32 nType = 0; nType < nBaseTypes; nType++ )
            *pTypes++ = *pBaseTypes++;
    }
    return maTypeSequence;
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

    if(pObj == NULL || pPage == NULL || pShape == NULL || NULL != pShape->GetSdrObject() )
        throw uno::RuntimeException();

    SdrObject* pSdrShape = pPage->_CreateSdrObject( xShape );
    if( pSdrShape->ISA(E3dObject) )
    {
        pObj->GetSubList()->NbcInsertObject( pSdrShape );
    }
    else
    {
        delete pSdrShape;
        pShape->InvalidateSdrObject();
        throw uno::RuntimeException();
    }
}

//----------------------------------------------------------------------
void SAL_CALL Svx3DSceneObject::remove( const Reference< drawing::XShape >& xShape )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    SvxShape* pShape = SvxShape::getImplementation( xShape );

    if(pObj == NULL || pShape == NULL)
        throw uno::RuntimeException();

    SdrObject* pSdrShape = pShape->GetSdrObject();
    if(pSdrShape == NULL || pSdrShape->GetObjList()->GetOwnerObj() != pObj)
    {
        throw uno::RuntimeException();
    }
    else
    {
        SdrObjList& rList = *pSdrShape->GetObjList();

        const nObjCount = rList.GetObjCount();
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

    if(pObj && pObj->ISA(E3dPolyScene) && pObj->GetSubList())
        nRetval = pObj->GetSubList()->GetObjCount();
    return nRetval;
}

//----------------------------------------------------------------------

uno::Any SAL_CALL Svx3DSceneObject::getByIndex( sal_Int32 Index )
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( pObj == NULL || pObj->GetSubList() == NULL )
        throw uno::RuntimeException();

    if( pObj->GetSubList()->GetObjCount() <= Index )
        throw lang::IndexOutOfBoundsException();

    Reference< drawing::XShape > xShape;
    SdrObject* pDestObj = pObj->GetSubList()->GetObj( Index );

    if(pDestObj)
        xShape = pPage->_CreateShape(pDestObj);
    else
        throw lang::IndexOutOfBoundsException();

    return uno::Any( &xShape, ::getCppuType((const Reference< drawing::XShape>*)0) );
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

    return pObj && pObj->GetSubList() && (pObj->GetSubList()->GetObjCount() > 0);
}

//----------------------------------------------------------------------
#define HOMOGEN_MATRIX_TO_OBJECT \
    drawing::HomogenMatrix m; \
    if( aValue >>= m ) { \
        Matrix4D aMat; \
        aMat[0][0] = m.Line1.Column1; \
        aMat[0][1] = m.Line1.Column2; \
        aMat[0][2] = m.Line1.Column3; \
        aMat[0][3] = m.Line1.Column4; \
        aMat[1][0] = m.Line2.Column1; \
        aMat[1][1] = m.Line2.Column2; \
        aMat[1][2] = m.Line2.Column3; \
        aMat[1][3] = m.Line2.Column4; \
        aMat[2][0] = m.Line3.Column1; \
        aMat[2][1] = m.Line3.Column2; \
        aMat[2][2] = m.Line3.Column3; \
        aMat[2][3] = m.Line3.Column4; \
        aMat[3][0] = m.Line4.Column1; \
        aMat[3][1] = m.Line4.Column2; \
        aMat[3][2] = m.Line4.Column3; \
        aMat[3][3] = m.Line4.Column4; \
        ((E3dObject*)pObj)->SetTransform(aMat); \
    }

#define OBJECT_TO_HOMOGEN_MATRIX \
    drawing::HomogenMatrix aHomMat; \
    const Matrix4D& rMat = ((E3dObject*)pObj)->GetTransform(); \
    aHomMat.Line1.Column1 = rMat[0][0]; \
    aHomMat.Line1.Column2 = rMat[0][1]; \
    aHomMat.Line1.Column3 = rMat[0][2]; \
    aHomMat.Line1.Column4 = rMat[0][3]; \
    aHomMat.Line2.Column1 = rMat[1][0]; \
    aHomMat.Line2.Column2 = rMat[1][1]; \
    aHomMat.Line2.Column3 = rMat[1][2]; \
    aHomMat.Line2.Column4 = rMat[1][3]; \
    aHomMat.Line3.Column1 = rMat[2][0]; \
    aHomMat.Line3.Column2 = rMat[2][1]; \
    aHomMat.Line3.Column3 = rMat[2][2]; \
    aHomMat.Line3.Column4 = rMat[2][3]; \
    aHomMat.Line4.Column1 = rMat[3][0]; \
    aHomMat.Line4.Column2 = rMat[3][1]; \
    aHomMat.Line4.Column3 = rMat[3][2]; \
    aHomMat.Line4.Column4 = rMat[3][3]; \
    return uno::Any( &aHomMat, ::getCppuType((const drawing::HomogenMatrix*)0) );

//----------------------------------------------------------------------
void SAL_CALL Svx3DSceneObject::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TRANSFORM_MATRIX)) == 0 )
    {
        // Transformationsmatrix in das Objekt packen
        HOMOGEN_MATRIX_TO_OBJECT
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

    if(pObj && PropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TRANSFORM_MATRIX)) == 0 )
    {
        // Objekt in eine homogene 4x4 Matrix packen
        OBJECT_TO_HOMOGEN_MATRIX
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
    addToSequence( aSeq, 1, "com.sun.star.drawing.Shape3DScene" );
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

    if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TRANSFORM_MATRIX)) == 0 )
    {
        // Transformationsmatrix in das Objekt packen
        HOMOGEN_MATRIX_TO_OBJECT
    }
    else if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_POS)) == 0 )
    {
        // Position in das Objekt packen
        drawing::Position3D aUnoPos;
        if( aValue >>= aUnoPos )
        {
            Vector3D aPos(aUnoPos.PositionX, aUnoPos.PositionY, aUnoPos.PositionZ);
            ((E3dCubeObj*)pObj)->SetCubePos(aPos);
        }
    }
    else if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_SIZE)) == 0)
    {
        // Groesse in das Objekt packen
        drawing::Direction3D aDirection;
        if( aValue >>= aDirection )
        {
            Vector3D aPos(aDirection.DirectionX, aDirection.DirectionY, aDirection.DirectionZ);
            ((E3dCubeObj*)pObj)->SetCubePos(aPos);
        }
    }
    else if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_POS_IS_CENTER)) == 0 )
    {
        // sal_Bool bPosIsCenter in das Objekt packen
        if( aValue.getValueType() == ::getCppuBooleanType() )
        {
            sal_Bool bNew = *(sal_Bool*)aValue.getValue();
            ((E3dCubeObj*)pObj)->SetPosIsCenter(bNew);
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

    if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TRANSFORM_MATRIX)) == 0)
    {
        // Transformation in eine homogene Matrix packen
        OBJECT_TO_HOMOGEN_MATRIX
    }
    else if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_POS)) == 0 )
    {
        // Position packen
        const Vector3D& rPos = ((E3dCubeObj*)pObj)->GetCubePos();
        drawing::Position3D aPos;

        aPos.PositionX = rPos.X();
        aPos.PositionY = rPos.Y();
        aPos.PositionZ = rPos.Z();

        return uno::Any( &aPos, ::getCppuType((const drawing::Position3D*)0) );
    }
    else if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_SIZE)) == 0 )
    {
        // Groesse packen
        const Vector3D& rSize = ((E3dCubeObj*)pObj)->GetCubeSize();
        drawing::Direction3D aDir;

        aDir.DirectionX = rSize.X();
        aDir.DirectionY = rSize.Y();
        aDir.DirectionZ = rSize.Z();

        return uno::Any( &aDir, ::getCppuType((const drawing::Direction3D*)0) );
    }
    else if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_POS_IS_CENTER)) == 0 )
    {
        // sal_Bool bPosIsCenter packen
        sal_Bool bIsCenter = ((E3dCubeObj*)pObj)->GetPosIsCenter();
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
    addToSequence( aSeq, 2, "com.sun.star.drawing.Shape3D",
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

    if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TRANSFORM_MATRIX)) == 0)
    {
        // Transformationsmatrix in das Objekt packen
        HOMOGEN_MATRIX_TO_OBJECT
    }
    else if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_POS)) == 0)
    {
        // Position in das Objekt packen
        drawing::Position3D aUnoPos;
        if( aValue >>= aUnoPos )
        {
            Vector3D aPos(aUnoPos.PositionX, aUnoPos.PositionY, aUnoPos.PositionZ);
            ((E3dSphereObj*)pObj)->SetCenter(aPos);
        }
    }
    else if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_SIZE)) == 0)
    {
        // Groesse in das Objekt packen
        drawing::Direction3D aDir;
        if( aValue >>= aDir )
        {
            Vector3D aPos(aDir.DirectionX, aDir.DirectionY, aDir.DirectionZ);
            ((E3dSphereObj*)pObj)->SetSize(aPos);
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

    if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TRANSFORM_MATRIX)) == 0)
    {
        // Transformation in eine homogene Matrix packen
        OBJECT_TO_HOMOGEN_MATRIX
    }
    else if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_POS)) == 0)
    {
        // Position packen
        const Vector3D& rPos = ((E3dSphereObj*)pObj)->Center();
        drawing::Position3D aPos;

        aPos.PositionX = rPos.X();
        aPos.PositionY = rPos.Y();
        aPos.PositionZ = rPos.Z();

        return uno::Any( &aPos, ::getCppuType((const drawing::Position3D*)0) );
    }
    else if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_SIZE)) == 0)
    {
        // Groesse packen
        const Vector3D& rSize = ((E3dSphereObj*)pObj)->Size();
        drawing::Direction3D aDir;

        aDir.DirectionX = rSize.X();
        aDir.DirectionY = rSize.Y();
        aDir.DirectionZ = rSize.Z();

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
    addToSequence( aSeq, 2, "com.sun.star.drawing.Shape3D",
                            "com.sun.star.drawing.Shape3DSphere");
    return aSeq;
}

/***********************************************************************
*                                                                      *
***********************************************************************/

#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONSHAPE3D_HPP_
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#endif



//----------------------------------------------------------------------
Svx3DLatheObject::Svx3DLatheObject( SdrObject* pObj ) throw()
:   SvxShape( pObj, aSvxMapProvider.GetMap(SVXMAP_3DLATHEOBJECT) )
{
}

//----------------------------------------------------------------------
Svx3DLatheObject::~Svx3DLatheObject() throw()
{
}

#define POLYPOLYGONSHAPE3D_TO_POLYPOLYGON3D \
    drawing::PolyPolygonShape3D aSourcePolyPolygon; \
    if( !(aValue >>= aSourcePolyPolygon) ) \
        throw lang::IllegalArgumentException(); \
    sal_Int32 nOuterSequenceCount = aSourcePolyPolygon.SequenceX.getLength(); \
    if(nOuterSequenceCount != aSourcePolyPolygon.SequenceY.getLength() \
        || nOuterSequenceCount != aSourcePolyPolygon.SequenceZ.getLength()) \
    { \
        throw lang::IllegalArgumentException(); \
    } \
    PolyPolygon3D aNewPolyPolygon; \
    drawing::DoubleSequence* pInnerSequenceX = aSourcePolyPolygon.SequenceX.getArray(); \
    drawing::DoubleSequence* pInnerSequenceY = aSourcePolyPolygon.SequenceY.getArray(); \
    drawing::DoubleSequence* pInnerSequenceZ = aSourcePolyPolygon.SequenceZ.getArray(); \
    for(sal_Int32 a=0;a<nOuterSequenceCount;a++) \
    { \
        sal_Int32 nInnerSequenceCount = pInnerSequenceX->getLength(); \
        if(nInnerSequenceCount != pInnerSequenceY->getLength() \
            || nInnerSequenceCount != pInnerSequenceZ->getLength()) \
        { \
            throw lang::IllegalArgumentException(); \
        } \
        Polygon3D aNewPolygon; \
        double* pArrayX = pInnerSequenceX->getArray(); \
        double* pArrayY = pInnerSequenceY->getArray(); \
        double* pArrayZ = pInnerSequenceZ->getArray(); \
        for(sal_Int32 b=0;b<nInnerSequenceCount;b++) \
        { \
            aNewPolygon[b].X() = *pArrayX++; \
            aNewPolygon[b].Y() = *pArrayY++; \
            aNewPolygon[b].Z() = *pArrayZ++; \
        } \
        pInnerSequenceX++; \
        pInnerSequenceY++; \
        pInnerSequenceZ++; \
        aNewPolyPolygon.Insert(aNewPolygon); \
    } \

#define POLYPOLYGON3D_TO_POLYPOLYGONSHAPE3D \
    drawing::PolyPolygonShape3D aRetval; \
    aRetval.SequenceX.realloc((sal_Int32)rPolyPoly.Count()); \
    aRetval.SequenceY.realloc((sal_Int32)rPolyPoly.Count()); \
    aRetval.SequenceZ.realloc((sal_Int32)rPolyPoly.Count()); \
    drawing::DoubleSequence* pOuterSequenceX = aRetval.SequenceX.getArray(); \
    drawing::DoubleSequence* pOuterSequenceY = aRetval.SequenceY.getArray(); \
    drawing::DoubleSequence* pOuterSequenceZ = aRetval.SequenceZ.getArray(); \
    for(sal_uInt16 a=0;a<rPolyPoly.Count();a++) \
    { \
        const Polygon3D& rPoly = rPolyPoly[a]; \
        pOuterSequenceX->realloc((sal_Int32)rPoly.GetPointCount()); \
        pOuterSequenceY->realloc((sal_Int32)rPoly.GetPointCount()); \
        pOuterSequenceZ->realloc((sal_Int32)rPoly.GetPointCount()); \
        double* pInnerSequenceX = pOuterSequenceX->getArray(); \
        double* pInnerSequenceY = pOuterSequenceY->getArray(); \
        double* pInnerSequenceZ = pOuterSequenceZ->getArray(); \
        for(sal_uInt16 b=0;b<rPoly.GetPointCount();b++) \
        { \
            *pInnerSequenceX++ = rPoly[b].X(); \
            *pInnerSequenceY++ = rPoly[b].Y(); \
            *pInnerSequenceZ++ = rPoly[b].Z(); \
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

    if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TRANSFORM_MATRIX)) == 0)
    {
        // Transformationsmatrix in das Objekt packen
        HOMOGEN_MATRIX_TO_OBJECT
    }
    else if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_POLYPOLYGON3D)) == 0)
    {
        // Polygondefinition in das Objekt packen
        POLYPOLYGONSHAPE3D_TO_POLYPOLYGON3D

        // Polygon setzen
        ((E3dLatheObj*)pObj)->SetPolyPoly3D(aNewPolyPolygon);
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

    if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TRANSFORM_MATRIX)) == 0 )
    {
        // Transformation in eine homogene Matrix packen
        OBJECT_TO_HOMOGEN_MATRIX
    }
    else if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_POLYPOLYGON3D)) == 0)
    {
        const PolyPolygon3D& rPolyPoly = ((E3dLatheObj*)pObj)->GetPolyPoly3D();

        POLYPOLYGON3D_TO_POLYPOLYGONSHAPE3D
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
    addToSequence( aSeq, 2, "com.sun.star.drawing.Shape3D",
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

    if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TRANSFORM_MATRIX)) == 0)
    {
        // Transformationsmatrix in das Objekt packen
        HOMOGEN_MATRIX_TO_OBJECT
    }
    else if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_POLYPOLYGON3D)) == 0)
    {
        // Polygondefinition in das Objekt packen
        POLYPOLYGONSHAPE3D_TO_POLYPOLYGON3D

        // Polygon setzen
        ((E3dExtrudeObj*)pObj)->SetExtrudePolygon(aNewPolyPolygon);
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

    if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TRANSFORM_MATRIX)) == 0)
    {
        // Transformation in eine homogene Matrix packen
        OBJECT_TO_HOMOGEN_MATRIX
    }
    else if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_POLYPOLYGON3D)) == 0)
    {
        // Polygondefinition packen
        const PolyPolygon3D& rPolyPoly = ((E3dExtrudeObj*)pObj)->GetExtrudePolygon();

        POLYPOLYGON3D_TO_POLYPOLYGONSHAPE3D
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
    addToSequence( aSeq, 2, "com.sun.star.drawing.Shape3D",
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

    if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TRANSFORM_MATRIX)) == 0)
    {
        // Transformationsmatrix in das Objekt packen
        HOMOGEN_MATRIX_TO_OBJECT
    }
    else if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_POLYPOLYGON3D)) == 0)
    {
        // Polygondefinition in das Objekt packen
        POLYPOLYGONSHAPE3D_TO_POLYPOLYGON3D

        // Polygon setzen
        ((E3dPolygonObj*)pObj)->SetPolyPolygon3D(aNewPolyPolygon);
    }
    else if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_NORMALSPOLYGON3D)) == 0)
    {
        // Normalendefinition in das Objekt packen
        POLYPOLYGONSHAPE3D_TO_POLYPOLYGON3D

        // Polygon setzen
        ((E3dPolygonObj*)pObj)->SetPolyNormals3D(aNewPolyPolygon);
    }
    else if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TEXTUREPOLYGON3D)) == 0)
    {
        // Texturdefinition in das Objekt packen
        POLYPOLYGONSHAPE3D_TO_POLYPOLYGON3D

        // Polygon setzen
        ((E3dPolygonObj*)pObj)->SetPolyTexture3D(aNewPolyPolygon);
    }
    else if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_LINEONLY)) == 0)
    {
        // sal_Bool bLineOnly in das Objekt packen
        if( aValue.getValueType() == ::getCppuBooleanType() )
        {
            sal_Bool bNew = *(sal_Bool*)aValue.getValue();
            ((E3dPolygonObj*)pObj)->SetLineOnly(bNew);
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

    if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TRANSFORM_MATRIX)) == 0)
    {
        // Transformation in eine homogene Matrix packen
        OBJECT_TO_HOMOGEN_MATRIX
    }
    else if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_POLYPOLYGON3D)) == 0)
    {
        // Polygondefinition packen
        const PolyPolygon3D& rPolyPoly = ((E3dPolygonObj*)pObj)->GetPolyPolygon3D();

        POLYPOLYGON3D_TO_POLYPOLYGONSHAPE3D
    }
    else if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_NORMALSPOLYGON3D)) == 0)
    {
        // Normalendefinition packen
        const PolyPolygon3D& rPolyPoly = ((E3dPolygonObj*)pObj)->GetPolyNormals3D();

        POLYPOLYGON3D_TO_POLYPOLYGONSHAPE3D
    }
    else if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_TEXTUREPOLYGON3D)) == 0)
    {
        // Texturdefinition packen
        const PolyPolygon3D& rPolyPoly = ((E3dPolygonObj*)pObj)->GetPolyTexture3D();

        POLYPOLYGON3D_TO_POLYPOLYGONSHAPE3D
    }
    else if(pObj && aPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_3D_LINEONLY)) == 0)
    {
        // sal_Bool bLineOnly packen
        sal_Bool bNew = ((E3dPolygonObj*)pObj)->GetLineOnly();
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
    addToSequence( aSeq, 2, "com.sun.star.drawing.Shape3D",
                            "com.sun.star.drawing.Shape3DPolygon");
    return aSeq;
}




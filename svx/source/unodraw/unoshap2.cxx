/*************************************************************************
 *
 *  $RCSfile: unoshap2.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-08 11:37:51 $
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

#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCESEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequence.hpp>
#endif
#ifndef _B2D_MATRIX3D_HXX
#include <goodies/matrix3d.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _FLTCALL_HXX
#include <svtools/fltcall.hxx>
#endif
#ifndef _SVX_IMPGRF_HXX
#include "impgrf.hxx"
#endif

#include <rtl/uuid.h>
#include <rtl/memory.h>
#include <tools/urlobj.hxx>

#include "unoprnms.hxx"
#include "unoshape.hxx"
#include "unopage.hxx"
#include "svdobj.hxx"
#include "svdpage.hxx"
#include "svdmodel.hxx"
#include "svdouno.hxx"

using namespace ::osl;
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

class GDIMetaFile;
class SvStream;
sal_Bool ConvertGDIMetaFileToWMF( const GDIMetaFile & rMTF, SvStream & rTargetStream,
                              PFilterCallback pCallback=NULL, void * pCallerData=NULL,
                              sal_Bool bPlaceable=sal_True);

/***********************************************************************
* class SvxShapeGroup                                                  *
***********************************************************************/
uno::Sequence< uno::Type > SvxShapeGroup::maTypeSequence;

SvxShapeGroup::SvxShapeGroup( SdrObject* pObj, SvxDrawPage* pDrawPage  )  throw() :
    SvxShape( pObj, aSvxMapProvider.GetMap(SVXMAP_GROUP) ),
    pPage( pDrawPage )
{
    if( pPage )
        pPage->acquire();
}

//----------------------------------------------------------------------
SvxShapeGroup::~SvxShapeGroup() throw()
{
    if( pPage )
        pPage->release();
}

//----------------------------------------------------------------------
void SvxShapeGroup::Create( SdrObject* pNewObj, SvxDrawPage* pNewPage ) throw()
{
    if( pPage )
    {
        pPage->release();
        pPage = NULL;
    }

    SvxShape::Create( pNewObj, pNewPage );

    pPage = pNewPage;
    if( pPage )
        pPage->acquire();

}

//----------------------------------------------------------------------
uno::Any SAL_CALL SvxShapeGroup::queryInterface( const uno::Type & rType ) throw(uno::RuntimeException)
{
    return OWeakAggObject::queryInterface( rType );
}

uno::Any SAL_CALL SvxShapeGroup::queryAggregation( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    uno::Any aAny;

    QUERYINT( drawing::XShapeGroup );
    else QUERYINT( drawing::XShapes );
    else QUERYINT( container::XIndexAccess );
    else QUERYINT( container::XElementAccess );
    else
        aAny <<= SvxShape::queryAggregation( rType );

    return aAny;
}

void SAL_CALL SvxShapeGroup::acquire() throw(uno::RuntimeException)
{
    OWeakAggObject::acquire();
}

void SAL_CALL SvxShapeGroup::release() throw(uno::RuntimeException)
{
    OWeakAggObject::release();
}

uno::Sequence< uno::Type > SAL_CALL SvxShapeGroup::getTypes()
    throw (uno::RuntimeException)
{
    if( maTypeSequence.getLength() == 0 )
    {
        const uno::Sequence< uno::Type > aBaseTypes( SvxShape::getTypes() );
        const uno::Type* pBaseTypes = aBaseTypes.getConstArray();
        const sal_Int32 nBaseTypes = aBaseTypes.getLength();
        const sal_Int32 nOwnTypes = 2;      // !DANGER! Keep this updated!

        maTypeSequence.realloc( nBaseTypes  + nOwnTypes );
        uno::Type* pTypes = maTypeSequence.getArray();

        *pTypes++ = ::getCppuType((const uno::Reference< drawing::XShapes>*)0);
        *pTypes++ = ::getCppuType((const uno::Reference< drawing::XShapeGroup>*)0);

        for( sal_Int32 nType = 0; nType < nBaseTypes; nType++ )
            *pTypes++ = *pBaseTypes++;
    }
    return maTypeSequence;
}

uno::Sequence< sal_Int8 > SAL_CALL SvxShapeGroup::getImplementationId()
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

// ::com::sun::star::drawing::XShape

//----------------------------------------------------------------------
OUString SAL_CALL SvxShapeGroup::getShapeType()
    throw( uno::RuntimeException )
{
    return SvxShape::getShapeType();
}

//------------------------------------------------------------------1----
awt::Point SAL_CALL SvxShapeGroup::getPosition() throw(uno::RuntimeException)
{
    return SvxShape::getPosition();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeGroup::setPosition( const awt::Point& Position ) throw(uno::RuntimeException)
{
    SvxShape::setPosition(Position);
}

//----------------------------------------------------------------------

awt::Size SAL_CALL SvxShapeGroup::getSize() throw(uno::RuntimeException)
{
    return SvxShape::getSize();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeGroup::setSize( const awt::Size& rSize )
    throw(beans::PropertyVetoException, uno::RuntimeException)
{
    SvxShape::setSize( rSize );
}

// drawing::XShapeGroup

//----------------------------------------------------------------------
void SAL_CALL SvxShapeGroup::enterGroup(  ) throw(uno::RuntimeException)
{
    // Todo
//  pDrView->EnterMarkedGroup();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeGroup::leaveGroup(  ) throw(uno::RuntimeException)
{
    // Todo
//  pDrView->LeaveOneGroup();
}

//----------------------------------------------------------------------

// XShapes
void SAL_CALL SvxShapeGroup::add( const uno::Reference< drawing::XShape >& xShape )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    SvxShape* pShape = SvxShape::getImplementation( xShape );

    if( pObj != NULL && pPage != NULL || pShape != NULL )
    {
        SdrObject* pSdrShape = pShape->GetSdrObject();
        if( pSdrShape == NULL )
            pSdrShape = pPage->_CreateSdrObject( xShape );

        if( pSdrShape->IsInserted() )
            pSdrShape->GetObjList()->RemoveObject( pSdrShape->GetOrdNum() );

        pObj->GetSubList()->NbcInsertObject( pSdrShape );

        if(pShape)
            pShape->Create( pSdrShape, pPage );

        if( pModel )
            pModel->SetChanged();
    }
    else
    {
        DBG_ERROR("could not add XShape to group shape!");
    }
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeGroup::remove( const uno::Reference< drawing::XShape >& xShape )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    SdrObject* pSdrShape = NULL;
    SvxShape* pShape = SvxShape::getImplementation( xShape );

    if( pShape )
        pSdrShape = pShape->GetSdrObject();

    if(pObj == NULL || pSdrShape == NULL || pSdrShape->GetObjList()->GetOwnerObj() != pObj )
        throw uno::RuntimeException();

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

    if( pModel )
        pModel->SetChanged();
}

// XIndexAccess

//----------------------------------------------------------------------
sal_Int32 SAL_CALL SvxShapeGroup::getCount() throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    sal_Int32 nRetval = 0;

    if(pObj && pObj->GetSubList())
        nRetval = pObj->GetSubList()->GetObjCount();
    else
        throw uno::RuntimeException();

    return nRetval;
}

//----------------------------------------------------------------------
uno::Any SAL_CALL SvxShapeGroup::getByIndex( sal_Int32 Index )
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( pObj == NULL || pObj->GetSubList() == NULL )
        throw uno::RuntimeException();


    if( pObj->GetSubList()->GetObjCount() <= (sal_uInt32)Index )
        throw lang::IndexOutOfBoundsException();

    SdrObject* pDestObj = pObj->GetSubList()->GetObj( Index );

    if(pDestObj == NULL)
        throw lang::IndexOutOfBoundsException();

    Reference< drawing::XShape > xShape( pDestObj->getUnoShape(), uno::UNO_QUERY );
    uno::Any aAny;
    aAny <<= xShape;
    return aAny;
}

// ::com::sun::star::container::XElementAccess

//----------------------------------------------------------------------
uno::Type SAL_CALL SvxShapeGroup::getElementType() throw( uno::RuntimeException )
{
    return ::getCppuType(( const Reference< drawing::XShape >*)0);
}

//----------------------------------------------------------------------
sal_Bool SAL_CALL SvxShapeGroup::hasElements() throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    return pObj && pObj->GetSubList() && (pObj->GetSubList()->GetObjCount() > 0);
}

//----------------------------------------------------------------------
// ::com::sun::star::lang::XServiceInfo

uno::Sequence< OUString > SAL_CALL SvxShapeGroup::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    return SvxShape::getSupportedServiceNames();
}

/***********************************************************************
*                                                                      *
***********************************************************************/
uno::Sequence< uno::Type > SvxShapeConnector::maTypeSequence;

SvxShapeConnector::SvxShapeConnector( SdrObject* pObj )  throw() :
    SvxShape( pObj, aSvxMapProvider.GetMap(SVXMAP_CONNECTOR) )
{
}

//----------------------------------------------------------------------
SvxShapeConnector::~SvxShapeConnector() throw()
{
}

//----------------------------------------------------------------------
uno::Any SAL_CALL SvxShapeConnector::queryInterface( const uno::Type & rType ) throw(uno::RuntimeException)
{
    return OWeakAggObject::queryInterface( rType );
}

uno::Any SAL_CALL SvxShapeConnector::queryAggregation( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    uno::Any aAny;

    QUERYINT( drawing::XConnectorShape );
    else
        return SvxShape::queryAggregation( rType );

    return aAny;
}

void SAL_CALL SvxShapeConnector::acquire() throw(uno::RuntimeException)
{
    OWeakAggObject::acquire();
}

void SAL_CALL SvxShapeConnector::release() throw(uno::RuntimeException)
{
    OWeakAggObject::release();
}
// XTypeProvider

uno::Sequence< uno::Type > SAL_CALL SvxShapeConnector::getTypes()
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

        *pTypes++ = ::getCppuType((const uno::Reference< drawing::XConnectorShape>*)0);

        for( sal_Int32 nType = 0; nType < nBaseTypes; nType++ )
            *pTypes++ = *pBaseTypes++;
    }
    return maTypeSequence;
}

uno::Sequence< sal_Int8 > SAL_CALL SvxShapeConnector::getImplementationId()
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

// ::com::sun::star::drawing::XShape

//----------------------------------------------------------------------
OUString SAL_CALL SvxShapeConnector::getShapeType()
    throw( uno::RuntimeException )
{
    return SvxShape::getShapeType();
}

//------------------------------------------------------------------1----
awt::Point SAL_CALL SvxShapeConnector::getPosition() throw(uno::RuntimeException)
{
    return SvxShape::getPosition();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeConnector::setPosition( const awt::Point& Position ) throw(uno::RuntimeException)
{
    SvxShape::setPosition(aPosition);
}

//----------------------------------------------------------------------

awt::Size SAL_CALL SvxShapeConnector::getSize() throw(uno::RuntimeException)
{
    return SvxShape::getSize();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeConnector::setSize( const awt::Size& rSize )
    throw(beans::PropertyVetoException, uno::RuntimeException)
{
    SvxShape::setSize( rSize );
}

//----------------------------------------------------------------------

// XConnectorShape

void SAL_CALL SvxShapeConnector::connectStart( const uno::Reference< drawing::XConnectableShape >& xShape, drawing::ConnectionType nPos ) throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    Reference< drawing::XShape > xRef( xShape, UNO_QUERY );
    SvxShape* pShape = SvxShape::getImplementation( xRef );

    if( pShape )
        pObj->ConnectToNode( sal_True, pShape->pObj );

    if( pModel )
        pModel->SetChanged();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeConnector::connectEnd( const uno::Reference< drawing::XConnectableShape >& xShape, drawing::ConnectionType nPos )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    Reference< drawing::XShape > xRef( xShape, UNO_QUERY );
    SvxShape* pShape = SvxShape::getImplementation( xRef );

    if( pObj && pShape )
        pObj->ConnectToNode( sal_False, pShape->pObj );

    if( pModel )
        pModel->SetChanged();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeConnector::disconnectBegin( const uno::Reference< drawing::XConnectableShape >& xShape )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(pObj)
        pObj->DisconnectFromNode( sal_True );

    if( pModel )
        pModel->SetChanged();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeConnector::disconnectEnd( const uno::Reference< drawing::XConnectableShape >& xShape )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(pObj)
        pObj->DisconnectFromNode( sal_False );

    if( pModel )
        pModel->SetChanged();
}

//----------------------------------------------------------------------
// ::com::sun::star::lang::XServiceInfo
//----------------------------------------------------------------------
uno::Sequence< OUString > SAL_CALL SvxShapeConnector::getSupportedServiceNames() throw( uno::RuntimeException )
{
    return SvxShape::getSupportedServiceNames();
}

/***********************************************************************
* class SvxShapeControl                                                *
***********************************************************************/

extern SfxItemPropertyMap* ImplGetSvxControlShapePropertyMap();

uno::Sequence< uno::Type > SvxShapeControl::maTypeSequence;

SvxShapeControl::SvxShapeControl( SdrObject* pObj )  throw() :
    SvxShape( pObj, ImplGetSvxControlShapePropertyMap() )
{
}

//----------------------------------------------------------------------
SvxShapeControl::~SvxShapeControl() throw()
{
}

//----------------------------------------------------------------------
uno::Any SAL_CALL SvxShapeControl::queryInterface( const uno::Type & rType ) throw(uno::RuntimeException)
{
    return OWeakAggObject::queryInterface( rType );
}

uno::Any SAL_CALL SvxShapeControl::queryAggregation( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    uno::Any aAny;

    QUERYINT( drawing::XControlShape );
    else
        return SvxShape::queryAggregation( rType );

    return aAny;
}

void SAL_CALL SvxShapeControl::acquire() throw(uno::RuntimeException)
{
    OWeakAggObject::acquire();
}

void SAL_CALL SvxShapeControl::release() throw(uno::RuntimeException)
{
    OWeakAggObject::release();
}
// XTypeProvider

uno::Sequence< uno::Type > SAL_CALL SvxShapeControl::getTypes()
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

        *pTypes++ = ::getCppuType((const uno::Reference< drawing::XControlShape>*)0);

        for( sal_Int32 nType = 0; nType < nBaseTypes; nType++ )
            *pTypes++ = *pBaseTypes++;
    }
    return maTypeSequence;
}

uno::Sequence< sal_Int8 > SAL_CALL SvxShapeControl::getImplementationId()
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

// ::com::sun::star::drawing::XShape

//----------------------------------------------------------------------
OUString SAL_CALL SvxShapeControl::getShapeType()
    throw( uno::RuntimeException )
{
    return SvxShape::getShapeType();
}

//------------------------------------------------------------------1----
awt::Point SAL_CALL SvxShapeControl::getPosition() throw(uno::RuntimeException)
{
    return SvxShape::getPosition();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeControl::setPosition( const awt::Point& Position ) throw(uno::RuntimeException)
{
    SvxShape::setPosition(Position);
}

//----------------------------------------------------------------------

awt::Size SAL_CALL SvxShapeControl::getSize() throw(uno::RuntimeException)
{
    return SvxShape::getSize();
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeControl::setSize( const awt::Size& rSize )
    throw(beans::PropertyVetoException, uno::RuntimeException)
{
    SvxShape::setSize( rSize );
}

//----------------------------------------------------------------------
// XControlShape

Reference< awt::XControlModel > SAL_CALL SvxShapeControl::getControl()
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    Reference< awt::XControlModel > xModel;

    SdrUnoObj* pUnoObj = PTR_CAST(SdrUnoObj, pObj);
    if( pUnoObj )
        xModel = pUnoObj->GetUnoControlModel();

    return xModel;
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeControl::setControl( const Reference< awt::XControlModel >& xControl )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    SdrUnoObj* pUnoObj = PTR_CAST(SdrUnoObj, pObj);
    if( pUnoObj )
        pUnoObj->SetUnoControlModel( xControl );

    if( pModel )
        pModel->SetChanged();
}

// XServiceInfo
uno::Sequence< OUString > SAL_CALL SvxShapeControl::getSupportedServiceNames() throw( uno::RuntimeException )
{
    return SvxShape::getSupportedServiceNames();
}

/***********************************************************************
* class SvxShapeDimensioning                                           *
***********************************************************************/

//----------------------------------------------------------------------
SvxShapeDimensioning::SvxShapeDimensioning( SdrObject* pObj ) throw()
:   SvxShape( pObj, aSvxMapProvider.GetMap(SVXMAP_DIMENSIONING) )
{
}

//----------------------------------------------------------------------
SvxShapeDimensioning::~SvxShapeDimensioning() throw()
{
}

// ::com::sun::star::lang::XServiceInfo
uno::Sequence< OUString > SAL_CALL SvxShapeDimensioning::getSupportedServiceNames() throw( uno::RuntimeException )
{
    return SvxShape::getSupportedServiceNames();
}

/***********************************************************************
*                                                                      *
***********************************************************************/

//----------------------------------------------------------------------
SvxShapeCircle::SvxShapeCircle( SdrObject* pObj ) throw()
:   SvxShape( pObj, aSvxMapProvider.GetMap(SVXMAP_CIRCLE) )
{
}

//----------------------------------------------------------------------
SvxShapeCircle::~SvxShapeCircle() throw()
{
}

// ::com::sun::star::lang::XServiceInfo
// XServiceInfo
uno::Sequence< OUString > SAL_CALL SvxShapeCircle::getSupportedServiceNames() throw( uno::RuntimeException )
{
    return SvxShape::getSupportedServiceNames();
}

/***********************************************************************
*                                                                      *
***********************************************************************/

#include "svdopath.hxx"

//----------------------------------------------------------------------
SvxShapePolyPolygon::SvxShapePolyPolygon( SdrObject* pObj , drawing::PolygonKind eNew )
 throw( com::sun::star::beans::PropertyVetoException, com::sun::star::lang::IllegalArgumentException) : SvxShape( pObj, aSvxMapProvider.GetMap(SVXMAP_POLYPOLYGON) ),
    ePolygonKind( eNew )
{
}

//----------------------------------------------------------------------
SvxShapePolyPolygon::~SvxShapePolyPolygon() throw()
{
}

void SAL_CALL ImplSvxPolyPolygonToPointSequenceSequence( const drawing::PointSequenceSequence* pOuterSequence, XPolyPolygon& rNewPolyPolygon ) throw()
{
    // Koordinaten in das PolyPolygon packen
    sal_Int32 nOuterSequenceCount = pOuterSequence->getLength();

    // Zeiger auf innere sequences holen
    const drawing::PointSequence* pInnerSequence = pOuterSequence->getConstArray();

    for(sal_Int32 a=0;a<nOuterSequenceCount;a++)
    {
        sal_Int32 nInnerSequenceCount = pInnerSequence->getLength();

        // Neues XPolygon vorbereiten
        XPolygon aNewPolygon((USHORT)nInnerSequenceCount);

        // Zeiger auf Arrays holen
        const awt::Point* pArray = pInnerSequence->getConstArray();

        for(sal_Int32 b=0;b<nInnerSequenceCount;b++)
        {
            aNewPolygon[(USHORT)b] = Point( pArray->X, pArray->Y );
            pArray++;
        }
        pInnerSequence++;

        // Neues Teilpolygon einfuegen
        rNewPolyPolygon.Insert(aNewPolygon);
    }
}

//----------------------------------------------------------------------

void SAL_CALL SvxShapePolyPolygon::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, com::sun::star::beans::PropertyVetoException, com::sun::star::lang::IllegalArgumentException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_POLYPOLYGON)))
    {
        if( !aValue.getValue() || aValue.getValueType() != ::getCppuType((const drawing::PointSequenceSequence*)0) )
            throw lang::IllegalArgumentException();

        XPolyPolygon aNewPolyPolygon;

        ImplSvxPolyPolygonToPointSequenceSequence( (drawing::PointSequenceSequence*)aValue.getValue(), aNewPolyPolygon );

        SetPolygon(aNewPolyPolygon);
    }
    else if(aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("Geometry")))
    {
        if( !aValue.getValue() || aValue.getValueType() != ::getCppuType((const drawing::PointSequenceSequence*)0) )
            throw lang::IllegalArgumentException();

        if( pObj )
        {
            XPolyPolygon aNewPolyPolygon;
            Matrix3D aMatrix3D;

            pObj->TRGetBaseGeometry(aMatrix3D, aNewPolyPolygon);

            ImplSvxPolyPolygonToPointSequenceSequence( (drawing::PointSequenceSequence*)aValue.getValue(), aNewPolyPolygon );

            pObj->TRSetBaseGeometry(aMatrix3D, aNewPolyPolygon);
        }
    }
    else if(aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_POLYGON)))
    {
        drawing::PointSequence* pSequence = (drawing::PointSequence*)aValue.getValue();

        if( !pSequence || aValue.getValueType() != ::getCppuType((const drawing::PointSequence*)0 ) )
            throw lang::IllegalArgumentException();

        // Koordinaten in das PolyPolygon packen
        const sal_Int32 nSequenceCount = pSequence->getLength();

        // Neues PolyPolygon vorbereiten
        XPolyPolygon aNewPolyPolygon;

        // Neues XPolygon vorbereiten
        XPolygon aNewPolygon((USHORT)nSequenceCount);

        // Zeiger auf Arrays holen
        awt::Point* pArray = pSequence->getArray();

        for(sal_Int32 b=0;b<nSequenceCount;b++)
        {
            aNewPolygon[(USHORT)b] = Point( pArray->X,  pArray->Y );
            pArray++;
        }

        // Neues Teilpolygon einfuegen
        aNewPolyPolygon.Insert(aNewPolygon);

        // Polygon setzen
        SetPolygon(aNewPolyPolygon);
    }
    else
    {
        SvxShape::setPropertyValue(aPropertyName, aValue);
    }

    if( pModel )
        pModel->SetChanged();
}

void SAL_CALL ImplSvxPointSequenceSequenceToPolyPolygon( const XPolyPolygon& rPolyPoly, drawing::PointSequenceSequence& rRetval )
{
    if( rRetval.getLength() != rPolyPoly.Count() )
        rRetval.realloc( rPolyPoly.Count() );

    // Zeiger auf aeussere Arrays holen
    drawing::PointSequence* pOuterSequence = rRetval.getArray();

    for(sal_uInt16 a=0;a<rPolyPoly.Count();a++)
    {
        // Einzelpolygon holen
        const XPolygon& rPoly = rPolyPoly[a];

        // Platz in Arrays schaffen
        pOuterSequence->realloc((sal_Int32)rPoly.GetPointCount());

        // Pointer auf arrays holen
        awt::Point* pInnerSequence = pOuterSequence->getArray();

        for(sal_uInt16 b=0;b<rPoly.GetPointCount();b++)
        {
            *pInnerSequence = awt::Point( rPoly[b].X(), rPoly[b].Y() );
            pInnerSequence++;
        }

        pOuterSequence++;
    }
}

//----------------------------------------------------------------------

uno::Any SAL_CALL SvxShapePolyPolygon::getPropertyValue( const OUString& aPropertyName )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_POLYPOLYGON)))
    {
        // PolyPolygon in eine struct PolyPolygon packen
        const XPolyPolygon& rPolyPoly = GetPolygon();
        drawing::PointSequenceSequence aRetval( rPolyPoly.Count() );

        ImplSvxPointSequenceSequenceToPolyPolygon( rPolyPoly, aRetval );

        return uno::Any( &aRetval, ::getCppuType((const drawing::PointSequenceSequence*)0) );
    }
    else if(aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("Geometry")))
    {
        // PolyPolygon in eine struct PolyPolygon packen
        XPolyPolygon aPolyPoly;
        Matrix3D aMatrix3D;
        if( pObj )
            pObj->TRGetBaseGeometry( aMatrix3D, aPolyPoly );

        drawing::PointSequenceSequence aRetval( aPolyPoly.Count() );

        ImplSvxPointSequenceSequenceToPolyPolygon( aPolyPoly, aRetval );

        return uno::Any( &aRetval, ::getCppuType((const drawing::PointSequenceSequence*)0) );
    }
    else if(aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_POLYGON)))
    {
        // PolyPolygon in eine struct PolyPolygon packen
        const XPolyPolygon& rPolyPoly = GetPolygon();

        sal_Int32 nCount = 0;
        if( rPolyPoly.Count() > 0 )
            nCount = rPolyPoly[0].GetPointCount();

        drawing::PointSequence aRetval( nCount );

        if( nCount > 0 )
        {
            // Einzelpolygon holen
            const XPolygon& rPoly = rPolyPoly[0];

            // Pointer auf arrays holen
            awt::Point* pSequence = aRetval.getArray();

            for(sal_Int32 b=0;b<nCount;b++)
                *pSequence++ = awt::Point( rPoly[(USHORT)b].X(), rPoly[(USHORT)b].Y() );
        }

        return uno::Any( &aRetval, ::getCppuType((const drawing::PointSequence*)0) );
    }
    else if(aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_POLYGONKIND)))
    {
        drawing::PolygonKind ePT = GetPolygonKind();
        return Any( &ePT, ::getCppuType((const drawing::PolygonKind*)0) );
    }
    else
    {
        return SvxShape::getPropertyValue(aPropertyName);
    }
}

//----------------------------------------------------------------------
drawing::PolygonKind SvxShapePolyPolygon::GetPolygonKind() const throw()
{
    return ePolygonKind;
}

//----------------------------------------------------------------------
void SvxShapePolyPolygon::SetPolygon(const XPolyPolygon& rNew) throw()
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(pObj)
        ((SdrPathObj*)pObj)->SetPathPoly(rNew);
}

//----------------------------------------------------------------------
const XPolyPolygon& SvxShapePolyPolygon::GetPolygon() const throw()
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(pObj)
        return ((SdrPathObj*)pObj)->GetPathPoly();
    return aEmptyPoly;
}

// ::com::sun::star::lang::XServiceInfo
uno::Sequence< OUString > SAL_CALL SvxShapePolyPolygon::getSupportedServiceNames() throw( uno::RuntimeException )
{
    return SvxShape::getSupportedServiceNames();
}

/***********************************************************************
* class SvxShapePolyPolygonBezier                                      *
***********************************************************************/

#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONBEZIERCOORDS_HPP_
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FLAGSEQUENCE_HPP_
#include <com/sun/star/drawing/FlagSequence.hpp>
#endif
//----------------------------------------------------------------------
SvxShapePolyPolygonBezier::SvxShapePolyPolygonBezier( SdrObject* pObj , drawing::PolygonKind eNew ) throw()
:   SvxShape( pObj, aSvxMapProvider.GetMap(SVXMAP_POLYPOLYGONBEZIER) ),
    ePolygonKind( eNew )
{
}

//----------------------------------------------------------------------
SvxShapePolyPolygonBezier::~SvxShapePolyPolygonBezier() throw()
{
}

void SAL_CALL ImplSvxPolyPolygonBezierCoordsToPolyPolygon( drawing::PolyPolygonBezierCoords* pSourcePolyPolygon, XPolyPolygon& rNewPolyPolygon )
    throw( IllegalArgumentException )
{
    sal_Int32 nOuterSequenceCount = pSourcePolyPolygon->Coordinates.getLength();
    if(pSourcePolyPolygon->Flags.getLength() != nOuterSequenceCount)
        throw IllegalArgumentException();

    // Zeiger auf innere sequences holen
    const drawing::PointSequence* pInnerSequence = pSourcePolyPolygon->Coordinates.getConstArray();
    const drawing::FlagSequence* pInnerSequenceFlags = pSourcePolyPolygon->Flags.getConstArray();

    for(sal_Int32 a=0;a<nOuterSequenceCount;a++)
    {
        sal_Int32 nInnerSequenceCount = pInnerSequence->getLength();

        if(pInnerSequenceFlags->getLength() != nInnerSequenceCount)
            throw IllegalArgumentException();

        // Neues XPolygon vorbereiten
        XPolygon aNewPolygon((USHORT)nInnerSequenceCount);

        // Zeiger auf Arrays holen
        const awt::Point* pArray = pInnerSequence->getConstArray();
        const drawing::PolygonFlags* pArrayFlags = pInnerSequenceFlags->getConstArray();

        for(sal_Int32 b=0;b<nInnerSequenceCount;b++)
        {
            aNewPolygon[(USHORT)b] = Point( pArray->X, pArray->Y );
            pArray++;
            aNewPolygon.SetFlags((USHORT)b, (XPolyFlags)((sal_uInt16)*pArrayFlags++));
        }

        pInnerSequence++;
        pInnerSequenceFlags++;

        // Neues Teilpolygon einfuegen
        rNewPolyPolygon.Insert(aNewPolygon);
    }
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapePolyPolygonBezier::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, com::sun::star::beans::PropertyVetoException, com::sun::star::lang::IllegalArgumentException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_POLYPOLYGONBEZIER)))
    {
        if( !aValue.getValue() || aValue.getValueType() != ::getCppuType((const drawing::PolyPolygonBezierCoords*)0) )
            throw IllegalArgumentException();

        XPolyPolygon aNewPolyPolygon;
        ImplSvxPolyPolygonBezierCoordsToPolyPolygon( (drawing::PolyPolygonBezierCoords*)aValue.getValue(), aNewPolyPolygon );
        SetPolygon(aNewPolyPolygon);
    }
    else if(aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("Geometry")))
    {
        if( !aValue.getValue() || aValue.getValueType() != ::getCppuType((const drawing::PolyPolygonBezierCoords*)0) )
            throw IllegalArgumentException();

        if( pObj )
        {
            Matrix3D aMatrix3D;
            XPolyPolygon aNewPolyPolygon;

            pObj->TRGetBaseGeometry(aMatrix3D, aNewPolyPolygon);
            ImplSvxPolyPolygonBezierCoordsToPolyPolygon( (drawing::PolyPolygonBezierCoords*)aValue.getValue(), aNewPolyPolygon );
            pObj->TRSetBaseGeometry(aMatrix3D, aNewPolyPolygon);
        }
    }
    else
    {
        SvxShape::setPropertyValue(aPropertyName, aValue);
    }

    if( pModel )
        pModel->SetChanged();
}

void SAL_CALL ImplSvxPolyPolygonToPolyPolygonBezierCoords( const XPolyPolygon& rPolyPoly, drawing::PolyPolygonBezierCoords& rRetval )
{
    // Polygone innerhalb vrobereiten
    rRetval.Coordinates.realloc((sal_Int32)rPolyPoly.Count());
    rRetval.Flags.realloc((sal_Int32)rPolyPoly.Count());

    // Zeiger auf aeussere Arrays holen
    drawing::PointSequence* pOuterSequence = rRetval.Coordinates.getArray();
    drawing::FlagSequence*  pOuterFlags = rRetval.Flags.getArray();

    for(sal_uInt16 a=0;a<rPolyPoly.Count();a++)
    {
        // Einzelpolygon holen
        const XPolygon& rPoly = rPolyPoly[a];

        // Platz in Arrays schaffen
        pOuterSequence->realloc((sal_Int32)rPoly.GetPointCount());
        pOuterFlags->realloc((sal_Int32)rPoly.GetPointCount());

        // Pointer auf arrays holen
        awt::Point* pInnerSequence = pOuterSequence->getArray();
        drawing::PolygonFlags* pInnerFlags = pOuterFlags->getArray();

        for(sal_uInt16 b=0;b<rPoly.GetPointCount();b++)
        {
            *pInnerSequence++ = awt::Point( rPoly[b].X(), rPoly[b].Y() );
            *pInnerFlags++ = (drawing::PolygonFlags)((sal_uInt16)rPoly.GetFlags(b));
        }

        pOuterSequence++;
        pOuterFlags++;
    }
}

//----------------------------------------------------------------------
uno::Any SAL_CALL SvxShapePolyPolygonBezier::getPropertyValue( const OUString& aPropertyName )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );
    uno::Any aAny;

    if(aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_POLYPOLYGONBEZIER)))
    {
        // PolyPolygon in eine struct PolyPolygon packen
        const XPolyPolygon& rPolyPoly = GetPolygon();
        drawing::PolyPolygonBezierCoords aRetval;
        ImplSvxPolyPolygonToPolyPolygonBezierCoords(rPolyPoly, aRetval );

        aAny <<= aRetval;
    }
    else if(aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("Geometry")))
    {
        if( pObj )
        {
            // PolyPolygon in eine struct PolyPolygon packen
            XPolyPolygon aPolyPoly;
            Matrix3D aMatrix3D;
            pObj->TRGetBaseGeometry( aMatrix3D, aPolyPoly );

            drawing::PolyPolygonBezierCoords aRetval;
            ImplSvxPolyPolygonToPolyPolygonBezierCoords(aPolyPoly, aRetval );

            aAny <<= aRetval;
        }
    }
    else if(aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_POLYGONKIND)))
    {
        aAny <<= (drawing::PolygonKind)GetPolygonKind();
    }
    else
    {
        return SvxShape::getPropertyValue(aPropertyName);
    }

    return aAny;
}

//----------------------------------------------------------------------
drawing::PolygonKind SvxShapePolyPolygonBezier::GetPolygonKind() const throw()
{
    return ePolygonKind;
}

//----------------------------------------------------------------------
void SvxShapePolyPolygonBezier::SetPolygon(const XPolyPolygon& rNew) throw()
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(pObj)
        ((SdrPathObj*)pObj)->SetPathPoly(rNew);
}

//----------------------------------------------------------------------
const XPolyPolygon& SvxShapePolyPolygonBezier::GetPolygon() const throw()
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(pObj)
        return ((SdrPathObj*)pObj)->GetPathPoly();
    return aEmptyPoly;
}


// ::com::sun::star::lang::XServiceInfo
uno::Sequence< OUString > SAL_CALL SvxShapePolyPolygonBezier::getSupportedServiceNames() throw( uno::RuntimeException )
{
    return SvxShape::getSupportedServiceNames();
}

/***********************************************************************
* class SvxGraphicObject                                               *
***********************************************************************/

#ifndef _COM_SUN_STAR_AWT_XBITMAP_HPP_
#include <com/sun/star/awt/XBitmap.hpp>
#endif
#ifndef _SV_CVTGRF_HXX
#include <vcl/cvtgrf.hxx>
#endif
#ifndef _SVDOGRAF_HXX
#include <svdograf.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX
#include <sfx2/fcontnr.hxx>
#endif

#include "toolkit/unohlp.hxx"

//----------------------------------------------------------------------
SvxGraphicObject::SvxGraphicObject( SdrObject* pObj ) throw()
:   SvxShape( pObj, aSvxMapProvider.GetMap(SVXMAP_GRAPHICOBJECT) )
{
}

//----------------------------------------------------------------------
SvxGraphicObject::~SvxGraphicObject() throw()
{
}

//----------------------------------------------------------------------
void SAL_CALL SvxGraphicObject::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, com::sun::star::beans::PropertyVetoException, com::sun::star::lang::IllegalArgumentException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(pObj && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_GRAPHOBJ_FILLBITMAP)))
    {
        if( aValue.getValueType() == ::getCppuType(( const uno::Sequence< sal_Int8 >*)0) )
        {
            uno::Sequence<sal_Int8>* pSeq = (uno::Sequence<sal_Int8>*)aValue.getValue();
            SvMemoryStream  aMemStm;
            Graphic         aGraphic;

            aMemStm.SetBuffer( (char*)pSeq->getConstArray(), pSeq->getLength(), sal_False, pSeq->getLength() );

            if( GraphicConverter::Import( aMemStm, aGraphic ) == ERRCODE_NONE )
            {
                ((SdrGrafObj*)pObj)->SetGraphic(aGraphic);
            }
        }
        else if( aValue.getValueType() == INTERFACE_TYPE( awt::XBitmap))
        {
            // Bitmap in das Objekt packen
            Reference< awt::XBitmap > xBmp;
            if( aValue >>= xBmp )
            {
                // Bitmap einsetzen
                Graphic aGraphic(VCLUnoHelper::GetBitmap( xBmp ));
                ((SdrGrafObj*)pObj)->SetGraphic(aGraphic);
            }
        }
    }
    else if( pObj && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_GRAPHOBJ_GRAFURL)))
    {
        OUString aURL;
        if(!(aValue >>= aURL))
            throw lang::IllegalArgumentException();

        String aGrafURL( aURL );

        if( ( aGrafURL.GetTokenCount( ':' ) == 2 ) &&
            ( aGrafURL.GetToken( 0, ':' ) == String( RTL_CONSTASCII_STRINGPARAM( UNO_NAME_GRAPHOBJ_URLPREFIX ) ).GetToken( 0, ':' ) ) )
        {
            // graphic manager url
            aURL = aURL.copy( sizeof( UNO_NAME_GRAPHOBJ_URLPREFIX ) - 1 );
            String aTmpStr(aURL);
            ByteString aUniqueID( aTmpStr, RTL_TEXTENCODING_UTF8 );
            GraphicObject aGrafObj( aUniqueID );
            ((SdrGrafObj*)pObj)->ReleaseGraphicLink();
            ((SdrGrafObj*)pObj)->SetGraphicObject( aGrafObj );
        }
        else if( ( aGrafURL.GetTokenCount( ':' ) != 2 ) ||
                 ( aGrafURL.GetToken( 0, ':' ) != String( RTL_CONSTASCII_STRINGPARAM( UNO_NAME_GRAPHOBJ_URLPKGPREFIX ) ).GetToken( 0, ':' ) ) )
        {
            // normal link
            const SfxFilter*    pSfxFilter = NULL;
            SfxMedium           aSfxMedium( aURL, STREAM_READ | STREAM_SHARE_DENYNONE, FALSE );
            String              aFilterName;

            SFX_APP()->GetFilterMatcher().GuessFilter( aSfxMedium, &pSfxFilter, SFX_FILTER_IMPORT, SFX_FILTER_NOTINSTALLED | SFX_FILTER_EXECUTABLE );

            if( !pSfxFilter )
            {
                INetURLObject aURLObj( aURL );

                if( aURLObj.GetProtocol() == INET_PROT_NOT_VALID )
                {
                    String aValidURL;

                    if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aURL, aValidURL ) )
                        aURLObj = INetURLObject( aValidURL );
                }

                if( aURLObj.GetProtocol() != INET_PROT_NOT_VALID )
                {
                    GraphicFilter* pGrfFilter = GetGrfFilter();
                    aFilterName = pGrfFilter->GetImportFormatName( pGrfFilter->GetImportFormatNumberForShortName( aURLObj.getExtension() ) );
                }
            }
            else
                aFilterName = pSfxFilter->GetFilterName();

            ((SdrGrafObj*)pObj)->SetGraphicLink( aURL, aFilterName );
        }

    }
    else if( pObj && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_GRAPHOBJ_GRAFSTREAMURL)))
    {
        OUString aStreamURL;

        if( !( aValue >>= aStreamURL ) )
            throw lang::IllegalArgumentException();

        if( ( aStreamURL.getTokenCount( ':' ) != 2 ) ||
            ( aStreamURL.getToken( 0, ':' ) != OUString::createFromAscii( UNO_NAME_GRAPHOBJ_URLPKGPREFIX ).getToken( 0, ':' ) ) )
        {
            aStreamURL = OUString();
        }

        ((SdrGrafObj*)pObj)->SetGrafStreamURL( aStreamURL );
    }
    else
    {
        SvxShape::setPropertyValue(aPropertyName, aValue);
    }
    if( pModel )
        pModel->SetChanged();
}

//----------------------------------------------------------------------
uno::Any SAL_CALL SvxGraphicObject::getPropertyValue( const OUString& aPropertyName )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(pObj && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_GRAPHOBJ_FILLBITMAP)))
    {
        const Graphic& rGraphic = ((SdrGrafObj*)pObj)->GetGraphic();

        if(rGraphic.GetType() != GRAPHIC_GDIMETAFILE)
        {
            // Objekt in eine Bitmap packen
            Reference< ::com::sun::star::awt::XBitmap >  xBitmap( VCLUnoHelper::CreateBitmap(((SdrGrafObj*)pObj)->GetGraphic().GetBitmapEx()) );
            return uno::Any( &xBitmap, ::getCppuType((const Reference<awt::XBitmap>*)0) );
        }
        else
        {
            SvMemoryStream aDestStrm( 65535, 65535 );

            ConvertGDIMetaFileToWMF( rGraphic.GetGDIMetaFile(), aDestStrm, NULL, NULL, sal_False );
            uno::Sequence<sal_Int8> aSeq((sal_Int8*)aDestStrm.GetData(), aDestStrm.GetSize());
            return uno::Any( &aSeq, ::getCppuType(( uno::Sequence< sal_Int8 >*)0) );
        }
    }
    else if( pObj && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_GRAPHOBJ_GRAFURL)) )
    {
        uno::Any aAny;
        if( ((SdrGrafObj*)pObj)->IsLinkedGraphic() )
        {
            aAny <<= OUString( ((SdrGrafObj*)pObj)->GetFileName() );
        }
        else
        {
            const GraphicObject& rGrafObj = ((SdrGrafObj*)pObj)->GetGraphicObject();
            OUString aURL( RTL_CONSTASCII_USTRINGPARAM(UNO_NAME_GRAPHOBJ_URLPREFIX));
            aURL += OUString::createFromAscii( rGrafObj.GetUniqueID().GetBuffer() );
            aAny <<= aURL;
        }
        return aAny;
    }
    else if( pObj && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_NAME_GRAPHOBJ_GRAFSTREAMURL)) )
    {
        const OUString  aStreamURL( ( (SdrGrafObj*) pObj )->GetGrafStreamURL() );
        uno::Any        aAny;

        if( aStreamURL.getLength() )
            aAny <<= aStreamURL;

        return aAny;
    }
    else
    {
        return SvxShape::getPropertyValue(aPropertyName);
    }
}

// ::com::sun::star::lang::XServiceInfo
uno::Sequence< OUString > SAL_CALL SvxGraphicObject::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    return SvxShape::getSupportedServiceNames();
}


void SvxConvertPolyPolygonBezierToXPolygon( const drawing::PolyPolygonBezierCoords* pSourcePolyPolygon, XPolygon& rNewPolygon )
    throw( lang::IllegalArgumentException )
{
    sal_Int32 nOuterSequenceCount = pSourcePolyPolygon->Coordinates.getLength();
    if( nOuterSequenceCount != 1 || pSourcePolyPolygon->Flags.getLength() != nOuterSequenceCount)
        throw lang::IllegalArgumentException();

    // Zeiger auf innere sequences holen
    const drawing::PointSequence* pInnerSequence = pSourcePolyPolygon->Coordinates.getConstArray();
    const drawing::FlagSequence* pInnerSequenceFlags = pSourcePolyPolygon->Flags.getConstArray();

    sal_Int32 nInnerSequenceCount = pInnerSequence->getLength();

    if(pInnerSequenceFlags->getLength() != nInnerSequenceCount)
        throw lang::IllegalArgumentException();

    // Zeiger auf Arrays holen
    const awt::Point* pArray = pInnerSequence->getConstArray();
    const drawing::PolygonFlags* pArrayFlags = pInnerSequenceFlags->getConstArray();

    for(sal_Int32 b=0;b<nInnerSequenceCount;b++)
    {
        rNewPolygon[(USHORT)b] = Point( pArray->X, pArray->Y );
        pArray++;
        rNewPolygon.SetFlags((USHORT)b, (XPolyFlags)((sal_uInt16)*pArrayFlags++));
    }
}

void SvxConvertXPolygonToPolyPolygonBezier( const XPolygon& rPolygon, drawing::PolyPolygonBezierCoords& rRetval ) throw()
{
    // Polygone innerhalb vrobereiten
    rRetval.Coordinates.realloc(1);
    rRetval.Flags.realloc(1);

    // Zeiger auf aeussere Arrays holen
    drawing::PointSequence* pOuterSequence = rRetval.Coordinates.getArray();
    drawing::FlagSequence*  pOuterFlags = rRetval.Flags.getArray();

    // Platz in Arrays schaffen
    pOuterSequence->realloc((sal_Int32)rPolygon.GetPointCount());
    pOuterFlags->realloc((sal_Int32)rPolygon.GetPointCount());

    // Pointer auf arrays holen
    awt::Point* pInnerSequence = pOuterSequence->getArray();
    drawing::PolygonFlags* pInnerFlags = pOuterFlags->getArray();

    for(sal_uInt16 b=0;b<rPolygon.GetPointCount();b++)
    {
        *pInnerSequence++ = awt::Point( rPolygon[b].X(), rPolygon[b].Y() );
        *pInnerFlags++ = (drawing::PolygonFlags)((sal_uInt16)rPolygon.GetFlags(b));
    }
}

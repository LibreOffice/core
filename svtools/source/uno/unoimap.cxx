/*************************************************************************
 *
 *  $RCSfile: unoimap.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-20 16:23:18 $
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

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP_
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POINT_HPP_
#include <com/sun/star/awt/Point.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequence.hpp>
#endif

#ifndef _COMPHELPER_SERVICEHELPER_HXX_
#include <comphelper/servicehelper.hxx>
#endif

#ifndef _COMPHELPER_PROPERTYSETHELPER_HXX_
#include <comphelper/propertysethelper.hxx>
#endif
#ifndef _COMPHELPER_PROPERTSETINFO_HXX_
#include <comphelper/propertysetinfo.hxx>
#endif

#ifndef _CPPUHELPER_WEAKAGG_HXX_
#include <cppuhelper/weakagg.hxx>
#endif

#include <cppuhelper/implbase3.hxx>

#include <stl/list>

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#include "unoevent.hxx"
#include "unoimap.hxx"
#include "imap.hxx"
#include "imapcirc.hxx"
#include "imaprect.hxx"
#include "imappoly.hxx"

#ifndef SEQTYPE
 #if defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)
  #define SEQTYPE(x) (new ::com::sun::star::uno::Type( x ))
 #else
  #define SEQTYPE(x) &(x)
 #endif
#endif

#define MAP_LEN(x) x, sizeof(x)


using namespace rtl;
using namespace comphelper;
using namespace cppu;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::document;
using namespace com::sun::star::drawing;

const sal_Int32 HANDLE_URL = 1;
const sal_Int32 HANDLE_DESCRIPTION = 2;
const sal_Int32 HANDLE_TARGET = 3;
const sal_Int32 HANDLE_NAME = 4;
const sal_Int32 HANDLE_ISACTIVE = 5;
const sal_Int32 HANDLE_POLYGON = 6;
const sal_Int32 HANDLE_CENTER = 7;
const sal_Int32 HANDLE_RADIUS = 8;
const sal_Int32 HANDLE_BOUNDARY = 9;

class SvUnoImageMapObject : public OWeakAggObject,
                            public XEventsSupplier,
                            public XServiceInfo,
                            public PropertySetHelper,
                            public XTypeProvider,
                            public XUnoTunnel
{
public:
    SvUnoImageMapObject( UINT16 nType, const SvEventDescription* pSupportedMacroItems );
    SvUnoImageMapObject( const IMapObject& rMapObject, const SvEventDescription* pSupportedMacroItems );
    virtual ~SvUnoImageMapObject();

    UNO3_GETIMPLEMENTATION_DECL( SvUnoImageMapObject )

    IMapObject* createIMapObject() const;

    SvMacroTableEventDescriptor* mpEvents;

    // overiden helpers from PropertySetHelper
    virtual void _setPropertyValues( const PropertyMapEntry** ppEntries, const Any* pValues ) throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException );
    virtual void _getPropertyValues( const PropertyMapEntry** ppEntries, Any* pValue ) throw(UnknownPropertyException, WrappedTargetException );

    // XInterface
    virtual Any SAL_CALL queryAggregation( const Type & rType ) throw(RuntimeException);
    virtual Any SAL_CALL queryInterface( const Type & rType ) throw(RuntimeException);
    virtual void SAL_CALL acquire() throw(RuntimeException);
    virtual void SAL_CALL release() throw(RuntimeException);

    // XTypeProvider
    virtual Sequence< Type > SAL_CALL getTypes(  ) throw(RuntimeException);
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(RuntimeException);

    // XEventsSupplier
    virtual Reference< ::com::sun::star::container::XNameReplace > SAL_CALL getEvents(  ) throw(RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( RuntimeException );
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw( RuntimeException );

private:
    static PropertySetInfo* SvUnoImageMapObject::createPropertySetInfo( UINT16 nType );


    UINT16 mnType;

    OUString maURL;
    OUString maDescription;
    OUString maTarget;
    OUString maName;
    sal_Bool mbIsActive;
    awt::Rectangle maBoundary;
    awt::Point maCenter;
    sal_Int32 mnRadius;
    PointSequence maPolygon;
};

UNO3_GETIMPLEMENTATION_IMPL( SvUnoImageMapObject );

PropertySetInfo* SvUnoImageMapObject::createPropertySetInfo( UINT16 nType )
{
    switch( nType )
    {
    case IMAP_OBJ_POLYGON:
        {
            static PropertyMapEntry aPolygonObj_Impl[] =
            {
                { MAP_LEN( "URL" ),         HANDLE_URL,         &::getCppuType((const OUString*)0),     0, 0 },
                { MAP_LEN( "Description" ), HANDLE_DESCRIPTION, &::getCppuType((const OUString*)0),     0, 0 },
                { MAP_LEN( "Target" ),      HANDLE_TARGET,      &::getCppuType((const OUString*)0),     0, 0 },
                { MAP_LEN( "Name" ),        HANDLE_NAME,        &::getCppuType((const OUString*)0),     0, 0 },
                { MAP_LEN( "IsActive" ),    HANDLE_ISACTIVE,    &::getBooleanCppuType(),                0, 0 },
                { MAP_LEN( "Polygon" ),     HANDLE_POLYGON,     SEQTYPE(::getCppuType((const PointSequence*)0)),    0, 0 },
                {0,0,0,0,0}
            };

            return new PropertySetInfo( aPolygonObj_Impl );
        }
    case IMAP_OBJ_CIRCLE:
        {
            static PropertyMapEntry aCircleObj_Impl[] =
            {
                { MAP_LEN( "URL" ),         HANDLE_URL,         &::getCppuType((const OUString*)0),     0, 0 },
                { MAP_LEN( "Description" ), HANDLE_DESCRIPTION, &::getCppuType((const OUString*)0),     0, 0 },
                { MAP_LEN( "Target" ),      HANDLE_TARGET,      &::getCppuType((const OUString*)0),     0, 0 },
                { MAP_LEN( "Name" ),            HANDLE_NAME,        &::getCppuType((const OUString*)0),     0, 0 },
                { MAP_LEN( "IsActive" ),        HANDLE_ISACTIVE,    &::getBooleanCppuType(),                0, 0 },
                { MAP_LEN( "Center" ),      HANDLE_CENTER,      &::getCppuType((const awt::Point*)0),   0, 0 },
                { MAP_LEN( "Radius" ),      HANDLE_RADIUS,      &::getCppuType((const sal_Int32*)0),    0, 0 },
                {0,0,0,0,0}
            };

            return new PropertySetInfo( aCircleObj_Impl );
        }
    case IMAP_OBJ_RECTANGLE:
    default:
        {
            static PropertyMapEntry aRectangleObj_Impl[] =
            {
                { MAP_LEN( "URL" ),         HANDLE_URL,         &::getCppuType((const OUString*)0), 0, 0 },
                { MAP_LEN( "Description" ), HANDLE_DESCRIPTION, &::getCppuType((const OUString*)0), 0, 0 },
                { MAP_LEN( "Target" ),      HANDLE_TARGET,      &::getCppuType((const OUString*)0), 0, 0 },
                { MAP_LEN( "Name" ),        HANDLE_NAME,        &::getCppuType((const OUString*)0), 0, 0 },
                { MAP_LEN( "IsActive" ),    HANDLE_ISACTIVE,    &::getBooleanCppuType(),            0, 0 },
                { MAP_LEN( "Boundary" ),    HANDLE_BOUNDARY,    &::getCppuType((const awt::Rectangle*)0),   0, 0 },
                {0,0,0,0,0}
            };

            return new PropertySetInfo( aRectangleObj_Impl );
        }
    }
}

SvUnoImageMapObject::SvUnoImageMapObject( UINT16 nType, const SvEventDescription* pSupportedMacroItems )
:   PropertySetHelper( createPropertySetInfo( nType ) ),
    mnType( nType )
{
    mpEvents = new SvMacroTableEventDescriptor( pSupportedMacroItems );
    mpEvents->acquire();
}

SvUnoImageMapObject::SvUnoImageMapObject( const IMapObject& rMapObject, const SvEventDescription* pSupportedMacroItems )
:   PropertySetHelper( createPropertySetInfo( rMapObject.GetType() ) ),
    mnType( rMapObject.GetType() )
{
    maURL = rMapObject.GetURL();
    maDescription = rMapObject.GetDescription();
    maTarget = rMapObject.GetTarget();
    maName = rMapObject.GetName();
    mbIsActive = rMapObject.IsActive();

    switch( mnType )
    {
    case IMAP_OBJ_RECTANGLE:
        {
            const Rectangle aRect( ((IMapRectangleObject*)&rMapObject)->GetRectangle() );
            maBoundary.X = aRect.Left();
            maBoundary.Y = aRect.Top();
            maBoundary.Width = aRect.GetWidth();
            maBoundary.Height = aRect.GetHeight();
        }
        break;
    case IMAP_OBJ_CIRCLE:
        {
            mnRadius = (sal_Int32)((IMapCircleObject*)&rMapObject)->GetRadius();
            const Point aPoint( ((IMapCircleObject*)&rMapObject)->GetCenter() );

            maCenter.X = aPoint.X();
            maCenter.Y = aPoint.Y();
        }
        break;
    case IMAP_OBJ_POLYGON:
        {
            const Polygon aPoly( ((IMapPolygonObject*)&rMapObject)->GetPolygon() );

            const USHORT nCount = aPoly.GetSize();
            maPolygon.realloc( nCount );
            awt::Point* pPoints = maPolygon.getArray();

            for( USHORT nPoint = 0; nPoint < nCount; nPoint++ )
            {
                const Point& rPoint = aPoly.GetPoint( nPoint );
                pPoints->X = rPoint.X();
                pPoints->Y = rPoint.Y();

                pPoints++;
            }
        }
    }

    mpEvents = new SvMacroTableEventDescriptor( rMapObject.GetMacroTable(), pSupportedMacroItems );
    mpEvents->acquire();
}

SvUnoImageMapObject::~SvUnoImageMapObject()
{
    mpEvents->release();
}

IMapObject* SvUnoImageMapObject::createIMapObject() const
{
    const String aURL( maURL );
    const String aDescription( maDescription );
    const String aTarget( maTarget );
    const String aName( maName );

    IMapObject* pNewIMapObject;

    switch( mnType )
    {
    case IMAP_OBJ_RECTANGLE:
        {
            const Rectangle aRect( maBoundary.X, maBoundary.Y, maBoundary.X + maBoundary.Width - 1, maBoundary.Y + maBoundary.Height - 1 );
            pNewIMapObject = new IMapRectangleObject( aRect, aURL, aDescription, aTarget, aName, mbIsActive );
        }
        break;

    case IMAP_OBJ_CIRCLE:
        {
            const Point aCenter( maCenter.X, maCenter.Y );
            pNewIMapObject = new IMapCircleObject( aCenter, mnRadius, aURL, aDescription, aTarget, aName, mbIsActive );
        }
        break;

    case IMAP_OBJ_POLYGON:
    default:
        {
            const sal_uInt16 nCount = (sal_uInt16)maPolygon.getLength();

            Polygon aPoly( nCount );
            for( sal_uInt16 nPoint = 0; nPoint < nCount; nPoint++ )
            {
                Point aPoint( maPolygon[nPoint].X, maPolygon[nPoint].Y );
                aPoly.SetPoint( aPoint, nPoint );
            }

            pNewIMapObject = new IMapPolygonObject( aPoly, aURL, aDescription, aTarget, aName, mbIsActive );
        }
        break;
    }

    SvxMacroTableDtor aMacroTable;
    mpEvents->copyMacrosIntoTable(aMacroTable);
    pNewIMapObject->SetMacroTable( aMacroTable );

    return pNewIMapObject;
}

// XInterface

Any SAL_CALL SvUnoImageMapObject::queryInterface( const Type & rType )
    throw( RuntimeException )
{
    return OWeakAggObject::queryInterface( rType );
}

Any SAL_CALL SvUnoImageMapObject::queryAggregation( const Type & rType )
    throw(RuntimeException)
{
    Any aAny;

    if( rType == ::getCppuType((const Reference< XServiceInfo >*)0) )
        aAny <<= Reference< XServiceInfo >(this);
    else if( rType == ::getCppuType((const Reference< XTypeProvider >*)0) )
        aAny <<= Reference< XTypeProvider >(this);
    else if( rType == ::getCppuType((const Reference< XPropertySet >*)0) )
        aAny <<= Reference< XPropertySet >(this);
    else if( rType == ::getCppuType((const Reference< XEventsSupplier >*)0) )
        aAny <<= Reference< XEventsSupplier >(this);
    else if( rType == ::getCppuType((const Reference< XMultiPropertySet >*)0) )
        aAny <<= Reference< XMultiPropertySet >(this);
    else if( rType == ::getCppuType((const Reference< XUnoTunnel >*)0) )
        aAny <<= Reference< XUnoTunnel >(this);
    else
        aAny <<= OWeakAggObject::queryAggregation( rType );

    return aAny;
}

void SAL_CALL SvUnoImageMapObject::acquire() throw(uno::RuntimeException)
{
    OWeakAggObject::acquire();
}

void SAL_CALL SvUnoImageMapObject::release() throw(uno::RuntimeException)
{
    OWeakAggObject::release();
}

uno::Sequence< uno::Type > SAL_CALL SvUnoImageMapObject::getTypes()
    throw (uno::RuntimeException)
{
    uno::Sequence< uno::Type > aTypes( 7 );
    uno::Type* pTypes = aTypes.getArray();

    *pTypes++ = ::getCppuType((const uno::Reference< XAggregation>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< XEventsSupplier>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< XServiceInfo>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< XPropertySet>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< XMultiPropertySet>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< XTypeProvider>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< XUnoTunnel>*)0);

    return aTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL SvUnoImageMapObject::getImplementationId()
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

// XServiceInfo

sal_Bool SAL_CALL SvUnoImageMapObject::supportsService( const  OUString& ServiceName ) throw(RuntimeException)
{
    const Sequence< OUString > aSNL( getSupportedServiceNames() );
    const OUString * pArray = aSNL.getConstArray();

    const sal_Int32 nCount = aSNL.getLength();
    for( sal_Int32 i = 0; i < nCount; i++ )
        if( pArray[i] == ServiceName )
            return sal_True;

    return sal_False;
}

Sequence< OUString > SAL_CALL SvUnoImageMapObject::getSupportedServiceNames()
    throw(RuntimeException)
{
    Sequence< OUString > aSNS( 2 );
    aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.image.ImageMapObject" ));
    switch( mnType )
    {
    case IMAP_OBJ_POLYGON:
        aSNS.getArray()[1] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.image.ImageMapPolygonObject" ));
        break;
    case IMAP_OBJ_RECTANGLE:
        aSNS.getArray()[1] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.image.ImageMapRectangleObject" ));
        break;
    case IMAP_OBJ_CIRCLE:
        aSNS.getArray()[1] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.image.ImageMapCircleObject" ));
        break;
    }
    return aSNS;
}

OUString SAL_CALL SvUnoImageMapObject::getImplementationName() throw(RuntimeException)
{
    switch( mnType )
    {
    case IMAP_OBJ_POLYGON:
        return OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.svt.ImageMapPolygonObject") );
    case IMAP_OBJ_CIRCLE:
        return OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.svt.ImageMapCircleObject") );
    case IMAP_OBJ_RECTANGLE:
    default:
        return OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.svt.ImageMapRectangleObject") );
    }
}

// overiden helpers from PropertySetHelper
void SvUnoImageMapObject::_setPropertyValues( const PropertyMapEntry** ppEntries, const Any* pValues )
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException )
{
    sal_Bool bOk = sal_False;

    while( *ppEntries )
    {
        switch( (*ppEntries)->mnHandle )
        {
        case HANDLE_URL:
            bOk = *pValues >>= maURL;
            break;
        case HANDLE_DESCRIPTION:
            bOk = *pValues >>= maDescription;
            break;
        case HANDLE_TARGET:
            bOk = *pValues >>= maTarget;
            break;
        case HANDLE_NAME:
            bOk = *pValues >>= maName;
            break;
        case HANDLE_ISACTIVE:
            bOk = *pValues >>= mbIsActive;
            break;
        case HANDLE_BOUNDARY:
            bOk = *pValues >>= maBoundary;
            break;
        case HANDLE_CENTER:
            bOk = *pValues >>= maCenter;
            break;
        case HANDLE_RADIUS:
            bOk = *pValues >>= mnRadius;
            break;
        case HANDLE_POLYGON:
            bOk = *pValues >>= maPolygon;
            break;
        }

        if( !bOk )
            throw IllegalArgumentException();

        ppEntries++;
        pValues++;
    }
}

void SvUnoImageMapObject::_getPropertyValues( const PropertyMapEntry** ppEntries, Any* pValues )
    throw(UnknownPropertyException, WrappedTargetException )
{
    while( *ppEntries )
    {
        switch( (*ppEntries)->mnHandle )
        {
        case HANDLE_URL:
            *pValues <<= maURL;
            break;
        case HANDLE_DESCRIPTION:
            *pValues <<= maDescription;
            break;
        case HANDLE_TARGET:
            *pValues <<= maTarget;
            break;
        case HANDLE_NAME:
            *pValues <<= maName;
            break;
        case HANDLE_ISACTIVE:
            *pValues <<= mbIsActive;
            break;
        case HANDLE_BOUNDARY:
            *pValues <<= maBoundary;
            break;
        case HANDLE_CENTER:
            *pValues <<= maCenter;
            break;
        case HANDLE_RADIUS:
            *pValues <<= mnRadius;
            break;
        case HANDLE_POLYGON:
            *pValues <<= maPolygon;
            break;
        }

        ppEntries++;
        pValues++;
    }
}


Reference< XNameReplace > SAL_CALL SvUnoImageMapObject::getEvents()
    throw( RuntimeException )
{
    // try weak reference first
    Reference< XNameReplace > xEvents( mpEvents );
    return xEvents;
}

///////////////////////////////////////////////////////////////////////

class SvUnoImageMap : public WeakImplHelper3< XIndexContainer, XServiceInfo, XUnoTunnel >
{
public:
    SvUnoImageMap( const SvEventDescription* pSupportedMacroItems );
    SvUnoImageMap( const ImageMap& rMap, const SvEventDescription* pSupportedMacroItems );
    virtual ~SvUnoImageMap();

    sal_Bool fillImageMap( ImageMap& rMap ) const;
    SvUnoImageMapObject* getObject( const Any& aElement ) const throw( IllegalArgumentException );

    UNO3_GETIMPLEMENTATION_DECL( SvUnoImageMap )

    // XIndexContainer
    virtual void SAL_CALL insertByIndex( sal_Int32 Index, const Any& Element ) throw( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException );
    virtual void SAL_CALL removeByIndex( sal_Int32 Index ) throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException );

    // XIndexReplace
    virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const Any& Element ) throw( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException );

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw( RuntimeException );
    virtual Any SAL_CALL getByIndex( sal_Int32 Index ) throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException );

    // XElementAccess
    virtual Type SAL_CALL getElementType(  ) throw( RuntimeException );
    virtual sal_Bool SAL_CALL hasElements(  ) throw( RuntimeException );

    // XSerivceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( RuntimeException );
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw( RuntimeException );

private:
    OUString maName;

    std::list< SvUnoImageMapObject* > maObjectList;
};

UNO3_GETIMPLEMENTATION_IMPL( SvUnoImageMap );

SvUnoImageMap::SvUnoImageMap( const SvEventDescription* pSupportedMacroItems )
{
}

SvUnoImageMap::SvUnoImageMap( const ImageMap& rMap, const SvEventDescription* pSupportedMacroItems )
{
    maName = rMap.GetName();

    const UINT16 nCount = rMap.GetIMapObjectCount();
    for( UINT16 nPos = 0; nPos < nCount; nPos++ )
    {
        IMapObject* pMapObject = rMap.GetIMapObject( nPos );
        SvUnoImageMapObject* pUnoObj = new SvUnoImageMapObject( *pMapObject, pSupportedMacroItems );
        pUnoObj->acquire();
        maObjectList.push_back( pUnoObj );
    }
}

SvUnoImageMap::~SvUnoImageMap()
{
    std::list< SvUnoImageMapObject* >::iterator aIter = maObjectList.begin();
    const std::list< SvUnoImageMapObject* >::iterator aEnd = maObjectList.end();
    while( aIter != aEnd )
    {
        (*aIter++)->release();
    }
}

SvUnoImageMapObject* SvUnoImageMap::getObject( const Any& aElement ) const
    throw( IllegalArgumentException )
{
    Reference< XInterface > xObject;
    aElement >>= xObject;

    SvUnoImageMapObject* pObject = SvUnoImageMapObject::getImplementation( xObject );
    if( NULL == pObject )
        throw IllegalArgumentException();

    return pObject;
}

// XIndexContainer
void SAL_CALL SvUnoImageMap::insertByIndex( sal_Int32 Index, const Any& Element )
    throw( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    SvUnoImageMapObject* pObject = getObject( Element );
    const sal_Int32 nCount = maObjectList.size();
    if( NULL == pObject || Index > nCount )
        throw IndexOutOfBoundsException();

    pObject->acquire();

    if( Index == nCount )
        maObjectList.push_back( pObject );
    else
    {
        std::list< SvUnoImageMapObject* >::iterator aIter = maObjectList.begin();
        for( sal_Int32 n = 0; n < Index; n++ )
            aIter++;

        maObjectList.insert( aIter, pObject );
    }
}

void SAL_CALL SvUnoImageMap::removeByIndex( sal_Int32 Index ) throw(IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    const sal_Int32 nCount = maObjectList.size();
    if( Index >= nCount )
        throw IndexOutOfBoundsException();

    if( nCount - 1 == Index )
    {
        maObjectList.back()->release();
        maObjectList.pop_back();
    }
    else
    {
        std::list< SvUnoImageMapObject* >::iterator aIter = maObjectList.begin();
        for( sal_Int32 n = 0; n < Index; n++ )
            aIter++;

        (*aIter)->release();
        maObjectList.erase( aIter );
    }
}

// XIndexReplace
void SAL_CALL SvUnoImageMap::replaceByIndex( sal_Int32 Index, const Any& Element ) throw(IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    SvUnoImageMapObject* pObject = getObject( Element );
    const sal_Int32 nCount = maObjectList.size();
    if( NULL == pObject || Index >= nCount )
        throw IndexOutOfBoundsException();

    std::list< SvUnoImageMapObject* >::iterator aIter = maObjectList.begin();
    for( sal_Int32 n = 0; n < Index; n++ )
        aIter++;

    (*aIter)->release();
    *aIter = pObject;
    pObject->acquire();
}

// XIndexAccess
sal_Int32 SAL_CALL SvUnoImageMap::getCount(  ) throw(RuntimeException)
{
    return maObjectList.size();
}

Any SAL_CALL SvUnoImageMap::getByIndex( sal_Int32 Index ) throw(IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    const sal_Int32 nCount = maObjectList.size();
    if( Index >= nCount )
        throw IndexOutOfBoundsException();

    std::list< SvUnoImageMapObject* >::iterator aIter = maObjectList.begin();
    for( sal_Int32 n = 0; n < Index; n++ )
        aIter++;

    Reference< XPropertySet > xObj( *aIter );
    return makeAny( xObj );
}

// XElementAccess
Type SAL_CALL SvUnoImageMap::getElementType(  ) throw(RuntimeException)
{
    return ::getCppuType((const Reference< XPropertySet >*)0);
}

sal_Bool SAL_CALL SvUnoImageMap::hasElements(  ) throw(RuntimeException)
{
    return maObjectList.size() == 0;
}

// XSerivceInfo
OUString SAL_CALL SvUnoImageMap::getImplementationName(  )
    throw(RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.comp.svt.SvUnoImageMap" ) );
}

sal_Bool SAL_CALL SvUnoImageMap::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    const Sequence< OUString > aSNL( getSupportedServiceNames() );
    const OUString * pArray = aSNL.getConstArray();

    const sal_Int32 nCount = aSNL.getLength();
    for( sal_Int32 i = 0; i < nCount; i++ )
        if( pArray[i] == ServiceName )
            return sal_True;

    return sal_False;
}

Sequence< OUString > SAL_CALL SvUnoImageMap::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    const OUString aSN( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.image.ImageMap" ) );
    return Sequence< OUString >( &aSN, 1 );
}

sal_Bool SvUnoImageMap::fillImageMap( ImageMap& rMap ) const
{
    rMap.ClearImageMap();

    rMap.SetName( maName );

    std::list< SvUnoImageMapObject* >::const_iterator aIter = maObjectList.begin();
    const std::list< SvUnoImageMapObject* >::const_iterator aEnd = maObjectList.end();
    while( aIter != aEnd )
    {
        IMapObject* pNewMapObject = (*aIter)->createIMapObject();
        rMap.InsertIMapObject( *pNewMapObject );
        delete pNewMapObject;

        aIter++;
    }

    return sal_True;
}

// -------------------------------------------------------------------
// factory helper methods
// -------------------------------------------------------------------

Reference< XInterface > SvUnoImageMapRectangleObject_createInstance( const SvEventDescription* pSupportedMacroItems )
{
    return (XWeak*)new SvUnoImageMapObject( IMAP_OBJ_RECTANGLE, pSupportedMacroItems );
}

Reference< XInterface > SvUnoImageMapCircleObject_createInstance( const SvEventDescription* pSupportedMacroItems )
{
    return (XWeak*)new SvUnoImageMapObject( IMAP_OBJ_CIRCLE, pSupportedMacroItems );
}

Reference< XInterface > SvUnoImageMapPolygonObject_createInstance( const SvEventDescription* pSupportedMacroItems )
{
    return (XWeak*)new SvUnoImageMapObject( IMAP_OBJ_POLYGON, pSupportedMacroItems );
}

Reference< XInterface > SvUnoImageMap_createInstance( const SvEventDescription* pSupportedMacroItems )
{
    return (XWeak*)new SvUnoImageMap( pSupportedMacroItems );
}

Reference< XInterface > SvUnoImageMap_createInstance( const ImageMap& rMap, const SvEventDescription* pSupportedMacroItems )
{
    return (XWeak*)new SvUnoImageMap( rMap, pSupportedMacroItems );
}

sal_Bool SvUnoImageMap_fillImageMap( Reference< XInterface > xImageMap, ImageMap& rMap )
{
    SvUnoImageMap* pUnoImageMap = SvUnoImageMap::getImplementation( xImageMap );
    if( NULL == pUnoImageMap )
        return sal_False;

    return pUnoImageMap->fillImageMap( rMap );
}

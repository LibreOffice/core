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

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <comphelper/servicehelper.hxx>
#include <comphelper/propertysethelper.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <cppuhelper/weakagg.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <algorithm>
#include <list>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <svtools/unoevent.hxx>
#include <svtools/unoimap.hxx>
#include <svtools/imap.hxx>
#include <svtools/imapcirc.hxx>
#include <svtools/imaprect.hxx>
#include <svtools/imappoly.hxx>

using namespace comphelper;
using namespace cppu;
using namespace com::sun::star;
using namespace css::uno;
using namespace css::lang;
using namespace css::container;
using namespace css::beans;
using namespace css::document;
using namespace css::drawing;

const sal_Int32 HANDLE_URL = 1;
const sal_Int32 HANDLE_DESCRIPTION = 2;
const sal_Int32 HANDLE_TARGET = 3;
const sal_Int32 HANDLE_NAME = 4;
const sal_Int32 HANDLE_ISACTIVE = 5;
const sal_Int32 HANDLE_POLYGON = 6;
const sal_Int32 HANDLE_CENTER = 7;
const sal_Int32 HANDLE_RADIUS = 8;
const sal_Int32 HANDLE_BOUNDARY = 9;
const sal_Int32 HANDLE_TITLE = 10;

class SvUnoImageMapObject : public OWeakAggObject,
                            public XEventsSupplier,
                            public XServiceInfo,
                            public PropertySetHelper,
                            public XTypeProvider,
                            public XUnoTunnel
{
public:
    SvUnoImageMapObject( sal_uInt16 nType, const SvEventDescription* pSupportedMacroItems );
    SvUnoImageMapObject( const IMapObject& rMapObject, const SvEventDescription* pSupportedMacroItems );

    UNO3_GETIMPLEMENTATION_DECL( SvUnoImageMapObject )

    IMapObject* createIMapObject() const;

    rtl::Reference<SvMacroTableEventDescriptor> mxEvents;

    // overriden helpers from PropertySetHelper
    virtual void _setPropertyValues( const PropertyMapEntry** ppEntries, const Any* pValues ) override;
    virtual void _getPropertyValues( const PropertyMapEntry** ppEntries, Any* pValue ) override;

    // XInterface
    virtual Any SAL_CALL queryAggregation( const Type & rType ) override;
    virtual Any SAL_CALL queryInterface( const Type & rType ) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XTypeProvider
    virtual Sequence< Type > SAL_CALL getTypes(  ) override;
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

    // XEventsSupplier
    virtual Reference< css::container::XNameReplace > SAL_CALL getEvents(  ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

private:
    static PropertySetInfo* createPropertySetInfo( sal_uInt16 nType );


    sal_uInt16 mnType;

    OUString maURL;
    OUString maAltText;
    OUString maDesc;
    OUString maTarget;
    OUString maName;
    bool mbIsActive;
    awt::Rectangle maBoundary;
    awt::Point maCenter;
    sal_Int32 mnRadius;
    PointSequence maPolygon;
};

UNO3_GETIMPLEMENTATION_IMPL( SvUnoImageMapObject );

PropertySetInfo * SvUnoImageMapObject::createPropertySetInfo( sal_uInt16 nType )
{
    switch( nType )
    {
    case IMAP_OBJ_POLYGON:
        {
            static PropertyMapEntry const aPolygonObj_Impl[] =
            {
                { OUString("URL"),         HANDLE_URL,         cppu::UnoType<OUString>::get(),     0, 0 },
                { OUString("Title"),       HANDLE_TITLE,       cppu::UnoType<OUString>::get(),     0, 0 },
                { OUString("Description"), HANDLE_DESCRIPTION, cppu::UnoType<OUString>::get(),     0, 0 },
                { OUString("Target"),      HANDLE_TARGET,      cppu::UnoType<OUString>::get(),     0, 0 },
                { OUString("Name"),        HANDLE_NAME,        cppu::UnoType<OUString>::get(),     0, 0 },
                { OUString("IsActive"),    HANDLE_ISACTIVE,    cppu::UnoType<bool>::get(),                0, 0 },
                { OUString("Polygon"),     HANDLE_POLYGON,     cppu::UnoType<PointSequence>::get(),    0, 0 },
                { OUString(), 0, css::uno::Type(), 0, 0 }
            };

            return new PropertySetInfo( aPolygonObj_Impl );
        }
    case IMAP_OBJ_CIRCLE:
        {
            static PropertyMapEntry const aCircleObj_Impl[] =
            {
                { OUString("URL"),         HANDLE_URL,         cppu::UnoType<OUString>::get(),     0, 0 },
                { OUString("Title"),       HANDLE_TITLE,       cppu::UnoType<OUString>::get(),     0, 0 },
                { OUString("Description"), HANDLE_DESCRIPTION, cppu::UnoType<OUString>::get(),     0, 0 },
                { OUString("Target"),      HANDLE_TARGET,      cppu::UnoType<OUString>::get(),     0, 0 },
                { OUString("Name"),        HANDLE_NAME,        cppu::UnoType<OUString>::get(),     0, 0 },
                { OUString("IsActive"),    HANDLE_ISACTIVE,    cppu::UnoType<bool>::get(),                0, 0 },
                { OUString("Center"),      HANDLE_CENTER,      cppu::UnoType<awt::Point>::get(),   0, 0 },
                { OUString("Radius"),      HANDLE_RADIUS,      cppu::UnoType<sal_Int32>::get(),    0, 0 },
                { OUString(), 0, css::uno::Type(), 0, 0 }
            };

            return new PropertySetInfo( aCircleObj_Impl );
        }
    case IMAP_OBJ_RECTANGLE:
    default:
        {
            static PropertyMapEntry const aRectangleObj_Impl[] =
            {
                { OUString("URL"),         HANDLE_URL,         cppu::UnoType<OUString>::get(), 0, 0 },
                { OUString("Title"),       HANDLE_TITLE,       cppu::UnoType<OUString>::get(),     0, 0 },
                { OUString("Description"), HANDLE_DESCRIPTION, cppu::UnoType<OUString>::get(), 0, 0 },
                { OUString("Target"),      HANDLE_TARGET,      cppu::UnoType<OUString>::get(), 0, 0 },
                { OUString("Name"),        HANDLE_NAME,        cppu::UnoType<OUString>::get(), 0, 0 },
                { OUString("IsActive"),    HANDLE_ISACTIVE,    cppu::UnoType<bool>::get(),            0, 0 },
                { OUString("Boundary"),    HANDLE_BOUNDARY,    cppu::UnoType<awt::Rectangle>::get(),   0, 0 },
                { OUString(), 0, css::uno::Type(), 0, 0 }
            };

            return new PropertySetInfo( aRectangleObj_Impl );
        }
    }
}

SvUnoImageMapObject::SvUnoImageMapObject( sal_uInt16 nType, const SvEventDescription* pSupportedMacroItems )
:   PropertySetHelper( createPropertySetInfo( nType ) ),
    mnType( nType )
,   mbIsActive( true )
,   mnRadius( 0 )
{
    mxEvents = new SvMacroTableEventDescriptor( pSupportedMacroItems );
}

SvUnoImageMapObject::SvUnoImageMapObject( const IMapObject& rMapObject, const SvEventDescription* pSupportedMacroItems )
:   PropertySetHelper( createPropertySetInfo( rMapObject.GetType() ) ),
    mnType( rMapObject.GetType() )
,   mbIsActive( true )
,   mnRadius( 0 )
{
    maURL = rMapObject.GetURL();
    maAltText = rMapObject.GetAltText();
    maDesc = rMapObject.GetDesc();
    maTarget = rMapObject.GetTarget();
    maName = rMapObject.GetName();
    mbIsActive = rMapObject.IsActive();

    switch( mnType )
    {
    case IMAP_OBJ_RECTANGLE:
        {
            const Rectangle aRect( static_cast<const IMapRectangleObject*>(&rMapObject)->GetRectangle(false) );
            maBoundary.X = aRect.Left();
            maBoundary.Y = aRect.Top();
            maBoundary.Width = aRect.GetWidth();
            maBoundary.Height = aRect.GetHeight();
        }
        break;
    case IMAP_OBJ_CIRCLE:
        {
            mnRadius = (sal_Int32)static_cast<const IMapCircleObject*>(&rMapObject)->GetRadius(false);
            const Point aPoint( static_cast<const IMapCircleObject*>(&rMapObject)->GetCenter(false) );

            maCenter.X = aPoint.X();
            maCenter.Y = aPoint.Y();
        }
        break;
    case IMAP_OBJ_POLYGON:
    default:
        {
            const tools::Polygon aPoly( static_cast<const IMapPolygonObject*>(&rMapObject)->GetPolygon(false) );

            const sal_uInt16 nCount = aPoly.GetSize();
            maPolygon.realloc( nCount );
            awt::Point* pPoints = maPolygon.getArray();

            for( sal_uInt16 nPoint = 0; nPoint < nCount; nPoint++ )
            {
                const Point& rPoint = aPoly.GetPoint( nPoint );
                pPoints->X = rPoint.X();
                pPoints->Y = rPoint.Y();

                pPoints++;
            }
        }
    }

    mxEvents = new SvMacroTableEventDescriptor( rMapObject.GetMacroTable(), pSupportedMacroItems );
}

IMapObject* SvUnoImageMapObject::createIMapObject() const
{
    const OUString aURL( maURL );
    const OUString aAltText( maAltText );
    const OUString aDesc( maDesc );
    const OUString aTarget( maTarget );
    const OUString aName( maName );

    IMapObject* pNewIMapObject;

    switch( mnType )
    {
    case IMAP_OBJ_RECTANGLE:
        {
            const Rectangle aRect( maBoundary.X, maBoundary.Y, maBoundary.X + maBoundary.Width - 1, maBoundary.Y + maBoundary.Height - 1 );
            pNewIMapObject = new IMapRectangleObject( aRect, aURL, aAltText, aDesc, aTarget, aName, mbIsActive, false );
        }
        break;

    case IMAP_OBJ_CIRCLE:
        {
            const Point aCenter( maCenter.X, maCenter.Y );
            pNewIMapObject = new IMapCircleObject( aCenter, mnRadius, aURL, aAltText, aDesc, aTarget, aName, mbIsActive, false );
        }
        break;

    case IMAP_OBJ_POLYGON:
    default:
        {
            const sal_uInt16 nCount = (sal_uInt16)maPolygon.getLength();

            tools::Polygon aPoly( nCount );
            for( sal_uInt16 nPoint = 0; nPoint < nCount; nPoint++ )
            {
                Point aPoint( maPolygon[nPoint].X, maPolygon[nPoint].Y );
                aPoly.SetPoint( aPoint, nPoint );
            }

            aPoly.Optimize( PolyOptimizeFlags::CLOSE );
            pNewIMapObject = new IMapPolygonObject( aPoly, aURL, aAltText, aDesc, aTarget, aName, mbIsActive, false );
        }
        break;
    }

    SvxMacroTableDtor aMacroTable;
    mxEvents->copyMacrosIntoTable(aMacroTable);
    pNewIMapObject->SetMacroTable( aMacroTable );

    return pNewIMapObject;
}

// XInterface

Any SAL_CALL SvUnoImageMapObject::queryInterface( const Type & rType )
{
    return OWeakAggObject::queryInterface( rType );
}

Any SAL_CALL SvUnoImageMapObject::queryAggregation( const Type & rType )
{
    Any aAny;

    if( rType == cppu::UnoType<XServiceInfo>::get())
        aAny <<= Reference< XServiceInfo >(this);
    else if( rType == cppu::UnoType<XTypeProvider>::get())
        aAny <<= Reference< XTypeProvider >(this);
    else if( rType == cppu::UnoType<XPropertySet>::get())
        aAny <<= Reference< XPropertySet >(this);
    else if( rType == cppu::UnoType<XEventsSupplier>::get())
        aAny <<= Reference< XEventsSupplier >(this);
    else if( rType == cppu::UnoType<XMultiPropertySet>::get())
        aAny <<= Reference< XMultiPropertySet >(this);
    else if( rType == cppu::UnoType<XUnoTunnel>::get())
        aAny <<= Reference< XUnoTunnel >(this);
    else
        aAny = OWeakAggObject::queryAggregation( rType );

    return aAny;
}

void SAL_CALL SvUnoImageMapObject::acquire() throw()
{
    OWeakAggObject::acquire();
}

void SAL_CALL SvUnoImageMapObject::release() throw()
{
    OWeakAggObject::release();
}

uno::Sequence< uno::Type > SAL_CALL SvUnoImageMapObject::getTypes()
{
    uno::Sequence< uno::Type > aTypes( 7 );
    uno::Type* pTypes = aTypes.getArray();

    *pTypes++ = cppu::UnoType<XAggregation>::get();
    *pTypes++ = cppu::UnoType<XEventsSupplier>::get();
    *pTypes++ = cppu::UnoType<XServiceInfo>::get();
    *pTypes++ = cppu::UnoType<XPropertySet>::get();
    *pTypes++ = cppu::UnoType<XMultiPropertySet>::get();
    *pTypes++ = cppu::UnoType<XTypeProvider>::get();
    *pTypes++ = cppu::UnoType<XUnoTunnel>::get();

    return aTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL SvUnoImageMapObject::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XServiceInfo
sal_Bool SAL_CALL SvUnoImageMapObject::supportsService( const  OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL SvUnoImageMapObject::getSupportedServiceNames()
{
    Sequence< OUString > aSNS( 2 );
    aSNS.getArray()[0] = "com.sun.star.image.ImageMapObject";
    switch( mnType )
    {
    case IMAP_OBJ_POLYGON:
    default:
        aSNS.getArray()[1] = "com.sun.star.image.ImageMapPolygonObject";
        break;
    case IMAP_OBJ_RECTANGLE:
        aSNS.getArray()[1] = "com.sun.star.image.ImageMapRectangleObject";
        break;
    case IMAP_OBJ_CIRCLE:
        aSNS.getArray()[1] = "com.sun.star.image.ImageMapCircleObject";
        break;
    }
    return aSNS;
}

OUString SAL_CALL SvUnoImageMapObject::getImplementationName()
{
    switch( mnType )
    {
    case IMAP_OBJ_POLYGON:
    default:
        return OUString("org.openoffice.comp.svt.ImageMapPolygonObject");
    case IMAP_OBJ_CIRCLE:
        return OUString("org.openoffice.comp.svt.ImageMapCircleObject");
    case IMAP_OBJ_RECTANGLE:
        return OUString("org.openoffice.comp.svt.ImageMapRectangleObject");
    }
}

// overriden helpers from PropertySetHelper
void SvUnoImageMapObject::_setPropertyValues( const PropertyMapEntry** ppEntries, const Any* pValues )
{
    bool bOk = false;

    while( *ppEntries )
    {
        switch( (*ppEntries)->mnHandle )
        {
        case HANDLE_URL:
            bOk = *pValues >>= maURL;
            break;
        case HANDLE_TITLE:
            bOk = *pValues >>= maAltText;
            break;
        case HANDLE_DESCRIPTION:
            bOk = *pValues >>= maDesc;
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
        default:
            OSL_FAIL( "SvUnoImageMapObject::_setPropertyValues: unexpected property handle" );
            break;
        }

        if( !bOk )
            throw IllegalArgumentException();

        ppEntries++;
        pValues++;
    }
}

void SvUnoImageMapObject::_getPropertyValues( const PropertyMapEntry** ppEntries, Any* pValues )
{
    while( *ppEntries )
    {
        switch( (*ppEntries)->mnHandle )
        {
        case HANDLE_URL:
            *pValues <<= maURL;
            break;
        case HANDLE_TITLE:
            *pValues <<= maAltText;
            break;
        case HANDLE_DESCRIPTION:
            *pValues <<= maDesc;
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
        default:
            OSL_FAIL( "SvUnoImageMapObject::_getPropertyValues: unexpected property handle" );
            break;
        }

        ppEntries++;
        pValues++;
    }
}


Reference< XNameReplace > SAL_CALL SvUnoImageMapObject::getEvents()
{
    return mxEvents.get();
}


class SvUnoImageMap : public WeakImplHelper< XIndexContainer, XServiceInfo, XUnoTunnel >
{
public:
    explicit SvUnoImageMap( const SvEventDescription* pSupportedMacroItems );
    SvUnoImageMap( const ImageMap& rMap, const SvEventDescription* pSupportedMacroItems );

    bool fillImageMap( ImageMap& rMap ) const;
    /// @throws IllegalArgumentException
    static SvUnoImageMapObject* getObject( const Any& aElement );

    UNO3_GETIMPLEMENTATION_DECL( SvUnoImageMap )

    // XIndexContainer
    virtual void SAL_CALL insertByIndex( sal_Int32 Index, const Any& Element ) override;
    virtual void SAL_CALL removeByIndex( sal_Int32 Index ) override;

    // XIndexReplace
    virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const Any& Element ) override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) override;
    virtual Any SAL_CALL getByIndex( sal_Int32 Index ) override;

    // XElementAccess
    virtual Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    // XSerivceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

private:
    OUString maName;

    std::list< rtl::Reference<SvUnoImageMapObject> > maObjectList;
};

UNO3_GETIMPLEMENTATION_IMPL( SvUnoImageMap );

SvUnoImageMap::SvUnoImageMap( const SvEventDescription* )
{
}

SvUnoImageMap::SvUnoImageMap( const ImageMap& rMap, const SvEventDescription* pSupportedMacroItems )
{
    maName = rMap.GetName();

    const std::size_t nCount = rMap.GetIMapObjectCount();
    for( std::size_t nPos = 0; nPos < nCount; nPos++ )
    {
        IMapObject* pMapObject = rMap.GetIMapObject( nPos );
        rtl::Reference<SvUnoImageMapObject> xUnoObj = new SvUnoImageMapObject( *pMapObject, pSupportedMacroItems );
        maObjectList.push_back( xUnoObj );
    }
}

SvUnoImageMapObject* SvUnoImageMap::getObject( const Any& aElement )
{
    Reference< XInterface > xObject;
    aElement >>= xObject;

    SvUnoImageMapObject* pObject = SvUnoImageMapObject::getImplementation( xObject );
    if( nullptr == pObject )
        throw IllegalArgumentException();

    return pObject;
}

// XIndexContainer
void SAL_CALL SvUnoImageMap::insertByIndex( sal_Int32 nIndex, const Any& Element )
{
    SvUnoImageMapObject* pObject = getObject( Element );
    const sal_Int32 nCount = maObjectList.size();
    if( nullptr == pObject || nIndex > nCount )
        throw IndexOutOfBoundsException();

    if( nIndex == nCount )
        maObjectList.push_back( pObject );
    else
    {
        auto aIter = maObjectList.begin();
        std::advance(aIter, nIndex);
        maObjectList.insert( aIter, pObject );
    }
}

void SAL_CALL SvUnoImageMap::removeByIndex( sal_Int32 nIndex )
{
    const sal_Int32 nCount = maObjectList.size();
    if( nIndex >= nCount )
        throw IndexOutOfBoundsException();

    if( nCount - 1 == nIndex )
    {
        maObjectList.pop_back();
    }
    else
    {
        auto aIter = maObjectList.begin();
        std::advance(aIter, nIndex);
        maObjectList.erase( aIter );
    }
}

// XIndexReplace
void SAL_CALL SvUnoImageMap::replaceByIndex( sal_Int32 nIndex, const Any& Element )
{
    SvUnoImageMapObject* pObject = getObject( Element );
    const sal_Int32 nCount = maObjectList.size();
    if( nullptr == pObject || nIndex >= nCount )
        throw IndexOutOfBoundsException();

    auto aIter = maObjectList.begin();
    std::advance(aIter, nIndex);
    *aIter = pObject;
}

// XIndexAccess
sal_Int32 SAL_CALL SvUnoImageMap::getCount(  )
{
    return maObjectList.size();
}

Any SAL_CALL SvUnoImageMap::getByIndex( sal_Int32 nIndex )
{
    const sal_Int32 nCount = maObjectList.size();
    if( nIndex >= nCount )
        throw IndexOutOfBoundsException();

    auto aIter = maObjectList.begin();
    std::advance(aIter, nIndex);

    Reference< XPropertySet > xObj( aIter->get() );
    return makeAny( xObj );
}

// XElementAccess
Type SAL_CALL SvUnoImageMap::getElementType(  )
{
    return cppu::UnoType<XPropertySet>::get();
}

sal_Bool SAL_CALL SvUnoImageMap::hasElements(  )
{
    return (!maObjectList.empty());
}

// XSerivceInfo
OUString SAL_CALL SvUnoImageMap::getImplementationName(  )
{
    return OUString( "org.openoffice.comp.svt.SvUnoImageMap" );
}

sal_Bool SAL_CALL SvUnoImageMap::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL SvUnoImageMap::getSupportedServiceNames(  )
{
    const OUString aSN( "com.sun.star.image.ImageMap" );
    return Sequence< OUString >( &aSN, 1 );
}

bool SvUnoImageMap::fillImageMap( ImageMap& rMap ) const
{
    rMap.ClearImageMap();

    rMap.SetName( maName );

    auto aIter = maObjectList.begin();
    auto const aEnd = maObjectList.end();
    while( aIter != aEnd )
    {
        IMapObject* pNewMapObject = (*aIter)->createIMapObject();
        rMap.InsertIMapObject( *pNewMapObject );
        delete pNewMapObject;

        ++aIter;
    }

    return true;
}


// factory helper methods


Reference< XInterface > SvUnoImageMapRectangleObject_createInstance( const SvEventDescription* pSupportedMacroItems )
{
    return static_cast<XWeak*>(new SvUnoImageMapObject( IMAP_OBJ_RECTANGLE, pSupportedMacroItems ));
}

Reference< XInterface > SvUnoImageMapCircleObject_createInstance( const SvEventDescription* pSupportedMacroItems )
{
    return static_cast<XWeak*>(new SvUnoImageMapObject( IMAP_OBJ_CIRCLE, pSupportedMacroItems ));
}

Reference< XInterface > SvUnoImageMapPolygonObject_createInstance( const SvEventDescription* pSupportedMacroItems )
{
    return static_cast<XWeak*>(new SvUnoImageMapObject( IMAP_OBJ_POLYGON, pSupportedMacroItems ));
}

Reference< XInterface > SvUnoImageMap_createInstance( const SvEventDescription* pSupportedMacroItems )
{
    return static_cast<XWeak*>(new SvUnoImageMap( pSupportedMacroItems ));
}

Reference< XInterface > SvUnoImageMap_createInstance( const ImageMap& rMap, const SvEventDescription* pSupportedMacroItems )
{
    return static_cast<XWeak*>(new SvUnoImageMap( rMap, pSupportedMacroItems ));
}

bool SvUnoImageMap_fillImageMap( const Reference< XInterface >& xImageMap, ImageMap& rMap )
{
    SvUnoImageMap* pUnoImageMap = SvUnoImageMap::getImplementation( xImageMap );
    if( nullptr == pUnoImageMap )
        return false;

    return pUnoImageMap->fillImageMap( rMap );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

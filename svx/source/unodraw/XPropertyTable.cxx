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


#include "svx/XPropertyTable.hxx"
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <svx/xdef.hxx>

#include "svx/unoapi.hxx"
#include <editeng/unoprnms.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/tools/unotools.hxx>

using namespace com::sun::star;
using namespace ::cppu;

class SvxUnoXPropertyTable : public WeakImplHelper< container::XNameContainer, lang::XServiceInfo >
{
private:
    XPropertyList*  mpList;
    sal_Int16 mnWhich;

    long getCount() const { return mpList ? mpList->Count() : 0; }
    XPropertyEntry* get( long index ) const;
public:
    SvxUnoXPropertyTable( sal_Int16 nWhich, XPropertyList* pList ) throw();

    virtual ~SvxUnoXPropertyTable() throw();

    virtual uno::Any getAny( const XPropertyEntry* pEntry ) const throw(uno::RuntimeException) = 0;
    virtual XPropertyEntry* getEntry( const OUString& rName, const uno::Any& rAny ) const throw(uno::RuntimeException, lang::IllegalArgumentException) = 0;

    // XServiceInfo
    virtual sal_Bool SAL_CALL supportsService( const  OUString& ServiceName ) throw( uno::RuntimeException, std::exception) override;

    // XNameContainer
    virtual void SAL_CALL insertByName( const  OUString& aName, const  uno::Any& aElement ) throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeByName( const  OUString& Name ) throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override;

    // XNameReplace
    virtual void SAL_CALL replaceByName( const  OUString& aName, const  uno::Any& aElement ) throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override;

    // XNameAccess
    virtual uno::Any SAL_CALL getByName( const  OUString& aName ) throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override;
    virtual uno::Sequence<  OUString > SAL_CALL getElementNames(  ) throw( uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const  OUString& aName ) throw( uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual sal_Bool SAL_CALL hasElements(  ) throw( uno::RuntimeException, std::exception) override;
};

SvxUnoXPropertyTable::SvxUnoXPropertyTable( sal_Int16 nWhich, XPropertyList* pList ) throw()
: mpList( pList ), mnWhich( nWhich )
{
}

SvxUnoXPropertyTable::~SvxUnoXPropertyTable() throw()
{
}

XPropertyEntry* SvxUnoXPropertyTable::get( long index ) const
{
    if( mpList )
        return mpList->Get(index);
    else
        return nullptr;
}

// XServiceInfo
sal_Bool SAL_CALL SvxUnoXPropertyTable::supportsService( const  OUString& ServiceName )
    throw( uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

// XNameContainer
void SAL_CALL SvxUnoXPropertyTable::insertByName( const  OUString& aName, const  uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if( nullptr == mpList )
        throw lang::IllegalArgumentException();

    if( hasByName( aName ) )
        throw container::ElementExistException();

    OUString aInternalName = SvxUnogetInternalNameForItem(mnWhich, aName);

    XPropertyEntry* pNewEntry = getEntry( aInternalName, aElement );
    if( nullptr == pNewEntry )
        throw lang::IllegalArgumentException();

    if( mpList )
        mpList->Insert( pNewEntry );
}

void SAL_CALL SvxUnoXPropertyTable::removeByName( const  OUString& Name )
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    OUString aInternalName = SvxUnogetInternalNameForItem(mnWhich, Name);

    const long nCount = getCount();
    long i;
    for( i = 0; i < nCount; i++ )
    {
        XPropertyEntry* pEntry = get( i );
        if (pEntry && aInternalName.equals(pEntry->GetName()))
        {
            if( mpList )
                delete mpList->Remove( i );
            return;
        }
    }

    throw container::NoSuchElementException();
}

// XNameReplace
void SAL_CALL SvxUnoXPropertyTable::replaceByName( const  OUString& aName, const  uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    OUString aInternalName = SvxUnogetInternalNameForItem(mnWhich, aName);

    const long nCount = getCount();
    long i;
    for( i = 0; i < nCount; i++ )
    {
        XPropertyEntry* pEntry = get( i );
        if (pEntry && aInternalName.equals(pEntry->GetName()))
        {
            XPropertyEntry* pNewEntry = getEntry( aInternalName, aElement );
            if( nullptr == pNewEntry )
                throw lang::IllegalArgumentException();

            if( mpList )
                delete mpList->Replace( pNewEntry, i );
            return;
        }
    }

    throw container::NoSuchElementException();
}

// XNameAccess
uno::Any SAL_CALL SvxUnoXPropertyTable::getByName( const  OUString& aName )
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    OUString aInternalName = SvxUnogetInternalNameForItem(mnWhich, aName);

    const long nCount = getCount();
    long i;
    for( i = 0; i < nCount; i++ )
    {
        XPropertyEntry* pEntry = get( i );

        if (pEntry && aInternalName.equals(pEntry->GetName()))
            return getAny( pEntry );
    }

    throw container::NoSuchElementException();
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXPropertyTable::getElementNames()
    throw( uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    const long nCount = getCount();
    uno::Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();
    long i;
    for( i = 0; i < nCount; i++ )
    {
        XPropertyEntry* pEntry = get( i );

        if (pEntry)
            *pNames++ = SvxUnogetApiNameForItem(mnWhich, pEntry->GetName());
    }

    return aNames;
}

sal_Bool SAL_CALL SvxUnoXPropertyTable::hasByName( const  OUString& aName )
    throw( uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    OUString aInternalName = SvxUnogetInternalNameForItem(mnWhich, aName);

    const long nCount = mpList?mpList->Count():0;
    long i;
    for( i = 0; i < nCount; i++ )
    {
        XPropertyEntry* pEntry = get( i );
        if (pEntry && aInternalName.equals(pEntry->GetName()))
            return sal_True;
    }

    return sal_False;
}

// XElementAccess
sal_Bool SAL_CALL SvxUnoXPropertyTable::hasElements(  )
    throw( uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    return getCount() != 0;
}


class SvxUnoXColorTable : public SvxUnoXPropertyTable
{
public:
    explicit SvxUnoXColorTable( XPropertyList* pList ) throw() : SvxUnoXPropertyTable( XATTR_LINECOLOR, pList ) {};

    // SvxUnoXPropertyTable
    virtual uno::Any getAny( const XPropertyEntry* pEntry ) const throw() override;
    virtual XPropertyEntry* getEntry( const OUString& rName, const uno::Any& rAny ) const throw() override;

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw( uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException, std::exception ) override;
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException, std::exception) override;
};

uno::Reference< uno::XInterface > SAL_CALL SvxUnoXColorTable_createInstance( XPropertyList* pList ) throw()
{
    return static_cast<OWeakObject*>(new SvxUnoXColorTable( pList ));
}

// SvxUnoXPropertyTable
uno::Any SvxUnoXColorTable::getAny( const XPropertyEntry* pEntry ) const throw()
{
    return uno::Any( (sal_Int32)static_cast<const XColorEntry*>(pEntry)->GetColor().GetColor() );
}

XPropertyEntry* SvxUnoXColorTable::getEntry( const OUString& rName, const uno::Any& rAny ) const throw()
{
    sal_Int32 nColor = 0;
    if( !(rAny >>= nColor) )
        return nullptr;

    const Color aColor( (ColorData)nColor );
    return new XColorEntry( aColor, rName );
}

// XElementAccess
uno::Type SAL_CALL SvxUnoXColorTable::getElementType()
    throw( uno::RuntimeException, std::exception )
{
    return ::cppu::UnoType<sal_Int32>::get();
}

// XServiceInfo
OUString SAL_CALL SvxUnoXColorTable::getImplementationName(  ) throw( uno::RuntimeException, std::exception )
{
    return OUString( "SvxUnoXColorTable" );
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXColorTable::getSupportedServiceNames(  ) throw( uno::RuntimeException, std::exception)
{
    const OUString aServiceName( "com.sun.star.drawing.ColorTable" );
    uno::Sequence< OUString > aServices( &aServiceName, 1 );
    return aServices;
}


class SvxUnoXLineEndTable : public SvxUnoXPropertyTable
{
public:
    explicit SvxUnoXLineEndTable( XPropertyList* pTable ) throw() : SvxUnoXPropertyTable( XATTR_LINEEND, pTable ) {};

    // SvxUnoXPropertyTable
    virtual uno::Any getAny( const XPropertyEntry* pEntry ) const throw() override;
    virtual XPropertyEntry* getEntry( const OUString& rName, const uno::Any& rAny ) const throw(lang::IllegalArgumentException) override;

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw( uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException, std::exception ) override;
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException, std::exception) override;
};

uno::Reference< uno::XInterface > SAL_CALL SvxUnoXLineEndTable_createInstance( XPropertyList* pTable ) throw()
{
    return static_cast<OWeakObject*>(new SvxUnoXLineEndTable( pTable ));
}

// SvxUnoXPropertyTable
uno::Any SvxUnoXLineEndTable::getAny( const XPropertyEntry* pEntry ) const throw()
{
    drawing::PolyPolygonBezierCoords aBezier;
    basegfx::unotools::b2DPolyPolygonToPolyPolygonBezier( static_cast<const XLineEndEntry*>(pEntry)->GetLineEnd(),
                                                          aBezier );
    return uno::Any(aBezier);
}

XPropertyEntry* SvxUnoXLineEndTable::getEntry( const OUString& rName, const uno::Any& rAny ) const throw(lang::IllegalArgumentException)
{
    if( !rAny.getValue() || rAny.getValueType() != cppu::UnoType<drawing::PolyPolygonBezierCoords>::get())
        return nullptr;

    basegfx::B2DPolyPolygon aPolyPolygon;
    drawing::PolyPolygonBezierCoords const * pCoords = static_cast<drawing::PolyPolygonBezierCoords const *>(rAny.getValue());
    if( pCoords->Coordinates.getLength() > 0 )
        aPolyPolygon = basegfx::unotools::polyPolygonBezierToB2DPolyPolygon( *pCoords );

    // #86265# make sure polygon is closed
    aPolyPolygon.setClosed(true);

    return new XLineEndEntry( aPolyPolygon, rName );
}

// XElementAccess
uno::Type SAL_CALL SvxUnoXLineEndTable::getElementType()
    throw( uno::RuntimeException, std::exception )
{
    return cppu::UnoType<drawing::PolyPolygonBezierCoords>::get();
}

// XServiceInfo
OUString SAL_CALL SvxUnoXLineEndTable::getImplementationName(  ) throw( uno::RuntimeException, std::exception )
{
    return OUString( "SvxUnoXLineEndTable" );
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXLineEndTable::getSupportedServiceNames(  ) throw( uno::RuntimeException, std::exception)
{
    const OUString aServiceName( "com.sun.star.drawing.LineEndTable" );
    uno::Sequence< OUString > aServices( &aServiceName, 1 );
    return aServices;
}


class SvxUnoXDashTable : public SvxUnoXPropertyTable
{
public:
    explicit SvxUnoXDashTable( XPropertyList* pTable ) throw() : SvxUnoXPropertyTable( XATTR_LINEDASH, pTable ) {};

    // SvxUnoXPropertyTable
    virtual uno::Any getAny( const XPropertyEntry* pEntry ) const throw() override;
    virtual XPropertyEntry* getEntry( const OUString& rName, const uno::Any& rAny ) const throw() override;

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw( uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException, std::exception ) override;
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException, std::exception) override;
};

uno::Reference< uno::XInterface > SAL_CALL SvxUnoXDashTable_createInstance( XPropertyList* pTable ) throw()
{
    return static_cast<OWeakObject*>(new SvxUnoXDashTable( pTable ));
}

// SvxUnoXPropertyTable
uno::Any SvxUnoXDashTable::getAny( const XPropertyEntry* pEntry ) const throw()
{
    const XDash& rXD = static_cast<const XDashEntry*>(pEntry)->GetDash();

    drawing::LineDash aLineDash;

    aLineDash.Style = (css::drawing::DashStyle)((sal_uInt16)rXD.GetDashStyle());
    aLineDash.Dots = rXD.GetDots();
    aLineDash.DotLen = rXD.GetDotLen();
    aLineDash.Dashes = rXD.GetDashes();
    aLineDash.DashLen = rXD.GetDashLen();
    aLineDash.Distance = rXD.GetDistance();

    return uno::Any(aLineDash);
}

XPropertyEntry* SvxUnoXDashTable::getEntry( const OUString& rName, const uno::Any& rAny ) const throw()
{
    drawing::LineDash aLineDash;
    if(!(rAny >>= aLineDash))
        return nullptr;

    XDash aXDash;

    aXDash.SetDashStyle((css::drawing::DashStyle)((sal_uInt16)(aLineDash.Style)));
    aXDash.SetDots(aLineDash.Dots);
    aXDash.SetDotLen(aLineDash.DotLen);
    aXDash.SetDashes(aLineDash.Dashes);
    aXDash.SetDashLen(aLineDash.DashLen);
    aXDash.SetDistance(aLineDash.Distance);

    return new XDashEntry( aXDash, rName );
}

// XElementAccess
uno::Type SAL_CALL SvxUnoXDashTable::getElementType()
    throw( uno::RuntimeException, std::exception )
{
    return cppu::UnoType<drawing::LineDash>::get();
}

// XServiceInfo
OUString SAL_CALL SvxUnoXDashTable::getImplementationName(  ) throw( uno::RuntimeException, std::exception )
{
    return OUString( "SvxUnoXDashTable" );
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXDashTable::getSupportedServiceNames(  ) throw( uno::RuntimeException, std::exception)
{
    const OUString aServiceName( "com.sun.star.drawing.DashTable" );
    uno::Sequence< OUString > aServices( &aServiceName, 1 );
    return aServices;
}


class SvxUnoXHatchTable : public SvxUnoXPropertyTable
{
public:
    explicit SvxUnoXHatchTable( XPropertyList* pTable ) throw() : SvxUnoXPropertyTable( XATTR_FILLHATCH, pTable ) {};

    // SvxUnoXPropertyTable
    virtual uno::Any getAny( const XPropertyEntry* pEntry ) const throw() override;
    virtual XPropertyEntry* getEntry( const OUString& rName, const uno::Any& rAny ) const throw() override;

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw( uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException, std::exception ) override;
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException, std::exception) override;
};

uno::Reference< uno::XInterface > SAL_CALL SvxUnoXHatchTable_createInstance( XPropertyList* pTable ) throw()
{
    return static_cast<OWeakObject*>(new SvxUnoXHatchTable( pTable ));
}

// SvxUnoXPropertyTable
uno::Any SvxUnoXHatchTable::getAny( const XPropertyEntry* pEntry ) const throw()
{
    const XHatch& aHatch = static_cast<const XHatchEntry*>(pEntry)->GetHatch();

    drawing::Hatch aUnoHatch;

    aUnoHatch.Style = (drawing::HatchStyle)aHatch.GetHatchStyle();
    aUnoHatch.Color = aHatch.GetColor().GetColor();
    aUnoHatch.Distance = aHatch.GetDistance();
    aUnoHatch.Angle = aHatch.GetAngle();

    return uno::Any(aUnoHatch);
}

XPropertyEntry* SvxUnoXHatchTable::getEntry( const OUString& rName, const uno::Any& rAny ) const throw()
{
    drawing::Hatch aUnoHatch;
    if(!(rAny >>= aUnoHatch))
        return nullptr;

    XHatch aXHatch;
    aXHatch.SetHatchStyle( (css::drawing::HatchStyle)aUnoHatch.Style );
    aXHatch.SetColor( aUnoHatch.Color );
    aXHatch.SetDistance( aUnoHatch.Distance );
    aXHatch.SetAngle( aUnoHatch.Angle );

    return new XHatchEntry( aXHatch, rName );
}

// XElementAccess
uno::Type SAL_CALL SvxUnoXHatchTable::getElementType()
    throw( uno::RuntimeException, std::exception )
{
    return cppu::UnoType<drawing::Hatch>::get();
}

// XServiceInfo
OUString SAL_CALL SvxUnoXHatchTable::getImplementationName(  ) throw( uno::RuntimeException, std::exception )
{
    return OUString( "SvxUnoXHatchTable" );
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXHatchTable::getSupportedServiceNames(  ) throw( uno::RuntimeException, std::exception)
{
    const OUString aServiceName( "com.sun.star.drawing.HatchTable" );
    uno::Sequence< OUString > aServices( &aServiceName, 1 );
    return aServices;
}


class SvxUnoXGradientTable : public SvxUnoXPropertyTable
{
public:
    explicit SvxUnoXGradientTable( XPropertyList* pTable ) throw() : SvxUnoXPropertyTable( XATTR_FILLGRADIENT, pTable ) {};

    // SvxUnoXPropertyTable
    virtual uno::Any getAny( const XPropertyEntry* pEntry ) const throw() override;
    virtual XPropertyEntry* getEntry( const OUString& rName, const uno::Any& rAny ) const throw() override;

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw( uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException, std::exception ) override;
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException, std::exception) override;
};

uno::Reference< uno::XInterface > SAL_CALL SvxUnoXGradientTable_createInstance( XPropertyList* pTable ) throw()
{
    return static_cast<OWeakObject*>(new SvxUnoXGradientTable( pTable ));
}

// SvxUnoXPropertyTable
uno::Any SvxUnoXGradientTable::getAny( const XPropertyEntry* pEntry ) const throw()
{
    const XGradient& aXGradient = static_cast<const XGradientEntry*>(pEntry)->GetGradient();
    awt::Gradient aGradient;

    aGradient.Style = (awt::GradientStyle) aXGradient.GetGradientStyle();
    aGradient.StartColor = (sal_Int32)aXGradient.GetStartColor().GetColor();
    aGradient.EndColor = (sal_Int32)aXGradient.GetEndColor().GetColor();
    aGradient.Angle = (short)aXGradient.GetAngle();
    aGradient.Border = aXGradient.GetBorder();
    aGradient.XOffset = aXGradient.GetXOffset();
    aGradient.YOffset = aXGradient.GetYOffset();
    aGradient.StartIntensity = aXGradient.GetStartIntens();
    aGradient.EndIntensity = aXGradient.GetEndIntens();
    aGradient.StepCount = aXGradient.GetSteps();

    return uno::Any(aGradient);
}

XPropertyEntry* SvxUnoXGradientTable::getEntry( const OUString& rName, const uno::Any& rAny ) const throw()
{
    awt::Gradient aGradient;
    if(!(rAny >>= aGradient))
        return nullptr;

    XGradient aXGradient;

    aXGradient.SetGradientStyle( (css::awt::GradientStyle) aGradient.Style );
    aXGradient.SetStartColor( aGradient.StartColor );
    aXGradient.SetEndColor( aGradient.EndColor );
    aXGradient.SetAngle( aGradient.Angle );
    aXGradient.SetBorder( aGradient.Border );
    aXGradient.SetXOffset( aGradient.XOffset );
    aXGradient.SetYOffset( aGradient.YOffset );
    aXGradient.SetStartIntens( aGradient.StartIntensity );
    aXGradient.SetEndIntens( aGradient.EndIntensity );
    aXGradient.SetSteps( aGradient.StepCount );

    return new XGradientEntry( aXGradient, rName );
}

// XElementAccess
uno::Type SAL_CALL SvxUnoXGradientTable::getElementType()
    throw( uno::RuntimeException, std::exception )
{
    return cppu::UnoType<awt::Gradient>::get();
}

// XServiceInfo
OUString SAL_CALL SvxUnoXGradientTable::getImplementationName(  ) throw( uno::RuntimeException, std::exception )
{
    return OUString( "SvxUnoXGradientTable" );
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXGradientTable::getSupportedServiceNames(  ) throw( uno::RuntimeException, std::exception)
{
    const OUString aServiceName( "com.sun.star.drawing.GradientTable" );
    uno::Sequence< OUString > aServices( &aServiceName, 1 );
    return aServices;
}


class SvxUnoXBitmapTable : public SvxUnoXPropertyTable
{
public:
    explicit SvxUnoXBitmapTable( XPropertyList* pTable ) throw() : SvxUnoXPropertyTable( XATTR_FILLBITMAP, pTable ) {};

    // SvxUnoXPropertyTable
    virtual uno::Any getAny( const XPropertyEntry* pEntry ) const throw(uno::RuntimeException) override;
    virtual XPropertyEntry* getEntry( const OUString& rName, const uno::Any& rAny ) const throw(uno::RuntimeException) override;

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw( uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException, std::exception ) override;
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException, std::exception) override;
};

uno::Reference< uno::XInterface > SAL_CALL SvxUnoXBitmapTable_createInstance( XPropertyList* pTable ) throw()
{
    return static_cast<OWeakObject*>(new SvxUnoXBitmapTable( pTable ));
}

// SvxUnoXPropertyTable
uno::Any SvxUnoXBitmapTable::getAny( const XPropertyEntry* pEntry ) const throw(uno::RuntimeException)
{
    OUString aURL( UNO_NAME_GRAPHOBJ_URLPREFIX);
    const GraphicObject& rGraphicObject(static_cast<const XBitmapEntry*>(pEntry)->GetGraphicObject());
    aURL += OStringToOUString(rGraphicObject.GetUniqueID(), RTL_TEXTENCODING_ASCII_US);

    return uno::Any(aURL);
}

XPropertyEntry* SvxUnoXBitmapTable::getEntry( const OUString& rName, const uno::Any& rAny ) const throw(uno::RuntimeException)
{
    OUString aURL;
    if(!(rAny >>= aURL))
        return nullptr;

    const GraphicObject aGrafObj(GraphicObject::CreateGraphicObjectFromURL(aURL));

    return new XBitmapEntry(aGrafObj, rName);
}

// XElementAccess
uno::Type SAL_CALL SvxUnoXBitmapTable::getElementType()
    throw( uno::RuntimeException, std::exception )
{
    return ::cppu::UnoType<OUString>::get();
}

// XServiceInfo
OUString SAL_CALL SvxUnoXBitmapTable::getImplementationName(  ) throw( uno::RuntimeException, std::exception )
{
    return OUString( "SvxUnoXBitmapTable" );
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXBitmapTable::getSupportedServiceNames(  ) throw( uno::RuntimeException, std::exception)
{
    const OUString aServiceName( "com.sun.star.drawing.BitmapTable" );
    uno::Sequence< OUString > aServices( &aServiceName, 1 );
    return aServices;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

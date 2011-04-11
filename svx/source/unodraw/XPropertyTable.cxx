/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include "svx/XPropertyTable.hxx"
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include <cppuhelper/implbase2.hxx>
#include "unopolyhelper.hxx"
#include <svx/xdef.hxx>

#include "svx/unoapi.hxx"
#include <editeng/unoprnms.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

using namespace com::sun::star;
using namespace ::cppu;
using namespace ::rtl;

class SvxUnoXPropertyTable : public WeakImplHelper2< container::XNameContainer, lang::XServiceInfo >
{
private:
    XPropertyTable* mpTable;
    XPropertyList*  mpList;
    sal_Int16 mnWhich;

    long getCount() const { return mpList ? mpList->Count() : (mpTable?mpTable->Count():0); }
    XPropertyEntry* get( long index ) const;
public:
    SvxUnoXPropertyTable( sal_Int16 nWhich, XPropertyList* pList ) throw();
    SvxUnoXPropertyTable( sal_Int16 nWhich, XPropertyTable* pTable ) throw();

    virtual ~SvxUnoXPropertyTable() throw();

    virtual uno::Any getAny( const XPropertyEntry* pEntry ) const throw() = 0;
    virtual XPropertyEntry* getEntry( const OUString& rName, const uno::Any& rAny ) const throw() = 0;

    // XServiceInfo
    virtual sal_Bool SAL_CALL supportsService( const  OUString& ServiceName ) throw( uno::RuntimeException);

    // XNameContainer
    virtual void SAL_CALL insertByName( const  OUString& aName, const  uno::Any& aElement ) throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException);
    virtual void SAL_CALL removeByName( const  OUString& Name ) throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);

    // XNameReplace
    virtual void SAL_CALL replaceByName( const  OUString& aName, const  uno::Any& aElement ) throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);

    // XNameAccess
    virtual uno::Any SAL_CALL getByName( const  OUString& aName ) throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);
    virtual uno::Sequence<  OUString > SAL_CALL getElementNames(  ) throw( uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const  OUString& aName ) throw( uno::RuntimeException);

    // XElementAccess
    virtual sal_Bool SAL_CALL hasElements(  ) throw( uno::RuntimeException);
};

SvxUnoXPropertyTable::SvxUnoXPropertyTable( sal_Int16 nWhich, XPropertyTable* pTable ) throw()
: mpTable( pTable ), mpList( NULL ), mnWhich( nWhich )
{
}

SvxUnoXPropertyTable::SvxUnoXPropertyTable( sal_Int16 nWhich, XPropertyList* pList ) throw()
: mpTable( NULL ), mpList( pList ), mnWhich( nWhich )
{
}

SvxUnoXPropertyTable::~SvxUnoXPropertyTable() throw()
{
}

XPropertyEntry* SvxUnoXPropertyTable::get( long index ) const
{
    if( mpTable )
        return mpTable->Get( index, 0 );
    else if( mpList )
        return mpList->Get( index, 0 );
    else
        return NULL;
}

// XServiceInfo
sal_Bool SAL_CALL SvxUnoXPropertyTable::supportsService( const  OUString& ServiceName )
    throw( uno::RuntimeException)
{
    const uno::Sequence< OUString > aServices( getSupportedServiceNames() );
    const OUString* pServices = aServices.getConstArray();
    const sal_Int32 nCount = aServices.getLength();
    sal_Int32 i;
    for( i = 0; i < nCount; i++ )
    {
        if( *pServices++ == ServiceName )
            return sal_True;
    }

    return sal_False;
}

// XNameContainer
void SAL_CALL SvxUnoXPropertyTable::insertByName( const  OUString& aName, const  uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( NULL == mpList && NULL == mpTable )
        throw lang::IllegalArgumentException();

    if( hasByName( aName ) )
        throw container::ElementExistException();

    String aInternalName;
    SvxUnogetInternalNameForItem( mnWhich, aName, aInternalName );

    XPropertyEntry* pNewEntry = getEntry( aInternalName, aElement );
    if( NULL == pNewEntry )
        throw lang::IllegalArgumentException();

    if( mpList )
        mpList->Insert( pNewEntry );
    else
        mpTable->Insert( mpTable->Count(), pNewEntry );
}

void SAL_CALL SvxUnoXPropertyTable::removeByName( const  OUString& Name )
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    String aInternalName;
    SvxUnogetInternalNameForItem( mnWhich, Name, aInternalName );

    const long nCount = getCount();
    long i;
    XPropertyEntry* pEntry;
    for( i = 0; i < nCount; i++ )
    {
        pEntry = get( i );
        if( pEntry && pEntry->GetName() == aInternalName )
        {
            if( mpList )
                delete mpList->Remove( i, 0 );
            else
                delete mpTable->Remove( i, 0 );
            return;
        }
    }

    throw container::NoSuchElementException();
}

// XNameReplace
void SAL_CALL SvxUnoXPropertyTable::replaceByName( const  OUString& aName, const  uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    String aInternalName;
    SvxUnogetInternalNameForItem( mnWhich, aName, aInternalName );

    const long nCount = getCount();
    long i;
    XPropertyEntry* pEntry;
    for( i = 0; i < nCount; i++ )
    {
        pEntry = get( i );
        if( pEntry && pEntry->GetName() == aInternalName )
        {
            XPropertyEntry* pNewEntry = getEntry( aInternalName, aElement );
            if( NULL == pNewEntry )
                throw lang::IllegalArgumentException();

            if( mpList )
                delete mpList->Replace( pNewEntry, i );
            else
                delete mpTable->Replace( i, pNewEntry );
            return;
        }
    }

    throw container::NoSuchElementException();
}

// XNameAccess
uno::Any SAL_CALL SvxUnoXPropertyTable::getByName( const  OUString& aName )
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    String aInternalName;
    SvxUnogetInternalNameForItem( mnWhich, aName, aInternalName );

    const long nCount = getCount();
    long i;
    XPropertyEntry* pEntry;
    for( i = 0; i < nCount; i++ )
    {
        pEntry = get( i );

        if( pEntry && pEntry->GetName() == aInternalName )
            return getAny( pEntry );
    }

    throw container::NoSuchElementException();
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXPropertyTable::getElementNames()
    throw( uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    const long nCount = getCount();
    uno::Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();
    long i;
    XPropertyEntry* pEntry;
    for( i = 0; i < nCount; i++ )
    {
        pEntry = get( i );

        if( pEntry )
        {
            SvxUnogetApiNameForItem( mnWhich, pEntry->GetName(), *pNames );
            pNames++;
        }
    }

    return aNames;
}

sal_Bool SAL_CALL SvxUnoXPropertyTable::hasByName( const  OUString& aName )
    throw( uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    String aInternalName;
    SvxUnogetInternalNameForItem( mnWhich, aName, aInternalName );

    const long nCount = mpList?mpList->Count():0;
    long i;
    XPropertyEntry* pEntry;
    for( i = 0; i < nCount; i++ )
    {
        pEntry = get( i );
        if( pEntry && pEntry->GetName() == aInternalName )
            return sal_True;
    }

    return sal_False;
}

// XElementAccess
sal_Bool SAL_CALL SvxUnoXPropertyTable::hasElements(  )
    throw( uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    return getCount() != 0;
}

///////////////////////////////////////////////////////////////////////

class SvxUnoXColorTable : public SvxUnoXPropertyTable
{
public:
    SvxUnoXColorTable( XPropertyTable* pTable ) throw() : SvxUnoXPropertyTable( XATTR_LINECOLOR, pTable ) {};

    // SvxUnoXPropertyTable
    virtual uno::Any getAny( const XPropertyEntry* pEntry ) const throw();
    virtual XPropertyEntry* getEntry( const OUString& rName, const uno::Any& rAny ) const throw();

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw( uno::RuntimeException );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException );
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException);
};

uno::Reference< uno::XInterface > SAL_CALL SvxUnoXColorTable_createInstance( XPropertyTable* pTable ) throw()
{
    return (OWeakObject*) new SvxUnoXColorTable( pTable );
}

// SvxUnoXPropertyTable
uno::Any SvxUnoXColorTable::getAny( const XPropertyEntry* pEntry ) const throw()
{
    uno::Any aAny;
    aAny <<= (sal_Int32)((XColorEntry*)pEntry)->GetColor().GetColor();
    return aAny;
}

XPropertyEntry* SvxUnoXColorTable::getEntry( const OUString& rName, const uno::Any& rAny ) const throw()
{
    sal_Int32 nColor = 0;
    if( !(rAny >>= nColor) )
        return NULL;

    const Color aColor( (ColorData)nColor );
    const String aName( rName );
    return new XColorEntry( aColor, aName );
}

// XElementAccess
uno::Type SAL_CALL SvxUnoXColorTable::getElementType()
    throw( uno::RuntimeException )
{
    return ::getCppuType((const sal_Int32*)0);
}

// XServiceInfo
OUString SAL_CALL SvxUnoXColorTable::getImplementationName(  ) throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SvxUnoXColorTable" ) );
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXColorTable::getSupportedServiceNames(  ) throw( uno::RuntimeException)
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.ColorTable" ) );
    uno::Sequence< OUString > aServices( &aServiceName, 1 );
    return aServices;
}

///////////////////////////////////////////////////////////////////////

class SvxUnoXLineEndTable : public SvxUnoXPropertyTable
{
public:
    SvxUnoXLineEndTable( XPropertyList* pTable ) throw() : SvxUnoXPropertyTable( XATTR_LINEEND, pTable ) {};

    // SvxUnoXPropertyTable
    virtual uno::Any getAny( const XPropertyEntry* pEntry ) const throw();
    virtual XPropertyEntry* getEntry( const OUString& rName, const uno::Any& rAny ) const throw();

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw( uno::RuntimeException );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException );
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException);
};

uno::Reference< uno::XInterface > SAL_CALL SvxUnoXLineEndTable_createInstance( XPropertyList* pTable ) throw()
{
    return (OWeakObject*)new SvxUnoXLineEndTable( pTable );
}

// SvxUnoXPropertyTable
uno::Any SvxUnoXLineEndTable::getAny( const XPropertyEntry* pEntry ) const throw()
{

    uno::Any aAny;
    drawing::PolyPolygonBezierCoords aBezier;
    SvxConvertB2DPolyPolygonToPolyPolygonBezier( ((XLineEndEntry*)pEntry)->GetLineEnd(), aBezier );
    aAny <<= aBezier;
    return aAny;
}

XPropertyEntry* SvxUnoXLineEndTable::getEntry( const OUString& rName, const uno::Any& rAny ) const throw()
{

    if( !rAny.getValue() || rAny.getValueType() != ::getCppuType((const drawing::PolyPolygonBezierCoords*)0) )
        return NULL;

    basegfx::B2DPolyPolygon aPolyPolygon;
    drawing::PolyPolygonBezierCoords* pCoords = (drawing::PolyPolygonBezierCoords*)rAny.getValue();
    if( pCoords->Coordinates.getLength() > 0 )
        aPolyPolygon = SvxConvertPolyPolygonBezierToB2DPolyPolygon( pCoords );

    // #86265# make sure polygon is closed
    aPolyPolygon.setClosed(true);

    const String aName( rName );
    return new XLineEndEntry( aPolyPolygon, aName );
}

// XElementAccess
uno::Type SAL_CALL SvxUnoXLineEndTable::getElementType()
    throw( uno::RuntimeException )
{
    return ::getCppuType((const drawing::PolyPolygonBezierCoords*)0);
}

// XServiceInfo
OUString SAL_CALL SvxUnoXLineEndTable::getImplementationName(  ) throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SvxUnoXLineEndTable" ) );
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXLineEndTable::getSupportedServiceNames(  ) throw( uno::RuntimeException)
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.LineEndTable" ) );
    uno::Sequence< OUString > aServices( &aServiceName, 1 );
    return aServices;
}

///////////////////////////////////////////////////////////////////////

class SvxUnoXDashTable : public SvxUnoXPropertyTable
{
public:
    SvxUnoXDashTable( XPropertyList* pTable ) throw() : SvxUnoXPropertyTable( XATTR_LINEDASH, pTable ) {};

    // SvxUnoXPropertyTable
    virtual uno::Any getAny( const XPropertyEntry* pEntry ) const throw();
    virtual XPropertyEntry* getEntry( const OUString& rName, const uno::Any& rAny ) const throw();

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw( uno::RuntimeException );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException );
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException);
};

uno::Reference< uno::XInterface > SAL_CALL SvxUnoXDashTable_createInstance( XPropertyList* pTable ) throw()
{
    return (OWeakObject*)new SvxUnoXDashTable( pTable );
}

// SvxUnoXPropertyTable
uno::Any SvxUnoXDashTable::getAny( const XPropertyEntry* pEntry ) const throw()
{
    const XDash& rXD = ((XDashEntry*)pEntry)->GetDash();

    drawing::LineDash aLineDash;

    aLineDash.Style = (::com::sun::star::drawing::DashStyle)((sal_uInt16)rXD.GetDashStyle());
    aLineDash.Dots = rXD.GetDots();
    aLineDash.DotLen = rXD.GetDotLen();
    aLineDash.Dashes = rXD.GetDashes();
    aLineDash.DashLen = rXD.GetDashLen();
    aLineDash.Distance = rXD.GetDistance();

    uno::Any aAny;
    aAny <<= aLineDash;
    return aAny;
}

XPropertyEntry* SvxUnoXDashTable::getEntry( const OUString& rName, const uno::Any& rAny ) const throw()
{
    drawing::LineDash aLineDash;
    if(!(rAny >>= aLineDash))
        return NULL;

    XDash aXDash;

    aXDash.SetDashStyle((XDashStyle)((sal_uInt16)(aLineDash.Style)));
    aXDash.SetDots(aLineDash.Dots);
    aXDash.SetDotLen(aLineDash.DotLen);
    aXDash.SetDashes(aLineDash.Dashes);
    aXDash.SetDashLen(aLineDash.DashLen);
    aXDash.SetDistance(aLineDash.Distance);

    const String aName( rName );
    return new XDashEntry( aXDash, aName );
}

// XElementAccess
uno::Type SAL_CALL SvxUnoXDashTable::getElementType()
    throw( uno::RuntimeException )
{
    return ::getCppuType((const drawing::LineDash*)0);
}

// XServiceInfo
OUString SAL_CALL SvxUnoXDashTable::getImplementationName(  ) throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SvxUnoXDashTable" ) );
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXDashTable::getSupportedServiceNames(  ) throw( uno::RuntimeException)
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.DashTable" ) );
    uno::Sequence< OUString > aServices( &aServiceName, 1 );
    return aServices;
}

///////////////////////////////////////////////////////////////////////

class SvxUnoXHatchTable : public SvxUnoXPropertyTable
{
public:
    SvxUnoXHatchTable( XPropertyList* pTable ) throw() : SvxUnoXPropertyTable( XATTR_FILLHATCH, pTable ) {};

    // SvxUnoXPropertyTable
    virtual uno::Any getAny( const XPropertyEntry* pEntry ) const throw();
    virtual XPropertyEntry* getEntry( const OUString& rName, const uno::Any& rAny ) const throw();

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw( uno::RuntimeException );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException );
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException);
};

uno::Reference< uno::XInterface > SAL_CALL SvxUnoXHatchTable_createInstance( XPropertyList* pTable ) throw()
{
    return (OWeakObject*)new SvxUnoXHatchTable( pTable );
}

// SvxUnoXPropertyTable
uno::Any SvxUnoXHatchTable::getAny( const XPropertyEntry* pEntry ) const throw()
{
    const XHatch& aHatch = ((XHatchEntry*)pEntry)->GetHatch();

    drawing::Hatch aUnoHatch;

    aUnoHatch.Style = (drawing::HatchStyle)aHatch.GetHatchStyle();
    aUnoHatch.Color = aHatch.GetColor().GetColor();
    aUnoHatch.Distance = aHatch.GetDistance();
    aUnoHatch.Angle = aHatch.GetAngle();

    uno::Any aAny;
    aAny <<= aUnoHatch;
    return aAny;
}

XPropertyEntry* SvxUnoXHatchTable::getEntry( const OUString& rName, const uno::Any& rAny ) const throw()
{
    drawing::Hatch aUnoHatch;
    if(!(rAny >>= aUnoHatch))
        return NULL;

    XHatch aXHatch;
    aXHatch.SetHatchStyle( (XHatchStyle)aUnoHatch.Style );
    aXHatch.SetColor( aUnoHatch.Color );
    aXHatch.SetDistance( aUnoHatch.Distance );
    aXHatch.SetAngle( aUnoHatch.Angle );

    const String aName( rName );
    return new XHatchEntry( aXHatch, aName );
}

// XElementAccess
uno::Type SAL_CALL SvxUnoXHatchTable::getElementType()
    throw( uno::RuntimeException )
{
    return ::getCppuType((const drawing::Hatch*)0);
}

// XServiceInfo
OUString SAL_CALL SvxUnoXHatchTable::getImplementationName(  ) throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SvxUnoXHatchTable" ) );
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXHatchTable::getSupportedServiceNames(  ) throw( uno::RuntimeException)
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.HatchTable" ) );
    uno::Sequence< OUString > aServices( &aServiceName, 1 );
    return aServices;
}

///////////////////////////////////////////////////////////////////////

class SvxUnoXGradientTable : public SvxUnoXPropertyTable
{
public:
    SvxUnoXGradientTable( XPropertyList* pTable ) throw() : SvxUnoXPropertyTable( XATTR_FILLGRADIENT, pTable ) {};

    // SvxUnoXPropertyTable
    virtual uno::Any getAny( const XPropertyEntry* pEntry ) const throw();
    virtual XPropertyEntry* getEntry( const OUString& rName, const uno::Any& rAny ) const throw();

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw( uno::RuntimeException );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException );
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException);
};

uno::Reference< uno::XInterface > SAL_CALL SvxUnoXGradientTable_createInstance( XPropertyList* pTable ) throw()
{
    return (OWeakObject*)new SvxUnoXGradientTable( pTable );
}

// SvxUnoXPropertyTable
uno::Any SvxUnoXGradientTable::getAny( const XPropertyEntry* pEntry ) const throw()
{
    const XGradient& aXGradient = ((XGradientEntry*)pEntry)->GetGradient();
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

    uno::Any aAny;
    aAny <<= aGradient;
    return aAny;
}

XPropertyEntry* SvxUnoXGradientTable::getEntry( const OUString& rName, const uno::Any& rAny ) const throw()
{
    awt::Gradient aGradient;
    if(!(rAny >>= aGradient))
        return NULL;

    XGradient aXGradient;

    aXGradient.SetGradientStyle( (XGradientStyle) aGradient.Style );
    aXGradient.SetStartColor( aGradient.StartColor );
    aXGradient.SetEndColor( aGradient.EndColor );
    aXGradient.SetAngle( aGradient.Angle );
    aXGradient.SetBorder( aGradient.Border );
    aXGradient.SetXOffset( aGradient.XOffset );
    aXGradient.SetYOffset( aGradient.YOffset );
    aXGradient.SetStartIntens( aGradient.StartIntensity );
    aXGradient.SetEndIntens( aGradient.EndIntensity );
    aXGradient.SetSteps( aGradient.StepCount );

    const String aName( rName );
    return new XGradientEntry( aXGradient, aName );
}

// XElementAccess
uno::Type SAL_CALL SvxUnoXGradientTable::getElementType()
    throw( uno::RuntimeException )
{
    return ::getCppuType((const awt::Gradient*)0);
}

// XServiceInfo
OUString SAL_CALL SvxUnoXGradientTable::getImplementationName(  ) throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SvxUnoXGradientTable" ) );
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXGradientTable::getSupportedServiceNames(  ) throw( uno::RuntimeException)
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.GradientTable" ) );
    uno::Sequence< OUString > aServices( &aServiceName, 1 );
    return aServices;
}

///////////////////////////////////////////////////////////////////////

class SvxUnoXBitmapTable : public SvxUnoXPropertyTable
{
public:
    SvxUnoXBitmapTable( XPropertyList* pTable ) throw() : SvxUnoXPropertyTable( XATTR_FILLBITMAP, pTable ) {};

    // SvxUnoXPropertyTable
    virtual uno::Any getAny( const XPropertyEntry* pEntry ) const throw();
    virtual XPropertyEntry* getEntry( const OUString& rName, const uno::Any& rAny ) const throw();

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw( uno::RuntimeException );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException );
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException);
};

uno::Reference< uno::XInterface > SAL_CALL SvxUnoXBitmapTable_createInstance( XPropertyList* pTable ) throw()
{
    return (OWeakObject*)new SvxUnoXBitmapTable( pTable );
}

// SvxUnoXPropertyTable
uno::Any SvxUnoXBitmapTable::getAny( const XPropertyEntry* pEntry ) const throw()
{
    OUString aURL( RTL_CONSTASCII_USTRINGPARAM(UNO_NAME_GRAPHOBJ_URLPREFIX));
    aURL += OUString::createFromAscii( ((XBitmapEntry*)pEntry)->GetXBitmap().GetGraphicObject().GetUniqueID().GetBuffer() );

    uno::Any aAny;
    aAny <<= aURL;
    return aAny;
}

XPropertyEntry* SvxUnoXBitmapTable::getEntry( const OUString& rName, const uno::Any& rAny ) const throw()
{
    OUString aURL;
    if(!(rAny >>= aURL))
        return NULL;

    GraphicObject aGrafObj( GraphicObject::CreateGraphicObjectFromURL( aURL ) );
    XOBitmap aBMP( aGrafObj );

    const String aName( rName );
    return new XBitmapEntry( aBMP, aName );
}

// XElementAccess
uno::Type SAL_CALL SvxUnoXBitmapTable::getElementType()
    throw( uno::RuntimeException )
{
    return ::getCppuType((const OUString*)0);
}

// XServiceInfo
OUString SAL_CALL SvxUnoXBitmapTable::getImplementationName(  ) throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SvxUnoXBitmapTable" ) );
}

uno::Sequence<  OUString > SAL_CALL SvxUnoXBitmapTable::getSupportedServiceNames(  ) throw( uno::RuntimeException)
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.BitmapTable" ) );
    uno::Sequence< OUString > aServices( &aServiceName, 1 );
    return aServices;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

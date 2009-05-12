/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: display.cxx,v $
 * $Revision: 1.6 $
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
#include "precompiled_vcl.hxx"
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#include <vcl/svapp.hxx>

#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase4.hxx>

#include <vector>
#include <tools/debug.hxx>


using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;

// -----------------------------------------------------------------------

namespace vcl
{

class DisplayInfo : public ::cppu::WeakAggImplHelper3< XPropertySet, XPropertySetInfo, XServiceInfo >
{
public:
    DisplayInfo( sal_uInt32 nDisplay );

    // XPropertySet
    virtual Reference< XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (RuntimeException);
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const Any& aValue ) throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException);
    virtual Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException);

    // XPropertySetInfo
    virtual Sequence< Property > SAL_CALL getProperties(  ) throw (RuntimeException);
    virtual Property SAL_CALL getPropertyByName( const OUString& aName ) throw (UnknownPropertyException, RuntimeException);
    virtual ::sal_Bool SAL_CALL hasPropertyByName( const OUString& Name ) throw (RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (RuntimeException);

private:
    sal_uInt32 mnDisplay;
};

static const char* pScreenAreaName = "ScreenArea";
static const char* pWorkAreaName = "WorkArea";
static const char* pScreenName = "ScreenName";

// --------------------------------------------------------------------

DisplayInfo::DisplayInfo( sal_uInt32 nDisplay )
: mnDisplay( nDisplay )
{
}

// XPropertySet
Reference< XPropertySetInfo > SAL_CALL DisplayInfo::getPropertySetInfo(  ) throw (RuntimeException)
{
    return this;
}

void SAL_CALL DisplayInfo::setPropertyValue( const OUString& /*aPropertyName* */, const Any& /*aValue*/ ) throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    throw PropertyVetoException();
}

Any SAL_CALL DisplayInfo::getPropertyValue( const OUString& PropertyName ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    Rectangle aRect;
    if( PropertyName.equalsAscii( pScreenAreaName ) )
    {
        aRect = Application::GetScreenPosSizePixel( mnDisplay );
    }
    else if( PropertyName.equalsAscii( pWorkAreaName ) )
    {
        aRect = Application::GetWorkAreaPosSizePixel( mnDisplay );
    }
    else if( PropertyName.equalsAscii( pScreenName ) )
    {
        return Any( Application::GetScreenName( mnDisplay ) );
    }
    else
        throw UnknownPropertyException();

    return Any( com::sun::star::awt::Rectangle( aRect.Left(), aRect.Top(), aRect.getWidth(), aRect.getHeight() ) );
}

void SAL_CALL DisplayInfo::addPropertyChangeListener( const OUString&, const Reference< XPropertyChangeListener >& ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException) {}
void SAL_CALL DisplayInfo::removePropertyChangeListener( const OUString&, const Reference< XPropertyChangeListener >& ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException) {}
void SAL_CALL DisplayInfo::addVetoableChangeListener( const OUString&, const Reference< XVetoableChangeListener >& ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException) {}
void SAL_CALL DisplayInfo::removeVetoableChangeListener( const OUString&, const Reference< XVetoableChangeListener >& ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException) {}

// XPropertySetInfo
Sequence< Property > SAL_CALL DisplayInfo::getProperties(  ) throw (RuntimeException)
{
    Sequence< Property > aProps(2);
    aProps[0] = getPropertyByName( OUString::createFromAscii( pScreenAreaName ) );
    aProps[1] = getPropertyByName( OUString::createFromAscii( pWorkAreaName ) );
    return aProps;
}

Property SAL_CALL DisplayInfo::getPropertyByName( const OUString& aName ) throw (UnknownPropertyException, RuntimeException)
{
    if( aName.equalsAscii( pScreenAreaName ) ||
        aName.equalsAscii( pWorkAreaName ) )
        return Property( aName, 0, ::getCppuType( (::com::sun::star::awt::Rectangle const *)0 ), PropertyAttribute::READONLY );
    throw UnknownPropertyException();
}

::sal_Bool SAL_CALL DisplayInfo::hasPropertyByName( const OUString& Name ) throw (RuntimeException)
{
    return Name.equalsAscii( pScreenAreaName ) ||
           Name.equalsAscii( pWorkAreaName );
}

// XServiceInfo
OUString SAL_CALL DisplayInfo::getImplementationName(  ) throw (RuntimeException)
{
    static OUString aImplementationName( RTL_CONSTASCII_USTRINGPARAM( "vcl::DisplayInfo" ) );
    return aImplementationName;
}

::sal_Bool SAL_CALL DisplayInfo::supportsService( const OUString& ServiceName ) throw (RuntimeException)
{
    Sequence< OUString > aSN( getSupportedServiceNames() );
    for( sal_Int32 nService = 0; nService < aSN.getLength(); nService++ )
    {
        if( aSN[nService] == ServiceName )
            return sal_True;
    }
    return sal_False;
}

Sequence< OUString > SAL_CALL DisplayInfo::getSupportedServiceNames(  ) throw (RuntimeException)
{
    static OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.DisplayInfo" ) );
    static Sequence< OUString > aServiceNames( &aServiceName, 1  );
    return aServiceNames;
}

// ====================================================================

class DisplayAccess : public ::cppu::WeakAggImplHelper4< XPropertySet, XPropertySetInfo, XIndexAccess, XServiceInfo >
{
public:
    DisplayAccess ();

    // XPropertySet
    virtual Reference< XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (RuntimeException);
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const Any& aValue ) throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException);
    virtual Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException);

    // XPropertySetInfo
    virtual Sequence< Property > SAL_CALL getProperties(  ) throw (RuntimeException);
    virtual Property SAL_CALL getPropertyByName( const OUString& aName ) throw (UnknownPropertyException, RuntimeException);
    virtual ::sal_Bool SAL_CALL hasPropertyByName( const OUString& Name ) throw (RuntimeException);

    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount() throw (RuntimeException);
    virtual Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (IndexOutOfBoundsException, WrappedTargetException, RuntimeException);

    // XElementAccess
    virtual Type SAL_CALL getElementType(  ) throw (RuntimeException);
    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (RuntimeException);
};

Sequence< OUString > DisplayAccess_getSupportedServiceNames()
{
    static OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.DisplayAccess" ) );
    static Sequence< OUString > aServiceNames( &aServiceName, 1 );
    return aServiceNames;
}

OUString DisplayAccess_getImplementationName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "vcl::DisplayAccess" ) );
}

Reference< XInterface > SAL_CALL DisplayAccess_createInstance( const Reference< XMultiServiceFactory >&  )
{
    return static_cast< ::cppu::OWeakObject * >( new DisplayAccess );
}

DisplayAccess::DisplayAccess()
{
}

static const char* pMultiDisplayName = "MultiDisplay";
static const char* pDefaultDisplayName = "DefaultDisplay";

// XPropertySet
Reference< XPropertySetInfo > SAL_CALL DisplayAccess::getPropertySetInfo(  ) throw (RuntimeException)
{
    return this;
}

void SAL_CALL DisplayAccess::setPropertyValue( const OUString& /*aPropertyName* */, const Any& /*aValue*/ ) throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    throw PropertyVetoException();
}

Any SAL_CALL DisplayAccess::getPropertyValue( const OUString& PropertyName ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    Any aRet;
    if( PropertyName.equalsAscii( pMultiDisplayName ) )
    {
        aRet <<= sal_Bool( Application::IsMultiDisplay() );
    }
    else if( PropertyName.equalsAscii( pDefaultDisplayName ) )
    {
        aRet <<= sal_Int32( Application::GetDefaultDisplayNumber() );
    }
    else
        throw UnknownPropertyException();

    return aRet;
}

void SAL_CALL DisplayAccess::addPropertyChangeListener( const OUString&, const Reference< XPropertyChangeListener >& ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException) {}
void SAL_CALL DisplayAccess::removePropertyChangeListener( const OUString&, const Reference< XPropertyChangeListener >& ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException) {}
void SAL_CALL DisplayAccess::addVetoableChangeListener( const OUString&, const Reference< XVetoableChangeListener >& ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException) {}
void SAL_CALL DisplayAccess::removeVetoableChangeListener( const OUString&, const Reference< XVetoableChangeListener >& ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException) {}

// XPropertySetInfo
Sequence< Property > SAL_CALL DisplayAccess::getProperties(  ) throw (RuntimeException)
{
    Sequence< Property > aProps(2);
    aProps[0] = getPropertyByName( OUString::createFromAscii( pMultiDisplayName ) );
    aProps[1] = getPropertyByName( OUString::createFromAscii( pDefaultDisplayName ) );
    return aProps;
}

Property SAL_CALL DisplayAccess::getPropertyByName( const OUString& aName ) throw (UnknownPropertyException, RuntimeException)
{
    if( aName.equalsAscii( pMultiDisplayName ) )
        return Property( aName, 0, ::getCppuType( (sal_Bool const *)0 ), PropertyAttribute::READONLY );

    if( aName.equalsAscii( pDefaultDisplayName ) )
        return Property( aName, 0, ::getCppuType( (sal_Int32 const *)0 ), PropertyAttribute::READONLY );
    throw UnknownPropertyException();
}

::sal_Bool SAL_CALL DisplayAccess::hasPropertyByName( const OUString& Name ) throw (RuntimeException)
{
    return Name.equalsAscii( pMultiDisplayName ) ||
           Name.equalsAscii( pDefaultDisplayName );
}

// XIndexAccess
::sal_Int32 SAL_CALL DisplayAccess::getCount() throw (RuntimeException)
{
    return Application::GetScreenCount();
}

Any SAL_CALL DisplayAccess::getByIndex( ::sal_Int32 Index ) throw (IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    if( (Index < 0) || (Index >= getCount()) )
        throw IndexOutOfBoundsException();

    return makeAny( Reference< XPropertySet >( new DisplayInfo( Index ) ) );
}

// XElementAccess
Type SAL_CALL DisplayAccess::getElementType(  ) throw (RuntimeException)
{
    return XPropertySet::static_type();
}

::sal_Bool SAL_CALL DisplayAccess::hasElements() throw (RuntimeException)
{
    return true;
}

// XServiceInfo
OUString SAL_CALL DisplayAccess::getImplementationName(  ) throw (RuntimeException)
{
    return DisplayAccess_getImplementationName();
}

::sal_Bool SAL_CALL DisplayAccess::supportsService( const OUString& ServiceName ) throw (RuntimeException)
{
    Sequence< OUString > aSN( DisplayAccess_getSupportedServiceNames() );
    for( sal_Int32 nService = 0; nService < aSN.getLength(); nService++ )
    {
        if( aSN[nService] == ServiceName )
            return sal_True;
    }
    return sal_False;
}

Sequence< OUString > SAL_CALL DisplayAccess::getSupportedServiceNames(  ) throw (RuntimeException)
{
    return DisplayAccess_getSupportedServiceNames();
}

} // namespace vcl

/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: genericpropertyset.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:56:49 $
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

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _CPPUHELPER_WEAKAGG_HXX_
#include <cppuhelper/weakagg.hxx>
#endif

#ifndef _COMPHELPER_PROPERTYSETHELPER_HXX_
#include <comphelper/propertysethelper.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _COMPHELPER_GENERICPROPERTYSET_HXX_
#include <comphelper/genericpropertyset.hxx>
#endif

#ifndef _COMPHELPER_PROPERTSETINFO_HXX_
#include <comphelper/propertysetinfo.hxx>
#endif

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif

///////////////////////////////////////////////////////////////////////

using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

DECLARE_STL_USTRINGACCESS_MAP( Any, GenericAnyMapImpl );

namespace comphelper
{
    struct IMPL_GenericPropertySet_MutexContainer
    {
        Mutex maMutex ;
    } ;

    class GenericPropertySet :  public OWeakAggObject,
                                public XServiceInfo,
                                public XTypeProvider,
                                public PropertySetHelper,
                                private IMPL_GenericPropertySet_MutexContainer
    {
    private:
        GenericAnyMapImpl   maAnyMap;

    protected:
        virtual void _setPropertyValues( const PropertyMapEntry** ppEntries, const  Any* pValues ) throw( UnknownPropertyException,  PropertyVetoException,  IllegalArgumentException,  WrappedTargetException );
        virtual void _getPropertyValues( const PropertyMapEntry** ppEntries,  Any* pValue ) throw( UnknownPropertyException,  WrappedTargetException );

    public:
        GenericPropertySet( PropertySetInfo* pInfo ) throw();
        virtual ~GenericPropertySet() throw();

        // XInterface
        virtual  Any SAL_CALL queryAggregation( const  Type & rType ) throw( RuntimeException);
        virtual  Any SAL_CALL queryInterface( const  Type & rType ) throw( RuntimeException);
        virtual void SAL_CALL acquire() throw();
        virtual void SAL_CALL release() throw();

        // XTypeProvider
        virtual  Sequence<  Type > SAL_CALL getTypes(  ) throw( RuntimeException);
        virtual  Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw( RuntimeException);

        // XServiceInfo
        virtual rtl::OUString SAL_CALL getImplementationName() throw(  RuntimeException );
        virtual sal_Bool SAL_CALL supportsService( const rtl::OUString& ServiceName ) throw(  RuntimeException );
        virtual  Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() throw(  RuntimeException );

    };

}

///////////////////////////////////////////////////////////////////////

GenericPropertySet::GenericPropertySet( PropertySetInfo* pInfo ) throw()
: PropertySetHelper( pInfo )
{
}

GenericPropertySet::~GenericPropertySet() throw()
{
}

void GenericPropertySet::_setPropertyValues( const PropertyMapEntry** ppEntries, const Any* pValues )
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException )
{
    MutexGuard aGuard( maMutex );

    while( *ppEntries )
    {
        const OUString aPropertyName( (*ppEntries)->mpName, (*ppEntries)->mnNameLen, RTL_TEXTENCODING_ASCII_US );
        maAnyMap[ aPropertyName ] = *pValues;

        ppEntries++;
        pValues++;
    }
}

void GenericPropertySet::_getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, Any* pValue )
    throw( UnknownPropertyException, WrappedTargetException )
{
    MutexGuard aGuard( maMutex );

    while( *ppEntries )
    {
        const OUString aPropertyName( (*ppEntries)->mpName, (*ppEntries)->mnNameLen, RTL_TEXTENCODING_ASCII_US );
        *pValue = maAnyMap[ aPropertyName ];

        ppEntries++;
        pValue++;
    }
}

// XInterface

Any SAL_CALL GenericPropertySet::queryInterface( const Type & rType )
    throw( RuntimeException )
{
    return OWeakAggObject::queryInterface( rType );
}

Any SAL_CALL GenericPropertySet::queryAggregation( const Type & rType )
    throw(RuntimeException)
{
    Any aAny;

    if( rType == ::getCppuType((const Reference< XServiceInfo >*)0) )
        aAny <<= Reference< XServiceInfo >(this);
    else if( rType == ::getCppuType((const Reference< XTypeProvider >*)0) )
        aAny <<= Reference< XTypeProvider >(this);
    else if( rType == ::getCppuType((const Reference< XPropertySet >*)0) )
        aAny <<= Reference< XPropertySet >(this);
    else if( rType == ::getCppuType((const Reference< XMultiPropertySet >*)0) )
        aAny <<= Reference< XMultiPropertySet >(this);
    else
        aAny <<= OWeakAggObject::queryAggregation( rType );

    return aAny;
}

void SAL_CALL GenericPropertySet::acquire() throw()
{
    OWeakAggObject::acquire();
}

void SAL_CALL GenericPropertySet::release() throw()
{
    OWeakAggObject::release();
}

uno::Sequence< uno::Type > SAL_CALL GenericPropertySet::getTypes()
    throw (uno::RuntimeException)
{
    uno::Sequence< uno::Type > aTypes( 5 );
    uno::Type* pTypes = aTypes.getArray();

    *pTypes++ = ::getCppuType((const uno::Reference< XAggregation>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< XServiceInfo>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< XTypeProvider>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< XPropertySet>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< XMultiPropertySet>*)0);

    return aTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL GenericPropertySet::getImplementationId()
    throw (uno::RuntimeException)
{
    MutexGuard aGuard( maMutex );

    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

// XServiceInfo

sal_Bool SAL_CALL GenericPropertySet::supportsService( const  OUString& ServiceName ) throw(RuntimeException)
{
    Sequence< OUString > aSNL( getSupportedServiceNames() );
    const OUString * pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); ++i )
        if( pArray[i] == ServiceName )
            return sal_True;

    return sal_False;
}

OUString SAL_CALL GenericPropertySet::getImplementationName() throw( RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.comphelper.GenericPropertySet") );
}

Sequence< OUString > SAL_CALL GenericPropertySet::getSupportedServiceNames(  )
    throw( RuntimeException )
{
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.beans.XPropertySet" ));
    return aSNS;
}

::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > comphelper::GenericPropertySet_CreateInstance( comphelper::PropertySetInfo* pInfo )
{
    return (XPropertySet*)new GenericPropertySet( pInfo );
}


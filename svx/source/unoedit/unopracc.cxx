/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unopracc.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 06:18:05 $
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

//------------------------------------------------------------------------
//
// Global header
//
//------------------------------------------------------------------------

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif


//------------------------------------------------------------------------
//
// Project-local header
//
//------------------------------------------------------------------------

#ifndef _SVX_UNOPRACC_HXX_
#include "unopracc.hxx"
#endif

#ifndef _SVX_UNOEDSRC_HXX
#include "unoedsrc.hxx"
#endif

using namespace ::com::sun::star;

//------------------------------------------------------------------------
//
// SvxAccessibleTextPropertySet implementation
//
//------------------------------------------------------------------------

SvxAccessibleTextPropertySet::SvxAccessibleTextPropertySet( const SvxEditSource* pEditSrc, const SfxItemPropertyMap* pPropMap )
    : SvxUnoTextRangeBase( pEditSrc, pPropMap )
{
}

SvxAccessibleTextPropertySet::~SvxAccessibleTextPropertySet() throw()
{
}

uno::Reference< text::XText > SAL_CALL SvxAccessibleTextPropertySet::getText() throw (uno::RuntimeException)
{
  // TODO (empty?)
  return uno::Reference< text::XText > ();
}

uno::Any SAL_CALL SvxAccessibleTextPropertySet::queryAggregation( const uno::Type & ) throw(uno::RuntimeException)
{
  // TODO (empty?)
  return uno::Any();
}

uno::Any SAL_CALL SvxAccessibleTextPropertySet::queryInterface( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    return OWeakObject::queryInterface(rType);
}

void SAL_CALL SvxAccessibleTextPropertySet::acquire()
    throw()
{
    OWeakObject::acquire();
}

void SAL_CALL SvxAccessibleTextPropertySet::release()
    throw()
{
    OWeakObject::release();
}

// XTypeProvider
uno::Sequence< uno::Type > SAL_CALL SvxAccessibleTextPropertySet::getTypes() throw ( uno::RuntimeException )
{
    static ::cppu::OTypeCollection* pTypeCollection = NULL ;

    // double-checked locking pattern.
    if ( pTypeCollection == NULL )
    {
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

        // Control these pointer again ... it can be, that another instance will be faster then these!
        if ( pTypeCollection == NULL )
        {
            // Create a static typecollection ...
            static ::cppu::OTypeCollection aTypeCollection(
                ::getCppuType( static_cast< const uno::Reference< beans::XPropertySet >* > (0) ),
                ::getCppuType( static_cast< const uno::Reference< beans::XMultiPropertySet >* > (0) ),
                ::getCppuType( static_cast< const uno::Reference< beans::XPropertyState >* > (0) ),
                ::getCppuType( static_cast< const uno::Reference< lang::XServiceInfo >* > (0) ),
                ::getCppuType( static_cast< const uno::Reference< lang::XTypeProvider >* > (0) ) );

            // ... and set his address to static pointer!
            pTypeCollection = &aTypeCollection ;
        }
    }

    return pTypeCollection->getTypes() ;
}

uno::Sequence< sal_Int8 > SAL_CALL SvxAccessibleTextPropertySet::getImplementationId()
    throw (uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( reinterpret_cast< sal_uInt8* > (aId.getArray()), 0, sal_True );
    }
    return aId;
}

// XServiceInfo
::rtl::OUString SAL_CALL SAL_CALL SvxAccessibleTextPropertySet::getImplementationName (void) throw (uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("SvxAccessibleTextPropertySet"));
}

sal_Bool SAL_CALL SvxAccessibleTextPropertySet::supportsService (const ::rtl::OUString& sServiceName) throw (uno::RuntimeException)
{
    //  Iterate over all supported service names and return true if on of them
    //  matches the given name.
    uno::Sequence< ::rtl::OUString> aSupportedServices (
        getSupportedServiceNames ());
    for (int i=0; i<aSupportedServices.getLength(); i++)
        if (sServiceName == aSupportedServices[i])
            return sal_True;
    return sal_False;
}

uno::Sequence< ::rtl::OUString> SAL_CALL SvxAccessibleTextPropertySet::getSupportedServiceNames (void) throw (uno::RuntimeException)
{
    // TODO
    return SvxUnoTextRangeBase::getSupportedServiceNames();
}

// XServiceName
::rtl::OUString SAL_CALL SvxAccessibleTextPropertySet::getServiceName() throw (uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.beans.PropertyValue"));
}



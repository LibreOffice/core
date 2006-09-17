/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xserviceinfo.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 13:40:33 $
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
#include "precompiled_chart2.hxx"
// Sample implementation of the service XServiceInfo
// -------------------------------------------------
//
// CLASSNAME:  Replace this with your classes name
//
// m_aMutex:   This is the mutex member of your class
//

// --------------------

#include <vector>
#include <algorithm>

// --------------------

using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;
using ::osl::MutexGuard;
using ::com::sun::star::uno::RuntimeException;

// --------------------

// this should be a private method of your class
const ::std::vector< OUString > & CLASSNAME::GetServiceNames()
{
    static ::std::vector< OUString > aServices;

    // /--
    MutexGuard aGuard( m_aMutex );

    if( aServices.size() == 0 )
    {
        aServices.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sample.Service1" )));
        aServices.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sample.Service2" )));
        aServices.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sample.Service3" )));
        // ...
    }

    return aServices;
    // \--
}

// --------------------

// Alternative using double-checked-locking pattern
// (untested!)

// #ifndef INCLUDED_OSL_DOUBLECHECKEDLOCKING_H
// #include <osl/doublecheckedlocking.h>
// #endif

// this should be a private method of your class
// const ::std::vector< OUString > & CLASSNAME::GetServiceNames()
// {
//     static ::std::vector< OUString > aServices;
//     static bool bIsInitialized = false;

//     if( ! bIsInitialized )
//     {
//         // /--
//         MutexGuard aGuard( m_aMutex );

//         if( ! bIsInitialized )
//         {
//             aServices.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sample.Service1" )));
//             aServices.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sample.Service2" )));
//             aServices.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sample.Service3" )));
//             // ...
//             OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
//             bIsInitialized = true;
//         }
//         // \--
//     }
//     else
//         OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();

//     return aServices;
// }

// ------------------------------------
// ::com::sun::star::lang::XServiceInfo
// ------------------------------------

OUString SAL_CALL CLASSNAME::getImplementationName()
    throw (RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.sample.CLASSNAME" ));
}

sal_Bool SAL_CALL CLASSNAME::supportsService( const ::rtl::OUString& ServiceName )
    throw (RuntimeException)
{
    const ::std::vector< OUString > & rServices = GetServiceNames();
    return ( rServices.end() != ::std::find( rServices.begin(), rServices.end(), ServiceName ) );
}

Sequence< OUString > SAL_CALL CLASSNAME::getSupportedServiceNames()
    throw (RuntimeException)
{
    const ::std::vector< OUString > & rServices = GetServiceNames();
    return Sequence< OUString >( &(* rServices.begin()), rServices.size() );
}

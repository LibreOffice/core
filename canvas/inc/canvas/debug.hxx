/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: debug.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:40:17 $
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

#if ! defined(INCLUDED_CANVAS_DEBUG_HXX)
#define INCLUDED_CANVAS_DEBUG_HXX

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

#include <boost/current_function.hpp>

// Class invariants
// ----------------
#if OSL_DEBUG_LEVEL > 0

/** This macro asserts the given condition, and throws an
    IllegalArgumentException afterwards.

    In production code, no assertion appears, but the
    IllegalArgumentException is thrown nevertheless (although without
    the given error string, to conserve space).
 */
#define CHECK_AND_THROW(c, m)   if( !(c) ) { \
                                     OSL_ENSURE(false, m); \
                                     throw ::com::sun::star::lang::IllegalArgumentException( \
                                     ::rtl::OUString::createFromAscii(BOOST_CURRENT_FUNCTION) + \
                                     ::rtl::OUString::createFromAscii(",\n"m), \
                                     ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >(), \
                                     0 ); }

/** This macro asserts the given condition, and throws an
    RuntimeException afterwards.

    In production code, no assertion appears, but the
    RuntimeException is thrown nevertheless (although without
    the given error string, to conserve space).
 */
#define ENSURE_AND_THROW(c, m)   if( !(c) ) { \
                                     OSL_ENSURE(false, m); \
                                     throw ::com::sun::star::uno::RuntimeException( \
                                     ::rtl::OUString::createFromAscii(BOOST_CURRENT_FUNCTION) + \
                                     ::rtl::OUString::createFromAscii(",\n"m), \
                                     ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() ); }

/** This macro asserts the given condition, and returns false
    afterwards.

    In production code, no assertion appears, but the return is issued
    nevertheless.
 */
#define ENSURE_AND_RETURN(c, m)  if( !(c) ) { \
                                     OSL_ENSURE(false, m); \
                                     return false; }

#else // ! (OSL_DEBUG_LEVEL > 0)

#define CHECK_AND_THROW(c, m)   if( !(c) )                                                  \
                                     throw ::com::sun::star::lang::IllegalArgumentException( \
                                     ::rtl::OUString::createFromAscii(BOOST_CURRENT_FUNCTION) + \
                                     ::rtl::OUString::createFromAscii(",\n"m), \
                                     ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >(), \
                                     0 )

#define ENSURE_AND_THROW(c, m)   if( !(c) )                                                     \
                                     throw ::com::sun::star::uno::RuntimeException( \
                                     ::rtl::OUString::createFromAscii(BOOST_CURRENT_FUNCTION) + \
                                     ::rtl::OUString::createFromAscii(",\n"m), \
                                     ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() )

#define ENSURE_AND_RETURN(c, m)  if( !(c) )                                                     \
                                     return false

#endif


// shared_ptr debugging
// --------------------

#ifdef BOOST_SP_ENABLE_DEBUG_HOOKS

#include <boost/shared_ptr.hpp>

::std::size_t find_unreachable_objects( bool );

#ifdef VERBOSE
#define SHARED_PTR_LEFTOVERS(a) OSL_TRACE( "%s\n%s: Unreachable objects still use %d bytes\n", \
                                           BOOST_CURRENT_FUNCTION, a, \
                                           find_unreachable_objects(true) )
#else
/** This macro shows how much memory is still used by shared_ptrs

    Use this macro at places in the code where normally all shared_ptr
    objects should have been deleted. You'll get the number of bytes
    still contained in those objects, which quite possibly are prevented
    from deletion by circular references.
 */
#define SHARED_PTR_LEFTOVERS(a) OSL_TRACE( "%s\n%s: Unreachable objects still use %d bytes\n", \
                                           BOOST_CURRENT_FUNCTION, a, \
                                           find_unreachable_objects(false) )
#endif

#else

#define SHARED_PTR_LEFTOVERS(a) ((void)0)

#endif

#endif // ! defined(INCLUDED_CANVAS_DEBUG_HXX)

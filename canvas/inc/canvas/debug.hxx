/*************************************************************************
 *
 *  $RCSfile: debug.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-13 17:58:25 $
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

#ifndef _SLIDESHOW_DEBUG_HXX
#define _SLIDESHOW_DEBUG_HXX

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

// Class invariants
// ----------------
#if OSL_DEBUG_LEVEL > 0

/// @internal
inline void debug_ensure_and_throw( bool bCondition, const char* pMsg )
{
    OSL_ENSURE(bCondition,pMsg);
    if( !bCondition )
        throw ::com::sun::star::uno::RuntimeException(
            ::rtl::OUString::createFromAscii( pMsg ),
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() );
}

/// @internal
inline void debug_check_and_throw( bool bCondition, const char* pMsg )
{
    OSL_ENSURE(bCondition,pMsg);
    if( !bCondition )
        throw ::com::sun::star::lang::IllegalArgumentException(
            ::rtl::OUString::createFromAscii( pMsg ),
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >(),
            0 );
}

/// @internal
inline bool debug_ensure_and_return( bool bCondition, const char* pMsg )
{
    OSL_ENSURE(bCondition,pMsg);

    return bCondition;
}

/** This macro asserts the given condition, and throws an
    IllegalArgumentException afterwards.

    In production code, no assertion appears, but the
    IllegalArgumentException is thrown nevertheless (although without
    the given error string, to conserve space).
 */
#define CHECK_AND_THROW(c, m)   debug_check_and_throw(c,m)

/** This macro asserts the given condition, and throws an
    RuntimeException afterwards.

    In production code, no assertion appears, but the
    RuntimeException is thrown nevertheless (although without
    the given error string, to conserve space).
 */
#define ENSURE_AND_THROW(c, m)   debug_ensure_and_throw(c,m)

/** This macro asserts the given condition, and returns false
    afterwards.

    In production code, no assertion appears, but the return is issued
    nevertheless.
 */
#define ENSURE_AND_RETURN(c, m)  if( !debug_ensure_and_return(c,m) ) return false

#else

#define CHECK_AND_THROW(c, m)   if( !(c) )                                                  \
                                     throw ::com::sun::star::lang::IllegalArgumentException()

#define ENSURE_AND_THROW(c, m)   if( !(c) )                                                     \
                                     throw ::com::sun::star::uno::RuntimeException()

#define ENSURE_AND_RETURN(c, m)  if( !(c) )                                                     \
                                     return false

#endif


// shared_ptr debugging
// --------------------

#ifdef BOOST_SP_ENABLE_DEBUG_HOOKS

#include <boost/shared_ptr.hpp>

::std::size_t find_unreachable_objects( bool );

#ifdef VERBOSE
#define SHARED_PTR_LEFTOVERS(a) OSL_TRACE( "%s: Unreachable objects still use %d bytes", \
                                           a,                                            \
                                           find_unreachable_objects(true) )
#else
/** This macro shows how much memory is still used by shared_ptrs

    Use this macro at places in the code where normally all shared_ptr
    objects should have been deleted. You'll get the number of bytes
    still contained in those objects, which quite possibly are prevented
    from deletion by circular references.
 */
#define SHARED_PTR_LEFTOVERS(a) OSL_TRACE( "%s: Unreachable objects still use %d bytes", \
                                           a,                                            \
                                           find_unreachable_objects(false) )
#endif

#else

#define SHARED_PTR_LEFTOVERS(a) ((void)0)

#endif

#endif /* _SLIDESHOW_DEBUG_HXX */

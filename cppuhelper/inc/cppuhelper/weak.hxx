/*************************************************************************
 *
 *  $RCSfile: weak.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: dbo $ $Date: 2002-11-18 09:59:14 $
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
#ifndef _CPPUHELPER_WEAK_HXX_
#define _CPPUHELPER_WEAK_HXX_

#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif
#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif
#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_WEAK_HPP_
#include <com/sun/star/uno/XWeak.hpp>
#endif


namespace cppu
{

/** @internal */
class OWeakConnectionPoint;

/** Base class to implement an UNO object supporting weak references, i.e. the object can be held
    weakly (by a ::com::sun::star::uno::WeakReference).
    This implementation copes with reference counting.  Upon last release(), the virtual dtor
    is called.

    @derive
    Inherit from this class and delegate acquire()/ release() calls.
*/
class OWeakObject : public ::com::sun::star::uno::XWeak
{
    /** @internal */
    friend class OWeakConnectionPoint;

protected:
    /** Virtual dtor.

        @attention
        Despite the fact that a RuntimeException is allowed to be thrown, you must not throw any
        exception upon destruction!
    */
    virtual ~OWeakObject() SAL_THROW( (::com::sun::star::uno::RuntimeException) );

    /** reference count.

        @attention
        Don't modify manually!  Use acquire() and release().
    */
    oslInterlockedCount m_refCount;

    /** Container of all weak reference listeners and the connection point from the weak reference.
        @internal
    */
    OWeakConnectionPoint * m_pWeakConnectionPoint;

    /** reserved for future use. do not use.
        @internal
    */
    void * m_pReserved;

public:
    // these are here to force memory de/allocation to sal lib.
    /** @internal */
    inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
        { return ::rtl_allocateMemory( nSize ); }
    /** @internal */
    inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
        { ::rtl_freeMemory( pMem ); }
    /** @internal */
    inline static void * SAL_CALL operator new( size_t, void * pMem ) SAL_THROW( () )
        { return pMem; }
    /** @internal */
    inline static void SAL_CALL operator delete( void *, void * ) SAL_THROW( () )
        {}

#ifdef _MSC_VER
    /** Default Constructor.  Sets the reference count to zero.
        Accidentally occurs in msvc mapfile = > had to be outlined.
    */
    OWeakObject() SAL_THROW( () );
#else
    /** Default Constructor.  Sets the reference count to zero.
    */
    inline OWeakObject() SAL_THROW( () )
        : m_refCount( 0 )
        , m_pWeakConnectionPoint( 0 )
        {}
#endif
    /** Dummy copy constructor.  Set the reference count to zero.

        @param rObj dummy param
    */
    inline OWeakObject( const OWeakObject & rObj ) SAL_THROW( () )
        : m_refCount( 0 )
        , m_pWeakConnectionPoint( 0 )
        {}
    /** Dummy assignment operator. Does not affect reference count.

        @return this OWeakObject
    */
    inline OWeakObject & SAL_CALL operator = ( const OWeakObject & rObj) SAL_THROW( () )
        { return *this; }

    /** Basic queryInterface() implementation supporting ::com::sun::star::uno::XWeak and
        ::com::sun::star::uno::XInterface.

        @param rType demanded type
        @return demanded type or empty any
    */
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        const ::com::sun::star::uno::Type & rType )
        throw (::com::sun::star::uno::RuntimeException);
    /** increasing m_refCount
    */
    virtual void SAL_CALL acquire()
        throw ();
    /** decreasing m_refCount
    */
    virtual void SAL_CALL release()
        throw ();

    /** XWeak::queryAdapter() implementation

        @return a ::com::sun::star::uno::XAdapter reference
    */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAdapter > SAL_CALL queryAdapter()
        throw (::com::sun::star::uno::RuntimeException);

    /** Cast operator to XInterface reference.

        @return XInterface reference
    */
    inline SAL_CALL operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > () SAL_THROW( () )
        { return this; }
};

}

#endif



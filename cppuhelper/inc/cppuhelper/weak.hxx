/*************************************************************************
 *
 *  $RCSfile: weak.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dbo $ $Date: 2001-02-14 10:17:30 $
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

/** */ //for docpp
namespace cppu
{

class OWeakConnectionPoint;

/**
 * The basic implementation to support weak references. The weak
 * implementation is based on interfaces, but you should use this base class to avoid
 * problems against changes in the future.
 * <BR><B>Not fully tested.</B>
 *
 * @author  Markus Meyer
 * @since   98/04/12
 */
class OWeakObject : public ::com::sun::star::uno::XWeak
{
    friend class OWeakConnectionPoint;
protected:
    /**
     * Call the destructor is only allowed if the reference count is zero.
     */
    virtual ~OWeakObject() throw(::com::sun::star::uno::RuntimeException);

    /**
     * The reference counter.
     */
    oslInterlockedCount         m_refCount;

    /**
     * The container of all weak reference listeners and the connection point
     * from the weak reference. Increment the reference count at m_pWeakConnectionPoint
     * object does not affect the
     */
    OWeakConnectionPoint *      m_pWeakConnectionPoint;
public:
    // these are here to force memory de/allocation to sal lib.
    static void * SAL_CALL operator new( size_t nSize ) throw()
        { return ::rtl_allocateMemory( nSize ); }
    static void SAL_CALL operator delete( void * pMem ) throw()
        { ::rtl_freeMemory( pMem ); }

    /**
     * Set the reference count to zero.
     */
    OWeakObject() throw()
        : m_refCount( 0 )
        , m_pWeakConnectionPoint( 0 )
        {}
    /**
     * Set the reference count to zero.
     */
    OWeakObject( const OWeakObject & rObj ) throw()
        : m_refCount( 0 )
        , m_pWeakConnectionPoint( 0 )
        {}
    /**
     * The assignement does not affect the reference count and the weak references
     * of this object.
     */
    inline OWeakObject & SAL_CALL operator = ( const OWeakObject & rObj) throw()
        { return *this; }

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XWeak
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAdapter > SAL_CALL queryAdapter()
        throw (::com::sun::star::uno::RuntimeException);

    /// Avoid ambigous cast error from compiler.
    inline SAL_CALL operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > () throw()
        { return this; }
};

}

#endif



/*************************************************************************
 *
 *  $RCSfile: compbase_ex.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dbo $ $Date: 2001-09-04 09:03:08 $
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
#ifndef _CPPUHELPER_COMPBASE_EX_HXX_
#define _CPPUHELPER_COMPBASE_EX_HXX_

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE_EX_HXX_
#include <cppuhelper/implbase_ex.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif


namespace cppu
{

/** Implementation helper base class for components. Inherits from ::cppu::OWeakObject and
    ::com::sun::star::lang::XComponent.
*/
class SAL_NO_VTABLE WeakComponentImplHelperBase
    : public ::cppu::OWeakObject
    , public ::com::sun::star::lang::XComponent
{
protected:
    /** boradcast helper for disposing events
    */
    ::cppu::OBroadcastHelper rBHelper;

    /** this function is called upon disposing the component
    */
    virtual void SAL_CALL disposing();

    /** This is the one and only constructor that is called from derived implementations.

        @param rMutex mutex to sync upon disposing
    */
    WeakComponentImplHelperBase( ::osl::Mutex & rMutex ) SAL_THROW( () );
public:
    /** Destructor
    */
    virtual ~WeakComponentImplHelperBase() SAL_THROW( () );

    // these are here to force memory de/allocation to sal lib.
    inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
        { return ::rtl_allocateMemory( nSize ); }
    inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
        { ::rtl_freeMemory( pMem ); }
    inline static void * SAL_CALL operator new( size_t, void * pMem ) SAL_THROW( () )
        { return pMem; }
    inline static void SAL_CALL operator delete( void *, void * ) SAL_THROW( () )
        {}

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        ::com::sun::star::uno::Type const & rType )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire()
        throw ();
    virtual void SAL_CALL release()
        throw ();
    virtual void SAL_CALL dispose()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener(
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > const & xListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener(
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > const & xListener )
        throw (::com::sun::star::uno::RuntimeException);
};

/** Implementation helper base class for components. Inherits from ::cppu::OWeakAggObject and
    ::com::sun::star::lang::XComponent.
*/
class SAL_NO_VTABLE WeakAggComponentImplHelperBase
    : public ::cppu::OWeakAggObject
    , public ::com::sun::star::lang::XComponent
{
protected:
    ::cppu::OBroadcastHelper rBHelper;

    /** Is called upon disposing the component.
    */
    virtual void SAL_CALL disposing();

    WeakAggComponentImplHelperBase( ::osl::Mutex & rMutex ) SAL_THROW( () );
public:
    virtual ~WeakAggComponentImplHelperBase() SAL_THROW( () );

    // these are here to force memory de/allocation to sal lib.
    inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
        { return ::rtl_allocateMemory( nSize ); }
    inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
        { ::rtl_freeMemory( pMem ); }
    inline static void * SAL_CALL operator new( size_t, void * pMem ) SAL_THROW( () )
        { return pMem; }
    inline static void SAL_CALL operator delete( void *, void * ) SAL_THROW( () )
        {}

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        ::com::sun::star::uno::Type const & rType )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(
        ::com::sun::star::uno::Type const & rType )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire()
        throw ();
    virtual void SAL_CALL release()
        throw ();
    virtual void SAL_CALL dispose()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener(
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > const & xListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener(
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > const & xListener )
        throw (::com::sun::star::uno::RuntimeException);
};

/** WeakComponentImplHelper
*/
::com::sun::star::uno::Any SAL_CALL WeakComponentImplHelper_query(
    ::com::sun::star::uno::Type const & rType,
    class_data * cd,
    void * that,
    ::cppu::WeakComponentImplHelperBase * pBase )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );
/** WeakComponentImplHelper
*/
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL WeakComponentImplHelper_getTypes(
    class_data * cd )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );

/** WeakAggComponentImplHelper
*/
::com::sun::star::uno::Any SAL_CALL WeakAggComponentImplHelper_queryAgg(
    ::com::sun::star::uno::Type const & rType,
    class_data * cd,
    void * that,
    ::cppu::WeakAggComponentImplHelperBase * pBase )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );
/** WeakAggComponentImplHelper
*/
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL WeakAggComponentImplHelper_getTypes(
    class_data * cd )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );

}

#endif

/*************************************************************************
 *
 *  $RCSfile: compbase1.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dbo $ $Date: 2001-05-21 09:14:50 $
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
#ifndef _CPPUHELPER_COMPBASE1_HXX_
#define _CPPUHELPER_COMPBASE1_HXX_

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE_HXX_
#include <cppuhelper/compbase.hxx>
#endif

/*
__DEF_COMPIMPLHELPER( 1 )
*/

namespace cppu
{
    /** This template class unites ::cppu::ImplHelperBaseN<>, com.sun.star.lang.XComponent
        and ::cppu::OWeakObject, thus delegating life-cycle to that implementation.
        Use this helper implementing an object, that can be held weakly using the
        ::cppu::WeakReference<> template class.
        The template class expects a C++ mutex reference for synchronization in its ctor.
        The com.sun.star.lang.XComponent implementation fires a final disposing() call
        to the implementation when the component is to be disposed.  So implement disposing()
        when sub-classing.

        All other virtual functions (inherited from given template parameter interfaces)
        have to be implemented by sub-classing from the templated class, e.g.
        class MyImpl : public ::cppu::WeakComponentImplHelperN<> { ... };
    */
    template< class Ifc1 >
    class SAL_NO_VTABLE WeakComponentImplHelper1
        : public ::cppu::WeakComponentImplHelperBase
        , public ImplHelperBase1< Ifc1 >
    {
        static ClassData1 s_aCD;
    public:
        WeakComponentImplHelper1( ::osl::Mutex & rMutex ) SAL_THROW( () )
            : WeakComponentImplHelperBase( rMutex )
            {}
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException)
            {
                ::com::sun::star::uno::Any aRet( getClassData( s_aCD ).query( rType, (ImplHelperBase1< Ifc1 > *)this ) );
                if (aRet.hasValue())
                    return aRet;
                return WeakComponentImplHelperBase::queryInterface( rType );
            }
        virtual void SAL_CALL acquire() throw ()
            { WeakComponentImplHelperBase::acquire(); }
        virtual void SAL_CALL release() throw ()
            { WeakComponentImplHelperBase::release(); }
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException)
            { return getClassData( s_aCD ).getTypes(); }
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException)
            { return getClassData( s_aCD ).getImplementationId(); }
    };
    /** This template class unites ::cppu::ImplHelperBaseN<>, com.sun.star.lang.XComponent
        and ::cppu::OWeakAggObject, thus delegating life-cycle to that implementation.
        Use this helper implementing an object, that can be held weakly using the
        ::cppu::WeakReference<> template class and can be aggregated by other objects.
        The template class expects a C++ mutex reference for synchronization in its ctor.
        The com.sun.star.lang.XComponent implementation fires a final disposing() call
        to the implementation when the component is to be disposed.  So implement disposing()
        when sub-classing.

        All other virtual functions (inherited from given template parameter interfaces)
        have to be implemented by sub-classing from the templated class, e.g.
        class MyImpl : public ::cppu::WeakAggComponentImplHelperN<> { ... };
    */
    template< class Ifc1 >
    class SAL_NO_VTABLE WeakAggComponentImplHelper1
        : public ::cppu::WeakAggComponentImplHelperBase
        , public ImplHelperBase1< Ifc1 >
    {
        static ClassData1 s_aCD;
    public:
        WeakAggComponentImplHelper1( ::osl::Mutex & rMutex ) SAL_THROW( () )
            : WeakAggComponentImplHelperBase( rMutex )
            {}
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException)
            { return WeakAggComponentImplHelperBase::queryInterface( rType ); }
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException)
            {
                ::com::sun::star::uno::Any aRet( getClassData( s_aCD ).query( rType, (ImplHelperBase1< Ifc1 > *)this ) );
                if (aRet.hasValue())
                    return aRet;
                return WeakAggComponentImplHelperBase::queryAggregation( rType );
            }
        virtual void SAL_CALL acquire() throw ()
            { WeakAggComponentImplHelperBase::acquire(); }
        virtual void SAL_CALL release() throw ()
            { WeakAggComponentImplHelperBase::release(); }
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException)
            { return getClassData( s_aCD ).getTypes(); }
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException)
            { return getClassData( s_aCD ).getImplementationId(); }
    };

#ifndef MACOSX
    template< class Ifc1 >
    ClassData1 WeakComponentImplHelper1< Ifc1 >::s_aCD = ClassData1( 4 );
    template< class Ifc1 >
    ClassData1 WeakAggComponentImplHelper1< Ifc1 >::s_aCD = ClassData1( 3 );
#endif

}

#endif

/*************************************************************************
 *
 *  $RCSfile: compbase.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2003-07-16 17:47:47 $
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
#ifndef _CPPUHELPER_COMPBASE_HXX_
#define _CPPUHELPER_COMPBASE_HXX_

#ifndef _CPPUHELPER_COMPBASE_EX_HXX_
#include <cppuhelper/compbase_ex.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE_HXX_
#include <cppuhelper/implbase.hxx>
#endif

/* This header should not be used anymore.  compbase1-N.hxx use implbase_ex.hxx except
   for MACOSX.
   @deprecated
*/

//==================================================================================================
/** @internal */
#define __DEF_COMPIMPLHELPER_A( N ) \
namespace cppu \
{ \
template< __CLASS_IFC##N > \
class SAL_NO_VTABLE WeakComponentImplHelper##N \
    : public ::cppu::WeakComponentImplHelperBase \
    , public ImplHelperBase##N< __IFC##N > \
{ \
    static ClassData##N s_aCD; \
public: \
    WeakComponentImplHelper##N( ::osl::Mutex & rMutex ) SAL_THROW( () ) \
        : WeakComponentImplHelperBase( rMutex ) \
        {} \
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException) \
    { \
        ::com::sun::star::uno::Any aRet( getClassData( s_aCD ).query( rType, (ImplHelperBase##N< __IFC##N > *)this ) ); \
        if (aRet.hasValue()) \
            return aRet; \
        return WeakComponentImplHelperBase::queryInterface( rType ); \
    } \
    virtual void SAL_CALL acquire() throw () \
        { WeakComponentImplHelperBase::acquire(); } \
    virtual void SAL_CALL release() throw () \
        { WeakComponentImplHelperBase::release(); } \
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException) \
        { return getClassData( s_aCD ).getTypes(); } \
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException) \
        { return getClassData( s_aCD ).getImplementationId(); } \
}; \
template< __CLASS_IFC##N > \
class SAL_NO_VTABLE WeakAggComponentImplHelper##N \
    : public ::cppu::WeakAggComponentImplHelperBase \
    , public ImplHelperBase##N< __IFC##N > \
{ \
    static ClassData##N s_aCD; \
public: \
    WeakAggComponentImplHelper##N( ::osl::Mutex & rMutex ) SAL_THROW( () ) \
        : WeakAggComponentImplHelperBase( rMutex ) \
        {} \
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException) \
        { return WeakAggComponentImplHelperBase::queryInterface( rType ); } \
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException) \
    { \
        ::com::sun::star::uno::Any aRet( getClassData( s_aCD ).query( rType, (ImplHelperBase##N< __IFC##N > *)this ) ); \
        if (aRet.hasValue()) \
            return aRet; \
        return WeakAggComponentImplHelperBase::queryAggregation( rType ); \
    } \
    virtual void SAL_CALL acquire() throw () \
        { WeakAggComponentImplHelperBase::acquire(); } \
    virtual void SAL_CALL release() throw () \
        { WeakAggComponentImplHelperBase::release(); } \
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException) \
        { return getClassData( s_aCD ).getTypes(); } \
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException) \
        { return getClassData( s_aCD ).getImplementationId(); } \
};

//==================================================================================================
/** @internal */
#define __DEF_COMPIMPLHELPER_B( N ) \
template< __CLASS_IFC##N > \
ClassData##N WeakComponentImplHelper##N< __IFC##N >::s_aCD = ClassData##N( 4 ); \
template< __CLASS_IFC##N > \
ClassData##N WeakAggComponentImplHelper##N< __IFC##N >::s_aCD = ClassData##N( 3 );
//==================================================================================================
/** @internal */
#define __DEF_COMPIMPLHELPER_C( N ) \
}
//==================================================================================================
// The Mac OS X gcc compiler cannot handle assignments to static data members
// of the generic template class. It can only handle assignments to specific
// instantiations of a template class.
#if defined(MACOSX) && ( __GNUC__ < 3 )
/** @internal */
#define __DEF_COMPIMPLHELPER( N ) \
__DEF_COMPIMPLHELPER_A( N ) \
__DEF_COMPIMPLHELPER_C( N )
#else
/** @internal */
#define __DEF_COMPIMPLHELPER( N ) \
__DEF_COMPIMPLHELPER_A( N ) \
__DEF_COMPIMPLHELPER_B( N ) \
__DEF_COMPIMPLHELPER_C( N )
#endif

#endif

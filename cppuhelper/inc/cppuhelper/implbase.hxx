/*************************************************************************
 *
 *  $RCSfile: implbase.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dbo $ $Date: 2000-10-06 15:15:09 $
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
#ifndef _CPPUHELPER_IMPLBASE_HXX_
#define _CPPUHELPER_IMPLBASE_HXX_

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_WEAKAGG_HXX_
#include <cppuhelper/weakagg.hxx>
#endif

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>


namespace cppu
{

struct Type_Offset
{
    sal_Int32 nOffset;
    typelib_InterfaceTypeDescription * pTD;
};
struct ClassDataBase
{
    sal_Bool  bOffsetsInit;
    sal_Int32 nType2Offset;

    sal_Int32 nClassCode;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > * pTypes;
    ::com::sun::star::uno::Sequence< sal_Int8 > * pId;

    ClassDataBase();
    ClassDataBase( sal_Int32 nClassCode );
    ~ClassDataBase();
};
struct ClassData : public ClassDataBase
{
    Type_Offset arType2Offset[1];

    void SAL_CALL initTypeProvider();
    void SAL_CALL writeTypeOffset( const ::com::sun::star::uno::Type & rType, sal_Int32 nOffset );

    ::com::sun::star::uno::Any SAL_CALL query(
        const ::com::sun::star::uno::Type & rType, ::com::sun::star::lang::XTypeProvider * pBase );
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes();
    ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId();
};

//==================================================================================================
::osl::Mutex & SAL_CALL getImplHelperInitMutex(void);
}

//
// settle down beavis, here comes the macro template hell :]
//

//==================================================================================================
#define __DEF_IMPLHELPER_PRE( N ) \
namespace cppu \
{ \
struct ClassData##N : public ClassDataBase \
{ \
    Type_Offset arType2Offset[ N ]; \
    ClassData##N( sal_Int32 nClassCode ) \
        : ClassDataBase( nClassCode ) \
        {} \
}; \
template< __CLASS_IFC##N > \
class ImplHelperBase##N \
    : public ::com::sun::star::lang::XTypeProvider, __PUBLIC_IFC##N \
{ \
protected: \
    ClassData & SAL_CALL getClassData( ClassDataBase & s_aCD ) \
    { \
        ClassData & rCD = * static_cast< ClassData * >( &s_aCD ); \
        if (! rCD.bOffsetsInit) \
        { \
            ::osl::MutexGuard aGuard( getImplHelperInitMutex() ); \
            if (! rCD.bOffsetsInit) \
            { \
                char * pBase = (char *)this;
//==================================================================================================
#define __IFC_WRITEOFFSET( N ) \
                rCD.writeTypeOffset( ::getCppuType( (const ::com::sun::star::uno::Reference< Ifc##N > *)0 ), \
                                     (char *)(Ifc##N *)this - pBase );
//==================================================================================================
#define __DEF_IMPLHELPER_POST_A( N ) \
                rCD.bOffsetsInit = sal_True; \
            } \
        } \
        return rCD; \
    } \
}; \
template< __CLASS_IFC##N > \
class ImplHelper##N \
    : public ImplHelperBase##N< __IFC##N > \
{ \
    static ClassData##N s_aCD; \
public: \
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException) \
        { return getClassData( s_aCD ).query( rType, (ImplHelperBase##N< __IFC##N > *)this ); } \
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException) \
        { return getClassData( s_aCD ).getTypes(); } \
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException) \
        { return getClassData( s_aCD ).getImplementationId(); } \
}; \
template< __CLASS_IFC##N > \
class WeakImplHelper##N \
    : public ::cppu::OWeakObject \
    , public ImplHelperBase##N< __IFC##N > \
{ \
    static ClassData##N s_aCD; \
public: \
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) \
    { \
        ::com::sun::star::uno::Any aRet( getClassData( s_aCD ).query( rType, (ImplHelperBase##N< __IFC##N > *)this ) ); \
        return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType )); \
    } \
    virtual void SAL_CALL acquire() throw() \
        { OWeakObject::acquire(); } \
    virtual void SAL_CALL release() throw() \
        { OWeakObject::release(); } \
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException) \
        { return getClassData( s_aCD ).getTypes(); } \
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException) \
        { return getClassData( s_aCD ).getImplementationId(); } \
}; \
template< __CLASS_IFC##N > \
class WeakAggImplHelper##N \
    : public ::cppu::OWeakAggObject \
    , public ImplHelperBase##N< __IFC##N > \
{ \
    static ClassData##N s_aCD; \
public: \
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) \
        { return OWeakAggObject::queryInterface( rType ); } \
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) \
    { \
        ::com::sun::star::uno::Any aRet( getClassData( s_aCD ).query( rType, (ImplHelperBase##N< __IFC##N > *)this ) ); \
        return (aRet.hasValue() ? aRet : OWeakAggObject::queryAggregation( rType )); \
    } \
    virtual void SAL_CALL acquire() throw() \
        { OWeakAggObject::acquire(); } \
    virtual void SAL_CALL release() throw() \
        { OWeakAggObject::release(); } \
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException) \
        { return getClassData( s_aCD ).getTypes(); } \
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException) \
        { return getClassData( s_aCD ).getImplementationId(); } \
};

//==================================================================================================
#define __DEF_IMPLHELPER_POST_B( N ) \
template< __CLASS_IFC##N > \
ClassData##N ImplHelper##N< __IFC##N >::s_aCD = ClassData##N( 0 ); \
template< __CLASS_IFC##N > \
ClassData##N WeakImplHelper##N< __IFC##N >::s_aCD = ClassData##N( 1 ); \
template< __CLASS_IFC##N > \
ClassData##N WeakAggImplHelper##N< __IFC##N >::s_aCD = ClassData##N( 2 );
//==================================================================================================
#define __DEF_IMPLHELPER_POST_C( N ) \
}
//==================================================================================================
// The Mac OS X gcc compiler cannot handle assignments to static data members
// of the generic template class. It can only handle assignments to specific
// instantiations of a template class.
#ifdef MACOSX
#define __DEF_IMPLHELPER_POST( N ) \
__DEF_IMPLHELPER_POST_A( N ) \
__DEF_IMPLHELPER_POST_C( N )
#else   /* MACOSX */
#define __DEF_IMPLHELPER_POST( N ) \
__DEF_IMPLHELPER_POST_A( N ) \
__DEF_IMPLHELPER_POST_B( N ) \
__DEF_IMPLHELPER_POST_C( N )
#endif  /* MACOSX */

#endif

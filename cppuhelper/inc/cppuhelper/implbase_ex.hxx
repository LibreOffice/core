/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: implbase_ex.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2005-10-05 14:27:57 $
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
#ifndef _CPPUHELPER_IMPLBASE_EX_HXX_
#define _CPPUHELPER_IMPLBASE_EX_HXX_

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_WEAKAGG_HXX_
#include <cppuhelper/weakagg.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

// Despite the fact that the following include is not used in this header, it has to remain,
// because it is expected by files including cppuhelper/implbaseN.hxx.
// So maybe we can omit it some time in the future...
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

/* If you need to define implementation helper classes that deal with more than
   12 interfaces, then use macros as follows, e.g. for 3 interfaces:

#include <cppuhelper/implbase_ex_pre.hxx>
#define __IFC_EX_TYPE_INIT3( class_cast ) \
    __IFC_EX_TYPE_INIT( class_cast, 1 ), __IFC_EX_TYPE_INIT( class_cast, 2 ), \
    __IFC_EX_TYPE_INIT( class_cast, 3 )
#include <cppuhelper/implbase_ex_post.hxx>
__DEF_IMPLHELPER_EX( 3 )
*/


namespace cppu
{

/** function pointer signature for getCppuType
    @internal
*/
typedef ::com::sun::star::uno::Type const & (SAL_CALL * fptr_getCppuType)( void * ) SAL_THROW( () );

/** single type + object offset
    @internal
*/
struct type_entry
{
    /** the type_entry is initialized with function pointer to ::getCppuType() function first,
        but holds an unacquired typelib_TypeDescriptionReference * after initialization,
        thus reusing the memory.  Flag class_data::m_storedTypeRefs
    */
    union
    {
        fptr_getCppuType getCppuType;
        typelib_TypeDescriptionReference * typeRef;
    } m_type;
    /** offset for interface pointer
    */
    sal_IntPtr m_offset;
};

/** identical dummy struct for casting class_dataN to class_data
    @internal
*/
struct class_data
{
    /** number of supported types in m_typeEntries
    */
    sal_Int16 m_nTypes;

    /** determines whether m_typeEntries is initialized and carries unacquired type refs
    */
    sal_Bool m_storedTypeRefs;

    /** determines whether an implementation id was created in m_id
    */
    sal_Bool m_createdId;

    /** implementation id
     */
    sal_Int8 m_id[ 16 ];

    /** type, object offset
    */
    type_entry m_typeEntries[ 1 ];
};

/** ImplHelper
    @internal
*/
::com::sun::star::uno::Any SAL_CALL ImplHelper_query(
    ::com::sun::star::uno::Type const & rType,
    class_data * cd,
    void * that )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );
/** ImplHelper
    @internal
*/
::com::sun::star::uno::Any SAL_CALL ImplHelper_queryNoXInterface(
    ::com::sun::star::uno::Type const & rType,
    class_data * cd,
    void * that )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );
/** ImplHelper
    @internal
*/
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL ImplHelper_getTypes(
    class_data * cd )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );
/** ImplHelper
    @internal
*/
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL ImplInhHelper_getTypes(
    class_data * cd,
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > const & rAddTypes )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );
/** ImplHelper
    @internal
*/
::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL ImplHelper_getImplementationId(
    class_data * cd )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );

/** WeakImplHelper
    @internal
*/
::com::sun::star::uno::Any SAL_CALL WeakImplHelper_query(
    ::com::sun::star::uno::Type const & rType,
    class_data * cd,
    void * that,
    ::cppu::OWeakObject * pBase )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );
/** WeakImplHelper
    @internal
*/
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL WeakImplHelper_getTypes(
    class_data * cd )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );

/** WeakAggImplHelper
    @internal
*/
::com::sun::star::uno::Any SAL_CALL WeakAggImplHelper_queryAgg(
    ::com::sun::star::uno::Type const & rType,
    class_data * cd,
    void * that,
    ::cppu::OWeakAggObject * pBase )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );
/** WeakAggImplHelper
    @internal
*/
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL WeakAggImplHelper_getTypes(
    class_data * cd )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );

}

#endif

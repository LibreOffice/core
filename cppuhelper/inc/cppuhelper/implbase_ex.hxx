/*************************************************************************
 *
 *  $RCSfile: implbase_ex.hxx,v $
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


namespace cppu
{

/** function pointer signature for getCppuType
*/
typedef ::com::sun::star::uno::Type const & (SAL_CALL * fptr_getCppuType)( void * ) SAL_THROW( () );

/** single type + object offset
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
    sal_Int32 m_offset;
};

/** identical dummy struct for casting class_dataN to class_data
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
*/
::com::sun::star::uno::Any SAL_CALL ImplHelper_query(
    ::com::sun::star::uno::Type const & rType,
    class_data * cd,
    void * that )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );
/** ImplHelper
*/
::com::sun::star::uno::Any SAL_CALL ImplHelper_queryNoXInterface(
    ::com::sun::star::uno::Type const & rType,
    class_data * cd,
    void * that )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );
/** ImplHelper
*/
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL ImplHelper_getTypes(
    class_data * cd )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );
/** ImplHelper
*/
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL ImplInhHelper_getTypes(
    class_data * cd,
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > const & rAddTypes )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );
/** ImplHelper
*/
::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL ImplHelper_getImplementationId(
    class_data * cd )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );

/** WeakImplHelper
*/
::com::sun::star::uno::Any SAL_CALL WeakImplHelper_query(
    ::com::sun::star::uno::Type const & rType,
    class_data * cd,
    void * that,
    ::cppu::OWeakObject * pBase )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );
/** WeakImplHelper
*/
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL WeakImplHelper_getTypes(
    class_data * cd )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );

/** WeakAggImplHelper
*/
::com::sun::star::uno::Any SAL_CALL WeakAggImplHelper_queryAgg(
    ::com::sun::star::uno::Type const & rType,
    class_data * cd,
    void * that,
    ::cppu::OWeakAggObject * pBase )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );
/** WeakAggImplHelper
*/
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL WeakAggImplHelper_getTypes(
    class_data * cd )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );

}

#endif

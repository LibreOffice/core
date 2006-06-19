/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChainablePropertySetInfo.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 22:40:53 $
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

#ifndef _COMPHELPER_CHAINABLEPROPERTYSETINFO_HXX_
#define _COMPHELPER_CHAINABLEPROPERTYSETINFO_HXX_

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COMPHELPER_PROPERTYINFOHASH_HXX_
#include <comphelper/PropertyInfoHash.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COMPHELPER_TYPEGENERATION_HXX_
#include <comphelper/TypeGeneration.hxx>
#endif
#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include "comphelper/comphelperdllapi.h"
#endif

/*
 * A ChainablePropertySetInfo is usually initialised with a pointer to the first element
 * of a null-terminated static table of PropertyInfo structs. This is placed in a hash_map
 * for fast access
 *
 */
namespace comphelper
{
    class COMPHELPER_DLLPUBLIC ChainablePropertySetInfo:
        public ::cppu::WeakImplHelper1<
        ::com::sun::star::beans::XPropertySetInfo >
    {
        friend class ChainablePropertySet;
        friend class MasterPropertySet;
    protected:
        PropertyInfoHash maMap;
        com::sun::star::uno::Sequence < com::sun::star::beans::Property > maProperties;
    public:
        ChainablePropertySetInfo()
            throw();
        ChainablePropertySetInfo( PropertyInfo * pMap )
            throw();

        virtual ~ChainablePropertySetInfo()
            throw();

        void add( PropertyInfo* pMap, sal_Int32 nCount = -1 )
            throw();
        void remove( const rtl::OUString& aName )
            throw();

        // XPropertySetInfo
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getProperties()
            throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::beans::Property SAL_CALL getPropertyByName( const ::rtl::OUString& aName )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL hasPropertyByName( const ::rtl::OUString& Name )
            throw(::com::sun::star::uno::RuntimeException);
    };
}
#endif

/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unopropertyarrayhelper.hxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _TOOLKIT_HELPER_UNOPROPERTYARRAYHELPER_HXX_
#define _TOOLKIT_HELPER_UNOPROPERTYARRAYHELPER_HXX_

#include <cppuhelper/propshlp.hxx>

#include <tools/table.hxx>

#include <list>

//  ----------------------------------------------------
//  class UnoPropertyArrayHelper
//  ----------------------------------------------------
class UnoPropertyArrayHelper : public ::cppu::IPropertyArrayHelper
{
private:
    Table       maIDs;

protected:
    sal_Bool    ImplHasProperty( sal_uInt16 nPropId ) const;

public:
                UnoPropertyArrayHelper( const ::com::sun::star::uno::Sequence<sal_Int32>& rIDs );
                UnoPropertyArrayHelper( const std::list< sal_uInt16 > &rIDs );

    // ::cppu::IPropertyArrayHelper
    sal_Bool SAL_CALL fillPropertyMembersByHandle( ::rtl::OUString * pPropName, sal_Int16 * pAttributes, sal_Int32 nHandle );
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getProperties();
    ::com::sun::star::beans::Property SAL_CALL getPropertyByName(const ::rtl::OUString& rPropertyName) throw (::com::sun::star::beans::UnknownPropertyException);
    sal_Bool SAL_CALL hasPropertyByName(const ::rtl::OUString& rPropertyName);
    sal_Int32 SAL_CALL getHandleByName( const ::rtl::OUString & rPropertyName );
    sal_Int32 SAL_CALL fillHandles( sal_Int32* pHandles, const ::com::sun::star::uno::Sequence< ::rtl::OUString > & rPropNames );
};



#endif // _TOOLKIT_HELPER_UNOPROPERTYARRAYHELPER_HXX_


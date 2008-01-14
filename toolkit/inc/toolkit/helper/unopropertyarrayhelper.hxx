/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unopropertyarrayhelper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 12:56:18 $
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


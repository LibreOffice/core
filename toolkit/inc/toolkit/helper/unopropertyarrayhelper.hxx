/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _TOOLKIT_HELPER_UNOPROPERTYARRAYHELPER_HXX_
#define _TOOLKIT_HELPER_UNOPROPERTYARRAYHELPER_HXX_

#include <toolkit/dllapi.h>
#include <cppuhelper/propshlp.hxx>

#include <tools/table.hxx>

#include <list>
#include "toolkit/dllapi.h"

//  ----------------------------------------------------
//  class UnoPropertyArrayHelper
//  ----------------------------------------------------
class TOOLKIT_DLLPUBLIC UnoPropertyArrayHelper : public ::cppu::IPropertyArrayHelper
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


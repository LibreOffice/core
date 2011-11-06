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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_toolkit.hxx"

#include <toolkit/controls/unocontrolcontainermodel.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <toolkit/helper/unopropertyarrayhelper.hxx>

//  ----------------------------------------------------
//  class UnoControlContainerModel
//  ----------------------------------------------------
UnoControlContainerModel::UnoControlContainerModel( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory )
    :UnoControlModel( i_factory )
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_BORDERCOLOR );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_TEXT );
}

::rtl::OUString UnoControlContainerModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlContainerModel );
}

::com::sun::star::uno::Any UnoControlContainerModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    ::com::sun::star::uno::Any aDefault;
    if ( nPropId == BASEPROPERTY_BORDER )
        aDefault <<= (sal_Int16) 0;
    else
        aDefault <<= UnoControlModel::ImplGetDefaultValue( nPropId );
    return aDefault;
}


::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > UnoControlContainerModel::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    static ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

::cppu::IPropertyArrayHelper& UnoControlContainerModel::getInfoHelper()
{
    ::osl::Guard< ::osl::Mutex > aGuard( ((UnoControlContainerModel*)this)->GetMutex() );

    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        ::com::sun::star::uno::Sequence<sal_Int32>  aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}





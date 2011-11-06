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



#ifndef _TOOLKIT_CONTROLS_UNOCONTROLCONTAINERMODEL_HXX_
#define _TOOLKIT_CONTROLS_UNOCONTROLCONTAINERMODEL_HXX_


#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/servicenames.hxx>


//  ----------------------------------------------------
//  class ::com::sun::star::awt::UnoControlContainerModel
//  ----------------------------------------------------
class UnoControlContainerModel : public UnoControlModel
{
protected:
    ::com::sun::star::uno::Any                                                      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >   getPropertySetInfo() const;
    ::cppu::IPropertyArrayHelper&                                                   SAL_CALL getInfoHelper();

public:
                        UnoControlContainerModel( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );
                        UnoControlContainerModel( const UnoControlContainerModel& rModel ) : UnoControlModel( rModel ) {;}

    UnoControlModel*    Clone() const { return new UnoControlContainerModel( *this ); }

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlContainerModel, UnoControlModel, szServiceName2_UnoControlContainerModel )
};



#endif // _TOOLKIT_CONTROLS_UNOCONTROLCONTAINERMODEL_HXX_


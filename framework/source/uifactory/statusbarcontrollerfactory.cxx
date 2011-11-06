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
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include "uifactory/statusbarcontrollerfactory.hxx"
#include "uifactory/factoryconfiguration.hxx"
#include <threadhelp/resetableguard.hxx>
#include "services.h"

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
#include <rtl/ustrbuf.hxx>
#include <cppuhelper/weak.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________
//

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace ::com::sun::star::frame;

//_________________________________________________________________________________________________________________
//  Namespace
//_________________________________________________________________________________________________________________
//

namespace framework
{
//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   StatusbarControllerFactory                      ,
                                            ::cppu::OWeakObject                             ,
                                            SERVICENAME_STATUSBARCONTROLLERFACTORY          ,
                                            IMPLEMENTATIONNAME_STATUSBARCONTROLLERFACTORY
                                        )

DEFINE_INIT_SERVICE                     (   StatusbarControllerFactory, {} )

StatusbarControllerFactory::StatusbarControllerFactory( const Reference< XMultiServiceFactory >& xServiceManager ) :
    ToolbarControllerFactory(xServiceManager,true)
{
    m_pConfigAccess = new ConfigurationAccess_ControllerFactory( m_xServiceManager,rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.UI.Controller/Registered/StatusBar" )),true );
    m_pConfigAccess->acquire();
}


} // namespace framework

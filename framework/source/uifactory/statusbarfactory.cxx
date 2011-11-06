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
#include <uifactory/statusbarfactory.hxx>
#include <uifactory/menubarfactory.hxx>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <uielement/statusbarwrapper.hxx>
#include <threadhelp/resetableguard.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>

#ifndef _COM_SUN_STAR_UI_XUICONFIGURATIONMANAGERSUPLLIER_HPP_
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>
#include <rtl/ustrbuf.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________
//

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace ::com::sun::star::ui;

namespace framework
{

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   StatusBarFactory                                ,
                                            ::cppu::OWeakObject                             ,
                                            SERVICENAME_STATUSBARFACTORY                    ,
                                            IMPLEMENTATIONNAME_STATUSBARFACTORY
                                        )

DEFINE_INIT_SERVICE                     (   StatusBarFactory, {} )

StatusBarFactory::StatusBarFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) :
    MenuBarFactory( xServiceManager,true )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarFactory::StatusBarFactory" );
}

// XUIElementFactory
Reference< XUIElement > SAL_CALL StatusBarFactory::createUIElement(
    const ::rtl::OUString& ResourceURL,
    const Sequence< PropertyValue >& Args )
throw ( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarFactory::createUIElement" );
    // SAFE
    ResetableGuard aLock( m_aLock );
    StatusBarWrapper* pWrapper = new StatusBarWrapper( m_xServiceManager );
    Reference< ::com::sun::star::ui::XUIElement > xMenuBar( (OWeakObject *)pWrapper, UNO_QUERY );
    Reference< ::com::sun::star::frame::XModuleManager > xModuleManager = m_xModuleManager;
    aLock.unlock();
    MenuBarFactory::CreateUIElement(ResourceURL,Args,NULL,"private:resource/statusbar/",xMenuBar,xModuleManager,m_xServiceManager);
    return xMenuBar;
}

}


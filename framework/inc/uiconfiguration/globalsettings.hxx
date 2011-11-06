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



#ifndef __FRAMEWORK_UICONFIGURATION_GLOBALSETTINGS_HXX_
#define __FRAMEWORK_UICONFIGURATION_GLOBALSETTINGS_HXX_

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>
#include <list>
#include <hash_map>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>

namespace framework
{

class GlobalSettings
{
    public:
        GlobalSettings( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rSrvMgr );
        ~GlobalSettings();

        enum UIElementType
        {
            UIELEMENT_TYPE_TOOLBAR,
            UIELEMENT_TYPE_DOCKWINDOW,
            UIELEMENT_TYPE_STATUSBAR
        };

        enum StateInfo
        {
            STATEINFO_LOCKED,
            STATEINFO_DOCKED
        };

        // settings access
        sal_Bool HasStatesInfo( UIElementType eElementType );
        sal_Bool GetStateInfo( UIElementType eElementType, StateInfo eStateInfo, ::com::sun::star::uno::Any& aValue );

    private:
        GlobalSettings();
        GlobalSettings(const GlobalSettings&);
        GlobalSettings& operator=(const GlobalSettings& );

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xSrvMgr;
};

} // namespace framework

#endif // __FRAMEWORK_UIELEMENT_WINDOWSTATECONFIGURATION_HXX_

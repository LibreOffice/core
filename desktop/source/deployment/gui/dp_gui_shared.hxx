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



#if !defined INCLUDED_DP_GUI_SHARED_HXX
#define INCLUDED_DP_GUI_SHARED_HXX

#include "unotools/configmgr.hxx"
#include "rtl/instance.hxx"
#include "tools/resmgr.hxx"


namespace css = ::com::sun::star;

namespace dp_gui {

struct DeploymentGuiResMgr :
    public ::rtl::StaticWithInit< ResMgr *, DeploymentGuiResMgr > {
        ResMgr * operator () () {
            return ResMgr::CreateResMgr( "deploymentgui" );
    }
};

struct BrandName : public ::rtl::StaticWithInit<const ::rtl::OUString, BrandName> {
    const ::rtl::OUString operator () () {
        return ::utl::ConfigManager::GetDirectConfigProperty(
            ::utl::ConfigManager::PRODUCTNAME ).get< ::rtl::OUString >();
    }
};

class DpGuiResId : public ResId
{
public:
    DpGuiResId( sal_uInt16 nId ):ResId( nId, *DeploymentGuiResMgr::get() ) {}
};

} // namespace dp_gui

#endif

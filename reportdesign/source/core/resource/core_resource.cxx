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



#include "core_resource.hxx"
#include <tools/simplerm.hxx>

// ---- needed as long as we have no contexts for components ---
#include <vcl/svapp.hxx>
//---------------------------------------------------
#include <comphelper/configurationhelper.hxx>
#include <osl/thread.h>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <rtl/uri.hxx>

#ifndef _SOLAR_HRC
#include <svl/solar.hrc>
#endif
#include "ModuleHelper.hxx"
//.........................................................................
namespace reportdesign
{
    using namespace ::com::sun::star;
    //==================================================================
    //= ResourceManager
    //==================================================================
    SimpleResMgr* ResourceManager::m_pImpl = NULL;

    //------------------------------------------------------------------
    ResourceManager::EnsureDelete::~EnsureDelete()
    {
        delete ResourceManager::m_pImpl;
    }

    //------------------------------------------------------------------
    void ResourceManager::ensureImplExists(const uno::Reference< lang::XMultiComponentFactory >& /* _rM */)
    {
        if (!m_pImpl)
        {
            // now that we have an impl class make sure it's deleted on unloading the library
            static ResourceManager::EnsureDelete    s_aDeleteTheImplClass;

            ::com::sun::star::lang::Locale aLocale = Application::GetSettings().GetUILocale();

            rtl::OString sResLibName = rtl::OString( "rpt" );
            m_pImpl = SimpleResMgr::Create( sResLibName.getStr(), aLocale);
        }
    }

    //------------------------------------------------------------------
    ::rtl::OUString ResourceManager::loadString(sal_uInt16 _nResId,const uno::Reference< lang::XMultiComponentFactory >& _rM)
    {
        ::rtl::OUString sReturn;

        ensureImplExists(_rM);
        if (m_pImpl)
            sReturn = m_pImpl->ReadString(_nResId);

        return sReturn;
    }

//.........................................................................
}
//.........................................................................


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
#include "precompiled_dbaccess.hxx"

#include "core_resource.hxx"

#include <tools/resmgr.hxx>

// ---- needed as long as we have no contexts for components ---
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
//---------------------------------------------------

#ifndef _SOLAR_HRC
#include <svl/solar.hrc>
#endif

//.........................................................................
namespace dbaccess
{

    //==================================================================
    //= ResourceManager
    //==================================================================
    ::osl::Mutex    ResourceManager::s_aMutex;
    sal_Int32       ResourceManager::s_nClients = 0;
    ResMgr*         ResourceManager::m_pImpl = NULL;

    //------------------------------------------------------------------
    void ResourceManager::ensureImplExists()
    {
        if (m_pImpl)
            return;

        ::com::sun::star::lang::Locale aLocale = Application::GetSettings().GetUILocale();

        ByteString sFileName("dba");

        m_pImpl = ResMgr::CreateResMgr(sFileName.GetBuffer(), aLocale);
    }

    //------------------------------------------------------------------
    ::rtl::OUString ResourceManager::loadString(sal_uInt16 _nResId)
    {
        ::rtl::OUString sReturn;

        ensureImplExists();
        if (m_pImpl)
            sReturn = String(ResId(_nResId,*m_pImpl));

        return sReturn;
    }

    //------------------------------------------------------------------
    ::rtl::OUString ResourceManager::loadString( sal_uInt16 _nResId, const sal_Char* _pPlaceholderAscii, const ::rtl::OUString& _rReplace )
    {
        String sString( loadString( _nResId ) );
        sString.SearchAndReplaceAscii( _pPlaceholderAscii, _rReplace );
        return sString;
    }

    //------------------------------------------------------------------
    ::rtl::OUString ResourceManager::loadString( sal_uInt16 _nResId, const sal_Char* _pPlaceholderAscii1, const ::rtl::OUString& _rReplace1,
        const sal_Char* _pPlaceholderAscii2, const ::rtl::OUString& _rReplace2 )
    {
        String sString( loadString( _nResId ) );
        sString.SearchAndReplaceAscii( _pPlaceholderAscii1, _rReplace1 );
        sString.SearchAndReplaceAscii( _pPlaceholderAscii2, _rReplace2 );
        return sString;
    }

    //-------------------------------------------------------------------------
    void ResourceManager::registerClient()
    {
        ::osl::MutexGuard aGuard(s_aMutex);
        ++s_nClients;
    }

    //-------------------------------------------------------------------------
    void ResourceManager::revokeClient()
    {
        ::osl::MutexGuard aGuard(s_aMutex);
        if (!--s_nClients && m_pImpl)
        {
            delete m_pImpl;
            m_pImpl = NULL;
        }
    }
//.........................................................................
}
//.........................................................................


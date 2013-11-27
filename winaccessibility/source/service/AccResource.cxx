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

#include "AccResource.hxx"

#include <tools/simplerm.hxx>

// ---- needed as long as we have no contexts for components ---
#include <vcl/svapp.hxx>

//2009.04. Comment the following code because the acc resource is not necessary now. Can open them if necessary
//#ifndef _TOOLS_INTN_HXX
//#include <tools/intn.hxx>
//#endif
//#ifndef _SOLAR_HRC
//#include <svtools/solar.hrc>
//#endif

//.........................................................................

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
void ResourceManager::ensureImplExists()
{
    //Comment the following code because the acc resource is not necessary now. Can open them if necessary
    /*
    if (m_pImpl)
        return;

    LanguageType nType = Application::GetSettings().GetUILanguage();

    ByteString sFileName("winaccessibility");
    sFileName += ByteString::CreateFromInt32( (sal_Int32)SOLARUPD );

    m_pImpl = SimpleResMgr::Create(sFileName.GetBuffer(), nType);

    if (m_pImpl)
    {
        // no that we have a impl class make sure it's deleted on unloading the library
        static ResourceManager::EnsureDelete    s_aDeleteTheImplClas;
    }
  */
}

//------------------------------------------------------------------
::rtl::OUString ResourceManager::loadString(sal_uInt16 _nResId)
{
    ::rtl::OUString sReturn;

    ensureImplExists();
    if (m_pImpl)
        sReturn = m_pImpl->ReadString(_nResId);

    return sReturn;
}



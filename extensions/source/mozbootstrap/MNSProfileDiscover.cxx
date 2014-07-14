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
#include "precompiled_extensions.hxx"
#include "MNSProfileDiscover.hxx"

// Registry Keys

static ::rtl::OUString szProfileSubtreeString=::rtl::OUString::createFromAscii("Profiles");
static ::rtl::OUString szCurrentProfileString= ::rtl::OUString::createFromAscii("CurrentProfile");
static ::rtl::OUString szDirectoryString =::rtl::OUString::createFromAscii("directory");

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif
#include <MNSFolders.hxx>
#include <MNSINIParser.hxx>

namespace connectivity
{
    namespace mozab
    {
        ProfileStruct::ProfileStruct(MozillaProductType aProduct,::rtl::OUString aProfileName,
            const ::rtl::OUString& aProfilePath)
        {
            product=aProduct;
            profileName = aProfileName;
            profilePath = aProfilePath;
        }
        ::rtl::OUString ProfileStruct::getProfilePath()
        {
            return profilePath;
        }

        ProfileAccess::~ProfileAccess()
        {
        }
        ProfileAccess::ProfileAccess()
        {
            LoadProductsInfo();
        }

        sal_Int32 ProfileAccess::LoadProductsInfo()
        {
            //load SeaMonkey 2 profiles to m_ProductProfileList
            sal_Int32 count = LoadXPToolkitProfiles(MozillaProductType_Mozilla);

            //load thunderbird profiles to m_ProductProfileList
            count += LoadXPToolkitProfiles(MozillaProductType_Thunderbird);

            //load firefox profiles to m_ProductProfileList
            //firefox profile does not containt address book, but maybe others need them
            count += LoadXPToolkitProfiles(MozillaProductType_Firefox);
            return count;
        }
        //Thunderbird and firefox profiles are saved in profiles.ini
        sal_Int32 ProfileAccess::LoadXPToolkitProfiles(MozillaProductType product)
        {
            sal_Int32 index=product;
            ProductStruct &m_Product = m_ProductProfileList[index];

            ::rtl::OUString regDir = getRegistryDir(product);
            ::rtl::OUString profilesIni( regDir );
            profilesIni += ::rtl::OUString::createFromAscii( "profiles.ini" );
            IniParser parser( profilesIni );
            IniSectionMap &mAllSection = *(parser.getAllSection());

            IniSectionMap::iterator iBegin = mAllSection.begin();
            IniSectionMap::iterator iEnd = mAllSection.end();
            for(;iBegin != iEnd;iBegin++)
            {
                ini_Section *aSection = &(*iBegin).second;
                ::rtl::OUString profileName;
                ::rtl::OUString profilePath;
                ::rtl::OUString sIsRelative;
                ::rtl::OUString sIsDefault;

                for(NameValueList::iterator itor=aSection->lList.begin();
                    itor != aSection->lList.end();
                    itor++)
                {
                        struct ini_NameValue * aValue = &(*itor);
                        if (aValue->sName.equals(::rtl::OUString::createFromAscii("Name")))
                        {
                            profileName = aValue->sValue;
                        }
                        else if (aValue->sName.equals(::rtl::OUString::createFromAscii("IsRelative")))
                        {
                            sIsRelative = aValue->sValue;
                        }
                        else if (aValue->sName.equals(::rtl::OUString::createFromAscii("Path")))
                        {
                            profilePath = aValue->sValue;
                        }
                        else if (aValue->sName.equals(::rtl::OUString::createFromAscii("Default")))
                        {
                            sIsDefault = aValue->sValue;
                        }
                }
                if (profileName.getLength() != 0 || profilePath.getLength() != 0)
                {
                    sal_Int32 isRelative = 0;
                    if (sIsRelative.getLength() != 0)
                    {
                        isRelative = sIsRelative.toInt32();
                    }

                    ProfileStruct*  profileItem     = new ProfileStruct(product,profileName,
                            regDir + profilePath);
                    m_Product.mProfileList[profileName] = profileItem;

                    sal_Int32 isDefault = 0;
                    if (sIsDefault.getLength() != 0)
                    {
                        isDefault = sIsDefault.toInt32();
                    }
                    if (isDefault)
                        m_Product.mCurrentProfileName = profileName;

                }

            }
            return static_cast< ::sal_Int32 >(m_Product.mProfileList.size());
        }

        ::rtl::OUString ProfileAccess::getProfilePath( ::com::sun::star::mozilla::MozillaProductType product, const ::rtl::OUString& profileName ) throw (::com::sun::star::uno::RuntimeException)
        {
            sal_Int32 index=product;
            ProductStruct &m_Product = m_ProductProfileList[index];
            if (!m_Product.mProfileList.size() || m_Product.mProfileList.find(profileName) == m_Product.mProfileList.end())
            {
                //Profile not found
                return ::rtl::OUString();
            }
            else
                return m_Product.mProfileList[profileName]->getProfilePath();
        }

        ::sal_Int32 ProfileAccess::getProfileCount( ::com::sun::star::mozilla::MozillaProductType product) throw (::com::sun::star::uno::RuntimeException)
        {
            sal_Int32 index=product;
            ProductStruct &m_Product = m_ProductProfileList[index];
            return static_cast< ::sal_Int32 >(m_Product.mProfileList.size());
        }
        ::sal_Int32 ProfileAccess::getProfileList( ::com::sun::star::mozilla::MozillaProductType product, ::com::sun::star::uno::Sequence< ::rtl::OUString >& list ) throw (::com::sun::star::uno::RuntimeException)
        {
            sal_Int32 index=product;
            ProductStruct &m_Product = m_ProductProfileList[index];
            list.realloc(static_cast<sal_Int32>(m_Product.mProfileList.size()));
            sal_Int32 i=0;
            for(ProfileList::iterator itor=m_Product.mProfileList.begin();
                itor != m_Product.mProfileList.end();
                itor++)
            {
                ProfileStruct * aProfile = (*itor).second;
                list[i] = aProfile->getProfileName();
                i++;
            }

            return static_cast< ::sal_Int32 >(m_Product.mProfileList.size());
        }

        ::rtl::OUString ProfileAccess::getDefaultProfile( ::com::sun::star::mozilla::MozillaProductType product ) throw (::com::sun::star::uno::RuntimeException)
        {
            sal_Int32 index=product;
            ProductStruct &m_Product = m_ProductProfileList[index];
            if (m_Product.mCurrentProfileName.getLength() != 0)
            {
                //default profile setted in mozilla registry
                return m_Product.mCurrentProfileName;
            }
            if (m_Product.mProfileList.size() == 0)
            {
                //there are not any profiles
                return ::rtl::OUString();
            }
            ProfileStruct * aProfile = (*m_Product.mProfileList.begin()).second;
            return aProfile->getProfileName();
        }

        ::sal_Bool ProfileAccess::isProfileLocked( ::com::sun::star::mozilla::MozillaProductType product, const ::rtl::OUString& profileName ) throw (::com::sun::star::uno::RuntimeException)
        {
            (void)product; /* avoid warning about unused parameter */
            (void)profileName; /* avoid warning about unused parameter */
            return sal_True;
        }

        ::sal_Bool ProfileAccess::getProfileExists( ::com::sun::star::mozilla::MozillaProductType product, const ::rtl::OUString& profileName ) throw (::com::sun::star::uno::RuntimeException)
        {
            sal_Int32 index=product;
            ProductStruct &m_Product = m_ProductProfileList[index];
            if (!m_Product.mProfileList.size() || m_Product.mProfileList.find(profileName) == m_Product.mProfileList.end())
            {
                return sal_False;
            }
            else
                return sal_True;
        }
    }
}



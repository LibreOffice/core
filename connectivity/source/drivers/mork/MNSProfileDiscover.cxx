/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "MNSProfileDiscover.hxx"

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif
#include <MNSFolders.hxx>
#include <MNSINIParser.hxx>

namespace connectivity
{
    namespace mork
    {
        ProfileStruct::ProfileStruct(MozillaProductType aProduct,::rtl::OUString aProfileName,
            const ::rtl::OUString& aProfilePath
          )
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
            profilesIni += ::rtl::OUString("profiles.ini");
            IniParser parser( profilesIni );
            IniSectionMap &mAllSection = *(parser.getAllSection());

            IniSectionMap::iterator iBegin = mAllSection.begin();
            IniSectionMap::iterator iEnd = mAllSection.end();
            for(;iBegin != iEnd;++iBegin)
            {
                ini_Section *aSection = &(*iBegin).second;
                ::rtl::OUString profileName;
                ::rtl::OUString profilePath;
                ::rtl::OUString sIsRelative;
                ::rtl::OUString sIsDefault;

                for(NameValueList::iterator itor=aSection->lList.begin();
                    itor != aSection->lList.end();
                    ++itor)
                {
                        struct ini_NameValue * aValue = &(*itor);
                        if ( aValue->sName == "Name" )
                        {
                            profileName = aValue->sValue;
                        }
                        else if ( aValue->sName == "IsRelative" )
                        {
                            sIsRelative = aValue->sValue;
                        }
                        else if ( aValue->sName == "Path" )
                        {
                            profilePath = aValue->sValue;
                        }
                        else if ( aValue->sName == "Default" )
                        {
                            sIsDefault = aValue->sValue;
                        }
                }
                if (!(profileName.isEmpty() && profilePath.isEmpty()))
                {
                    sal_Int32 isRelative = 0;
                    if (!sIsRelative.isEmpty())
                    {
                        isRelative = sIsRelative.toInt32();
                    }

                    rtl::OUString fullProfilePath;
                    if(isRelative)
                    {
                        fullProfilePath = regDir + profilePath;
                    }
                    else
                    {
                        fullProfilePath = profilePath;
                    }

                    ProfileStruct*  profileItem     = new ProfileStruct(product,profileName,
                            fullProfilePath
                        );
                    m_Product.mProfileList[profileName] = profileItem;

                    sal_Int32 isDefault = 0;
                    if (!sIsDefault.isEmpty())
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

        ::rtl::OUString ProfileAccess::getDefaultProfile( ::com::sun::star::mozilla::MozillaProductType product ) throw (::com::sun::star::uno::RuntimeException)
        {
            sal_Int32 index=product;
            ProductStruct &m_Product = m_ProductProfileList[index];
            if (!m_Product.mCurrentProfileName.isEmpty())
            {
                //default profile setted in mozilla registry
                return m_Product.mCurrentProfileName;
            }
            if (m_Product.mProfileList.empty())
            {
                //there are not any profiles
                return ::rtl::OUString();
            }
            ProfileStruct * aProfile = (*m_Product.mProfileList.begin()).second;
            return aProfile->getProfileName();
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

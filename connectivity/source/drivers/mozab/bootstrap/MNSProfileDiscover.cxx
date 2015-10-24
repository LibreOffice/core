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
#include "MNSFolders.hxx"
#include "MNSINIParser.hxx"

namespace connectivity
{
    namespace mozab
    {
        ProfileStruct::ProfileStruct(MozillaProductType aProduct, const OUString& aProfileName,
            const OUString& aProfilePath
          )
        {
            product=aProduct;
            profileName = aProfileName;
            profilePath = aProfilePath;
        }
        OUString ProfileStruct::getProfilePath()
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
            //tdf#39279: LO should search Thunderbird first then Seamonkey and finally Firefox
            //load thunderbird profiles to m_ProductProfileList
            sal_Int32 count = LoadXPToolkitProfiles(MozillaProductType_Thunderbird);

            //load SeaMonkey 2 profiles to m_ProductProfileList
            count += LoadXPToolkitProfiles(MozillaProductType_Mozilla);

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

            OUString regDir = getRegistryDir(product);
            OUString profilesIni = regDir + "profiles.ini";
            IniParser parser( profilesIni );
            IniSectionMap &mAllSection = parser.getAllSection();

            IniSectionMap::iterator iBegin = mAllSection.begin();
            IniSectionMap::iterator iEnd = mAllSection.end();
            for(;iBegin != iEnd;++iBegin)
            {
                ini_Section *aSection = &(*iBegin).second;
                OUString profileName;
                OUString profilePath;
                OUString sIsRelative;
                OUString sIsDefault;

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

                    OUString fullProfilePath;
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

        OUString ProfileAccess::getProfilePath( ::com::sun::star::mozilla::MozillaProductType product, const OUString& profileName ) throw (::com::sun::star::uno::RuntimeException)
        {
            sal_Int32 index=product;
            ProductStruct &m_Product = m_ProductProfileList[index];
            if (m_Product.mProfileList.empty() || m_Product.mProfileList.find(profileName) == m_Product.mProfileList.end())
            {
                //Profile not found
                return OUString();
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
        ::sal_Int32 ProfileAccess::getProfileList( ::com::sun::star::mozilla::MozillaProductType product, ::com::sun::star::uno::Sequence< OUString >& list ) throw (::com::sun::star::uno::RuntimeException)
        {
            sal_Int32 index=product;
            ProductStruct &m_Product = m_ProductProfileList[index];
            list.realloc(static_cast<sal_Int32>(m_Product.mProfileList.size()));
            sal_Int32 i=0;
            for(ProfileList::iterator itor=m_Product.mProfileList.begin();
                itor != m_Product.mProfileList.end();
                ++itor)
            {
                ProfileStruct * aProfile = (*itor).second;
                list[i] = aProfile->getProfileName();
                i++;
            }

            return static_cast< ::sal_Int32 >(m_Product.mProfileList.size());
        }

        OUString ProfileAccess::getDefaultProfile( ::com::sun::star::mozilla::MozillaProductType product ) throw (::com::sun::star::uno::RuntimeException)
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
                return OUString();
            }
            ProfileStruct * aProfile = (*m_Product.mProfileList.begin()).second;
            return aProfile->getProfileName();
        }
        bool ProfileAccess::isProfileLocked( ::com::sun::star::mozilla::MozillaProductType product, const OUString& profileName ) throw (::com::sun::star::uno::RuntimeException)
        {
            (void)product; /* avoid warning about unused parameter */
            (void)profileName; /* avoid warning about unused parameter */
            return true;
        }

        bool ProfileAccess::getProfileExists( ::com::sun::star::mozilla::MozillaProductType product, const OUString& profileName ) throw (::com::sun::star::uno::RuntimeException)
        {
            sal_Int32 index=product;
            ProductStruct &m_Product = m_ProductProfileList[index];
            if (m_Product.mProfileList.empty() || m_Product.mProfileList.find(profileName) == m_Product.mProfileList.end())
            {
                return false;
            }
            else
                return true;
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

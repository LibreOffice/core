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
        ProfileStruct::ProfileStruct()
        {
        }

        ProfileStruct::ProfileStruct(const OUString& aProfileName,
                                     const OUString& aProfilePath)
            : profileName(aProfileName)
            , profilePath(aProfilePath)
        {
        }

        const OUString& ProfileStruct::getProfilePath() const
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

        void ProfileAccess::LoadProductsInfo()
        {
            //tdf#39279: LO should search Thunderbird first then Seamonkey and finally Firefox
            //load thunderbird profiles to m_ProductProfileList
            LoadXPToolkitProfiles(MozillaProductType_Thunderbird);

            //load SeaMonkey 2 profiles to m_ProductProfileList
            LoadXPToolkitProfiles(MozillaProductType_Mozilla);

            //load firefox profiles to m_ProductProfileList
            //firefox profile does not contain address book, but maybe others need them
            LoadXPToolkitProfiles(MozillaProductType_Firefox);
        }
        //Thunderbird and firefox profiles are saved in profiles.ini
        void ProfileAccess::LoadXPToolkitProfiles(MozillaProductType product)
        {
            sal_Int32 index=static_cast<sal_Int32>(product);
            ProductStruct &rProduct = m_ProductProfileList[index];

            OUString regDir = getRegistryDir(product);
            OUString profilesIni = regDir + "profiles.ini";
            IniParser parser( profilesIni );
            IniSectionMap &rAllSection = parser.getAllSection();

            for(auto& rSection : rAllSection)
            {
                ini_Section *aSection = &rSection.second;
                OUString profileName;
                OUString profilePath;
                OUString sIsRelative;
                OUString sIsDefault;

                for(auto& rValue : aSection->vVector)
                {
                    struct ini_NameValue * aValue = &rValue;
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

                    rProduct.mProfileList[profileName] = ProfileStruct(profileName,fullProfilePath);

                    sal_Int32 isDefault = 0;
                    if (!sIsDefault.isEmpty())
                    {
                        isDefault = sIsDefault.toInt32();
                    }
                    if (isDefault)
                        rProduct.mCurrentProfileName = profileName;

                }

            }
        }

        OUString ProfileAccess::getProfilePath( css::mozilla::MozillaProductType product, const OUString& profileName )
        {
            sal_Int32 index=static_cast<sal_Int32>(product);
            ProductStruct &rProduct = m_ProductProfileList[index];
            if (rProduct.mProfileList.empty() || rProduct.mProfileList.find(profileName) == rProduct.mProfileList.end())
            {
                //Profile not found
                return OUString();
            }
            else
                return rProduct.mProfileList[profileName].getProfilePath();
        }

        ::sal_Int32 ProfileAccess::getProfileCount( css::mozilla::MozillaProductType product)
        {
            sal_Int32 index=static_cast<sal_Int32>(product);
            ProductStruct &rProduct = m_ProductProfileList[index];
            return static_cast< ::sal_Int32 >(rProduct.mProfileList.size());
        }
        ::sal_Int32 ProfileAccess::getProfileList( css::mozilla::MozillaProductType product, css::uno::Sequence< OUString >& list )
        {
            sal_Int32 index=static_cast<sal_Int32>(product);
            ProductStruct &rProduct = m_ProductProfileList[index];
            list.realloc(static_cast<sal_Int32>(rProduct.mProfileList.size()));
            sal_Int32 i=0;
            for(const auto& rEntry : rProduct.mProfileList)
            {
                const ProfileStruct& rProfile = rEntry.second;
                list[i] = rProfile.getProfileName();
                i++;
            }

            return static_cast< ::sal_Int32 >(rProduct.mProfileList.size());
        }

        OUString ProfileAccess::getDefaultProfile( css::mozilla::MozillaProductType product )
        {
            sal_Int32 index=static_cast<sal_Int32>(product);
            ProductStruct &rProduct = m_ProductProfileList[index];
            if (!rProduct.mCurrentProfileName.isEmpty())
            {
                //default profile set in mozilla registry
                return rProduct.mCurrentProfileName;
            }
            if (rProduct.mProfileList.empty())
            {
                //there are not any profiles
                return OUString();
            }
            const ProfileStruct& rProfile = (*rProduct.mProfileList.begin()).second;
            return rProfile.getProfileName();
        }

        bool ProfileAccess::getProfileExists( css::mozilla::MozillaProductType product, const OUString& profileName )
        {
            sal_Int32 index=static_cast<sal_Int32>(product);
            ProductStruct &rProduct = m_ProductProfileList[index];
            return rProduct.mProfileList.find(profileName) != rProduct.mProfileList.end();
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

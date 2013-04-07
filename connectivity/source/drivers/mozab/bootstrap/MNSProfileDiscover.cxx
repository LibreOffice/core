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
#ifndef MINIMAL_PROFILEDISCOVER
#include "MNSProfile.hxx"

#include "pratom.h"
#include "prmem.h"
#include "plstr.h"
#include "prenv.h"

#include "nsIEnumerator.h"
#include "prprf.h"
#include "nsCOMPtr.h"
#include "nsIComponentManager.h"
#include "nsEscape.h"
#include "nsDirectoryServiceDefs.h"
#include "nsAppDirectoryServiceDefs.h"
#include "nsILocalFile.h"
#include "nsReadableUtils.h"


#if defined(XP_MAC) || defined(XP_MACOSX)
#include <Processes.h>
#include <CFBundle.h>
#include "nsILocalFileMac.h"
#endif

#ifdef XP_UNIX
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include "prnetdb.h"
#include "prsystem.h"
#endif

#ifdef VMS
#include <rmsdef.h>
#endif

#include "pre_include_mozilla.h"
#include "nsICharsetConverterManager.h"
#include "nsIPlatformCharset.h"
#include "post_include_mozilla.h"

#if defined (XP_UNIX)
#   define USER_ENVIRONMENT_VARIABLE "USER"
#   define LOGNAME_ENVIRONMENT_VARIABLE "LOGNAME"
#   define HOME_ENVIRONMENT_VARIABLE "HOME"
#   define PROFILE_NAME_ENVIRONMENT_VARIABLE "PROFILE_NAME"
#   define PROFILE_HOME_ENVIRONMENT_VARIABLE "PROFILE_HOME"
#   define DEFAULT_UNIX_PROFILE_NAME "default"
#   ifndef XP_MACOSX   /* Don't use symlink-based locking on OS X */
#       define USE_SYMLINK_LOCKING
#   endif
#endif

// IID and CIDs of all the services needed
static NS_DEFINE_CID(kCharsetConverterManagerCID, NS_ICHARSETCONVERTERMANAGER_CID);
#endif

#include <MNSFolders.hxx>
#include <MNSINIParser.hxx>

namespace connectivity
{
    namespace mozab
    {
        ProfileStruct::ProfileStruct(MozillaProductType aProduct,OUString aProfileName,
#ifdef MINIMAL_PROFILEDISCOVER
            const OUString& aProfilePath
#else
            nsILocalFile * aProfilePath
#endif
          )
        {
            product=aProduct;
            profileName = aProfileName;
            profilePath = aProfilePath;
        }
        OUString ProfileStruct::getProfilePath()
        {
#ifdef MINIMAL_PROFILEDISCOVER
            return profilePath;
#else
            if (profilePath)
            {
                nsAutoString path;
                nsresult rv = profilePath->GetPath(path);
                NS_ENSURE_SUCCESS(rv, OUString());
                // PRUnichar != sal_Unicode in mingw
                return OUString(reinterpret_cast_mingw_only<const sal_Unicode *>(path.get()));
            }
            else
                return OUString();
#endif
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

#ifndef MINIMAL_PROFILEDISCOVER
            nsresult rv;
#endif
            OUString regDir = getRegistryDir(product);
            OUString profilesIni( regDir );
            profilesIni += OUString("profiles.ini");
            IniParser parser( profilesIni );
            IniSectionMap &mAllSection = *(parser.getAllSection());

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

#ifndef MINIMAL_PROFILEDISCOVER
                    nsCOMPtr<nsILocalFile> rootDir;
                    rv = NS_NewLocalFile(EmptyString(), PR_TRUE,
                                            getter_AddRefs(rootDir));
                    if (NS_FAILED(rv)) continue;

                    OString sPath = OUStringToOString(profilePath, RTL_TEXTENCODING_UTF8);
                    nsCAutoString filePath(sPath.getStr());

                    if (isRelative) {
                        // PRUnichar != sal_Unicode in mingw
                        nsAutoString registryDir( reinterpret_cast_mingw_only<const PRUnichar *>(regDir.getStr()) );
                        nsCOMPtr<nsILocalFile>     mAppData;
                        rv = NS_NewLocalFile(registryDir, PR_TRUE,
                                        getter_AddRefs(mAppData));
                        if (NS_FAILED(rv)) continue;
                        rv = rootDir->SetRelativeDescriptor(mAppData, filePath);
                    } else {
                        rv = rootDir->SetPersistentDescriptor(filePath);
                    }
                    if (NS_FAILED(rv)) continue;
#else
                    OUString fullProfilePath;
                    if(isRelative)
                    {
                        fullProfilePath = regDir + profilePath;
                    }
                    else
                    {
                        fullProfilePath = profilePath;
                    }
#endif

                    ProfileStruct*  profileItem     = new ProfileStruct(product,profileName,
#ifdef MINIMAL_PROFILEDISCOVER
                            fullProfilePath
#else
                            rootDir
#endif
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
            if (!m_Product.mProfileList.size() || m_Product.mProfileList.find(profileName) == m_Product.mProfileList.end())
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
#ifndef MINIMAL_PROFILEDISCOVER
        nsresult ProfileAccess::isExistFileOrSymlink(nsILocalFile* aFile,PRBool *bExist)
        {
            nsresult rv;
            nsAutoString path;
            aFile->GetPath(path);
            rv = aFile->Exists(bExist);
            NS_ENSURE_SUCCESS(rv, rv);
            if (!*bExist)
            {
                rv = aFile->IsSymlink(bExist);
                NS_ENSURE_SUCCESS(rv, rv);
            }
            return rv;
        }
        nsresult ProfileAccess::isLockExist(nsILocalFile* aFile)
        {
#if defined (XP_MACOSX)
            NS_NAMED_LITERAL_STRING(LOCKFILE_NAME, ".parentlock");
            NS_NAMED_LITERAL_STRING(OLD_LOCKFILE_NAME, "parent.lock");
#elif defined (XP_UNIX)
            NS_ConvertASCIItoUTF16 OLD_LOCKFILE_NAME("lock");
            NS_ConvertASCIItoUTF16 LOCKFILE_NAME(".parentlock");
#else
            NS_NAMED_LITERAL_STRING(OLD_LOCKFILE_NAME, "parent.lock");
            NS_NAMED_LITERAL_STRING(LOCKFILE_NAME, "parent.lock");
#endif

            nsresult rv;

            PRBool isDir;
            rv = aFile->IsDirectory(&isDir);
            NS_ENSURE_SUCCESS(rv, rv);
            if (!isDir)
                return NS_ERROR_FILE_NOT_DIRECTORY;

            nsCOMPtr<nsILocalFile> lockFile;
            rv = aFile->Clone((nsIFile **)((void **)getter_AddRefs(lockFile)));
            NS_ENSURE_SUCCESS(rv, rv);

            rv = lockFile->Append(LOCKFILE_NAME);
            NS_ENSURE_SUCCESS(rv, rv);
            PRBool nExist=PR_FALSE;
            rv = isExistFileOrSymlink(lockFile,&nExist);
            NS_ENSURE_SUCCESS(rv, rv);
            if (!nExist) // Check OLD_LOCKFILE_NAME
            {
                nsCOMPtr<nsILocalFile> oldlockFile;
                rv = aFile->Clone((nsIFile **)((void **)getter_AddRefs(oldlockFile)));
                NS_ENSURE_SUCCESS(rv, rv);

                rv = oldlockFile->Append(OLD_LOCKFILE_NAME);
                NS_ENSURE_SUCCESS(rv, rv);
                rv = isExistFileOrSymlink(oldlockFile,&nExist);
                NS_ENSURE_SUCCESS(rv, rv);
            }
            return nExist;
        }

#endif
        ::sal_Bool ProfileAccess::isProfileLocked( ::com::sun::star::mozilla::MozillaProductType product, const OUString& profileName ) throw (::com::sun::star::uno::RuntimeException)
        {
#ifdef MINIMAL_PROFILEDISCOVER
            (void)product; /* avoid warning about unused parameter */
            (void)profileName; /* avoid warning about unused parameter */
            return sal_True;
#else
            OUString path = getProfilePath(product,profileName);
            if (path.isEmpty())
                return sal_True;

            // PRUnichar != sal_Unicode in mingw
            nsAutoString filePath(reinterpret_cast_mingw_only<const PRUnichar *>(path.getStr()));

            nsresult rv;
            nsCOMPtr<nsILocalFile>  localFile;
            rv = NS_NewLocalFile(filePath, PR_TRUE,
                                getter_AddRefs(localFile));
            NS_ENSURE_SUCCESS(rv,sal_True);

            PRBool exists = PR_FALSE;
            rv = localFile->Exists(&exists);
            NS_ENSURE_SUCCESS(rv, sal_True);
            if (!exists)
                return sal_True;

            // If the profile is locked, we return true
            rv = isLockExist(localFile);
            if (rv)
                return sal_True;
            return sal_False;
#endif
        }

        ::sal_Bool ProfileAccess::getProfileExists( ::com::sun::star::mozilla::MozillaProductType product, const OUString& profileName ) throw (::com::sun::star::uno::RuntimeException)
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

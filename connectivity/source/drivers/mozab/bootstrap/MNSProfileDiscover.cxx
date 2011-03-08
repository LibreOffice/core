/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

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

#include "nsICharsetConverterManager.h"
#include "nsIPlatformCharset.h"


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

// Registry Keys

static ::rtl::OUString szProfileSubtreeString( RTL_CONSTASCII_USTRINGPARAM( "Profiles" ));
static ::rtl::OUString szCurrentProfileString( RTL_CONSTASCII_USTRINGPARAM( "CurrentProfile" ));
static ::rtl::OUString szDirectoryString( RTL_CONSTASCII_USTRINGPARAM( "directory" ));

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
#ifdef MINIMAL_PROFILEDISCOVER
            const ::rtl::OUString& aProfilePath
#else
            nsILocalFile * aProfilePath
#endif
          )
        {
            product=aProduct;
            profileName = aProfileName;
            profilePath = aProfilePath;
        }
        ::rtl::OUString ProfileStruct::getProfilePath()
        {
#ifdef MINIMAL_PROFILEDISCOVER
            return profilePath;
#else
            if (profilePath)
            {
                nsAutoString path;
                nsresult rv = profilePath->GetPath(path);
                NS_ENSURE_SUCCESS(rv, ::rtl::OUString());
                // PRUnichar != sal_Unicode in mingw
                return ::rtl::OUString(reinterpret_cast_mingw_only<const sal_Unicode *>(path.get()));
            }
            else
                return ::rtl::OUString();
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
            ::rtl::OUString regDir = getRegistryDir(product);
            ::rtl::OUString profilesIni( regDir );
            profilesIni += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("profiles.ini"));
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
                        if (aValue->sName.equals(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name"))))
                        {
                            profileName = aValue->sValue;
                        }
                        else if (aValue->sName.equals(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsRelative"))))
                        {
                            sIsRelative = aValue->sValue;
                        }
                        else if (aValue->sName.equals(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Path"))))
                        {
                            profilePath = aValue->sValue;
                        }
                        else if (aValue->sName.equals(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Default"))))
                        {
                            sIsDefault = aValue->sValue;
                        }
                }
                if (profileName.getLength() != 0 || profilePath.getLength() != 0)
                {
#ifndef MINIMAL_PROFILEDISCOVER
                    sal_Int32 isRelative = 0;
                    if (sIsRelative.getLength() != 0)
                    {
                        isRelative = sIsRelative.toInt32();
                    }

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
#endif

                    ProfileStruct*  profileItem     = new ProfileStruct(product,profileName,
#ifdef MINIMAL_PROFILEDISCOVER
                            regDir + profilePath
#else
                            rootDir
#endif
                        );
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
        ::sal_Bool ProfileAccess::isProfileLocked( ::com::sun::star::mozilla::MozillaProductType product, const ::rtl::OUString& profileName ) throw (::com::sun::star::uno::RuntimeException)
        {
#ifdef MINIMAL_PROFILEDISCOVER
            (void)product; /* avoid warning about unused parameter */
            (void)profileName; /* avoid warning about unused parameter */
            return sal_True;
#else
            ::rtl::OUString path = getProfilePath(product,profileName);
            if (!path.getLength())
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

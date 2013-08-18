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

#include <config_features.h>
#include <config_folders.h>

#include "sal/config.h"

#include "userinstall.hxx"
#include "langselect.hxx"

#include <stdio.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <osl/process.h>
#include <osl/diagnose.h>
#include <osl/security.hxx>
#include <rtl/ref.hxx>

#include <officecfg/Setup.hxx>
#include <unotools/bootstrap.hxx>
#include <svl/languageoptions.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <i18nlangtag/mslangid.hxx>
#include <com/sun/star/lang/XLocalizable.hpp>
#include <com/sun/star/lang/Locale.hpp>

#include "app.hxx"

using namespace osl;
using namespace utl;
using namespace com::sun::star::container;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;


namespace desktop {

    static UserInstall::UserInstallStatus create_user_install(OUString&);

    static bool is_user_install()
    {
        try
        {
            Reference< XMultiServiceFactory > theConfigProvider(
                com::sun::star::configuration::theDefaultProvider::get(
                    comphelper::getProcessComponentContext() ) );

            // localize the provider to user selection
            Reference< XLocalizable > localizable(theConfigProvider, UNO_QUERY_THROW);
            OUString aUserLanguage = LanguageSelection::getLanguageString();
            LanguageTag aLanguageTag(aUserLanguage);
            localizable->setLocale(aLanguageTag.getLocale( false));

            return officecfg::Setup::Office::ooSetupInstCompleted::get();
        }
        catch (Exception const & e)
        {
            OString msg(OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US));
            OSL_FAIL(msg.getStr());
        }

        return false;
    }

    UserInstall::UserInstallStatus UserInstall::finalize()
    {
        OUString aUserInstallPath;
        utl::Bootstrap::PathStatus aLocateResult =
            utl::Bootstrap::locateUserInstallation(aUserInstallPath);

        switch (aLocateResult) {

            case utl::Bootstrap::DATA_INVALID:
            case utl::Bootstrap::DATA_MISSING:
            case utl::Bootstrap::DATA_UNKNOWN:
                // cannot find a valid path or path is missing
                return E_Unknown;

            case utl::Bootstrap::PATH_EXISTS:
            {
                // path exists, check if an installation lives there
                if ( is_user_install() )
                {
                    return Ok;
                }
                // Note: fall-thru intended.
            }
            case utl::Bootstrap::PATH_VALID:
                // found a path but need to create user install
                return create_user_install(aUserInstallPath);
            default:
                return E_Unknown;
        }
    }

#if HAVE_FEATURE_DESKTOP
    static osl::FileBase::RC copy_recursive( const OUString& srcUnqPath, const OUString& dstUnqPath)
    {
        FileBase::RC err;
        DirectoryItem aDirItem;
        DirectoryItem::get(srcUnqPath, aDirItem);
        FileStatus aFileStatus(osl_FileStatus_Mask_All);
        aDirItem.getFileStatus(aFileStatus);

        if( aFileStatus.getFileType() == FileStatus::Directory)
        {
            // create directory if not already there
            err = Directory::create( dstUnqPath );
            if (err == osl::FileBase::E_EXIST)
                err = osl::FileBase::E_None;

            FileBase::RC next = err;
            if (err == osl::FileBase::E_None)
            {
                // iterate through directory contents
                Directory aDir( srcUnqPath );
                aDir.open();
                while (err ==  osl::FileBase::E_None &&
                    (next = aDir.getNextItem( aDirItem )) == osl::FileBase::E_None)
                {
                    aDirItem.getFileStatus(aFileStatus);
                    // generate new src/dst pair and make recursive call
                    OUString newSrcUnqPath = aFileStatus.getFileURL();
                    OUString newDstUnqPath = dstUnqPath;
                    OUString itemname = aFileStatus.getFileName();
                    // append trailing '/' if needed
                    if (newDstUnqPath.lastIndexOf(sal_Unicode('/')) != newDstUnqPath.getLength()-1)
                        newDstUnqPath += OUString("/");
                    newDstUnqPath += itemname;
                    // recursion
                    err = copy_recursive(newSrcUnqPath, newDstUnqPath);
                }
                aDir.close();

                if ( err != osl::FileBase::E_None )
                    return err;
                if( next != FileBase::E_NOENT )
                    err = FileBase::E_INVAL;
            }
        }
        else
        {
            // copy single file - foldback
            err = File::copy( srcUnqPath,dstUnqPath );
        }

        return err;
    }
#endif

    static UserInstall::UserInstallStatus create_user_install(OUString& aUserPath)
    {
        OUString aBasePath;
        if (utl::Bootstrap::locateBaseInstallation(aBasePath) != utl::Bootstrap::PATH_EXISTS)
            return UserInstall::E_InvalidBaseinstall;

        // create the user directory
        FileBase::RC rc = Directory::createPath(aUserPath);
        if ((rc != FileBase::E_None) && (rc != FileBase::E_EXIST)) return UserInstall::E_Creation;

#if defined(UNIX) && !defined(ANDROID) && !defined(IOS)
        // Set safer permissions for the user directory by default:
        File::setAttributes(aUserPath, osl_File_Attribute_OwnWrite| osl_File_Attribute_OwnRead| osl_File_Attribute_OwnExe);
#endif

#if !defined(ANDROID) && !defined(IOS)
        // as of now osl_copyFile does not work on Android => don't do this.

        // Copy data from shared data directory of base installation:
        rc = copy_recursive(
            aBasePath + OUString("/" LIBO_SHARE_PRESETS_FOLDER),
            aUserPath + OUString("/user"));
        if ((rc != FileBase::E_None) && (rc != FileBase::E_EXIST))
        {
            if ( rc == FileBase::E_NOSPC )
                return UserInstall::E_NoDiskSpace;
            else if ( rc == FileBase::E_ACCES )
                return UserInstall::E_NoWriteAccess;
            else
                return UserInstall::E_Creation;
        }
#endif

        boost::shared_ptr< comphelper::ConfigurationChanges > batch(
            comphelper::ConfigurationChanges::create());
        officecfg::Setup::Office::ooSetupInstCompleted::set(true, batch);
        batch->commit();

        return UserInstall::Created;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

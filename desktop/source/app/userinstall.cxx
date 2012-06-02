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
#include <i18npool/mslangid.hxx>
#include <com/sun/star/lang/XLocalizable.hpp>
#include <com/sun/star/lang/Locale.hpp>

#include "app.hxx"

using rtl::OString;
using rtl::OUString;
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
            Locale aLocale = LanguageSelection::IsoStringToLocale(aUserLanguage);
            localizable->setLocale(aLocale);

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

    static osl::FileBase::RC copy_recursive( const rtl::OUString& srcUnqPath, const rtl::OUString& dstUnqPath)
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
                    rtl::OUString newSrcUnqPath = aFileStatus.getFileURL();
                    rtl::OUString newDstUnqPath = dstUnqPath;
                    rtl::OUString itemname = aFileStatus.getFileName();
                    // append trailing '/' if needed
                    if (newDstUnqPath.lastIndexOf(sal_Unicode('/')) != newDstUnqPath.getLength()-1)
                        newDstUnqPath += rtl::OUString("/");
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

#ifdef ANDROID
        fprintf (stderr, "copy_recursive '%s' to '%s' returns (%d)0x%x\n",
                 rtl::OUStringToOString(srcUnqPath, RTL_TEXTENCODING_UTF8).getStr(),
                 rtl::OUStringToOString(dstUnqPath, RTL_TEXTENCODING_UTF8).getStr(),
                 (int)err, (int)err);
#endif
        return err;
    }

    static UserInstall::UserInstallStatus create_user_install(OUString& aUserPath)
    {
        OUString aBasePath;
        if (utl::Bootstrap::locateBaseInstallation(aBasePath) != utl::Bootstrap::PATH_EXISTS)
            return UserInstall::E_InvalidBaseinstall;

        // create the user directory
        FileBase::RC rc = Directory::createPath(aUserPath);
        if ((rc != FileBase::E_None) && (rc != FileBase::E_EXIST)) return UserInstall::E_Creation;

#ifdef UNIX
        // Set safer permissions for the user directory by default:
        File::setAttributes(aUserPath, osl_File_Attribute_OwnWrite| osl_File_Attribute_OwnRead| osl_File_Attribute_OwnExe);
#endif

#ifndef ANDROID
        // as of now osl_copyFile does not work on Android => don't do this.

        // Copy data from shared data directory of base installation:
        rc = copy_recursive(
            aBasePath + rtl::OUString("/presets"),
            aUserPath + rtl::OUString("/user"));
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

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
#include "precompiled_desktop.hxx"


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

#include <tools/resmgr.hxx>
#include <unotools/bootstrap.hxx>
#include <svl/languageoptions.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <i18npool/mslangid.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/beans/XHierarchicalPropertySet.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
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

    static UserInstall::UserInstallError create_user_install(OUString&);

    static bool is_user_install()
    {
        try
        {
            OUString sConfigSrvc(
                 RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.configuration.ConfigurationProvider" ) );
            OUString sAccessSrvc(
                 RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.configuration.ConfigurationAccess" ) );

            // get configuration provider
            Reference< XMultiServiceFactory > theMSF
                = comphelper::getProcessServiceFactory();
            Reference< XMultiServiceFactory > theConfigProvider
                = Reference< XMultiServiceFactory >(
                    theMSF->createInstance(sConfigSrvc), UNO_QUERY_THROW);

            // localize the provider to user selection
            Reference< XLocalizable > localizable(theConfigProvider, UNO_QUERY_THROW);
            OUString aUserLanguage = LanguageSelection::getLanguageString();
            Locale aLocale = LanguageSelection::IsoStringToLocale(aUserLanguage);
            localizable->setLocale(aLocale);

            Sequence< Any > theArgs(1);
            NamedValue v;
            v.Name = OUString(RTL_CONSTASCII_USTRINGPARAM("NodePath"));
            v.Value = makeAny(OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Setup")));
            theArgs[0] <<= v;
            Reference< XHierarchicalNameAccess> hnacc(
                theConfigProvider->createInstanceWithArguments(
                    sAccessSrvc, theArgs), UNO_QUERY_THROW);

            try
            {
                sal_Bool bValue = sal_False;
                hnacc->getByHierarchicalName(
                        OUString( RTL_CONSTASCII_USTRINGPARAM(
                            "Office/ooSetupInstCompleted" ) ) ) >>= bValue;

                return bValue ? true : false;
            }
            catch ( NoSuchElementException const & )
            {
                // just return false in this case.
            }
        }
        catch (Exception const & e)
        {
            OString msg(OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US));
            OSL_ENSURE(sal_False, msg.getStr());
        }

        return false;
    }

    UserInstall::UserInstallError UserInstall::finalize()
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
                    return E_None;
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
        FileStatus aFileStatus(FileStatusMask_All);
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
                        newDstUnqPath += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
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

    static const char *pszSrcList[] = {
        "/presets",
        NULL
    };
    static const char *pszDstList[] = {
        "/user",
        NULL
    };


    static UserInstall::UserInstallError create_user_install(OUString& aUserPath)
    {
        OUString aBasePath;
        if (utl::Bootstrap::locateBaseInstallation(aBasePath) != utl::Bootstrap::PATH_EXISTS)
            return UserInstall::E_InvalidBaseinstall;

        // create the user directory
        FileBase::RC rc = Directory::createPath(aUserPath);
        if ((rc != FileBase::E_None) && (rc != FileBase::E_EXIST)) return UserInstall::E_Creation;

#ifdef UNIX
    // set safer permissions for the user directory by default
    File::setAttributes(aUserPath, Attribute_OwnWrite| Attribute_OwnRead| Attribute_OwnExe);
#endif

            // copy data from shared data directory of base installation
        for (sal_Int32 i=0; pszSrcList[i]!=NULL && pszDstList[i]!=NULL; i++)
        {
            rc = copy_recursive(
                    aBasePath + OUString::createFromAscii(pszSrcList[i]),
                    aUserPath + OUString::createFromAscii(pszDstList[i]));
            if ((rc != FileBase::E_None) && (rc != FileBase::E_EXIST))
            {
                if ( rc == FileBase::E_NOSPC )
                    return UserInstall::E_NoDiskSpace;
                else if ( rc == FileBase::E_ACCES )
                    return UserInstall::E_NoWriteAccess;
                else
                    return UserInstall::E_Creation;
            }
        }
        try
        {
            OUString sConfigSrvc(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationProvider"));
            OUString sAccessSrvc(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationUpdateAccess"));

            // get configuration provider
            Reference< XMultiServiceFactory > theMSF = comphelper::getProcessServiceFactory();
            Reference< XMultiServiceFactory > theConfigProvider = Reference< XMultiServiceFactory >(
                theMSF->createInstance(sConfigSrvc), UNO_QUERY_THROW);
            Sequence< Any > theArgs(1);
            NamedValue v(OUString(RTL_CONSTASCII_USTRINGPARAM("NodePath")), makeAny(OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Setup"))));
            theArgs[0] <<= v;
            Reference< XHierarchicalPropertySet> hpset(
                theConfigProvider->createInstanceWithArguments(sAccessSrvc, theArgs), UNO_QUERY_THROW);
            hpset->setHierarchicalPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Office/ooSetupInstCompleted")), makeAny(sal_True));
            Reference< XChangesBatch >(hpset, UNO_QUERY_THROW)->commitChanges();
        }
        catch ( PropertyVetoException& )
        {
            // we are not allowed to change this
        }
        catch (Exception& e)
        {
            OString aMsg("create_user_install(): ");
            aMsg += OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US);
            OSL_ENSURE(sal_False, aMsg.getStr());
            return UserInstall::E_Creation;
        }

        return UserInstall::E_None;

    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: userinstall.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: hr $ $Date: 2005-10-24 18:34:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#include "userinstall.hxx"
#include "langselect.hxx"

#include <stdio.h>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _VOS_SECURITY_HXX_
#include <vos/security.hxx>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif

#ifndef _TOOLS_RESMGR_HXX_
#include <tools/resmgr.hxx>
#endif

#ifndef _UTL_BOOTSTRAP_HXX
#include <unotools/bootstrap.hxx>
#endif

#ifndef _SVTOOLS_LANGUAGEOPTIONS_HXX
#include <svtools/languageoptions.hxx>
#endif
#ifndef _SVTOOLS_SYSLOCALEOPTIONSOPTIONS_HXX
#include <svtools/syslocaleoptions.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <tools/isolang.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/beans/XHierarchicalPropertySet.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XLocalizable.hpp>
#include <com/sun/star/lang/Locale.hpp>

#include "app.hxx"

using namespace rtl;
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
//            Reference< XLocalizable > localizable(theConfigProvider, UNO_QUERY_THROW);
//            LanguageType aUserLanguageType = LanguageSelection::getLanguageType();
//            OUString aUserLanguage = ConvertLanguageToIsoString(aUserLanguageType);
//            Locale aLocale = LanguageSelection::IsoStringToLocale(aUserLanguage);
//            localizable->setLocale(aLocale);

            Reference< XLocalizable > localizable(theConfigProvider, UNO_QUERY_THROW);
            OUString aUserLanguage = LanguageSelection::getLanguageString();
            Locale aLocale = LanguageSelection::IsoStringToLocale(aUserLanguage);
            localizable->setLocale(aLocale);

            Sequence< Any > theArgs(1);
            NamedValue v;
            v.Name = OUString::createFromAscii("NodePath");
            v.Value = makeAny(OUString::createFromAscii("org.openoffice.Setup"));
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
        Bootstrap::PathStatus aLocateResult =
            Bootstrap::locateUserInstallation(aUserInstallPath);

        switch (aLocateResult) {

            case Bootstrap::DATA_INVALID:
            case Bootstrap::DATA_MISSING:
            case Bootstrap::DATA_UNKNOWN:
                // cannot find a valid path or path is missing
                return E_Unknown;

            case Bootstrap::PATH_EXISTS:
            {
                // path exists, check if an installation lives there
                if ( is_user_install() )
                {
                    return E_None;
                }
                // Note: fall-thru intended.
            }
            case Bootstrap::PATH_VALID:
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
                        newDstUnqPath += rtl::OUString::createFromAscii("/");
                    newDstUnqPath += itemname;
                    // recursion
                    err = copy_recursive(newSrcUnqPath, newDstUnqPath);
                }
                aDir.close();
                if( next != FileBase::E_NOENT ) err = FileBase::E_INVAL;
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
        if (Bootstrap::locateBaseInstallation(aBasePath) != Bootstrap::PATH_EXISTS)
            return UserInstall::E_InvalidBaseinstall;

        // create the user directory
        FileBase::RC rc = Directory::create(aUserPath);
        if ((rc != FileBase::E_None) && (rc != FileBase::E_EXIST)) return UserInstall::E_Creation;

            // copy data from shared data directory of base installation
        for (sal_Int32 i=0; pszSrcList[i]!=NULL && pszDstList[i]!=NULL; i++)
        {
            rc = copy_recursive(
                    aBasePath + OUString::createFromAscii(pszSrcList[i]),
                    aUserPath + OUString::createFromAscii(pszDstList[i]));
            if ((rc != FileBase::E_None) && (rc != FileBase::E_EXIST)) return UserInstall::E_Creation;
        }
        try
        {
            OUString sConfigSrvc = OUString::createFromAscii("com.sun.star.configuration.ConfigurationProvider");
            OUString sAccessSrvc = OUString::createFromAscii("com.sun.star.configuration.ConfigurationUpdateAccess");

            // get configuration provider
            Reference< XMultiServiceFactory > theMSF = comphelper::getProcessServiceFactory();
            Reference< XMultiServiceFactory > theConfigProvider = Reference< XMultiServiceFactory >(
                theMSF->createInstance(sConfigSrvc), UNO_QUERY_THROW);
            Sequence< Any > theArgs(1);
            NamedValue v(OUString::createFromAscii("NodePath"), makeAny(OUString::createFromAscii("org.openoffice.Setup")));
            //v.Name = OUString::createFromAscii("NodePath");
            //v.Value = makeAny(OUString::createFromAscii("org.openoffice.Setup"));
            theArgs[0] <<= v;
            Reference< XHierarchicalPropertySet> hpset(
                theConfigProvider->createInstanceWithArguments(sAccessSrvc, theArgs), UNO_QUERY_THROW);
            hpset->setHierarchicalPropertyValue(OUString::createFromAscii("Office/ooSetupInstCompleted"), makeAny(sal_True));
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

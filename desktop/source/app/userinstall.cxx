/*************************************************************************
 *
 *  $RCSfile: userinstall.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-07 14:52:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include "userinstall.hxx"
#include "langselect.hxx"

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

using namespace rtl;
using namespace osl;
using namespace utl;

namespace desktop {

    static UserInstall::UserInstallError create_user_install(OUString&);

#ifdef UNX
    static const char *szReadme = "/README";
#else
    static const char *szReadme = "/readme.txt";
#endif

    UserInstall::UserInstallError UserInstall::finalize()
    {
        OUString aUserInstallPath;
        Bootstrap::PathStatus aLocateResult =
            Bootstrap::locateUserInstallation(aUserInstallPath);
        OUString aReadme = aUserInstallPath + (OUString::createFromAscii(szReadme));
        File aReadmeFile(aReadme);

        switch (aLocateResult) {

            case Bootstrap::DATA_INVALID:
            case Bootstrap::DATA_MISSING:
            case Bootstrap::DATA_UNKNOWN:
                // cannot find a valid path or path is missing
                return E_Unknown;
                break;
            case Bootstrap::PATH_EXISTS:
                // path exists, check if an installation lives there
                if (aReadmeFile.open(0) == FileBase::E_None) return E_None;
            case Bootstrap::PATH_VALID:
                // found a path but need to create user install
                    return create_user_install(aUserInstallPath);
                break;
            default:
                return E_Unknown;
        }
    }

    // get the installation language for this user:
    LanguageType UserInstall::getUserInstallLanguage()
    {
        static sal_Bool bLanguageFound = sal_False;
        static LanguageType aLanguage = LANGUAGE_DONTKNOW;

        if (bLanguageFound)
            return aLanguage;

        LanguageSelection l;
        aLanguage = l.Execute();
        bLanguageFound = sal_True;
        return aLanguage;

    }

    // run a setup in response file mode to do tasks that are not
    // handled inside the office yet
    static UserInstall::UserInstallError run_hidden_setup(OUString& aDestinationURL)
    {
        // get commandline for setup
        ::vos::OStartupInfo aInfo;
        OUString aProgName;
        OUString aDir;
        aInfo.getExecutableFile( aProgName );
        sal_uInt32 lastIndex = aProgName.lastIndexOf('/');
        if ( lastIndex > 0 )
        {
            aProgName = aProgName.copy( 0, lastIndex+1 );
            aDir      = aProgName;
            aProgName += OUString( RTL_CONSTASCII_USTRINGPARAM( "setup" ));
    #ifdef WNT
            aProgName += OUString( RTL_CONSTASCII_USTRINGPARAM( ".exe" ));
    #endif
        }

        /* prepare command line parameters and environment
        -l selected language
        -d destination directory
        -r responsefile
        -nocopy  (no files and directories are created from the setup)
        */
        LanguageType aLanguage = UserInstall::getUserInstallLanguage();
        OUString aDestinationPath;
        FileBase::getSystemPathFromFileURL(aDestinationURL, aDestinationPath);
        OUString aResponsefileURL;
        Bootstrap::locateSharedData(aResponsefileURL);
        aResponsefileURL += OUString::createFromAscii("/response/response_workstation.txt");
        OUString aResponsefilePath;
        FileBase::getSystemPathFromFileURL(aResponsefileURL, aResponsefilePath);
        OUString aArgListArray[] = {
            OUString::createFromAscii("-l"), OUString::createFromAscii(ResMgr::GetLang(aLanguage, 0)),
            OUString::createFromAscii("-d"), aDestinationPath,
            OUString::createFromAscii("-r"), aResponsefilePath,
            OUString::createFromAscii("-noruncheck")
            //,OUString::createFromAscii("-nocopy")
        };

        vos::OSecurity aSecurity;
        vos::OEnvironment aEnv;
        vos::OArgumentList aArgumentList( aArgListArray, 7 );
        //vos::OArgumentList aArgumentList( aArgListArray, 7 );

        // launch the setup process
        vos::OProcess aProcess( aProgName, aDir );
        vos::OProcess::TProcessError aProcessError = aProcess.execute(
                vos::OProcess::TOption_Wait, aSecurity, aArgumentList, aEnv );

        // check the result...
        if ( aProcessError != vos::OProcess::E_None )
        {
            // an error occured...
            return UserInstall::E_SetupFailed;
        } else {
            return UserInstall::E_None;
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
#ifdef UNX
    static const char *pszCopyList[] = {
        "/user",
        "/README",
        "/README.html",
        "/THIRDPARTYLICENSEREADME.html",
        NULL
    };
#else
    static const char *pszCopyList[] = {
//        "/user",
        "/readme.txt",
        "/readme.html",
//        "/THIRDPARTYLICENSEREADME.html",
        NULL
    };
#endif

    static UserInstall::UserInstallError create_user_install(OUString& aUserPath)
    {
        OUString aBasePath;
        if (Bootstrap::locateBaseInstallation(aBasePath) == Bootstrap::PATH_EXISTS)
        {
            // create the user directory
            FileBase::RC rc = Directory::create(aUserPath);
            if ((rc != FileBase::E_None) && (rc != FileBase::E_EXIST)) return UserInstall::E_Creation;

            // copy data from shared data directory of base installation
            for (sal_Int32 i=0; pszCopyList[i]!=NULL; i++)
            {
                rc = copy_recursive(
                    aBasePath + OUString::createFromAscii(pszCopyList[i]),
                    aUserPath + OUString::createFromAscii(pszCopyList[i]));
                if ((rc != FileBase::E_None) && (rc != FileBase::E_EXIST)) return UserInstall::E_Creation;
            }

            // do tasks that still need setup
            UserInstall::UserInstallError err = run_hidden_setup(aUserPath);
            if (err == UserInstall::E_None) {
                return UserInstall::E_None;
            }
            else {
                // XXX rollback?
                return err;
            }
        }
        else
        {
            // cannot locate base installation
            return UserInstall::E_InvalidBaseinstall;
        }
    }
}

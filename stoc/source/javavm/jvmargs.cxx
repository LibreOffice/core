/*************************************************************************
 *
 *  $RCSfile: jvmargs.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-01 09:04:23 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef UNIX
#define CLASSPATH_DELIMETER ":"

#else
#define CLASSPATH_DELIMETER ";"

#endif


#include <string.h>

#include "jvmargs.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "rtl/bootstrap.hxx"
#include <osl/diagnose.h>
#include <osl/thread.h>
#include "osl/file.h"
#include "osl/file.hxx"

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#define OUSTR(x) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( x ))

using namespace rtl;
namespace css = com::sun::star;
namespace stoc_javavm {

JVM::JVM() throw()//: _enabled(sal_False)
{
}

void JVM::pushProp(const OUString & property)
{
    sal_Int32 index = property.indexOf((sal_Unicode)'=');
    if(index > 0)
    {
        OUString left = property.copy(0, index).trim();
        OUString right(property.copy(index + 1).trim());

//         if(left.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("Enable"))))
//         {
//             setEnabled(right.toInt32());
//         }
//        else
            if(left.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("Version"))))
        {
            setVersion(right);
        }
        else if(left.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("Home"))))
        {
            setJavaHome(right);
        }
        else if (left.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("ApplicationClassesDir"))))
        {
            setApplicationClassesDir(right);
        }
        else if(left.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("RuntimeLib"))))
        {
            setRuntimeLib(right);
        }
        else
            _props.push_back(property);
    }
    else
    { // no '=', could be -X
        _props.push_back(property);
    }
}

void JVM::setApplicationClassesDir(const rtl::OUString & sDir)
{
    rtl::OUString sPaths = buildClassPathFromDirectory(sDir);
    addClassPath(sPaths);
}
/** The path has the form: foo/bar
    That is, separator is a slash.
    @exception com::sun::star::uno::Exception
 */
rtl::OUString JVM::buildClassPathFromDirectory(const rtl::OUString & relPath)
{
    rtl::OUString sInstallDir;
    rtl::Bootstrap::get(OUSTR("BaseInstallation"),
                        sInstallDir,
                        OUSTR("${$SYSBINDIR/" SAL_CONFIGFILE("bootstrap") ":BaseInstallation}"));
    rtl::OUString sClassesDir = sInstallDir +  OUSTR("/") + relPath;

    osl::Directory dir(sClassesDir);
    osl::FileBase::RC fileErrorCode;
    if ((fileErrorCode = dir.open()) != osl::FileBase::E_None)
//    if ((fileErrorCode = dir.open()) != osl::FileBase::RC::E_None)
    {
        rtl::OUString sMsg(OUSTR("[javavm.cxx]File operation failed. Error: ")
                           + rtl::OUString::valueOf((sal_Int32) fileErrorCode));
        throw css::uno::Exception(sMsg, 0);
    }

    osl::DirectoryItem dirItem;
    rtl::OUStringBuffer sBuffer(2048);
    char szSep[] = {SAL_PATHSEPARATOR,0};

    //insert the path to the directory, so that .class files can be found
    rtl::OUString sDirPath;
    if ((fileErrorCode = osl::FileBase::getSystemPathFromFileURL(
             sClassesDir, sDirPath))
        != osl::FileBase::E_None)
    {
        rtl::OUString sMsg(OUSTR("[javavm.cxx]File operation failed. Error: ")
                           + rtl::OUString::valueOf((sal_Int32) fileErrorCode));
        throw css::uno::Exception(sMsg, 0);
    }
    sBuffer.append(sDirPath);
    sBuffer.appendAscii(szSep);

    rtl::OUString sJarExtension(OUSTR(".jar"));
    sal_Int32 nJarExtensionLength = sJarExtension.getLength();
    for(;;)
    {
        fileErrorCode = dir.getNextItem(dirItem);
        if (fileErrorCode == osl::FileBase::E_None)
        {
            osl::FileStatus stat(FileStatusMask_All);
            if ((fileErrorCode = dirItem.getFileStatus(stat)) !=
                osl::FileBase::E_None)
            {
                rtl::OUString sMsg(OUSTR("[javavm.cxx]File operation failed. Error: ")
                                   + rtl::OUString::valueOf(
                                       (sal_Int32) fileErrorCode));
                throw css::uno::Exception(sMsg, 0);
            }
            // check if the item is a file.
            switch (stat.getFileType())
            {
            case osl::FileStatus::Regular:
                break;
            case osl::FileStatus::Link:
            {
                rtl::OUString sLinkURL = stat.getLinkTargetURL();
                osl::DirectoryItem itemLink;
                if (osl::DirectoryItem::get(sLinkURL, itemLink)
                    != osl::FileBase::E_None)
                {
                    throw css::uno::Exception();
                }
                osl::FileStatus statLink(FileStatusMask_All);
                if (statLink.getFileType() != osl::FileStatus::Regular)
                    continue;
                //ToDo check if the link is also a regular file:
                break;
            }
            default:
                continue;
            }

            //check if the file is a .jar, class files are ignored
            rtl::OUString sFileName = stat.getFileName();
            sal_Int32 len = sFileName.getLength();
            sal_Int32 nIndex = sFileName.lastIndexOf(sJarExtension);
            if ((nIndex == -1)
                || (nIndex + nJarExtensionLength != len))
                continue;

            rtl::OUString sFileURL = stat.getFileURL();
            rtl::OUString sFilePath;
            if ((fileErrorCode = osl::FileBase::getSystemPathFromFileURL(
                     sFileURL, sFilePath))
                != osl::FileBase::E_None)
            {
                rtl::OUString sMsg(OUSTR("[javavm.cxx]File operation failed. Error: ")
                                   + rtl::OUString::valueOf(
                                       (sal_Int32) fileErrorCode));
                throw css::uno::Exception(sMsg, 0);
            }
            sBuffer.append(sFilePath);
            sBuffer.appendAscii(szSep);
        }
        else if (fileErrorCode == osl::FileBase::E_NOENT)
        {
            break;
        }
        else
        {
            rtl::OUString sMsg(OUSTR("File operation failed. Error:")
                               + rtl::OUString::valueOf(
                                   (sal_Int32) fileErrorCode));
            throw css::uno::Exception();
        }
    }
    return sBuffer.makeStringAndClear();
}

void JVM::setVersion(const rtl::OUString & sVersion)
{
    _version = sVersion;
}

const ::rtl::OUString & JVM::getVersion() const
{
    return _version;
}

void JVM::setJavaHome(const ::rtl::OUString & sHomeUrl)
{
    // we need a system path and not a file URL
    OUString usSysPath;
    oslFileError er= osl_getSystemPathFromFileURL( sHomeUrl.pData, &usSysPath.pData);
    _java_home = usSysPath;
}

// void JVM::setEnabled(sal_Bool sbFlag)
// {
//     _enabled = sbFlag;
// }

void JVM::addClassPath(const OUString & classpath)
{
    if( classpath.getLength() )
    {
        OUStringBuffer buf(_classpath.getLength() +1 + classpath.getLength());
        if( _classpath.getLength() )
        {
            buf.append( _classpath );
            buf.appendAscii( CLASSPATH_DELIMETER );
        }
        buf.append( classpath );
        _classpath = buf.makeStringAndClear();
    }
}


void JVM::setRuntimeLib(const OUString & libName)
{
    _runtimeLib = libName;
}

const OUString & JVM::getRuntimeLib() const
{
    return _runtimeLib;
}

// sal_Bool JVM::isEnabled() const
// {
//     return _enabled;
// }

rtl::OUString JVM::getClassPath() const
{
    return _classpath;
}

const ::std::vector< ::rtl::OUString > & JVM::getProperties() const
{
    return _props;
}

const OUString& JVM::getJavaHome() const
{
        return _java_home;
}
}

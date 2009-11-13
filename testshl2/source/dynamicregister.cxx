/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dynamicregister.cxx,v $
 * $Revision: 1.11 $
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
#include "precompiled_testshl2.hxx"

#include "testshl/dynamicregister.hxx"
#include <osl/process.h>
// #include <osl/mutex.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include "testshl/filehelper.hxx"

#include <unistd.h>

#if defined(WIN32)
#include <direct.h>    /* w.g. _chdir() */
#endif

namespace fixes
{
    int changedir(const char* _sPath)
    {
#if defined(WIN32)
        // chdir(_sPath) is marked depricated since Visual C++ 2005
        // use _chdir instead
        return ::_chdir(_sPath);
#else
        return ::chdir(_sPath);
#endif
    }
}

// -----------------------------------------------------------------------------

DynamicLibraryHelper::DynamicLibraryHelper(rtl::OUString const& _suDLLName, GetOpt & _aOptions)
        :m_pModule(new ::osl::Module()),
         m_suDLLName(_suDLLName),
         m_aOptions(_aOptions)
{
    // create and load the module (shared library)
    m_suAbsolutePathFile = FileHelper::convertPath( _suDLLName );

    // due to some problems on mac OS
    // we split the absolute pathname to path and filename
    // change to the path and load the filename direct
    // then change back to the old path.
    rtl::OUString suPathSeparator = rtl::OUString( rtl::OUString::createFromAscii("/"));
    sal_Int32 nPos = m_suAbsolutePathFile.lastIndexOf(suPathSeparator);
    if (nPos != -1)
    {
        m_suAbsolutePath = m_suAbsolutePathFile.copy(0, nPos);
        m_suFilename = m_suAbsolutePathFile.copy(nPos + 1);
    }
    else
    {
        // Should never happen.
        rtl::OString sPath = rtl::OUStringToOString(m_suAbsolutePathFile, RTL_TEXTENCODING_ASCII_US);
        fprintf(stderr, "There is a problem with path '%s'.\n", sPath.getStr());
        exit(1);
    }

    if (getOptions().hasOpt("-absolutepath"))
    {
        fprintf(stderr, "Hint: Use absolute path to load test library.\n");
        loadLibraryFromAbsolutePath();
    }
    else if (getOptions().hasOpt("-localpath"))
    {
        fprintf(stderr, "Hint: make a chdir() to the test library, then try to load the test library without given path.\n");
        loadLibraryFromLocalPath();
    }
    else
    {

// PLEASE DON'T CHANGE THIS STUPID STRUCTURE, JUST ADD YOUR ENVIRONMENT
#if defined(LINUX)
        loadLibraryFromAbsolutePath();
        // will fail if load local

#elif defined(SOLARIS)
        loadLibraryFromAbsolutePath();
        // will also be right if load local

#elif defined(WIN32)
        loadLibraryFromAbsolutePath();
        // will fail if load local

#elif defined(MACOSX)
        loadLibraryFromLocalPath();
        // will fail if local absolute
#else
        // default is load absolute
        loadLibraryFromAbsolutePath();
#endif
}
}

void DynamicLibraryHelper::showFilenameIfVerbose()
{
    if (getOptions().hasOpt("-verbose"))
    {
        rtl::OString sFilename = rtl::OUStringToOString(m_suFilename, RTL_TEXTENCODING_ASCII_US);
        rtl::OString sPath = rtl::OUStringToOString(m_suAbsolutePath, RTL_TEXTENCODING_ASCII_US);
        fprintf(stderr, "Try to load '%s' from '%s'.\n", sFilename.getStr(), sPath.getStr());

        // check filename
    }
}

void DynamicLibraryHelper::realLoadLibrary(rtl::OUString const& _suLibToLoad)
{
    if (! m_pModule->load(_suLibToLoad, SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL))
    {
        rtl::OString sDLLName = rtl::OUStringToOString(m_suDLLName, RTL_TEXTENCODING_ASCII_US);
        fprintf(stderr, "warning: Can't load module '%s'.\n", sDLLName.getStr());
    }
}

void DynamicLibraryHelper::loadLibraryFromAbsolutePath()
{
    showFilenameIfVerbose();
    realLoadLibrary(m_suAbsolutePathFile);
}

void DynamicLibraryHelper::loadLibraryFromLocalPath()
{
    sal_Int32 nPos;
    rtl::OUString suPathSeparator = rtl::OUString( rtl::OUString::createFromAscii("/"));
#if defined(WIN32)
    suPathSeparator = rtl::OUString( rtl::OUString::createFromAscii("\\"));
#endif
    rtl::OUString suSystemPathFile;
    osl::FileBase::getSystemPathFromFileURL(m_suAbsolutePathFile, suSystemPathFile);

    nPos = suSystemPathFile.lastIndexOf(suPathSeparator);
    rtl::OUString suCurrentDirPath;
    if (nPos != -1)
    {
        // the filename only, no '/' in the path
        rtl::OUString suNewPath = suSystemPathFile.copy(0, nPos );
        if (suNewPath.getLength() > 0)
        {
            rtl::OString sPath = rtl::OUStringToOString(suNewPath, RTL_TEXTENCODING_ASCII_US);
            osl_getProcessWorkingDir( &suCurrentDirPath.pData );

            fixes::changedir(sPath.getStr());

            // curNewDirPath should be suPath, small self test
            rtl::OUString curNewDirPath;
            osl_getProcessWorkingDir( &curNewDirPath.pData );
            if (! curNewDirPath.equals(m_suAbsolutePath))
            {
                fprintf(stderr, "There is a problem with path '%s'.\n", sPath.getStr());
            }
        }
    }

    showFilenameIfVerbose();
    realLoadLibrary(m_suFilename);

    // change back to old directory
    if (suCurrentDirPath.getLength() > 0)
    {
        rtl::OString sCurrentDirPath = rtl::OUStringToOString(suCurrentDirPath, RTL_TEXTENCODING_ASCII_US);
        fixes::changedir(sCurrentDirPath.getStr());
    }
}

DynamicLibraryHelper::~DynamicLibraryHelper()
{
    if (getOptions().hasOpt("-verbose"))
    {
        fprintf(stderr, "Dtor DynamicLibraryHelper.\n");
        fprintf(stderr, "Delete loaded module.");
    }
    delete m_pModule;
    if (getOptions().hasOpt("-verbose"))
    {
        fprintf(stderr, " [done].\n");
    }
}


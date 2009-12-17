/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 * $Revision$
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
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

#ifndef DYNAMICREGISTER_HXX
#define DYNAMICREGISTER_HXX

#include <rtl/ustring.hxx>
#include <osl/module.hxx>

#include "testshl/getopt.hxx"
#include "testshl/nocopy.hxx"

// -------------------------------------------------

class DynamicLibraryHelper : NOCOPY
{
protected:
    osl::Module*  m_pModule;
    rtl::OUString m_suDLLName;
    GetOpt & m_aOptions;

    GetOpt& getOptions() {return m_aOptions;}

public:
    DynamicLibraryHelper(rtl::OUString const& _sDLLName, GetOpt & _aOptions);
    virtual ~DynamicLibraryHelper();

private:
    void showFilenameIfVerbose();
    void realLoadLibrary(rtl::OUString const& _sLibToLoad);
    void loadLibraryFromAbsolutePath();
    void loadLibraryFromLocalPath();

    rtl::OUString m_suAbsolutePathFile;  // file:///D:/foo/bar/library.dll
    rtl::OUString m_suAbsolutePath;      // file:///D:/foo/bar
    rtl::OUString m_suFilename;          // library.dll
};

#endif


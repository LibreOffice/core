/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dynamicregister.cxx,v $
 * $Revision: 1.9 $
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

#include "dynamicregister.hxx"
#include <osl/process.h>
// #include <osl/mutex.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include "filehelper.hxx"

// -----------------------------------------------------------------------------

DynamicLibraryHelper::DynamicLibraryHelper(rtl::OUString const& _sDLLName, GetOpt & _aOptions)
        :m_pModule(new ::osl::Module()),
         m_suDLLName(_sDLLName),
         m_aOptions(_aOptions)
{
    // create and load the module (shared library)
    rtl::OUString suFile = FileHelper::convertPath( _sDLLName );
    rtl::OString sDLLName = rtl::OUStringToOString(suFile, RTL_TEXTENCODING_ASCII_US);
    if (_aOptions.hasOpt("-verbose"))
    {
        fprintf(stderr, "Try to load '%s'.\n", sDLLName.getStr());
    }

    if (! m_pModule->load(suFile, SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL))
    {
        sDLLName = rtl::OUStringToOString(_sDLLName, RTL_TEXTENCODING_ASCII_US);
        fprintf(stderr, "warning: Can't load module '%s'.\n", sDLLName.getStr());
    }
}

DynamicLibraryHelper::~DynamicLibraryHelper()
{
    delete m_pModule;
}


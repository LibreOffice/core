/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dynamicregister.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:27:49 $
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

#include "dynamicregister.hxx"
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
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


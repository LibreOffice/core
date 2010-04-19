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
#include "precompiled_testshl2.hxx"

#include <vector>
#include <stdio.h>
#include "registerfunc.h"
#include <osl/module.hxx>
#include <osl/process.h>
#include <osl/file.hxx>

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

typedef std::vector<FktPtr> FunctionList;
FunctionList m_Functions;

extern "C" bool SAL_CALL registerFunc(FktPtr _pFunc, const char* _sFuncName)
{
   printf("Function register call for func(%s) successful.\n", _sFuncName);
   m_Functions.push_back(_pFunc);
   // FktPtr pFunc = _pFunc;
   // if (_pFunc)
   // {
   //    (_pFunc)();
   // }

   return true;
}

void callAll()
{
   for(FunctionList::const_iterator it = m_Functions.begin();
       it != m_Functions.end();
       ++it)
   {
      FktPtr pFunc = *it;
      if (pFunc)
      {
         (pFunc)();
      }
   }
}

// void test_link_at_compiletime()
// {
//     FktRegFuncPtr pRegisterFunc = &registerFunc;
//     registerAll(pRegisterFunc);
//     callAll();
// }


// -----------------------------------------------------------------------------
rtl::OUString convertPath( rtl::OString const& sysPth )
{
    // PRE: String should contain a filename, relativ or absolut
    rtl::OUString sysPath( rtl::OUString::createFromAscii( sysPth.getStr() ) );
    rtl::OUString fURL;
    if ( sysPth.indexOf("..") == 0 )
    {
        // filepath contains '..' so it's a relative path make it absolut.
        rtl::OUString curDirPth;
        osl_getProcessWorkingDir( &curDirPth.pData );
        osl::FileBase::getAbsoluteFileURL( curDirPth, sysPath, fURL );
    }
    else
    {
        osl::FileBase::getFileURLFromSystemPath( sysPath, fURL );
    }
    return fURL;
}
// -----------------------------------------------------------------------------

void test_link_at_runtime()
{
    ::osl::Module* pModule;
    pModule = new ::osl::Module();
    // create and load the module (shared library)
    // pModule = new ::osl::Module();
#if (defined WNT) || (defined OS2)
    pModule->load( convertPath( rtl::OString( "onefunc_DLL.dll" ) ) );
#endif
#ifdef UNX
    pModule->load( convertPath( rtl::OString( "libonefunc_DLL.so" ) ) );
#endif

    // get entry pointer
    FktRegAllPtr pFunc = (FktRegAllPtr) pModule->getSymbol( rtl::OUString::createFromAscii( "registerAllTestFunction" ) );

    if (pFunc)
    {
        FktRegFuncPtr pRegisterFunc = &registerFunc;
        pFunc(pRegisterFunc);
        callAll();
    }

    delete pModule;
}



// ----------------------------------- Main -----------------------------------
#if (defined UNX) || (defined OS2)
int main( int argc, char* argv[] )
#else
int _cdecl main( int argc, char* argv[] )
#endif
{
    (void) argc;
    (void) argv;
//  test_link_at_compiletime();
    test_link_at_runtime();
    return 0;
}

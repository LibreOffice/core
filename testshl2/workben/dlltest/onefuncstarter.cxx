/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: onefuncstarter.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 10:17:13 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_testshl2.hxx"

#include <vector>
#include <stdio.h>
#include "registerfunc.h"

#ifndef _OSL_MODULE_HXX_
#include <osl/module.hxx>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

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
#ifdef WNT
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

/*************************************************************************
 *
 *  $RCSfile: onefuncstarter.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-08-07 15:08:38 $
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
//  test_link_at_compiletime();
    test_link_at_runtime();
    return 0;
}

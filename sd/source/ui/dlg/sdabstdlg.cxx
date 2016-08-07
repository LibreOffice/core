/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "sdabstdlg.hxx"

#include <osl/module.hxx>
#include <tools/string.hxx>
#include <vcl/unohelp.hxx>

typedef SdAbstractDialogFactory* (__LOADONCALLAPI *SdFuncPtrCreateDialogFactory)();

extern "C" { static void SAL_CALL thisModule() {} }

SdAbstractDialogFactory* SdAbstractDialogFactory::Create()
{
    SdFuncPtrCreateDialogFactory fp = 0;
    static ::osl::Module aDialogLibrary;
    static const ::rtl::OUString sLibName(::vcl::unohelper::CreateLibraryName("sdui", sal_True));
    if ( aDialogLibrary.is() || aDialogLibrary.loadRelative( &thisModule, String( sLibName ) ) )
        fp = ( SdAbstractDialogFactory* (__LOADONCALLAPI*)() )
            aDialogLibrary.getFunctionSymbol( ::rtl::OUString::createFromAscii("CreateDialogFactory") );
    if ( fp )
        return fp();
    return 0;
}



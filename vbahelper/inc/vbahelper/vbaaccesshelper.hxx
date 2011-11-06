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


#ifndef OOVBAAPI_VBA_ACCESS_HELPER_HXX
#define OOVBAAPI_VBA_ACCESS_HELPER_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <basic/basmgr.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
//#define VBAHELPER_DLLIMPLEMENTATION
#include <vbahelper/vbadllapi.h>
#include <memory>
namespace css = ::com::sun::star;
namespace ooo
{
    namespace vba
    {

        VBAHELPER_DLLPRIVATE inline css::uno::Reference< css::lang::XMultiServiceFactory > getVBAServiceFactory( SfxObjectShell* pShell )
        {
            css::uno::Any aUnoVar;
            if ( !pShell || ! pShell->GetBasicManager()->GetGlobalUNOConstant( "VBAGlobals", aUnoVar ) )
                throw css::lang::IllegalArgumentException();
            css::uno::Reference< css::lang::XMultiServiceFactory > xVBAFactory( aUnoVar, css::uno::UNO_QUERY_THROW );
            return xVBAFactory;
        }

        VBAHELPER_DLLPUBLIC inline css::uno::Reference< css::uno::XInterface > createVBAUnoAPIServiceWithArgs( SfxObjectShell* pShell,  const sal_Char* _pAsciiName, const css::uno::Sequence< css::uno::Any >& aArgs ) throw (css::uno::RuntimeException)
        {
            OSL_PRECOND( pShell, "createVBAUnoAPIService: no shell!" );
            ::rtl::OUString sVarName( ::rtl::OUString::createFromAscii( _pAsciiName ) );
            css::uno::Reference< css::uno::XInterface > xIf = getVBAServiceFactory( pShell )->createInstanceWithArguments( sVarName, aArgs  );
            return xIf;
        }


        VBAHELPER_DLLPRIVATE inline bool isAlienDoc( SfxObjectShell& rDocShell, const char* pMimeType )
        {
            bool bRes( false );
            const SfxFilter *pFilt = rDocShell.GetMedium()->GetFilter();
            if ( pFilt && pFilt->IsAlienFormat() )
                bRes = ( pFilt->GetMimeType().CompareToAscii( pMimeType ) == 0 );
            return bRes;
        }
        VBAHELPER_DLLPUBLIC inline bool isAlienExcelDoc( SfxObjectShell& rDocShell ) { return isAlienDoc( rDocShell, "application/vnd.ms-excel" ); }
        VBAHELPER_DLLPUBLIC inline bool isAlienWordDoc( SfxObjectShell& rDocShell ) { return isAlienDoc( rDocShell, "application/vnd.ms-word" ); }

    } // openoffice
} // org

#endif

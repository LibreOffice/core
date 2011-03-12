/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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
        //VBAHELPER_DLLPUBLIC inline bool isAlienWordDoc( SfxObjectShell& rDocShell ) { return isAlienDoc( rDocShell, "application/vnd.ms-word" ); }
        // word seems to return an erroneous mime type :-/ "application/msword"  not consistent with the excel one
        VBAHELPER_DLLPUBLIC inline bool isAlienWordDoc( SfxObjectShell& rDocShell ) { return isAlienDoc( rDocShell, "application/msword" ); }

    } // openoffice
} // org

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

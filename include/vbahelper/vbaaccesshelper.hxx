/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_VBAHELPER_VBAACCESSHELPER_HXX
#define INCLUDED_VBAHELPER_VBAACCESSHELPER_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <basic/basmgr.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
//#define VBAHELPER_DLLIMPLEMENTATION
#include <vbahelper/vbadllapi.h>
#include <memory>

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

        VBAHELPER_DLLPRIVATE inline css::uno::Reference< css::uno::XInterface > createVBAUnoAPIServiceWithArgs( SfxObjectShell* pShell,  const sal_Char* _pAsciiName, const css::uno::Sequence< css::uno::Any >& aArgs ) throw (css::uno::Exception)
        {
            OSL_PRECOND( pShell, "createVBAUnoAPIService: no shell!" );
            OUString sVarName( OUString::createFromAscii( _pAsciiName ) );
            css::uno::Reference< css::uno::XInterface > xIf = getVBAServiceFactory( pShell )->createInstanceWithArguments( sVarName, aArgs  );
            return xIf;
        }


        VBAHELPER_DLLPRIVATE inline bool isAlienDoc( SfxObjectShell& rDocShell, const char* pMimeType )
        {
            bool bRes( false );
            const SfxMedium *pMedium = rDocShell.GetMedium();
            const SfxFilter *pFilt = pMedium ? pMedium->GetFilter() : nullptr;
            if ( pFilt && pFilt->IsAlienFormat() )
                bRes = pFilt->GetMimeType().equalsAscii( pMimeType );
            return bRes;
        }
        VBAHELPER_DLLPRIVATE inline bool isAlienExcelDoc( SfxObjectShell& rDocShell ) { return isAlienDoc( rDocShell, "application/vnd.ms-excel" ); }
        //VBAHELPER_DLLPRIVATE inline bool isAlienWordDoc( SfxObjectShell& rDocShell ) { return isAlienDoc( rDocShell, "application/vnd.ms-word" ); }
        // word seems to return an erroneous mime type :-/ "application/msword"  not consistent with the excel one
        VBAHELPER_DLLPRIVATE inline bool isAlienWordDoc( SfxObjectShell& rDocShell ) { return isAlienDoc( rDocShell, "application/msword" ); }

    } // openoffice
} // org

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

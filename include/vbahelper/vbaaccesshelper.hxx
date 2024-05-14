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

#include <memory>

#include <basic/basmgr.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <osl/diagnose.h>
//#define VBAHELPER_DLLIMPLEMENTATION
#include <rtl/ustring.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>

namespace ooo::vba
{

        inline css::uno::Reference< css::lang::XMultiServiceFactory > getVBAServiceFactory( SfxObjectShell const * pShell )
        {
            css::uno::Any aUnoVar;
            if ( !pShell || ! pShell->GetBasicManager()->GetGlobalUNOConstant( u"VBAGlobals"_ustr, aUnoVar ) )
                throw css::lang::IllegalArgumentException();
            css::uno::Reference< css::lang::XMultiServiceFactory > xVBAFactory( aUnoVar, css::uno::UNO_QUERY_THROW );
            return xVBAFactory;
        }

        /// @throws css::uno::Exception
        inline css::uno::Reference< css::uno::XInterface > createVBAUnoAPIServiceWithArgs( SfxObjectShell const * pShell,  const char* _pAsciiName, const css::uno::Sequence< css::uno::Any >& aArgs )
        {
            OSL_PRECOND( pShell, "createVBAUnoAPIService: no shell!" );
            OUString sVarName( OUString::createFromAscii( _pAsciiName ) );
            css::uno::Reference< css::uno::XInterface > xIf = getVBAServiceFactory( pShell )->createInstanceWithArguments( sVarName, aArgs  );
            return xIf;
        }


        inline bool isAlienDoc( SfxObjectShell const & rDocShell, const char* pMimeType )
        {
            bool bRes( false );
            const SfxMedium *pMedium = rDocShell.GetMedium();
            std::shared_ptr<const SfxFilter> pFilt = pMedium ? pMedium->GetFilter() : nullptr;
            if ( pFilt && pFilt->IsAlienFormat() )
                bRes = pFilt->GetMimeType().equalsAscii( pMimeType );
            return bRes;
        }
        inline bool isAlienExcelDoc( SfxObjectShell const & rDocShell ) { return isAlienDoc( rDocShell, "application/vnd.ms-excel" ); }
        //VBAHELPER_DLLPRIVATE inline bool isAlienWordDoc( SfxObjectShell& rDocShell ) { return isAlienDoc( rDocShell, "application/vnd.ms-word" ); }
        // word seems to return an erroneous mime type :-/ "application/msword"  not consistent with the excel one
        inline bool isAlienWordDoc( SfxObjectShell const & rDocShell ) { return isAlienDoc( rDocShell, "application/msword" ); }

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

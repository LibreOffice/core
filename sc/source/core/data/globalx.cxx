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

#include <callform.hxx>
#include <global.hxx>
#include <osl/diagnose.h>
#include <osl/file.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <ucbhelper/content.hxx>

#include <unotools/pathoptions.hxx>

#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>

#include <com/sun/star/i18n/OrdinalSuffix.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/localedatawrapper.hxx>

namespace com::sun::star::ucb { class XCommandEnvironment; }

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;

void ScGlobal::InitAddIns()
{
    if (utl::ConfigManager::IsFuzzing())
        return;

    // multi paths separated by semicolons
    SvtPathOptions aPathOpt;
    const OUString& aMultiPath = aPathOpt.GetAddinPath();
    if (aMultiPath.isEmpty())
        return;

    sal_Int32 nIdx {0};
    do
    {
        OUString aPath = aMultiPath.getToken(0, ';', nIdx);
        if (aPath.isEmpty())
            continue;

        OUString aUrl;
        if ( osl::FileBase::getFileURLFromSystemPath( aPath, aUrl ) == osl::FileBase::E_None )
            aPath = aUrl;

        INetURLObject aObj;
        aObj.SetSmartURL( aPath );
        aObj.setFinalSlash();
        try
        {
            ::ucbhelper::Content aCnt( aObj.GetMainURL(INetURLObject::DecodeMechanism::NONE),
                Reference< XCommandEnvironment >(),
                comphelper::getProcessComponentContext() );
            Reference< sdbc::XResultSet > xResultSet;
            Sequence< OUString > aProps;
            try
            {
                xResultSet = aCnt.createCursor(
                    aProps, ::ucbhelper::INCLUDE_DOCUMENTS_ONLY );
            }
            catch ( Exception& )
            {
                // ucb may throw different exceptions on failure now
                // no assertion if AddIn directory doesn't exist
            }

            if ( xResultSet.is() )
            {
                Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
                try
                {
                    if ( xResultSet->first() )
                    {
                        do
                        {
                            OUString aId = xContentAccess->queryContentIdentifierString();
                            InitExternalFunc( aId );
                        }
                        while ( xResultSet->next() );
                    }
                }
                catch ( Exception& )
                {
                    TOOLS_WARN_EXCEPTION( "sc", "" );
                }
            }
        }
        catch ( Exception& )
        {
            TOOLS_WARN_EXCEPTION( "sc", "" );
        }
        catch ( ... )
        {
            OSL_FAIL( "unexpected exception caught!" );
        }
    }
    while (nIdx>0);
}

OUString ScGlobal::GetOrdinalSuffix( sal_Int32 nNumber)
{
    try
    {
        if (!xOrdinalSuffix.is())
        {
            xOrdinalSuffix = i18n::OrdinalSuffix::create( ::comphelper::getProcessComponentContext() );
        }
        uno::Sequence< OUString > aSuffixes = xOrdinalSuffix->getOrdinalSuffix( nNumber,
                ScGlobal::getLocaleDataPtr()->getLanguageTag().getLocale());
        if ( aSuffixes.hasElements() )
            return aSuffixes[0];
        else
            return OUString();
    }
    catch ( Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sc", "GetOrdinalSuffix: exception caught during init" );
    }
    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

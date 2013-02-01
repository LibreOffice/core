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

#include "callform.hxx"
#include "global.hxx"
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/localfilehelper.hxx>

#include <unotools/pathoptions.hxx>

#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>

#include <com/sun/star/i18n/OrdinalSuffix.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <unotools/localedatawrapper.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;


void ScGlobal::InitAddIns()
{
    // multi paths separated by semicolons
    SvtPathOptions aPathOpt;
    rtl::OUString aMultiPath = aPathOpt.GetAddinPath();
    if (aMultiPath.isEmpty())
        return;

    sal_Int32 nTokens = comphelper::string::getTokenCount(aMultiPath, ';');
    for (sal_Int32 j = 0; j < nTokens; ++j)
    {
        rtl::OUString aPath = comphelper::string::getToken(aMultiPath, j, ';');
        if (aPath.isEmpty())
            continue;

        //  use LocalFileHelper to convert the path to a URL that always points
        //  to the file on the server
        rtl::OUString aUrl;
        if ( utl::LocalFileHelper::ConvertPhysicalNameToURL( aPath, aUrl ) )
            aPath = aUrl;

        INetURLObject aObj;
        aObj.SetSmartURL( aPath );
        aObj.setFinalSlash();
        try
        {
            ::ucbhelper::Content aCnt( aObj.GetMainURL(INetURLObject::NO_DECODE),
                Reference< XCommandEnvironment >(),
                comphelper::getProcessComponentContext() );
            Reference< sdbc::XResultSet > xResultSet;
            Sequence< rtl::OUString > aProps;
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
                Reference< sdbc::XRow > xRow( xResultSet, UNO_QUERY );
                Reference< XContentAccess >
                    xContentAccess( xResultSet, UNO_QUERY );
                try
                {
                    if ( xResultSet->first() )
                    {
                        do
                        {
                            rtl::OUString aId = xContentAccess->queryContentIdentifierString();
                            InitExternalFunc( aId );
                        }
                        while ( xResultSet->next() );
                    }
                }
                catch ( Exception& )
                {
                    OSL_FAIL( "ResultSetException caught!" );
                }
            }
        }
        catch ( Exception& )
        {
            OSL_FAIL( "Exception caught!" );
        }
        catch ( ... )
        {
            OSL_FAIL( "unexpected exception caught!" );
        }
    }
}


String ScGlobal::GetOrdinalSuffix( sal_Int32 nNumber)
{
    try
    {
        if (!xOrdinalSuffix.is())
        {
            xOrdinalSuffix = i18n::OrdinalSuffix::create( ::comphelper::getProcessComponentContext() );
        }
        uno::Sequence< rtl::OUString > aSuffixes = xOrdinalSuffix->getOrdinalSuffix( nNumber,
                ScGlobal::pLocaleData->getLanguageTag().getLocale());
        if ( aSuffixes.getLength() > 0 )
            return aSuffixes[0];
        else
            return String();
    }
    catch ( Exception& )
    {
        OSL_FAIL( "GetOrdinalSuffix: exception caught during init" );
    }
    return String();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

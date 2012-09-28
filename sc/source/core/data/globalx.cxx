/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <com/sun/star/i18n/XOrdinalSuffix.hpp>
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
    if (!xOrdinalSuffix.is())
    {
        try
        {
            Reference< lang::XMultiServiceFactory > xServiceManager =
                ::comphelper::getProcessServiceFactory();
            Reference< XInterface > xInterface =
                xServiceManager->createInstance(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.i18n.OrdinalSuffix")));
            if  (xInterface.is())
                xOrdinalSuffix = Reference< i18n::XOrdinalSuffix >( xInterface, UNO_QUERY);
        }
        catch ( Exception& )
        {
            OSL_FAIL( "GetOrdinalSuffix: exception caught during init" );
        }
    }
    OSL_ENSURE( xOrdinalSuffix.is(), "GetOrdinalSuffix: createInstance failed");
    if (xOrdinalSuffix.is())
    {
        try
        {
            uno::Sequence< rtl::OUString > aSuffixes = xOrdinalSuffix->getOrdinalSuffix( nNumber,
                    ScGlobal::pLocaleData->getLocale());
            if ( aSuffixes.getLength() > 0 )
                return aSuffixes[0];
            else
                return String();
        }
        catch ( Exception& )
        {
            OSL_FAIL( "GetOrdinalSuffix: exception caught during getOrdinalSuffix" );
        }
    }
    return String();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

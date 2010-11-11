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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"


#include "callform.hxx"
#include "global.hxx"
#include <tools/urlobj.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/localfilehelper.hxx>

#include <tools/debug.hxx>
#include <unotools/pathoptions.hxx>

#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>

#include <com/sun/star/i18n/XOrdinalSuffix.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/localedatawrapper.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;


// static
void ScGlobal::InitAddIns()
{
    // multi paths separated by semicolons
    SvtPathOptions aPathOpt;
    String aMultiPath = aPathOpt.GetAddinPath();
    if ( aMultiPath.Len() > 0 )
    {
        xub_StrLen nTokens = aMultiPath.GetTokenCount( ';' );
        xub_StrLen nIndex = 0;
        for ( xub_StrLen j=0; j<nTokens; j++ )
        {
            String aPath( aMultiPath.GetToken( 0, ';', nIndex ) );
            if ( aPath.Len() > 0 )
            {
                //  use LocalFileHelper to convert the path to a URL that always points
                //  to the file on the server
                String aUrl;
                if ( utl::LocalFileHelper::ConvertPhysicalNameToURL( aPath, aUrl ) )
                    aPath = aUrl;

                INetURLObject aObj;
                aObj.SetSmartURL( aPath );
                aObj.setFinalSlash();
                try
                {
                    ::ucbhelper::Content aCnt( aObj.GetMainURL(INetURLObject::NO_DECODE),
                        Reference< XCommandEnvironment > () );
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
                                    rtl::OUString aId( xContentAccess->queryContentIdentifierString() );
                                    InitExternalFunc( aId );
                                }
                                while ( xResultSet->next() );
                            }
                        }
                        catch ( Exception& )
                        {
                            DBG_ERRORFILE( "ResultSetException catched!" );
                        }
                    }
                }
                catch ( Exception& )
                {
                    DBG_ERRORFILE( "Exception catched!" );
                }
                catch ( ... )
                {

                    DBG_ERRORFILE( "unexpected exception caught!" );
                }
            }
        }
    }
}


// static
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
            DBG_ERRORFILE( "GetOrdinalSuffix: exception caught during init" );
        }
    }
    DBG_ASSERT( xOrdinalSuffix.is(), "GetOrdinalSuffix: createInstance failed");
    if (xOrdinalSuffix.is())
    {
        try
        {
            return xOrdinalSuffix->getOrdinalSuffix( nNumber,
                    ScGlobal::pLocaleData->getLocale());
        }
        catch ( Exception& )
        {
            DBG_ERRORFILE( "GetOrdinalSuffix: exception caught during getOrdinalSuffix" );
        }
    }
    return String();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

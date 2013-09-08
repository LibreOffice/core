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

#include <services/urltransformer.hxx>
#include <threadhelp/resetableguard.hxx>
#include <macros/debug.hxx>
#include <services.h>

#include <tools/urlobj.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>

namespace framework{

using namespace ::osl                           ;
using namespace ::cppu                          ;
using namespace ::com::sun::star::uno           ;
using namespace ::com::sun::star::lang          ;
using namespace ::com::sun::star::util          ;

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
URLTransformer::URLTransformer( const Reference< XMultiServiceFactory >& /*xFactory*/ )
{
    // Safe impossible cases.
    // Method not defined for all incoming parameter.
    //SAL_WARN_IF( !xFactory.is(), "fwk", "URLTransformer::URLTransformer(): Invalid parameter detected!" );
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
URLTransformer::~URLTransformer()
{
}

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************

DEFINE_XSERVICEINFO_MULTISERVICE    (   URLTransformer                      ,
                                        OWeakObject                         ,
                                        DECLARE_ASCII("com.sun.star.util.URLTransformer"),
                                        IMPLEMENTATIONNAME_URLTRANSFORMER
                                    )

DEFINE_INIT_SERVICE                 (   URLTransformer,
                                        {
                                        }
                                    )

namespace
{
    void lcl_ParserHelper(INetURLObject& _rParser,URL& _rURL,bool _bUseIntern)
    {
        // Get all information about this URL.
        _rURL.Protocol  = INetURLObject::GetScheme( _rParser.GetProtocol() );
        _rURL.User      = _rParser.GetUser  ( INetURLObject::DECODE_WITH_CHARSET );
        _rURL.Password  = _rParser.GetPass  ( INetURLObject::DECODE_WITH_CHARSET );
        _rURL.Server        = _rParser.GetHost  ( INetURLObject::DECODE_WITH_CHARSET );
        _rURL.Port      = (sal_Int16)_rParser.GetPort();

        sal_Int32 nCount = _rParser.getSegmentCount( false );
        if ( nCount > 0 )
        {
            // Don't add last segment as it is the name!
            --nCount;

            OUStringBuffer aPath;
            for ( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
            {
                aPath.append( sal_Unicode( '/' ));
                aPath.append( _rParser.getName( nIndex, false, INetURLObject::NO_DECODE ));
            }

            if ( nCount > 0 )
                aPath.append( sal_Unicode( '/' )); // final slash!

            _rURL.Path = aPath.makeStringAndClear();
            _rURL.Name = _rParser.getName( INetURLObject::LAST_SEGMENT, false, INetURLObject::NO_DECODE );
        }
        else
        {
            _rURL.Path       = _rParser.GetURLPath( INetURLObject::NO_DECODE           );
            _rURL.Name      = _rParser.GetName  (                                    );
        }

        _rURL.Arguments  = _rParser.GetParam  ( INetURLObject::NO_DECODE           );
        _rURL.Mark      = _rParser.GetMark  ( INetURLObject::DECODE_WITH_CHARSET );

        // INetURLObject supports only an intelligent method of parsing URL's. So write
        // back Complete to have a valid encoded URL in all cases!
        _rURL.Complete  = _rParser.GetMainURL( INetURLObject::NO_DECODE           );
        if ( _bUseIntern )
            _rURL.Complete   = _rURL.Complete.intern();

        _rParser.SetMark    ( OUString() );
        _rParser.SetParam( OUString() );

        _rURL.Main       = _rParser.GetMainURL( INetURLObject::NO_DECODE           );
    }
}
//*****************************************************************************************************************
//  XURLTransformer
//*****************************************************************************************************************
sal_Bool SAL_CALL URLTransformer::parseStrict( URL& aURL ) throw( RuntimeException )
{
    // Safe impossible cases.
    if  (( &aURL                        ==  NULL    )   ||
         ( aURL.Complete.isEmpty() ) )
    {
        return sal_False;
    }
    // Try to extract the protocol
    sal_Int32 nURLIndex = aURL.Complete.indexOf( sal_Unicode( ':' ));
    OUString aProtocol;
    if ( nURLIndex > 1 )
    {
        aProtocol = aURL.Complete.copy( 0, nURLIndex+1 );

        // If INetURLObject knows this protocol let it parse
        if ( INetURLObject::CompareProtocolScheme( aProtocol ) != INET_PROT_NOT_VALID )
        {
            // Initialize parser with given URL.
            INetURLObject aParser( aURL.Complete );

            // Get all information about this URL.
            INetProtocol eINetProt = aParser.GetProtocol();
            if ( eINetProt == INET_PROT_NOT_VALID )
            {
                return sal_False;
            }
            else if ( !aParser.HasError() )
            {
                lcl_ParserHelper(aParser,aURL,false);
                // Return "URL is parsed".
                return sal_True;
            }
        }
        else
        {
            // Minmal support for unknown protocols. This is mandatory to support the "Protocol Handlers" implemented
            // in framework!
            aURL.Protocol   = aProtocol;
            aURL.Main       = aURL.Complete;
            aURL.Path       = aURL.Complete.copy( nURLIndex+1 );;

            // Return "URL is parsed".
            return sal_True;
        }
    }

    return sal_False;
}

//*****************************************************************************************************************
//  XURLTransformer
//*****************************************************************************************************************
sal_Bool SAL_CALL URLTransformer::parseSmart(           URL&        aURL            ,
                                                const   OUString&    sSmartProtocol  ) throw( RuntimeException )
{
    // Safe impossible cases.
    if  (( &aURL                            ==  NULL    ) ||
         ( aURL.Complete.isEmpty() ) )
    {
        return sal_False;
    }

    // Initialize parser with given URL.
    INetURLObject aParser;

    aParser.SetSmartProtocol( INetURLObject::CompareProtocolScheme( sSmartProtocol ));
    bool bOk = aParser.SetSmartURL( aURL.Complete );
    if ( bOk )
    {
        lcl_ParserHelper(aParser,aURL,true);
        // Return "URL is parsed".
        return sal_True;
    }
    else
    {
        // Minmal support for unknown protocols. This is mandatory to support the "Protocol Handlers" implemented
        // in framework!
        if ( INetURLObject::CompareProtocolScheme( sSmartProtocol ) == INET_PROT_NOT_VALID )
        {
            // Try to extract the protocol
            sal_Int32 nIndex = aURL.Complete.indexOf( sal_Unicode( ':' ));
            OUString aProtocol;
            if ( nIndex > 1 )
            {
                aProtocol = aURL.Complete.copy( 0, nIndex+1 );

                // If INetURLObject knows this protocol something is wrong as detected before =>
                // give up and return false!
                if ( INetURLObject::CompareProtocolScheme( aProtocol ) != INET_PROT_NOT_VALID )
                    return sal_False;
                else
                    aURL.Protocol = aProtocol;
            }
            else
                return sal_False;

            aURL.Main = aURL.Complete;
            aURL.Path = aURL.Complete.copy( nIndex+1 );
            return sal_True;
        }
        else
            return sal_False;
    }
}

//*****************************************************************************************************************
//  XURLTransformer
//*****************************************************************************************************************
sal_Bool SAL_CALL URLTransformer::assemble( URL& aURL ) throw( RuntimeException )
{
    // Safe impossible cases.
    if  ( &aURL == NULL )
        return sal_False ;

    // Initialize parser.
    INetURLObject aParser;

    if ( INetURLObject::CompareProtocolScheme( aURL.Protocol ) != INET_PROT_NOT_VALID )
    {
        OUStringBuffer aCompletePath( aURL.Path );

        // Concat the name if it is provided, just support a final slash
        if ( !aURL.Name.isEmpty() )
        {
            sal_Int32 nIndex = aURL.Path.lastIndexOf( sal_Unicode('/') );
            if ( nIndex == ( aURL.Path.getLength() -1 ))
                aCompletePath.append( aURL.Name );
            else
            {
                aCompletePath.append( sal_Unicode( '/' ) );
                aCompletePath.append( aURL.Name );
            }
        }

        bool bResult = aParser.ConcatData(
                            INetURLObject::CompareProtocolScheme( aURL.Protocol )   ,
                             aURL.User                                              ,
                            aURL.Password                                           ,
                            aURL.Server                                             ,
                             aURL.Port                                              ,
                            aCompletePath.makeStringAndClear()                          );

        if ( !bResult )
            return sal_False;

        // First parse URL WITHOUT ...
        aURL.Main = aParser.GetMainURL( INetURLObject::NO_DECODE );
        // ...and then WITH parameter and mark.
        aParser.SetParam( aURL.Arguments);
        aParser.SetMark ( aURL.Mark, INetURLObject::ENCODE_ALL );
        aURL.Complete = aParser.GetMainURL( INetURLObject::NO_DECODE );

        // Return "URL is assembled".
        return sal_True;
    }
    else if ( !aURL.Protocol.isEmpty() )
    {
        // Minimal support for unknown protocols
        OUStringBuffer aBuffer( aURL.Protocol );
        aBuffer.append( aURL.Path );
        aURL.Complete   = aBuffer.makeStringAndClear();
        aURL.Main       = aURL.Complete;
        return sal_True;
    }

    return sal_False;
}

//*****************************************************************************************************************
//  XURLTransformer
//*****************************************************************************************************************
OUString SAL_CALL URLTransformer::getPresentation(   const   URL&        aURL            ,
                                                            sal_Bool    bWithPassword   ) throw( RuntimeException )
{
    // Safe impossible cases.
    if  (( &aURL                        ==  NULL        )   ||
         ( aURL.Complete.isEmpty()                      )   ||
            (( bWithPassword            !=  sal_True    )   &&
             ( bWithPassword            !=  sal_False   )       ) )
    {
        return OUString();
    }

    // Check given URL
    URL aTestURL = aURL;
    sal_Bool bParseResult = parseSmart( aTestURL, aTestURL.Protocol );
    if ( bParseResult )
    {
        if ( !bWithPassword && !aTestURL.Password.isEmpty() )
        {
            // Exchange password text with other placeholder string
            aTestURL.Password = OUString("<******>");
            assemble( aTestURL );
        }

        // Convert internal URLs to "praesentation"-URLs!
        OUString sPraesentationURL;
        INetURLObject::translateToExternal( aTestURL.Complete, sPraesentationURL, INetURLObject::DECODE_UNAMBIGUOUS );

        return sPraesentationURL;
    }
    else
        return OUString();
}

//_________________________________________________________________________________________________________________
//  debug methods
//_________________________________________________________________________________________________________________


}       //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
    //LOG_ASSERT( xFactory.is(), "URLTransformer::URLTransformer()\nInvalid parameter detected!\n" )
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

            rtl::OUStringBuffer aPath;
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

        _rParser.SetMark    ( ::rtl::OUString() );
        _rParser.SetParam( ::rtl::OUString() );

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
    ::rtl::OUString aProtocol;
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
                                                const   ::rtl::OUString&    sSmartProtocol  ) throw( RuntimeException )
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
            ::rtl::OUString aProtocol;
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
        ::rtl::OUStringBuffer aCompletePath( aURL.Path );

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
        ::rtl::OUStringBuffer aBuffer( aURL.Protocol );
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
::rtl::OUString SAL_CALL URLTransformer::getPresentation(   const   URL&        aURL            ,
                                                            sal_Bool    bWithPassword   ) throw( RuntimeException )
{
    // Safe impossible cases.
    if  (( &aURL                        ==  NULL        )   ||
         ( aURL.Complete.isEmpty()                      )   ||
            (( bWithPassword            !=  sal_True    )   &&
             ( bWithPassword            !=  sal_False   )       ) )
    {
        return ::rtl::OUString();
    }

    // Check given URL
    URL aTestURL = aURL;
    sal_Bool bParseResult = parseSmart( aTestURL, aTestURL.Protocol );
    if ( bParseResult )
    {
        if ( !bWithPassword && !aTestURL.Password.isEmpty() )
        {
            // Exchange password text with other placeholder string
            aTestURL.Password = ::rtl::OUString("<******>");
            assemble( aTestURL );
        }

        // Convert internal URLs to "praesentation"-URLs!
        rtl::OUString sPraesentationURL;
        INetURLObject::translateToExternal( aTestURL.Complete, sPraesentationURL, INetURLObject::DECODE_UNAMBIGUOUS );

        return sPraesentationURL;
    }
    else
        return ::rtl::OUString();
}

//_________________________________________________________________________________________________________________
//  debug methods
//_________________________________________________________________________________________________________________


}       //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

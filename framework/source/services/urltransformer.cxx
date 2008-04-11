/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: urltransformer.cxx,v $
 * $Revision: 1.16 $
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
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <services/urltransformer.hxx>
#include <threadhelp/resetableguard.hxx>
#include <macros/debug.hxx>
#include <services.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
#include <tools/urlobj.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

using namespace ::osl                           ;
using namespace ::rtl                           ;
using namespace ::cppu                          ;
using namespace ::com::sun::star::uno           ;
using namespace ::com::sun::star::lang          ;
using namespace ::com::sun::star::util          ;

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
URLTransformer::URLTransformer( const Reference< XMultiServiceFactory >& xFactory )
        //  Init baseclasses first
        //  Attention:
        //      Don't change order of initialization!
        //      ThreadHelpBase is a struct with a mutex as member. We can't use a mutex as member, while
        //      we must garant right initialization and a valid value of this! First initialize
        //      baseclasses and then members. And we need the mutex for other baseclasses !!!
        :   ThreadHelpBase  ( &Application::GetSolarMutex() )
        ,   OWeakObject     (                               )
        // Init member
        ,   m_xFactory      ( xFactory                      )
{
    // Safe impossible cases.
    // Method not defined for all incoming parameter.
    LOG_ASSERT( xFactory.is(), "URLTransformer::URLTransformer()\nInvalid parameter detected!\n" )
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

DEFINE_XINTERFACE_3                 (   URLTransformer                      ,
                                        OWeakObject                         ,
                                        DIRECT_INTERFACE(XTypeProvider      ),
                                        DIRECT_INTERFACE(XServiceInfo       ),
                                        DIRECT_INTERFACE(XURLTransformer    )
                                    )

DEFINE_XTYPEPROVIDER_3              (   URLTransformer  ,
                                        XTypeProvider   ,
                                        XServiceInfo    ,
                                        XURLTransformer
                                    )

DEFINE_XSERVICEINFO_MULTISERVICE    (   URLTransformer                      ,
                                        OWeakObject                         ,
                                        SERVICENAME_URLTRANSFORMER          ,
                                        IMPLEMENTATIONNAME_URLTRANSFORMER
                                    )

DEFINE_INIT_SERVICE                 (   URLTransformer,
                                        {
                                        }
                                    )

//*****************************************************************************************************************
//  XURLTransformer
//*****************************************************************************************************************
sal_Bool SAL_CALL URLTransformer::parseStrict( URL& aURL ) throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );

    // Safe impossible cases.
    if  (( &aURL                        ==  NULL    )   ||
         ( aURL.Complete.getLength()    <   1       )       )
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
                aURL.Protocol   = INetURLObject::GetScheme( aParser.GetProtocol() );
                aURL.User       = aParser.GetUser   ( INetURLObject::DECODE_WITH_CHARSET );
                aURL.Password   = aParser.GetPass   ( INetURLObject::DECODE_WITH_CHARSET );
                aURL.Server     = aParser.GetHost   ( INetURLObject::DECODE_WITH_CHARSET );
                aURL.Port       = (sal_Int16)aParser.GetPort();

                sal_Int32 nCount = aParser.getSegmentCount( false );
                if ( nCount > 0 )
                {
                    // Don't add last segment as it is the name!
                    --nCount;

                    rtl::OUStringBuffer aPath;
                    for ( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
                    {
                        aPath.append( sal_Unicode( '/' ));
                        aPath.append( aParser.getName( nIndex, false, INetURLObject::NO_DECODE ));
                    }

                    if ( nCount > 0 )
                        aPath.append( sal_Unicode( '/' )); // final slash!

                    aURL.Path = aPath.makeStringAndClear();
                    aURL.Name = aParser.getName( INetURLObject::LAST_SEGMENT, false, INetURLObject::NO_DECODE );
                }
                else
                {
                    aURL.Path       = aParser.GetURLPath( INetURLObject::NO_DECODE           );
                    aURL.Name       = aParser.GetName   (                                    );
                }

                aURL.Arguments  = aParser.GetParam  ( INetURLObject::NO_DECODE           );
                aURL.Mark       = aParser.GetMark   ( INetURLObject::DECODE_WITH_CHARSET );

                // INetURLObject supports only an intelligent method of parsing URL's. So write
                // back Complete to have a valid encoded URL in all cases!
                aURL.Complete   = aParser.GetMainURL( INetURLObject::NO_DECODE           );
                aURL.Complete   = aURL.Complete.intern();

                aParser.SetMark ( OUString() );
                aParser.SetParam( OUString() );

                aURL.Main       = aParser.GetMainURL( INetURLObject::NO_DECODE           );

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
                                                const   OUString&   sSmartProtocol  ) throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );
    // Safe impossible cases.
    if  (( &aURL                            ==  NULL    ) ||
         ( aURL.Complete.getLength()        <   1       )    )
    {
        return sal_False;
    }

    // Initialize parser with given URL.
    INetURLObject aParser;

    aParser.SetSmartProtocol( INetURLObject::CompareProtocolScheme( sSmartProtocol ));
    bool bOk = aParser.SetSmartURL( aURL.Complete );
    if ( bOk )
    {
        // Get all information about this URL.
        aURL.Protocol   = INetURLObject::GetScheme( aParser.GetProtocol() );
        aURL.User       = aParser.GetUser   ( INetURLObject::DECODE_WITH_CHARSET );
        aURL.Password   = aParser.GetPass   ( INetURLObject::DECODE_WITH_CHARSET );
        aURL.Server     = aParser.GetHost   ( INetURLObject::DECODE_WITH_CHARSET );
        aURL.Port       = (sal_Int16)aParser.GetPort();

        sal_Int32 nCount = aParser.getSegmentCount( false );
        if ( nCount > 0 )
        {
            // Don't add last segment as it is the name!
            --nCount;

            rtl::OUStringBuffer aPath;
            for ( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
            {
                aPath.append( sal_Unicode( '/' ));
                aPath.append( aParser.getName( nIndex, false, INetURLObject::NO_DECODE ));
            }

            if ( nCount > 0 )
                aPath.append( sal_Unicode( '/' )); // final slash!

            aURL.Path = aPath.makeStringAndClear();
            aURL.Name = aParser.getName( INetURLObject::LAST_SEGMENT, false, INetURLObject::NO_DECODE );
        }
        else
        {
            aURL.Path       = aParser.GetURLPath( INetURLObject::NO_DECODE           );
            aURL.Name       = aParser.GetName   (                                    );
        }

        aURL.Arguments  = aParser.GetParam  ( INetURLObject::NO_DECODE           );
        aURL.Mark       = aParser.GetMark   ( INetURLObject::DECODE_WITH_CHARSET );

        aURL.Complete   = aParser.GetMainURL( INetURLObject::NO_DECODE           );

        aParser.SetMark ( OUString() );
        aParser.SetParam( OUString() );

        aURL.Main       = aParser.GetMainURL( INetURLObject::NO_DECODE           );

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
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );

    // Safe impossible cases.
    if  ( &aURL == NULL )
        return sal_False ;

    // Initialize parser.
    INetURLObject aParser;

    if ( INetURLObject::CompareProtocolScheme( aURL.Protocol ) != INET_PROT_NOT_VALID )
    {
        OUStringBuffer aCompletePath( aURL.Path );

        // Concat the name if it is provided, just support a final slash
        if ( aURL.Name.getLength() > 0 )
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
    else if ( aURL.Protocol.getLength() > 0 )
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
OUString SAL_CALL URLTransformer::getPresentation(  const   URL&        aURL            ,
                                                            sal_Bool    bWithPassword   ) throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );

    // Safe impossible cases.
    if  (( &aURL                        ==  NULL        )   ||
         ( aURL.Complete.getLength()    <   1           )   ||
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
        if ( !bWithPassword && aTestURL.Password.getLength() > 0 )
        {
            // Exchange password text with other placeholder string
            aTestURL.Password = OUString::createFromAscii( "<******>" );
            assemble( aTestURL );
        }

        // Convert internal URLs to "praesentation"-URLs!
        rtl::OUString sPraesentationURL;
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

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

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <tools/urlobj.hxx>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace {

class URLTransformer : public ::cppu::WeakImplHelper< css::util::XURLTransformer, css::lang::XServiceInfo>
{
public:
    URLTransformer() {}

    virtual OUString SAL_CALL getImplementationName() override
    {
        return "com.sun.star.comp.framework.URLTransformer";
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return {"com.sun.star.util.URLTransformer"};
    }

    virtual sal_Bool SAL_CALL parseStrict( css::util::URL& aURL ) override;

    virtual sal_Bool SAL_CALL parseSmart( css::util::URL& aURL, const OUString& sSmartProtocol ) override;

    virtual sal_Bool SAL_CALL assemble( css::util::URL& aURL ) override;

    virtual OUString SAL_CALL getPresentation( const css::util::URL& aURL, sal_Bool bWithPassword ) override;
};

void lcl_ParserHelper(INetURLObject& _rParser, css::util::URL& _rURL,bool _bUseIntern)
{
    // Get all information about this URL.
    _rURL.Protocol  = INetURLObject::GetScheme( _rParser.GetProtocol() );
    _rURL.User      = _rParser.GetUser  ( INetURLObject::DecodeMechanism::WithCharset );
    _rURL.Password  = _rParser.GetPass  ( INetURLObject::DecodeMechanism::WithCharset );
    _rURL.Server        = _rParser.GetHost  ( INetURLObject::DecodeMechanism::WithCharset );
    _rURL.Port      = static_cast<sal_Int16>(_rParser.GetPort());

    sal_Int32 nCount = _rParser.getSegmentCount( false );
    if ( nCount > 0 )
    {
        // Don't add last segment as it is the name!
        --nCount;

        OUStringBuffer aPath(128);
        for ( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
        {
            aPath.append( '/');
            aPath.append( _rParser.getName( nIndex, false, INetURLObject::DecodeMechanism::NONE ));
        }

        if ( nCount > 0 )
            aPath.append( '/' ); // final slash!

        _rURL.Path = aPath.makeStringAndClear();
        _rURL.Name = _rParser.getName( INetURLObject::LAST_SEGMENT, false, INetURLObject::DecodeMechanism::NONE );
    }
    else
    {
        _rURL.Path       = _rParser.GetURLPath( INetURLObject::DecodeMechanism::NONE           );
        _rURL.Name = _rParser.GetLastName();
    }

    _rURL.Arguments  = _rParser.GetParam();
    _rURL.Mark      = _rParser.GetMark( INetURLObject::DecodeMechanism::WithCharset );

    // INetURLObject supports only an intelligent method of parsing URL's. So write
    // back Complete to have a valid encoded URL in all cases!
    _rURL.Complete  = _rParser.GetMainURL( INetURLObject::DecodeMechanism::NONE           );
    if ( _bUseIntern )
        _rURL.Complete   = _rURL.Complete.intern();

    _rParser.SetMark( u"" );
    _rParser.SetParam( u"" );

    _rURL.Main       = _rParser.GetMainURL( INetURLObject::DecodeMechanism::NONE           );
}

//  XURLTransformer
sal_Bool SAL_CALL URLTransformer::parseStrict( css::util::URL& aURL )
{
    // Safe impossible cases.
    if ( aURL.Complete.isEmpty() )
    {
        return false;
    }
    // Try to extract the protocol
    sal_Int32 nURLIndex = aURL.Complete.indexOf( ':' );
    if ( nURLIndex > 1 )
    {
        OUString aProtocol = aURL.Complete.copy( 0, nURLIndex+1 );

        // If INetURLObject knows this protocol let it parse
        if ( INetURLObject::CompareProtocolScheme( aProtocol ) != INetProtocol::NotValid )
        {
            // Initialize parser with given URL.
            INetURLObject aParser( aURL.Complete );

            // Get all information about this URL.
            INetProtocol eINetProt = aParser.GetProtocol();
            if ( eINetProt == INetProtocol::NotValid )
            {
                return false;
            }
            else if ( !aParser.HasError() )
            {
                lcl_ParserHelper(aParser,aURL,false);
                // Return "URL is parsed".
                return true;
            }
        }
        else
        {
            // Minimal support for unknown protocols. This is mandatory to support the "Protocol Handlers" implemented
            // in framework!
            aURL.Protocol   = aProtocol;
            aURL.Main       = aURL.Complete;
            aURL.Path       = aURL.Complete.copy( nURLIndex+1 );

            // Return "URL is parsed".
            return true;
        }
    }

    return false;
}

//  XURLTransformer

sal_Bool SAL_CALL URLTransformer::parseSmart( css::util::URL& aURL,
                                                const   OUString&    sSmartProtocol  )
{
    // Safe impossible cases.
    if ( aURL.Complete.isEmpty() )
    {
        return false;
    }

    // Initialize parser with given URL.
    INetURLObject aParser;

    aParser.SetSmartProtocol( INetURLObject::CompareProtocolScheme( sSmartProtocol ));
    bool bOk = aParser.SetSmartURL( aURL.Complete );
    if ( bOk )
    {
        lcl_ParserHelper(aParser,aURL,true);
        // Return "URL is parsed".
        return true;
    }
    else
    {
        // Minimal support for unknown protocols. This is mandatory to support the "Protocol Handlers" implemented
        // in framework!
        if ( INetURLObject::CompareProtocolScheme( sSmartProtocol ) == INetProtocol::NotValid )
        {
            // Try to extract the protocol
            sal_Int32 nIndex = aURL.Complete.indexOf( ':' );
            if ( nIndex > 1 )
            {
                OUString aProtocol = aURL.Complete.copy( 0, nIndex+1 );

                // If INetURLObject knows this protocol something is wrong as detected before =>
                // give up and return false!
                if ( INetURLObject::CompareProtocolScheme( aProtocol ) != INetProtocol::NotValid )
                    return false;
                else
                    aURL.Protocol = aProtocol;
            }
            else
                return false;

            aURL.Main = aURL.Complete;
            aURL.Path = aURL.Complete.copy( nIndex+1 );
            return true;
        }
        else
            return false;
    }
}

//  XURLTransformer
sal_Bool SAL_CALL URLTransformer::assemble( css::util::URL& aURL )
{
    // Initialize parser.
    INetURLObject aParser;

    if ( INetURLObject::CompareProtocolScheme( aURL.Protocol ) != INetProtocol::NotValid )
    {
        OUStringBuffer aCompletePath( aURL.Path );

        // Concat the name if it is provided, just support a final slash
        if ( !aURL.Name.isEmpty() )
        {
            sal_Int32 nIndex = aURL.Path.lastIndexOf( '/' );
            if ( nIndex == ( aURL.Path.getLength() -1 ))
                aCompletePath.append( aURL.Name );
            else
            {
                aCompletePath.append( '/' );
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
            return false;

        // First parse URL WITHOUT ...
        aURL.Main = aParser.GetMainURL( INetURLObject::DecodeMechanism::NONE );
        // ...and then WITH parameter and mark.
        aParser.SetParam( aURL.Arguments);
        aParser.SetMark ( aURL.Mark, INetURLObject::EncodeMechanism::All );
        aURL.Complete = aParser.GetMainURL( INetURLObject::DecodeMechanism::NONE );

        // Return "URL is assembled".
        return true;
    }
    else if ( !aURL.Protocol.isEmpty() )
    {
        // Minimal support for unknown protocols
        aURL.Complete   = aURL.Protocol + aURL.Path;
        aURL.Main       = aURL.Complete;
        return true;
    }

    return false;
}

//  XURLTransformer

OUString SAL_CALL URLTransformer::getPresentation( const css::util::URL& aURL,
                                                            sal_Bool    bWithPassword   )
{
    // Safe impossible cases.
    if  ( aURL.Complete.isEmpty() )
    {
        return OUString();
    }

    // Check given URL
    css::util::URL aTestURL = aURL;
    bool bParseResult = parseSmart( aTestURL, aTestURL.Protocol );
    if ( bParseResult )
    {
        if ( !bWithPassword && !aTestURL.Password.isEmpty() )
        {
            // Exchange password text with other placeholder string
            aTestURL.Password = "<******>";
            assemble( aTestURL );
        }

        // Convert internal URLs to "praesentation"-URLs!
        OUString sPraesentationURL;
        INetURLObject::translateToExternal( aTestURL.Complete, sPraesentationURL, INetURLObject::DecodeMechanism::Unambiguous );

        return sPraesentationURL;
    }
    else
        return OUString();
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_URLTransformer_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new URLTransformer());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <services.h>

#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <tools/urlobj.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace {

class URLTransformer : public ::cppu::WeakImplHelper2< css::util::XURLTransformer, css::lang::XServiceInfo>
{
public:
    URLTransformer() {}

    virtual ~URLTransformer() {}

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException)
    {
        return OUString("com.sun.star.comp.framework.URLTransformer");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException)
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException)
    {
        css::uno::Sequence< OUString > aRet(1);
        aRet[0] = "com.sun.star.util.URLTransformer";
        return aRet;
    }

    virtual sal_Bool SAL_CALL parseStrict( css::util::URL& aURL )
        throw( css::uno::RuntimeException );

    virtual sal_Bool SAL_CALL parseSmart( css::util::URL& aURL, const OUString& sSmartProtocol )
        throw( css::uno::RuntimeException );

    virtual sal_Bool SAL_CALL assemble( css::util::URL& aURL )
        throw( css::uno::RuntimeException );

    virtual OUString SAL_CALL getPresentation( const css::util::URL& aURL, sal_Bool bWithPassword )
        throw( css::uno::RuntimeException );
};

namespace
{
    void lcl_ParserHelper(INetURLObject& _rParser, css::util::URL& _rURL,bool _bUseIntern)
    {
        
        _rURL.Protocol  = INetURLObject::GetScheme( _rParser.GetProtocol() );
        _rURL.User      = _rParser.GetUser  ( INetURLObject::DECODE_WITH_CHARSET );
        _rURL.Password  = _rParser.GetPass  ( INetURLObject::DECODE_WITH_CHARSET );
        _rURL.Server        = _rParser.GetHost  ( INetURLObject::DECODE_WITH_CHARSET );
        _rURL.Port      = (sal_Int16)_rParser.GetPort();

        sal_Int32 nCount = _rParser.getSegmentCount( false );
        if ( nCount > 0 )
        {
            
            --nCount;

            OUStringBuffer aPath;
            for ( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
            {
                aPath.append( '/');
                aPath.append( _rParser.getName( nIndex, false, INetURLObject::NO_DECODE ));
            }

            if ( nCount > 0 )
                aPath.append( '/' ); 

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

        
        
        _rURL.Complete  = _rParser.GetMainURL( INetURLObject::NO_DECODE           );
        if ( _bUseIntern )
            _rURL.Complete   = _rURL.Complete.intern();

        _rParser.SetMark    ( OUString() );
        _rParser.SetParam( OUString() );

        _rURL.Main       = _rParser.GetMainURL( INetURLObject::NO_DECODE           );
    }
}



sal_Bool SAL_CALL URLTransformer::parseStrict( css::util::URL& aURL ) throw( css::uno::RuntimeException )
{
    
    if  (( &aURL                        ==  NULL    )   ||
         ( aURL.Complete.isEmpty() ) )
    {
        return sal_False;
    }
    
    sal_Int32 nURLIndex = aURL.Complete.indexOf( ':' );
    OUString aProtocol;
    if ( nURLIndex > 1 )
    {
        aProtocol = aURL.Complete.copy( 0, nURLIndex+1 );

        
        if ( INetURLObject::CompareProtocolScheme( aProtocol ) != INET_PROT_NOT_VALID )
        {
            
            INetURLObject aParser( aURL.Complete );

            
            INetProtocol eINetProt = aParser.GetProtocol();
            if ( eINetProt == INET_PROT_NOT_VALID )
            {
                return sal_False;
            }
            else if ( !aParser.HasError() )
            {
                lcl_ParserHelper(aParser,aURL,false);
                
                return sal_True;
            }
        }
        else
        {
            
            
            aURL.Protocol   = aProtocol;
            aURL.Main       = aURL.Complete;
            aURL.Path       = aURL.Complete.copy( nURLIndex+1 );;

            
            return sal_True;
        }
    }

    return sal_False;
}




sal_Bool SAL_CALL URLTransformer::parseSmart( css::util::URL& aURL,
                                                const   OUString&    sSmartProtocol  ) throw( css::uno::RuntimeException )
{
    
    if  (( &aURL                            ==  NULL    ) ||
         ( aURL.Complete.isEmpty() ) )
    {
        return sal_False;
    }

    
    INetURLObject aParser;

    aParser.SetSmartProtocol( INetURLObject::CompareProtocolScheme( sSmartProtocol ));
    bool bOk = aParser.SetSmartURL( aURL.Complete );
    if ( bOk )
    {
        lcl_ParserHelper(aParser,aURL,true);
        
        return sal_True;
    }
    else
    {
        
        
        if ( INetURLObject::CompareProtocolScheme( sSmartProtocol ) == INET_PROT_NOT_VALID )
        {
            
            sal_Int32 nIndex = aURL.Complete.indexOf( ':' );
            OUString aProtocol;
            if ( nIndex > 1 )
            {
                aProtocol = aURL.Complete.copy( 0, nIndex+1 );

                
                
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




sal_Bool SAL_CALL URLTransformer::assemble( css::util::URL& aURL ) throw( css::uno::RuntimeException )
{
    
    if  ( &aURL == NULL )
        return sal_False ;

    
    INetURLObject aParser;

    if ( INetURLObject::CompareProtocolScheme( aURL.Protocol ) != INET_PROT_NOT_VALID )
    {
        OUStringBuffer aCompletePath( aURL.Path );

        
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
            return sal_False;

        
        aURL.Main = aParser.GetMainURL( INetURLObject::NO_DECODE );
        
        aParser.SetParam( aURL.Arguments);
        aParser.SetMark ( aURL.Mark, INetURLObject::ENCODE_ALL );
        aURL.Complete = aParser.GetMainURL( INetURLObject::NO_DECODE );

        
        return sal_True;
    }
    else if ( !aURL.Protocol.isEmpty() )
    {
        
        OUStringBuffer aBuffer( aURL.Protocol );
        aBuffer.append( aURL.Path );
        aURL.Complete   = aBuffer.makeStringAndClear();
        aURL.Main       = aURL.Complete;
        return sal_True;
    }

    return sal_False;
}




OUString SAL_CALL URLTransformer::getPresentation( const css::util::URL& aURL,
                                                            sal_Bool    bWithPassword   ) throw( css::uno::RuntimeException )
{
    
    if  (( &aURL                        ==  NULL        )   ||
         ( aURL.Complete.isEmpty()                      )   ||
            (( bWithPassword            !=  sal_True    )   &&
             ( bWithPassword            !=  sal_False   )       ) )
    {
        return OUString();
    }

    
    css::util::URL aTestURL = aURL;
    sal_Bool bParseResult = parseSmart( aTestURL, aTestURL.Protocol );
    if ( bParseResult )
    {
        if ( !bWithPassword && !aTestURL.Password.isEmpty() )
        {
            
            aTestURL.Password = "<******>";
            assemble( aTestURL );
        }

        
        OUString sPraesentationURL;
        INetURLObject::translateToExternal( aTestURL.Complete, sPraesentationURL, INetURLObject::DECODE_UNAMBIGUOUS );

        return sPraesentationURL;
    }
    else
        return OUString();
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_URLTransformer_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new URLTransformer());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

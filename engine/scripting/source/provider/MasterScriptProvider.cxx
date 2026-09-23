/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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


#include <comphelper/SetFlagContextHelper.hxx>
#include <comphelper/documentinfo.hxx>

#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/urlobj.hxx>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/script/provider/ScriptFrameworkErrorException.hpp>
#include <com/sun/star/uri/XUriReference.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>

#include <com/sun/star/deployment/XPackage.hpp>
#include <com/sun/star/script/browse/BrowseNodeTypes.hpp>
#include <com/sun/star/script/provider/theMasterScriptProviderFactory.hpp>
#include <com/sun/star/script/provider/ScriptFrameworkErrorType.hpp>

#include <util/MiscUtils.hxx>
#include <sal/log.hxx>

#include "MasterScriptProvider.hxx"

using namespace ::com::sun::star;
using namespace ::cpo::uno;
using namespace cpo::uno;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::document;
using namespace ::sf_misc;

namespace func_provider
{

static bool endsWith( std::u16string_view target, std::u16string_view item )
{
    size_t index = target.find( item );
    return index != std::u16string_view::npos  &&
           index == ( target.size() - item.size() );
}

/* should be available in some central location. */

// XScriptProvider implementation


MasterScriptProvider::MasterScriptProvider( const Reference< XComponentContext > & xContext ):
        m_xContext( xContext ), m_bIsValid( false ), m_bInitialised( false )
{
    ENSURE_OR_THROW( m_xContext.is(), "MasterScriptProvider::MasterScriptProvider: No context available\n" );
    m_xMgr = m_xContext->getServiceManager();
    ENSURE_OR_THROW( m_xMgr.is(), "MasterScriptProvider::MasterScriptProvider: No service manager available\n" );
    m_bIsValid = true;
}


MasterScriptProvider::~MasterScriptProvider()
{
}


void MasterScriptProvider::initialize( const Sequence < Any >& args )
{
    if ( m_bInitialised )
        return;

    m_bIsValid = false;

    sal_Int32 len = args.getLength();
    if ( len > 1  )
    {
        throw RuntimeException(
            u"MasterScriptProvider::initialize: invalid number of arguments"_ustr );
    }

    Sequence< Any > invokeArgs( len );

    if ( len != 0 )
    {
        auto pinvokeArgs = invokeArgs.getArray();
        // check if first parameter is a string
        // if it is, this implies that this is a MSP created
        // with a user or share ctx ( used for browse functionality )

        if ( args[ 0 ] >>= m_sCtxString )
        {
            pinvokeArgs[ 0  ] = args[ 0 ];
            if ( m_sCtxString.startsWith( "vnd.sun.star.tdoc" ) )
            {
                m_xModel =  MiscUtils::tDocUrlToModel( m_sCtxString );
            }
        }
        else if ( args[ 0 ] >>= m_xInvocationContext )
        {
            m_xModel.set( m_xInvocationContext->getScriptContainer(), UNO_QUERY_THROW );
        }
        else
        {
            args[ 0 ] >>= m_xModel;
        }

        if ( m_xModel.is() )
        {
            // from the arguments, we were able to deduce a model. That alone doesn't
            // suffice, we also need an XEmbeddedScripts which actually indicates support
            // for embedding scripts
            Reference< XEmbeddedScripts > xScripts( m_xModel, UNO_QUERY );
            if ( !xScripts.is() )
            {
                throw lang::IllegalArgumentException(
                    u"The given document does not support embedding scripts into it, and cannot be associated with such a document."_ustr,
                    *this,
                    1
                );
            }

            try
            {
                m_sCtxString =  MiscUtils::xModelToTdocUrl( m_xModel, m_xContext );
            }
            catch ( const cpo::uno::Exception& )
            {
                Any aError( ::cppu::getCaughtException() );

                Exception aException;
                aError >>= aException;
                OUString buf =
                    "MasterScriptProvider::initialize: caught " +
                    aError.getValueTypeName() +
                    ":" +
                    aException.Message;
                throw lang::WrappedTargetException( buf, *this, aError );
            }

            if ( m_xInvocationContext.is() && m_xInvocationContext != m_xModel )
                pinvokeArgs[ 0 ] <<= m_xInvocationContext;
            else
                pinvokeArgs[ 0 ] <<= m_sCtxString;
        }
    }
    else // no args
    {
        // use either scripting context or maybe zero args?
        invokeArgs = Sequence< Any >( 0 ); // no arguments
    }
    m_sAargs = std::move(invokeArgs);
    m_bInitialised = true;
    m_bIsValid = true;
}



Reference< provider::XScript >
MasterScriptProvider::getScript( const OUString& scriptURI )
{
    if ( !m_bIsValid )
    {
        throw provider::ScriptFrameworkErrorException(
            u"MasterScriptProvider not initialised"_ustr, Reference< XInterface >(),
            scriptURI, u""_ustr,
            provider::ScriptFrameworkErrorType::UNKNOWN );
    }

    // need to get the language from the string

    Reference< uri::XUriReferenceFactory > xFac ( uri::UriReferenceFactory::create( m_xContext )  );

    Reference<  uri::XUriReference > uriRef = xFac->parse( scriptURI );

    Reference < uri::XVndSunStarScriptUrl > sfUri( uriRef, UNO_QUERY );

    if ( !uriRef.is() || !sfUri.is() )
    {
        throw provider::ScriptFrameworkErrorException(
            "Incorrect format for Script URI: " + scriptURI,
            Reference< XInterface >(),
            scriptURI, u""_ustr,
            provider::ScriptFrameworkErrorType::UNKNOWN );
    }

    OUString langKey(u"language"_ustr);
    OUString locKey(u"location"_ustr);

    if ( !sfUri->hasParameter( langKey ) ||
         !sfUri->hasParameter( locKey ) ||
         ( sfUri->getName().isEmpty()  ) )
    {
        throw provider::ScriptFrameworkErrorException(
            "Incorrect format for Script URI: " + scriptURI,
            Reference< XInterface >(),
            scriptURI, u""_ustr,
            provider::ScriptFrameworkErrorType::UNKNOWN );
    }

    OUString language = sfUri->getParameter( langKey );
    OUString location = sfUri->getParameter( locKey );

    Reference< provider::XScript > xScript;

    // If the script location is in the same location context as this
    // MSP then delete to the language provider controlled by this MSP
    // ** Special case is BASIC, all calls to getScript will be handled
    // by the language script provider in the current location context
    // even if it's different
    if  (   (   location == "document"
            &&  m_xModel.is()
            )
            ||  ( endsWith( m_sCtxString, location ) )
            ||  ( language == "Basic" )
         )
    {
        Reference< provider::XScriptProvider > xScriptProvider;
        OUString serviceName = "com.sun.star.script.provider.ScriptProviderFor" + language;
        if ( !providerCache() )
        {
            throw provider::ScriptFrameworkErrorException(
                u"No LanguageProviders detected"_ustr,
                Reference< XInterface >(),
                sfUri->getName(), language,
                provider::ScriptFrameworkErrorType::NOTSUPPORTED );
        }

        try
        {
            xScriptProvider.set(
                providerCache()->getProvider( serviceName ),
                UNO_SET_THROW );
        }
        catch( const Exception& e )
        {
            throw provider::ScriptFrameworkErrorException(
                e.Message, Reference< XInterface >(),
                sfUri->getName(), language,
                provider::ScriptFrameworkErrorType::NOTSUPPORTED );
        }

        xScript=xScriptProvider->getScript( scriptURI );
    }
    else
    {
        Reference< provider::XScriptProviderFactory > xFac_ =
            provider::theMasterScriptProviderFactory::get( m_xContext );

        Reference< provider::XScriptProvider > xSP(
            xFac_->createScriptProvider( Any( location ) ), UNO_SET_THROW );
        xScript = xSP->getScript( scriptURI );
    }

    return xScript;
}


ProviderCache*
MasterScriptProvider::providerCache()
{
    std::scoped_lock aGuard( m_mutex );
    if ( !m_pPCache )
    {
        m_pPCache.reset( new ProviderCache( m_xContext, m_sAargs ) );
    }
    return m_pPCache.get();
}


OUString
MasterScriptProvider::getName()
{
    OUString sCtx = getContextString();
    {
        if ( sCtx.startsWith( "vnd.sun.star.tdoc" ) )
        {
            Reference< frame::XModel > xModel = m_xModel;
            if ( !xModel.is() )
            {
                xModel = MiscUtils::tDocUrlToModel( sCtx );
            }

            m_sNodeName = ::comphelper::DocumentInfo::getDocumentTitle( xModel );
        }
        else
        {
            m_sNodeName = parseLocationName( getContextString() );
        }
    }
    return m_sNodeName;
}


Sequence< Reference< browse::XBrowseNode > >
MasterScriptProvider::getChildNodes()
{
    Sequence< Reference< provider::XScriptProvider > > providers = providerCache()->getAllProviders();

    Sequence<  Reference< browse::XBrowseNode > > children( providers.getLength() );
    auto childrenRange = asNonConstRange(children);
    sal_Int32 provIndex = 0;
    for ( ; provIndex < providers.getLength(); provIndex++ )
    {
        childrenRange[ provIndex ].set( providers[ provIndex ], UNO_QUERY );
    }

    return children;
}


bool
MasterScriptProvider::hasChildNodes()
{
    return true;
}


sal_Int16
MasterScriptProvider::getType()
{
    return browse::BrowseNodeTypes::CONTAINER;
}


OUString
MasterScriptProvider::parseLocationName( const OUString& location )
{
    // strip out the last leaf of location name
    // e.g. file://dir1/dir2/Blah.sxw - > Blah.sxw
    OUString temp = location;
    INetURLObject aURLObj( temp );
    if ( !aURLObj.HasError() )
        temp = aURLObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset );
    return temp;
}


OUString MasterScriptProvider::getImplementationName( )
{
    return u"com.sun.star.script.provider.MasterScriptProvider"_ustr;
}

bool MasterScriptProvider::supportsService( const OUString& serviceName )
{
    return cppu::supportsService(this, serviceName);
}


Sequence< OUString > MasterScriptProvider::getSupportedServiceNames( )
{
    return {
        u"com.sun.star.script.provider.MasterScriptProvider"_ustr,
        u"com.sun.star.script.browse.BrowseNode"_ustr,
        u"com.sun.star.script.provider.ScriptProvider"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT cpo::uno::XInterface*
scripting_MasterScriptProvider_get_implementation(
    cpo::uno::XComponentContext* context, cpo::uno::Sequence<cpo::uno::Any> const&)
{
    return cppu::acquire(new MasterScriptProvider(context));
}

} // namespace func_provider


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

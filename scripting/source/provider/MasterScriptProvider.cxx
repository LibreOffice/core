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
using namespace ::com::sun::star::uno;
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
        m_xContext( xContext ), m_bIsValid( false ), m_bInitialised( false ),
        m_bIsPkgMSP( false )
{
    ENSURE_OR_THROW( m_xContext.is(), "MasterScriptProvider::MasterScriptProvider: No context available\n" );
    m_xMgr = m_xContext->getServiceManager();
    ENSURE_OR_THROW( m_xMgr.is(), "MasterScriptProvider::MasterScriptProvider: No service manager available\n" );
    m_bIsValid = true;
}


MasterScriptProvider::~MasterScriptProvider()
{
}


void SAL_CALL MasterScriptProvider::initialize( const Sequence < Any >& args )
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
            catch ( const Exception& )
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

        OUString pkgSpec = u"uno_packages"_ustr;
        sal_Int32 indexOfPkgSpec = m_sCtxString.lastIndexOf( pkgSpec );

        // if context string ends with "uno_packages"
        if ( indexOfPkgSpec > -1 && m_sCtxString.match( pkgSpec, indexOfPkgSpec ) )
        {
            m_bIsPkgMSP = true;
        }
        else
        {
            m_bIsPkgMSP = false;
        }
    }
    else // no args
    {
        // use either scripting context or maybe zero args?
        invokeArgs = Sequence< Any >( 0 ); // no arguments
    }
    m_sAargs = invokeArgs;
    // don't create pkg mgr MSP for documents, not supported
    if ( !m_bIsPkgMSP && !m_xModel.is() )
    {
        createPkgProvider();
    }

    m_bInitialised = true;
    m_bIsValid = true;
}


void MasterScriptProvider::createPkgProvider()
{
    try
    {
        Any location;
        location <<= m_sCtxString + ":uno_packages";

        Reference< provider::XScriptProviderFactory > xFac =
            provider::theMasterScriptProviderFactory::get( m_xContext );

        m_xMSPPkg.set(
            xFac->createScriptProvider( location ), UNO_SET_THROW );

    }
    catch ( const Exception& )
    {
        TOOLS_WARN_EXCEPTION("scripting.provider", "Exception creating MasterScriptProvider for uno_packages in context "
                << m_sCtxString );
    }
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

    // if script us located in uno pkg
    sal_Int32 index = -1;
    OUString pkgTag(u":uno_packages"_ustr);
    // for languages other than basic,  scripts located in uno packages
    // are merged into the user/share location context.
    // For other languages the location attribute in script url has the form
    // location = [user|share]:uno_packages or location :uno_packages/xxxx.uno.pkg
    // we need to extract the value of location part from the
    // location attribute of the script, if the script is located in an
    // uno package then that is the location part up to and including
    // ":uno_packages", if the script is not in a uno package then the
    // normal value is used e.g. user or share.
    // The value extracted will be used to determine if the script is
    // located in the same location context as this MSP.
    // For Basic, the language script provider can handle the execution of a
    // script in any location context
    if ( ( index = location.indexOf( pkgTag ) ) > -1 )
    {
        location = location.copy( 0, index + pkgTag.getLength() );
    }

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
    if ( !m_pPCache )
    {
        std::scoped_lock aGuard( m_mutex );
        if ( !m_pPCache )
        {
            Sequence<OUString> denylist { u"com.sun.star.script.provider.ScriptProviderForBasic"_ustr };

            if ( !m_bIsPkgMSP )
            {
                m_pPCache.reset( new ProviderCache( m_xContext, m_sAargs ) );
            }
            else
            {
                m_pPCache.reset( new ProviderCache( m_xContext, m_sAargs, denylist ) );
            }
        }
    }
    return m_pPCache.get();
}


OUString SAL_CALL
MasterScriptProvider::getName()
{
    if ( !m_bIsPkgMSP )
    {
        OUString sCtx = getContextString();
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
    else
    {
        m_sNodeName = "uno_packages";
    }
    return m_sNodeName;
}


Sequence< Reference< browse::XBrowseNode > > SAL_CALL
MasterScriptProvider::getChildNodes()
{
    Sequence< Reference< provider::XScriptProvider > > providers = providerCache()->getAllProviders();

    sal_Int32 size = providers.getLength();
    bool hasPkgs = m_xMSPPkg.is();
    if ( hasPkgs  )
    {
        size++;
    }
    Sequence<  Reference< browse::XBrowseNode > > children( size );
    auto childrenRange = asNonConstRange(children);
    sal_Int32 provIndex = 0;
    for ( ; provIndex < providers.getLength(); provIndex++ )
    {
        childrenRange[ provIndex ].set( providers[ provIndex ], UNO_QUERY );
    }

    if ( hasPkgs  )
    {
        childrenRange[ provIndex ].set( m_xMSPPkg, UNO_QUERY );

    }

    return children;
}


sal_Bool SAL_CALL
MasterScriptProvider::hasChildNodes()
{
    return true;
}


sal_Int16 SAL_CALL
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

namespace
{
template <typename Proc> bool FindProviderAndApply(ProviderCache& rCache, Proc p)
{
    auto pass = [&rCache, &p]() -> bool
    {
        bool bResult = false;
        const Sequence<Reference<provider::XScriptProvider>> aAllProviders = rCache.getAllProviders();
        for (const auto& rProv : aAllProviders)
        {
            Reference<container::XNameContainer> xCont(rProv, UNO_QUERY);
            if (!xCont.is())
            {
                continue;
            }
            try
            {
                bResult = p(xCont);
                if (bResult)
                    break;
            }
            catch (const Exception&)
            {
                TOOLS_INFO_EXCEPTION("scripting.provider", "ignoring");
            }
        }
        return bResult;
    };
    bool bSuccess = false;
    // 1. Try to perform the operation without trying to enable JVM (if disabled)
    // This allows us to avoid useless user interaction in case when other provider
    // (not JVM) actually handles the operation.
    {
        css::uno::ContextLayer layer(comphelper::NoEnableJavaInteractionContext());
        bSuccess = pass();
    }
    // 2. Now retry asking to enable JVM in case we didn't succeed first time
    if (!bSuccess)
    {
        bSuccess = pass();
    }
    return bSuccess;
}
} // namespace

// Register Package
void SAL_CALL
MasterScriptProvider::insertByName( const OUString& aName, const Any& aElement )
{
    if ( !m_bIsPkgMSP )
    {
        if ( !m_xMSPPkg.is() )
        {
            throw RuntimeException( u"PackageMasterScriptProvider is unitialised"_ustr );
        }

        Reference< container::XNameContainer > xCont( m_xMSPPkg, UNO_QUERY_THROW );
        xCont->insertByName( aName, aElement );
    }
    else
    {
        Reference< deployment::XPackage > xPkg( aElement, UNO_QUERY );
        if ( !xPkg.is() )
        {
            throw lang::IllegalArgumentException( u"Couldn't convert to XPackage"_ustr,
                                                      Reference < XInterface > (), 2 );
        }
        if ( aName.isEmpty() )
        {
            throw lang::IllegalArgumentException( u"Name not set!!"_ustr,
                                                      Reference < XInterface > (), 1 );
        }
        // TODO for library package parse the language, for the moment will try
        // to get each provider to process the new Package, the first one the succeeds
        // will terminate processing
        const bool bSuccess = FindProviderAndApply(
            *providerCache(), [&aName, &aElement](Reference<container::XNameContainer>& xCont) {
                xCont->insertByName(aName, aElement);
                return true;
            });
        if (!bSuccess)
        {
            // No script providers could process the package
            throw lang::IllegalArgumentException( "Failed to register package for " + aName,
                Reference < XInterface > (), 2 );
        }
   }
}


// Revoke Package
void SAL_CALL
MasterScriptProvider::removeByName( const OUString& Name )
{
    if ( !m_bIsPkgMSP )
    {
        if ( !m_xMSPPkg.is() )
        {
            throw RuntimeException( u"PackageMasterScriptProvider is unitialised"_ustr );
        }

        Reference< container::XNameContainer > xCont( m_xMSPPkg, UNO_QUERY_THROW );
        xCont->removeByName( Name );
    }
    else
    {
        if ( Name.isEmpty() )
        {
            throw lang::IllegalArgumentException( u"Name not set!!"_ustr,
                                                      Reference < XInterface > (), 1 );
        }
        // TODO for Script library package url parse the language,
        // for the moment will just try to get each provider to process remove/revoke
        // request, the first one the succeeds will terminate processing
        const bool bSuccess = FindProviderAndApply(
            *providerCache(), [&Name](Reference<container::XNameContainer>& xCont) {
                xCont->removeByName(Name);
                return true;
            });
        if (!bSuccess)
        {
            // No script providers could process the package
            throw lang::IllegalArgumentException( "Failed to revoke package for " + Name,
                                                  Reference < XInterface > (), 1 );
        }

    }
}


void SAL_CALL
MasterScriptProvider::replaceByName( const OUString& /*aName*/, const Any& /*aElement*/ )
{
    // TODO needs implementing
     throw RuntimeException( u"replaceByName not implemented!!!!"_ustr );
}

Any SAL_CALL
MasterScriptProvider::getByName( const OUString& /*aName*/ )
{
    // TODO needs to be implemented
    throw RuntimeException( u"getByName not implemented!!!!"_ustr );
}

sal_Bool SAL_CALL
MasterScriptProvider::hasByName( const OUString& aName )
{
    bool result = false;
    if ( !m_bIsPkgMSP )
    {
        if ( m_xMSPPkg.is() )
        {
            Reference< container::XNameContainer > xCont( m_xMSPPkg, UNO_QUERY_THROW );
            result = xCont->hasByName( aName );
        }
        // If this is a document provider then we shouldn't
        // have a PackageProvider
        else if (!m_xModel.is())
        {
            throw RuntimeException( u"PackageMasterScriptProvider is unitialised"_ustr );
        }

    }
    else
    {
        if ( aName.isEmpty() )
        {
            throw lang::IllegalArgumentException( u"Name not set!!"_ustr,
                                                      Reference < XInterface > (), 1 );
        }
        // TODO for Script library package url parse the language,
        // for the moment will just try to get each provider to see if the
        // package exists in any provider, first one that succeed will
        // terminate the loop
        result = FindProviderAndApply(
            *providerCache(), [&aName](Reference<container::XNameContainer>& xCont) {
                return xCont->hasByName(aName);
            });
    }
    return result;
}


Sequence< OUString > SAL_CALL
MasterScriptProvider::getElementNames(  )
{
    // TODO needs implementing
    throw RuntimeException( u"getElementNames not implemented!!!!"_ustr );
}

Type SAL_CALL
MasterScriptProvider::getElementType(  )
{
    // TODO needs implementing
    Type t;
    return t;
}

sal_Bool SAL_CALL MasterScriptProvider::hasElements(  )
{
    // TODO needs implementing
    throw RuntimeException( u"hasElements not implemented!!!!"_ustr );
}


OUString SAL_CALL MasterScriptProvider::getImplementationName( )
{
    return u"com.sun.star.script.provider.MasterScriptProvider"_ustr;
}

sal_Bool SAL_CALL MasterScriptProvider::supportsService( const OUString& serviceName )
{
    return cppu::supportsService(this, serviceName);
}


Sequence< OUString > SAL_CALL MasterScriptProvider::getSupportedServiceNames( )
{
    return {
        u"com.sun.star.script.provider.MasterScriptProvider"_ustr,
        u"com.sun.star.script.browse.BrowseNode"_ustr,
        u"com.sun.star.script.provider.ScriptProvider"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
scripting_MasterScriptProvider_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new MasterScriptProvider(context));
}

} // namespace func_provider


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

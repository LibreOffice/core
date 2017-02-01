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


#include <comphelper/documentinfo.hxx>

#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <tools/diagnose_ex.h>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>

#include <com/sun/star/uri/XUriReference.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>

#include <com/sun/star/deployment/XPackage.hpp>
#include <com/sun/star/script/browse/BrowseNodeTypes.hpp>
#include <com/sun/star/script/provider/theMasterScriptProviderFactory.hpp>
#include <com/sun/star/script/provider/ScriptFrameworkErrorType.hpp>

#include <util/MiscUtils.hxx>

#include "ActiveMSPList.hxx"
#include "MasterScriptProvider.hxx"
#include "URIHelper.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::document;
using namespace ::sf_misc;

namespace func_provider
{

bool endsWith( const OUString& target,
    const OUString& item )
{
    sal_Int32 index = 0;
    if (  ( index = target.indexOf( item ) ) != -1  &&
       ( index == ( target.getLength() - item.getLength() ) ) )
    {
        return true;
    }
    return false;
}

/* should be available in some central location. */

// XScriptProvider implementation


MasterScriptProvider::MasterScriptProvider( const Reference< XComponentContext > & xContext ):
        m_xContext( xContext ), m_bIsValid( false ), m_bInitialised( false ),
        m_bIsPkgMSP( false ), m_pPCache( nullptr )
{
    ENSURE_OR_THROW( m_xContext.is(), "MasterScriptProvider::MasterScriptProvider: No context available\n" );
    m_xMgr = m_xContext->getServiceManager();
    ENSURE_OR_THROW( m_xMgr.is(), "MasterScriptProvider::MasterScriptProvider: No service manager available\n" );
    m_bIsValid = true;
}


MasterScriptProvider::~MasterScriptProvider()
{
    if ( m_pPCache )
    {
        delete m_pPCache;
    }
    m_pPCache = nullptr;
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
            "MasterScriptProvider::initialize: invalid number of arguments" );
    }

    Sequence< Any > invokeArgs( len );

    if ( len != 0 )
    {
        // check if first parameter is a string
        // if it is, this implies that this is a MSP created
        // with a user or share ctx ( used for browse functionality )

        if ( args[ 0 ] >>= m_sCtxString )
        {
            invokeArgs[ 0  ] = args[ 0 ];
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
                    "The given document does not support embedding scripts into it, and cannot be associated with such a document.",
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

                OUStringBuffer buf;
                buf.append( "MasterScriptProvider::initialize: caught " );
                buf.append( aError.getValueTypeName() );
                buf.append( ":" );

                Exception aException;
                aError >>= aException;
                buf.append     ( aException.Message );
                throw lang::WrappedTargetException( buf.makeStringAndClear(), *this, aError );
            }

            if ( m_xInvocationContext.is() && m_xInvocationContext != m_xModel )
                invokeArgs[ 0 ] <<= m_xInvocationContext;
            else
                invokeArgs[ 0 ] <<= m_sCtxString;
        }

        OUString pkgSpec = "uno_packages";
        sal_Int32 indexOfPkgSpec = m_sCtxString.lastIndexOf( pkgSpec );

        // if contex string ends with "uno_packages"
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
        OUString sPkgCtx =  m_sCtxString.concat( ":uno_packages" );
        location <<= sPkgCtx;

        Reference< provider::XScriptProviderFactory > xFac =
            provider::theMasterScriptProviderFactory::get( m_xContext );

        m_xMSPPkg.set(
            xFac->createScriptProvider( location ), UNO_QUERY_THROW );

    }
    catch ( const Exception& e )
    {
        (void)e;
        SAL_WARN("scripting.provider", "Exception creating MasterScriptProvider for uno_packages in context "
                << m_sCtxString << ": " << e.Message );
    }
}


Reference< provider::XScript >
MasterScriptProvider::getScript( const OUString& scriptURI )
{
    if ( !m_bIsValid )
    {
        throw provider::ScriptFrameworkErrorException(
            "MasterScriptProvider not initialised", Reference< XInterface >(),
            scriptURI, "",
            provider::ScriptFrameworkErrorType::UNKNOWN );
    }

    // need to get the language from the string

    Reference< uri::XUriReferenceFactory > xFac ( uri::UriReferenceFactory::create( m_xContext )  );

    Reference<  uri::XUriReference > uriRef(
        xFac->parse( scriptURI ), UNO_QUERY );

    Reference < uri::XVndSunStarScriptUrl > sfUri( uriRef, UNO_QUERY );

    if ( !uriRef.is() || !sfUri.is() )
    {
        OUString errorMsg = "Incorrect format for Script URI: ";
        errorMsg = errorMsg.concat( scriptURI );
        throw provider::ScriptFrameworkErrorException(
            errorMsg, Reference< XInterface >(),
            scriptURI, "",
            provider::ScriptFrameworkErrorType::UNKNOWN );
    }

    OUString langKey("language");
    OUString locKey("location");

    if ( !sfUri->hasParameter( langKey ) ||
         !sfUri->hasParameter( locKey ) ||
         ( sfUri->getName().isEmpty()  ) )
    {
        OUString errorMsg = "Incorrect format for Script URI: ";
        errorMsg = errorMsg.concat( scriptURI );
        throw provider::ScriptFrameworkErrorException(
            errorMsg, Reference< XInterface >(),
            scriptURI, "",
            provider::ScriptFrameworkErrorType::UNKNOWN );
    }

    OUString language = sfUri->getParameter( langKey );
    OUString location = sfUri->getParameter( locKey );

    // if script us located in uno pkg
    sal_Int32 index = -1;
    OUString pkgTag(":uno_packages");
    // for languages other than basic,  scripts located in uno packages
    // are merged into the user/share location context.
    // For other languages the location attribute in script url has the form
    // location = [user|share]:uno_packages or location :uno_pacakges/xxxx.uno.pkg
    // we need to extract the value of location part from the
    // location attribute of the script, if the script is located in an
    // uno package then that is the location part up to and including
    // ":uno_packages", if the script is not in an uno package then the
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
    // even if its different
    if  (   (   location == "document"
            &&  m_xModel.is()
            )
            ||  ( endsWith( m_sCtxString, location ) )
            ||  ( language == "Basic" )
         )
    {
        Reference< provider::XScriptProvider > xScriptProvider;
        OUStringBuffer buf( 80 );
        buf.append( "com.sun.star.script.provider.ScriptProviderFor");
        buf.append( language );
        OUString serviceName = buf.makeStringAndClear();
        if ( providerCache() )
        {
            try
            {
                xScriptProvider.set(
                    providerCache()->getProvider( serviceName ),
                    UNO_QUERY_THROW );
            }
            catch( const Exception& e )
            {
                throw provider::ScriptFrameworkErrorException(
                    e.Message, Reference< XInterface >(),
                    sfUri->getName(), language,
                    provider::ScriptFrameworkErrorType::NOTSUPPORTED );
            }
        }
        else
        {
            throw provider::ScriptFrameworkErrorException(
                "No LanguageProviders detected",
                Reference< XInterface >(),
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
            xFac_->createScriptProvider( makeAny( location ) ), UNO_QUERY_THROW );
        xScript = xSP->getScript( scriptURI );
    }

    return xScript;
}


ProviderCache*
MasterScriptProvider::providerCache()
{
    if ( !m_pPCache )
    {
        ::osl::MutexGuard aGuard( m_mutex );
        if ( !m_pPCache )
        {
            OUString serviceName1 = "com.sun.star.script.provider.ScriptProviderForBasic";
            Sequence<OUString> blacklist { serviceName1 };

            if ( !m_bIsPkgMSP )
            {
                m_pPCache = new ProviderCache( m_xContext, m_sAargs );
            }
            else
            {
                m_pPCache = new ProviderCache( m_xContext, m_sAargs, blacklist );
            }
        }
    }
    return m_pPCache;
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
    if ( !providerCache() )
        throw RuntimeException( "MasterScriptProvider::getAllProviders, cache not initialised" );
    Sequence< Reference< provider::XScriptProvider > > providers = providerCache()->getAllProviders();

    sal_Int32 size = providers.getLength();
    bool hasPkgs = m_xMSPPkg.is();
    if ( hasPkgs  )
    {
        size++;
    }
    Sequence<  Reference< browse::XBrowseNode > > children( size );
    sal_Int32 provIndex = 0;
    for ( ; provIndex < providers.getLength(); provIndex++ )
    {
        children[ provIndex ].set( providers[ provIndex ], UNO_QUERY );
    }

    if ( hasPkgs  )
    {
        children[ provIndex ].set( m_xMSPPkg, UNO_QUERY );

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


// Register Package
void SAL_CALL
MasterScriptProvider::insertByName( const OUString& aName, const Any& aElement )
{
    if ( !m_bIsPkgMSP )
    {
        if ( m_xMSPPkg.is() )
        {
            Reference< container::XNameContainer > xCont( m_xMSPPkg, UNO_QUERY );
            if ( !xCont.is() )
            {
                throw RuntimeException(
                    "PackageMasterScriptProvider doesn't implement XNameContainer" );
            }
            xCont->insertByName( aName, aElement );
        }
        else
        {
            throw RuntimeException( "PackageMasterScriptProvider is unitialised" );
        }

    }
    else
    {
        Reference< deployment::XPackage > xPkg( aElement, UNO_QUERY );
        if ( !xPkg.is() )
        {
            throw lang::IllegalArgumentException( "Couldn't convert to XPackage",
                                                      Reference < XInterface > (), 2 );
        }
        if ( aName.isEmpty() )
        {
            throw lang::IllegalArgumentException( "Name not set!!",
                                                      Reference < XInterface > (), 1 );
        }
        // TODO for library pacakge parse the language, for the moment will try
        // to get each provider to process the new Package, the first one the succeeds
        // will terminate processing
        if ( !providerCache() )
        {
            throw RuntimeException(
                "insertByName cannot instantiate "
                "child script providers." );
        }
        Sequence < Reference< provider::XScriptProvider > > xSProviders =
            providerCache()->getAllProviders();
        sal_Int32 index = 0;

        for ( ; index < xSProviders.getLength(); index++ )
        {
            Reference< container::XNameContainer > xCont( xSProviders[ index ], UNO_QUERY );
            if ( !xCont.is() )
            {
                continue;
            }
            try
            {
                xCont->insertByName( aName, aElement );
                break;
            }
            catch ( Exception& e )
            {
                SAL_INFO(
                    "scripting.provider", "ignoring Exception " << e.Message);
            }

        }
        if ( index == xSProviders.getLength() )
        {
            // No script providers could process the package
            OUString message = "Failed to register package for ";
            message = message.concat( aName );
            throw lang::IllegalArgumentException( message,
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
        if ( m_xMSPPkg.is() )
        {
            Reference< container::XNameContainer > xCont( m_xMSPPkg, UNO_QUERY );
            if ( !xCont.is() )
            {
                throw RuntimeException(
                    "PackageMasterScriptProvider doesn't implement XNameContainer" );
            }
            xCont->removeByName( Name );
        }
        else
        {
            throw RuntimeException( "PackageMasterScriptProvider is unitialised" );
        }

   }
   else
   {
        if ( Name.isEmpty() )
        {
            throw lang::IllegalArgumentException( "Name not set!!",
                                                      Reference < XInterface > (), 1 );
        }
        // TODO for Script library pacakge url parse the language,
        // for the moment will just try to get each provider to process remove/revoke
        // request, the first one the succeeds will terminate processing

        if ( !providerCache() )
        {
            throw RuntimeException(
                "removeByName() cannot instantiate "
                "child script providers." );
        }
        Sequence < Reference< provider::XScriptProvider > > xSProviders =
            providerCache()->getAllProviders();
        sal_Int32 index = 0;
        for ( ; index < xSProviders.getLength(); index++ )
        {
            Reference< container::XNameContainer > xCont( xSProviders[ index ], UNO_QUERY );
            if ( !xCont.is() )
            {
                continue;
            }
            try
            {
                xCont->removeByName( Name );
                break;
            }
            catch ( Exception& )
            {
            }

        }
        if ( index == xSProviders.getLength() )
        {
            // No script providers could process the package
            OUString message = "Failed to revoke package for ";
            message = message.concat( Name );
            throw lang::IllegalArgumentException( message,
                                                      Reference < XInterface > (), 1 );
        }

    }
}


void SAL_CALL
MasterScriptProvider::replaceByName( const OUString& aName, const Any& aElement )
{
    (void)aName;
    (void)aElement;

    // TODO needs implementing
    if ( true )
    {
        throw RuntimeException( "replaceByName not implemented!!!!" );
    }
}

Any SAL_CALL
MasterScriptProvider::getByName( const OUString& aName )
{
    (void)aName;

    // TODO needs to be implemented
    Any result;
    if ( true )
    {
        throw RuntimeException( "getByName not implemented!!!!" );
    }
    return result;
}

sal_Bool SAL_CALL
MasterScriptProvider::hasByName( const OUString& aName )
{
    bool result = false;
    if ( !m_bIsPkgMSP )
    {
        if ( m_xMSPPkg.is() )
        {
            Reference< container::XNameContainer > xCont( m_xMSPPkg, UNO_QUERY );
            if ( !xCont.is() )
            {
                throw RuntimeException(
                    "PackageMasterScriptProvider doesn't implement XNameContainer" );
            }

            result = xCont->hasByName( aName );
        }
        // If this is a document provider then we shouldn't
        // have a PackageProvider
        else if (!m_xModel.is())
        {
            throw RuntimeException( "PackageMasterScriptProvider is unitialised" );
        }

   }
   else
   {
        if ( aName.isEmpty() )
        {
            throw lang::IllegalArgumentException( "Name not set!!",
                                                      Reference < XInterface > (), 1 );
        }
        // TODO for Script library pacakge url parse the language,
        // for the moment will just try to get each provider to see if the
        // package exists in any provider, first one that succeed will
        // terminate the loop

        if ( !providerCache() )
        {
            throw RuntimeException(
                "removeByName() cannot instantiate "
                "child script providers." );
        }
        Sequence < Reference< provider::XScriptProvider > > xSProviders =
            providerCache()->getAllProviders();
        for ( sal_Int32 index = 0; index < xSProviders.getLength(); index++ )
        {
            Reference< container::XNameContainer > xCont( xSProviders[ index ], UNO_QUERY );
            if ( !xCont.is() )
            {
                continue;
            }
            try
            {
                result = xCont->hasByName( aName );
                if ( result )
                {
                    break;
                }
            }
            catch ( Exception& )
            {
            }

        }
    }
    return result;
}


Sequence< OUString > SAL_CALL
MasterScriptProvider::getElementNames(  )
{
    // TODO needs implementing
    Sequence< OUString >  names;
    if ( true )
    {
        throw RuntimeException( "getElementNames not implemented!!!!" );
    }
    return names;
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
    if ( true )
    {
        throw RuntimeException( "hasElements not implemented!!!!" );
    }
    return false;
}


OUString SAL_CALL MasterScriptProvider::getImplementationName( )
{
    return OUString( "com.sun.star.script.provider.MasterScriptProvider"  );
}

sal_Bool SAL_CALL MasterScriptProvider::supportsService( const OUString& serviceName )
{
    return cppu::supportsService(this, serviceName);
}


Sequence< OUString > SAL_CALL MasterScriptProvider::getSupportedServiceNames( )
{
    OUString names[3];

    names[0] = "com.sun.star.script.provider.MasterScriptProvider";
    names[1] = "com.sun.star.script.browse.BrowseNode";
    names[2] = "com.sun.star.script.provider.ScriptProvider";

    return Sequence< OUString >( names, 3 );
}

} // namespace func_provider


namespace scripting_runtimemgr
{

Reference< XInterface > SAL_CALL sp_create(
    const Reference< XComponentContext > & xCompC )
{
    return static_cast<cppu::OWeakObject *>(new ::func_provider::MasterScriptProvider( xCompC ));
}


Sequence< OUString > sp_getSupportedServiceNames( )
{
    OUString names[3];

    names[0] = "com.sun.star.script.provider.MasterScriptProvider";
    names[1] = "com.sun.star.script.browse.BrowseNode";
    names[2] = "com.sun.star.script.provider.ScriptProvider";

    return Sequence< OUString >( names, 3 );
}


OUString sp_getImplementationName( )
{
    return OUString( "com.sun.star.script.provider.MasterScriptProvider"  );
}

// ***** registration or ScriptingFrameworkURIHelper
Reference< XInterface > SAL_CALL urihelper_create(
    const Reference< XComponentContext > & xCompC )
{
    return static_cast<cppu::OWeakObject *>(
        new ::func_provider::ScriptingFrameworkURIHelper( xCompC ));
}

Sequence< OUString > urihelper_getSupportedServiceNames( )
{
    OUString serviceNameList[] = {
        OUString(
            "com.sun.star.script.provider.ScriptURIHelper" ) };

    Sequence< OUString > serviceNames = Sequence <
        OUString > ( serviceNameList, 1 );

    return serviceNames;
}

OUString urihelper_getImplementationName( )
{
    return OUString(
        "com.sun.star.script.provider.ScriptURIHelper");
}

static const struct cppu::ImplementationEntry s_entries [] =
    {
        {
            sp_create, sp_getImplementationName,
            sp_getSupportedServiceNames, cppu::createSingleComponentFactory,
            nullptr, 0
        },
        {
            urihelper_create,
            urihelper_getImplementationName,
            urihelper_getSupportedServiceNames,
            cppu::createSingleComponentFactory,
            nullptr, 0
        },
        {
            func_provider::mspf_create, func_provider::mspf_getImplementationName,
            func_provider::mspf_getSupportedServiceNames, cppu::createSingleComponentFactory,
            nullptr, 0
        },
        {
            browsenodefactory::bnf_create, browsenodefactory::bnf_getImplementationName,
            browsenodefactory::bnf_getSupportedServiceNames, cppu::createSingleComponentFactory,
            nullptr, 0
        },
        { nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
    };
}


//#### EXPORTED ##############################################################


extern "C"
{
    /**
     * This function is called to get service factories for an implementation.
     *
     * @param pImplName       name of implementation
     * @param pServiceManager a service manager, need for component creation
     * @param pRegistryKey    the registry key for this component, need for persistent
     *                        data
     * @return a component factory
     */
    SAL_DLLPUBLIC_EXPORT void * SAL_CALL scriptframe_component_getFactory(
        const sal_Char * pImplName,
        void * pServiceManager,
        void * pRegistryKey )
    {
        return ::cppu::component_getFactoryHelper( pImplName, pServiceManager,
            pRegistryKey, ::scripting_runtimemgr::s_entries );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

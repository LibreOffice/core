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

#include "basprov.hxx"
#include "basscript.hxx"
#include "baslibnode.hxx"
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/script/browse/BrowseNodeTypes.hpp>
#include <com/sun/star/script/provider/ScriptFrameworkErrorType.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>

#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/uri.hxx>
#include <osl/process.h>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <basic/sbx.hxx>
#include <basic/basmgr.hxx>
#include <basic/basicmanagerrepository.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbmeth.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>

#include <com/sun/star/util/theMacroExpander.hpp>
#include <com/sun/star/script/XLibraryContainer2.hpp>
#include <com/sun/star/uri/XUriReference.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>

#include <util/MiscUtils.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::document;
using namespace ::sf_misc;


namespace basprov
{


    // component operations


    static OUString getImplementationName_BasicProviderImpl()
    {
        return OUString( "com.sun.star.comp.scripting.ScriptProviderForBasic"  );
    }


    static Sequence< OUString > getSupportedServiceNames_BasicProviderImpl()
    {
        static Sequence< OUString >* pNames = nullptr;
        if ( !pNames )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pNames )
            {
                static Sequence< OUString > aNames(4);
                aNames.getArray()[0] = "com.sun.star.script.provider.ScriptProviderForBasic";
                aNames.getArray()[1] = "com.sun.star.script.provider.LanguageScriptProvider";
                aNames.getArray()[2] = "com.sun.star.script.provider.ScriptProvider";
                aNames.getArray()[3] = "com.sun.star.script.browse.BrowseNode";
                pNames = &aNames;
            }
        }
        return *pNames;
    }


    // BasicProviderImpl


    BasicProviderImpl::BasicProviderImpl( const Reference< XComponentContext >& xContext )
        :m_pAppBasicManager( nullptr )
        ,m_pDocBasicManager( nullptr )
        ,m_xLibContainerApp( nullptr )
        ,m_xLibContainerDoc( nullptr )
        ,m_xContext( xContext )
        ,m_bIsAppScriptCtx( true )
        ,m_bIsUserCtx(true)
    {
    }


    BasicProviderImpl::~BasicProviderImpl()
    {
    }


    bool BasicProviderImpl::isLibraryShared( const Reference< script::XLibraryContainer >& rxLibContainer, const OUString& rLibName )
    {
        bool bIsShared = false;

        Reference< script::XLibraryContainer2 > xLibContainer( rxLibContainer, UNO_QUERY );
        if ( xLibContainer.is() && xLibContainer->hasByName( rLibName ) && xLibContainer->isLibraryLink( rLibName ) )
        {
            OUString aFileURL;
            if ( m_xContext.is() )
            {
                Reference< uri::XUriReferenceFactory > xUriFac( uri::UriReferenceFactory::create( m_xContext ) );

                OUString aLinkURL( xLibContainer->getLibraryLinkURL( rLibName ) );
                Reference<  uri::XUriReference > xUriRef( xUriFac->parse( aLinkURL ), UNO_QUERY );

                if ( xUriRef.is() )
                {
                    OUString aScheme = xUriRef->getScheme();
                    if ( aScheme.equalsIgnoreAsciiCase("file") )
                    {
                        aFileURL = aLinkURL;
                    }
                    else if ( aScheme.equalsIgnoreAsciiCase("vnd.sun.star.pkg") )
                    {
                        OUString aAuthority = xUriRef->getAuthority();
                        if ( aAuthority.matchIgnoreAsciiCase( "vnd.sun.star.expand:" ) )
                        {
                            OUString aDecodedURL( aAuthority.copy( sizeof ( "vnd.sun.star.expand:" ) - 1 ) );
                            aDecodedURL = ::rtl::Uri::decode( aDecodedURL, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
                            Reference<util::XMacroExpander> xMacroExpander =
                                util::theMacroExpander::get(m_xContext);
                            aFileURL = xMacroExpander->expandMacros( aDecodedURL );
                        }
                    }
                }
            }

            if ( !aFileURL.isEmpty() )
            {
                osl::DirectoryItem aFileItem;
                osl::FileStatus aFileStatus( osl_FileStatus_Mask_FileURL );
                OSL_VERIFY( osl::DirectoryItem::get( aFileURL, aFileItem ) == osl::FileBase::E_None );
                OSL_VERIFY( aFileItem.getFileStatus( aFileStatus ) == osl::FileBase::E_None );
                OUString aCanonicalFileURL( aFileStatus.getFileURL() );

                OUString aSearchURL1( "share/basic"  );
                OUString aSearchURL2( "share/uno_packages"  );
                if( aCanonicalFileURL.indexOf( aSearchURL1 ) != -1 || aCanonicalFileURL.indexOf( aSearchURL2 ) != -1 )
                    bIsShared = true;
            }
        }

        return bIsShared;
    }

    // XServiceInfo
    OUString BasicProviderImpl::getImplementationName(  )
    {
        return getImplementationName_BasicProviderImpl();
    }

    sal_Bool BasicProviderImpl::supportsService( const OUString& rServiceName )
    {
        return cppu::supportsService(this, rServiceName);
    }

    Sequence< OUString > BasicProviderImpl::getSupportedServiceNames(  )
    {
        return getSupportedServiceNames_BasicProviderImpl();
    }


    // XInitialization


    void BasicProviderImpl::initialize( const Sequence< Any >& aArguments )
    {
        // TODO

        SolarMutexGuard aGuard;

        if ( aArguments.getLength() != 1 )
        {
            throw IllegalArgumentException(
                "BasicProviderImpl::initialize: incorrect argument count.",
                *this,
                1
            );
        }

        Reference< frame::XModel > xModel;

        m_xInvocationContext.set( aArguments[0], UNO_QUERY );
        if ( m_xInvocationContext.is() )
        {
            xModel.set( m_xInvocationContext->getScriptContainer(), UNO_QUERY );
            if ( !xModel.is() )
            {
                throw IllegalArgumentException(
                    "BasicProviderImpl::initialize: unable to determine the document model from the script invocation context.",
                    *this,
                    1
                );
            }
        }
        else
        {
            if ( !( aArguments[0] >>= m_sScriptingContext ) )
            {
                throw IllegalArgumentException(
                    "BasicProviderImpl::initialize: incorrect argument type " + aArguments[0].getValueTypeName(),
                    *this,
                    1
                );
            }

            OUString sDoc = "vnd.sun.star.tdoc";
            if ( m_sScriptingContext.startsWith( sDoc  ) )
            {
                xModel = MiscUtils::tDocUrlToModel(  m_sScriptingContext );
                // TODO: use ScriptingContantsPool for SCRIPTING_DOC_REF
            }
        }

        if ( xModel.is() )
        {
            Reference< XEmbeddedScripts > xDocumentScripts( xModel, UNO_QUERY );
            if ( xDocumentScripts.is() )
            {
                m_pDocBasicManager = ::basic::BasicManagerRepository::getDocumentBasicManager( xModel );
                m_xLibContainerDoc.set( xDocumentScripts->getBasicLibraries(), UNO_QUERY );
                OSL_ENSURE( m_pDocBasicManager && m_xLibContainerDoc.is(),
                    "BasicProviderImpl::initialize: invalid BasicManager, or invalid script container!" );
            }
            m_bIsAppScriptCtx = false;
        }
        else
        {
            // Provider has been created with application context for user
            // or share
            if ( m_sScriptingContext != "user" )
            {
                m_bIsUserCtx = false;
            }
            else
            {
                /*
                throw RuntimeException(
                    "BasicProviderImpl::initialize: no scripting context!" );
                */
            }
        }

        // TODO
        if ( !m_pAppBasicManager )
            m_pAppBasicManager = SfxApplication::GetBasicManager();

        if ( !m_xLibContainerApp.is() )
            m_xLibContainerApp.set( SfxGetpApp()->GetBasicContainer(), UNO_QUERY );
    }


    // XScriptProvider


    Reference < provider::XScript > BasicProviderImpl::getScript( const OUString& scriptURI )
    {
        // TODO

        SolarMutexGuard aGuard;

        Reference< provider::XScript > xScript;
        Reference< uri::XUriReferenceFactory > xFac ( uri::UriReferenceFactory::create( m_xContext )  );

        Reference<  uri::XUriReference > uriRef(
            xFac->parse( scriptURI ), UNO_QUERY );

        Reference < uri::XVndSunStarScriptUrl > sfUri( uriRef, UNO_QUERY );

        if ( !uriRef.is() || !sfUri.is() )
        {
            OUString errorMsg("BasicProviderImpl::getScript: failed to parse URI: ");
            errorMsg = errorMsg.concat( scriptURI );
            throw provider::ScriptFrameworkErrorException(
                errorMsg, Reference< XInterface >(),
                scriptURI, "Basic",
                provider::ScriptFrameworkErrorType::MALFORMED_URL );
        }


        OUString aDescription = sfUri->getName();
        OUString aLocation = sfUri->getParameter( "location" );

        sal_Int32 nIndex = 0;
        // In some strange circumstances the Library name can have an
        // apparently illegal '.' in it ( in imported VBA )

        BasicManager* pBasicMgr =  nullptr;
        if ( aLocation == "document" )
        {
            pBasicMgr = m_pDocBasicManager;
        }
        else if ( aLocation == "application" )
        {
            pBasicMgr = m_pAppBasicManager;
        }
        OUString sProjectName;
        if (  pBasicMgr )
            sProjectName = pBasicMgr->GetName();

        OUString aLibrary;
        if ( !sProjectName.isEmpty() && aDescription.match( sProjectName ) )
        {
            SAL_WARN("scripting", "LibraryName " << sProjectName << " is part of the url " << aDescription );
            aLibrary = sProjectName;
            nIndex = sProjectName.getLength() + 1;
        }
        else
            aLibrary = aDescription.getToken( 0, (sal_Unicode)'.', nIndex );
        OUString aModule;
        if ( nIndex != -1 )
            aModule = aDescription.getToken( 0, (sal_Unicode)'.', nIndex );
        OUString aMethod;
        if ( nIndex != -1 )
            aMethod = aDescription.getToken( 0, (sal_Unicode)'.', nIndex );

        if ( !aLibrary.isEmpty() && !aModule.isEmpty() && !aMethod.isEmpty() && !aLocation.isEmpty() )
        {

            if ( pBasicMgr )
            {
                StarBASIC* pBasic = pBasicMgr->GetLib( aLibrary );
                if ( !pBasic )
                {
                    sal_uInt16 nId = pBasicMgr->GetLibId( aLibrary );
                    if ( nId != LIB_NOTFOUND )
                    {
                        pBasicMgr->LoadLib( nId );
                        pBasic = pBasicMgr->GetLib( aLibrary );
                    }
                }
                if ( pBasic )
                {
                    SbModule* pModule = pBasic->FindModule( aModule );
                    if ( pModule )
                    {
                        SbMethod* pMethod = pModule->FindMethod( aMethod, SbxClassType::Method );
                        if ( pMethod && !pMethod->IsHidden() )
                        {
                            if ( m_pDocBasicManager == pBasicMgr )
                                xScript = new BasicScriptImpl( aDescription, pMethod, *m_pDocBasicManager, m_xInvocationContext );
                            else
                                xScript = new BasicScriptImpl( aDescription, pMethod );
                        }
                    }
                }
            }
        }

        if ( !xScript.is() )
        {
            throw provider::ScriptFrameworkErrorException(
                "The following Basic script could not be found:\n"
                "library: '" + aLibrary + "'\n"
                "module: '" + aModule + "'\n"
                "method: '" + aMethod + "'\n"
                "location: '" + aLocation + "'\n",
                Reference< XInterface >(),
                scriptURI, "Basic",
                provider::ScriptFrameworkErrorType::NO_SUCH_SCRIPT );
        }

        return xScript;
    }


    // XBrowseNode


    OUString BasicProviderImpl::getName(  )
    {
        return OUString("Basic");
    }


    Sequence< Reference< browse::XBrowseNode > > BasicProviderImpl::getChildNodes(  )
    {
        SolarMutexGuard aGuard;

        Reference< script::XLibraryContainer > xLibContainer;
        BasicManager* pBasicManager = nullptr;

        if ( m_bIsAppScriptCtx )
        {
            xLibContainer = m_xLibContainerApp;
            pBasicManager = m_pAppBasicManager;
        }
        else
        {
            xLibContainer = m_xLibContainerDoc;
            pBasicManager = m_pDocBasicManager;
        }

        Sequence< Reference< browse::XBrowseNode > > aChildNodes;

        if ( pBasicManager && xLibContainer.is() )
        {
            Sequence< OUString > aLibNames = xLibContainer->getElementNames();
            sal_Int32 nLibCount = aLibNames.getLength();
            const OUString* pLibNames = aLibNames.getConstArray();
            aChildNodes.realloc( nLibCount );
            Reference< browse::XBrowseNode >* pChildNodes = aChildNodes.getArray();
            sal_Int32 childrenFound = 0;

            for ( sal_Int32 i = 0 ; i < nLibCount ; ++i )
            {
                bool bCreate = false;
                if ( m_bIsAppScriptCtx )
                {
                    bool bShared = isLibraryShared( xLibContainer, pLibNames[i] );
                    if ( ( m_bIsUserCtx && !bShared ) || ( !m_bIsUserCtx && bShared ) )
                        bCreate = true;
                }
                else
                {
                    bCreate = true;
                }
                if ( bCreate )
                {
                    pChildNodes[childrenFound++] = static_cast< browse::XBrowseNode* >( new BasicLibraryNodeImpl(
                        m_xContext, m_sScriptingContext, pBasicManager, xLibContainer, pLibNames[i], m_bIsAppScriptCtx ) );
                }
            }

            if ( childrenFound != nLibCount )
                aChildNodes.realloc( childrenFound );
        }

        return aChildNodes;
    }


    sal_Bool BasicProviderImpl::hasChildNodes(  )
    {
        SolarMutexGuard aGuard;

        bool bReturn = false;
        Reference< script::XLibraryContainer > xLibContainer;
        if ( m_bIsAppScriptCtx )
        {
            xLibContainer = m_xLibContainerApp;
        }
        else
        {
             xLibContainer = m_xLibContainerDoc;
        }
        if ( xLibContainer.is() )
            bReturn = xLibContainer->hasElements();

        return bReturn;
    }


    sal_Int16 BasicProviderImpl::getType(  )
    {
        return browse::BrowseNodeTypes::CONTAINER;
    }


    // component operations


    static Reference< XInterface > SAL_CALL create_BasicProviderImpl(
        Reference< XComponentContext > const & xContext )
    {
        return static_cast< lang::XTypeProvider * >( new BasicProviderImpl( xContext ) );
    }


    static struct ::cppu::ImplementationEntry s_component_entries [] =
    {
        {
            create_BasicProviderImpl, getImplementationName_BasicProviderImpl,
            getSupportedServiceNames_BasicProviderImpl, ::cppu::createSingleComponentFactory,
            nullptr, 0
        },
        { nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
    };


}   // namespace basprov


// component exports


extern "C"
{
    SAL_DLLPUBLIC_EXPORT void * SAL_CALL basprov_component_getFactory(
        const sal_Char * pImplName, void * pServiceManager,
        void * pRegistryKey )
    {
        return ::cppu::component_getFactoryHelper(
            pImplName, pServiceManager, pRegistryKey, ::basprov::s_component_entries );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

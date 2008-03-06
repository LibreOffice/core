/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basprov.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 16:17:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_scripting.hxx"

#ifndef SCRIPTING_BASPROV_HXX
#include "basprov.hxx"
#endif

#ifndef SCRIPTING_BASSCRIPT_HXX
#include "basscript.hxx"
#endif
#ifndef SCRIPTING_BASLIBNODE_HXX
#include "baslibnode.hxx"
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_BROWSE_BROWSENODETYPES_HPP_
#include <com/sun/star/script/browse/BrowseNodeTypes.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_PROVIDER_SCRIPTFRAMEWORKERRORTYPE_HPP_
#include <com/sun/star/script/provider/ScriptFrameworkErrorType.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEMBEDDEDSCRIPTS_HPP_
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#endif

#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif

#ifndef _RTL_URI_HXX_
#include <rtl/uri.hxx>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SBXCLASS_HXX
#include <basic/sbx.hxx>
#endif
#ifndef _BASMGR_HXX
#include <basic/basmgr.hxx>
#endif
#ifndef BASICMANAGERREPOSITORY_HXX
#include <basic/basicmanagerrepository.hxx>
#endif
#ifndef _SB_SBSTAR_HXX
#include <basic/sbstar.hxx>
#endif
#ifndef _SB_SBMOD_HXX
#include <basic/sbmod.hxx>
#endif
#ifndef _SB_SBMETH_HXX
#include <basic/sbmeth.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif

#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/script/XLibraryContainer2.hpp>
#include <com/sun/star/uri/XUriReference.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>

#include <util/util.hxx>
#include <util/MiscUtils.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::document;
using namespace ::sf_misc;

//.........................................................................
namespace basprov
{
//.........................................................................

    // =============================================================================
    // component operations
    // =============================================================================

    static ::rtl::OUString getImplementationName_BasicProviderImpl()
    {
        static ::rtl::OUString* pImplName = 0;
        if ( !pImplName )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pImplName )
            {
                static ::rtl::OUString aImplName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.scripting.ScriptProviderForBasic" ) );
                pImplName = &aImplName;
            }
        }
        return *pImplName;
    }

    // -----------------------------------------------------------------------------

    static Sequence< ::rtl::OUString > getSupportedServiceNames_BasicProviderImpl()
    {
        static Sequence< ::rtl::OUString >* pNames = 0;
        if ( !pNames )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pNames )
            {
                static Sequence< ::rtl::OUString > aNames(4);
                aNames.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.script.provider.ScriptProviderForBasic" ) );
                aNames.getArray()[1] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.script.provider.LanguageScriptProvider" ) );
                aNames.getArray()[2] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.script.provider.ScriptProvider" ) );
                aNames.getArray()[3] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.script.browse.BrowseNode" ) );
                pNames = &aNames;
            }
        }
        return *pNames;
    }

    // =============================================================================
    // BasicProviderImpl
    // =============================================================================

    BasicProviderImpl::BasicProviderImpl( const Reference< XComponentContext >& xContext )
        :m_pAppBasicManager( 0 )
        ,m_pDocBasicManager( 0 )
        ,m_xLibContainerApp( 0 )
        ,m_xLibContainerDoc( 0 )
        ,m_xContext( xContext )
        ,m_bIsAppScriptCtx( true )
        ,m_bIsUserCtx(true)
    {
    }

    // -----------------------------------------------------------------------------

    BasicProviderImpl::~BasicProviderImpl()
    {
    }

    // -----------------------------------------------------------------------------

    bool BasicProviderImpl::isLibraryShared( const Reference< script::XLibraryContainer >& rxLibContainer, const ::rtl::OUString& rLibName )
    {
        bool bIsShared = false;

        Reference< script::XLibraryContainer2 > xLibContainer( rxLibContainer, UNO_QUERY );
        if ( xLibContainer.is() && xLibContainer->hasByName( rLibName ) && xLibContainer->isLibraryLink( rLibName ) )
        {
            ::rtl::OUString aFileURL;
            if ( m_xContext.is() )
            {
                Reference< uri::XUriReferenceFactory > xUriFac;
                Reference< lang::XMultiComponentFactory > xSMgr( m_xContext->getServiceManager() );
                if ( xSMgr.is() )
                {
                    xUriFac.set( xSMgr->createInstanceWithContext( ::rtl::OUString::createFromAscii(
                        "com.sun.star.uri.UriReferenceFactory" ), m_xContext ), UNO_QUERY );
                }

                if ( xUriFac.is() )
                {
                    ::rtl::OUString aLinkURL( xLibContainer->getLibraryLinkURL( rLibName ) );
                    Reference<  uri::XUriReference > xUriRef( xUriFac->parse( aLinkURL ), UNO_QUERY );

                    if ( xUriRef.is() )
                    {
                        ::rtl::OUString aScheme = xUriRef->getScheme();
                        if ( aScheme.equalsIgnoreAsciiCaseAscii( "file" ) )
                        {
                            aFileURL = aLinkURL;
                        }
                        else if ( aScheme.equalsIgnoreAsciiCaseAscii( "vnd.sun.star.pkg" ) )
                        {
                            ::rtl::OUString aAuthority = xUriRef->getAuthority();
                            if ( aAuthority.matchIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "vnd.sun.star.expand:" ) ) )
                            {
                                ::rtl::OUString aDecodedURL( aAuthority.copy( sizeof ( "vnd.sun.star.expand:" ) - 1 ) );
                                aDecodedURL = ::rtl::Uri::decode( aDecodedURL, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
                                Reference<util::XMacroExpander> xMacroExpander(
                                    m_xContext->getValueByName(
                                    ::rtl::OUString::createFromAscii( "/singletons/com.sun.star.util.theMacroExpander" ) ),
                                    UNO_QUERY );
                                if ( xMacroExpander.is() )
                                    aFileURL = xMacroExpander->expandMacros( aDecodedURL );
                            }
                        }
                    }
                }
            }

            if ( aFileURL.getLength() )
            {
                osl::DirectoryItem aFileItem;
                osl::FileStatus aFileStatus( FileStatusMask_FileURL );
                OSL_VERIFY( osl::DirectoryItem::get( aFileURL, aFileItem ) == osl::FileBase::E_None );
                OSL_VERIFY( aFileItem.getFileStatus( aFileStatus ) == osl::FileBase::E_None );
                ::rtl::OUString aCanonicalFileURL( aFileStatus.getFileURL() );

                ::rtl::OUString aShareURL;
                OSL_VERIFY( osl_getExecutableFile( &aShareURL.pData ) == osl_Process_E_None );
                sal_Int32 nIndex = aShareURL.lastIndexOf( '/' );
                if ( nIndex >= 0 )
                {
                    nIndex = aShareURL.lastIndexOf( '/', nIndex );
                    if ( nIndex >= 0 )
                    {
                        aShareURL = aShareURL.copy( 0, nIndex + 1 );
                        aShareURL += ::rtl::OUString::createFromAscii( "share" );
                    }
                }

                osl::DirectoryItem aShareItem;
                osl::FileStatus aShareStatus( FileStatusMask_FileURL );
                OSL_VERIFY( osl::DirectoryItem::get( aShareURL, aShareItem ) == osl::FileBase::E_None );
                OSL_VERIFY( aShareItem.getFileStatus( aShareStatus ) == osl::FileBase::E_None );
                ::rtl::OUString aCanonicalShareURL( aShareStatus.getFileURL() );

                if ( aCanonicalFileURL.match( aCanonicalShareURL ) )
                    bIsShared = true;
            }
        }

        return bIsShared;
    }

    // -----------------------------------------------------------------------------
    // XServiceInfo
    // -----------------------------------------------------------------------------

    ::rtl::OUString BasicProviderImpl::getImplementationName(  ) throw (RuntimeException)
    {
        return getImplementationName_BasicProviderImpl();
    }

    // -----------------------------------------------------------------------------

    sal_Bool BasicProviderImpl::supportsService( const ::rtl::OUString& rServiceName ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aNames( getSupportedServiceNames() );
        const ::rtl::OUString* pNames = aNames.getConstArray();
        const ::rtl::OUString* pEnd = pNames + aNames.getLength();
        for ( ; pNames != pEnd && !pNames->equals( rServiceName ); ++pNames )
            ;

        return pNames != pEnd;
    }

    // -----------------------------------------------------------------------------

    Sequence< ::rtl::OUString > BasicProviderImpl::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        return getSupportedServiceNames_BasicProviderImpl();
    }

    // -----------------------------------------------------------------------------
    // XInitialization
    // -----------------------------------------------------------------------------

    void BasicProviderImpl::initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException)
    {
        // TODO

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        if ( aArguments.getLength() != 1 )
        {
            throw IllegalArgumentException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BasicProviderImpl::initialize: incorrect argument count." ) ),
                *this,
                1
            );
        }

        Reference< frame::XModel > xModel;

        m_xInvocationContext.set( aArguments[0], UNO_QUERY );;
        if ( m_xInvocationContext.is() )
        {
            xModel.set( m_xInvocationContext->getScriptContainer(), UNO_QUERY );
            if ( !xModel.is() )
            {
                throw IllegalArgumentException(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BasicProviderImpl::initialize: unable to determine the document model from the script invocation context." ) ),
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
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BasicProviderImpl::initialize: incorrect argument type " )  ).concat(  aArguments[0].getValueTypeName() ),
                    *this,
                    1
                );
            }

            ::rtl::OUString sDoc = OUSTR("vnd.sun.star.tdoc");
            if ( m_sScriptingContext.indexOf( sDoc  ) == 0 )
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
            if ( !m_sScriptingContext.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "user" ) ) )
            {
                m_bIsUserCtx = false;
            }
            else
            {
                /*
                throw RuntimeException(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BasicProviderImpl::initialize: no scripting context!" ) ),
                    Reference< XInterface >() );
                */
            }
        }

        // TODO
        if ( !m_pAppBasicManager )
            m_pAppBasicManager = SFX_APP()->GetBasicManager();

        if ( !m_xLibContainerApp.is() )
            m_xLibContainerApp = Reference< script::XLibraryContainer >( SFX_APP()->GetBasicContainer(), UNO_QUERY );
    }

    // -----------------------------------------------------------------------------

    // XScriptProvider
    // -----------------------------------------------------------------------------

    Reference < provider::XScript > BasicProviderImpl::getScript( const ::rtl::OUString& scriptURI )
        throw ( provider::ScriptFrameworkErrorException, RuntimeException)
    {
        // TODO

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        Reference< provider::XScript > xScript;
        Reference< lang::XMultiComponentFactory > xMcFac ( m_xContext->getServiceManager() );
        Reference< uri::XUriReferenceFactory > xFac (
            xMcFac->createInstanceWithContext( rtl::OUString::createFromAscii(
            "com.sun.star.uri.UriReferenceFactory"), m_xContext ) , UNO_QUERY );

        if ( !xFac.is() )
        {
            throw provider::ScriptFrameworkErrorException(
                OUSTR( "Failed to instantiate UriReferenceFactory" ), Reference< XInterface >(),
                scriptURI, OUSTR("Basic"),
                provider::ScriptFrameworkErrorType::UNKNOWN );
        }

        Reference<  uri::XUriReference > uriRef(
            xFac->parse( scriptURI ), UNO_QUERY );

        Reference < uri::XVndSunStarScriptUrl > sfUri( uriRef, UNO_QUERY );

        if ( !uriRef.is() || !sfUri.is() )
        {
            ::rtl::OUString errorMsg = ::rtl::OUString::createFromAscii( "BasicProviderImpl::getScript: failed to parse URI: " );
            errorMsg = errorMsg.concat( scriptURI );
            throw provider::ScriptFrameworkErrorException(
                errorMsg, Reference< XInterface >(),
                scriptURI, OUSTR("Basic"),
                provider::ScriptFrameworkErrorType::UNKNOWN );
        }


        ::rtl::OUString aDescription = sfUri->getName();
        ::rtl::OUString aLocation = sfUri->getParameter(
            ::rtl::OUString::createFromAscii( "location" ) );

        sal_Int32 nIndex = 0;
        ::rtl::OUString aLibrary = aDescription.getToken( 0, (sal_Unicode)'.', nIndex );
        ::rtl::OUString aModule;
        if ( nIndex != -1 )
            aModule = aDescription.getToken( 0, (sal_Unicode)'.', nIndex );
        ::rtl::OUString aMethod;
        if ( nIndex != -1 )
            aMethod = aDescription.getToken( 0, (sal_Unicode)'.', nIndex );

        if ( aLibrary.getLength() != 0 && aModule.getLength() != 0 && aMethod.getLength() != 0 && aLocation.getLength() != 0 )
        {
            BasicManager* pBasicMgr =  NULL;
            if ( aLocation.equals( ::rtl::OUString::createFromAscii("document") ) )
            {
                pBasicMgr = m_pDocBasicManager;
            }
            else if ( aLocation.equals( ::rtl::OUString::createFromAscii("application") ) )
            {
                pBasicMgr = m_pAppBasicManager;
            }

            if ( pBasicMgr )
            {
                StarBASIC* pBasic = pBasicMgr->GetLib( aLibrary );
                if ( !pBasic )
                {
                    USHORT nId = pBasicMgr->GetLibId( aLibrary );
                    pBasicMgr->LoadLib( nId );
                    pBasic = pBasicMgr->GetLib( aLibrary );
                }
                if ( pBasic )
                {
                    SbModule* pModule = pBasic->FindModule( aModule );
                    if ( pModule )
                    {
                        SbxArray* pMethods = pModule->GetMethods();
                        if ( pMethods )
                        {
                            SbMethod* pMethod = static_cast< SbMethod* >( pMethods->Find( aMethod, SbxCLASS_METHOD ) );
                            if ( pMethod )
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
        }

        if ( !xScript.is() )
        {
            ::rtl::OUStringBuffer aMessage;
            aMessage.appendAscii( "The following Basic script could not be found:\n" );
            aMessage.appendAscii( "library: '" ).append( aLibrary ).appendAscii( "'\n" );
            aMessage.appendAscii( "module: '" ).append( aModule ).appendAscii( "'\n" );
            aMessage.appendAscii( "method: '" ).append( aMethod ).appendAscii( "'\n" );
            aMessage.appendAscii( "location: '" ).append( aLocation ).appendAscii( "'\n" );
            throw provider::ScriptFrameworkErrorException(
                aMessage.makeStringAndClear(),
                Reference< XInterface >(),
                scriptURI, OUSTR("Basic"),
                provider::ScriptFrameworkErrorType::UNKNOWN );
        }

        return xScript;
    }

    // -----------------------------------------------------------------------------
    // XBrowseNode
    // -----------------------------------------------------------------------------

    ::rtl::OUString BasicProviderImpl::getName(  ) throw (RuntimeException)
    {
        // TODO

        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        return ::rtl::OUString::createFromAscii( "Basic" );
    }

    // -----------------------------------------------------------------------------

    Sequence< Reference< browse::XBrowseNode > > BasicProviderImpl::getChildNodes(  ) throw (RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        Reference< script::XLibraryContainer > xLibContainer;
        BasicManager* pBasicManager = NULL;

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
            Sequence< ::rtl::OUString > aLibNames = xLibContainer->getElementNames();
            sal_Int32 nLibCount = aLibNames.getLength();
            const ::rtl::OUString* pLibNames = aLibNames.getConstArray();
            aChildNodes.realloc( nLibCount );
            Reference< browse::XBrowseNode >* pChildNodes = aChildNodes.getArray();
            sal_Int32 childsFound = 0;

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
                    pChildNodes[childsFound++] = static_cast< browse::XBrowseNode* >( new BasicLibraryNodeImpl(
                        m_xContext, m_sScriptingContext, pBasicManager, xLibContainer, pLibNames[i], m_bIsAppScriptCtx ) );
                }
            }

            if ( childsFound != nLibCount )
                aChildNodes.realloc( childsFound );
        }

        return aChildNodes;
    }

    // -----------------------------------------------------------------------------

    sal_Bool BasicProviderImpl::hasChildNodes(  ) throw (RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        sal_Bool bReturn = sal_False;
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

    // -----------------------------------------------------------------------------

    sal_Int16 BasicProviderImpl::getType(  ) throw (RuntimeException)
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        return browse::BrowseNodeTypes::CONTAINER;
    }

    // =============================================================================
    // component operations
    // =============================================================================

    static Reference< XInterface > SAL_CALL create_BasicProviderImpl(
        Reference< XComponentContext > const & xContext )
        SAL_THROW( () )
    {
        return static_cast< lang::XTypeProvider * >( new BasicProviderImpl( xContext ) );
    }

    // -----------------------------------------------------------------------------

    static struct ::cppu::ImplementationEntry s_component_entries [] =
    {
        {
            create_BasicProviderImpl, getImplementationName_BasicProviderImpl,
            getSupportedServiceNames_BasicProviderImpl, ::cppu::createSingleComponentFactory,
            0, 0
        },
        { 0, 0, 0, 0, 0, 0 }
    };

    // -----------------------------------------------------------------------------

//.........................................................................
}   // namespace basprov
//.........................................................................


// =============================================================================
// component exports
// =============================================================================

extern "C"
{
    void SAL_CALL component_getImplementationEnvironment(
        const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
    {
        (void)ppEnv;

        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    sal_Bool SAL_CALL component_writeInfo(
        lang::XMultiServiceFactory * pServiceManager, registry::XRegistryKey * pRegistryKey )
    {
        return ::cppu::component_writeInfoHelper(
            pServiceManager, pRegistryKey, ::basprov::s_component_entries );
    }

    void * SAL_CALL component_getFactory(
        const sal_Char * pImplName, lang::XMultiServiceFactory * pServiceManager,
        registry::XRegistryKey * pRegistryKey )
    {
        return ::cppu::component_getFactoryHelper(
            pImplName, pServiceManager, pRegistryKey, ::basprov::s_component_entries );
    }
}

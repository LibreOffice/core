/*************************************************************************
 *
 *  $RCSfile: basprov.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: tbe $ $Date: 2003-09-23 15:06:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
#ifndef _DRAFTS_COM_SUN_STAR_SCRIPT_FRAMEWORK_BROWSE_BROWSENODETYPES_HPP_
#include <drafts/com/sun/star/script/framework/browse/BrowseNodeTypes.hpp>
#endif
#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif

#ifndef _SBXCLASS_HXX
#include <svtools/sbx.hxx>
#endif
#ifndef _BASMGR_HXX
#include <basic/basmgr.hxx>
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


using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::script::framework;


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
                static Sequence< ::rtl::OUString > aNames(3);
                aNames.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "drafts.com.sun.star.script.framework.provider.ScriptProviderForBasic" ) );
                aNames.getArray()[1] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "drafts.com.sun.star.script.framework.provider.LanguageScriptProvider" ) );
                aNames.getArray()[2] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "drafts.com.sun.star.script.framework.provider.ScriptProvider" ) );
                pNames = &aNames;
            }
        }
        return *pNames;
    }

    // =============================================================================
    // BasicProviderImpl
    // =============================================================================

    BasicProviderImpl::BasicProviderImpl( const Reference< XComponentContext >& xContext )
        :m_pBasicManager( 0 )
        ,m_xLibContainer( 0 )
        ,m_xContext( xContext )
        ,m_xScriptingContext( 0 )
    {
    }

    // -----------------------------------------------------------------------------

    BasicProviderImpl::~BasicProviderImpl()
    {
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

        ::osl::MutexGuard aGuard( StarBASIC::GetGlobalMutex() );

        if ( aArguments.getLength() == 1 )
        {
            aArguments[0] >>= m_xScriptingContext;

            if ( m_xScriptingContext.is() )
            {
                Reference< frame::XModel > xModel;
                // TODO: use ScriptingContantsPool for SCRIPTING_DOC_REF
                m_xScriptingContext->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SCRIPTING_DOC_REF" ) ) ) >>= xModel;

                if ( xModel.is() )
                {
                    for ( SfxObjectShell* pObjShell = SfxObjectShell::GetFirst(); pObjShell; pObjShell = SfxObjectShell::GetNext( *pObjShell ) )
                    {
                        if ( xModel == pObjShell->GetModel() )
                        {
                            m_pBasicManager = pObjShell->GetBasicManager();
                            m_xLibContainer = Reference< script::XLibraryContainer >( pObjShell->GetBasicContainer(), UNO_QUERY );
                            break;
                        }
                    }
                }
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
        else
        {
            /*
            throw RuntimeException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BasicProviderImpl::initialize: incorrect number of arguments!" ) ),
                Reference< XInterface >() );
            */
        }

        // TODO
        if ( !m_pBasicManager )
            m_pBasicManager = SFX_APP()->GetBasicManager();

        if ( !m_xLibContainer.is() )
            m_xLibContainer = Reference< script::XLibraryContainer >( SFX_APP()->GetBasicContainer(), UNO_QUERY );
    }

    // -----------------------------------------------------------------------------
    // XScriptProvider
    // -----------------------------------------------------------------------------

    Reference < provider::XScript > BasicProviderImpl::getScript( const ::rtl::OUString& scriptURI )
        throw (IllegalArgumentException, RuntimeException)
    {
        // TODO

        ::osl::MutexGuard aGuard( StarBASIC::GetGlobalMutex() );

        Reference< provider::XScript > xScript;

        // parse scriptURI
        // TODO: use URI parsing class
        ::rtl::OUString aSchema( ::rtl::OUString::createFromAscii( "script://" ) );
        sal_Int32 nSchemaLen = aSchema.getLength();
        sal_Int32 nLen = scriptURI.indexOf( '?' );
        ::rtl::OUString aDescription;
        if ( nLen - nSchemaLen > 0 )
            aDescription = scriptURI.copy( nSchemaLen, nLen - nSchemaLen );
        sal_Int32 nIndex = 0;
        ::rtl::OUString aLibrary = aDescription.getToken( 0, (sal_Unicode)'.', nIndex );
        ::rtl::OUString aModule;
        if ( nIndex != -1 )
            aModule = aDescription.getToken( 0, (sal_Unicode)'.', nIndex );
        ::rtl::OUString aMethod;
        if ( nIndex != -1 )
            aMethod = aDescription.getToken( 0, (sal_Unicode)'.', nIndex );

        if ( aLibrary.getLength() != 0 && aModule.getLength() != 0 && aMethod.getLength() != 0 )
        {
            if ( m_pBasicManager )
            {
                StarBASIC* pBasic = m_pBasicManager->GetLib( aLibrary );
                if ( !pBasic )
                {
                    USHORT nId = m_pBasicManager->GetLibId( aLibrary );
                    m_pBasicManager->LoadLib( nId );
                    pBasic = m_pBasicManager->GetLib( aLibrary );
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
                                xScript = static_cast< provider::XScript* >( new BasicScriptImpl( pMethod ) );
                        }
                    }
                }
            }
        }

        if ( !xScript.is() )
        {
            throw IllegalArgumentException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BasicProviderImpl::getScript: no script!" ) ),
                Reference< XInterface >(), 1 );
        }

        return xScript;
    }

    // -----------------------------------------------------------------------------
    // XBrowseNode
    // -----------------------------------------------------------------------------

    ::rtl::OUString BasicProviderImpl::getName(  ) throw (RuntimeException)
    {
        // TODO

        ::osl::MutexGuard aGuard( StarBASIC::GetGlobalMutex() );

        return ::rtl::OUString::createFromAscii( "Basic" );
    }

    // -----------------------------------------------------------------------------

    Sequence< Reference< browse::XBrowseNode > > BasicProviderImpl::getChildNodes(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( StarBASIC::GetGlobalMutex() );

        Sequence< Reference< browse::XBrowseNode > > aChildNodes;

        if ( m_pBasicManager && m_xLibContainer.is() )
        {
            Sequence< ::rtl::OUString > aLibNames = m_xLibContainer->getElementNames();
            sal_Int32 nLibCount = aLibNames.getLength();
            const ::rtl::OUString* pLibNames = aLibNames.getConstArray();
            aChildNodes.realloc( nLibCount );
            Reference< browse::XBrowseNode >* pChildNodes = aChildNodes.getArray();

            for ( sal_Int32 i = 0 ; i < nLibCount ; ++i )
            {
                pChildNodes[i] = static_cast< browse::XBrowseNode* >( new BasicLibraryNodeImpl( m_pBasicManager, m_xLibContainer, pLibNames[i] ) );
            }
        }

        return aChildNodes;
    }

    // -----------------------------------------------------------------------------

    sal_Bool BasicProviderImpl::hasChildNodes(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( StarBASIC::GetGlobalMutex() );

        sal_Bool bReturn = sal_False;
        if ( m_xLibContainer.is() )
            bReturn = m_xLibContainer->hasElements();

        return bReturn;
    }

    // -----------------------------------------------------------------------------

    sal_Int16 BasicProviderImpl::getType(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( StarBASIC::GetGlobalMutex() );

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

/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ucb.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:16:06 $
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

#ifndef _UCB_HXX
#define _UCB_HXX

#ifndef _COM_SUN_STAR_UCB_XCOMMANDPROCESSOR_HPP_
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XContentProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTIDENTIFIERFACTORY_HPP_
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDERMANAGER_HPP_
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _OSL_INTERLOCK_H_ //@@@ see initialize() method
#include <osl/interlck.h>
#endif

#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif

#ifndef _UCB_PROVIDERMAP_HXX_
#include "providermap.hxx"
#endif

//=========================================================================

#define UCB_SERVICE_NAME "com.sun.star.ucb.UniversalContentBroker"

//=========================================================================

namespace cppu { class OInterfaceContainerHelper; }

namespace com { namespace sun { namespace star { namespace ucb {
    class XCommandInfo;
    struct GlobalTransferCommandArgument;
} } } }

class UniversalContentBroker :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::lang::XComponent,
                public com::sun::star::lang::XServiceInfo,
                public com::sun::star::lang::XInitialization,
                public com::sun::star::ucb::XContentProviderManager,
                public com::sun::star::ucb::XContentProvider,
                public com::sun::star::ucb::XContentIdentifierFactory,
                public com::sun::star::ucb::XCommandProcessor
{
public:
    UniversalContentBroker( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rXSMgr );
    virtual ~UniversalContentBroker();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    XSERVICEINFO_DECL()

    // XComponent
    virtual void SAL_CALL
    dispose()
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    addEventListener( const com::sun::star::uno::Reference<
                        com::sun::star::lang::XEventListener >& Listener )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    removeEventListener( const com::sun::star::uno::Reference<
                            com::sun::star::lang::XEventListener >& Listener )
        throw( com::sun::star::uno::RuntimeException );

    // XInitialization
    virtual void SAL_CALL
    initialize( const com::sun::star::uno::Sequence<
                        com::sun::star::uno::Any >& aArguments )
        throw( com::sun::star::uno::Exception,
               com::sun::star::uno::RuntimeException );

    // XContentProviderManager
    virtual com::sun::star::uno::Reference<
        com::sun::star::ucb::XContentProvider > SAL_CALL
    registerContentProvider( const com::sun::star::uno::Reference<
                                com::sun::star::ucb::XContentProvider >&
                                    Provider,
                             const rtl::OUString& Scheme,
                             sal_Bool ReplaceExisting )
        throw( com::sun::star::ucb::DuplicateProviderException,
               com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    deregisterContentProvider( const com::sun::star::uno::Reference<
                                    com::sun::star::ucb::XContentProvider >&
                                        Provider,
                               const rtl::OUString& Scheme )
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence<
        com::sun::star::ucb::ContentProviderInfo > SAL_CALL
    queryContentProviders()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Reference<
        com::sun::star::ucb::XContentProvider > SAL_CALL
    queryContentProvider( const rtl::OUString& Identifier )
        throw( com::sun::star::uno::RuntimeException );

    // XContentProvider
    virtual com::sun::star::uno::Reference<
        com::sun::star::ucb::XContent > SAL_CALL
    queryContent( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XContentIdentifier >& Identifier )
        throw( com::sun::star::ucb::IllegalIdentifierException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Int32 SAL_CALL
    compareContentIds( const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XContentIdentifier >& Id1,
                       const com::sun::star::uno::Reference<
                           com::sun::star::ucb::XContentIdentifier >& Id2 )
        throw( com::sun::star::uno::RuntimeException );

    // XContentIdentifierFactory
    virtual com::sun::star::uno::Reference<
        com::sun::star::ucb::XContentIdentifier > SAL_CALL
    createContentIdentifier( const rtl::OUString& ContentId )
        throw( com::sun::star::uno::RuntimeException );

    // XCommandProcessor
    virtual sal_Int32 SAL_CALL
    createCommandIdentifier()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Any SAL_CALL
    execute( const com::sun::star::ucb::Command& aCommand,
             sal_Int32 CommandId,
             const com::sun::star::uno::Reference<
                 com::sun::star::ucb::XCommandEnvironment >& Environment )
        throw( com::sun::star::uno::Exception,
               com::sun::star::ucb::CommandAbortedException,
               com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    abort( sal_Int32 CommandId )
        throw( com::sun::star::uno::RuntimeException );

private:
    com::sun::star::uno::Reference< com::sun::star::ucb::XContentProvider >
    queryContentProvider( const rtl::OUString& Identifier,
                          sal_Bool bResolved );

    com::sun::star::uno::Reference< com::sun::star::ucb::XCommandInfo >
    getCommandInfo();

    void
    globalTransfer(
            const com::sun::star::ucb::GlobalTransferCommandArgument & rArg,
            const com::sun::star::uno::Reference<
                 com::sun::star::ucb::XCommandEnvironment >& xEnv )
        throw( com::sun::star::uno::Exception );

    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > m_xSMgr;
    ProviderMap_Impl m_aProviders;
    osl::Mutex m_aMutex;
    cppu::OInterfaceContainerHelper* m_pDisposeEventListeners;
    oslInterlockedCount m_nInitCount; //@@@ see initialize() method
    sal_Int32 m_nCommandId;
};

#endif /* !_UCB_HXX */

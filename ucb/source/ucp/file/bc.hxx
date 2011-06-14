/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _BC_HXX_
#define _BC_HXX_

#include "osl/mutex.hxx"
#include "rtl/ustring.hxx"
#include <cppuhelper/weak.hxx>
#include <ucbhelper/macros.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/beans/XPropertiesChangeNotifier.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/XPropertySetInfoChangeNotifier.hpp>
#include <com/sun/star/beans/XPropertySetInfoChangeListener.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/ucb/XContentCreator.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/InteractiveBadTransferURLException.hpp>
#include "shell.hxx"


namespace fileaccess {

    class PropertyListeners;
    class shell;
    class FileProvider;

    class BaseContent:
        public cppu::OWeakObject,
        public com::sun::star::lang::XComponent,
        public com::sun::star::lang::XServiceInfo,
        public com::sun::star::lang::XTypeProvider,
        public com::sun::star::ucb::XCommandProcessor,
        public com::sun::star::beans::XPropertiesChangeNotifier,
        public com::sun::star::beans::XPropertyContainer,
        public com::sun::star::beans::XPropertySetInfoChangeNotifier,
        public com::sun::star::ucb::XContentCreator,
        public com::sun::star::container::XChild,
        public com::sun::star::ucb::XContent,
        public fileaccess::Notifier    // implementation class
    {
    private:

        // A special creator for inserted contents; Creates an ugly object
        BaseContent( shell* pMyShell,
                     const rtl::OUString& parentName,
                     sal_Bool bFolder );

    public:
        BaseContent(
            shell* pMyShell,
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& xContentIdentifier,
            const rtl::OUString& aUnqPath );

        virtual ~BaseContent();

        // XInterface
        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface(
            const com::sun::star::uno::Type& aType )
            throw( com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        acquire(
            void )
            throw();

        virtual void SAL_CALL
        release(
            void )
            throw();


        // XComponent
        virtual void SAL_CALL
        dispose(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        addEventListener(
            const com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >& xListener )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        removeEventListener( const com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >& aListener )
            throw( com::sun::star::uno::RuntimeException );


        // XTypeProvider

        XTYPEPROVIDER_DECL()


        // XServiceInfo
        virtual rtl::OUString SAL_CALL
        getImplementationName()
            throw( com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL
        supportsService( const rtl::OUString& ServiceName )
            throw( com::sun::star::uno::RuntimeException);

        virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
        getSupportedServiceNames()
            throw( com::sun::star::uno::RuntimeException );


        // XCommandProcessor
        virtual sal_Int32 SAL_CALL
        createCommandIdentifier(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual com::sun::star::uno::Any SAL_CALL
        execute(
            const com::sun::star::ucb::Command& aCommand,
            sal_Int32 CommandId,
            const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >& Environment )
            throw( com::sun::star::uno::Exception,
                   com::sun::star::ucb::CommandAbortedException,
                   com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        abort(
            sal_Int32 CommandId )
            throw( com::sun::star::uno::RuntimeException );


        // XContent
        virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier > SAL_CALL
        getIdentifier(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual rtl::OUString SAL_CALL
        getContentType(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        addContentEventListener(
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContentEventListener >& Listener )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        removeContentEventListener(
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContentEventListener >& Listener )
            throw( com::sun::star::uno::RuntimeException );

        // XPropertiesChangeNotifier

        virtual void SAL_CALL
        addPropertiesChangeListener(
            const com::sun::star::uno::Sequence< rtl::OUString >& PropertyNames,
            const com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertiesChangeListener >& Listener )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        removePropertiesChangeListener( const com::sun::star::uno::Sequence< rtl::OUString >& PropertyNames,
                                        const com::sun::star::uno::Reference<
                                        com::sun::star::beans::XPropertiesChangeListener >& Listener )
            throw( com::sun::star::uno::RuntimeException );

        // XPropertyContainer

        virtual void SAL_CALL
        addProperty(
            const rtl::OUString& Name,
            sal_Int16 Attributes,
            const com::sun::star::uno::Any& DefaultValue )
            throw( com::sun::star::beans::PropertyExistException,
                   com::sun::star::beans::IllegalTypeException,
                   com::sun::star::lang::IllegalArgumentException,
                   com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        removeProperty(
            const rtl::OUString& Name )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::beans::NotRemoveableException,
                   com::sun::star::uno::RuntimeException );

        // XPropertySetInfoChangeNotifier

        virtual void SAL_CALL
        addPropertySetInfoChangeListener(
            const com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertySetInfoChangeListener >& Listener )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        removePropertySetInfoChangeListener(
            const com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertySetInfoChangeListener >& Listener )
            throw( com::sun::star::uno::RuntimeException );


        // XContentCreator

        virtual com::sun::star::uno::Sequence< com::sun::star::ucb::ContentInfo > SAL_CALL
        queryCreatableContentsInfo(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContent > SAL_CALL
        createNewContent(
            const com::sun::star::ucb::ContentInfo& Info )
            throw( com::sun::star::uno::RuntimeException );


        // XChild
        virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL
        getParent(
            void ) throw( com::sun::star::uno::RuntimeException );

        // Not supported
        virtual void SAL_CALL
        setParent( const com::sun::star::uno::Reference< com::sun::star::uno::XInterface >& Parent )
            throw( com::sun::star::lang::NoSupportException,
                   com::sun::star::uno::RuntimeException);


        // Notifier

        ContentEventNotifier*          cDEL( void );
        ContentEventNotifier*          cEXC( const rtl::OUString aNewName );
        ContentEventNotifier*          cCEL( void );
        PropertySetInfoChangeNotifier* cPSL( void );
        PropertyChangeNotifier*        cPCL( void );
        rtl::OUString                  getKey( void );

    private:
        // Data members
        shell*                                                                      m_pMyShell;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >   m_xContentIdentifier;
        rtl::OUString                                                               m_aUncPath;

        enum state { NameForInsertionSet = 1,
                     JustInserted = 2,
                     Deleted = 4,
                     FullFeatured = 8,
                     Connected = 16 };
        sal_Bool                                                                    m_bFolder;
        sal_uInt16                                                                  m_nState;

        osl::Mutex                         m_aMutex;

        osl::Mutex                          m_aEventListenerMutex;
        cppu::OInterfaceContainerHelper*    m_pDisposeEventListeners;
        cppu::OInterfaceContainerHelper*    m_pContentEventListeners;
        cppu::OInterfaceContainerHelper*    m_pPropertySetInfoChangeListeners;
        PropertyListeners*                  m_pPropertyListener;


        // Private Methods
        com::sun::star::uno::Reference< com::sun::star::ucb::XCommandInfo > SAL_CALL
        getCommandInfo()
            throw( com::sun::star::uno::RuntimeException );

        virtual com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo(
            sal_Int32 nMyCommandIdentifier )
            throw( com::sun::star::uno::RuntimeException );

        virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XRow > SAL_CALL
        getPropertyValues(
            sal_Int32 nMyCommandIdentifier,
            const com::sun::star::uno::Sequence< com::sun::star::beans::Property >& PropertySet )
            throw( com::sun::star::uno::RuntimeException );

        com::sun::star::uno::Sequence< com::sun::star::uno::Any > SAL_CALL
        setPropertyValues(
            sal_Int32 nMyCommandIdentifier,
            const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& Values )
            throw( );

        com::sun::star::uno::Reference< com::sun::star::ucb::XDynamicResultSet > SAL_CALL
        open(
            sal_Int32 nMyCommandIdentifier,
            const com::sun::star::ucb::OpenCommandArgument2& aCommandArgument )
            throw();

        void SAL_CALL
        deleteContent( sal_Int32 nMyCommandIdentifier )
            throw();


        void SAL_CALL
        transfer( sal_Int32 nMyCommandIdentifier,
                  const com::sun::star::ucb::TransferInfo& aTransferInfo )
            throw();

        void SAL_CALL
        insert( sal_Int32 nMyCommandIdentifier,
                const com::sun::star::ucb::InsertCommandArgument& aInsertArgument )
            throw();

        void SAL_CALL endTask( sal_Int32 CommandId );

        friend class ContentEventNotifier;
    };

}             // end namespace fileaccess

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

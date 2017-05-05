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

#ifndef INCLUDED_UCB_SOURCE_UCP_FILE_BC_HXX
#define INCLUDED_UCB_SOURCE_UCP_FILE_BC_HXX

#include "osl/mutex.hxx"
#include "rtl/ustring.hxx"
#include <cppuhelper/weak.hxx>
#include <ucbhelper/macros.hxx>
#include <comphelper/interfacecontainer2.hxx>
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
#include "filtask.hxx"


namespace fileaccess {

    class PropertyListeners;
    class TaskManager;

    class BaseContent:
        public cppu::OWeakObject,
        public css::lang::XComponent,
        public css::lang::XServiceInfo,
        public css::lang::XTypeProvider,
        public css::ucb::XCommandProcessor,
        public css::beans::XPropertiesChangeNotifier,
        public css::beans::XPropertyContainer,
        public css::beans::XPropertySetInfoChangeNotifier,
        public css::ucb::XContentCreator,
        public css::container::XChild,
        public css::ucb::XContent,
        public fileaccess::Notifier    // implementation class
    {
    private:

        // A special creator for inserted contents; Creates an ugly object
        BaseContent( TaskManager* pMyShell,
                     const OUString& parentName,
                     bool bFolder );

    public:
        BaseContent(
            TaskManager* pMyShell,
            const css::uno::Reference< css::ucb::XContentIdentifier >& xContentIdentifier,
            const OUString& aUnqPath );

        virtual ~BaseContent() override;

        // XInterface
        virtual css::uno::Any SAL_CALL
        queryInterface( const css::uno::Type& aType ) override;

        virtual void SAL_CALL
        acquire()
            throw() override;

        virtual void SAL_CALL
        release()
            throw() override;


        // XComponent
        virtual void SAL_CALL
        dispose() override;

        virtual void SAL_CALL
        addEventListener(
            const css::uno::Reference< css::lang::XEventListener >& xListener ) override;

        virtual void SAL_CALL
        removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;


        // XTypeProvider

        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;


        // XServiceInfo
        virtual OUString SAL_CALL
        getImplementationName() override;

        virtual sal_Bool SAL_CALL
        supportsService( const OUString& ServiceName ) override;

        virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames() override;


        // XCommandProcessor
        virtual sal_Int32 SAL_CALL
        createCommandIdentifier() override;

        virtual css::uno::Any SAL_CALL
        execute(
            const css::ucb::Command& aCommand,
            sal_Int32 CommandId,
            const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment ) override;

        virtual void SAL_CALL
        abort( sal_Int32 CommandId ) override;


        // XContent
        virtual css::uno::Reference< css::ucb::XContentIdentifier > SAL_CALL
        getIdentifier() override;

        virtual OUString SAL_CALL
        getContentType() override;

        virtual void SAL_CALL
        addContentEventListener(
            const css::uno::Reference< css::ucb::XContentEventListener >& Listener ) override;

        virtual void SAL_CALL
        removeContentEventListener(
            const css::uno::Reference< css::ucb::XContentEventListener >& Listener ) override;

        // XPropertiesChangeNotifier

        virtual void SAL_CALL
        addPropertiesChangeListener(
            const css::uno::Sequence< OUString >& PropertyNames,
            const css::uno::Reference< css::beans::XPropertiesChangeListener >& Listener ) override;

        virtual void SAL_CALL
        removePropertiesChangeListener( const css::uno::Sequence< OUString >& PropertyNames,
                                        const css::uno::Reference< css::beans::XPropertiesChangeListener >& Listener ) override;

        // XPropertyContainer

        virtual void SAL_CALL
        addProperty(
            const OUString& Name,
            sal_Int16 Attributes,
            const css::uno::Any& DefaultValue ) override;

        virtual void SAL_CALL
        removeProperty( const OUString& Name ) override;

        // XPropertySetInfoChangeNotifier

        virtual void SAL_CALL
        addPropertySetInfoChangeListener(
            const css::uno::Reference< css::beans::XPropertySetInfoChangeListener >& Listener ) override;

        virtual void SAL_CALL
        removePropertySetInfoChangeListener(
            const css::uno::Reference< css::beans::XPropertySetInfoChangeListener >& Listener ) override;


        // XContentCreator

        virtual css::uno::Sequence< css::ucb::ContentInfo > SAL_CALL
        queryCreatableContentsInfo() override;

        virtual css::uno::Reference< css::ucb::XContent > SAL_CALL
        createNewContent( const css::ucb::ContentInfo& Info ) override;


        // XChild
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
        getParent() override;

        // Not supported
        virtual void SAL_CALL
        setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) override;


        // Notifier

        ContentEventNotifier*          cDEL() override;
        ContentEventNotifier*          cEXC( const OUString& aNewName ) override;
        ContentEventNotifier*          cCEL() override;
        PropertySetInfoChangeNotifier* cPSL() override;
        PropertyChangeNotifier*        cPCL() override;

    private:
        // Data members
        TaskManager*                                                m_pMyShell;
        css::uno::Reference< css::ucb::XContentIdentifier >   m_xContentIdentifier;
        OUString                                              m_aUncPath;

        enum state { NameForInsertionSet = 1,
                     JustInserted = 2,
                     Deleted = 4,
                     FullFeatured = 8 };
        bool                                                                    m_bFolder;
        sal_uInt16                                                                  m_nState;

        osl::Mutex                         m_aMutex;

        osl::Mutex                          m_aEventListenerMutex;
        comphelper::OInterfaceContainerHelper2*   m_pDisposeEventListeners;
        comphelper::OInterfaceContainerHelper2*   m_pContentEventListeners;
        comphelper::OInterfaceContainerHelper2*   m_pPropertySetInfoChangeListeners;
        PropertyListeners*                  m_pPropertyListener;


        // Private Methods
        /// @throws css::uno::RuntimeException
        css::uno::Reference< css::ucb::XCommandInfo > SAL_CALL
        getCommandInfo();

        /// @throws css::uno::RuntimeException
        css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo();

        /// @throws css::uno::RuntimeException
        css::uno::Reference< css::sdbc::XRow > SAL_CALL
        getPropertyValues(
            sal_Int32 nMyCommandIdentifier,
            const css::uno::Sequence< css::beans::Property >& PropertySet );

        css::uno::Sequence< css::uno::Any > SAL_CALL
        setPropertyValues(
            sal_Int32 nMyCommandIdentifier,
            const css::uno::Sequence< css::beans::PropertyValue >& Values );

        css::uno::Reference< css::ucb::XDynamicResultSet > SAL_CALL
        open(
            sal_Int32 nMyCommandIdentifier,
            const css::ucb::OpenCommandArgument2& aCommandArgument );

        void SAL_CALL
        deleteContent( sal_Int32 nMyCommandIdentifier );


        void SAL_CALL
        transfer( sal_Int32 nMyCommandIdentifier,
                  const css::ucb::TransferInfo& aTransferInfo );

        void SAL_CALL
        insert( sal_Int32 nMyCommandIdentifier,
                const css::ucb::InsertCommandArgument& aInsertArgument );

        void SAL_CALL endTask( sal_Int32 CommandId );

        friend class ContentEventNotifier;
    };

}             // end namespace fileaccess

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

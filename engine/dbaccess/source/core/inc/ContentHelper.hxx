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
#pragma once

#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/beans/XPropertiesChangeNotifier.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <comphelper/interfacecontainer3.hxx>
#include <comphelper/multiinterfacecontainer3.hxx>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/sdbcx/XRename.hpp>
#include <connectivity/sqlerror.hxx>
#include <memory>

namespace com::sun::star::beans { struct PropertyValue; }

namespace dbaccess
{
    class ODatabaseModelImpl;
    struct ContentProperties
    {
        OUString aTitle;         // Title
        ::std::optional< OUString >
                        aContentType;   // ContentType (aka MediaType aka MimeType)
        bool        bIsDocument;    // IsDocument
        bool        bIsFolder;      // IsFolder
        bool        bAsTemplate;    // AsTemplate
        OUString sPersistentName;// persistent name of the document

        ContentProperties()
            :bIsDocument( true )
            ,bIsFolder( false )
            ,bAsTemplate( false )
        {
        }
    };

    class OContentHelper_Impl
    {
    public:
        OContentHelper_Impl();
        virtual ~OContentHelper_Impl();

        ContentProperties   m_aProps;
        ODatabaseModelImpl* m_pDataSource; // this will stay alive as long as the content exists
    };

    typedef std::shared_ptr<OContentHelper_Impl> TContentPtr;

    typedef comphelper::OMultiTypeInterfaceContainerHelperVar3<css::beans::XPropertiesChangeListener, OUString>
        PropertyChangeListenerContainer;
    typedef ::cppu::WeakComponentImplHelper<   css::ucb::XContent
                                           ,   css::ucb::XCommandProcessor
                                           ,   css::lang::XServiceInfo
                                           ,   css::beans::XPropertiesChangeNotifier
                                           ,   css::beans::XPropertyContainer
                                           ,   css::lang::XInitialization
                                           ,   css::container::XChild
                                           ,   css::sdbcx::XRename
                                           >   OContentHelper_COMPBASE;

    class OContentHelper :   public ::cppu::BaseMutex
                            ,public OContentHelper_COMPBASE
    {
        cpo::uno::Sequence< cpo::uno::Any >
            setPropertyValues( const cpo::uno::Sequence< css::beans::PropertyValue >& rValues );

        void impl_rename_throw(const OUString& _sNewName,bool _bNotify = true);

    protected:
        ::comphelper::OInterfaceContainerHelper3<css::ucb::XContentEventListener> m_aContentListeners;
        PropertyChangeListenerContainer         m_aPropertyChangeListeners;
        css::uno::Reference< css::uno::XInterface >
                                                m_xParentContainer;
        const css::uno::Reference< css::uno::XComponentContext >
                                                m_aContext;
        const ::connectivity::SQLError          m_aErrorHelper;
        TContentPtr                             m_pImpl;
        sal_uInt32                              m_nCommandId;

        // helper
        virtual void disposing() override;

        void notifyDataSourceModified();

        /**
        * This method can be used to propagate changes of property values.
        *
        * @param evt is a sequence of property change events.
        */
        void notifyPropertiesChange( const cpo::uno::Sequence< css::beans::PropertyChangeEvent >& evt ) const;

        OUString impl_getHierarchicalName( bool _includingRootContainer ) const;

    public:

        OContentHelper( const css::uno::Reference< css::uno::XComponentContext >& _xORB
                        ,const css::uno::Reference< css::uno::XInterface >&   _xParentContainer
                        ,TContentPtr _pImpl
                    );

        // css::lang::XTypeProvider
        virtual cpo::uno::Sequence<sal_Int8> getImplementationId(  ) override;

        // css::lang::XServiceInfo
        virtual bool supportsService( const OUString& ServiceName ) override;
        virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;
        virtual OUString getImplementationName(  ) override;

        // XContent
        virtual css::uno::Reference< css::ucb::XContentIdentifier > getIdentifier(  ) override ;
        virtual OUString getContentType(  ) override ;
        virtual void addContentEventListener( const css::uno::Reference< css::ucb::XContentEventListener >& Listener ) override ;
        virtual void removeContentEventListener( const css::uno::Reference< css::ucb::XContentEventListener >& Listener ) override ;

        // XCommandProcessor
        virtual sal_Int32 createCommandIdentifier(  ) override ;
        virtual cpo::uno::Any execute( const css::ucb::Command& aCommand, sal_Int32 CommandId, const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment ) override ;
        virtual void abort( sal_Int32 CommandId ) override ;

        // XPropertiesChangeNotifier
        virtual void addPropertiesChangeListener( const cpo::uno::Sequence< OUString >& PropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& Listener ) override ;
        virtual void removePropertiesChangeListener( const cpo::uno::Sequence< OUString >& PropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& Listener ) override ;

        // XPropertyContainer
        virtual void addProperty( const OUString& Name, sal_Int16 Attributes, const cpo::uno::Any& DefaultValue ) override ;
        virtual void removeProperty( const OUString& Name ) override ;

        // XInitialization
        virtual void initialize( const cpo::uno::Sequence< cpo::uno::Any >& aArguments ) override;

        // css::container::XChild
        virtual css::uno::Reference< css::uno::XInterface > getParent(  ) override;
        virtual void setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) override;

        // XRename
        virtual void rename( const OUString& newName ) override;

        const ContentProperties& getContentProperties() const { return m_pImpl->m_aProps; }
        css::uno::Reference< css::sdbc::XRow >
            getPropertyValues( const cpo::uno::Sequence< css::beans::Property >& rProperties );

        const css::uno::Reference< css::uno::XComponentContext >& getContext() const { return m_aContext; }

        const TContentPtr& getImpl() const { return m_pImpl; }

    protected:
        virtual OUString determineContentType() const = 0;
    };

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

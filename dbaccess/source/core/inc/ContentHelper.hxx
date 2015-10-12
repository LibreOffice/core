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
#ifndef INCLUDED_DBACCESS_SOURCE_CORE_INC_CONTENTHELPER_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_INC_CONTENTHELPER_HXX

#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/beans/XPropertiesChangeNotifier.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/compbase.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/sdbcx/XRename.hpp>
#include <connectivity/sqlerror.hxx>
#include <memory>

namespace com { namespace sun { namespace star { namespace beans {
    struct PropertyValue;
} } } }

namespace dbaccess
{
    class ODatabaseModelImpl;
    struct ContentProperties
    {
        OUString aTitle;         // Title
        ::boost::optional< OUString >
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

    typedef cppu::OMultiTypeInterfaceContainerHelperVar<OUString>
        PropertyChangeListenerContainer;
    typedef ::comphelper::OBaseMutex    OContentHelper_MBASE;
    typedef ::cppu::WeakComponentImplHelper<   css::ucb::XContent
                                           ,   css::ucb::XCommandProcessor
                                           ,   css::lang::XServiceInfo
                                           ,   css::beans::XPropertiesChangeNotifier
                                           ,   css::beans::XPropertyContainer
                                           ,   css::lang::XInitialization
                                           ,   css::lang::XUnoTunnel
                                           ,   css::container::XChild
                                           ,   css::sdbcx::XRename
                                           >   OContentHelper_COMPBASE;

    class OContentHelper :   public OContentHelper_MBASE
                            ,public OContentHelper_COMPBASE
    {
        css::uno::Sequence< css::uno::Any >
            setPropertyValues( const css::uno::Sequence< css::beans::PropertyValue >& rValues,
                               const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

        void impl_rename_throw(const OUString& _sNewName,bool _bNotify = true);

    protected:
        ::cppu::OInterfaceContainerHelper       m_aContentListeners;
        PropertyChangeListenerContainer         m_aPropertyChangeListeners;
        css::uno::Reference< css::uno::XInterface >
                                                m_xParentContainer;
        const css::uno::Reference< css::uno::XComponentContext >
                                                m_aContext;
        const ::connectivity::SQLError          m_aErrorHelper;
        TContentPtr                             m_pImpl;
        sal_uInt32                              m_nCommandId;

        // helper
        virtual void SAL_CALL disposing() override;

        virtual void notifyDataSourceModified();

        /**
        * This method can be used to propagate changes of property values.
        *
        * @param evt is a sequence of property change events.
        */
        void notifyPropertiesChange( const css::uno::Sequence< css::beans::PropertyChangeEvent >& evt ) const;

        OUString impl_getHierarchicalName( bool _includingRootContainer ) const;

    public:

        OContentHelper( const css::uno::Reference< css::uno::XComponentContext >& _xORB
                        ,const css::uno::Reference< css::uno::XInterface >&   _xParentContainer
                        ,const TContentPtr& _pImpl
                    );

        // css::lang::XTypeProvider
        virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) override;
        static css::uno::Sequence< sal_Int8 >  getUnoTunnelImplementationId();
        // css::lang::XServiceInfo
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XContent
        virtual css::uno::Reference< css::ucb::XContentIdentifier > SAL_CALL getIdentifier(  ) throw (css::uno::RuntimeException, std::exception) override ;
        virtual OUString SAL_CALL getContentType(  ) throw (css::uno::RuntimeException, std::exception) override ;
        virtual void SAL_CALL addContentEventListener( const css::uno::Reference< css::ucb::XContentEventListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override ;
        virtual void SAL_CALL removeContentEventListener( const css::uno::Reference< css::ucb::XContentEventListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override ;

        // XCommandProcessor
        virtual sal_Int32 SAL_CALL createCommandIdentifier(  ) throw (css::uno::RuntimeException, std::exception) override ;
        virtual css::uno::Any SAL_CALL execute( const css::ucb::Command& aCommand, sal_Int32 CommandId, const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment ) throw (css::uno::Exception, css::ucb::CommandAbortedException, css::uno::RuntimeException, std::exception) override ;
        virtual void SAL_CALL abort( sal_Int32 CommandId ) throw (css::uno::RuntimeException, std::exception) override ;

        // XPropertiesChangeNotifier
        virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >& PropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override ;
        virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Sequence< OUString >& PropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override ;

        // XPropertyContainer
        virtual void SAL_CALL addProperty( const OUString& Name, sal_Int16 Attributes, const css::uno::Any& DefaultValue ) throw (css::beans::PropertyExistException, css::beans::IllegalTypeException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override ;
        virtual void SAL_CALL removeProperty( const OUString& Name ) throw (css::beans::UnknownPropertyException, css::beans::NotRemoveableException, css::uno::RuntimeException, std::exception) override ;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw(css::uno::Exception, css::uno::RuntimeException, std::exception) override;

        // css::lang::XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw(css::uno::RuntimeException, std::exception) override;
        static OContentHelper* getImplementation( const css::uno::Reference< css::uno::XInterface >& _rxComponent );

        // css::container::XChild
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;

        // XRename
        virtual void SAL_CALL rename( const OUString& newName ) throw (css::sdbc::SQLException, css::container::ElementExistException, css::uno::RuntimeException, std::exception) override;

        inline const ContentProperties& getContentProperties() const { return m_pImpl->m_aProps; }
        css::uno::Reference< css::sdbc::XRow >
            getPropertyValues( const css::uno::Sequence< css::beans::Property >& rProperties );

        const css::uno::Reference< css::uno::XComponentContext >& getContext() const { return m_aContext; }

        inline TContentPtr getImpl() const { return m_pImpl; }

    protected:
        virtual OUString determineContentType() const = 0;
    };

}   // namespace dbaccess

#endif // INCLUDED_DBACCESS_SOURCE_CORE_INC_CONTENTHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

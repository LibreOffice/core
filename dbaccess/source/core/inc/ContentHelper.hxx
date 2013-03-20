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
#pragma once
#if 1

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
#include <cppuhelper/compbase9.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/componentcontext.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/stl_types.hxx>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/sdbcx/XRename.hpp>
#include <connectivity/sqlerror.hxx>
#include <boost/shared_ptr.hpp>

namespace dbaccess
{
    class ODatabaseModelImpl;
    struct ContentProperties
    {
        ::rtl::OUString aTitle;         // Title
        ::boost::optional< ::rtl::OUString >
                        aContentType;   // ContentType (aka MediaType aka MimeType)
        sal_Bool        bIsDocument;    // IsDocument
        sal_Bool        bIsFolder;      // IsFolder
        sal_Bool        bAsTemplate;    // AsTemplate
        ::rtl::OUString sPersistentName;// persistent name of the document

        ContentProperties()
            :bIsDocument( sal_True )
            ,bIsFolder( sal_False )
            ,bAsTemplate( sal_False )
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

    typedef ::boost::shared_ptr<OContentHelper_Impl> TContentPtr;


    typedef ::cppu::OMultiTypeInterfaceContainerHelperVar<  ::rtl::OUString,
                                                            ::rtl::OUStringHash,
                                                            ::comphelper::UStringEqual
                                                        > PropertyChangeListenerContainer;
    typedef ::comphelper::OBaseMutex    OContentHelper_MBASE;
    typedef ::cppu::WeakComponentImplHelper9    <   ::com::sun::star::ucb::XContent
                                                ,   ::com::sun::star::ucb::XCommandProcessor
                                                ,   ::com::sun::star::lang::XServiceInfo
                                                ,   ::com::sun::star::beans::XPropertiesChangeNotifier
                                                ,   ::com::sun::star::beans::XPropertyContainer
                                                ,   ::com::sun::star::lang::XInitialization
                                                ,   ::com::sun::star::lang::XUnoTunnel
                                                ,   ::com::sun::star::container::XChild
                                                ,   ::com::sun::star::sdbcx::XRename
                                                >   OContentHelper_COMPBASE;

    class OContentHelper :   public OContentHelper_MBASE
                            ,public OContentHelper_COMPBASE
    {
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
            setPropertyValues( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::beans::PropertyValue >& rValues,
                            const ::com::sun::star::uno::Reference<
                                ::com::sun::star::ucb::XCommandEnvironment >& xEnv );
        com::sun::star::uno::Sequence< com::sun::star::beans::Property >
            getProperties( const com::sun::star::uno::Reference<
                            com::sun::star::ucb::XCommandEnvironment > & xEnv );

        void impl_rename_throw(const ::rtl::OUString& _sNewName,bool _bNotify = true);

    protected:
        ::cppu::OInterfaceContainerHelper       m_aContentListeners;
        PropertyChangeListenerContainer         m_aPropertyChangeListeners;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                                                m_xParentContainer;
        const ::comphelper::ComponentContext    m_aContext;
        const ::connectivity::SQLError          m_aErrorHelper;
        TContentPtr                             m_pImpl;
        sal_uInt32                              m_nCommandId;

        // helper
        virtual void SAL_CALL disposing();

        virtual void notifyDataSourceModified();

        /**
        * This method can be used to propagate changes of property values.
        *
        * @param evt is a sequence of property change events.
        */
        void notifyPropertiesChange( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyChangeEvent >& evt ) const;

        ::rtl::OUString impl_getHierarchicalName( bool _includingRootContainer ) const;

    public:

        OContentHelper( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xORB
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >&   _xParentContainer
                        ,const TContentPtr& _pImpl
                    );

        // com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< sal_Int8 >  getUnoTunnelImplementationId();
        // ::com::sun::star::lang::XServiceInfo
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);

        // XContent
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContentIdentifier > SAL_CALL getIdentifier(  ) throw (::com::sun::star::uno::RuntimeException) ;
        virtual ::rtl::OUString SAL_CALL getContentType(  ) throw (::com::sun::star::uno::RuntimeException) ;
        virtual void SAL_CALL addContentEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContentEventListener >& Listener ) throw (::com::sun::star::uno::RuntimeException) ;
        virtual void SAL_CALL removeContentEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContentEventListener >& Listener ) throw (::com::sun::star::uno::RuntimeException) ;

        // XCommandProcessor
        virtual sal_Int32 SAL_CALL createCommandIdentifier(  ) throw (::com::sun::star::uno::RuntimeException) ;
        virtual ::com::sun::star::uno::Any SAL_CALL execute( const ::com::sun::star::ucb::Command& aCommand, sal_Int32 CommandId, const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >& Environment ) throw (::com::sun::star::uno::Exception, ::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::RuntimeException) ;
        virtual void SAL_CALL abort( sal_Int32 CommandId ) throw (::com::sun::star::uno::RuntimeException) ;

        // XPropertiesChangeNotifier
        virtual void SAL_CALL addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& PropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& Listener ) throw (::com::sun::star::uno::RuntimeException) ;
        virtual void SAL_CALL removePropertiesChangeListener( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& PropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& Listener ) throw (::com::sun::star::uno::RuntimeException) ;

        // XPropertyContainer
        virtual void SAL_CALL addProperty( const ::rtl::OUString& Name, sal_Int16 Attributes, const ::com::sun::star::uno::Any& DefaultValue ) throw (::com::sun::star::beans::PropertyExistException, ::com::sun::star::beans::IllegalTypeException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException) ;
        virtual void SAL_CALL removeProperty( const ::rtl::OUString& Name ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::NotRemoveableException, ::com::sun::star::uno::RuntimeException) ;

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // com::sun::star::lang::XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
        static OContentHelper* getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxComponent );

        // ::com::sun::star::container::XChild
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

        // XRename
        virtual void SAL_CALL rename( const ::rtl::OUString& newName ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);

        inline const ContentProperties& getContentProperties() const { return m_pImpl->m_aProps; }
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
            getPropertyValues( const ::com::sun::star::uno::Sequence<
                            ::com::sun::star::beans::Property >& rProperties );

        const ::comphelper::ComponentContext& getContext() const { return m_aContext; }

        inline TContentPtr getImpl() const { return m_pImpl; }

    protected:
        virtual ::rtl::OUString determineContentType() const = 0;
    };

}   // namespace dbaccess

#endif // DBA_CONTENTHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

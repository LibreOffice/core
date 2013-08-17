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

#ifndef _DBA_COREDATAACCESS_DOCUMENTDEFINITION_HXX_
#define _DBA_COREDATAACCESS_DOCUMENTDEFINITION_HXX_

#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/implbase4.hxx>
#include "ContentHelper.hxx"
#include <comphelper/propertystatecontainer.hxx>
#include <comphelper/proparrhlp.hxx>
#include "apitools.hxx"
#include <comphelper/uno3.hxx>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/embed/XStateChangeListener.hpp>
#include <com/sun/star/sdb/XSubDocument.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include <com/sun/star/container/XHierarchicalName.hpp>

namespace comphelper
{
    class NamedValueCollection;
}

namespace dbaccess
{

    class OInterceptor;
    class OEmbeddedClientHelper;
// ODocumentDefinition - a database "document" which is simply a link to a real
//                   document

typedef ::cppu::ImplHelper4 <   ::com::sun::star::embed::XComponentSupplier
                            ,   ::com::sun::star::sdb::XSubDocument
                            ,   ::com::sun::star::util::XCloseListener
                            ,   ::com::sun::star::container::XHierarchicalName
                            >   ODocumentDefinition_Base;

class ODocumentDefinition
        :public OContentHelper
        ,public ::comphelper::OPropertyStateContainer
        ,public ::comphelper::OPropertyArrayUsageHelper< ODocumentDefinition >
        ,public ODocumentDefinition_Base
{
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedObject>         m_xEmbeddedObject;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStateChangeListener >   m_xListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >             m_xLastKnownConnection;

    OInterceptor*                                                                       m_pInterceptor;
    sal_Bool                                                                            m_bForm; // <TRUE/> if it is a form
    sal_Bool                                                                            m_bOpenInDesign;
    sal_Bool                                                                            m_bInExecute;
    sal_Bool                                                                            m_bRemoveListener;
    OEmbeddedClientHelper*                                                              m_pClientHelper;

protected:
    virtual ~ODocumentDefinition();

public:

    ODocumentDefinition(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContainer,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&,
            const TContentPtr& _pImpl,
            sal_Bool _bForm
        );

    void    initialLoad(
                const ::com::sun::star::uno::Sequence< sal_Int8 >& i_rClassID,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& i_rCreationArgs,
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& i_rConnection
            );

// com::sun::star::lang::XTypeProvider
    DECLARE_TYPEPROVIDER( );

// ::com::sun::star::uno::XInterface
    DECLARE_XINTERFACE( )

// ::com::sun::star::lang::XServiceInfo
    DECLARE_SERVICE_INFO_STATIC();

// ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // OPropertySetHelper
    virtual void SAL_CALL getFastPropertyValue(
                                ::com::sun::star::uno::Any& o_rValue,
                                sal_Int32 i_nHandle
                            ) const;

    // XComponentSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable > SAL_CALL getComponent(  ) throw (::com::sun::star::uno::RuntimeException);

    // XSubDocument
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > SAL_CALL open(  ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > SAL_CALL openDesign(  ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL store(  ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL close(  ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XHierarchicalName
    virtual OUString SAL_CALL getHierarchicalName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL composeHierarchicalName( const OUString& aRelativeName ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

// OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    // XCommandProcessor
    virtual ::com::sun::star::uno::Any SAL_CALL execute( const ::com::sun::star::ucb::Command& aCommand, sal_Int32 CommandId, const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >& Environment ) throw (::com::sun::star::uno::Exception, ::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::RuntimeException) ;

    // XRename
    virtual void SAL_CALL rename( const OUString& newName ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);

    // XCloseListener
    virtual void SAL_CALL queryClosing( const ::com::sun::star::lang::EventObject& Source, ::sal_Bool GetsOwnership ) throw (::com::sun::star::util::CloseVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL notifyClosing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    /** returns the forms/reports container storage, depending on m_bForm. Our own storage
        inside this container storage is the one with the name as indicated by m_pImpl->m_aProps.sPersistentName.
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
        getContainerStorage() const;

    sal_Bool save(sal_Bool _bApprove);
    sal_Bool saveAs();
    void closeObject();
    sal_Bool isModified();
    inline sal_Bool isNewReport() const { return !m_bForm && !m_pImpl->m_aProps.bAsTemplate; }

    static void fillReportData(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & _rxContext,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable >& _rxComponent,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxActiveConnection
                );

    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >&
        getConnection() const { return m_xLastKnownConnection; }

    /** prepares closing the document component

        The method suspends the controller associated with the document, and saves the document
        if necessary.

        @return
            <TRUE/> if and only if the document component can be closed
    */
    bool prepareClose();

    static ::com::sun::star::uno::Sequence< sal_Int8 > getDefaultDocumentTypeClassId();

    static OUString GetDocumentServiceFromMediaType(
        const OUString& _rMediaType,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & _rxContext,
        ::com::sun::star::uno::Sequence< sal_Int8 >& _rClassId
    );
    static OUString GetDocumentServiceFromMediaType(
        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxContainerStorage,
        const OUString& _rEntityName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & _rxContext,
        ::com::sun::star::uno::Sequence< sal_Int8 >& _rClassId
    );

    struct NotifierAccess { friend class NameChangeNotifier; private: NotifierAccess() { } };
    const OUString& getCurrentName() const { return m_pImpl->m_aProps.aTitle; }
    void firePropertyChange(
                  sal_Int32 i_nHandle,
            const ::com::sun::star::uno::Any& i_rNewValue,
            const ::com::sun::star::uno::Any& i_rOldValue,
                  sal_Bool i_bVetoable,
            const NotifierAccess
        );

private:
    /** does necessary initializations after our embedded object has been switched to ACTIVE
    */
    void    impl_onActivateEmbeddedObject_nothrow( const bool i_bReactivated );

    /** initializes a newly created view/controller of a form which is displaying our embedded object

        Has only to be called if the respective embedded object has been loaded for design (and
        not for data entry)

        @param  _rxController
            the controller which belongs to the XModel of our (active) embedded object
    */
    static void impl_initFormEditView( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& _rxController );

    /** removes the given frame from the desktop's frame collection
        @raises ::com::sun::star::uno::RuntimeException
    */
    static void impl_removeFrameFromDesktop_throw(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rContxt,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxFrame
                );

    /** opens the UI for this sub document
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
            impl_openUI_nolck_throw( bool _bForEditing );

    /** stores our document, if it's already loaded
    */
    void    impl_store_throw();

    /** closes our document, if it's open
    */
    bool    impl_close_throw();

    /** returns our component, creates it if necessary
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable >
            impl_getComponent_throw( const bool i_ForceCreate = true );

    /** shows or hides our component

        The embedded object must exist, and be in state LOADED, at least.
    */
    void    impl_showOrHideComponent_throw( const bool i_bShow );

    // OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    virtual void getPropertyDefaultByHandle( sal_Int32 _nHandle, ::com::sun::star::uno::Any& _rDefault ) const;

    // helper
    virtual void SAL_CALL disposing();

    // OContentHelper overridables
    virtual OUString determineContentType() const;

    /** fills the load arguments
    */
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
        fillLoadArgs(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
            const bool _bSuppressMacros,
            const bool _bReadOnly,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& i_rOpenCommandArguments,
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _out_rEmbeddedObjectDescriptor
        );

    /** splits the given arguments to an "open*" command into arguments for loading the document, and arguments to be
        put into the EmbeddedObjectDescriptor

        Any values already present in <code>o_rDocumentLoadArgs</code> and <code>o_rEmbeddedObjectDescriptor</code>
        will be overwritten by values from <code>i_rOpenCommandArguments</code>, if applicable, otherwise they will
        be preserved.

        @param i_rOpenCommandArguments
            the arguments passed to the "open*" command at the content
        @param o_rDocumentLoadArgs
            the arguments to be passed when actually loading the embedded document.
        @param o_rEmbeddedObjectDescriptor
            the EmbeddedObjectDescriptor to be passed when initializing the embedded object
    */
    void separateOpenCommandArguments(
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >&    i_rOpenCommandArguments,
            ::comphelper::NamedValueCollection&                                                 o_rDocumentLoadArgs,
            ::comphelper::NamedValueCollection&                                                 o_rEmbeddedObjectDescriptor
        );

    /** loads the EmbeddedObject if not already loaded
        @param  _aClassID
            If set, it will be used to create the embedded object.
    */
    void loadEmbeddedObject(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
                const ::com::sun::star::uno::Sequence< sal_Int8 >& _aClassID,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rAdditionalArgs,
                const bool _bSuppressMacros,
                const bool _bReadOnly
            );

    /** loads the embedded object, if not already loaded. No new object can be created with this method.
    */
    void    loadEmbeddedObject( bool _bSuppressMacros = false )
    {
        loadEmbeddedObject(
            NULL,
            ::com::sun::star::uno::Sequence< sal_Int8 >(),
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >(),
            _bSuppressMacros,
            false
        );
    }

    /** loads the embedded object for preview. Macros will be suppressed, and the document will
        be read-only.
    */
    void    loadEmbeddedObjectForPreview()
    {
        loadEmbeddedObject(
            NULL,
            ::com::sun::star::uno::Sequence< sal_Int8 >(),
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >(),
            true,
            true
        );
    }

    /** searches for read-only flag in the args of the model and sets it to the given value,
        if the value was not found, it will be appended.
        @param  _bReadOnly
            If <TRUE/> the document will be switched to readonly mode
    */
    void updateDocumentTitle();

    void registerProperties();

    /** determines whether the document we represent supports embedded scripts and macros
    */
    sal_Bool objectSupportsEmbeddedScripts() const;

    //- commands

    void onCommandGetDocumentProperties( ::com::sun::star::uno::Any& _rProps );
    void onCommandInsert( const OUString& _sURL, const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >& Environment ) throw( ::com::sun::star::uno::Exception );
    void onCommandPreview( ::com::sun::star::uno::Any& _rImage );
    ::com::sun::star::uno::Any
        onCommandOpenSomething(
            const ::com::sun::star::uno::Any& _rArgument,
            const bool _bActivate,
            const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >& _rxEnvironment
        );
private:
    using ::cppu::OPropertySetHelper::getFastPropertyValue;
};

class NameChangeNotifier
{
public:
    NameChangeNotifier(
        ODocumentDefinition& i_rDocumentDefinition,
        const OUString& i_rNewName,
        ::osl::ResettableMutexGuard& i_rClearForNotify
    );
    ~NameChangeNotifier();

private:
            ODocumentDefinition&            m_rDocumentDefinition;
    const   ::com::sun::star::uno::Any      m_aOldValue;
    const   ::com::sun::star::uno::Any      m_aNewValue;
    ::osl::ResettableMutexGuard&    m_rClearForNotify;

    void    impl_fireEvent_throw( const sal_Bool i_bVetoable );
};

}   // namespace dbaccess

#endif // _DBA_COREDATAACCESS_DOCUMENTDEFINITION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

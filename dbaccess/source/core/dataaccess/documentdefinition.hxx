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

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_DOCUMENTDEFINITION_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_DOCUMENTDEFINITION_HXX

#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/implbase.hxx>
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

typedef ::cppu::ImplHelper  <   css::embed::XComponentSupplier
                            ,   css::sdb::XSubDocument
                            ,   css::util::XCloseListener
                            ,   css::container::XHierarchicalName
                            >   ODocumentDefinition_Base;

class ODocumentDefinition
        :public OContentHelper
        ,public ::comphelper::OPropertyStateContainer
        ,public ::comphelper::OPropertyArrayUsageHelper< ODocumentDefinition >
        ,public ODocumentDefinition_Base
{
    css::uno::Reference< css::embed::XEmbeddedObject>         m_xEmbeddedObject;
    css::uno::Reference< css::embed::XStateChangeListener >   m_xListener;
    css::uno::Reference< css::sdbc::XConnection >             m_xLastKnownConnection;

    rtl::Reference<OInterceptor>                              m_pInterceptor;
    bool                                                      m_bForm; // <TRUE/> if it is a form
    bool                                                      m_bOpenInDesign;
    bool                                                      m_bInExecute;
    bool                                                      m_bRemoveListener;
    rtl::Reference<OEmbeddedClientHelper>                     m_pClientHelper;

protected:
    virtual ~ODocumentDefinition() override;

public:

    ODocumentDefinition(
            const css::uno::Reference< css::uno::XInterface >& _rxContainer,
            const css::uno::Reference< css::uno::XComponentContext >&,
            const TContentPtr& _pImpl,
            bool _bForm
        );

    void    initialLoad(
                const css::uno::Sequence< sal_Int8 >& i_rClassID,
                const css::uno::Sequence< css::beans::PropertyValue >& i_rCreationArgs,
                const css::uno::Reference< css::sdbc::XConnection >& i_rConnection
            );

    virtual css::uno::Sequence<css::uno::Type> SAL_CALL getTypes()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId()
        throw (css::uno::RuntimeException, std::exception) override;

// css::uno::XInterface
    DECLARE_XINTERFACE( )

// css::beans::XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

    // OPropertySetHelper
    virtual void SAL_CALL getFastPropertyValue(
                                css::uno::Any& o_rValue,
                                sal_Int32 i_nHandle
                            ) const override;

    // XComponentSupplier
    virtual css::uno::Reference< css::util::XCloseable > SAL_CALL getComponent(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XSubDocument
    virtual css::uno::Reference< css::lang::XComponent > SAL_CALL open(  ) throw (css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::lang::XComponent > SAL_CALL openDesign(  ) throw (css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL store(  ) throw (css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL close(  ) throw (css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XHierarchicalName
    virtual OUString SAL_CALL getHierarchicalName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL composeHierarchicalName( const OUString& aRelativeName ) throw (css::lang::IllegalArgumentException, css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;

// OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

    // XCommandProcessor
    virtual css::uno::Any SAL_CALL execute( const css::ucb::Command& aCommand, sal_Int32 CommandId, const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment ) throw (css::uno::Exception, css::ucb::CommandAbortedException, css::uno::RuntimeException, std::exception) override ;

    // XRename
    virtual void SAL_CALL rename( const OUString& newName ) throw (css::sdbc::SQLException, css::container::ElementExistException, css::uno::RuntimeException, std::exception) override;

    // XCloseListener
    virtual void SAL_CALL queryClosing( const css::lang::EventObject& Source, sal_Bool GetsOwnership ) throw (css::util::CloseVetoException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL notifyClosing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;

    /** returns the forms/reports container storage, depending on m_bForm. Our own storage
        inside this container storage is the one with the name as indicated by m_pImpl->m_aProps.sPersistentName.
    */
    css::uno::Reference< css::embed::XStorage >
        getContainerStorage() const;

    bool save(bool _bApprove);
    void saveAs();
    void closeObject();
    bool isModified();
    inline bool isNewReport() const { return !m_bForm && !m_pImpl->m_aProps.bAsTemplate; }

    static void fillReportData(
                    const css::uno::Reference< css::uno::XComponentContext > & _rxContext,
                    const css::uno::Reference< css::util::XCloseable >& _rxComponent,
                    const css::uno::Reference< css::sdbc::XConnection >& _rxActiveConnection
                );

    const css::uno::Reference< css::sdbc::XConnection >&
        getConnection() const { return m_xLastKnownConnection; }

    /** prepares closing the document component

        The method suspends the controller associated with the document, and saves the document
        if necessary.

        @return
            <TRUE/> if and only if the document component can be closed
    */
    bool prepareClose();

    static OUString GetDocumentServiceFromMediaType(
        const OUString& _rMediaType,
        const css::uno::Reference< css::uno::XComponentContext > & _rxContext,
        css::uno::Sequence< sal_Int8 >& _rClassId
    );
    static OUString GetDocumentServiceFromMediaType(
        const css::uno::Reference< css::embed::XStorage >& _rxContainerStorage,
        const OUString& _rEntityName,
        const css::uno::Reference< css::uno::XComponentContext > & _rxContext,
        css::uno::Sequence< sal_Int8 >& _rClassId
    );

    struct NotifierAccess { friend class NameChangeNotifier; private: NotifierAccess() { } };
    const OUString& getCurrentName() const { return m_pImpl->m_aProps.aTitle; }
    void firePropertyChange(
                  sal_Int32 i_nHandle,
            const css::uno::Any& i_rNewValue,
            const css::uno::Any& i_rOldValue,
                  bool i_bVetoable,
            const NotifierAccess&
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
    static void impl_initFormEditView( const css::uno::Reference< css::frame::XController >& _rxController );

    /** removes the given frame from the desktop's frame collection
        @throws css::uno::RuntimeException
    */
    static void impl_removeFrameFromDesktop_throw(
                    const css::uno::Reference< css::uno::XComponentContext >& _rContxt,
                    const css::uno::Reference< css::frame::XFrame >& _rxFrame
                );

    /** opens the UI for this sub document
    */
    css::uno::Reference< css::lang::XComponent >
            impl_openUI_nolck_throw( bool _bForEditing );

    /** stores our document, if it's already loaded
    */
    void    impl_store_throw();

    /** closes our document, if it's open
    */
    bool    impl_close_throw();

    /** returns our component, creates it if necessary
    */
    css::uno::Reference< css::util::XCloseable >
            impl_getComponent_throw( const bool i_ForceCreate = true );

    /** shows or hides our component

        The embedded object must exist, and be in state LOADED, at least.
    */
    void    impl_showOrHideComponent_throw( const bool i_bShow );

    // OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

    virtual void getPropertyDefaultByHandle( sal_Int32 _nHandle, css::uno::Any& _rDefault ) const override;

    // helper
    virtual void SAL_CALL disposing() override;

    // OContentHelper overridables
    virtual OUString determineContentType() const override;

    /** fills the load arguments
    */
    css::uno::Sequence< css::beans::PropertyValue >
        fillLoadArgs(
            const css::uno::Reference< css::sdbc::XConnection>& _xConnection,
            const bool _bSuppressMacros,
            const bool _bReadOnly,
            const css::uno::Sequence< css::beans::PropertyValue >& i_rOpenCommandArguments,
            css::uno::Sequence< css::beans::PropertyValue >& _out_rEmbeddedObjectDescriptor
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
    static void separateOpenCommandArguments(
            const css::uno::Sequence< css::beans::PropertyValue >&    i_rOpenCommandArguments,
            ::comphelper::NamedValueCollection&                                                 o_rDocumentLoadArgs,
            ::comphelper::NamedValueCollection&                                                 o_rEmbeddedObjectDescriptor
        );

    /** loads the EmbeddedObject if not already loaded
        @param  _aClassID
            If set, it will be used to create the embedded object.
    */
    void loadEmbeddedObject(
                const css::uno::Reference< css::sdbc::XConnection>& _xConnection,
                const css::uno::Sequence< sal_Int8 >& _aClassID,
                const css::uno::Sequence< css::beans::PropertyValue >& _rAdditionalArgs,
                const bool _bSuppressMacros,
                const bool _bReadOnly
            );

    /** loads the embedded object for preview. Macros will be suppressed, and the document will
        be read-only.
    */
    void    loadEmbeddedObjectForPreview()
    {
        loadEmbeddedObject(
            nullptr,
            css::uno::Sequence< sal_Int8 >(),
            css::uno::Sequence< css::beans::PropertyValue >(),
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
    bool objectSupportsEmbeddedScripts() const;

    //- commands

    void onCommandGetDocumentProperties( css::uno::Any& _rProps );
    void onCommandInsert( const OUString& _sURL, const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment ) throw( css::uno::Exception );
    void onCommandPreview( css::uno::Any& _rImage );
    css::uno::Any
        onCommandOpenSomething(
            const css::uno::Any& _rArgument,
            const bool _bActivate,
            const css::uno::Reference< css::ucb::XCommandEnvironment >& _rxEnvironment
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
    const   css::uno::Any                   m_aOldValue;
    const   css::uno::Any                   m_aNewValue;
    ::osl::ResettableMutexGuard&            m_rClearForNotify;

    void    impl_fireEvent_throw( const bool i_bVetoable );
};

}   // namespace dbaccess

#endif // INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_DOCUMENTDEFINITION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

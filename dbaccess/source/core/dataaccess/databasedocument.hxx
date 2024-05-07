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

#include <sal/config.h>

#include <map>
#include <memory>

#include <ModelImpl.hxx>
#include "documenteventnotifier.hxx"

#include <com/sun/star/document/XDocumentSubStorageSupplier.hpp>
#include <com/sun/star/frame/DoubleInitializationException.hpp>
#include <com/sun/star/frame/XModel3.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <com/sun/star/frame/XTitleChangeBroadcaster.hpp>
#include <com/sun/star/frame/XUntitledNumbers.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/sdb/XReportDocumentsSupplier.hpp>
#include <com/sun/star/sdb/XFormDocumentsSupplier.hpp>
#include <com/sun/star/view/XPrintable.hpp>
#include <com/sun/star/frame/XModuleManager2.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/NotInitializedException.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/embed/XTransactionListener.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>
#include <com/sun/star/script/XStorageBasedLibraryContainer.hpp>
#include <com/sun/star/script/provider/XScriptProviderSupplier.hpp>
#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/document/XDocumentEventBroadcaster.hpp>
#include <com/sun/star/document/XDocumentRecovery.hpp>
#include <com/sun/star/ui/XUIConfigurationManager2.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/XModifiable.hpp>

#include <comphelper/interfacecontainer3.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase3.hxx>
#include <rtl/ref.hxx>

namespace comphelper {
    class NamedValueCollection;
}

namespace dbaccess
{

class DocumentEvents;
class DocumentEventExecutor;
class DocumentGuard;

typedef std::vector< css::uno::Reference< css::frame::XController > >   Controllers;

// ViewMonitor
/** helper class monitoring the views of a document, and firing appropriate events
    when views are attached / detached
*/
class ViewMonitor
{
public:
    explicit ViewMonitor( DocumentEventNotifier& _rEventNotifier )
        :m_rEventNotifier( _rEventNotifier )
        ,m_bIsNewDocument( true )
        ,m_bEverHadController( false )
        ,m_bLastIsFirstEverController( false )
        ,m_xLastConnectedController()
    {
    }

    ViewMonitor(const ViewMonitor&) = delete;
    const ViewMonitor& operator=(const ViewMonitor&) = delete;

    void    reset()
    {
        m_bEverHadController = false;
        m_bLastIsFirstEverController = false;
        m_xLastConnectedController.clear();
    }

    /** to be called when a view (aka controller) has been connected to the document
        @return
            <TRUE/> if and only if this was the first-ever controller connected to the document
    */
    bool    onControllerConnected(
                const css::uno::Reference< css::frame::XController >& _rxController
             );

    /**  to be called when a controller is set as current controller
        @return <TRUE/>
            if and only if the controller connection indicates that loading the document is finished. This
            is the case if the given controller has previously been connected, and it was the first controller
            ever for which this happened.
    */
    bool    onSetCurrentController(
                const css::uno::Reference< css::frame::XController >& _rxController
             );

    void    onLoadedDocument() { m_bIsNewDocument = false; }

private:
    DocumentEventNotifier&  m_rEventNotifier;
    bool                    m_bIsNewDocument;
    bool                    m_bEverHadController;
    bool                    m_bLastIsFirstEverController;
    css::uno::Reference< css::frame::XController >
                            m_xLastConnectedController;
};

// ODatabaseDocument
typedef cppu::PartialWeakComponentImplHelper<   css::frame::XModel3
                                                ,   css::util::XModifiable
                                                ,   css::frame::XStorable
                                                ,   css::document::XEventBroadcaster
                                                ,   css::document::XDocumentEventBroadcaster
                                                ,   css::view::XPrintable
                                                ,   css::util::XCloseable
                                                ,   css::lang::XServiceInfo
                                                ,   css::sdb::XOfficeDatabaseDocument
                                                ,   css::ui::XUIConfigurationManagerSupplier
                                                ,   css::document::XStorageBasedDocument
                                                ,   css::document::XEmbeddedScripts
                                                ,   css::document::XScriptInvocationContext
                                                ,   css::script::provider::XScriptProviderSupplier
                                                ,   css::document::XEventsSupplier
                                                ,   css::frame::XLoadable
                                                ,   css::document::XDocumentRecovery
                                                , css::frame::XTitle
                                                ,   css::frame::XTitleChangeBroadcaster
                                                ,   css::frame::XUntitledNumbers
                                                >   ODatabaseDocument_OfficeDocument;

class ODatabaseDocument :public ModelDependentComponent             // ModelDependentComponent must be first!
                        ,public ODatabaseDocument_OfficeDocument
{
    enum InitState
    {
        NotInitialized,
        Initializing,
        Initialized
    };

    typedef std::map< OUString, css::uno::Reference< css::frame::XUntitledNumbers > > TNumberedController;
    css::uno::Reference< css::ui::XUIConfigurationManager2>                                     m_xUIConfigurationManager;

    ::comphelper::OInterfaceContainerHelper3<css::util::XModifyListener>                        m_aModifyListeners;
    ::comphelper::OInterfaceContainerHelper3<css::util::XCloseListener>                         m_aCloseListener;
    ::comphelper::OInterfaceContainerHelper3<css::document::XStorageChangeListener>             m_aStorageListeners;

    std::unique_ptr<DocumentEvents>                                                             m_pEventContainer;
    ::rtl::Reference< DocumentEventExecutor >                                                   m_pEventExecutor;
    DocumentEventNotifier                                                                       m_aEventNotifier;

    css::uno::Reference< css::frame::XController >                                              m_xCurrentController;
    Controllers                                                                                 m_aControllers;
    ViewMonitor                                                                                 m_aViewMonitor;

    css::uno::WeakReference< css::container::XNameAccess >                                      m_xForms;
    css::uno::WeakReference< css::container::XNameAccess >                                      m_xReports;
    css::uno::WeakReference< css::script::provider::XScriptProvider >                           m_xScriptProvider;

    /** @short  such module manager is used to classify new opened documents. */
    css::uno::Reference< css::frame::XModuleManager2 >                                          m_xModuleManager;
    css::uno::Reference< css::frame::XTitle >                                                   m_xTitleHelper;
    TNumberedController                                                                         m_aNumberedControllers;

    /** true if and only if the DatabaseDocument's "initNew" or "load" have been called (or, well,
        the document has be initialized implicitly - see storeAsURL
    */
    InitState                                                                                   m_eInitState;
    bool                                                                                        m_bClosing;
    bool                                                                                        m_bAllowDocumentScripting;
    bool                                                                                        m_bHasBeenRecovered;
    /// If XModel::attachResource() was called to inform us that the document is embedded into another one.
    bool                                                                                        m_bEmbedded;

    enum StoreType { SAVE, SAVE_AS };
    /** stores the document to the given URL, rebases it to the respective new storage, if necessary, resets
        the modified flag, and notifies any listeners as required

        @param _rURL
            the URL to store the document to
        @param _rArguments
            arguments for storing the document (MediaDescriptor)
        @param _eType
            the type of the store process (Save or SaveAs). The method will automatically
            notify the proper events for this type.
        @param _rGuard
            the instance lock to be released before doing synchronous notifications
        @throws css::io::IOException
        @throws css::uno::RuntimeException
    */
    void impl_storeAs_throw(
            const OUString& _rURL,
            const ::comphelper::NamedValueCollection& _rArguments,
            const StoreType _eType,
            DocumentGuard& _rGuard
         );

    /** notifies our storage change listeners that our underlying storage changed

        @param _rxNewRootStorage
            the new root storage to be notified. If <NULL/>, it is assumed that no storage change actually
            happened, and the listeners are not notified.
    */
    void    impl_notifyStorageChange_nolck_nothrow(
                const css::uno::Reference< css::embed::XStorage >& _rxNewRootStorage
            );

    /// write a single XML stream into the package
    void WriteThroughComponent(
        const css::uno::Reference< css::lang::XComponent > & xComponent,  /// the component we export
        const OUString& rStreamName,                                                                /// the stream name
        const OUString& rServiceName,                                                               /// service name of the component
        const css::uno::Sequence< css::uno::Any> & rArguments,            /// the argument (XInitialization)
        const css::uno::Sequence< css::beans::PropertyValue> & rMediaDesc,/// output descriptor
        const css::uno::Reference< css::embed::XStorage >& _xStorageToSaveTo
    ) const;

    /// write a single output stream
    /// (to be called either directly or by WriteThroughComponent(...))
    void WriteThroughComponent(
        const css::uno::Reference< css::io::XOutputStream >& xOutputStream,
        const css::uno::Reference< css::lang::XComponent >& xComponent,
        const OUString& rServiceName,
        const css::uno::Sequence< css::uno::Any >& rArguments,
        const css::uno::Sequence< css::beans::PropertyValue> & rMediaDesc
    ) const;

    /** writes the content and settings
        @param  sURL
            The URL
        @param  lArguments
            The media descriptor
        @param  _xStorageToSaveTo
            The storage which should be used for saving
    */
    void impl_writeStorage_throw(
        const css::uno::Reference< css::embed::XStorage >& _rxTargetStorage,
        const ::comphelper::NamedValueCollection& _rMediaDescriptor
    ) const;

    // ModelDependentComponent overridables
    virtual css::uno::Reference< css::uno::XInterface > getThis() const override;

    css::uno::Reference< css::frame::XTitle > const &     impl_getTitleHelper_throw();
    css::uno::Reference< css::frame::XUntitledNumbers >   impl_getUntitledHelper_throw(
        const css::uno::Reference< css::uno::XInterface >& _xComponent = css::uno::Reference< css::uno::XInterface >());

private:
    explicit ODatabaseDocument(const ::rtl::Reference<ODatabaseModelImpl>& _pImpl);
    // Do NOT create those documents directly, always use ODatabaseModelImpl::getModel. Reason is that
    // ODatabaseDocument requires clear ownership, and in turn lifetime synchronisation with the ModelImpl.
    // If you create a ODatabaseDocument directly, you might easily create a leak.
    // #i50905#

protected:
    virtual void SAL_CALL disposing() override;

    virtual ~ODatabaseDocument() override;

public:
    struct FactoryAccess { friend class ODatabaseModelImpl; private: FactoryAccess() { } };
    static rtl::Reference<ODatabaseDocument> createDatabaseDocument( const ::rtl::Reference<ODatabaseModelImpl>& _pImpl, FactoryAccess /*accessControl*/ )
    {
        return new ODatabaseDocument( _pImpl );
    }

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // XInterface
    virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

    // XComponent
    virtual void SAL_CALL dispose(  ) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // XModel
    virtual sal_Bool SAL_CALL attachResource( const OUString& URL, const css::uno::Sequence< css::beans::PropertyValue >& Arguments ) override ;
    virtual OUString SAL_CALL getURL(  ) override ;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getArgs(  ) override ;
    virtual void SAL_CALL connectController( const css::uno::Reference< css::frame::XController >& Controller ) override ;
    virtual void SAL_CALL disconnectController( const css::uno::Reference< css::frame::XController >& Controller ) override ;
    virtual void SAL_CALL lockControllers(  ) override ;
    virtual void SAL_CALL unlockControllers(  ) override ;
    virtual sal_Bool SAL_CALL hasControllersLocked(  ) override ;
    virtual css::uno::Reference< css::frame::XController > SAL_CALL getCurrentController(  ) override ;
    virtual void SAL_CALL setCurrentController( const css::uno::Reference< css::frame::XController >& Controller ) override ;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getCurrentSelection(  ) override ;

    // XModel2
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL getControllers(  ) override ;
    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableViewControllerNames(  ) override ;
    virtual css::uno::Reference< css::frame::XController2 > SAL_CALL createDefaultViewController( const css::uno::Reference< css::frame::XFrame >& Frame ) override ;
    virtual css::uno::Reference< css::frame::XController2 > SAL_CALL createViewController( const OUString& ViewName, const css::uno::Sequence< css::beans::PropertyValue >& Arguments, const css::uno::Reference< css::frame::XFrame >& Frame ) override ;
    virtual void SAL_CALL setArgs(const css::uno::Sequence<css::beans::PropertyValue>& aArgs) override;

    // XModel3
    virtual ::css::uno::Sequence< ::css::beans::PropertyValue > SAL_CALL getArgs2( const ::css::uno::Sequence< ::rtl::OUString >& requestedArgs ) override;

    // XStorable
    virtual sal_Bool SAL_CALL hasLocation(  ) override ;
    virtual OUString SAL_CALL getLocation(  ) override ;
    virtual sal_Bool SAL_CALL isReadonly(  ) override ;
    virtual void SAL_CALL store(  ) override ;
    virtual void SAL_CALL storeAsURL( const OUString& sURL, const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) override ;
    virtual void SAL_CALL storeToURL( const OUString& sURL, const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) override ;

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void SAL_CALL removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) override;

    // css::util::XModifiable
    virtual sal_Bool SAL_CALL isModified(  ) override ;
    virtual void SAL_CALL setModified( sal_Bool bModified ) override ;

    // XEventBroadcaster
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::document::XEventListener >& aListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::document::XEventListener >& aListener ) override;

    // XDocumentEventBroadcaster
    virtual void SAL_CALL addDocumentEventListener( const css::uno::Reference< css::document::XDocumentEventListener >& Listener ) override;
    virtual void SAL_CALL removeDocumentEventListener( const css::uno::Reference< css::document::XDocumentEventListener >& Listener ) override;
    virtual void SAL_CALL notifyDocumentEvent( const OUString& EventName, const css::uno::Reference< css::frame::XController2 >& ViewController, const css::uno::Any& Supplement ) override;

    // XPrintable
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getPrinter(  ) override ;
    virtual void SAL_CALL setPrinter( const css::uno::Sequence< css::beans::PropertyValue >& aPrinter ) override ;
    virtual void SAL_CALL print( const css::uno::Sequence< css::beans::PropertyValue >& xOptions ) override ;

    // XFormDocumentsSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getFormDocuments(  ) override;

    // XReportDocumentsSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getReportDocuments(  ) override;

    // XCloseable
    virtual void SAL_CALL close( sal_Bool DeliverOwnership ) override;
    virtual void SAL_CALL addCloseListener( const css::uno::Reference< css::util::XCloseListener >& Listener ) override;
    virtual void SAL_CALL removeCloseListener( const css::uno::Reference< css::util::XCloseListener >& Listener ) override;

    // XUIConfigurationManagerSupplier
    virtual css::uno::Reference< css::ui::XUIConfigurationManager > SAL_CALL getUIConfigurationManager(  ) override;

    // XDocumentSubStorageSupplier
    virtual css::uno::Reference< css::embed::XStorage > SAL_CALL getDocumentSubStorage( const OUString& aStorageName, sal_Int32 nMode ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getDocumentSubStoragesNames(  ) override;

    // XOfficeDatabaseDocument
    virtual css::uno::Reference< css::sdbc::XDataSource > SAL_CALL getDataSource() override;

    // XStorageBasedDocument
    virtual void SAL_CALL loadFromStorage( const css::uno::Reference< css::embed::XStorage >& xStorage, const css::uno::Sequence< css::beans::PropertyValue >& aMediaDescriptor ) override;
    virtual void SAL_CALL storeToStorage( const css::uno::Reference< css::embed::XStorage >& xStorage, const css::uno::Sequence< css::beans::PropertyValue >& aMediaDescriptor ) override;
    virtual void SAL_CALL switchToStorage( const css::uno::Reference< css::embed::XStorage >& xStorage ) override;
    virtual css::uno::Reference< css::embed::XStorage > SAL_CALL getDocumentStorage(  ) override;
    virtual void SAL_CALL addStorageChangeListener( const css::uno::Reference< css::document::XStorageChangeListener >& xListener ) override;
    virtual void SAL_CALL removeStorageChangeListener( const css::uno::Reference< css::document::XStorageChangeListener >& xListener ) override;

    // XEmbeddedScripts
    virtual css::uno::Reference< css::script::XStorageBasedLibraryContainer > SAL_CALL getBasicLibraries() override;
    virtual css::uno::Reference< css::script::XStorageBasedLibraryContainer > SAL_CALL getDialogLibraries() override;
    virtual sal_Bool SAL_CALL getAllowMacroExecution() override;

    // XScriptInvocationContext
    virtual css::uno::Reference< css::document::XEmbeddedScripts > SAL_CALL getScriptContainer() override;

    // XScriptProviderSupplier
    virtual css::uno::Reference< css::script::provider::XScriptProvider > SAL_CALL getScriptProvider(  ) override;

    // XEventsSupplier
    virtual css::uno::Reference< css::container::XNameReplace > SAL_CALL getEvents(  ) override;

    // XLoadable
    virtual void SAL_CALL initNew(  ) override;
    virtual void SAL_CALL load( const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) override;

    // css.document.XDocumentRecovery
    virtual sal_Bool SAL_CALL wasModifiedSinceLastSave() override;
    virtual void SAL_CALL storeToRecoveryFile( const OUString& i_TargetLocation, const css::uno::Sequence< css::beans::PropertyValue >& i_MediaDescriptor ) override;
    virtual void SAL_CALL recoverFromFile( const OUString& i_SourceLocation, const OUString& i_SalvagedFile, const css::uno::Sequence< css::beans::PropertyValue >& i_MediaDescriptor ) override;

    // XTitle
    virtual OUString SAL_CALL getTitle(  ) override;
    virtual void SAL_CALL setTitle( const OUString& sTitle ) override;

    // XTitleChangeBroadcaster
    virtual void SAL_CALL addTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener ) override;
    virtual void SAL_CALL removeTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener ) override;

    // XUntitledNumbers
    virtual ::sal_Int32 SAL_CALL leaseNumber( const css::uno::Reference< css::uno::XInterface >& xComponent ) override;
    virtual void SAL_CALL releaseNumber( ::sal_Int32 nNumber ) override;
    virtual void SAL_CALL releaseNumberForComponent( const css::uno::Reference< css::uno::XInterface >& xComponent ) override;
    virtual OUString SAL_CALL getUntitledPrefix(  ) override;

    /** clears the given object container

        Clearing is done via disposal - the method calls XComponent::dispose at the given object,
        which must be one of our impl's or our object containers (m_xForms, m_xReports,
        m_xTableDefinitions, m_xCommandDefinitions)

        @param _rxContainer
            the container to clear
    */
    static void clearObjectContainer(
                css::uno::WeakReference< css::container::XNameAccess >& _rxContainer);

    /** checks whether the component is already initialized, throws a NotInitializedException if not
    */
    void checkInitialized() const
    {
        if ( !impl_isInitialized() )
            throw css::lang::NotInitializedException( OUString(), getThis() );
    }

    /** checks the document is currently in the initialization phase, or already initialized.
        Throws NotInitializedException if not so.
    */
    void checkNotUninitialized() const
    {
        if ( impl_isInitialized() || impl_isInitializing() )
            // fine
            return;

        throw css::lang::NotInitializedException( OUString(), getThis() );
    }

    /** checks whether the document is currently being initialized, or already initialized,
        throws a DoubleInitializationException if so
    */
    void checkNotInitialized() const
    {
        if ( impl_isInitializing() || impl_isInitialized() )
            throw css::frame::DoubleInitializationException( OUString(), getThis() );
    }

private:
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::ui::XUIConfigurationManager2 > const & getUIConfigurationManager2();

    /** returns whether the model is currently being initialized
    */
    bool    impl_isInitializing() const { return m_eInitState == Initializing; }

    /** returns whether the model is already initialized, i.e. the XModel's "initNew" or "load" methods have been called
    */
    bool    impl_isInitialized() const { return m_eInitState == Initialized; }

    /// tells the model it is being initialized now
    void    impl_setInitializing() { m_eInitState = Initializing; }

    /// tells the model its initialization is done
    void    impl_setInitialized();

    /** closes the frames of all connected controllers

    @param _bDeliverOwnership
        determines if the ownership should be transferred to the component which
        possibly vetos the closing

    @throws css::util::CloseVetoException
        if the closing was vetoed by any instance
    */
    void    impl_closeControllerFrames_nolck_throw( bool _bDeliverOwnership );

    /** disposes the frames of all controllers which are still left in m_aControllers.
    */
    void    impl_disposeControllerFrames_nothrow();

    /** does a reparenting at the given object container to ourself

        Calls XChild::setParent at the given object, which must be one of our impl's or our
        object containers (m_xForms, m_xReports, m_xTableDefinitions, m_xCommandDefinitions)
    */
    void    impl_reparent_nothrow( const css::uno::WeakReference< css::container::XNameAccess >& _rxContainer );

    /** retrieves the forms or reports contained, creates and initializes it, if necessary

        @throws DisposedException
            if the instance is already disposed
        @throws IllegalArgumentException
            if <arg>_eType</arg> is not ODatabaseModelImpl::E_FORM and not ODatabaseModelImpl::E_REPORT
    */
    css::uno::Reference< css::container::XNameAccess >
            impl_getDocumentContainer_throw( ODatabaseModelImpl::ObjectType _eType );

    /** resets everything

        @precond
            m_pImpl is not <NULL/>
    */
    void
            impl_reset_nothrow();

    /** imports the document from the given resource.
    */
    static void
            impl_import_nolck_throw(
                const css::uno::Reference< css::uno::XComponentContext >& _rContext,
                const css::uno::Reference< css::uno::XInterface >& _rxTargetComponent,
                const ::comphelper::NamedValueCollection& _rResource
            );

    /** creates a storage for the given URL, truncating it if a file with this name already exists

        @throws Exception
            if creating the storage failed

        @return
            the newly created storage for the file at the given URL
    */
    css::uno::Reference< css::embed::XStorage >
            impl_createStorageFor_throw(
                const OUString& _rURL
            ) const;

    /** Extracts storage from arguments, or creates for the given URL, truncating it if a file with
        this name already exists

        @throws Exception
            if creating the storage failed

        @return
            the storage that is either extracted from arguments, or newly created for the file at
            the given URL
    */
    css::uno::Reference<css::embed::XStorage> impl_GetStorageOrCreateFor_throw(
        const ::comphelper::NamedValueCollection& _rArguments, const OUString& _rURL) const;

    /** sets our "modified" flag

        will notify all our respective listeners, if the "modified" state actually changed

        @param _bModified
            the (new) flag indicating whether the document is currently modified or not
        @param _rGuard
            the guard for our instance. At method entry, the guard must hold the lock. At the moment
            of method leave, the lock will be released.
        @precond
            our mutex is locked
        @postcond
            our mutex is not locked
    */
    void    impl_setModified_nothrow( bool _bModified, DocumentGuard& _rGuard );

    /** stores the document to the given storage

        Note that the document is actually not rebased to this storage, it just stores a copy of itself
        to the given target storage.

        @param _rxTargetStorage
            denotes the storage to store the document into
        @param _rMediaDescriptor
            contains additional parameters for storing the document
        @param _rDocGuard
            a guard which holds the (only) lock to the document, and which will be temporarily
            released where necessary (e.g. for notifications, or calling into other components)

        @throws css::uno::IllegalArgumentException
            if the given storage is <NULL/>.

        @throws css::uno::RuntimeException
            when any of the used operations throws it

        @throws css::io::IOException
            when any of the used operations throws it, or any other exception occurs which is no
            RuntimeException and no IOException
    */
    void    impl_storeToStorage_throw(
                const css::uno::Reference< css::embed::XStorage >& _rxTargetStorage,
                const css::uno::Sequence< css::beans::PropertyValue >& _rMediaDescriptor,
                DocumentGuard& _rDocGuard
            ) const;

    /** impl-version of attachResource

        @param  i_rLogicalDocumentURL
            denotes the logical URL of the document, to be reported by getURL/getLocation
        @param  i_rMediaDescriptor
            denotes additional document parameters
        @param  _rDocGuard
            is the guard which currently protects the document instance
    */
    bool    impl_attachResource(
                    const OUString& i_rLogicalDocumentURL,
                    const css::uno::Sequence< css::beans::PropertyValue >& i_rMediaDescriptor,
                    DocumentGuard& _rDocGuard
                );

    /** throws an IOException with the message as defined in the RID_STR_ERROR_WHILE_SAVING resource, wrapping
        the given caught non-IOException error
    */
    void        impl_throwIOExceptionCausedBySave_throw(
                    const css::uno::Any& i_rError,
                    std::u16string_view i_rTargetURL
                ) const;
};

/** an extended version of the ModelMethodGuard, which also cares for the initialization state
    of the document
*/
class DocumentGuard : private ModelMethodGuard
{
public:
    enum InitMethod_
    {
        // a method which is to initialize the document
        InitMethod,
    };

    enum DefaultMethod_
    {
        // a default method
        DefaultMethod
    };

    enum MethodUsedDuringInit_
    {
        // a method which is used (externally) during the initialization phase
        MethodUsedDuringInit
    };

    enum MethodWithoutInit_
    {
        // a method which does not need initialization - use with care!
        MethodWithoutInit
    };


    /** constructs the guard

        @param _document
            the ODatabaseDocument instance

        @throws css::lang::DisposedException
            If the given component is already disposed

        @throws css::lang::NotInitializedException
            if the given component is not yet initialized
    */
    DocumentGuard(const ODatabaseDocument& _document, DefaultMethod_)
        : ModelMethodGuard(_document)
        , m_document(_document )
    {
        m_document.checkInitialized();
    }

    /** constructs the guard

        @param _document
            the ODatabaseDocument instance

        @throws css::lang::DisposedException
            If the given component is already disposed

        @throws css::frame::DoubleInitializationException
            if the given component is already initialized, or currently being initialized.
    */
    DocumentGuard(const ODatabaseDocument& _document, InitMethod_)
        : ModelMethodGuard(_document)
        , m_document(_document)
    {
        m_document.checkNotInitialized();
    }

    /** constructs the guard

        @param _document
            the ODatabaseDocument instance

        @throws css::lang::DisposedException
            If the given component is already disposed

        @throws css::lang::NotInitializedException
            if the component is still uninitialized, and not in the initialization
            phase currently.
    */
    DocumentGuard(const ODatabaseDocument& _document, MethodUsedDuringInit_)
        : ModelMethodGuard(_document)
        , m_document(_document)
    {
        m_document.checkNotUninitialized();
    }

    /** constructs the guard

        @param _document
            the ODatabaseDocument instance

        @throws css::lang::DisposedException
            If the given component is already disposed
    */
    DocumentGuard(const ODatabaseDocument& _document, MethodWithoutInit_)
        : ModelMethodGuard( _document )
        , m_document( _document )
    {
    }

    void clear()
    {
        ModelMethodGuard::clear();
    }
    void reset()
    {
        ModelMethodGuard::reset();
        m_document.checkDisposed();
    }

private:

    const ODatabaseDocument& m_document;
};

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

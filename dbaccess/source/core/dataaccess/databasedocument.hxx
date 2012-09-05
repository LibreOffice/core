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
#ifndef _DBA_COREDATAACCESS_DATABASEDOCUMENT_HXX_
#define _DBA_COREDATAACCESS_DATABASEDOCUMENT_HXX_

#include "ModelImpl.hxx"
#include "documenteventnotifier.hxx"

#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/document/XDocumentSubStorageSupplier.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <com/sun/star/frame/XTitleChangeBroadcaster.hpp>
#include <com/sun/star/frame/XUntitledNumbers.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/sdb/XReportDocumentsSupplier.hpp>
#include <com/sun/star/sdb/XFormDocumentsSupplier.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/view/XPrintable.hpp>
#include <com/sun/star/frame/XModuleManager2.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
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

#ifndef INCLUDED_COMPHELPER_IMPLBASE_VAR_HXX_17
#define INCLUDED_COMPHELPER_IMPLBASE_VAR_HXX_17
#define COMPHELPER_IMPLBASE_INTERFACE_NUMBER 17
#include <comphelper/implbase_var.hxx>
#endif

#include <cppuhelper/compbase10.hxx>
#include <cppuhelper/implbase3.hxx>
#include <rtl/ref.hxx>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace comphelper {
    class NamedValueCollection;
}

//........................................................................
namespace dbaccess
{
//........................................................................

class DocumentEvents;
class DocumentEventExecutor;
class DocumentGuard;

typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > >   Controllers;

//============================================================
//= ViewMonitor
//============================================================
/** helper class monitoring the views of a document, and firing appropriate events
    when views are attached / detached
*/
class ViewMonitor : public boost::noncopyable
{
public:
    ViewMonitor( DocumentEventNotifier& _rEventNotifier )
        :m_rEventNotifier( _rEventNotifier )
        ,m_bIsNewDocument( true )
        ,m_bEverHadController( false )
        ,m_bLastIsFirstEverController( false )
        ,m_xLastConnectedController()
    {
    }

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
                const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& _rxController
             );

    /**  to be called when a controller is set as current controller
        @return <TRUE/>
            if and only if the controller connection indicates that loading the document is finished. This
            is the case if the given controller has previously been connected, and it was the first controller
            ever for which this happened.
    */
    bool    onSetCurrentController(
                const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& _rxController
             );

    void    onLoadedDocument() { m_bIsNewDocument = false; }

private:
    DocumentEventNotifier&  m_rEventNotifier;
    bool                    m_bIsNewDocument;
    bool                    m_bEverHadController;
    bool                    m_bLastIsFirstEverController;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >
                            m_xLastConnectedController;
};

//============================================================
//= ODatabaseDocument
//============================================================
typedef ::comphelper::PartialWeakComponentImplHelper17 <   ::com::sun::star::frame::XModel2
                                                ,   ::com::sun::star::util::XModifiable
                                                ,   ::com::sun::star::frame::XStorable
                                                ,   ::com::sun::star::document::XEventBroadcaster
                                                ,   ::com::sun::star::document::XDocumentEventBroadcaster
                                                ,   ::com::sun::star::view::XPrintable
                                                ,   ::com::sun::star::util::XCloseable
                                                ,   ::com::sun::star::lang::XServiceInfo
                                                ,   ::com::sun::star::sdb::XOfficeDatabaseDocument
                                                ,   ::com::sun::star::ui::XUIConfigurationManagerSupplier
                                                ,   ::com::sun::star::document::XStorageBasedDocument
                                                ,   ::com::sun::star::document::XEmbeddedScripts
                                                ,   ::com::sun::star::document::XScriptInvocationContext
                                                ,   ::com::sun::star::script::provider::XScriptProviderSupplier
                                                ,   ::com::sun::star::document::XEventsSupplier
                                                ,   ::com::sun::star::frame::XLoadable
                                                ,   ::com::sun::star::document::XDocumentRecovery
                                                >   ODatabaseDocument_OfficeDocument;

typedef ::cppu::ImplHelper3<    ::com::sun::star::frame::XTitle
                            ,   ::com::sun::star::frame::XTitleChangeBroadcaster
                            ,   ::com::sun::star::frame::XUntitledNumbers
                            >   ODatabaseDocument_Title;

class ODatabaseDocument :public ModelDependentComponent             // ModelDependentComponent must be first!
                        ,public ODatabaseDocument_OfficeDocument
                        ,public ODatabaseDocument_Title
{
    enum InitState
    {
        NotInitialized,
        Initializing,
        Initialized
    };

    DECLARE_STL_USTRINGACCESS_MAP(::com::sun::star::uno::Reference< ::com::sun::star::frame::XUntitledNumbers >,TNumberedController);
    ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager>            m_xUIConfigurationManager;

    ::cppu::OInterfaceContainerHelper                                                           m_aModifyListeners;
    ::cppu::OInterfaceContainerHelper                                                           m_aCloseListener;
    ::cppu::OInterfaceContainerHelper                                                           m_aStorageListeners;

    DocumentEvents*                                                                             m_pEventContainer;
    ::rtl::Reference< DocumentEventExecutor >                                                   m_pEventExecutor;
    DocumentEventNotifier                                                                       m_aEventNotifier;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >                    m_xCurrentController;
    Controllers                                                                                 m_aControllers;
    ViewMonitor                                                                                 m_aViewMonitor;

    ::com::sun::star::uno::WeakReference< ::com::sun::star::container::XNameAccess >            m_xForms;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::container::XNameAccess >            m_xReports;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::script::provider::XScriptProvider > m_xScriptProvider;

    /** @short  such module manager is used to classify new opened documents. */
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModuleManager2 >                 m_xModuleManager;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XTitle >                         m_xTitleHelper;
    TNumberedController                                                                         m_aNumberedControllers;

    /** true if and only if the DatabaseDocument's "initNew" or "load" have been called (or, well,
        the document has be initialized implicitly - see storeAsURL
    */
    InitState                                                                                   m_eInitState;
    bool                                                                                        m_bClosing;
    bool                                                                                        m_bAllowDocumentScripting;
    bool                                                                                        m_bHasBeenRecovered;

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
    */
    void impl_storeAs_throw(
            const ::rtl::OUString& _rURL,
            const ::comphelper::NamedValueCollection& _rArguments,
            const StoreType _eType,
            DocumentGuard& _rGuard
         )
         throw  (   ::com::sun::star::io::IOException
                ,   ::com::sun::star::uno::RuntimeException );

    /** notifies our storage change listeners that our underlying storage changed

        @param _rxNewRootStorage
            the new root storage to be notified. If <NULL/>, it is assumed that no storage change actually
            happened, and the listeners are not notified.
    */
    void    impl_notifyStorageChange_nolck_nothrow(
                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxNewRootStorage
            );

    /// write a single XML stream into the package
    void WriteThroughComponent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > & xComponent,  /// the component we export
        const sal_Char* pStreamName,                                                                /// the stream name
        const sal_Char* pServiceName,                                                               /// service name of the component
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any> & rArguments,            /// the argument (XInitialization)
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue> & rMediaDesc,/// output descriptor
        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _xStorageToSaveTo
    ) const;


    /// write a single output stream
    /// (to be called either directly or by WriteThroughComponent(...))
    void WriteThroughComponent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xOutputStream,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xComponent,
        const sal_Char* pServiceName,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rArguments,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue> & rMediaDesc
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
        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxTargetStorage,
        const ::comphelper::NamedValueCollection& _rMediaDescriptor
    ) const;

    // ModelDependentComponent overridables
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getThis() const;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XTitle >             impl_getTitleHelper_throw();
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XUntitledNumbers >   impl_getUntitledHelper_throw(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xComponent = ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >());

private:
    ODatabaseDocument(const ::rtl::Reference<ODatabaseModelImpl>& _pImpl);
    // Do NOT create those documents directly, always use ODatabaseModelImpl::getModel. Reason is that
    // ODatabaseDocument requires clear ownership, and in turn lifetime synchronisation with the ModelImpl.
    // If you create a ODatabaseDocument directly, you might easily create a leak.
    // #i50905#

protected:
    virtual void SAL_CALL disposing();

    virtual ~ODatabaseDocument();

public:
    struct FactoryAccess { friend class ODatabaseModelImpl; private: FactoryAccess() { } };
    static ODatabaseDocument* createDatabaseDocument( const ::rtl::Reference<ODatabaseModelImpl>& _pImpl, FactoryAccess /*accessControl*/ )
    {
        return new ODatabaseDocument( _pImpl );
    }

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo - static methods
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::rtl::OUString getImplementationName_static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
        SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

    // XInterface
    virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw ();
    virtual void SAL_CALL release(  ) throw ();

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    // XModel
    virtual sal_Bool SAL_CALL attachResource( const ::rtl::OUString& URL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Arguments ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual ::rtl::OUString SAL_CALL getURL(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getArgs(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL connectController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& Controller ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL disconnectController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& Controller ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL lockControllers(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL unlockControllers(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual sal_Bool SAL_CALL hasControllersLocked(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > SAL_CALL getCurrentController(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL setCurrentController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& Controller ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getCurrentSelection(  ) throw (::com::sun::star::uno::RuntimeException) ;

    // XModel2
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL getControllers(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableViewControllerNames(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController2 > SAL_CALL createDefaultViewController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& Frame ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController2 > SAL_CALL createViewController( const ::rtl::OUString& ViewName, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Arguments, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& Frame ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException) ;

    // XStorable
    virtual sal_Bool SAL_CALL hasLocation(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual ::rtl::OUString SAL_CALL getLocation(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual sal_Bool SAL_CALL isReadonly(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL store(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL storeAsURL( const ::rtl::OUString& sURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArguments ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL storeToURL( const ::rtl::OUString& sURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArguments ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException) ;

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::util::XModifiable
    virtual sal_Bool SAL_CALL isModified(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL setModified( sal_Bool bModified ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException) ;

    // XEventBroadcaster
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    // XDocumentEventBroadcaster
    virtual void SAL_CALL addDocumentEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentEventListener >& _Listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeDocumentEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentEventListener >& _Listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL notifyDocumentEvent( const ::rtl::OUString& _EventName, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController2 >& _ViewController, const ::com::sun::star::uno::Any& _Supplement ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

    // XPrintable
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getPrinter(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL setPrinter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aPrinter ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL print( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& xOptions ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException) ;

    // XFormDocumentsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getFormDocuments(  ) throw (::com::sun::star::uno::RuntimeException);

    // XReportDocumentsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getReportDocuments(  ) throw (::com::sun::star::uno::RuntimeException);

    // XCloseable
    virtual void SAL_CALL close( sal_Bool DeliverOwnership ) throw (::com::sun::star::util::CloseVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addCloseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeCloseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);

    // XUIConfigurationManagerSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager > SAL_CALL getUIConfigurationManager(  ) throw (::com::sun::star::uno::RuntimeException);

    // XDocumentSubStorageSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > SAL_CALL getDocumentSubStorage( const ::rtl::OUString& aStorageName, sal_Int32 nMode ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getDocumentSubStoragesNames(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // XOfficeDatabaseDocument
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource > SAL_CALL getDataSource() throw (::com::sun::star::uno::RuntimeException);

    // XStorageBasedDocument
    virtual void SAL_CALL loadFromStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aMediaDescriptor ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::frame::DoubleInitializationException, ::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL storeToStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aMediaDescriptor ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL switchToStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > SAL_CALL getDocumentStorage(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addStorageChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XStorageChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeStorageChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XStorageChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

    // XEmbeddedScripts
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::script::XStorageBasedLibraryContainer > SAL_CALL getBasicLibraries() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::script::XStorageBasedLibraryContainer > SAL_CALL getDialogLibraries() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getAllowMacroExecution() throw (::com::sun::star::uno::RuntimeException);

    // XScriptInvocationContext
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::document::XEmbeddedScripts > SAL_CALL getScriptContainer() throw (::com::sun::star::uno::RuntimeException);

    // XScriptProviderSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::script::provider::XScriptProvider > SAL_CALL getScriptProvider(  ) throw (::com::sun::star::uno::RuntimeException);

    // XEventsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > SAL_CALL getEvents(  ) throw (::com::sun::star::uno::RuntimeException);

    // XLoadable
    virtual void SAL_CALL initNew(  ) throw (::com::sun::star::frame::DoubleInitializationException, ::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL load( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArguments ) throw (::com::sun::star::frame::DoubleInitializationException, ::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // css.document.XDocumentRecovery
    virtual ::sal_Bool SAL_CALL wasModifiedSinceLastSave() throw ( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL storeToRecoveryFile( const ::rtl::OUString& i_TargetLocation, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& i_MediaDescriptor ) throw ( ::com::sun::star::uno::RuntimeException, ::com::sun::star::io::IOException, ::com::sun::star::lang::WrappedTargetException );
    virtual void SAL_CALL recoverFromFile( const ::rtl::OUString& i_SourceLocation, const ::rtl::OUString& i_SalvagedFile, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& i_MediaDescriptor ) throw ( ::com::sun::star::uno::RuntimeException, ::com::sun::star::io::IOException, ::com::sun::star::lang::WrappedTargetException );

    // XTitle
    virtual ::rtl::OUString SAL_CALL getTitle(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTitle( const ::rtl::OUString& sTitle ) throw (::com::sun::star::uno::RuntimeException);

    // XTitleChangeBroadcaster
    virtual void SAL_CALL addTitleChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XTitleChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTitleChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XTitleChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

    // XUntitledNumbers
    virtual ::sal_Int32 SAL_CALL leaseNumber( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xComponent ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL releaseNumber( ::sal_Int32 nNumber ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL releaseNumberForComponent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xComponent ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getUntitledPrefix(  ) throw (::com::sun::star::uno::RuntimeException);

    /** clears the given object container

        Clearing is done via disposal - the method calls XComponent::dispose at the given object,
        which must be one of our impl's or our object containers (m_xForms, m_xReports,
        m_xTableDefinitions, m_xCommandDefinitions)

        @param _rxContainer
            the container to clear
    */
    static void clearObjectContainer(
                ::com::sun::star::uno::WeakReference< ::com::sun::star::container::XNameAccess >& _rxContainer);

    /** checks whether the component is already initialized, throws a NotInitializedException if not
    */
    inline void checkInitialized() const
    {
        if ( !impl_isInitialized() )
            throw ::com::sun::star::lang::NotInitializedException( ::rtl::OUString(), getThis() );
    }

    /** checks the document is currently in the initialization phase, or already initialized.
        Throws NotInitializedException if not so.
    */
    inline void checkNotUninitilized() const
    {
        if ( impl_isInitialized() || impl_isInitializing() )
            // fine
            return;

        throw ::com::sun::star::lang::NotInitializedException( ::rtl::OUString(), getThis() );
    }

    /** checks whether the document is currently being initialized, or already initialized,
        throws a DoubleInitializationException if so
    */
    inline void checkNotInitialized() const
    {
        if ( impl_isInitializing() || impl_isInitialized() )
            throw ::com::sun::star::frame::DoubleInitializationException( ::rtl::OUString(), getThis() );
    }

private:
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
        determines if the ownership should be transfered to the component which
        possibly vetos the closing

    @raises ::com::sun::star::util::CloseVetoException
        if the closing was vetoed by any instance
    */
    void    impl_closeControllerFrames_nolck_throw( sal_Bool _bDeliverOwnership );

    /** disposes the frames of all controllers which are still left in m_aControllers.
    */
    void    impl_disposeControllerFrames_nothrow();

    /** does a reparenting at the given object container to ourself

        Calls XChild::setParent at the given object, which must be one of our impl's or our
        object containers (m_xForms, m_xReports, m_xTableDefinitions, m_xCommandDefinitions)
    */
    void    impl_reparent_nothrow( const ::com::sun::star::uno::WeakReference< ::com::sun::star::container::XNameAccess >& _rxContainer );

    /** retrieves the forms or reports contained, creates and initializes it, if necessary

        @raises DisposedException
            if the instance is already disposed
        @raises IllegalArgumentException
            if <arg>_eType</arg> is not ODatabaseModelImpl::E_FORM and not ODatabaseModelImpl::E_REPORT
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
            impl_getDocumentContainer_throw( ODatabaseModelImpl::ObjectType _eType );

    /** resets everything

        @precond
            m_pImpl is not <NULLL/>
    */
    void
            impl_reset_nothrow();

    /** imports the document from the given resource.
    */
    static void
            impl_import_nolck_throw(
                const ::comphelper::ComponentContext _rContext,
                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxTargetComponent,
                const ::comphelper::NamedValueCollection& _rResource
            );

    /** creates a storage for the given URL, truncating it if a file with this name already exists

        @throws Exception
            if creating the storage failed

        @return
            the newly created storage for the file at the given URL
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
            impl_createStorageFor_throw(
                const ::rtl::OUString& _rURL
            ) const;

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
    void    impl_setModified_nothrow( sal_Bool _bModified, DocumentGuard& _rGuard );

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

        @throws ::com::sun::star::uno::IllegalArgumentException
            if the given storage is <NULL/>.

        @throws ::com::sun::star::uno::RuntimeException
            when any of the used operations throws it

        @throws ::com::sun::star::io::IOException
            when any of the used operations throws it, or any other exception occurs which is no
            RuntimeException and no IOException
    */
    void    impl_storeToStorage_throw(
                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxTargetStorage,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rMediaDescriptor,
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
    sal_Bool    impl_attachResource(
                    const ::rtl::OUString& i_rLogicalDocumentURL,
                    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& i_rMediaDescriptor,
                    DocumentGuard& _rDocGuard
                );

    /** throws an IOException with the message as defined in the RID_STR_ERROR_WHILE_SAVING resource, wrapping
        the given caught non-IOException error
    */
    void        impl_throwIOExceptionCausedBySave_throw(
                    const ::com::sun::star::uno::Any& i_rError,
                    const ::rtl::OUString& i_rTargetURL
                ) const;
};

/** an extended version of the ModelMethodGuard, which also cares for the initialization state
    of the document
*/
class DocumentGuard : private ModelMethodGuard
{
public:
    enum MethodType
    {
        // a method which is to initialize the document
        InitMethod,
        // a default method
        DefaultMethod,
        // a method which is used (externally) during the initialization phase
        MethodUsedDuringInit,
        // a method which does not need initialization - use with care!
        MethodWithoutInit
    };

    /** constructs the guard

        @param _document
            the ODatabaseDocument instance

        @throws ::com::sun::star::lang::DisposedException
            If the given component is already disposed

        @throws ::com::sun::star::frame::DoubleInitializationException
            if _eType is InitMethod, and the given component is already initialized, or currently being initialized.

        @throws ::com::sun::star::lang::NotInitializedException
            if _eType is DefaultMethod, and the given component is not yet initialized; or if _eType
            is MethodUsedDuringInit, and the component is still uninitialized, and not in the initialization
            phase currently.
    */
    DocumentGuard( const ODatabaseDocument& _document, MethodType _eType = DefaultMethod )
        :ModelMethodGuard( _document )
        ,m_document( _document )
    {
        switch ( _eType )
        {
            case InitMethod:            m_document.checkNotInitialized();    break;
            case DefaultMethod:         m_document.checkInitialized();       break;
            case MethodUsedDuringInit:  m_document.checkNotUninitilized();   break;
            case MethodWithoutInit:                                         break;
        }
    }

    ~DocumentGuard()
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
#endif // _DBA_COREDATAACCESS_DATABASEDOCUMENT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

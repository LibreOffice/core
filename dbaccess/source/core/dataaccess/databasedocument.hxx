/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: databasedocument.hxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 14:33:18 $
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
#ifndef _DBA_COREDATAACCESS_DATABASEDOCUMENT_HXX_
#define _DBA_COREDATAACCESS_DATABASEDOCUMENT_HXX_

#include "ModelImpl.hxx"

/** === begin UNO includes === **/
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
#include <com/sun/star/frame/XModuleManager.hpp>
#include <cppuhelper/compbase10.hxx>
#include <cppuhelper/implbase3.hxx>

#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/embed/XTransactionListener.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>
#include <com/sun/star/script/XStorageBasedLibraryContainer.hpp>
#include <com/sun/star/script/provider/XScriptProviderSupplier.hpp>
/** === end UNO includes === **/

#if ! defined(INCLUDED_COMPHELPER_IMPLBASE_VAR_HXX_14)
#define INCLUDED_COMPHELPER_IMPLBASE_VAR_HXX_14
#define COMPHELPER_IMPLBASE_INTERFACE_NUMBER 14
#include <comphelper/implbase_var.hxx>
#endif

#include <boost/shared_ptr.hpp>

namespace comphelper {
    class NamedValueCollection;
}

//........................................................................
namespace dbaccess
{
//........................................................................

class ODatabaseContext;

typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > >   Controllers;

//============================================================
//= ODatabaseDocument
//============================================================
typedef ::comphelper::WeakComponentImplHelper14 <   ::com::sun::star::frame::XModel2
                                                ,   ::com::sun::star::util::XModifiable
                                                ,   ::com::sun::star::frame::XStorable
                                                ,   ::com::sun::star::document::XEventBroadcaster
                                                ,   ::com::sun::star::document::XEventListener
                                                ,   ::com::sun::star::view::XPrintable
                                                ,   ::com::sun::star::util::XCloseable
                                                ,   ::com::sun::star::lang::XServiceInfo
                                                ,   ::com::sun::star::sdb::XOfficeDatabaseDocument
                                                ,   ::com::sun::star::ui::XUIConfigurationManagerSupplier
                                                ,   ::com::sun::star::document::XStorageBasedDocument
                                                ,   ::com::sun::star::document::XEmbeddedScripts
                                                ,   ::com::sun::star::document::XScriptInvocationContext
                                                ,   ::com::sun::star::script::provider::XScriptProviderSupplier
                                                >   ODatabaseDocument_OfficeDocument;

typedef ::cppu::ImplHelper3<    ::com::sun::star::frame::XTitle
                            ,   ::com::sun::star::frame::XTitleChangeBroadcaster
                            ,   ::com::sun::star::frame::XUntitledNumbers
                            >   ODatabaseDocument_Title;

class ODatabaseDocument :public ModelDependentComponent             // ModelDependentComponent must be first!
                        ,public ODatabaseDocument_OfficeDocument
                        ,public ODatabaseDocument_Title
{
    DECLARE_STL_USTRINGACCESS_MAP(::com::sun::star::uno::Reference< ::com::sun::star::frame::XUntitledNumbers >,TNumberedController);
    ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager>            m_xUIConfigurationManager;

    ::cppu::OInterfaceContainerHelper                                                           m_aModifyListeners;
    ::cppu::OInterfaceContainerHelper                                                           m_aCloseListener;
    ::cppu::OInterfaceContainerHelper                                                           m_aDocEventListeners;
    ::cppu::OInterfaceContainerHelper                                                           m_aStorageListeners;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >                    m_xCurrentController;
    Controllers                                                                                 m_aControllers;

    ::com::sun::star::uno::WeakReference< ::com::sun::star::container::XNameAccess >            m_xForms;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::container::XNameAccess >            m_xReports;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::script::provider::XScriptProvider > m_xScriptProvider;

    /** stores the document to the given URL, rebases it to the respective new storage, if necessary, resets
        the modified flag, and notifies any listeners as required
    */
    void impl_storeAs_throw(
            const ::rtl::OUString& _rURL,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& _rArguments,
            const sal_Char* _pAsciiDocumentEventName,
            ModelMethodGuard& _rGuard
         );

    /** @short  such module manager is used to classify new opened documents. */
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModuleManager >         m_xModuleManager;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XTitle >                 m_xTitleHelper;
    TNumberedController                                                                 m_aNumberedControllers;

    /** notifies the global event broadcaster
        The method must be called without our mutex locked
    */
    void impl_notifyEvent_nolck_nothrow( const ::com::sun::star::document::EventObject& _rEvent );

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
    void writeStorage(
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
    // #i50905# / 2005-06-20 / frank.schonheit@sun.com

protected:
    virtual void SAL_CALL disposing();

    virtual ~ODatabaseDocument();

public:
    struct FactoryAccess { friend class ODatabaseModelImpl; private: FactoryAccess() { } };
    static ODatabaseDocument* createDatabaseDocument( const ::rtl::Reference<ODatabaseModelImpl>& _pImpl, FactoryAccess /*accessControl*/ )
    {
        return new ODatabaseDocument( _pImpl );
    }

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException);

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

// ::com::sun::star::frame::XModel2
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL getControllers(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableViewControllerNames(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > SAL_CALL createDefaultViewController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& Frame, ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& ComponentWindow ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > SAL_CALL createViewController( const ::rtl::OUString& ViewName, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Arguments, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& Frame, ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& ComponentWindow ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException) ;

// ::com::sun::star::frame::XStorable
    virtual sal_Bool SAL_CALL hasLocation(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual ::rtl::OUString SAL_CALL getLocation(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual sal_Bool SAL_CALL isReadonly(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL store(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL storeAsURL( const ::rtl::OUString& sURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArguments ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL storeToURL( const ::rtl::OUString& sURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArguments ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException) ;

// ::com::sun::star::util::XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

// ::com::sun::star::util::XModifiable
    virtual sal_Bool SAL_CALL isModified(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL setModified( sal_Bool bModified ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException) ;
// ::com::sun::star::document::XEventBroadcaster
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

// ::com::sun::star::document::XEventListener
    virtual void SAL_CALL notifyEvent( const ::com::sun::star::document::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);


// ::com::sun::star::view::XPrintable
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

private:
    /** closes the frames of all connected controllers

    @param _bDeliverOwnership
        determines if the ownership should be transfered to the component which
        possibly vetos the closing

    @raises ::com::sun::star::util::CloseVetoException
        if the closing was vetoed by any instance
    */
    void    impl_closeControllerFrames( sal_Bool _bDeliverOwnership );

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
    bool    impl_import_throw( const ::comphelper::NamedValueCollection& _rResource );

    /** creates a storage for the given URL, truncating it if a file with this name already exists

        @throws IOException
            if the storage could not be created, and the error causing this could not be handled
            by the document's interaction handler. The message of the IOException will note the failure,
            and also the message of the originally thrown exception.

        @return
            <TRUE/> if and only if the storage could be created. If <FALSE/> is returned, then the storage
            could not be created, but the error had successfully been handled by the document's
            interaction handler.
    */
    bool    impl_createStorageFor_throw(
                const ::rtl::OUString& _rURL,
                ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _out_rxStorage
            ) const;

    /** clears the guard before notifying.
    */
    void    impl_setModified_throw( sal_Bool _bModified, ModelMethodGuard& _rGuard );

    /** stores the document to the given storage

        Note that the document is actually not rebased to this storage, it just stores a copy of itself
        to the given target storage.

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
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rMediaDescriptor
            ) const;

    /// determines whether we should disable the scripting related interfaces
    bool    impl_shouldDisallowScripting_nolck_nothrow() const;
};

//........................................................................
}   // namespace dbaccess
//........................................................................
#endif // _DBA_COREDATAACCESS_DATABASEDOCUMENT_HXX_

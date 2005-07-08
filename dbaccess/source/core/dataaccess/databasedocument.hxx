/*************************************************************************
 *
 *  $RCSfile: databasedocument.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-08 10:36:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Ocke Janssen
 *
 *
 ************************************************************************/
#ifndef _DBA_COREDATAACCESS_DATABASEDOCUMENT_HXX_
#define _DBA_COREDATAACCESS_DATABASEDOCUMENT_HXX_

#ifndef _COM_SUN_STAR_UI_XUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTSUBSTORAGESUPPLIER_HPP_
#include <com/sun/star/document/XDocumentSubStorageSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XREPORTDOCUMENTSSUPPLIER_HPP_
#include <com/sun/star/sdb/XReportDocumentsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XFORMDOCUMENTSSUPPLIER_HPP_
#include <com/sun/star/sdb/XFormDocumentsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XPRINTABLE_HPP_
#include <com/sun/star/view/XPrintable.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE10_HXX_
#include <cppuhelper/compbase10.hxx>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTLISTENER_HPP_
#include <com/sun/star/document/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XOFFICEDATABASEDOCUMENT_HPP_
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#endif
#include <com/sun/star/embed/XTransactionListener.hpp>
#include <boost/shared_ptr.hpp>
#ifndef _DBA_COREDATAACCESS_MODELIMPL_HXX_
#include "ModelImpl.hxx"
#endif
#include <cppuhelper/compbase11.hxx>
//........................................................................
namespace dbaccess
{
//........................................................................

class ODatabaseContext;
//============================================================
//= ODatabaseDocument
//============================================================
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
    ODatabaseDocument_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

typedef ::cppu::WeakComponentImplHelper10<  ::com::sun::star::frame::XModel
                            ,   ::com::sun::star::util::XModifiable
                            ,   ::com::sun::star::frame::XStorable
                            ,   ::com::sun::star::document::XEventBroadcaster
                            ,   ::com::sun::star::document::XEventListener
                            ,   ::com::sun::star::view::XPrintable
                            ,   ::com::sun::star::util::XCloseable
                            ,   ::com::sun::star::lang::XServiceInfo
                            ,   ::com::sun::star::sdb::XOfficeDatabaseDocument
                            ,   ::com::sun::star::ui::XUIConfigurationManagerSupplier
                            //, ::com::sun::star::document::XStorageBasedDocument
                            >   ODatabaseDocument_OfficeDocument;

class ODatabaseDocument : public ::comphelper::OBaseMutex
                         ,public ODatabaseDocument_OfficeDocument
{
    ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager>    m_xUIConfigurationManager;
    ::com::sun::star::uno::Reference< ::com::sun::star::document::XEventListener >      m_xDocEventBroadcaster;

    ::cppu::OInterfaceContainerHelper                   m_aModifyListeners;
    ::cppu::OInterfaceContainerHelper                   m_aCloseListener;
    ::cppu::OInterfaceContainerHelper                   m_aDocEventListeners;
    ::rtl::Reference<ODatabaseModelImpl>                m_pImpl;
    sal_Bool                                            m_bCommitMasterStorage;

    void setMeAsParent(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xName);

    /** stores the model
        @param  sURL
            The URL
        @param  lArguments
            The media descriptor
        @param  _xStorageToSaveTo
            The storage which should be used for saving
    */
    void store(const ::rtl::OUString& sURL
             , const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& lArguments);

    /** notifies the global event broadcaster
        @param  _sEventName
            On of
            OnNew      => new document
            OnLoad      => load document
            OnUnload   => close document
            OnSaveDone   => "Save" ended
            OnSaveAsDone   => "SaveAs" ended
            OnModifyChanged   => modified/unmodified
    */
    void notifyEvent(const ::rtl::OUString& _sEventName);

    /// write a single XML stream into the package
    sal_Bool WriteThroughComponent(
        /// the component we export
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XComponent> & xComponent,
        const sal_Char* pStreamName,        /// the stream name
        const sal_Char* pServiceName,       /// service name of the component
        /// the argument (XInitialization)
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Any> & rArguments,
        /// output descriptor
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue> & rMediaDesc,
        sal_Bool bPlainStream
        , const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _xStorageToSaveTo);          /// neither compress nor encrypt

    /// write a single output stream
    /// (to be called either directly or by WriteThroughComponent(...))
    sal_Bool WriteThroughComponent(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::io::XOutputStream> & xOutputStream,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XComponent> & xComponent,
        const sal_Char* pServiceName,
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Any> & rArguments,
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue> & rMediaDesc);

    /** writes the content and settings
        @param  sURL
            The URL
        @param  lArguments
            The media descriptor
        @param  _xStorageToSaveTo
            The storage which should be used for saving
    */
    void writeStorage(const ::rtl::OUString& _sURL
                    , const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& lArguments
                    , const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _xStorageToSaveTo);


private:
    ODatabaseDocument(const ::rtl::Reference<ODatabaseModelImpl>& _pImpl);
    // Do NOT create those documents directly, always use ODatabaseModelImpl::getModel. Reason is that
    // ODatabaseDocument require clear ownership, and in turn lifetime synchronisation with the ModelImpl.
    // If you create a ODatabaseDocument directly, you might easily create a leak.
    // #i50905# / 2005-06-20 / frank.schonheit@sun.com

protected:
    virtual void SAL_CALL disposing();

    virtual ~ODatabaseDocument();

public:
    struct FactoryAccess { friend class ODatabaseModelImpl; private: FactoryAccess() { } };
    static ODatabaseDocument* createDatabaseDocument( const ::rtl::Reference<ODatabaseModelImpl>& _pImpl, FactoryAccess accessControl )
    {
        return new ODatabaseDocument( _pImpl );
    }

// ::com::sun::star::lang::XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XServiceInfo - static methods
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::rtl::OUString getImplementationName_Static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
        SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

// ::com::sun::star::frame::XModel
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
/*
    virtual void SAL_CALL loadFromStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aMediaDescriptor ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::frame::DoubleInitializationException, ::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL storeToStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aMediaDescriptor ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL switchToStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > SAL_CALL getDocumentStorage(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addStorageChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XStorageChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeStorageChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XStorageChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
*/
};

//........................................................................
}   // namespace dbaccess
//........................................................................
#endif // _DBA_COREDATAACCESS_DATABASEDOCUMENT_HXX_

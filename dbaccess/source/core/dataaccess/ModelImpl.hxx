/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ModelImpl.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:08:41 $
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

#ifndef _DBA_COREDATAACCESS_MODELIMPL_HXX_
#define _DBA_COREDATAACCESS_MODELIMPL_HXX_

#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XBOOKMARKSSUPPLIER_HPP_
#include <com/sun/star/sdb/XBookmarksSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERYDEFINITIONSSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueryDefinitionsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XISOLATEDCONNECTION_HPP_
#include <com/sun/star/sdbc/XIsolatedConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTLISTENER_HPP_
#include <com/sun/star/document/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif
#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _DBA_REGHELPER_HXX_
#include "dba_reghelper.hxx"
#endif
#ifndef _DBA_CORE_BOOKMARKCONTAINER_HXX_
#include "bookmarkcontainer.hxx"
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include <connectivity/CommonTools.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOMPLETEDCONNECTION_HPP_
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XREPORTDOCUMENTSSUPPLIER_HPP_
#include <com/sun/star/sdb/XReportDocumentsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XFORMDOCUMENTSSUPPLIER_HPP_
#include <com/sun/star/sdb/XFormDocumentsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif
#ifndef DBA_CONTENTHELPER_HXX
#include "ContentHelper.hxx"
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTSUBSTORAGESUPPLIER_HPP_
#include <com/sun/star/document/XDocumentSubStorageSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XSTORAGEBASEDDOCUMENT_HPP_
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XTRANSACTIONLISTENER_HPP_
#include <com/sun/star/embed/XTransactionListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XREFRESHABLE_HPP_
#include <com/sun/star/util/XRefreshable.hpp>
#endif
#include <memory>

//........................................................................
namespace dbaccess
{
//........................................................................

typedef ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XConnection > OWeakConnection;
typedef std::vector< OWeakConnection > OWeakConnectionArray;

class ODatabaseContext;
class OSharedConnectionManager;

//============================================================
//= SharedMutex
//============================================================
class SharedMutex
{
private:
    oslInterlockedCount m_refCount;
    ::osl::Mutex        m_aMutex;

public:
    SharedMutex();

    void SAL_CALL acquire();
    void SAL_CALL release();

    inline ::osl::Mutex&   getMutex() { return m_aMutex; }

private:
    ~SharedMutex();
};

//============================================================
//= ODatabaseModelImpl
//============================================================
DECLARE_STL_USTRINGACCESS_MAP(::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >,TStorages);

class ODatabaseContext;
class DocumentStorageAccess;
class OSharedConnectionManager;
class ODatabaseModelImpl : public ::rtl::IReference
{
private:
    ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XModel >     m_xModel;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XDataSource > m_xDataSource;

    DocumentStorageAccess*                                                      m_pStorageAccess;
    ::rtl::Reference< SharedMutex >                                             m_xMutex;

public:

    enum ObjectType
    {
        E_FORM   = 0,
        E_REPORT = 1,
        E_QUERY  = 2,
        E_TABLE  = 3
    };
    OWeakConnectionArray        m_aConnections;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xServiceFactory;

    ::std::vector<TContentPtr>      m_aContainer;
    TStorages                       m_aStorages;
    ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController> > m_aControllers;

    ::com::sun::star::uno::WeakReference< ::com::sun::star::container::XNameAccess >    m_xCommandDefinitions;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::container::XNameAccess >    m_xTableDefinitions;

    /// the URL the document was loaded from
    ::rtl::OUString                                     m_sFileURL;
    /** the URL which the document should report as it's URL

        This might differ from ->m_sFileURL in case the document was loaded
        as part of a crash recovery process. In this case, ->m_sFileURL points to
        the temporary file where the DB had been saved to, after a crash.
        ->m_sRealFileURL then is the URL of the document which actually had
        been recovered.
    */
    ::rtl::OUString                                     m_sRealFileURL;

// <properties>
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >
                                                        m_xNumberFormatsSupplier;
    ::rtl::OUString                                     m_sConnectURL;
    ::rtl::OUString                                     m_sName;        // transient, our creator has to tell us the title
    ::rtl::OUString                                     m_sUser;
    ::rtl::OUString                                     m_aPassword;    // transient !
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>
                                                        m_aLayoutInformation;
    sal_Int32                                           m_nLoginTimeout;
    sal_Bool                                            m_bReadOnly : 1;
    sal_Bool                                            m_bPasswordRequired : 1;
    sal_Bool                                            m_bSuppressVersionColumns : 1;
    sal_Bool                                            m_bModified : 1;
    sal_Bool                                            m_bDocumentReadOnly : 1;
    sal_Bool                                            m_bDisposingSubStorages;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                                                        m_aInfo;
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  m_aTableFilter;
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  m_aTableTypeFilter;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                                                        m_aArgs;
// </properties>


    // ::cppu::OInterfaceContainerHelper                    m_aStorageListeners;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener>               m_xSharedConnectionManager;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController>                 m_xCurrentController;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >                   m_xStorage;

    ODatabaseContext*                                   m_pDBContext;
    OSharedConnectionManager*                           m_pSharedConnectionManager;
    oslInterlockedCount                                 m_refCount;
    sal_uInt16                                          m_nControllerLockCount;
    sal_Bool                                            m_bOwnStorage;


    void lateInit();

    /** determines whether the database document has an embedded data storage
    */
    inline bool isEmbeddedDatabase() const { return ( m_sConnectURL.compareToAscii( "sdbc:embedded:", 14 ) == 0 ); }

    /** stores the embedded storage ("database")

        @param _bPreventRootCommits
            Normally, committing the embedded storage results in also commiting the root storage
            - this is an automatism for data safety reasons.
            If you pass <TRUE/> here, committing the root storage is prevented for this particular
            call.
        @return <TRUE/> if the storage could be commited, otherwise <FALSE/>
    */
    sal_Bool    commitEmbeddedStorage( sal_Bool _bPreventRootCommits = sal_False );

    /** commits all storages
    */
    void commitStorages()
            SAL_THROW(( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException ));


    /** dispose all frames for registered controllers
    */
    void disposeControllerFrames();

    /** notifies the global event broadcaster
        @param  _sEventName
            One of
            OnNew      => new document
            OnLoad      => load document
            OnUnload   => close document
            OnSaveDone   => "Save" ended
            OnSaveAsDone   => "SaveAs" ended
            OnModifyChanged   => modified/unmodified
    */
    void notifyEvent(const ::rtl::OUString& _sEventName);

    ODatabaseModelImpl(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory
        , const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel>& _xModel = ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel>()
        );
    virtual ~ODatabaseModelImpl();

    ODatabaseModelImpl(
        const ::rtl::OUString& _rRegistrationName
        ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory
        ,ODatabaseContext* _pDBContext = NULL
        );

// com::sun::star::beans::XPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    // XEventListener
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    void setModified( sal_Bool bModified );
// ::com::sun::star::sdb::XCompletedConnection
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL connectWithCompletion( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& handler ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::sdbc::XDataSource
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection( const ::rtl::OUString& user, const ::rtl::OUString& password ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL setLoginTimeout( sal_Int32 seconds ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    sal_Int32 SAL_CALL getLoginTimeout(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

// :: com::sun::star::sdb::XBookmarksSupplier
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getBookmarks(  ) throw (::com::sun::star::uno::RuntimeException);

// :: com::sun::star::sdb::XQueryDefinitionsSupplier
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getQueryDefinitions(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::sdbc::XIsolatedConnection
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getIsolatedConnection( const ::rtl::OUString& user, const ::rtl::OUString& password ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getIsolatedConnectionWithCompletion( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& handler ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    void dispose();

    ::rtl::OUString getURL();

// ::com::sun::star::util::XModifyBroadcaster
    void SAL_CALL addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

// ::com::sun::star::document::XEventListener
    void SAL_CALL notifyEvent( const ::com::sun::star::document::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

// XCloseable
    void SAL_CALL close( sal_Bool DeliverOwnership ) throw (::com::sun::star::util::CloseVetoException, ::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage> getStorage(const ::rtl::OUString& _sStorageName,sal_Int32 nMode = ::com::sun::star::embed::ElementModes::READWRITE);
// helper
    const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >&
            getNumberFormatsSupplier();


// other stuff
    void    flushTables();

    // disposes all elements in m_aStorages, and clears it
    void    disposeStorages() SAL_THROW(());

    /// creates a ->com::sun::star::embed::StorageFactory
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory >
            createStorageFactory() const;

    /// commits our storage
    void    commitRootStorage();

    /// commits a given storage if it's not readonly
    static  bool    commitStorageIfWriteable(
                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxStorage
            )
            SAL_THROW((
                ::com::sun::star::io::IOException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException
            ));

    /// commits a given storage if it's not readonly, ignoring (but asserting) all errors
    static  bool    commitStorageIfWriteable_ignoreErrors(
                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxStorage
            )
            SAL_THROW(());

    void clearConnections();

    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage> getStorage();

    /** returns the data source. If it doesn't exist it will be created
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource> getDataSource( bool _bCreateIfNecessary = true );

    /** returns the model, if there already exists one
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > getModel_noCreate();

    /** returns a new ->ODatabaseDocument

        @precond
            No ->ODatabaseDocument exists so far
        @seealso
            getModel_noCreate
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > createNewModel_deliverOwnership();

    struct ResetModelAccess { friend class ODatabaseDocument; private: ResetModelAccess() { } };

    /** resets the model to NULL

        Only to be called when the model is being disposed
    */
    void    modelIsDisposing( ResetModelAccess ) { m_xModel = ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XModel >(); }

    DocumentStorageAccess*
            getDocumentStorageAccess();

    ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentSubStorageSupplier >
            getDocumentSubStorageSupplier();

    inline ::rtl::Reference< SharedMutex > getSharedMutex() const { return m_xMutex; }

    /** @see osl_incrementInterlockedCount.
     */
    virtual oslInterlockedCount SAL_CALL acquire();

    /** @see osl_decrementInterlockedCount.
     */
    virtual oslInterlockedCount SAL_CALL release();
};

/** a small base class for UNO components whose functionality depends on a ODatabaseModelImpl
*/
class ModelDependentComponent
{
protected:
    ::rtl::Reference< ODatabaseModelImpl >  m_pImpl;
    ::rtl::Reference< SharedMutex >         m_xMutex;

protected:
    ModelDependentComponent( const ::rtl::Reference< ODatabaseModelImpl >& _model );
    virtual ~ModelDependentComponent();

    /** returns the component itself
    */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getThis() = 0;

    inline ::osl::Mutex& getMutex()
    {
        return m_xMutex->getMutex();
    }

public:
    struct GuardAccess { friend class ModelMethodGuard; private: GuardAccess() { } };

    /** returns the mutex used for thread safety

        @throws ::com::sun::star::lang::DisposedException
            if m_pImpl is <NULL/>. Usually, you will set this member in your derived
            component's <code>dispose</code> method to <NULL/>.
    */
    inline ::osl::Mutex& getMutex( GuardAccess )
    {
        return getMutex();
    }
    inline ::rtl::Reference< ODatabaseModelImpl > getImpl( GuardAccess )
    {
        return m_pImpl;
    }

    void checkDisposed()
    {
        if ( !m_pImpl.is() )
            throw ::com::sun::star::lang::DisposedException( ::rtl::OUString::createFromAscii( "Component is already disposed." ), getThis() );
    }
};

/** a guard for public methods of objects dependent on a ODatabaseModelImpl instance

    Just put this guard onto the stack at the beginning of your method. Don't bother yourself
    with a MutexGuard, checks for being disposed, and the like.
*/
class ModelMethodGuard  :public ::osl::ResettableMutexGuard
{
private:
    typedef ::osl::ResettableMutexGuard             BaseMutexGuard;

public:
    /** constructs the guard

        @param _component
            the component whose functionality depends on a ODatabaseModelImpl instance

        @throws ::com::sun::star::lang::DisposedException
            If the given component is already disposed
    */
    ModelMethodGuard( ModelDependentComponent& _component )
        :BaseMutexGuard( _component.getMutex( ModelDependentComponent::GuardAccess() ) )
    {
        _component.checkDisposed();
    }

    inline void clear()
    {
        BaseMutexGuard::clear();
    }

    inline void reset()
    {
        BaseMutexGuard::reset();
    }
};

//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // _DBA_COREDATAACCESS_DATALINK_HXX_


/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ModelImpl.hxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-29 08:51:05 $
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

#include "apitools.hxx"
#include "bookmarkcontainer.hxx"
#include "ContentHelper.hxx"
#include "dba_reghelper.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/document/XDocumentSubStorageSupplier.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactionListener.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/sdb/XBookmarksSupplier.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/sdb/XFormDocumentsSupplier.hpp>
#include <com/sun/star/sdb/XQueryDefinitionsSupplier.hpp>
#include <com/sun/star/sdb/XReportDocumentsSupplier.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdbc/XIsolatedConnection.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
/** === end UNO includes === **/

#include <comphelper/broadcasthelper.hxx>
#include <comphelper/proparrhlp.hxx>
#include <connectivity/CommonTools.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/weakref.hxx>
#include <sfx2/docmacromode.hxx>
#include <tools/string.hxx>
#include <vos/ref.hxx>

#include <memory>

//........................................................................
namespace dbaccess
{
//........................................................................

typedef ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XConnection > OWeakConnection;
typedef std::vector< OWeakConnection > OWeakConnectionArray;

struct AsciiPropertyValue
{
    // note: the canonic member order would be AsciiName / DefaultValue, but
    // this crashes on unxlngi6.pro, since there's a bug which somehow results in
    // getDefaultDataSourceSettings returning corrupted Any instances then.
    ::com::sun::star::uno::Any  DefaultValue;
    const sal_Char*             AsciiName;

    AsciiPropertyValue( const sal_Char* _pAsciiName, const ::com::sun::star::uno::Any& _rDefaultValue )
        :DefaultValue( _rDefaultValue )
        ,AsciiName( _pAsciiName )
    {
    }
};

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
class ODatabaseModelImpl    :public ::rtl::IReference
                            ,public ::sfx2::IMacroDocumentAccess
{
public:
    enum ObjectType
    {
        E_FORM   = 0,
        E_REPORT = 1,
        E_QUERY  = 2,
        E_TABLE  = 3
    };

private:
    ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XModel >     m_xModel;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XDataSource > m_xDataSource;

    DocumentStorageAccess*                                                      m_pStorageAccess;
    ::rtl::Reference< SharedMutex >                                             m_xMutex;
    ::std::vector< TContentPtr >                                                m_aContainer;   // one for each ObjectType
    TStorages                                                                   m_aStorages;
    ::sfx2::DocumentMacroMode                                                   m_aMacroMode;

public:
    OWeakConnectionArray                                                                m_aConnections;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xServiceFactory;

public:
    ::com::sun::star::uno::WeakReference< ::com::sun::star::container::XNameAccess >    m_xCommandDefinitions;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::container::XNameAccess >    m_xTableDefinitions;

    /// the URL the document was loaded from
    ::rtl::OUString                                     m_sFileURL;

    /// do we have any object (forms/reports) which contains macros?
    bool                                                m_bHasAnyObjectWithMacros;

    /** the URL which the document should report as it's URL

        This might differ from ->m_sFileURL in case the document was loaded
        as part of a crash recovery process. In this case, ->m_sFileURL points to
        the temporary file where the DB had been saved to, after a crash.
        ->m_sRealFileURL then is the URL of the document which actually had
        been recovered.
    */
    ::rtl::OUString                                     m_sRealFileURL;

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
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyAccess >
                                                        m_xSettings;
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  m_aTableFilter;
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  m_aTableTypeFilter;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                                                        m_aArgs;


    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener>               m_xSharedConnectionManager;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >                   m_xStorage;

    ODatabaseContext*                                   m_pDBContext;
    OSharedConnectionManager*                           m_pSharedConnectionManager;
    oslInterlockedCount                                 m_refCount;
    sal_uInt16                                          m_nControllerLockCount;
    sal_Bool                                            m_bOwnStorage;

    void reset();

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

    // XEventListener
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    void setModified( sal_Bool bModified );

    void dispose();

    ::rtl::OUString getURL();

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

    /// returns a all known data source settings, including their default values
    static const AsciiPropertyValue* getDefaultDataSourceSettings();

    /** retrieves the requested container of objects (forms/reports/tables/queries)
    */
    TContentPtr&    getObjectContainer( const ObjectType _eType );

    /** determines whether the given storage is the storage of our embedded database (named "database"), if any
    */
    bool            isDatabaseStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxStorage ) const;

    /** returns the name of the storage which is used to stored objects of the given type, if applicable
    */
    static ::rtl::OUString
                    getObjectContainerStorageName( const ObjectType _eType );

    /** determines whether the database document has any object (form/report) which contains macros

        In such a case, *all* objects in the document keep the macro capability, and the database document
        itself does *not* allow embedding macros.
    */
    bool    hasAnyObjectWithMacros() const { return m_bHasAnyObjectWithMacros; }

    /** checks our document's macro execution mode, using the interaction handler as supplied with our
        load arguments
    */
    void            checkMacrosOnLoading();

    /** adjusts our document's macro execution mode, without using any UI, assuming the user
        would reject execution of macros, if she would have been asked.

        If checkMacrosOnLoading has been called before (and thus the macro execution mode
        is already adjusted), then the current execution mode is simply returned.

        @return
            whether or not macro execution is allowed
    */
    bool            adjustMacroMode_AutoReject();

    /** resets our macro execute mode, so next time  the checkMacrosOnLoading is called, it will
        behave as if it has never been called before
    */
    void            resetMacroExecutionMode();

public:
    // IMacroDocumentAccess overridables
    virtual sal_Int16 getImposedMacroExecMode() const;
    virtual ::rtl::OUString getDocumentLocation() const;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > getLastCommitDocumentStorage();
    virtual bool documentStorageHasMacros() const;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::document::XEmbeddedScripts > getEmbeddedDocumentScripts() const;
    virtual sal_Int16 getScriptingSignatureState() const;
    virtual void showBrokenSignatureWarning( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& _rxInteraction ) const;

private:
    void    impl_construct_nothrow();
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


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

#ifndef _DBA_COREDATAACCESS_MODELIMPL_HXX_
#define _DBA_COREDATAACCESS_MODELIMPL_HXX_

#include "apitools.hxx"
#include "bookmarkcontainer.hxx"
#include "ContentHelper.hxx"
#include "core_resource.hxx"
#include "documentevents.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertyBag.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/document/XDocumentSubStorageSupplier.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactionListener.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/NotInitializedException.hpp>
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
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/frame/DoubleInitializationException.hpp>

#include <comphelper/broadcasthelper.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/sharedmutex.hxx>
#include <comphelper/solarmutex.hxx>
#include <connectivity/CommonTools.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/weakref.hxx>
#include <sfx2/docmacromode.hxx>
#include <sfx2/docstoragemodifylistener.hxx>
#include <unotools/sharedunocomponent.hxx>
#include <rtl/ref.hxx>

#include <memory>

namespace comphelper
{
    class NamedValueCollection;
}

namespace dbaccess
{

typedef ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XConnection > OWeakConnection;
typedef std::vector< OWeakConnection > OWeakConnectionArray;

struct AsciiPropertyValue
{
    // note: the canonic member order would be AsciiName / DefaultValue, but
    // this crashes on unxlngi6.pro, since there's a bug which somehow results in
    // getDefaultDataSourceSettings returning corrupted Any instances then.
    ::com::sun::star::uno::Any          DefaultValue;
    const sal_Char*                     AsciiName;
    const ::com::sun::star::uno::Type&  ValueType;

    AsciiPropertyValue()
        :DefaultValue( )
        ,AsciiName( NULL )
        ,ValueType( ::cppu::UnoType< ::cppu::UnoVoidType >::get() )
    {
    }

    AsciiPropertyValue( const sal_Char* _pAsciiName, const ::com::sun::star::uno::Any& _rDefaultValue )
        :DefaultValue( _rDefaultValue )
        ,AsciiName( _pAsciiName )
        ,ValueType( _rDefaultValue.getValueType() )
    {
        OSL_ENSURE( ValueType.getTypeClass() != ::com::sun::star::uno::TypeClass_VOID,
            "AsciiPropertyValue::AsciiPropertyValue: NULL values not allowed here, use the other CTOR for this!" );
    }
    AsciiPropertyValue( const sal_Char* _pAsciiName, const ::com::sun::star::uno::Type& _rValeType )
        :DefaultValue()
        ,AsciiName( _pAsciiName )
        ,ValueType( _rValeType )
    {
        OSL_ENSURE( ValueType.getTypeClass() != ::com::sun::star::uno::TypeClass_VOID,
            "AsciiPropertyValue::AsciiPropertyValue: VOID property values not supported!" );
    }
};

class ODatabaseContext;
class OSharedConnectionManager;

// VosMutexFacade
/** a class which provides an IMutex interface to an OSL-based mutex
*/
class VosMutexFacade : public comphelper::SolarMutex
{
public:
    /** beware of life time: the mutex you pass here must live as least as long
        as the VosMutexFacade instance lives.
    */
    VosMutexFacade( ::osl::Mutex& _rMutex );

    virtual void acquire();
    virtual void release();
    virtual bool tryToAcquire();

private:
    ::osl::Mutex&   m_rMutex;
};

// ODatabaseModelImpl
typedef ::utl::SharedUNOComponent< ::com::sun::star::embed::XStorage >  SharedStorage;

class ODatabaseContext;
class DocumentStorageAccess;
class OSharedConnectionManager;
class ODatabaseModelImpl    :public ::rtl::IReference
                            ,public ::sfx2::IMacroDocumentAccess
                            ,public ::sfx2::IModifiableDocument
{
public:
    enum ObjectType
    {
        E_FORM   = 0,
        E_REPORT = 1,
        E_QUERY  = 2,
        E_TABLE  = 3
    };

    enum EmbeddedMacros
    {
        // the database document (storage) itself contains macros
        eDocumentWideMacros,
        // there are sub document( storage)s containing macros
        eSubDocumentMacros,
        // there are no known macro( storage)s
        eNoMacros
    };

private:
    OModuleClient                                                               m_aModuleClient;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XModel >     m_xModel;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XDataSource > m_xDataSource;

    DocumentStorageAccess*                                                      m_pStorageAccess;
    ::comphelper::SharedMutex                                                   m_aMutex;
    VosMutexFacade                                                              m_aMutexFacade;
    ::std::vector< TContentPtr >                                                m_aContainer;   // one for each ObjectType
    ::sfx2::DocumentMacroMode                                                   m_aMacroMode;
    sal_Int16                                                                   m_nImposedMacroExecMode;

    ::com::sun::star::uno::Reference< ::com::sun::star::script::XStorageBasedLibraryContainer > m_xBasicLibraries;
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XStorageBasedLibraryContainer > m_xDialogLibraries;

    SharedStorage                                                               m_xDocumentStorage;
    ::rtl::Reference< ::sfx2::DocumentStorageModifyListener >                   m_pStorageModifyListener;
    ODatabaseContext*                                                           m_pDBContext;
    DocumentEventsData                                                          m_aDocumentEvents;

    ::comphelper::NamedValueCollection                                          m_aMediaDescriptor;
    /// the URL the document was loaded from
    OUString                                                             m_sDocFileLocation;

    oslInterlockedCount                                 m_refCount;

    /// do we have any object (forms/reports) which contains macros?
    ::boost::optional< EmbeddedMacros >                 m_aEmbeddedMacros;

    /// true if setting the Modified flag of the document is currently locked
    bool                                                m_bModificationLock;

    /// true if and only if a database document existed previously (though meanwhile disposed), and was already initialized
    bool                                                m_bDocumentInitialized;

    /** the URL which the document should report as it's URL

        This might differ from ->m_sDocFileLocation in case the document was loaded
        as part of a crash recovery process. In this case, ->m_sDocFileLocation points to
        the temporary file where the DB had been saved to, after a crash.
        ->m_sDocumentURL then is the URL of the document which actually had
        been recovered.
    */
    OUString                                     m_sDocumentURL;

public:
    OWeakConnectionArray                                                        m_aConnections;
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_aContext;

public:
    ::com::sun::star::uno::WeakReference< ::com::sun::star::container::XNameAccess >    m_xCommandDefinitions;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::container::XNameAccess >    m_xTableDefinitions;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >
                                                        m_xNumberFormatsSupplier;
    OUString                                     m_sConnectURL;
    OUString                                     m_sName;        // transient, our creator has to tell us the title
    OUString                                     m_sUser;
    OUString                                     m_aPassword;    // transient !
    OUString                                     m_sFailedPassword;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>
                                                        m_aLayoutInformation;
    sal_Int32                                           m_nLoginTimeout;
    sal_Bool                                            m_bReadOnly : 1;
    sal_Bool                                            m_bPasswordRequired : 1;
    sal_Bool                                            m_bSuppressVersionColumns : 1;
    sal_Bool                                            m_bModified : 1;
    sal_Bool                                            m_bDocumentReadOnly : 1;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyBag >
                                                        m_xSettings;
    ::com::sun::star::uno::Sequence< OUString >  m_aTableFilter;
    ::com::sun::star::uno::Sequence< OUString >  m_aTableTypeFilter;
    OSharedConnectionManager*                           m_pSharedConnectionManager;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >
                                                        m_xSharedConnectionManager;
    sal_uInt16                                          m_nControllerLockCount;

    void reset();

    /** determines whether the database document has an embedded data storage
    */
    inline bool isEmbeddedDatabase() const { return ( m_sConnectURL.startsWith("sdbc:embedded:") ); }

    /** stores the embedded storage ("database")

        @param _bPreventRootCommits
            Normally, committing the embedded storage results in also commiting the root storage
            - this is an automatism for data safety reasons.
            If you pass <TRUE/> here, committing the root storage is prevented for this particular
            call.
        @return <TRUE/> if the storage could be commited, otherwise <FALSE/>
    */
    bool        commitEmbeddedStorage( bool _bPreventRootCommits = false );

    /// commits all sub storages
    void commitStorages()
            SAL_THROW(( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException ));

    ODatabaseModelImpl(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
        ODatabaseContext& _pDBContext
    );
    virtual ~ODatabaseModelImpl();

    ODatabaseModelImpl(
        const OUString& _rRegistrationName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
        ODatabaseContext& _rDBContext
        );

    // XEventListener
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    void setModified( sal_Bool bModified );

    void dispose();

    inline OUString getURL() const               { return m_sDocumentURL;     }
    inline OUString getDocFileLocation() const   { return m_sDocFileLocation; }

    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
            getStorage(
                const ObjectType _eType, const sal_Int32 _nDesiredMode = ::com::sun::star::embed::ElementModes::READWRITE );

// helper
    const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >&
            getNumberFormatsSupplier();

    DocumentEventsData&
            getDocumentEvents() { return m_aDocumentEvents; }

    const ::comphelper::NamedValueCollection&
            getMediaDescriptor() const { return m_aMediaDescriptor; }

    void    setResource(
                const OUString& _rURL,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rArgs
            );
    void    setDocFileLocation(
                const OUString& i_rLoadedFrom
            );

    static ::comphelper::NamedValueCollection
            stripLoadArguments( const ::comphelper::NamedValueCollection& _rArguments );

// other stuff
    void    flushTables();

    // disposes all elements in m_aStorages, and clears it
    void    disposeStorages() SAL_THROW(());

    /// creates a ->com::sun::star::embed::StorageFactory
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory >
            createStorageFactory() const;

    /// commits our storage
    void    commitRootStorage();

    /// commits a given storage if it's not readonly, ignoring (but asserting) all errors
    static  bool    commitStorageIfWriteable_ignoreErrors(
                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxStorage
            )
            SAL_THROW(());

    void clearConnections();

            ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > getOrCreateRootStorage();
    inline  ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > getRootStorage() const { return m_xDocumentStorage.getTyped(); }
    inline  void resetRootStroage() { impl_switchToStorage_throw( NULL ); }

    /** returns the data source. If it doesn't exist it will be created
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource> getOrCreateDataSource();

    /** returns the model, if there already exists one
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > getModel_noCreate() const;

    /** returns a new ->ODatabaseDocument

        @param _bInitializeIfNecessary
            calls XLoadable::initNew on the newly created model, if necessary

        @precond
            No ->ODatabaseDocument exists so far

        @seealso
            getModel_noCreate
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > createNewModel_deliverOwnership( bool _bInitialize );

    struct ResetModelAccess { friend class ODatabaseDocument; private: ResetModelAccess() { } };

    /** resets the model to NULL

        Only to be called when the model is being disposed
    */
    void    modelIsDisposing( const bool _wasInitialized, ResetModelAccess );

    bool    hadInitializedDocument() const { return m_bDocumentInitialized; }

    DocumentStorageAccess*
            getDocumentStorageAccess();

    ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentSubStorageSupplier >
            getDocumentSubStorageSupplier();

    inline const ::comphelper::SharedMutex& getSharedMutex() const { return m_aMutex; }

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

    /** returns the name of the storage which is used to stored objects of the given type, if applicable
    */
    static OUString
                    getObjectContainerStorageName( const ObjectType _eType );

    /** revokes the data source registration at the database context
    */
    void            revokeDataSource() const;

    /** determines whether a given object storage contains macros
    */
    static bool     objectHasMacros(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxContainerStorage,
                        const OUString& _rPersistentName
                    );

    /** determines which kind of embedded macros are present in the document
    */
    EmbeddedMacros  determineEmbeddedMacros();

    /** checks our document's macro execution mode, using the interaction handler as supplied with our
        load arguments
    */
    bool            checkMacrosOnLoading();

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

    /** ensures that ->m_xBasicLibraries resp. m_xDialogLibraries exists

        @return
            the requested library container. Is never <NULL/>.

        @throws RuntimeException
            if something does wrong, which indicates a server error in the installation
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XStorageBasedLibraryContainer >
            getLibraryContainer( bool _bScript );

    /** lets our library containers store themself into the given root storage
    */
    void    storeLibraryContainersTo( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxToRootStorage );

    /** rebases the document to the given storage

        No actual committing, copying, saving, whatsoever happens. The storage is just remembered as the documents
        new storage, nothing more.

        @throws ::com::sun::star::lang::IllegalArgumentException
            if the given storage is <NULL/>
        @throws ::com::sun::star::lang::RuntimeException
            if any of the invoked operations does so
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
            switchToStorage(
                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxNewRootStorage
            );

    /** returns the macro mode imposed by an external instance, which passed it to attachResource
    */
    sal_Int16       getImposedMacroExecMode() const
    {
        return m_nImposedMacroExecMode;
    }
    void            setImposedMacroExecMode( const sal_Int16 _nMacroMode )
    {
        m_nImposedMacroExecMode = _nMacroMode;
    }

public:
    // IMacroDocumentAccess overridables
    virtual sal_Int16 getCurrentMacroExecMode() const;
    virtual sal_Bool setCurrentMacroExecMode( sal_uInt16 );
    virtual OUString getDocumentLocation() const;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > getZipStorageToSign();
    virtual sal_Bool documentStorageHasMacros() const;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::document::XEmbeddedScripts > getEmbeddedDocumentScripts() const;
    virtual sal_Int16 getScriptingSignatureState();
    virtual sal_Bool hasTrustedScriptingSignature( sal_Bool bAllowUIToAddAuthor );
    virtual void showBrokenSignatureWarning( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& _rxInteraction ) const;

    // IModifiableDocument
    virtual void storageIsModified();

    // don't use directly, use the ModifyLock class instead
    void    lockModify()              { m_bModificationLock = true; }
    void    unlockModify()            { m_bModificationLock = false; }
    bool    isModifyLocked() const    { return m_bModificationLock; }

private:
    void    impl_construct_nothrow();
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
            impl_switchToStorage_throw( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxNewRootStorage );

    /** switches to the given document URL, which denotes the logical URL of the document, not necessariy the
        URL where the doc was loaded/recovered from
    */
    void    impl_switchToLogicalURL(
                const OUString& i_rDocumentURL
            );

};

/** a small base class for UNO components whose functionality depends on a ODatabaseModelImpl
*/
class ModelDependentComponent
{
protected:
    ::rtl::Reference< ODatabaseModelImpl >  m_pImpl;
    mutable ::comphelper::SharedMutex       m_aMutex;

protected:
    ModelDependentComponent( const ::rtl::Reference< ODatabaseModelImpl >& _model );
    virtual ~ModelDependentComponent();

    /** returns the component itself
    */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getThis() const = 0;

    inline ::osl::Mutex& getMutex() const
    {
        return m_aMutex;
    }

public:
    struct GuardAccess { friend class ModelMethodGuard; private: GuardAccess() { } };

    /** returns the mutex used for thread safety

        @throws ::com::sun::star::lang::DisposedException
            if m_pImpl is <NULL/>. Usually, you will set this member in your derived
            component's <code>dispose</code> method to <NULL/>.
    */
    inline ::osl::Mutex& getMutex( GuardAccess ) const
    {
        return getMutex();
    }
    inline ::rtl::Reference< ODatabaseModelImpl > getImpl( GuardAccess ) const
    {
        return m_pImpl;
    }

    /// checks whether the component is already disposed, throws a DisposedException if so
    inline void checkDisposed() const
    {
        if ( !m_pImpl.is() )
            throw ::com::sun::star::lang::DisposedException( "Component is already disposed.", getThis() );
    }

    inline void lockModify()
    {
        m_pImpl->lockModify();
    }

    inline void unlockModify()
    {
        m_pImpl->unlockModify();
    }
};

class ModifyLock
{
public:
    ModifyLock( ModelDependentComponent& _component )
        :m_rComponent( _component )
    {
        m_rComponent.lockModify();
    }

    ~ModifyLock()
    {
        m_rComponent.unlockModify();
    }

private:
    ModelDependentComponent&    m_rComponent;
};

/** a guard for public methods of objects dependent on a ODatabaseModelImpl instance

    Just put this guard onto the stack at the beginning of your method. Don't bother yourself
    with a MutexGuard, checks for being disposed, and the like.
*/
class ModelMethodGuard : public ::osl::ResettableMutexGuard
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
    ModelMethodGuard( const ModelDependentComponent& _component )
        :BaseMutexGuard( _component.getMutex( ModelDependentComponent::GuardAccess() ) )
    {
        _component.checkDisposed();
    }

    ~ModelMethodGuard()
    {
    }
};

}   // namespace dbaccess

#endif // _DBA_COREDATAACCESS_DATALINK_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include "ContentHelper.hxx"
#include "documentevents.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertyBag.hpp>
#include <com/sun/star/document/XDocumentSubStorageSupplier.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/script/XStorageBasedLibraryContainer.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/weakref.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/docmacromode.hxx>
#include <sfx2/docstoragemodifylistener.hxx>
#include <unotools/sharedunocomponent.hxx>
#include <rtl/ref.hxx>

namespace comphelper
{
    class NamedValueCollection;
}

namespace dbaccess
{

typedef std::vector< css::uno::WeakReference< css::sdbc::XConnection > > OWeakConnectionArray;

struct AsciiPropertyValue
{
    // note: the canonic member order would be AsciiName / DefaultValue, but
    // this crashes on unxlngi6.pro, since there's a bug which somehow results in
    // getDefaultDataSourceSettings returning corrupted Any instances then.
    css::uno::Any          DefaultValue;
    const char*            AsciiName;
    const css::uno::Type&  ValueType;

    AsciiPropertyValue()
        :DefaultValue( )
        ,AsciiName( nullptr )
        ,ValueType( ::cppu::UnoType<void>::get() )
    {
    }

    AsciiPropertyValue( const char* _pAsciiName, const css::uno::Any& _rDefaultValue )
        :DefaultValue( _rDefaultValue )
        ,AsciiName( _pAsciiName )
        ,ValueType( _rDefaultValue.getValueType() )
    {
        OSL_ENSURE( ValueType.getTypeClass() != css::uno::TypeClass_VOID,
            "AsciiPropertyValue::AsciiPropertyValue: NULL values not allowed here, use the other CTOR for this!" );
    }
    AsciiPropertyValue( const char* _pAsciiName, const css::uno::Type& _rValeType )
        :DefaultValue()
        ,AsciiName( _pAsciiName )
        ,ValueType( _rValeType )
    {
        OSL_ENSURE( ValueType.getTypeClass() != css::uno::TypeClass_VOID,
            "AsciiPropertyValue::AsciiPropertyValue: VOID property values not supported!" );
    }
};

class ODatabaseContext;
class OSharedConnectionManager;

// ODatabaseModelImpl
typedef ::utl::SharedUNOComponent< css::embed::XStorage >  SharedStorage;

class ODatabaseContext;
class DocumentStorageAccess;
class OSharedConnectionManager;
class ODatabaseModelImpl    :public ::sfx2::IMacroDocumentAccess
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
    css::uno::WeakReference< css::frame::XModel >                     m_xModel;
    css::uno::WeakReference< css::sdbc::XDataSource >                 m_xDataSource;

    rtl::Reference<DocumentStorageAccess>                             m_pStorageAccess;
    std::vector< TContentPtr >                                      m_aContainer;   // one for each ObjectType
    ::sfx2::DocumentMacroMode                                         m_aMacroMode;
    sal_Int16                                                         m_nImposedMacroExecMode;

    css::uno::Reference< css::script::XStorageBasedLibraryContainer > m_xBasicLibraries;
    css::uno::Reference< css::script::XStorageBasedLibraryContainer > m_xDialogLibraries;

    SharedStorage                                                     m_xDocumentStorage;
    ::rtl::Reference< ::sfx2::DocumentStorageModifyListener >         m_pStorageModifyListener;
    ODatabaseContext&                                                 m_rDBContext;
    DocumentEventsData                                                m_aDocumentEvents;

    ::comphelper::NamedValueCollection                                m_aMediaDescriptor;
    /// the URL the document was loaded from
    OUString                                                          m_sDocFileLocation;

    oslInterlockedCount                                 m_refCount;

    /// do we have any object (forms/reports) which contains macros?
    ::std::optional< EmbeddedMacros >                 m_aEmbeddedMacros;

    /// true if setting the Modified flag of the document is currently locked
    bool                                                m_bModificationLock;

    /// true if and only if a database document existed previously (though meanwhile disposed), and was already initialized
    bool                                                m_bDocumentInitialized;

    /** the URL which the document should report as its URL

        This might differ from ->m_sDocFileLocation in case the document was loaded
        as part of a crash recovery process. In this case, ->m_sDocFileLocation points to
        the temporary file where the DB had been saved to, after a crash.
        ->m_sDocumentURL then is the URL of the document which actually had
        been recovered.
    */
    OUString                                     m_sDocumentURL;

    SignatureState m_nScriptingSignatureState;

public:
    OWeakConnectionArray                                                        m_aConnections;
    const css::uno::Reference< css::uno::XComponentContext >  m_aContext;

public:
    css::uno::WeakReference< css::container::XNameAccess >    m_xCommandDefinitions;
    css::uno::WeakReference< css::container::XNameAccess >    m_xTableDefinitions;

    css::uno::Reference< css::util::XNumberFormatsSupplier >
                                                              m_xNumberFormatsSupplier;
    OUString                                     m_sConnectURL;
    OUString                                     m_sName;        // transient, our creator has to tell us the title
    OUString                                     m_sUser;
    OUString                                     m_aPassword;    // transient !
    OUString                                     m_sFailedPassword;
    css::uno::Sequence< css::beans::PropertyValue>
                                                    m_aLayoutInformation;
    sal_Int32                                       m_nLoginTimeout;
    bool                                            m_bReadOnly : 1;
    bool                                            m_bPasswordRequired : 1;
    bool                                            m_bSuppressVersionColumns : 1;
    bool                                            m_bModified : 1;
    bool                                            m_bDocumentReadOnly : 1;
    bool                                            m_bMacroCallsSeenWhileLoading : 1;
    css::uno::Reference< css::beans::XPropertyBag >
                                                        m_xSettings;
    css::uno::Sequence< OUString >                      m_aTableFilter;
    css::uno::Sequence< OUString >                      m_aTableTypeFilter;
    OSharedConnectionManager*                           m_pSharedConnectionManager;
    css::uno::Reference< css::lang::XEventListener >
                                                        m_xSharedConnectionManager;
    css::uno::Reference<css::awt::XWindow>
                                                        m_xDialogParent;
    sal_uInt16                                          m_nControllerLockCount;

    void reset();

    /** determines whether the database document has an embedded data storage
    */
    bool isEmbeddedDatabase() const { return m_sConnectURL.startsWith("sdbc:embedded:"); }

    /** stores the embedded storage ("database")

        @param _bPreventRootCommits
            Normally, committing the embedded storage results in also committing the root storage
            - this is an automatism for data safety reasons.
            If you pass <TRUE/> here, committing the root storage is prevented for this particular
            call.
        @return <TRUE/> if the storage could be committed, otherwise <FALSE/>
    */
    bool        commitEmbeddedStorage( bool _bPreventRootCommits = false );

    /// commits all sub storages
    void commitStorages();

    ODatabaseModelImpl(
        const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
        ODatabaseContext& _pDBContext
    );
    virtual ~ODatabaseModelImpl();

    ODatabaseModelImpl(
        const OUString& _rRegistrationName,
        const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
        ODatabaseContext& _rDBContext
        );

    // XEventListener
    /// @throws css::uno::RuntimeException
    void disposing( const css::lang::EventObject& Source );

    void setModified( bool bModified );

    void dispose();

    const OUString& getURL() const               { return m_sDocumentURL;     }
    const OUString& getDocFileLocation() const   { return m_sDocFileLocation; }

    css::uno::Reference< css::embed::XStorage >
            getStorage( const ObjectType _eType );

// helper
    const css::uno::Reference< css::util::XNumberFormatsSupplier >&
            getNumberFormatsSupplier();

    DocumentEventsData&
            getDocumentEvents() { return m_aDocumentEvents; }

    const ::comphelper::NamedValueCollection&
            getMediaDescriptor() const { return m_aMediaDescriptor; }

    void    setResource(
                const OUString& _rURL,
                const css::uno::Sequence< css::beans::PropertyValue >& _rArgs
            );
    void    setDocFileLocation(
                const OUString& i_rLoadedFrom
            );

    static ::comphelper::NamedValueCollection
            stripLoadArguments( const ::comphelper::NamedValueCollection& _rArguments );

// other stuff

    // disposes all elements in m_aStorages, and clears it
    void    disposeStorages();

    /// creates a ->css::embed::StorageFactory
    css::uno::Reference< css::lang::XSingleServiceFactory >
            createStorageFactory() const;

    /// commits our storage
    void    commitRootStorage();

    /// commits a given storage if it's not readonly, ignoring (but asserting) all errors
    bool    commitStorageIfWriteable_ignoreErrors(
                const css::uno::Reference< css::embed::XStorage >& _rxStorage
            );

    void clearConnections();

    css::uno::Reference< css::embed::XStorage > const & getOrCreateRootStorage();
    css::uno::Reference< css::embed::XStorage > const & getRootStorage() const { return m_xDocumentStorage.getTyped(); }
    void resetRootStorage() { impl_switchToStorage_throw( nullptr ); }

    /** returns the data source. If it doesn't exist it will be created
    */
    css::uno::Reference< css::sdbc::XDataSource> getOrCreateDataSource();

    /** returns the model, if there already exists one
    */
    css::uno::Reference< css::frame::XModel > getModel_noCreate() const;

    /** returns a new ->ODatabaseDocument

        @precond
            No ->ODatabaseDocument exists so far

        @seealso
            getModel_noCreate
    */
    css::uno::Reference< css::frame::XModel > createNewModel_deliverOwnership();

    struct ResetModelAccess { friend class ODatabaseDocument; private: ResetModelAccess() { } };

    /** resets the model to NULL

        Only to be called when the model is being disposed
    */
    void    modelIsDisposing( const bool _wasInitialized, ResetModelAccess );

    bool    hadInitializedDocument() const { return m_bDocumentInitialized; }

    DocumentStorageAccess*
            getDocumentStorageAccess();

    css::uno::Reference< css::document::XDocumentSubStorageSupplier >
            getDocumentSubStorageSupplier();

    void acquire();

    void release();

    /// returns all known data source settings, including their default values
    static const AsciiPropertyValue* getDefaultDataSourceSettings();

    /** retrieves the requested container of objects (forms/reports/tables/queries)
    */
    TContentPtr&    getObjectContainer( const ObjectType _eType );

    /** returns the name of the storage which is used to stored objects of the given type, if applicable
    */
    static OUString
                    getObjectContainerStorageName( const ObjectType _eType );

    /** determines whether a given object storage contains macros
    */
    static bool     objectHasMacros(
                        const css::uno::Reference< css::embed::XStorage >& _rxContainerStorage,
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
    css::uno::Reference< css::script::XStorageBasedLibraryContainer >
            getLibraryContainer( bool _bScript );

    /** lets our library containers store themself into the given root storage
    */
    void    storeLibraryContainersTo( const css::uno::Reference< css::embed::XStorage >& _rxToRootStorage );

    /** rebases the document to the given storage

        No actual committing, copying, saving, whatsoever happens. The storage is just remembered as the documents
        new storage, nothing more.

        @throws css::lang::IllegalArgumentException
            if the given storage is <NULL/>
        @throws css::lang::RuntimeException
            if any of the invoked operations does so
    */
    css::uno::Reference< css::embed::XStorage >
            switchToStorage(
                const css::uno::Reference< css::embed::XStorage >& _rxNewRootStorage
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
    virtual sal_Int16 getCurrentMacroExecMode() const override;
    virtual void setCurrentMacroExecMode( sal_uInt16 ) override;
    virtual OUString getDocumentLocation() const override;
    virtual bool documentStorageHasMacros() const override;
    virtual bool macroCallsSeenWhileLoading() const override;
    virtual css::uno::Reference< css::document::XEmbeddedScripts > getEmbeddedDocumentScripts() const override;
    virtual SignatureState getScriptingSignatureState() override;
    virtual bool hasTrustedScriptingSignature( bool bAllowUIToAddAuthor ) override;

    // IModifiableDocument
    virtual void storageIsModified() override;

    // don't use directly, use the ModifyLock class instead
    void    lockModify()              { m_bModificationLock = true; }
    void    unlockModify()            { m_bModificationLock = false; }
    bool    isModifyLocked() const    { return m_bModificationLock; }

    weld::Window* GetFrameWeld();

private:
    void    impl_construct_nothrow();
    css::uno::Reference< css::embed::XStorage > const &
            impl_switchToStorage_throw( const css::uno::Reference< css::embed::XStorage >& _rxNewRootStorage );

    /** switches to the given document URL, which denotes the logical URL of the document, not necessarily the
        URL where the doc was loaded/recovered from
    */
    void    impl_switchToLogicalURL(
                const OUString& i_rDocumentURL
            );

};

/** a small base class for UNO components whose functionality depends on an ODatabaseModelImpl
*/
class ModelDependentComponent
{
protected:
    ::rtl::Reference< ODatabaseModelImpl >  m_pImpl;
    ::osl::Mutex m_aMutex; // only use this to init WeakComponentImplHelper

protected:
    explicit ModelDependentComponent( const ::rtl::Reference< ODatabaseModelImpl >& _model );
    virtual ~ModelDependentComponent();

    /** returns the component itself
    */
    virtual css::uno::Reference< css::uno::XInterface > getThis() const = 0;

    ::osl::Mutex& getMutex()
    {
        return m_aMutex;
    }

public:
    /// checks whether the component is already disposed, throws a DisposedException if so
    void checkDisposed() const
    {
        if ( !m_pImpl.is() )
            throw css::lang::DisposedException( "Component is already disposed.", getThis() );
    }

    void lockModify()
    {
        m_pImpl->lockModify();
    }

    void unlockModify()
    {
        m_pImpl->unlockModify();
    }
};

class ModifyLock
{
public:
    explicit ModifyLock( ModelDependentComponent& _component )
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

/** a guard for public methods of objects dependent on an ODatabaseModelImpl instance

    Just put this guard onto the stack at the beginning of your method. Don't bother yourself
    with a MutexGuard, checks for being disposed, and the like.
*/
class ModelMethodGuard
{
private:
    // to avoid deadlocks, lock SolarMutex
    SolarMutexResettableGuard m_SolarGuard;

public:
    /** constructs the guard

        @param _component
            the component whose functionality depends on an ODatabaseModelImpl instance

        @throws css::lang::DisposedException
            If the given component is already disposed
    */
    explicit ModelMethodGuard( const ModelDependentComponent& _component )
    {
        _component.checkDisposed();
    }

    void clear()
    {
        // note: this only releases *once* so may still be locked
        m_SolarGuard.clear();
    }

    void reset()
    {
        m_SolarGuard.reset();
    }
};

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

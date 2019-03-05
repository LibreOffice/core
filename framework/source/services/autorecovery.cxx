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

#include <config_features.h>

#include <loadenv/loadenv.hxx>

#include <loadenv/targethelper.hxx>
#include <pattern/frame.hxx>

#include <strings.hrc>
#include <classes/fwkresid.hxx>
#include <protocols.h>
#include <properties.h>

#include <helper/mischelper.hxx>

#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/theGlobalEventBroadcaster.hpp>
#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentRecovery.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/document/XDocumentEventListener.hpp>
#include <com/sun/star/document/XDocumentEventBroadcaster.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

#include <comphelper/configuration.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/mediadescriptor.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/sequence.hxx>
#include <vcl/evntpost.hxx>
#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>
#include <unotools/pathoptions.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/tempfile.hxx>
#include <ucbhelper/content.hxx>

#include <osl/time.h>
#include <vcl/weld.hxx>
#include <osl/file.hxx>
#include <sal/log.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/configmgr.hxx>
#include <svl/documentlockfile.hxx>
#include <tools/urlobj.hxx>
#include <officecfg/Office/Recovery.hxx>
#include <officecfg/Setup.hxx>

#include <general.h>
#include <stdtypes.h>

using namespace css::uno;
using namespace css::document;
using namespace css::frame;
using namespace css::lang;
using namespace framework;

namespace {

/** @short  hold all needed information for an asynchronous dispatch alive.

    @descr  Because some operations are forced to be executed asynchronously
            (e.g. requested by our CreashSave/Recovery dialog) ... we must make sure
            that these information won't be set as "normal" members of our AutoRecovery
            instance. Otherwise they can disturb our normal AutoSave-timer handling.
            e.g. it can be unclear then, which progress has to be used for storing documents...
 */
struct DispatchParams
{
public:
     DispatchParams();
     DispatchParams(const ::comphelper::SequenceAsHashMap&             lArgs ,
                    const css::uno::Reference< css::uno::XInterface >& xOwner);

     void forget();

public:

    /** @short  can be set from outside and is provided to
                our internal started operations.

        @descr  Normally we use the normal status indicator
                of the document windows to show a progress.
                But in case we are used by any special UI,
                it can provide its own status indicator object
                to us - so we use it instead of the normal one.
     */
    css::uno::Reference< css::task::XStatusIndicator > m_xProgress;

    /** TODO document me */
    OUString m_sSavePath;

    /** @short  define the current cache entry, which should be used for current
                backup or cleanUp operation ... which is may be done asynchronous */
    sal_Int32 m_nWorkingEntryID;

    /** @short  used for async operations, to prevent us from dying.

        @descr  If our dispatch() method was forced to start the
                internal operation asynchronous... we send an event
                to start and return immediately. But we must be sure that
                our instance live if the event callback reach us.
                So we hold an uno reference to ourself.
     */
    css::uno::Reference< css::uno::XInterface > m_xHoldRefForAsyncOpAlive;
};

/**
    implements the functionality of AutoSave and AutoRecovery
    of documents - including features of an EmergencySave in
    case a GPF occurs.
 */
typedef ::cppu::WeakComponentImplHelper<
            css::lang::XServiceInfo,
            css::frame::XDispatch,
            css::document::XDocumentEventListener, // => css.lang.XEventListener
            css::util::XChangesListener,      // => css.lang.XEventListener
            css::util::XModifyListener >      // => css.lang.XEventListener
         AutoRecovery_BASE;

class AutoRecovery  : private cppu::BaseMutex
                    , public  AutoRecovery_BASE
                    , public  ::cppu::OPropertySetHelper            // => XPropertySet, XFastPropertySet, XMultiPropertySet
{
public:

    /** These values are used as flags and represent the current state of a document.
        Every state of the life time of a document has to be recognized here.

        @attention  Do not change (means reorganize) already used numbers.
                    There exists some code inside SVX, which uses the same numbers,
                    to analyze such document states.
                    Not the best design ... but may be it will be changed later .-)
    */
    enum EDocStates
    {
        /* TEMP STATES */

        /// default state, if a document was new created or loaded
        E_UNKNOWN = 0,
        /// modified against the original file
        E_MODIFIED = 1,
        /// an active document can be postponed to be saved later.
        E_POSTPONED = 2,
        /// was already handled during one AutoSave/Recovery session.
        E_HANDLED = 4,
        /** an action was started (saving/loading) ... Can be interesting later if the process may be was interrupted by an exception. */
        E_TRY_SAVE = 8,
        E_TRY_LOAD_BACKUP = 16,
        E_TRY_LOAD_ORIGINAL = 32,

        /* FINAL STATES */

        /// the Auto/Emergency saved document is not usable any longer
        E_DAMAGED = 64,
        /// the Auto/Emergency saved document is not really up-to-date (some changes can be missing)
        E_INCOMPLETE = 128,
        /// the Auto/Emergency saved document was processed successfully
        E_SUCCEEDED = 512
    };

    /** @short  indicates the results of a FAILURE_SAFE operation

        @descr  We must know, which reason was the real one in case
                we couldn't copy a "failure document" to a user specified path.
                We must know, if we can forget our cache entry or not.
     */
    enum EFailureSafeResult
    {
        E_COPIED,
        E_ORIGINAL_FILE_MISSING,
        E_WRONG_TARGET_PATH
    };

    // TODO document me
    enum ETimerType
    {
        /** the timer should not be used next time */
        E_DONT_START_TIMER,
        /** timer (was/must be) started with normal AutoSaveTimeIntervall */
        E_NORMAL_AUTOSAVE_INTERVALL,
        /** timer must be started with special short time interval,
            to poll for an user idle period */
        E_POLL_FOR_USER_IDLE,
        /** timer must be started with a very(!) short time interval,
            to poll for the end of an user action, which does not allow saving documents in general */
        E_POLL_TILL_AUTOSAVE_IS_ALLOWED,
        /** don't start the timer - but calls the same action then before immediately again! */
        E_CALL_ME_BACK
    };

    // TODO document me ... flag field
    // Emergency_Save and Recovery overwrites Auto_Save!
    enum EJob
    {
        E_NO_JOB                    =   0,
        E_AUTO_SAVE                 =   1,
        E_EMERGENCY_SAVE            =   2,
        E_RECOVERY                  =   4,
        E_ENTRY_BACKUP              =   8,
        E_ENTRY_CLEANUP             =  16,
        E_PREPARE_EMERGENCY_SAVE    =  32,
        E_SESSION_SAVE              =  64,
        E_SESSION_RESTORE           = 128,
        E_DISABLE_AUTORECOVERY      = 256,
        E_SET_AUTOSAVE_STATE        = 512,
        E_SESSION_QUIET_QUIT        = 1024,
        E_USER_AUTO_SAVE            = 2048
    };

    /** @short  combine different information about one office document. */
    struct TDocumentInfo
    {
        public:

            TDocumentInfo()
                : DocumentState   (E_UNKNOWN)
                , UsedForSaving   (false)
                , ListenForModify (false)
                , IgnoreClosing   (false)
                , ID              (-1       )
            {}

            /** @short  points to the document. */
            css::uno::Reference< css::frame::XModel > Document;

            /** @short  knows, if the document is really modified since the last autosave,
                        or  was postponed, because it was an active one etcpp...

                @descr  Because we have no CHANGE TRACKING mechanism, based on office document,
                        we implements it by ourself. We listen for MODIFIED events
                        of each document and update this state flag here.

                        Further we postpone saving of active documents, e.g. if the user
                        works currently on it. We wait for an idle period then ...
             */
            sal_Int32 DocumentState;

            /** Because our applications not ready for concurrent save requests at the same time,
                we have suppress our own AutoSave for the moment, a document will be already saved
                by others.
             */
            bool UsedForSaving;

            /** For every user action, which modifies a document (e.g. key input) we get
                a notification as XModifyListener. That seems to be a "performance issue" .-)
                So we decided to listen for such modify events only for the time in which the document
                was stored as temp. file and was not modified again by the user.
            */
            bool ListenForModify;

            /** For SessionSave we must close all open documents by ourself.
                But because we are listen for documents events, we get some ...
                and deregister these documents from our configuration.
                That's why we mark these documents as "Closed by ourself" so we can
                ignore these "OnUnload" or disposing() events .-)
            */
            bool IgnoreClosing;

            /** TODO: document me */
            OUString OrgURL;
            OUString FactoryURL;
            OUString TemplateURL;

            OUString OldTempURL;
            OUString NewTempURL;

            OUString AppModule;      // e.g. com.sun.star.text.TextDocument - used to identify app module
            OUString FactoryService; // the service to create a document of the module
            OUString RealFilter;     // real filter, which was used at loading time
            OUString DefaultFilter;  // supports saving of the default format without losing data
            OUString Extension;      // file extension of the default filter
            OUString Title;          // can be used as "DisplayName" on every recovery UI!
            css::uno::Sequence< OUString >
                            ViewNames;      // names of the view which were active at emergency-save time

            sal_Int32 ID;
    };

    /** @short  used to know every currently open document. */
    typedef ::std::vector< TDocumentInfo > TDocumentList;

// member

private:

    /** @short  the global uno service manager.
        @descr  Must be used to create own needed services.
     */
    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    /** @short  points to the underlying recovery configuration.
        @descr  This instance does not cache - it calls directly the
                configuration API!
      */
    css::uno::Reference< css::container::XNameAccess > m_xRecoveryCFG;

    /** @short  proxy weak binding to forward Events to ourself without
                an ownership cycle
      */
    css::uno::Reference< css::util::XChangesListener > m_xRecoveryCFGListener;

    /** @short  points to the used configuration package or.openoffice.Setup
        @descr  This instance does not cache - it calls directly the
                configuration API!
      */
    css::uno::Reference< css::container::XNameAccess > m_xModuleCFG;

    /** @short  holds the global event broadcaster alive,
                where we listen for new created documents.
      */
    css::uno::Reference< css::frame::XGlobalEventBroadcaster > m_xNewDocBroadcaster;

    /** @short  proxy weak binding to forward Events to ourself without
                an ownership cycle
      */
    css::uno::Reference< css::document::XDocumentEventListener > m_xNewDocBroadcasterListener;

    /** @short  because we stop/restart listening sometimes, it's a good idea to know
                if we already registered as listener .-)
    */
    bool m_bListenForDocEvents;
    bool m_bListenForConfigChanges;

    /** @short  specify the time interval between two save actions.
        @descr  tools::Time is measured in [min].
     */
    sal_Int32 m_nAutoSaveTimeIntervall;

    /** @short  for an asynchronous operation we must know, if there is
                at least one running job (may be asynchronous!).
     */
    sal_Int32 m_eJob;

    /** @short  the timer, which is used to be informed about the next
                saving time ...
        @remark must lock SolarMutex to use
     */
    Timer m_aTimer;

    /** @short  make our dispatch asynchronous ... if required to do so! */
    std::unique_ptr<vcl::EventPoster> m_xAsyncDispatcher;

    /** @see    DispatchParams
     */
    DispatchParams m_aDispatchParams;

    /** @short  indicates, which time period is currently used by the
                internal timer.
     */
    ETimerType m_eTimerType;

    /** @short  this cache is used to hold all information about
                recovery/emergency save documents alive.
     */
    TDocumentList m_lDocCache;

    // TODO document me
    sal_Int32 m_nIdPool;

    /** @short  contains all status listener registered at this instance.
     */
    ListenerHash m_lListener;

    /** @descr  This member is used to prevent us against re-entrance problems.
                A mutex can't help to prevent us from concurrent using of members
                inside the same thread. But e.g. our internally used stl structures
                are not threadsafe ... and furthermore they can't be used at the same time
                for iteration and add/remove requests!
                So we have to detect such states and ... show a warning.
                May be there will be a better solution next time ... (copying the cache temp.
                bevor using).

                And further it's not possible to use a simple boolean value here.
                Because if more than one operation iterates over the same stl container ...
                (only to modify it's elements but don't add new or removing existing ones!)
                it should be possible doing so. But we must guarantee that the last operation reset
                this lock ... not the first one ! So we use a "ref count" mechanism for that."
     */
    sal_Int32 m_nDocCacheLock;

    /** @descr  These members are used to check the minimum disc space, which must exists
                to start the corresponding operation.
     */
    sal_Int32 m_nMinSpaceDocSave;
    sal_Int32 m_nMinSpaceConfigSave;

// interface

public:

    explicit AutoRecovery(const css::uno::Reference< css::uno::XComponentContext >& xContext);
    virtual ~AutoRecovery(                                                                   ) override;

    virtual OUString SAL_CALL getImplementationName() override
    {
        return OUString("com.sun.star.comp.framework.AutoRecovery");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return {"com.sun.star.frame.AutoRecovery"};
    }

    // XInterface
    virtual void SAL_CALL acquire() throw () override
        { OWeakObject::acquire(); }
    virtual void SAL_CALL release() throw () override
        { OWeakObject::release(); }
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& type) override;

    /// Initialization function after having acquire()'d.
    void initListeners();

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;

    // css.frame.XDispatch
    virtual void SAL_CALL dispatch(const css::util::URL&                                  aURL      ,
                                   const css::uno::Sequence< css::beans::PropertyValue >& lArguments) override;

    virtual void SAL_CALL addStatusListener(const css::uno::Reference< css::frame::XStatusListener >& xListener,
                                            const css::util::URL&                                     aURL     ) override;

    virtual void SAL_CALL removeStatusListener(const css::uno::Reference< css::frame::XStatusListener >& xListener,
                                               const css::util::URL&                                     aURL     ) override;

    // css.document.XDocumentEventListener
    /** @short  informs about created/opened documents.

        @descr  Every new opened/created document will be saved internally
                so it can be checked if its modified. This modified state
                is used later to decide, if it must be saved or not.

        @param  aEvent
                points to the new created/opened document.
     */
    virtual void SAL_CALL documentEventOccured(const css::document::DocumentEvent& aEvent) override;

    // css.util.XChangesListener
    virtual void SAL_CALL changesOccurred(const css::util::ChangesEvent& aEvent) override;

    // css.util.XModifyListener
    virtual void SAL_CALL modified(const css::lang::EventObject& aEvent) override;

    // css.lang.XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent) override;

protected:

    // OPropertySetHelper

    virtual sal_Bool SAL_CALL convertFastPropertyValue(      css::uno::Any& aConvertedValue,
                                                             css::uno::Any& aOldValue      ,
                                                             sal_Int32      nHandle        ,
                                                       const css::uno::Any& aValue         ) override;

    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(      sal_Int32      nHandle,
                                                           const css::uno::Any& aValue ) override;
    using cppu::OPropertySetHelper::getFastPropertyValue;
    virtual void SAL_CALL getFastPropertyValue(css::uno::Any& aValue ,
                                               sal_Int32      nHandle) const override;

    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() override;

private:
    virtual void SAL_CALL disposing() final override;

    /** @short  open the underlying configuration.

        @descr  This method must be called every time
                a configuration call is needed. Because
                method works together with the member
                m_xCFG, open it on demand and cache it
                afterwards.

        @throw  [com.sun.star.uno.RuntimeException]
                if config could not be opened successfully!

        @threadsafe
      */
    void implts_openConfig();

    /** @short  read the underlying configuration.

        @descr  After that we know the initial state - means:
                - if AutoSave was enabled by the user
                - which time interval has to be used
                - which recovery entries may already exists

        @throw  [com.sun.star.uno.RuntimeException]
                if config could not be opened or read successfully!

        @threadsafe
      */
    void implts_readConfig();

    /** @short  read the underlying configuration...

        @descr  ... but only keys related to the AutoSave mechanism.
                Means: State and Timer interval.
                E.g. the recovery list is not addressed here.

        @throw  [com.sun.star.uno.RuntimeException]
                if config could not be opened or read successfully!

        @threadsafe
      */
    void implts_readAutoSaveConfig();

    // TODO document me
    void implts_flushConfigItem(const AutoRecovery::TDocumentInfo& rInfo                ,
                                      bool                     bRemoveIt = false);

    // TODO document me
    void implts_startListening();
    void implts_startModifyListeningOnDoc(AutoRecovery::TDocumentInfo& rInfo);

    // TODO document me
    void implts_stopListening();
    void implts_stopModifyListeningOnDoc(AutoRecovery::TDocumentInfo& rInfo);

    /** @short  stops and may be(!) restarts the timer.

        @descr  A running timer is stopped every time here.
                But starting depends from the different internal
                timer variables (e.g. AutoSaveEnabled, AutoSaveTimeIntervall,
                TimerType etcpp.)

        @throw  [com.sun.star.uno.RuntimeException]
                if timer could not be stopped or started!

        @threadsafe
     */
    void implts_updateTimer();

    /** @short  stop the timer.

        @descr  Double calls will be ignored - means we do
                nothing here, if the timer is already disabled.

        @throw  [com.sun.star.uno.RuntimeException]
                if timer could not be stopped!

        @threadsafe
     */
    void implts_stopTimer();

    /** @short  callback of our internal timer.
     */
    DECL_LINK(implts_timerExpired, Timer*, void);

    /** @short  makes our dispatch() method asynchronous!
     */
    DECL_LINK(implts_asyncDispatch, LinkParamNone*, void);

    /** @short  implements the dispatch real. */
    void implts_dispatch(const DispatchParams& aParams);

    /** @short  validate new detected document and add it into the internal
                document list.

        @descr  This method should be called only, if its clear that a new
                document was opened/created during office runtime.
                This method checks, if it's a top level document (means not an embedded one).
                Only such top level documents can be recognized by this auto save mechanism.

        @param  xDocument
                the new document, which should be checked and registered.

        @threadsafe
     */
    void implts_registerDocument(const css::uno::Reference< css::frame::XModel >& xDocument);

    /** @short  remove the specified document from our internal document list.

        @param  xDocument
                the new document, which should be deregistered.

        @param  bStopListening
                sal_False: must be used in case this method is called within disposing() of the document,
                       where it make no sense to deregister our listener. The container dies...
                sal_True : must be used in case this method is used on "deregistration" of this document, where
                       we must deregister our listener .-)

        @threadsafe
     */
    void implts_deregisterDocument(const css::uno::Reference< css::frame::XModel >& xDocument                ,
                                         bool                                   bStopListening = true);

    // TODO document me
    void implts_markDocumentModifiedAgainstLastBackup(const css::uno::Reference< css::frame::XModel >& xDocument);

    // TODO document me
    void implts_updateModifiedState(const css::uno::Reference< css::frame::XModel >& xDocument);

    // TODO document me
    void implts_updateDocumentUsedForSavingState(const css::uno::Reference< css::frame::XModel >& xDocument      ,
                                                       bool                                   bSaveInProgress);

    // TODO document me
    void implts_markDocumentAsSaved(const css::uno::Reference< css::frame::XModel >& xDocument);

    /** @short  search a document inside given list.

        @param  rList
                reference to a vector, which can contain such
                document.

        @param  xDocument
                the document, which should be located inside the
                given list.

        @return [TDocumentList::iterator]
                which points to the located document.
                If document does not exists - its set to
                rList.end()!
     */
    static TDocumentList::iterator impl_searchDocument(      AutoRecovery::TDocumentList&               rList    ,
                                                       const css::uno::Reference< css::frame::XModel >& xDocument);

    /** TODO document me */
    void implts_changeAllDocVisibility(bool bVisible);
    void implts_prepareSessionShutdown();

    /** @short  save all current opened documents to a specific
                backup directory.

        @descr  Only really changed documents will be saved here.

                Further this method returns a suggestion, if and how it should
                be called again. May be some documents was not saved yet
                and must wait for an user idle period ...

        @param  bAllowUserIdleLoop
                Because this method is used for different uses cases, it must
                know, which actions are allowed or not.
                AUTO_SAVE =>
                             If a document is the most active one, saving it
                             will be postponed if there exists other unsaved
                             documents. This feature was implemented, because
                             we don't wish to disturb the user on it's work.
                             ... bAllowUserIdleLoop should be set to sal_True
                EMERGENCY_SAVE / SESSION_SAVE =>
                             Here we must finish our work ASAP! It's not allowed
                             to postpone any document.
                             ... bAllowUserIdleLoop must(!) be set to sal_False

        @param  pParams
                sometimes this method is required inside an external dispatch request.
                The it contains some special environment variables, which overwrites
                our normal environment.
                AutoSave              => pParams == 0
                SessionSave/CrashSave => pParams != 0

        @return A suggestion, how the timer (if it's not already disabled!)
                should be restarted to fulfill the requirements.

        @threadsafe
     */
    AutoRecovery::ETimerType implts_saveDocs(      bool        bAllowUserIdleLoop,
                                                   bool        bRemoveLockFiles,
                                             const DispatchParams* pParams        = nullptr);

    /** @short  save one of the current documents to a specific
                backup directory.

        @descr  It:
                - defines a new(!) unique temp file name
                - save the new temp file
                - remove the old temp file
                - patch the given info struct
                - and return errors.

                It does not:
                - patch the configuration.

                Note further: it patches the info struct
                more than ones. E.g. the new temp URL is set
                before the file is saved. And the old URL is removed
                only if removing of the old file was successfully.
                If this method returns without an exception - everything
                was OK. Otherwise the info struct can be analyzed to
                get more information, e.g. when the problem occurs.

        @param  sBackupPath
                the base path for saving such temp files.

        @param  rInfo
                points to an information structure, where
                e.g. the document, its modified state, the count
                of autosave-retries etcpp. exists.
                Its used also to return the new temp file name
                and some other state values!

        @threadsafe
      */
    void implts_saveOneDoc(const OUString&                                    sBackupPath      ,
                                 AutoRecovery::TDocumentInfo&                        rInfo            ,
                           const css::uno::Reference< css::task::XStatusIndicator >& xExternalProgress);

    /** @short  recovery all documents, which was saved during
                a crash before.

        @return A suggestion, how this method must be called back!

        @threadsafe
     */
    AutoRecovery::ETimerType implts_openDocs(const DispatchParams& aParams);

    // TODO document me
    void implts_openOneDoc(const OUString&               sURL       ,
                                 utl::MediaDescriptor& lDescriptor,
                                 AutoRecovery::TDocumentInfo&   rInfo      );

    // TODO document me
    void implts_generateNewTempURL(const OUString&               sBackupPath     ,
                                         utl::MediaDescriptor& rMediaDescriptor,
                                         AutoRecovery::TDocumentInfo&   rInfo           );

    /** @short  notifies all interested listener about the current state
                of the currently running operation.

        @descr  We support different set's of functions. AUTO_SAVE, EMERGENCY_SAVE,
                AUTO_RECOVERY, FAILURE_SAVE ... etcpp.
                Listener can register itself for any type of supported
                functionality ... but not for document URL's in special.

        @param  eJob
                is used to know, which set of listener we must notify.

        @param  aEvent
                describe the event more in detail.

        @threadsafe
      */
    void implts_informListener(      sal_Int32                      eJob  ,
                               const css::frame::FeatureStateEvent& aEvent);

    /** short   create a feature event struct, which can be send
                to any interested listener.

        @param  eJob
                describe the current running operation
                AUTOSAVE, EMERGENCYSAVE, RECOVERY

        @param  sEventType
                describe the type of this event
                START, STOP, UPDATE

        @param  pInfo
                if sOperation is an update, this parameter must be different from NULL
                and is used to send information regarding the current handled document.

        @return [css::frame::FeatureStateEvent]
                the event structure for sending.
     */
    static css::frame::FeatureStateEvent implst_createFeatureStateEvent(      sal_Int32                    eJob      ,
                                                                        const OUString&             sEventType,
                                                                        AutoRecovery::TDocumentInfo const * pInfo     );

    class ListenerInformer
    {
    private:
        AutoRecovery &m_rRecovery;
        sal_Int32 m_eJob;
        bool m_bStopped;
    public:
        ListenerInformer(AutoRecovery &rRecovery, sal_Int32 eJob)
            : m_rRecovery(rRecovery), m_eJob(eJob), m_bStopped(false)
        {
        }
        void start();
        void stop();
        ~ListenerInformer()
        {
            stop();
        }
    };

    // TODO document me
    void implts_resetHandleStates();

    // TODO document me
    void implts_specifyDefaultFilterAndExtension(AutoRecovery::TDocumentInfo& rInfo);

    // TODO document me
    void implts_specifyAppModuleAndFactory(AutoRecovery::TDocumentInfo& rInfo);

    /** retrieves the names of all active views of the given document
        @param rInfo
            the document info, whose <code>Document</code> member must not be <NULL/>.
    */
    void implts_collectActiveViewNames( AutoRecovery::TDocumentInfo& rInfo );

    /** updates the configuration so that for all documents, their current view/names are stored
    */
    void implts_persistAllActiveViewNames();

    // TODO document me
    void implts_prepareEmergencySave();

    // TODO document me
    void implts_doEmergencySave(const DispatchParams& aParams);

    // TODO document me
    void implts_doRecovery(const DispatchParams& aParams);

    // TODO document me
    void implts_doSessionSave(const DispatchParams& aParams);

    // TODO document me
    void implts_doSessionQuietQuit();

    // TODO document me
    void implts_doSessionRestore(const DispatchParams& aParams);

    // TODO document me
    void implts_backupWorkingEntry(const DispatchParams& aParams);

    // TODO document me
    void implts_cleanUpWorkingEntry(const DispatchParams& aParams);

    /** try to make sure that all changed config items (not our used
        config access only) will be flushed back to disc.

        E.g. our svtools::ConfigItems() has to be flushed explicitly .-(

        Note: This method can't fail. Flushing of config entries is an
              optional feature. Errors can be ignored.
     */
    void impl_flushALLConfigChanges();

    // TODO document me
    AutoRecovery::EFailureSafeResult implts_copyFile(const OUString& sSource    ,
                                                     const OUString& sTargetPath,
                                                     const OUString& sTargetName);

    /** @short  converts m_eJob into a job description, which
                can be used to inform an outside listener
                about the current running operation

        @param  eJob
                describe the current running operation
                AUTOSAVE, EMERGENCYSAVE, RECOVERY

        @return [string]
                a suitable job description of form:
                    vnd.sun.star.autorecovery:/do...
     */
    static OUString implst_getJobDescription(sal_Int32 eJob);

    /** @short  map the given URL to an internal int representation.

        @param  aURL
                the url, which describe the next starting or may be already running
                operation.

        @return [long]
                the internal int representation
                see enum EJob
     */
    static sal_Int32 implst_classifyJob(const css::util::URL& aURL);

    /// TODO
    void implts_verifyCacheAgainstDesktopDocumentList();

    /// TODO document me
    bool impl_enoughDiscSpace(sal_Int32 nRequiredSpace);

    /// TODO document me
    static void impl_showFullDiscError();

    /** @short  try to create/use a progress and set it inside the
                environment.

        @descr  The problem behind: There exists different use case of this method.
                a) An external progress is provided by our CrashSave or Recovery dialog.
                b) We must create our own progress e.g. for an AutoSave
                c) Sometimes our application filters don't use the progress
                   provided by the MediaDescriptor. They use the Frame every time to create
                   its own progress. So we implemented a HACK for these and now we set
                   an InterceptedProgress there for the time WE use this frame for loading/storing documents .-)

        @param  xNewFrame
                must be set only in case WE create a new frame (e.g. for loading documents
                on session restore or recovery). Then search for a frame using rInfo.Document must
                be suppressed and xFrame must be preferred instead .-)

        @param  rInfo
                used e.g. to find the frame corresponding to a document.
                This frame must be used to create a new progress e.g. for an AutoSave.

        @param  rArgs
                is used to set the new created progress as parameter on these set.
     */
    void impl_establishProgress(const AutoRecovery::TDocumentInfo&               rInfo    ,
                                      utl::MediaDescriptor&             rArgs    ,
                                const css::uno::Reference< css::frame::XFrame >& xNewFrame);

    void impl_forgetProgress(const AutoRecovery::TDocumentInfo&               rInfo    ,
                                   utl::MediaDescriptor&             rArgs    ,
                             const css::uno::Reference< css::frame::XFrame >& xNewFrame);

    /** try to remove the specified file from disc.

        Every URL supported by our UCB component can be used here.
        Further it doesn't matter if the file really exists or not.
        Because removing a non existent file will have the same
        result at the end... a non existing file .-)

        On the other side removing of files from disc is an optional
        feature. If we are not able doing so... it's not a real problem.
        Ok - users disc place will be smaller then... but we should produce
        a crash during crash save because we can't delete a temporary file only!

        @param  sURL
                the url of the file, which should be removed.
     */
    void st_impl_removeFile(const OUString& sURL);

    /** try to remove ".lock" file from disc if office will be terminated
        not using the official way .-)

        This method has to be handled "optional". So every error inside
        has to be ignored ! This method CAN NOT FAIL ... it can forget something only .-)
     */
    void st_impl_removeLockFile();
};

// recovery.xcu
static const char CFG_PACKAGE_RECOVERY[] = "org.openoffice.Office.Recovery/";

static const char CFG_ENTRY_AUTOSAVE_ENABLED[] = "AutoSave/Enabled";
static const char CFG_ENTRY_AUTOSAVE_TIMEINTERVALL[] = "AutoSave/TimeIntervall"; //sic!

static const char CFG_ENTRY_REALDEFAULTFILTER[] = "ooSetupFactoryActualFilter";

static const char CFG_ENTRY_PROP_TEMPURL[] = "TempURL";
static const char CFG_ENTRY_PROP_ORIGINALURL[] = "OriginalURL";
static const char CFG_ENTRY_PROP_TEMPLATEURL[] = "TemplateURL";
static const char CFG_ENTRY_PROP_FACTORYURL[] = "FactoryURL";
static const char CFG_ENTRY_PROP_MODULE[] = "Module";
static const char CFG_ENTRY_PROP_DOCUMENTSTATE[] = "DocumentState";
static const char CFG_ENTRY_PROP_FILTER[] = "Filter";
static const char CFG_ENTRY_PROP_TITLE[] = "Title";
static const char CFG_ENTRY_PROP_ID[] = "ID";
static const char CFG_ENTRY_PROP_VIEWNAMES[] = "ViewNames";

static const char FILTER_PROP_TYPE[] = "Type";
static const char TYPE_PROP_EXTENSIONS[] = "Extensions";

// setup.xcu
static const char CFG_ENTRY_PROP_EMPTYDOCUMENTURL[] = "ooSetupFactoryEmptyDocumentURL";
static const char CFG_ENTRY_PROP_FACTORYSERVICE[] = "ooSetupFactoryDocumentService";

static const char EVENT_ON_NEW[] = "OnNew";
static const char EVENT_ON_LOAD[] = "OnLoad";
static const char EVENT_ON_UNLOAD[] = "OnUnload";
static const char EVENT_ON_MODIFYCHANGED[] = "OnModifyChanged";
static const char EVENT_ON_SAVE[] = "OnSave";
static const char EVENT_ON_SAVEAS[] = "OnSaveAs";
static const char EVENT_ON_SAVETO[] = "OnCopyTo";
static const char EVENT_ON_SAVEDONE[] = "OnSaveDone";
static const char EVENT_ON_SAVEASDONE[] = "OnSaveAsDone";
static const char EVENT_ON_SAVETODONE[] = "OnCopyToDone";
static const char EVENT_ON_SAVEFAILED[] = "OnSaveFailed";
static const char EVENT_ON_SAVEASFAILED[] = "OnSaveAsFailed";
static const char EVENT_ON_SAVETOFAILED[] = "OnCopyToFailed";

static const char RECOVERY_ITEM_BASE_IDENTIFIER[] = "recovery_item_";

static const char CMD_PROTOCOL[] = "vnd.sun.star.autorecovery:";

static const char CMD_DO_AUTO_SAVE[] = "/doAutoSave";    // force AutoSave ignoring the AutoSave timer
static const char CMD_DO_PREPARE_EMERGENCY_SAVE[] = "/doPrepareEmergencySave";    // prepare the office for the following EmergencySave step (hide windows etcpp.)
static const char CMD_DO_EMERGENCY_SAVE[] = "/doEmergencySave";    // do EmergencySave on crash
static const char CMD_DO_RECOVERY[] = "/doAutoRecovery";    // recover all crashed documents
static const char CMD_DO_ENTRY_BACKUP[] = "/doEntryBackup";    // try to store a temp or original file to a user defined location
static const char CMD_DO_ENTRY_CLEANUP[] = "/doEntryCleanUp";    // remove the specified entry from the recovery cache
static const char CMD_DO_SESSION_SAVE[] = "/doSessionSave";    // save all open documents if e.g. a window manager closes an user session
static const char CMD_DO_SESSION_QUIET_QUIT[] = "/doSessionQuietQuit";    // let the current session be quietly closed ( the saving should be done using doSessionSave previously ) if e.g. a window manager closes an user session
static const char CMD_DO_SESSION_RESTORE[] = "/doSessionRestore";    // restore a saved user session from disc
static const char CMD_DO_DISABLE_RECOVERY[] = "/disableRecovery";    // disable recovery and auto save (!) temp. for this office session
static const char CMD_DO_SET_AUTOSAVE_STATE[] = "/setAutoSaveState";    // disable/enable auto save (not crash save) for this office session

static const char REFERRER_USER[] = "private:user";

static const char PROP_DISPATCH_ASYNCHRON[] = "DispatchAsynchron";
static const char PROP_PROGRESS[] = "StatusIndicator";
static const char PROP_SAVEPATH[] = "SavePath";
static const char PROP_ENTRY_ID[] = "EntryID";
static const char PROP_AUTOSAVE_STATE[] = "AutoSaveState";

static const char OPERATION_START[] = "start";
static const char OPERATION_STOP[] = "stop";
static const char OPERATION_UPDATE[] = "update";

static const sal_Int32       MIN_DISCSPACE_DOCSAVE                  =   5; // [MB]
static const sal_Int32       MIN_DISCSPACE_CONFIGSAVE               =   1; // [MB]
static const sal_Int32       RETRY_STORE_ON_FULL_DISC_FOREVER       = 300; // not forever ... but often enough .-)
static const sal_Int32       RETRY_STORE_ON_MIGHT_FULL_DISC_USEFULL =   3; // in case FULL DISC does not seem the real problem
static const sal_Int32       GIVE_UP_RETRY                          =   1; // in case FULL DISC does not seem the real problem

#define SAVE_IN_PROGRESS            true
#define SAVE_FINISHED               false

#define LOCK_FOR_CACHE_ADD_REMOVE   true
#define LOCK_FOR_CACHE_USE          false

#define MIN_TIME_FOR_USER_IDLE 10000 // 10s user idle

// enable the following defines in case you wish to simulate a full disc for debug purposes .-)

// this define throws every time a document is stored or a configuration change
// should be flushed an exception ... so the special error handler for this scenario is triggered
// #define TRIGGER_FULL_DISC_CHECK

// force "return sal_False" for the method impl_enoughDiscSpace().
// #define SIMULATE_FULL_DISC

class CacheLockGuard
{
    private:

        // holds the outside caller alive, so it's shared resources
        // are valid every time
        css::uno::Reference< css::uno::XInterface > m_xOwner;

        // mutex shared with outside caller!
        osl::Mutex& m_rSharedMutex;

        // this variable knows the state of the "cache lock"
        sal_Int32& m_rCacheLock;

        // to prevent increasing/decreasing of m_rCacheLock more than once
        // we must know if THIS guard has an actual lock set there!
        bool m_bLockedByThisGuard;

    public:

        CacheLockGuard(AutoRecovery* pOwner                      ,
                       osl::Mutex&   rMutex                      ,
                       sal_Int32&    rCacheLock                  ,
                       bool      bLockForAddRemoveVectorItems);
        ~CacheLockGuard();

        void lock(bool bLockForAddRemoveVectorItems);
        void unlock();
};

CacheLockGuard::CacheLockGuard(AutoRecovery* pOwner                      ,
                               osl::Mutex&   rMutex                      ,
                               sal_Int32&    rCacheLock                  ,
                               bool      bLockForAddRemoveVectorItems)
    : m_xOwner            (static_cast< css::frame::XDispatch* >(pOwner))
    , m_rSharedMutex      (rMutex                                       )
    , m_rCacheLock        (rCacheLock                                   )
    , m_bLockedByThisGuard(false                                    )
{
    lock(bLockForAddRemoveVectorItems);
}

CacheLockGuard::~CacheLockGuard()
{
    unlock();
    m_xOwner.clear();
}

void CacheLockGuard::lock(bool bLockForAddRemoveVectorItems)
{
    /* SAFE */
    osl::MutexGuard g(m_rSharedMutex);

    if (m_bLockedByThisGuard)
        return;

    // This cache lock is needed only to prevent us from removing/adding
    // items from/into the recovery cache ... during it's used at another code place
    // for iterating .-)

    // Modifying of item properties is allowed and sometimes needed!
    // So we should detect only the dangerous state of concurrent add/remove
    // requests and throw an exception then ... which can of course break the whole
    // operation. On the other side a crash reasoned by an invalid stl iterator
    // will have the same effect .-)

    if ( (m_rCacheLock > 0) && bLockForAddRemoveVectorItems )
    {
        OSL_FAIL("Re-entrance problem detected. Using of an stl structure in combination with iteration, adding, removing of elements etcpp.");
        throw css::uno::RuntimeException(
                "Re-entrance problem detected. Using of an stl structure in combination with iteration, adding, removing of elements etcpp.",
                m_xOwner);
    }

    ++m_rCacheLock;
    m_bLockedByThisGuard = true;
    /* SAFE */
}

void CacheLockGuard::unlock()
{
    /* SAFE */
    osl::MutexGuard g(m_rSharedMutex);

    if ( ! m_bLockedByThisGuard)
        return;

    --m_rCacheLock;
    m_bLockedByThisGuard = false;

    if (m_rCacheLock < 0)
    {
        OSL_FAIL("Wrong using of member m_nDocCacheLock detected. A ref counted value shouldn't reach values <0 .-)");
        throw css::uno::RuntimeException(
                "Wrong using of member m_nDocCacheLock detected. A ref counted value shouldn't reach values <0 .-)",
                m_xOwner);
    }
    /* SAFE */
}

DispatchParams::DispatchParams()
    : m_nWorkingEntryID(-1)
{
};

DispatchParams::DispatchParams(const ::comphelper::SequenceAsHashMap&             lArgs ,
                               const css::uno::Reference< css::uno::XInterface >& xOwner)
{
    m_nWorkingEntryID         = lArgs.getUnpackedValueOrDefault(PROP_ENTRY_ID, sal_Int32(-1)                                       );
    m_xProgress               = lArgs.getUnpackedValueOrDefault(PROP_PROGRESS, css::uno::Reference< css::task::XStatusIndicator >());
    m_sSavePath               = lArgs.getUnpackedValueOrDefault(PROP_SAVEPATH, OUString()                                   );
    m_xHoldRefForAsyncOpAlive = xOwner;
};

void DispatchParams::forget()
{
    m_sSavePath.clear();
    m_nWorkingEntryID = -1;
    m_xProgress.clear();
    m_xHoldRefForAsyncOpAlive.clear();
};

AutoRecovery::AutoRecovery(const css::uno::Reference< css::uno::XComponentContext >& xContext)
    : AutoRecovery_BASE         (m_aMutex)
    , ::cppu::OPropertySetHelper(cppu::WeakComponentImplHelperBase::rBHelper)
    , m_xContext                (xContext                                           )
    , m_bListenForDocEvents     (false                                          )
    , m_bListenForConfigChanges (false                                          )
    , m_nAutoSaveTimeIntervall  (0                                                  )
    , m_eJob                    (AutoRecovery::E_NO_JOB                             )
    , m_aTimer                  ( "Auto save timer" )
    , m_xAsyncDispatcher        (new vcl::EventPoster( LINK( this, AutoRecovery, implts_asyncDispatch )  ))
    , m_eTimerType              (E_DONT_START_TIMER                                 )
    , m_nIdPool                 (0                                                  )
    , m_lListener               (cppu::WeakComponentImplHelperBase::rBHelper.rMutex)
    , m_nDocCacheLock           (0                                                  )
    , m_nMinSpaceDocSave        (MIN_DISCSPACE_DOCSAVE                              )
    , m_nMinSpaceConfigSave     (MIN_DISCSPACE_CONFIGSAVE                           )
{
    m_aTimer.SetDebugName( "framework::AutoRecovery m_aTimer" );
}

void AutoRecovery::initListeners()
{
    // read configuration to know if autosave/recovery is on/off etcpp...
    implts_readConfig();

    implts_startListening();

    // establish callback for our internal used timer.
    // Note: Its only active, if the timer will be started ...
    SolarMutexGuard g;
    m_aTimer.SetInvokeHandler(LINK(this, AutoRecovery, implts_timerExpired));
}

AutoRecovery::~AutoRecovery()
{
    assert(!m_aTimer.IsActive());
}

void AutoRecovery::disposing()
{
    implts_stopTimer();
    SolarMutexGuard g;
    m_xAsyncDispatcher.reset();
}

Any SAL_CALL AutoRecovery::queryInterface( const css::uno::Type& _rType )
{
    Any aRet = AutoRecovery_BASE::queryInterface( _rType );
    if ( !aRet.hasValue() )
        aRet = OPropertySetHelper::queryInterface( _rType );
    return aRet;
}

Sequence< css::uno::Type > SAL_CALL AutoRecovery::getTypes(  )
{
    return comphelper::concatSequences(
        AutoRecovery_BASE::getTypes(),
        ::cppu::OPropertySetHelper::getTypes()
    );
}

void SAL_CALL AutoRecovery::dispatch(const css::util::URL&                                  aURL      ,
                                     const css::uno::Sequence< css::beans::PropertyValue >& lArguments)
{
    SAL_INFO("fwk.autorecovery", "AutoRecovery::dispatch() starts ..." << aURL.Complete);

    // valid request ?
    sal_Int32 eNewJob = AutoRecovery::implst_classifyJob(aURL);
    if (eNewJob == AutoRecovery::E_NO_JOB)
        return;

    bool bAsync;
    DispatchParams aParams;
    /* SAFE */ {
    osl::ClearableMutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);

    // still running operation ... ignoring AUTO_SAVE.
    // All other requests has higher prio!
    if (
        ( m_eJob                               != AutoRecovery::E_NO_JOB   ) &&
        ((m_eJob & AutoRecovery::E_AUTO_SAVE ) != AutoRecovery::E_AUTO_SAVE)
       )
    {
        SAL_INFO("fwk.autorecovery", "AutoRecovery::dispatch(): There is already an asynchronous dispatch() running. New request will be ignored!");
        return;
    }

    ::comphelper::SequenceAsHashMap lArgs(lArguments);

    // check if somewhere wish to disable recovery temp. for this office session
    // This can be done immediately... must not been done asynchronous.
    if ((eNewJob & AutoRecovery::E_DISABLE_AUTORECOVERY) == AutoRecovery::E_DISABLE_AUTORECOVERY)
    {
        // it's important to set a flag internally, so AutoRecovery will be suppressed - even if it's requested.
        m_eJob |= eNewJob;
        implts_stopTimer();
        implts_stopListening();
        return;
    }

    // disable/enable AutoSave for this office session only
    // independent from the configuration entry.
    if ((eNewJob & AutoRecovery::E_SET_AUTOSAVE_STATE) == AutoRecovery::E_SET_AUTOSAVE_STATE)
    {
        bool bOn = lArgs.getUnpackedValueOrDefault(PROP_AUTOSAVE_STATE, true);
        if (bOn)
        {
            // don't enable AutoSave hardly !
            // reload configuration to know the current state.
            implts_readAutoSaveConfig();
            g.clear();
            implts_updateTimer();
            // can it happen that might be the listener was stopped? .-)
            // make sure it runs always... even if AutoSave itself was disabled temporarily.
            implts_startListening();
        }
        else
        {
            implts_stopTimer();
            m_eJob       &= ~AutoRecovery::E_AUTO_SAVE;
            m_eTimerType  =  AutoRecovery::E_DONT_START_TIMER;
        }
        return;
    }

    m_eJob |= eNewJob;

    bAsync = lArgs.getUnpackedValueOrDefault(PROP_DISPATCH_ASYNCHRON, false);
    aParams = DispatchParams(lArgs, static_cast< css::frame::XDispatch* >(this));

    // Hold this instance alive till the asynchronous operation will be finished.
    if (bAsync)
        m_aDispatchParams = aParams;

    } /* SAFE */

    if (bAsync)
        m_xAsyncDispatcher->Post();
    else
        implts_dispatch(aParams);
}

void AutoRecovery::ListenerInformer::start()
{
    m_rRecovery.implts_informListener(m_eJob,
        AutoRecovery::implst_createFeatureStateEvent(m_eJob, OPERATION_START, nullptr));
}

void AutoRecovery::ListenerInformer::stop()
{
    if (m_bStopped)
        return;
    m_rRecovery.implts_informListener(m_eJob,
        AutoRecovery::implst_createFeatureStateEvent(m_eJob, OPERATION_STOP, nullptr));
    m_bStopped = true;
}

void AutoRecovery::implts_dispatch(const DispatchParams& aParams)
{
    sal_Int32 eJob;
    /* SAFE */ {
    osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);
    eJob = m_eJob;
    } /* SAFE */

    // in case a new dispatch overwrites a may ba active AutoSave session
    // we must restore this session later. see below ...
    bool bWasAutoSaveActive = ((eJob & AutoRecovery::E_AUTO_SAVE) == AutoRecovery::E_AUTO_SAVE);
    bool bWasUserAutoSaveActive =
        ((eJob & AutoRecovery::E_USER_AUTO_SAVE) == AutoRecovery::E_USER_AUTO_SAVE);

    // On the other side it make no sense to reactivate the AutoSave operation
    // if the new dispatch indicates a final decision ...
    // E.g. an EmergencySave/SessionSave indicates the end of life of the current office session.
    // It make no sense to reactivate an AutoSave then.
    // But a Recovery or SessionRestore should reactivate a may be already active AutoSave.
    bool bAllowAutoSaveReactivation = true;

    implts_stopTimer();
    implts_stopListening();

    ListenerInformer aListenerInformer(*this, eJob);
    aListenerInformer.start();

    try
    {
        //  Auto save is called from our internal timer ... not via dispatch() API !
        // else
        if (
            ((eJob & AutoRecovery::E_PREPARE_EMERGENCY_SAVE) == AutoRecovery::E_PREPARE_EMERGENCY_SAVE) &&
            ((eJob & AutoRecovery::E_DISABLE_AUTORECOVERY      ) != AutoRecovery::E_DISABLE_AUTORECOVERY      )
           )
        {
            SAL_INFO("fwk.autorecovery", "... prepare emergency save ...");
            bAllowAutoSaveReactivation = false;
            implts_prepareEmergencySave();
        }
        else
        if (
            ((eJob & AutoRecovery::E_EMERGENCY_SAVE  ) == AutoRecovery::E_EMERGENCY_SAVE  ) &&
            ((eJob & AutoRecovery::E_DISABLE_AUTORECOVERY) != AutoRecovery::E_DISABLE_AUTORECOVERY)
           )
        {
            SAL_INFO("fwk.autorecovery", "... do emergency save ...");
            bAllowAutoSaveReactivation = false;
            implts_doEmergencySave(aParams);
        }
        else
        if (
            ((eJob & AutoRecovery::E_RECOVERY        ) == AutoRecovery::E_RECOVERY        ) &&
            ((eJob & AutoRecovery::E_DISABLE_AUTORECOVERY) != AutoRecovery::E_DISABLE_AUTORECOVERY)
           )
        {
            SAL_INFO("fwk.autorecovery", "... do recovery ...");
            implts_doRecovery(aParams);
        }
        else
        if (
            ((eJob & AutoRecovery::E_SESSION_SAVE    ) == AutoRecovery::E_SESSION_SAVE    ) &&
            ((eJob & AutoRecovery::E_DISABLE_AUTORECOVERY) != AutoRecovery::E_DISABLE_AUTORECOVERY)
            )
        {
            SAL_INFO("fwk.autorecovery", "... do session save ...");
            bAllowAutoSaveReactivation = false;
            implts_doSessionSave(aParams);
        }
        else
        if (
            ((eJob & AutoRecovery::E_SESSION_QUIET_QUIT    ) == AutoRecovery::E_SESSION_QUIET_QUIT ) &&
            ((eJob & AutoRecovery::E_DISABLE_AUTORECOVERY) != AutoRecovery::E_DISABLE_AUTORECOVERY)
            )
        {
            SAL_INFO("fwk.autorecovery", "... do session quiet quit ...");
            bAllowAutoSaveReactivation = false;
            implts_doSessionQuietQuit();
        }
        else
        if (
            ((eJob & AutoRecovery::E_SESSION_RESTORE ) == AutoRecovery::E_SESSION_RESTORE ) &&
            ((eJob & AutoRecovery::E_DISABLE_AUTORECOVERY) != AutoRecovery::E_DISABLE_AUTORECOVERY)
            )
        {
            SAL_INFO("fwk.autorecovery", "... do session restore ...");
            implts_doSessionRestore(aParams);
        }
        else
        if (
            ((eJob & AutoRecovery::E_ENTRY_BACKUP    ) == AutoRecovery::E_ENTRY_BACKUP    ) &&
            ((eJob & AutoRecovery::E_DISABLE_AUTORECOVERY) != AutoRecovery::E_DISABLE_AUTORECOVERY)
            )
            implts_backupWorkingEntry(aParams);
        else
        if (
            ((eJob & AutoRecovery::E_ENTRY_CLEANUP   ) == AutoRecovery::E_ENTRY_CLEANUP   ) &&
            ((eJob & AutoRecovery::E_DISABLE_AUTORECOVERY) != AutoRecovery::E_DISABLE_AUTORECOVERY)
            )
            implts_cleanUpWorkingEntry(aParams);
    }
    catch(const css::uno::RuntimeException&)
    {
        throw;
    }
    catch(const css::uno::Exception&)
    {
        // TODO better error handling
    }

    aListenerInformer.stop();

    /* SAFE */ {
    osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);
    m_eJob = E_NO_JOB;
    if ( bAllowAutoSaveReactivation && bWasAutoSaveActive )
    {
        m_eJob |= AutoRecovery::E_AUTO_SAVE;

        if (bWasUserAutoSaveActive)
        {
            m_eJob |= AutoRecovery::E_USER_AUTO_SAVE;
        }
    }

    } /* SAFE */

    // depends on bAllowAutoSaveReactivation implicitly by looking on m_eJob=E_AUTO_SAVE! see before ...
    implts_updateTimer();

    if (bAllowAutoSaveReactivation)
        implts_startListening();
}

void SAL_CALL AutoRecovery::addStatusListener(const css::uno::Reference< css::frame::XStatusListener >& xListener,
                                              const css::util::URL&                                     aURL     )
{
    if (!xListener.is())
        throw css::uno::RuntimeException("Invalid listener reference.", static_cast< css::frame::XDispatch* >(this));
    // container is threadsafe by using a shared mutex!
    m_lListener.addInterface(aURL.Complete, xListener);

    // REENTRANT !? -> --------------------------------
    CacheLockGuard aCacheLock(this, cppu::WeakComponentImplHelperBase::rBHelper.rMutex, m_nDocCacheLock, LOCK_FOR_CACHE_USE);

    /* SAFE */ {
    osl::ResettableMutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);

    for (auto const& elem : m_lDocCache)
    {
        css::frame::FeatureStateEvent aEvent = AutoRecovery::implst_createFeatureStateEvent(m_eJob, OPERATION_UPDATE, &elem);

        // } /* SAFE */
        g.clear();
        xListener->statusChanged(aEvent);
        g.reset();
        // /* SAFE */ {
    }

    } /* SAFE */
}

void SAL_CALL AutoRecovery::removeStatusListener(const css::uno::Reference< css::frame::XStatusListener >& xListener,
                                                 const css::util::URL&                                     aURL     )
{
    if (!xListener.is())
        throw css::uno::RuntimeException("Invalid listener reference.", static_cast< css::frame::XDispatch* >(this));
    // container is threadsafe by using a shared mutex!
    m_lListener.removeInterface(aURL.Complete, xListener);
}

void SAL_CALL AutoRecovery::documentEventOccured(const css::document::DocumentEvent& aEvent)
{
    css::uno::Reference< css::frame::XModel > xDocument(aEvent.Source, css::uno::UNO_QUERY);

    // new document => put it into the internal list
    if (
        (aEvent.EventName == EVENT_ON_NEW) ||
        (aEvent.EventName == EVENT_ON_LOAD)
       )
    {
        implts_registerDocument(xDocument);
    }
    // document modified => set its modify state new (means modified against the original file!)
    else if ( aEvent.EventName == EVENT_ON_MODIFYCHANGED )
    {
        implts_updateModifiedState(xDocument);
    }
    /* at least one document starts saving process =>
       Our application code is not ready for multiple save requests
       at the same time. So we have to suppress our AutoSave feature
       for the moment, till this other save requests will be finished.
     */
    else if (
        (aEvent.EventName == EVENT_ON_SAVE) ||
        (aEvent.EventName == EVENT_ON_SAVEAS) ||
        (aEvent.EventName == EVENT_ON_SAVETO)
       )
    {
        implts_updateDocumentUsedForSavingState(xDocument, SAVE_IN_PROGRESS);
    }
    // document saved => remove tmp. files - but hold config entries alive!
    else if (
        (aEvent.EventName == EVENT_ON_SAVEDONE) ||
        (aEvent.EventName == EVENT_ON_SAVEASDONE)
       )
    {
        SolarMutexGuard g;
        implts_markDocumentAsSaved(xDocument);
        implts_updateDocumentUsedForSavingState(xDocument, SAVE_FINISHED);
    }
    /* document saved as copy => mark it as "non used by concurrent save operation".
       so we can try to create a backup copy if next time AutoSave is started too.
       Don't remove temp. files or change the modified state of the document!
       It was not really saved to the original file...
    */
    else if ( aEvent.EventName == EVENT_ON_SAVETODONE )
    {
        implts_updateDocumentUsedForSavingState(xDocument, SAVE_FINISHED);
    }
    // If saving of a document failed by an error ... we have to save this document
    // by ourself next time AutoSave or EmergencySave is triggered.
    // But we can reset the state "used for other save requests". Otherwise
    // these documents will never be saved!
    else if (
        (aEvent.EventName == EVENT_ON_SAVEFAILED) ||
        (aEvent.EventName == EVENT_ON_SAVEASFAILED) ||
        (aEvent.EventName == EVENT_ON_SAVETOFAILED)
       )
    {
        implts_updateDocumentUsedForSavingState(xDocument, SAVE_FINISHED);
    }
    // document closed => remove temp. files and configuration entries
    else if ( aEvent.EventName == EVENT_ON_UNLOAD )
    {
        implts_deregisterDocument(xDocument); // sal_True => stop listening for disposing() !
    }
}

void SAL_CALL AutoRecovery::changesOccurred(const css::util::ChangesEvent& aEvent)
{
    const css::uno::Sequence< css::util::ElementChange > lChanges (aEvent.Changes);
    const css::util::ElementChange*                      pChanges = lChanges.getConstArray();

    sal_Int32 c = lChanges.getLength();
    sal_Int32 i = 0;

    /* SAFE */ {
    osl::ResettableMutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);

    // Changes of the configuration must be ignored if AutoSave/Recovery was disabled for this
    // office session. That can happen if e.g. the command line arguments "--norestore" or "--headless"
    // was set.
    if ((m_eJob & AutoRecovery::E_DISABLE_AUTORECOVERY) == AutoRecovery::E_DISABLE_AUTORECOVERY)
       return;

    for (i=0; i<c; ++i)
    {
        OUString sPath;
        pChanges[i].Accessor >>= sPath;

        if ( sPath == CFG_ENTRY_AUTOSAVE_ENABLED )
        {
            bool bEnabled = false;
            if (pChanges[i].Element >>= bEnabled)
            {
                if (bEnabled)
                {
                    m_eJob       |= AutoRecovery::E_AUTO_SAVE;
                    m_eTimerType  = AutoRecovery::E_NORMAL_AUTOSAVE_INTERVALL;
                }
                else
                {
                    m_eJob       &= ~AutoRecovery::E_AUTO_SAVE;
                    m_eTimerType  = AutoRecovery::E_DONT_START_TIMER;
                }
            }
        }
        else
        if ( sPath == CFG_ENTRY_AUTOSAVE_TIMEINTERVALL )
            pChanges[i].Element >>= m_nAutoSaveTimeIntervall;
    }

    } /* SAFE */

    // Note: This call stops the timer and starts it again.
    // But it checks the different timer states internally and
    // may be suppress the restart!
    implts_updateTimer();
}

void SAL_CALL AutoRecovery::modified(const css::lang::EventObject& aEvent)
{
    css::uno::Reference< css::frame::XModel > xDocument(aEvent.Source, css::uno::UNO_QUERY);
    if (! xDocument.is())
        return;

    implts_markDocumentModifiedAgainstLastBackup(xDocument);
}

void SAL_CALL AutoRecovery::disposing(const css::lang::EventObject& aEvent)
{
    /* SAFE */
    osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);

    if (aEvent.Source == m_xNewDocBroadcaster)
    {
        m_xNewDocBroadcaster.clear();
        return;
    }

    if (aEvent.Source == m_xRecoveryCFG)
    {
        m_xRecoveryCFG.clear();
        return;
    }

    // dispose from one of our cached documents ?
    // Normally they should send a OnUnload message ...
    // But some stacktraces shows another possible use case .-)
    css::uno::Reference< css::frame::XModel > xDocument(aEvent.Source, css::uno::UNO_QUERY);
    if (xDocument.is())
    {
        implts_deregisterDocument(xDocument, false); // sal_False => don't call removeEventListener() .. because it's not needed here
        return;
    }

    /* SAFE */
}

void AutoRecovery::implts_openConfig()
{
    /* SAFE */ {
    osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);

    if (m_xRecoveryCFG.is())
        return;
    } /* SAFE */

    css::uno::Reference<css::lang::XMultiServiceFactory> xConfigProvider(
            css::configuration::theDefaultProvider::get(m_xContext));

    std::vector<css::uno::Any> lParams;
    css::beans::PropertyValue aParam;

    // set root path
    aParam.Name = "nodepath";
    aParam.Value <<= OUString(CFG_PACKAGE_RECOVERY);
    lParams.push_back(css::uno::Any(aParam));

    // throws a RuntimeException if an error occurs!
    css::uno::Reference<css::container::XNameAccess> xCFG(
            xConfigProvider->createInstanceWithArguments(
                    "com.sun.star.configuration.ConfigurationAccess",
                    comphelper::containerToSequence(lParams)),
            css::uno::UNO_QUERY);

    sal_Int32 nMinSpaceDocSave    = MIN_DISCSPACE_DOCSAVE;
    sal_Int32 nMinSpaceConfigSave = MIN_DISCSPACE_CONFIGSAVE;

    try
    {
        nMinSpaceDocSave = officecfg::Office::Recovery::AutoSave::MinSpaceDocSave::get(m_xContext);
        nMinSpaceConfigSave = officecfg::Office::Recovery::AutoSave::MinSpaceConfigSave::get(m_xContext);
    }
    catch(const css::uno::Exception&)
    {
        // These config keys are not sooooo important, that
        // we are interested on errors here really .-)
        nMinSpaceDocSave    = MIN_DISCSPACE_DOCSAVE;
        nMinSpaceConfigSave = MIN_DISCSPACE_CONFIGSAVE;
    }

    /* SAFE */ {
    osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);
    m_xRecoveryCFG        = xCFG;
    m_nMinSpaceDocSave    = nMinSpaceDocSave;
    m_nMinSpaceConfigSave = nMinSpaceConfigSave;
    } /* SAFE */
}

void AutoRecovery::implts_readAutoSaveConfig()
{
    implts_openConfig();

    // AutoSave [bool]
    bool bEnabled(officecfg::Office::Recovery::AutoSave::Enabled::get(m_xContext));

    /* SAFE */ {
    osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);
    if (bEnabled)
    {
        bool bUserEnabled(officecfg::Office::Recovery::AutoSave::UserAutoSaveEnabled::get(m_xContext));

        m_eJob       |= AutoRecovery::E_AUTO_SAVE;
        m_eTimerType  = AutoRecovery::E_NORMAL_AUTOSAVE_INTERVALL;

        if (bUserEnabled)
        {
            m_eJob |= AutoRecovery::E_USER_AUTO_SAVE;
        }
        else
        {
            m_eJob &= ~AutoRecovery::E_USER_AUTO_SAVE;
        }
    }
    else
    {
        m_eJob       &= ~AutoRecovery::E_AUTO_SAVE;
        m_eTimerType  = AutoRecovery::E_DONT_START_TIMER;
    }
    } /* SAFE */

    // AutoSaveTimeIntervall [int] in min
    sal_Int32 nTimeIntervall(officecfg::Office::Recovery::AutoSave::TimeIntervall::get(m_xContext));

    /* SAFE */ {
    osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);
    m_nAutoSaveTimeIntervall = nTimeIntervall;
    } /* SAFE */
}

void AutoRecovery::implts_readConfig()
{
    implts_readAutoSaveConfig();

    // REENTRANT -> --------------------------------
    CacheLockGuard aCacheLock(this, cppu::WeakComponentImplHelperBase::rBHelper.rMutex, m_nDocCacheLock, LOCK_FOR_CACHE_ADD_REMOVE);

    /* SAFE */ {
    osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);
    // reset current cache load cache
    m_lDocCache.clear();
    m_nIdPool = 0;
    } /* SAFE */

    aCacheLock.unlock();
    // <- REENTRANT --------------------------------

    css::uno::Reference<css::container::XNameAccess> xRecoveryList(
            officecfg::Office::Recovery::RecoveryList::get(m_xContext));
    const OUString sRECOVERY_ITEM_BASE_IDENTIFIER(RECOVERY_ITEM_BASE_IDENTIFIER);
    const css::uno::Sequence< OUString > lItems = xRecoveryList->getElementNames();
    const OUString*                      pItems = lItems.getConstArray();
    sal_Int32                            c      = lItems.getLength();
    sal_Int32                            i      = 0;

    // REENTRANT -> --------------------------
    aCacheLock.lock(LOCK_FOR_CACHE_ADD_REMOVE);

    for (i=0; i<c; ++i)
    {
        css::uno::Reference< css::beans::XPropertySet > xItem;
        xRecoveryList->getByName(pItems[i]) >>= xItem;
        if (!xItem.is())
            continue;

        AutoRecovery::TDocumentInfo aInfo;
        aInfo.NewTempURL.clear();
        aInfo.Document.clear();
        xItem->getPropertyValue(CFG_ENTRY_PROP_ORIGINALURL) >>= aInfo.OrgURL;
        xItem->getPropertyValue(CFG_ENTRY_PROP_TEMPURL) >>= aInfo.OldTempURL;
        xItem->getPropertyValue(CFG_ENTRY_PROP_TEMPLATEURL) >>= aInfo.TemplateURL;
        xItem->getPropertyValue(CFG_ENTRY_PROP_FILTER) >>= aInfo.RealFilter;
        xItem->getPropertyValue(CFG_ENTRY_PROP_DOCUMENTSTATE) >>= aInfo.DocumentState;
        xItem->getPropertyValue(CFG_ENTRY_PROP_MODULE) >>= aInfo.AppModule;
        xItem->getPropertyValue(CFG_ENTRY_PROP_TITLE) >>= aInfo.Title;
        xItem->getPropertyValue(CFG_ENTRY_PROP_VIEWNAMES) >>= aInfo.ViewNames;
        implts_specifyAppModuleAndFactory(aInfo);
        implts_specifyDefaultFilterAndExtension(aInfo);

        if (pItems[i].startsWith(sRECOVERY_ITEM_BASE_IDENTIFIER))
        {
            OUString sID = pItems[i].copy(sRECOVERY_ITEM_BASE_IDENTIFIER.getLength());
            aInfo.ID = sID.toInt32();
            /* SAFE */ {
            osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);
            if (aInfo.ID > m_nIdPool)
            {
                m_nIdPool = aInfo.ID+1;
                SAL_WARN_IF(m_nIdPool<0, "fwk.autorecovery", "AutoRecovery::implts_readConfig(): Overflow of IDPool detected!");
            }
            } /* SAFE */
        }
        else
            SAL_INFO("fwk.autorecovery", "AutoRecovery::implts_readConfig(): Who changed numbering of recovery items? Cache will be inconsistent then! I do not know, what will happen next time .-)");

        /* SAFE */ {
        osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);
        m_lDocCache.push_back(aInfo);
        } /* SAFE */
    }

    aCacheLock.unlock();
    // <- REENTRANT --------------------------

    implts_updateTimer();
}

void AutoRecovery::implts_specifyDefaultFilterAndExtension(AutoRecovery::TDocumentInfo& rInfo)
{
    if (rInfo.AppModule.isEmpty())
    {
        throw css::uno::RuntimeException(
                "Can not find out the default filter and its extension, if no application module is known!",
                static_cast< css::frame::XDispatch* >(this));
    }

    css::uno::Reference< css::container::XNameAccess> xCFG;
    /* SAFE */ {
    osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);
    xCFG = m_xModuleCFG;
    } /* SAFE */

    try
    {
        if (! xCFG.is())
        {
            implts_openConfig();
            // open module config on demand and cache the update access
            xCFG.set(officecfg::Setup::Office::Factories::get(m_xContext),
                    css::uno::UNO_QUERY_THROW);

            /* SAFE */ {
            osl::MutexGuard g2(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);
            m_xModuleCFG = xCFG;
            } /* SAFE */
        }

        css::uno::Reference< css::container::XNameAccess > xModuleProps(
            xCFG->getByName(rInfo.AppModule),
            css::uno::UNO_QUERY_THROW);

        xModuleProps->getByName(CFG_ENTRY_REALDEFAULTFILTER) >>= rInfo.DefaultFilter;

        css::uno::Reference< css::container::XNameAccess > xFilterCFG(
                m_xContext->getServiceManager()->createInstanceWithContext(
                    "com.sun.star.document.FilterFactory", m_xContext), css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::container::XNameAccess > xTypeCFG(
                m_xContext->getServiceManager()->createInstanceWithContext(
                    "com.sun.star.document.TypeDetection", m_xContext), css::uno::UNO_QUERY_THROW);

        ::comphelper::SequenceAsHashMap       lFilterProps        (xFilterCFG->getByName(rInfo.DefaultFilter));
        OUString                       sTypeRegistration   = lFilterProps.getUnpackedValueOrDefault(FILTER_PROP_TYPE, OUString());
        ::comphelper::SequenceAsHashMap       lTypeProps          (xTypeCFG->getByName(sTypeRegistration));
        css::uno::Sequence< OUString > lExtensions         = lTypeProps.getUnpackedValueOrDefault(TYPE_PROP_EXTENSIONS, css::uno::Sequence< OUString >());
        if (lExtensions.getLength())
        {
            rInfo.Extension = "." + lExtensions[0];
        }
        else
            rInfo.Extension = ".unknown";
    }
    catch(const css::uno::Exception&)
    {
        rInfo.DefaultFilter.clear();
        rInfo.Extension.clear();
    }
}

void AutoRecovery::implts_specifyAppModuleAndFactory(AutoRecovery::TDocumentInfo& rInfo)
{
    ENSURE_OR_THROW2(
        !rInfo.AppModule.isEmpty() || rInfo.Document.is(),
        "Can not find out the application module nor its factory URL, if no application module (or a suitable) document is known!",
        *this );

    css::uno::Reference< css::frame::XModuleManager2 > xManager = ModuleManager::create(m_xContext);

    if (rInfo.AppModule.isEmpty())
        rInfo.AppModule = xManager->identify(rInfo.Document);

    ::comphelper::SequenceAsHashMap lModuleDescription(xManager->getByName(rInfo.AppModule));
    lModuleDescription[OUString(CFG_ENTRY_PROP_EMPTYDOCUMENTURL)] >>= rInfo.FactoryURL;
    lModuleDescription[OUString(CFG_ENTRY_PROP_FACTORYSERVICE)] >>= rInfo.FactoryService;
}

void AutoRecovery::implts_collectActiveViewNames( AutoRecovery::TDocumentInfo& i_rInfo )
{
    ENSURE_OR_THROW2( i_rInfo.Document.is(), "need at document, at the very least", *this );

    i_rInfo.ViewNames.realloc(0);

    // obtain list of controllers of this document
    ::std::vector< OUString > aViewNames;
    const Reference< XModel2 > xModel( i_rInfo.Document, UNO_QUERY );
    if ( xModel.is() )
    {
        const Reference< css::container::XEnumeration > xEnumControllers( xModel->getControllers() );
        while ( xEnumControllers->hasMoreElements() )
        {
            const Reference< XController2 > xController( xEnumControllers->nextElement(), UNO_QUERY );
            OUString sViewName;
            if ( xController.is() )
                sViewName = xController->getViewControllerName();
            OSL_ENSURE( !sViewName.isEmpty(), "AutoRecovery::implts_collectActiveViewNames: (no XController2 ->) no view name -> no recovery of this view!" );

            if ( !sViewName.isEmpty() )
                aViewNames.push_back( sViewName );
        }
    }
    else
    {
        const Reference< XController2 > xController( xModel->getCurrentController(), UNO_QUERY );
        OUString sViewName;
        if ( xController.is() )
            sViewName = xController->getViewControllerName();
        OSL_ENSURE( !sViewName.isEmpty(), "AutoRecovery::implts_collectActiveViewNames: (no XController2 ->) no view name -> no recovery of this view!" );

        if ( !sViewName.isEmpty() )
            aViewNames.push_back( sViewName );
    }

    i_rInfo.ViewNames.realloc( aViewNames.size() );
    ::std::copy( aViewNames.begin(), aViewNames.end(), i_rInfo.ViewNames.getArray() );
}

void AutoRecovery::implts_persistAllActiveViewNames()
{
    osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);

    // This list will be filled with every document
    for (auto & elem : m_lDocCache)
    {
        implts_collectActiveViewNames(elem);
        implts_flushConfigItem(elem);
    }
}

void AutoRecovery::implts_flushConfigItem(const AutoRecovery::TDocumentInfo& rInfo, bool bRemoveIt)
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(
            comphelper::ConfigurationChanges::create(m_xContext));

    try
    {
        implts_openConfig();

        css::uno::Reference<css::container::XNameAccess> xCheck(
                officecfg::Office::Recovery::RecoveryList::get(batch));

        css::uno::Reference< css::container::XNameContainer >   xModify(xCheck, css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::lang::XSingleServiceFactory > xCreate(xCheck, css::uno::UNO_QUERY_THROW);

        OUStringBuffer sIDBuf;
        sIDBuf.append(RECOVERY_ITEM_BASE_IDENTIFIER);
        sIDBuf.append(rInfo.ID);
        OUString sID = sIDBuf.makeStringAndClear();

        // remove
        if (bRemoveIt)
        {
            // Catch NoSuchElementException.
            // It's not a good idea inside multithreaded environments to call hasElement - removeElement.
            // DO IT!
            try
            {
                xModify->removeByName(sID);
            }
            catch(const css::container::NoSuchElementException&)
            {
                return;
            }
        }
        else
        {
            // new/modify
            css::uno::Reference< css::beans::XPropertySet > xSet;
            bool                                        bNew = !xCheck->hasByName(sID);
            if (bNew)
                xSet.set(xCreate->createInstance(), css::uno::UNO_QUERY_THROW);
            else
                xCheck->getByName(sID) >>= xSet;

            xSet->setPropertyValue(CFG_ENTRY_PROP_ORIGINALURL, css::uno::makeAny(rInfo.OrgURL       ));
            xSet->setPropertyValue(CFG_ENTRY_PROP_TEMPURL, css::uno::makeAny(rInfo.OldTempURL   ));
            xSet->setPropertyValue(CFG_ENTRY_PROP_TEMPLATEURL, css::uno::makeAny(rInfo.TemplateURL  ));
            xSet->setPropertyValue(CFG_ENTRY_PROP_FILTER, css::uno::makeAny(rInfo.RealFilter));
            xSet->setPropertyValue(CFG_ENTRY_PROP_DOCUMENTSTATE, css::uno::makeAny(rInfo.DocumentState));
            xSet->setPropertyValue(CFG_ENTRY_PROP_MODULE, css::uno::makeAny(rInfo.AppModule));
            xSet->setPropertyValue(CFG_ENTRY_PROP_TITLE, css::uno::makeAny(rInfo.Title));
            xSet->setPropertyValue(CFG_ENTRY_PROP_VIEWNAMES, css::uno::makeAny(rInfo.ViewNames));

            if (bNew)
                xModify->insertByName(sID, css::uno::makeAny(xSet));
        }
    }
    catch(const css::uno::RuntimeException&)
    {
        throw;
    }
    catch(const css::uno::Exception&)
    {
        // ??? can it happen that a full disc let these set of operations fail too ???
    }

    sal_Int32 nRetry = RETRY_STORE_ON_FULL_DISC_FOREVER;
    do
    {
        try
        {
            batch->commit();

#ifdef TRIGGER_FULL_DISC_CHECK
            throw css::uno::Exception("trigger full disk check");
#else  // TRIGGER_FULL_DISC_CHECK
            nRetry = 0;
#endif // TRIGGER_FULL_DISC_CHECK
        }
        catch(const css::uno::Exception&)
        {
            // a) FULL DISC seems to be the problem behind                              => show error and retry it forever (e.g. retry=300)
            // b) unknown problem (may be locking problem)                              => reset RETRY value to more useful value(!) (e.g. retry=3)
            // c) unknown problem (may be locking problem) + 1..2 repeating operations  => throw the original exception to force generation of a stacktrace !

            sal_Int32 nMinSpaceConfigSave;
            /* SAFE */ {
            osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);
            nMinSpaceConfigSave = m_nMinSpaceConfigSave;
            } /* SAFE */

            if (! impl_enoughDiscSpace(nMinSpaceConfigSave))
                AutoRecovery::impl_showFullDiscError();
            else if (nRetry > RETRY_STORE_ON_MIGHT_FULL_DISC_USEFULL)
                nRetry = RETRY_STORE_ON_MIGHT_FULL_DISC_USEFULL;
            else if (nRetry <= GIVE_UP_RETRY)
                throw; // force stacktrace to know if there exist might other reasons, why an AutoSave can fail !!!

            --nRetry;
        }
    }
    while(nRetry>0);
}

void AutoRecovery::implts_startListening()
{
    css::uno::Reference< css::util::XChangesNotifier > xCFG;
    css::uno::Reference< css::frame::XGlobalEventBroadcaster > xBroadcaster;
    bool bListenForDocEvents;
    bool bListenForConfigChanges;
    /* SAFE */ {
    osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);
    xCFG.set              (m_xRecoveryCFG, css::uno::UNO_QUERY);
    xBroadcaster        = m_xNewDocBroadcaster;
    bListenForDocEvents = m_bListenForDocEvents;
    bListenForConfigChanges = m_bListenForConfigChanges;
    } /* SAFE */

    if (
        (  xCFG.is()                ) &&
        (! bListenForConfigChanges)
       )
    {
        css::uno::Reference<css::util::XChangesListener> const xListener(
                new WeakChangesListener(this));
        xCFG->addChangesListener(xListener);
        /* SAFE */ {
        osl::MutexGuard g2(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);
        m_xRecoveryCFGListener = xListener;
        m_bListenForConfigChanges = true;
        } /* SAFE */
    }

    if (!xBroadcaster.is())
    {
        xBroadcaster = css::frame::theGlobalEventBroadcaster::get(m_xContext);
        /* SAFE */ {
        osl::MutexGuard g2(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);
        m_xNewDocBroadcaster = xBroadcaster;
        } /* SAFE */
    }

    if (
        (  xBroadcaster.is()  ) &&
        (! bListenForDocEvents)
       )
    {
        css::uno::Reference<css::document::XDocumentEventListener> const
            xListener(new WeakDocumentEventListener(this));
        xBroadcaster->addDocumentEventListener(xListener);
        /* SAFE */ {
        osl::MutexGuard g2(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);
        m_xNewDocBroadcasterListener = xListener;
        m_bListenForDocEvents = true;
        } /* SAFE */
    }
}

void AutoRecovery::implts_stopListening()
{
    css::uno::Reference< css::util::XChangesNotifier > xCFG;
    css::uno::Reference< css::document::XDocumentEventBroadcaster > xGlobalEventBroadcaster;
    /* SAFE */ {
    osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);
    // Attention: Don't reset our internal members here too.
    // May be we must work with our configuration, but don't wish to be informed
    // about changes any longer. Needed e.g. during EMERGENCY_SAVE!
    xCFG.set                   (m_xRecoveryCFG      , css::uno::UNO_QUERY);
    xGlobalEventBroadcaster.set(m_xNewDocBroadcaster, css::uno::UNO_QUERY);
    } /* SAFE */

    if (xGlobalEventBroadcaster.is() && m_bListenForDocEvents)
    {
        xGlobalEventBroadcaster->removeDocumentEventListener(m_xNewDocBroadcasterListener);
        m_bListenForDocEvents = false;
    }

    if (xCFG.is() && m_bListenForConfigChanges)
    {
        xCFG->removeChangesListener(m_xRecoveryCFGListener);
        m_bListenForConfigChanges = false;
    }
}

void AutoRecovery::implts_startModifyListeningOnDoc(AutoRecovery::TDocumentInfo& rInfo)
{
    if (rInfo.ListenForModify)
        return;

    css::uno::Reference< css::util::XModifyBroadcaster > xBroadcaster(rInfo.Document, css::uno::UNO_QUERY);
    if (xBroadcaster.is())
    {
        css::uno::Reference< css::util::XModifyListener > xThis(static_cast< css::frame::XDispatch* >(this), css::uno::UNO_QUERY);
        xBroadcaster->addModifyListener(xThis);
        rInfo.ListenForModify = true;
    }
}

void AutoRecovery::implts_stopModifyListeningOnDoc(AutoRecovery::TDocumentInfo& rInfo)
{
    if (! rInfo.ListenForModify)
        return;

    css::uno::Reference< css::util::XModifyBroadcaster > xBroadcaster(rInfo.Document, css::uno::UNO_QUERY);
    if (xBroadcaster.is())
    {
        css::uno::Reference< css::util::XModifyListener > xThis(static_cast< css::frame::XDispatch* >(this), css::uno::UNO_QUERY);
        xBroadcaster->removeModifyListener(xThis);
        rInfo.ListenForModify = false;
    }
}

void AutoRecovery::implts_updateTimer()
{
    implts_stopTimer();

    sal_Int32 nMilliSeconds = 0;

    /* SAFE */ {
    osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);

    if (
        (m_eJob       == AutoRecovery::E_NO_JOB          ) || // TODO may be superfluous - E_DONT_START_TIMER should be used only
        (m_eTimerType == AutoRecovery::E_DONT_START_TIMER)
       )
        return;

    if (m_eTimerType == AutoRecovery::E_NORMAL_AUTOSAVE_INTERVALL)
    {
        nMilliSeconds = (m_nAutoSaveTimeIntervall*60000); // [min] => 60.000 ms
    }
    else if (m_eTimerType == AutoRecovery::E_POLL_FOR_USER_IDLE)
    {
        nMilliSeconds = MIN_TIME_FOR_USER_IDLE;
    }
    else if (m_eTimerType == AutoRecovery::E_POLL_TILL_AUTOSAVE_IS_ALLOWED)
        nMilliSeconds = 300; // there is a minimum time frame, where the user can lose some key input data!


    } /* SAFE */

    SolarMutexGuard g;
    m_aTimer.SetTimeout(nMilliSeconds);
    m_aTimer.Start();
}

void AutoRecovery::implts_stopTimer()
{
    SolarMutexGuard g;

    if (!m_aTimer.IsActive())
        return;
    m_aTimer.Stop();
}

IMPL_LINK_NOARG(AutoRecovery, implts_timerExpired, Timer *, void)
{
    try
    {
        // This method is called by using a pointer to us.
        // But we must be aware that we can be destroyed hardly
        // if our uno reference will be gone!
        // => Hold this object alive till this method finish its work.
        css::uno::Reference< css::uno::XInterface > xSelfHold(static_cast< css::lang::XTypeProvider* >(this));

        // Needed! Otherwise every reschedule request allow a new triggered timer event :-(
        implts_stopTimer();

        // The timer must be ignored if AutoSave/Recovery was disabled for this
        // office session. That can happen if e.g. the command line arguments "--norestore" or "--headless"
        // was set. But normally the timer was disabled if recovery was disabled ...
        // But so we are more "safe" .-)
        /* SAFE */ {
        osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);
        if ((m_eJob & AutoRecovery::E_DISABLE_AUTORECOVERY) == AutoRecovery::E_DISABLE_AUTORECOVERY)
           return;
        } /* SAFE */

        // check some "states", where it's not allowed (better: not a good idea) to
        // start an AutoSave. (e.g. if the user makes drag & drop ...)
        // Then we poll till this "disallowed" state is gone.
        bool bAutoSaveNotAllowed = Application::IsUICaptured();
        if (bAutoSaveNotAllowed)
        {
            /* SAFE */ {
            osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);
            m_eTimerType = AutoRecovery::E_POLL_TILL_AUTOSAVE_IS_ALLOWED;
            } /* SAFE */
            implts_updateTimer();
            return;
        }

        // analyze timer type.
        // If we poll for an user idle period, may be we must
        // do nothing here and start the timer again.
        /* SAFE */ {
        osl::ClearableMutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);

        if (m_eTimerType == AutoRecovery::E_POLL_FOR_USER_IDLE)
        {
            bool bUserIdle = Application::GetLastInputInterval() > MIN_TIME_FOR_USER_IDLE;
            if (!bUserIdle)
            {
                g.clear();
                implts_updateTimer();
                return;
            }
        }

        } /* SAFE */

        implts_informListener(AutoRecovery::E_AUTO_SAVE,
            AutoRecovery::implst_createFeatureStateEvent(AutoRecovery::E_AUTO_SAVE, OPERATION_START, nullptr));

        // force save of all currently open documents
        // The called method returns an info, if and how this
        // timer must be restarted.
        AutoRecovery::ETimerType eSuggestedTimer = implts_saveDocs(true/*bAllowUserIdleLoop*/, false);

        // If timer is not used for "short callbacks" (means polling
        // for special states) ... reset the handle state of all
        // cache items. Such handle state indicates, that a document
        // was already saved during the THIS(!) AutoSave session.
        // Of course NEXT AutoSave session must be started without
        // any "handle" state ...
        if (
            (eSuggestedTimer == AutoRecovery::E_DONT_START_TIMER         ) ||
            (eSuggestedTimer == AutoRecovery::E_NORMAL_AUTOSAVE_INTERVALL)
           )
        {
            implts_resetHandleStates();
        }

        implts_informListener(AutoRecovery::E_AUTO_SAVE,
            AutoRecovery::implst_createFeatureStateEvent(AutoRecovery::E_AUTO_SAVE, OPERATION_STOP, nullptr));

        // restart timer - because it was disabled before ...
        /* SAFE */ {
        osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);
        m_eTimerType = eSuggestedTimer;
        } /* SAFE */

        implts_updateTimer();
    }
    catch(const css::uno::Exception&)
    {
    }
}

IMPL_LINK_NOARG(AutoRecovery, implts_asyncDispatch, LinkParamNone*, void)
{
    DispatchParams aParams;
    /* SAFE */ {
        osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);
        aParams = m_aDispatchParams;
        css::uno::Reference< css::uno::XInterface > xHoldRefForMethodAlive = aParams.m_xHoldRefForAsyncOpAlive;
        m_aDispatchParams.forget(); // clears all members ... including the ref-hold object .-)
    } /* SAFE */

    try
    {
        implts_dispatch(aParams);
    }
    catch (...)
    {
    }
}

void AutoRecovery::implts_registerDocument(const css::uno::Reference< css::frame::XModel >& xDocument)
{
    // ignore corrupted events, where no document is given ... Runtime Error ?!
    if (!xDocument.is())
        return;

    CacheLockGuard aCacheLock(this, cppu::WeakComponentImplHelperBase::rBHelper.rMutex, m_nDocCacheLock, LOCK_FOR_CACHE_USE);

    // notification for already existing document !
    // Can happen if events came in asynchronous on recovery time.
    // Then our cache was filled from the configuration ... but now we get some
    // asynchronous events from the global event broadcaster. We must be sure that
    // we don't add the same document more than once.
    AutoRecovery::TDocumentList::iterator pIt = AutoRecovery::impl_searchDocument(m_lDocCache, xDocument);
    if (pIt != m_lDocCache.end())
    {
        // Normally nothing must be done for this "late" notification.
        // But may be the modified state was changed inbetween.
        // Check it...
        implts_updateModifiedState(xDocument);
        return;
    }

    aCacheLock.unlock();

    utl::MediaDescriptor lDescriptor(xDocument->getArgs());

    // check if this document must be ignored for recovery !
    // Some use cases don't wish support for AutoSave/Recovery ... as e.g. OLE-Server / ActiveX Control etcpp.
    bool bNoAutoSave = lDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_NOAUTOSAVE(), false);
    if (bNoAutoSave)
        return;

    // Check if doc is well known on the desktop. Otherwise ignore it!
    // Other frames mostly are used from external programs - e.g. the bean ...
    css::uno::Reference< css::frame::XController > xController = xDocument->getCurrentController();
    if (!xController.is())
        return;

    css::uno::Reference< css::frame::XFrame >   xFrame   = xController->getFrame();
    if (!xFrame.is())
        return;
    css::uno::Reference< css::frame::XDesktop > xDesktop (xFrame->getCreator(), css::uno::UNO_QUERY);
    if (!xDesktop.is())
        return;

    // if the document doesn't support the XDocumentRecovery interface, we're not interested in it.
    Reference< XDocumentRecovery > xDocRecovery( xDocument, UNO_QUERY );
    if ( !xDocRecovery.is() )
        return;

    // get all needed information of this document
    // We need it to update our cache or to locate already existing elements there!
    AutoRecovery::TDocumentInfo aNew;
    aNew.Document = xDocument;

    // TODO replace getLocation() with getURL() ... it's a workaround currently only!
    css::uno::Reference< css::frame::XStorable > xDoc(aNew.Document, css::uno::UNO_QUERY_THROW);
    aNew.OrgURL = xDoc->getLocation();

    css::uno::Reference< css::frame::XTitle > xTitle(aNew.Document, css::uno::UNO_QUERY_THROW);
    aNew.Title = xTitle->getTitle ();

    // classify the used application module, which is used by this document.
    implts_specifyAppModuleAndFactory(aNew);

    // Hack! Check for "illegal office documents"... as e.g. the Basic IDE
    // It's not really a full featured office document. It doesn't provide an URL, any filter, a factory URL etcpp.
    // TODO file bug to Basic IDE developers. They must remove the office document API from its service.
    if (
        (aNew.OrgURL.isEmpty()) &&
        (aNew.FactoryURL.isEmpty())
       )
    {
        OSL_FAIL( "AutoRecovery::implts_registerDocument: this should not happen anymore!" );
        // nowadays, the Basic IDE should already die on the "supports XDocumentRecovery" check. And no other known
        // document type fits in here ...
        return;
    }

    // By the way - get some information about the default format for saving!
    // and save an information about the real used filter by this document.
    // We save this document with DefaultFilter ... and load it with the RealFilter.
    implts_specifyDefaultFilterAndExtension(aNew);
    aNew.RealFilter = lDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_FILTERNAME()  , OUString());

    // Further we must know, if this document base on a template.
    // Then we must load it in a different way.
    css::uno::Reference< css::document::XDocumentPropertiesSupplier > xSupplier(aNew.Document, css::uno::UNO_QUERY);
    if (xSupplier.is()) // optional interface!
    {
        css::uno::Reference< css::document::XDocumentProperties > xDocProps(xSupplier->getDocumentProperties(), css::uno::UNO_QUERY_THROW);
        aNew.TemplateURL = xDocProps->getTemplateURL();
    }

    css::uno::Reference< css::util::XModifiable > xModifyCheck(xDocument, css::uno::UNO_QUERY_THROW);
    if (xModifyCheck->isModified())
    {
        aNew.DocumentState |= AutoRecovery::E_MODIFIED;
    }

    aCacheLock.lock(LOCK_FOR_CACHE_ADD_REMOVE);

    AutoRecovery::TDocumentInfo aInfo;
    /* SAFE */ {
    osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);

    // create a new cache entry ... this document is not known.
    ++m_nIdPool;
    aNew.ID = m_nIdPool;
    SAL_WARN_IF(m_nIdPool<0, "fwk.autorecovery", "AutoRecovery::implts_registerDocument(): Overflow of ID pool detected.");
    m_lDocCache.push_back(aNew);

    AutoRecovery::TDocumentList::iterator pIt1  = AutoRecovery::impl_searchDocument(m_lDocCache, xDocument);
    aInfo = *pIt1;

    } /* SAFE */

    implts_flushConfigItem(aInfo);
    implts_startModifyListeningOnDoc(aInfo);

    aCacheLock.unlock();
}

void AutoRecovery::implts_deregisterDocument(const css::uno::Reference< css::frame::XModel >& xDocument     ,
                                                   bool                                   bStopListening)
{
    AutoRecovery::TDocumentInfo aInfo;
    /* SAFE */ {
    osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);

    // Attention: Don't leave SAFE section, if you work with pIt!
    // Because it points directly into the m_lDocCache list ...
    CacheLockGuard aCacheLock(this, cppu::WeakComponentImplHelperBase::rBHelper.rMutex, m_nDocCacheLock, LOCK_FOR_CACHE_USE);

    AutoRecovery::TDocumentList::iterator pIt = AutoRecovery::impl_searchDocument(m_lDocCache, xDocument);
    if (pIt == m_lDocCache.end())
        return; // unknown document => not a runtime error! Because we register only a few documents. see registration ...

    aInfo = *pIt;

    aCacheLock.unlock();

    // Sometimes we close documents by ourself.
    // And these documents can't be deregistered.
    // Otherwise we lose our configuration data... but need it !
    // see SessionSave !
    if (aInfo.IgnoreClosing)
        return;

    CacheLockGuard aCacheLock2(this, cppu::WeakComponentImplHelperBase::rBHelper.rMutex, m_nDocCacheLock, LOCK_FOR_CACHE_ADD_REMOVE);
    pIt = AutoRecovery::impl_searchDocument(m_lDocCache, xDocument);
    if (pIt != m_lDocCache.end())
        m_lDocCache.erase(pIt);
    pIt = m_lDocCache.end(); // otherwise it's not specified what pIt means!
    aCacheLock2.unlock();

    } /* SAFE */

    /* This method is called within disposing() of the document too. But there it's not a good idea to
       deregister us as listener. Further it make no sense - because the broadcaster dies.
       So we suppress deregistration in such case...
    */
    if (bStopListening)
        implts_stopModifyListeningOnDoc(aInfo);

    AutoRecovery::st_impl_removeFile(aInfo.OldTempURL);
    AutoRecovery::st_impl_removeFile(aInfo.NewTempURL);
    implts_flushConfigItem(aInfo, true); // sal_True => remove it from config
}

void AutoRecovery::implts_markDocumentModifiedAgainstLastBackup(const css::uno::Reference< css::frame::XModel >& xDocument)
{
    CacheLockGuard aCacheLock(this, cppu::WeakComponentImplHelperBase::rBHelper.rMutex, m_nDocCacheLock, LOCK_FOR_CACHE_USE);

    /* SAFE */ {
    osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);

    AutoRecovery::TDocumentList::iterator pIt = AutoRecovery::impl_searchDocument(m_lDocCache, xDocument);
    if (pIt != m_lDocCache.end())
    {
        /* Now we know, that this document was modified again and must be saved next time.
           But we don't need this information for every e.g. key input of the user.
           So we stop listening here.
           But if the document was saved as temp. file we start listening for this event again.
        */
        implts_stopModifyListeningOnDoc(*pIt);
    }

    } /* SAFE */
}

void AutoRecovery::implts_updateModifiedState(const css::uno::Reference< css::frame::XModel >& xDocument)
{
    // use true as fallback to get every document on EmergencySave/AutoRecovery!
    bool bModified = true;
    css::uno::Reference< css::util::XModifiable > xModify(xDocument, css::uno::UNO_QUERY);
    if (xModify.is())
        bModified = xModify->isModified();

    CacheLockGuard aCacheLock(this, cppu::WeakComponentImplHelperBase::rBHelper.rMutex, m_nDocCacheLock, LOCK_FOR_CACHE_USE);

    /* SAFE */ {
    osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);

    AutoRecovery::TDocumentList::iterator pIt = AutoRecovery::impl_searchDocument(m_lDocCache, xDocument);
    if (pIt != m_lDocCache.end())
    {
        AutoRecovery::TDocumentInfo& rInfo = *pIt;

        if (bModified)
        {
            rInfo.DocumentState |= AutoRecovery::E_MODIFIED;
        }
        else
        {
            rInfo.DocumentState &= ~AutoRecovery::E_MODIFIED;
        }
    }

    } /* SAFE */
}

void AutoRecovery::implts_updateDocumentUsedForSavingState(const css::uno::Reference< css::frame::XModel >& xDocument      ,
                                                                 bool                                   bSaveInProgress)
{
    CacheLockGuard aCacheLock(this, cppu::WeakComponentImplHelperBase::rBHelper.rMutex, m_nDocCacheLock, LOCK_FOR_CACHE_USE);

    /* SAFE */ {
    osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);

    AutoRecovery::TDocumentList::iterator pIt = AutoRecovery::impl_searchDocument(m_lDocCache, xDocument);
    if (pIt == m_lDocCache.end())
        return;
    AutoRecovery::TDocumentInfo& rInfo = *pIt;
    rInfo.UsedForSaving = bSaveInProgress;

    } /* SAFE */
}

void AutoRecovery::implts_markDocumentAsSaved(const css::uno::Reference< css::frame::XModel >& xDocument)
{
    CacheLockGuard aCacheLock(this, cppu::WeakComponentImplHelperBase::rBHelper.rMutex, m_nDocCacheLock, LOCK_FOR_CACHE_USE);

    AutoRecovery::TDocumentInfo aInfo;
    OUString sRemoveURL1;
    OUString sRemoveURL2;
    /* SAFE */ {
    osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);

    AutoRecovery::TDocumentList::iterator pIt = AutoRecovery::impl_searchDocument(m_lDocCache, xDocument);
    if (pIt == m_lDocCache.end())
        return;
    aInfo = *pIt;

    /* Since the document has been saved, update its entry in the document
     * cache. We essentially reset the state of the document from an
     * autorecovery perspective, updating things like the filename (which
     * would change in the case of a 'Save as' operation) and the associated
     * backup file URL.  */

    aInfo.DocumentState = AutoRecovery::E_UNKNOWN;
    // TODO replace getLocation() with getURL() ... it's a workaround currently only!
    css::uno::Reference< css::frame::XStorable > xDoc(aInfo.Document, css::uno::UNO_QUERY);
    aInfo.OrgURL = xDoc->getLocation();

    /* Save off the backup file URLs and then clear them. NOTE - it is
     * important that we clear them - otherwise, we could enter a state
     * where pIt->OldTempURL == pIt->NewTempURL and our backup algorithm
     * in implts_saveOneDoc will write to that URL and then delete the file
     * at that URL (bug #96607) */
    sRemoveURL1 = aInfo.OldTempURL;
    sRemoveURL2 = aInfo.NewTempURL;
    aInfo.OldTempURL.clear();
    aInfo.NewTempURL.clear();

    utl::MediaDescriptor lDescriptor(aInfo.Document->getArgs());
    aInfo.RealFilter = lDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_FILTERNAME(), OUString());

    css::uno::Reference< css::frame::XTitle > xDocTitle(xDocument, css::uno::UNO_QUERY);
    if (xDocTitle.is ())
        aInfo.Title = xDocTitle->getTitle ();
    else
    {
        aInfo.Title      = lDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_TITLE()     , OUString());
        if (aInfo.Title.isEmpty())
            aInfo.Title  = lDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_DOCUMENTTITLE(), OUString());
    }

    aInfo.UsedForSaving = false;

    *pIt = aInfo;

    } /* SAFE */

    implts_flushConfigItem(aInfo);

    aCacheLock.unlock();

    AutoRecovery::st_impl_removeFile(sRemoveURL1);
    AutoRecovery::st_impl_removeFile(sRemoveURL2);
}

AutoRecovery::TDocumentList::iterator AutoRecovery::impl_searchDocument(      AutoRecovery::TDocumentList&               rList    ,
                                                                        const css::uno::Reference< css::frame::XModel >& xDocument)
{
    return std::find_if(rList.begin(), rList.end(),
        [&xDocument](const AutoRecovery::TDocumentInfo& rInfo) { return rInfo.Document == xDocument; });
}

void lcl_changeVisibility( const css::uno::Reference< css::frame::XFramesSupplier >& i_rFrames, bool i_bVisible )
{
    css::uno::Reference< css::container::XIndexAccess > xFramesContainer( i_rFrames->getFrames(), css::uno::UNO_QUERY );
    const sal_Int32 count = xFramesContainer->getCount();

    Any aElement;
    for ( sal_Int32 i=0; i < count; ++i )
    {
        aElement = xFramesContainer->getByIndex(i);
        // check for sub frames
        css::uno::Reference< css::frame::XFramesSupplier > xFramesSupp( aElement, css::uno::UNO_QUERY );
        if ( xFramesSupp.is() )
            lcl_changeVisibility( xFramesSupp, i_bVisible );

        css::uno::Reference< css::frame::XFrame > xFrame( aElement, css::uno::UNO_QUERY );
        if ( !xFrame.is() )
            continue;

        css::uno::Reference< css::awt::XWindow > xWindow( xFrame->getContainerWindow(), UNO_SET_THROW );
        xWindow->setVisible( i_bVisible );
    }
}

void AutoRecovery::implts_changeAllDocVisibility(bool bVisible)
{
    css::uno::Reference< css::frame::XFramesSupplier > xDesktop( css::frame::Desktop::create(m_xContext), css::uno::UNO_QUERY);
    lcl_changeVisibility( xDesktop, bVisible );
}

/* Currently the document is not closed in case of crash,
   so the lock file must be removed explicitly
*/
void lc_removeLockFile(AutoRecovery::TDocumentInfo const & rInfo)
{
#if !HAVE_FEATURE_MULTIUSER_ENVIRONMENT || HAVE_FEATURE_MACOSX_SANDBOX
    (void) rInfo;
#else
    if ( rInfo.Document.is() )
    {
        try
        {
            css::uno::Reference< css::frame::XStorable > xStore(rInfo.Document, css::uno::UNO_QUERY_THROW);
            OUString aURL = xStore->getLocation();
            if ( !aURL.isEmpty() )
            {
                ::svt::DocumentLockFile aLockFile( aURL );
                aLockFile.RemoveFile();
            }
        }
        catch( const css::uno::Exception& )
        {
        }
    }
#endif
}

void AutoRecovery::implts_prepareSessionShutdown()
{
    SAL_INFO("fwk.autorecovery", "AutoRecovery::implts_prepareSessionShutdown() starts ...");

    // a) reset modified documents (of course the must be saved before this method is called!)
    // b) close it without showing any UI!

    /* SAFE */ {
    CacheLockGuard aCacheLock(this, cppu::WeakComponentImplHelperBase::rBHelper.rMutex, m_nDocCacheLock, LOCK_FOR_CACHE_USE);

    for (auto & info : m_lDocCache)
    {
        // WORKAROUND... Since the documents are not closed the lock file must be removed explicitly
        // it is not done on documents saving since shutdown can be cancelled
        lc_removeLockFile( info );

        // Prevent us from deregistration of these documents.
        // Because we close these documents by ourself (see XClosable below) ...
        // it's fact, that we reach our deregistration method. There we
        // must not(!) update our configuration ... Otherwise all
        // session data are lost !!!
        info.IgnoreClosing = true;

        // reset modified flag of these documents (ignoring the notification about it!)
        // Otherwise a message box is shown on closing these models.
        implts_stopModifyListeningOnDoc(info);

        // if the session save is still running the documents should not be thrown away,
        // actually that would be a bad sign, that means that the SessionManager tries
        // to kill the session before the saving is ready
        if ((m_eJob & AutoRecovery::E_SESSION_SAVE) != AutoRecovery::E_SESSION_SAVE)
        {
            css::uno::Reference< css::util::XModifiable > xModify(info.Document, css::uno::UNO_QUERY);
            if (xModify.is())
                xModify->setModified(false);

            // close the model.
            css::uno::Reference< css::util::XCloseable > xClose(info.Document, css::uno::UNO_QUERY);
            if (xClose.is())
            {
                try
                {
                    xClose->close(false);
                }
                catch(const css::uno::Exception&)
                {
                    // At least it's only a try to close these documents before anybody else it does.
                    // So it seems to be possible to ignore any error here .-)
                }

                info.Document.clear();
            }
        }
    }

    aCacheLock.unlock();
    } /* SAFE */
}

/* TODO WORKAROUND:

        #i64599#

        Normally the MediaDescriptor argument NoAutoSave indicates,
        that a document must be ignored for AutoSave and Recovery.
        But sometimes XModel->getArgs() does not contained this information
        if implts_registerDocument() was called.
        So we have to check a second time, if this property is set...
        Best place doing so is to check it immediately before saving
        and suppressing saving the document then.
        Of course removing the corresponding cache entry is not an option.
        Because it would disturb iteration over the cache!
        So we ignore such documents only...
        Hopefully next time they are not inserted in our cache.
*/
bool lc_checkIfSaveForbiddenByArguments(AutoRecovery::TDocumentInfo const & rInfo)
{
    if (! rInfo.Document.is())
        return true;

    utl::MediaDescriptor lDescriptor(rInfo.Document->getArgs());
    bool bNoAutoSave = lDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_NOAUTOSAVE(), false);

    return bNoAutoSave;
}

AutoRecovery::ETimerType AutoRecovery::implts_saveDocs(       bool        bAllowUserIdleLoop,
                                                              bool        bRemoveLockFiles,
                                                        const DispatchParams* pParams           )
{
    css::uno::Reference< css::task::XStatusIndicator > xExternalProgress;
    if (pParams)
        xExternalProgress = pParams->m_xProgress;

    css::uno::Reference< css::frame::XDesktop2 > xDesktop = css::frame::Desktop::create(m_xContext);
    OUString                              sBackupPath(SvtPathOptions().GetBackupPath());

    css::uno::Reference< css::frame::XController > xActiveController;
    css::uno::Reference< css::frame::XModel >      xActiveModel;
    css::uno::Reference< css::frame::XFrame >      xActiveFrame     = xDesktop->getActiveFrame();
    if (xActiveFrame.is())
        xActiveController = xActiveFrame->getController();
    if (xActiveController.is())
        xActiveModel = xActiveController->getModel();

    // Set the default timer action for our call.
    // Default = NORMAL_AUTOSAVE
    // We return a suggestion for an active timer only.
    // It will be ignored if the timer was disabled by the user ...
    // Further this state can be set to USER_IDLE only later in this method.
    // It's not allowed to reset such state then. Because we must know, if
    // there exists POSTPONED documents. see below ...
    AutoRecovery::ETimerType eTimer = AutoRecovery::E_NORMAL_AUTOSAVE_INTERVALL;

    sal_Int32 eJob = m_eJob;

    CacheLockGuard aCacheLock(this, cppu::WeakComponentImplHelperBase::rBHelper.rMutex, m_nDocCacheLock, LOCK_FOR_CACHE_USE);

    /* SAFE */ {
    osl::ResettableMutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);

    // This list will be filled with every document
    // which should be saved as last one. E.g. if it was used
    // already for an UI save operation => crashed ... and
    // now we try to save it again ... which can fail again ( of course .-) ).
    ::std::vector< AutoRecovery::TDocumentList::iterator > lDangerousDocs;

    AutoRecovery::TDocumentList::iterator pIt;
    for (  pIt  = m_lDocCache.begin();
           pIt != m_lDocCache.end();
         ++pIt                       )
    {
        AutoRecovery::TDocumentInfo aInfo = *pIt;

        // WORKAROUND... Since the documents are not closed the lock file must be removed explicitly
        if ( bRemoveLockFiles )
            lc_removeLockFile( aInfo );

        // WORKAROUND ... see comment of this method
        if (lc_checkIfSaveForbiddenByArguments(aInfo))
            continue;

        // already auto saved during this session :-)
        // This state must be reset for all documents
        // if timer is started with normal AutoSaveTimerIntervall!
        if ((aInfo.DocumentState & AutoRecovery::E_HANDLED) == AutoRecovery::E_HANDLED)
            continue;

        // Not modified documents are not saved.
        // We safe an information about the URL only!
        Reference< XDocumentRecovery > xDocRecover( aInfo.Document, UNO_QUERY_THROW );
        if ( !xDocRecover->wasModifiedSinceLastSave() )
        {
            aInfo.DocumentState |= AutoRecovery::E_HANDLED;
            continue;
        }

        // check if this document is still used by a concurrent save operation
        // e.g. if the user tried to save via UI.
        // Handle it in the following way:
        // i)   For an AutoSave ... ignore this document! It will be saved and next time we will (hopefully)
        //      get a notification about the state of this operation.
        //      And if a document was saved by the user we can remove our temp. file. But that will be done inside
        //      our callback for SaveDone notification.
        // ii)  For a CrashSave ... add it to the list of dangerous documents and
        //      save it after all other documents was saved successfully. That decrease
        //      the chance for a crash inside a crash.
        //      On the other side it's not necessary for documents, which are not modified.
        //      They can be handled normally - means we patch the corresponding configuration entry only.
        // iii) For a SessionSave ... ignore it! There is no time to wait for this save operation.
        //      Because the WindowManager will kill the process if it doesn't react immediately.
        //      On the other side we can't risk a concurrent save request ... because we know
        //      that it will produce a crash.

        // Attention: Because eJob is used as a flag field, you have to check for the worst case first.
        // E.g. a CrashSave can overwrite an AutoSave. So you have to check for a CrashSave before an AutoSave!
        if (aInfo.UsedForSaving)
        {
            if ((eJob & AutoRecovery::E_EMERGENCY_SAVE) == AutoRecovery::E_EMERGENCY_SAVE)
            {
                lDangerousDocs.push_back(pIt);
                continue;
            }
            else
            if ((eJob & AutoRecovery::E_SESSION_SAVE) == AutoRecovery::E_SESSION_SAVE)
            {
                continue;
            }
            else
            if ((eJob & AutoRecovery::E_AUTO_SAVE) == AutoRecovery::E_AUTO_SAVE)
            {
                eTimer = AutoRecovery::E_POLL_TILL_AUTOSAVE_IS_ALLOWED;
                aInfo.DocumentState |= AutoRecovery::E_POSTPONED;
                continue;
            }
        }

        // a) Document was not postponed - and is     active now. => postpone it (restart timer, restart loop)
        // b) Document was not postponed - and is not active now. => save it
        // c) Document was     postponed - and is not active now. => save it
        // d) Document was     postponed - and is     active now. => save it (because user idle was checked already)
        bool bActive       = (xActiveModel == aInfo.Document);
        bool bWasPostponed = ((aInfo.DocumentState & AutoRecovery::E_POSTPONED) == AutoRecovery::E_POSTPONED);

        if (
            ! bWasPostponed &&
              bActive
           )
        {
            aInfo.DocumentState |= AutoRecovery::E_POSTPONED;
            *pIt = aInfo;
            // postponed documents will be saved if this method is called again!
            // That can be done by an outside started timer           => E_POLL_FOR_USER_IDLE (if normal AutoSave is active)
            // or it must be done directly without starting any timer => E_CALL_ME_BACK       (if Emergency- or SessionSave is active and must be finished ASAP!)
            eTimer = AutoRecovery::E_POLL_FOR_USER_IDLE;
            if (!bAllowUserIdleLoop)
                eTimer = AutoRecovery::E_CALL_ME_BACK;
            continue;
        }

        // b, c, d)
        // } /* SAFE */
        g.clear();
        // changing of aInfo and flushing it is done inside implts_saveOneDoc!
        implts_saveOneDoc(sBackupPath, aInfo, xExternalProgress);
        implts_informListener(eJob, AutoRecovery::implst_createFeatureStateEvent(eJob, OPERATION_UPDATE, &aInfo));
        g.reset();
        // /* SAFE */ {

        *pIt = aInfo;
    }

    // Did we have some "dangerous candidates" ?
    // Try to save it ... but may be it will fail !
    for (auto const& dangerousDoc : lDangerousDocs)
    {
        pIt = dangerousDoc;
        AutoRecovery::TDocumentInfo aInfo = *pIt;

        // } /* SAFE */
        g.clear();
        // changing of aInfo and flushing it is done inside implts_saveOneDoc!
        implts_saveOneDoc(sBackupPath, aInfo, xExternalProgress);
        implts_informListener(eJob, AutoRecovery::implst_createFeatureStateEvent(eJob, OPERATION_UPDATE, &aInfo));
        g.reset();
        // /* SAFE */ {

        *pIt = aInfo;
    }

    } /* SAFE */

    return eTimer;
}

void AutoRecovery::implts_saveOneDoc(const OUString&                                    sBackupPath      ,
                                           AutoRecovery::TDocumentInfo&                        rInfo            ,
                                     const css::uno::Reference< css::task::XStatusIndicator >& xExternalProgress)
{
    // no document? => can occur if we loaded our configuration with files,
    // which couldn't be recovered successfully. In such case we have all needed information
    // excepting the real document instance!

    // TODO: search right place, where such "dead files" can be removed from the configuration!
    if (!rInfo.Document.is())
        return;

    utl::MediaDescriptor lOldArgs(rInfo.Document->getArgs());
    implts_generateNewTempURL(sBackupPath, lOldArgs, rInfo);

    // if the document was loaded with a password, it should be
    // stored with password
    utl::MediaDescriptor lNewArgs;
    css::uno::Sequence< css::beans::NamedValue > aEncryptionData =
        lOldArgs.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_ENCRYPTIONDATA(),
                css::uno::Sequence< css::beans::NamedValue >());
    if (aEncryptionData.getLength() > 0)
        lNewArgs[utl::MediaDescriptor::PROP_ENCRYPTIONDATA()] <<= aEncryptionData;

    // Further it must be saved using the default file format of that application.
    // Otherwise we will some data lost.
    if (!rInfo.DefaultFilter.isEmpty())
        lNewArgs[utl::MediaDescriptor::PROP_FILTERNAME()] <<= rInfo.DefaultFilter;

    // prepare frame/document/mediadescriptor in a way, that it uses OUR progress .-)
    if (xExternalProgress.is())
        lNewArgs[utl::MediaDescriptor::PROP_STATUSINDICATOR()] <<= xExternalProgress;
    impl_establishProgress(rInfo, lNewArgs, css::uno::Reference< css::frame::XFrame >());

    // #i66598# use special handling of property "DocumentBaseURL" (it must be an empty string!)
    // for make hyperlinks working
    lNewArgs[utl::MediaDescriptor::PROP_DOCUMENTBASEURL()] <<= OUString();

    // try to save this document as a new temp file every time.
    // Mark AutoSave state as "INCOMPLETE" if it failed.
    // Because the last temp file is to old and does not include all changes.
    Reference< XDocumentRecovery > xDocRecover(rInfo.Document, css::uno::UNO_QUERY_THROW);

    // safe the state about "trying to save"
    // ... we need it for recovery if e.g. a crash occurs inside next line!
    rInfo.DocumentState |= AutoRecovery::E_TRY_SAVE;
    implts_flushConfigItem(rInfo);

    // If userautosave is enabled, first try to save the original file.
    // Note that we must do it *before* calling storeToRecoveryFile, so in case of failure here
    // we won't remain with the modified flag set to true, even though the autorecovery save succeeded.
    try
    {
        // We must check here for an empty URL to avoid a "This operation is not supported on this operating system."
        // message during autosave.
        if ((m_eJob & AutoRecovery::E_USER_AUTO_SAVE) == AutoRecovery::E_USER_AUTO_SAVE && !rInfo.OrgURL.isEmpty())
        {
            Reference< XStorable > xDocSave(rInfo.Document, css::uno::UNO_QUERY_THROW);
            xDocSave->store();
        }
    }
    catch(const css::uno::Exception&)
    {
    }

    sal_Int32 nRetry = RETRY_STORE_ON_FULL_DISC_FOREVER;
    bool  bError = false;
    do
    {
        try
        {
            xDocRecover->storeToRecoveryFile( rInfo.NewTempURL, lNewArgs.getAsConstPropertyValueList() );

#ifdef TRIGGER_FULL_DISC_CHECK
            throw css::uno::Exception("trigger full disk check");
#else  // TRIGGER_FULL_DISC_CHECK

            bError = false;
            nRetry = 0;
#endif // TRIGGER_FULL_DISC_CHECK
        }
        catch(const css::uno::Exception&)
        {
            bError = true;

            // a) FULL DISC seems to be the problem behind                              => show error and retry it forever (e.g. retry=300)
            // b) unknown problem (may be locking problem)                              => reset RETRY value to more useful value(!) (e.g. retry=3)
            // c) unknown problem (may be locking problem) + 1..2 repeating operations  => throw the original exception to force generation of a stacktrace !

            sal_Int32 nMinSpaceDocSave;
            /* SAFE */ {
            osl::MutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);
            nMinSpaceDocSave = m_nMinSpaceDocSave;
            } /* SAFE */

            if (! impl_enoughDiscSpace(nMinSpaceDocSave))
                AutoRecovery::impl_showFullDiscError();
            else if (nRetry > RETRY_STORE_ON_MIGHT_FULL_DISC_USEFULL)
                nRetry = RETRY_STORE_ON_MIGHT_FULL_DISC_USEFULL;
            else if (nRetry <= GIVE_UP_RETRY)
                throw; // force stacktrace to know if there exist might other reasons, why an AutoSave can fail !!!

            --nRetry;
        }
    }
    while(nRetry>0);

    if (! bError)
    {
        // safe the state about success
        // ... you know the reason: to know it on recovery time if next line crash .-)
        rInfo.DocumentState &= ~AutoRecovery::E_TRY_SAVE;
        rInfo.DocumentState |=  AutoRecovery::E_HANDLED;
        rInfo.DocumentState |=  AutoRecovery::E_SUCCEEDED;
    }
    else
    {
        // safe the state about error ...
        rInfo.NewTempURL.clear();
        rInfo.DocumentState &= ~AutoRecovery::E_TRY_SAVE;
        rInfo.DocumentState |=  AutoRecovery::E_HANDLED;
        rInfo.DocumentState |=  AutoRecovery::E_INCOMPLETE;
    }

    // make sure the progress is not referred any longer
    impl_forgetProgress(rInfo, lNewArgs, css::uno::Reference< css::frame::XFrame >());

    // try to remove the old temp file.
    // Ignore any error here. We have a new temp file, which is up to date.
    // The only thing is: we fill the disk with temp files, if we can't remove old ones :-)
    OUString sRemoveFile      = rInfo.OldTempURL;
    rInfo.OldTempURL = rInfo.NewTempURL;
    rInfo.NewTempURL.clear();

    implts_flushConfigItem(rInfo);

    // We must know if the user modifies the document again ...
    implts_startModifyListeningOnDoc(rInfo);

    AutoRecovery::st_impl_removeFile(sRemoveFile);
}

AutoRecovery::ETimerType AutoRecovery::implts_openDocs(const DispatchParams& aParams)
{
    AutoRecovery::ETimerType eTimer = AutoRecovery::E_DONT_START_TIMER;

    CacheLockGuard aCacheLock(this, cppu::WeakComponentImplHelperBase::rBHelper.rMutex, m_nDocCacheLock, LOCK_FOR_CACHE_USE);

    /* SAFE */ {
    osl::ResettableMutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);

    sal_Int32                             eJob = m_eJob;
    for (auto & info : m_lDocCache)
    {
        // Such documents are already loaded by the last loop.
        // Don't check E_SUCCEEDED here! It may be the final state of an AutoSave
        // operation before!!!
        if ((info.DocumentState & AutoRecovery::E_HANDLED) == AutoRecovery::E_HANDLED)
            continue;

        // a1,b1,c1,d2,e2,f2)
        if ((info.DocumentState & AutoRecovery::E_DAMAGED) == AutoRecovery::E_DAMAGED)
        {
            // don't forget to inform listener! May be this document was
            // damaged on last saving time ...
            // Then our listener need this notification.
            // If it was damaged during last "try to open" ...
            // it will be notified more than once. SH.. HAPPENS ...
            // } /* SAFE */
            g.clear();
            implts_informListener(eJob,
                AutoRecovery::implst_createFeatureStateEvent(eJob, OPERATION_UPDATE, &info));
            g.reset();
            // /* SAFE */ {
            continue;
        }

        utl::MediaDescriptor lDescriptor;

        // it's an UI feature - so the "USER" itself must be set as referer
        lDescriptor[utl::MediaDescriptor::PROP_REFERRER()] <<= OUString(REFERRER_USER);
        lDescriptor[utl::MediaDescriptor::PROP_SALVAGEDFILE()] <<= OUString();

        // recovered documents are loaded hidden, and shown all at once, later
        lDescriptor[utl::MediaDescriptor::PROP_HIDDEN()] <<= true;

        if (aParams.m_xProgress.is())
            lDescriptor[utl::MediaDescriptor::PROP_STATUSINDICATOR()] <<= aParams.m_xProgress;

        bool bBackupWasTried   = (
                                        ((info.DocumentState & AutoRecovery::E_TRY_LOAD_BACKUP  ) == AutoRecovery::E_TRY_LOAD_BACKUP) || // temp. state!
                                        ((info.DocumentState & AutoRecovery::E_INCOMPLETE       ) == AutoRecovery::E_INCOMPLETE     )    // transport TRY_LOAD_BACKUP from last loop to this new one!
                                     );
        bool bOriginalWasTried = ((info.DocumentState & AutoRecovery::E_TRY_LOAD_ORIGINAL) == AutoRecovery::E_TRY_LOAD_ORIGINAL);

        if (bBackupWasTried)
        {
            if (!bOriginalWasTried)
            {
                info.DocumentState |= AutoRecovery::E_INCOMPLETE;
                // try original URL ... ! don't continue with next item here ...
            }
            else
            {
                info.DocumentState |= AutoRecovery::E_DAMAGED;
                continue;
            }
        }

        OUString sLoadOriginalURL;
        OUString sLoadBackupURL;

        if (!bBackupWasTried)
            sLoadBackupURL = info.OldTempURL;

        if (!info.OrgURL.isEmpty())
        {
            sLoadOriginalURL = info.OrgURL;
        }
        else if (!info.TemplateURL.isEmpty())
        {
            sLoadOriginalURL = info.TemplateURL;
            lDescriptor[utl::MediaDescriptor::PROP_ASTEMPLATE()]   <<= true;
            lDescriptor[utl::MediaDescriptor::PROP_TEMPLATENAME()] <<= info.TemplateURL;
        }
        else if (!info.FactoryURL.isEmpty())
        {
            sLoadOriginalURL = info.FactoryURL;
            lDescriptor[utl::MediaDescriptor::PROP_ASTEMPLATE()] <<= true;
        }

        // A "Salvaged" item must exists every time. The core can make something special then for recovery.
        // Of course it should be the real file name of the original file, in case we load the temp. backup here.
        OUString sURL;
        if (!sLoadBackupURL.isEmpty())
        {
            sURL = sLoadBackupURL;
            info.DocumentState |= AutoRecovery::E_TRY_LOAD_BACKUP;
            lDescriptor[utl::MediaDescriptor::PROP_SALVAGEDFILE()] <<= sLoadOriginalURL;
        }
        else if (!sLoadOriginalURL.isEmpty())
        {
            sURL = sLoadOriginalURL;
            info.DocumentState |= AutoRecovery::E_TRY_LOAD_ORIGINAL;
        }
        else
            continue; // TODO ERROR!

        LoadEnv::initializeUIDefaults( m_xContext, lDescriptor, true, nullptr );

        // } /* SAFE */
        g.clear();

        implts_flushConfigItem(info);
        implts_informListener(eJob,
            AutoRecovery::implst_createFeatureStateEvent(eJob, OPERATION_UPDATE, &info));

        try
        {
            implts_openOneDoc(sURL, lDescriptor, info);
        }
        catch(const css::uno::Exception&)
        {
            info.DocumentState &= ~AutoRecovery::E_TRY_LOAD_BACKUP;
            info.DocumentState &= ~AutoRecovery::E_TRY_LOAD_ORIGINAL;
            if (!sLoadBackupURL.isEmpty())
            {
                info.DocumentState |= AutoRecovery::E_INCOMPLETE;
                eTimer               = AutoRecovery::E_CALL_ME_BACK;
            }
            else
            {
                info.DocumentState |=  AutoRecovery::E_HANDLED;
                info.DocumentState |=  AutoRecovery::E_DAMAGED;
            }

            implts_flushConfigItem(info, true);
            implts_informListener(eJob,
                AutoRecovery::implst_createFeatureStateEvent(eJob, OPERATION_UPDATE, &info));

            // /* SAFE */ {
            // Needed for next loop!
            g.reset();
            continue;
        }

        if (!info.RealFilter.isEmpty())
        {
            utl::MediaDescriptor lPatchDescriptor(info.Document->getArgs());
            lPatchDescriptor[utl::MediaDescriptor::PROP_FILTERNAME()] <<= info.RealFilter;
            info.Document->attachResource(info.Document->getURL(), lPatchDescriptor.getAsConstPropertyValueList());
                // do *not* use sURL here. In case this points to the recovery file, it has already been passed
                // to recoverFromFile. Also, passing it here is logically wrong, as attachResource is intended
                // to take the logical file URL.
        }

        css::uno::Reference< css::util::XModifiable > xModify(info.Document, css::uno::UNO_QUERY);
        if ( xModify.is() )
        {
            bool bModified = ((info.DocumentState & AutoRecovery::E_MODIFIED) == AutoRecovery::E_MODIFIED);
            xModify->setModified(bModified);
        }

        info.DocumentState &= ~AutoRecovery::E_TRY_LOAD_BACKUP;
        info.DocumentState &= ~AutoRecovery::E_TRY_LOAD_ORIGINAL;
        info.DocumentState |=  AutoRecovery::E_HANDLED;
        info.DocumentState |=  AutoRecovery::E_SUCCEEDED;

        implts_flushConfigItem(info);
        implts_informListener(eJob,
            AutoRecovery::implst_createFeatureStateEvent(eJob, OPERATION_UPDATE, &info));

        /* Normally we listen as XModifyListener on a document to know if a document was changed
           since our last AutoSave. And we deregister us in case we know this state.
           But directly after one document as recovered ... we must start listening.
           Otherwise the first "modify" doesn't reach us. Because we ourself called setModified()
           on the document via API. And currently we don't listen for any events (not at theGlobalEventBroadcaster
           nor at any document!).
        */
        implts_startModifyListeningOnDoc(info);

        // /* SAFE */ {
        // Needed for next loop. Don't unlock it again!
        g.reset();
    }

    } /* SAFE */

    return eTimer;
}

void AutoRecovery::implts_openOneDoc(const OUString&               sURL       ,
                                           utl::MediaDescriptor& lDescriptor,
                                           AutoRecovery::TDocumentInfo&   rInfo      )
{
    css::uno::Reference< css::frame::XDesktop2 > xDesktop = css::frame::Desktop::create(m_xContext);

    ::std::vector< Reference< XComponent > > aCleanup;
    try
    {
        // create a new document of the desired type
        Reference< XModel2 > xModel(m_xContext->getServiceManager()->createInstanceWithContext(
                    rInfo.FactoryService, m_xContext), UNO_QUERY_THROW);
        aCleanup.emplace_back(xModel.get() );

        // put the filter name into the descriptor - we're not going to involve any type detection, so
        // the document might be lost without the FilterName property
        if ( (rInfo.DocumentState & AutoRecovery::E_TRY_LOAD_ORIGINAL) == AutoRecovery::E_TRY_LOAD_ORIGINAL)
            lDescriptor[ utl::MediaDescriptor::PROP_FILTERNAME() ] <<= rInfo.RealFilter;
        else
            lDescriptor[ utl::MediaDescriptor::PROP_FILTERNAME() ] <<= rInfo.DefaultFilter;

        if ( sURL == rInfo.FactoryURL )
        {
            // if the document was a new, unmodified document, then there's nothing to recover, just to init
            ENSURE_OR_THROW( ( rInfo.DocumentState & AutoRecovery::E_MODIFIED ) == 0,
                "unexpected document state" );
            Reference< XLoadable > xModelLoad( xModel, UNO_QUERY_THROW );
            xModelLoad->initNew();

            // TODO: remove load-process specific arguments from the descriptor, e.g. the status indicator
            xModel->attachResource( sURL, lDescriptor.getAsConstPropertyValueList() );
        }
        else
        {
            // let it recover itself
            Reference< XDocumentRecovery > xDocRecover( xModel, UNO_QUERY_THROW );
            xDocRecover->recoverFromFile(
                sURL,
                lDescriptor.getUnpackedValueOrDefault( utl::MediaDescriptor::PROP_SALVAGEDFILE(), OUString() ),
                lDescriptor.getAsConstPropertyValueList()
            );

            // No attachResource needed here. By definition (of XDocumentRecovery), the implementation is responsible
            // for completely initializing the model, which includes attachResource (or equivalent), if required.
        }

        // re-create all the views
        ::std::vector< OUString > aViewsToRestore( rInfo.ViewNames.getLength() );
        if ( rInfo.ViewNames.getLength() )
            ::std::copy( rInfo.ViewNames.begin(), rInfo.ViewNames.end(), aViewsToRestore.begin() );
        // if we don't have views for whatever reason, then create a default-view, at least
        if ( aViewsToRestore.empty() )
            aViewsToRestore.emplace_back( );

        for (auto const& viewToRestore : aViewsToRestore)
        {
            // create a frame
            Reference< XFrame > xTargetFrame = xDesktop->findFrame( SPECIALTARGET_BLANK, 0 );
            aCleanup.emplace_back(xTargetFrame.get() );

            // create a view to the document
            Reference< XController2 > xController;
            if ( viewToRestore.getLength() )
            {
                xController.set( xModel->createViewController( viewToRestore, Sequence< css::beans::PropertyValue >(), xTargetFrame ), UNO_SET_THROW );
            }
            else
            {
                xController.set( xModel->createDefaultViewController( xTargetFrame ), UNO_SET_THROW );
            }

            // introduce model/view/controller to each other
            xController->attachModel( xModel.get() );
            xModel->connectController( xController.get() );
            xTargetFrame->setComponent( xController->getComponentWindow(), xController.get() );
            xController->attachFrame( xTargetFrame );
            xModel->setCurrentController( xController.get() );
        }

        rInfo.Document = xModel.get();
    }
    catch(const css::uno::RuntimeException&)
    {
        throw;
    }
    catch(const css::uno::Exception&)
    {
        Any aCaughtException( ::cppu::getCaughtException() );

        // clean up
        for (auto const& component : aCleanup)
        {
            css::uno::Reference< css::util::XCloseable > xClose(component, css::uno::UNO_QUERY);
            if ( xClose.is() )
                xClose->close( true );
            else
                component->dispose();
        }

        // re-throw
        throw css::lang::WrappedTargetException(
            "Recovery of \"" + sURL + "\" failed.",
            static_cast< css::frame::XDispatch* >(this),
            aCaughtException
        );
    }
}

void AutoRecovery::implts_generateNewTempURL(const OUString&               sBackupPath     ,
                                                   utl::MediaDescriptor& /*rMediaDescriptor*/,
                                                   AutoRecovery::TDocumentInfo&   rInfo           )
{
    // specify URL for saving (which points to a temp file inside backup directory)
    // and define an unique name, so we can locate it later.
    // This unique name must solve an optimization problem too!
    // In case we are asked to save unmodified documents too - and one of them
    // is an empty one (because it was new created using e.g. an URL private:factory/...)
    // we should not save it really. Then we put the information about such "empty document"
    // into the configuration and don't create any recovery file on disk.
    // We use the title of the document to make it unique.
    OUStringBuffer sUniqueName;
    if (!rInfo.OrgURL.isEmpty())
    {
        css::uno::Reference< css::util::XURLTransformer > xParser(css::util::URLTransformer::create(m_xContext));
        css::util::URL aURL;
        aURL.Complete = rInfo.OrgURL;
        xParser->parseStrict(aURL);
        sUniqueName.append(aURL.Name);
    }
    else if (!rInfo.FactoryURL.isEmpty())
        sUniqueName.append("untitled");
    sUniqueName.append("_");

    // TODO: Must we strip some illegal signes - if we use the title?

    OUString sName(sUniqueName.makeStringAndClear());
    OUString sExtension(rInfo.Extension);
    OUString sPath(sBackupPath);
    ::utl::TempFile aTempFile(sName, true, &sExtension, &sPath);

    rInfo.NewTempURL = aTempFile.GetURL();
}

void AutoRecovery::implts_informListener(      sal_Int32                      eJob  ,
                                         const css::frame::FeatureStateEvent& aEvent)
{
    // Helper shares mutex with us -> threadsafe!
    ::cppu::OInterfaceContainerHelper* pListenerForURL = nullptr;
    OUString                           sJob            = AutoRecovery::implst_getJobDescription(eJob);

    // inform listener, which are registered for any URLs(!)
    pListenerForURL = m_lListener.getContainer(sJob);
    if(pListenerForURL != nullptr)
    {
        ::cppu::OInterfaceIteratorHelper pIt(*pListenerForURL);
        while(pIt.hasMoreElements())
        {
            try
            {
                css::uno::Reference< css::frame::XStatusListener > xListener(static_cast<css::frame::XStatusListener*>(pIt.next()), css::uno::UNO_QUERY);
                xListener->statusChanged(aEvent);
            }
            catch(const css::uno::RuntimeException&)
            {
                pIt.remove();
            }
        }
    }
}

OUString AutoRecovery::implst_getJobDescription(sal_Int32 eJob)
{
    // describe the current running operation
    OUStringBuffer sFeature(256);
    sFeature.append(CMD_PROTOCOL);

    // Attention: Because "eJob" is used as a flag field the order of checking these
    // flags is important. We must prefer job with higher priorities!
    // E.g. EmergencySave has an higher prio then AutoSave ...
    // On the other side there exist a well defined order between two different jobs.
    // e.g. PrepareEmergencySave must be done before EmergencySave is started of course.

    if ((eJob & AutoRecovery::E_PREPARE_EMERGENCY_SAVE) == AutoRecovery::E_PREPARE_EMERGENCY_SAVE)
        sFeature.append(CMD_DO_PREPARE_EMERGENCY_SAVE);
    else if ((eJob & AutoRecovery::E_EMERGENCY_SAVE) == AutoRecovery::E_EMERGENCY_SAVE)
        sFeature.append(CMD_DO_EMERGENCY_SAVE);
    else if ((eJob & AutoRecovery::E_RECOVERY) == AutoRecovery::E_RECOVERY)
        sFeature.append(CMD_DO_RECOVERY);
    else if ((eJob & AutoRecovery::E_SESSION_SAVE) == AutoRecovery::E_SESSION_SAVE)
        sFeature.append(CMD_DO_SESSION_SAVE);
    else if ((eJob & AutoRecovery::E_SESSION_QUIET_QUIT) == AutoRecovery::E_SESSION_QUIET_QUIT)
        sFeature.append(CMD_DO_SESSION_QUIET_QUIT);
    else if ((eJob & AutoRecovery::E_SESSION_RESTORE) == AutoRecovery::E_SESSION_RESTORE)
        sFeature.append(CMD_DO_SESSION_RESTORE);
    else if ((eJob & AutoRecovery::E_ENTRY_BACKUP) == AutoRecovery::E_ENTRY_BACKUP)
        sFeature.append(CMD_DO_ENTRY_BACKUP);
    else if ((eJob & AutoRecovery::E_ENTRY_CLEANUP) == AutoRecovery::E_ENTRY_CLEANUP)
        sFeature.append(CMD_DO_ENTRY_CLEANUP);
    else if ((eJob & AutoRecovery::E_AUTO_SAVE) == AutoRecovery::E_AUTO_SAVE)
        sFeature.append(CMD_DO_AUTO_SAVE);
    else if ( eJob != AutoRecovery::E_NO_JOB )
        SAL_INFO("fwk.autorecovery", "AutoRecovery::implst_getJobDescription(): Invalid job identifier detected.");

    return sFeature.makeStringAndClear();
}

sal_Int32 AutoRecovery::implst_classifyJob(const css::util::URL& aURL)
{
    if ( aURL.Protocol == CMD_PROTOCOL )
    {
        if ( aURL.Path == CMD_DO_PREPARE_EMERGENCY_SAVE )
            return AutoRecovery::E_PREPARE_EMERGENCY_SAVE;
        else if ( aURL.Path == CMD_DO_EMERGENCY_SAVE )
            return AutoRecovery::E_EMERGENCY_SAVE;
        else if ( aURL.Path == CMD_DO_RECOVERY )
            return AutoRecovery::E_RECOVERY;
        else if ( aURL.Path == CMD_DO_ENTRY_BACKUP )
            return AutoRecovery::E_ENTRY_BACKUP;
        else if ( aURL.Path == CMD_DO_ENTRY_CLEANUP )
            return AutoRecovery::E_ENTRY_CLEANUP;
        else if ( aURL.Path == CMD_DO_SESSION_SAVE )
            return AutoRecovery::E_SESSION_SAVE;
        else if ( aURL.Path == CMD_DO_SESSION_QUIET_QUIT )
            return AutoRecovery::E_SESSION_QUIET_QUIT;
        else if ( aURL.Path == CMD_DO_SESSION_RESTORE )
            return AutoRecovery::E_SESSION_RESTORE;
        else if ( aURL.Path == CMD_DO_DISABLE_RECOVERY )
            return AutoRecovery::E_DISABLE_AUTORECOVERY;
        else if ( aURL.Path == CMD_DO_SET_AUTOSAVE_STATE )
            return AutoRecovery::E_SET_AUTOSAVE_STATE;
    }

    SAL_INFO("fwk.autorecovery", "AutoRecovery::implts_classifyJob(): Invalid URL (protocol).");
    return AutoRecovery::E_NO_JOB;
}

css::frame::FeatureStateEvent AutoRecovery::implst_createFeatureStateEvent(      sal_Int32                    eJob      ,
                                                                           const OUString&             sEventType,
                                                                           AutoRecovery::TDocumentInfo const * pInfo     )
{
    css::frame::FeatureStateEvent aEvent;
    aEvent.FeatureURL.Complete   = AutoRecovery::implst_getJobDescription(eJob);
    aEvent.FeatureDescriptor     = sEventType;

    if (pInfo && sEventType == OPERATION_UPDATE)
    {
        // pack rInfo for transport via UNO
        ::comphelper::NamedValueCollection aInfo;
        aInfo.put( OUString(CFG_ENTRY_PROP_ID), pInfo->ID );
        aInfo.put( OUString(CFG_ENTRY_PROP_ORIGINALURL), pInfo->OrgURL );
        aInfo.put( OUString(CFG_ENTRY_PROP_FACTORYURL), pInfo->FactoryURL );
        aInfo.put( OUString(CFG_ENTRY_PROP_TEMPLATEURL), pInfo->TemplateURL );
        aInfo.put( OUString(CFG_ENTRY_PROP_TEMPURL), pInfo->OldTempURL.isEmpty() ? pInfo->NewTempURL : pInfo->OldTempURL );
        aInfo.put( OUString(CFG_ENTRY_PROP_MODULE), pInfo->AppModule);
        aInfo.put( OUString(CFG_ENTRY_PROP_TITLE), pInfo->Title);
        aInfo.put( OUString(CFG_ENTRY_PROP_VIEWNAMES), pInfo->ViewNames);
        aInfo.put( OUString(CFG_ENTRY_PROP_DOCUMENTSTATE), pInfo->DocumentState);

        aEvent.State <<= aInfo.getPropertyValues();
    }

    return aEvent;
}

void AutoRecovery::implts_resetHandleStates()
{
    CacheLockGuard aCacheLock(this, cppu::WeakComponentImplHelperBase::rBHelper.rMutex, m_nDocCacheLock, LOCK_FOR_CACHE_USE);

    /* SAFE */ {
    osl::ResettableMutexGuard g(cppu::WeakComponentImplHelperBase::rBHelper.rMutex);

    for (auto & info : m_lDocCache)
    {
        info.DocumentState &= ~AutoRecovery::E_HANDLED;
        info.DocumentState &= ~AutoRecovery::E_POSTPONED;

        // } /* SAFE */
        g.clear();
        implts_flushConfigItem(info);
        g.reset();
        // /* SAFE */ {
    }
    } /* SAFE */
}

void AutoRecovery::implts_prepareEmergencySave()
{
    // Be sure to know all open documents really .-)
    implts_verifyCacheAgainstDesktopDocumentList();

    // hide all docs, so the user can't disturb our emergency save .-)
    implts_changeAllDocVisibility(false);
}

void AutoRecovery::implts_doEmergencySave(const DispatchParams& aParams)
{
    // Write a hint "we crashed" into the configuration, so
    // the error report tool is started too in case no recovery
    // documents exists and was saved.

    std::shared_ptr<comphelper::ConfigurationChanges> batch(
            comphelper::ConfigurationChanges::create(m_xContext));
    officecfg::Office::Recovery::RecoveryInfo::Crashed::set(true, batch);
    batch->commit();

    // for all docs, store their current view/names in the configuration
    implts_persistAllActiveViewNames();

    // The called method for saving documents runs
    // during normal AutoSave more than once. Because
    // it postpone active documents and save it later.
    // That is normally done by recalling it from a timer.
    // Here we must do it immediately!
    // Of course this method returns the right state -
    // because it knows, that we are running in EMERGENCY SAVE mode .-)

    bool const bAllowUserIdleLoop = false; // not allowed to change that .-)
    AutoRecovery::ETimerType eSuggestedTimer    = AutoRecovery::E_DONT_START_TIMER;
    do
    {
        eSuggestedTimer = implts_saveDocs(bAllowUserIdleLoop, true, &aParams);
    }
    while(eSuggestedTimer == AutoRecovery::E_CALL_ME_BACK);

    // reset the handle state of all
    // cache items. Such handle state indicates, that a document
    // was already saved during the THIS(!) EmergencySave session.
    // Of course following recovery session must be started without
    // any "handle" state ...
    implts_resetHandleStates();

    // flush config cached back to disc.
    impl_flushALLConfigChanges();

    // try to make sure next time office will be started user won't be
    // notified about any other might be running office instance
    // remove ".lock" file from disc !
    AutoRecovery::st_impl_removeLockFile();
}

void AutoRecovery::implts_doRecovery(const DispatchParams& aParams)
{
    AutoRecovery::ETimerType eSuggestedTimer = AutoRecovery::E_DONT_START_TIMER;
    do
    {
        eSuggestedTimer = implts_openDocs(aParams);
    }
    while(eSuggestedTimer == AutoRecovery::E_CALL_ME_BACK);

    // reset the handle state of all
    // cache items. Such handle state indicates, that a document
    // was already saved during the THIS(!) Recovery session.
    // Of course a may be following EmergencySave session must be started without
    // any "handle" state ...
    implts_resetHandleStates();

    // Reset the configuration hint "we was crashed"!
    std::shared_ptr<comphelper::ConfigurationChanges> batch(
            comphelper::ConfigurationChanges::create(m_xContext));
    officecfg::Office::Recovery::RecoveryInfo::Crashed::set(false, batch);
    batch->commit();
}

void AutoRecovery::implts_doSessionSave(const DispatchParams& aParams)
{
    SAL_INFO("fwk.autorecovery", "AutoRecovery::implts_doSessionSave()");

    // Be sure to know all open documents really .-)
    implts_verifyCacheAgainstDesktopDocumentList();

    // for all docs, store their current view/names in the configuration
    implts_persistAllActiveViewNames();

    // The called method for saving documents runs
    // during normal AutoSave more than once. Because
    // it postpone active documents and save it later.
    // That is normally done by recalling it from a timer.
    // Here we must do it immediately!
    // Of course this method returns the right state -
    // because it knows, that we are running in SESSION SAVE mode .-)

    bool const bAllowUserIdleLoop = false; // not allowed to change that .-)
    AutoRecovery::ETimerType eSuggestedTimer    = AutoRecovery::E_DONT_START_TIMER;
    do
    {
        // do not remove lock files of the documents, it will be done on session quit
        eSuggestedTimer = implts_saveDocs(bAllowUserIdleLoop, false, &aParams);
    }
    while(eSuggestedTimer == AutoRecovery::E_CALL_ME_BACK);

    // reset the handle state of all
    // cache items. Such handle state indicates, that a document
    // was already saved during the THIS(!) save session.
    // Of course following restore session must be started without
    // any "handle" state ...
    implts_resetHandleStates();

    // flush config cached back to disc.
    impl_flushALLConfigChanges();
}

void AutoRecovery::implts_doSessionQuietQuit()
{
    SAL_INFO("fwk.autorecovery", "AutoRecovery::implts_doSessionQuietQuit()");

    // try to make sure next time office will be started user won't be
    // notified about any other might be running office instance
    // remove ".lock" file from disc!
    // it is done as a first action for session save since Gnome sessions
    // do not provide enough time for shutdown, and the dialog looks to be
    // confusing for the user
    AutoRecovery::st_impl_removeLockFile();

    // reset all modified documents, so the don't show any UI on closing ...
    // and close all documents, so we can shutdown the OS!
    implts_prepareSessionShutdown();

    // Write a hint for "stored session data" into the configuration, so
    // the on next startup we know what's happen last time
    std::shared_ptr<comphelper::ConfigurationChanges> batch(
            comphelper::ConfigurationChanges::create(m_xContext));
    officecfg::Office::Recovery::RecoveryInfo::SessionData::set(true, batch);
    batch->commit();

    // flush config cached back to disc.
    impl_flushALLConfigChanges();
}

void AutoRecovery::implts_doSessionRestore(const DispatchParams& aParams)
{
    SAL_INFO("fwk.autorecovery", "AutoRecovery::implts_doSessionRestore() ...");

    AutoRecovery::ETimerType eSuggestedTimer = AutoRecovery::E_DONT_START_TIMER;
    do
    {
        eSuggestedTimer = implts_openDocs(aParams);
    }
    while(eSuggestedTimer == AutoRecovery::E_CALL_ME_BACK);

    // reset the handle state of all
    // cache items. Such handle state indicates, that a document
    // was already saved during the THIS(!) Restore session.
    // Of course a may be following save session must be started without
    // any "handle" state ...
    implts_resetHandleStates();

    // make all opened documents visible
    implts_changeAllDocVisibility(true);

    // Reset the configuration hint for "session save"!
    SAL_INFO("fwk.autorecovery", "... reset config key 'SessionData'");
    std::shared_ptr<comphelper::ConfigurationChanges> batch(
            comphelper::ConfigurationChanges::create(m_xContext));
    officecfg::Office::Recovery::RecoveryInfo::SessionData::set(false, batch);
    batch->commit();

    SAL_INFO("fwk.autorecovery", "... AutoRecovery::implts_doSessionRestore()");
}

void AutoRecovery::implts_backupWorkingEntry(const DispatchParams& aParams)
{
    CacheLockGuard aCacheLock(this, cppu::WeakComponentImplHelperBase::rBHelper.rMutex, m_nDocCacheLock, LOCK_FOR_CACHE_USE);

    for (auto const& info : m_lDocCache)
    {
        if (info.ID != aParams.m_nWorkingEntryID)
            continue;

        OUString sSourceURL;
        // Prefer temp file. It contains the changes against the original document!
        if (!info.OldTempURL.isEmpty())
            sSourceURL = info.OldTempURL;
        else if (!info.NewTempURL.isEmpty())
            sSourceURL = info.NewTempURL;
        else if (!info.OrgURL.isEmpty())
            sSourceURL = info.OrgURL;
        else
            continue; // nothing real to save! An unmodified but new created document.

        INetURLObject aParser(sSourceURL);
        // AutoRecovery::EFailureSafeResult eResult =
        implts_copyFile(sSourceURL, aParams.m_sSavePath, aParser.getName());

        // TODO: Check eResult and react for errors (InteractionHandler!?)
        // Currently we ignore it ...
        // DON'T UPDATE THE CACHE OR REMOVE ANY TEMP. FILES FROM DISK.
        // That has to be forced from outside explicitly.
        // See implts_cleanUpWorkingEntry() for further details.
    }
}

void AutoRecovery::implts_cleanUpWorkingEntry(const DispatchParams& aParams)
{
    CacheLockGuard aCacheLock(this, cppu::WeakComponentImplHelperBase::rBHelper.rMutex, m_nDocCacheLock, LOCK_FOR_CACHE_ADD_REMOVE);

    AutoRecovery::TDocumentList::iterator pIt = std::find_if(m_lDocCache.begin(), m_lDocCache.end(),
        [&aParams](const AutoRecovery::TDocumentInfo& rInfo) { return rInfo.ID == aParams.m_nWorkingEntryID; });
    if (pIt != m_lDocCache.end())
    {
        AutoRecovery::TDocumentInfo& rInfo = *pIt;
        AutoRecovery::st_impl_removeFile(rInfo.OldTempURL);
        AutoRecovery::st_impl_removeFile(rInfo.NewTempURL);
        implts_flushConfigItem(rInfo, true); // sal_True => remove it from xml config!

        m_lDocCache.erase(pIt);
    }
}

AutoRecovery::EFailureSafeResult AutoRecovery::implts_copyFile(const OUString& sSource    ,
                                                               const OUString& sTargetPath,
                                                               const OUString& sTargetName)
{
    // create content for the parent folder and call transfer on that content with the source content
    // and the destination file name as parameters

    css::uno::Reference< css::ucb::XCommandEnvironment > xEnvironment;

    ::ucbhelper::Content aSourceContent;
    ::ucbhelper::Content aTargetContent;

    try
    {
        aTargetContent = ::ucbhelper::Content(sTargetPath, xEnvironment, m_xContext);
    }
    catch(const css::uno::Exception&)
    {
        return AutoRecovery::E_WRONG_TARGET_PATH;
    }

    sal_Int32 nNameClash;
    nNameClash = css::ucb::NameClash::RENAME;

    try
    {
        bool bSuccess = ::ucbhelper::Content::create(sSource, xEnvironment, m_xContext, aSourceContent);
        if (!bSuccess)
            return AutoRecovery::E_ORIGINAL_FILE_MISSING;
        aTargetContent.transferContent(aSourceContent, ::ucbhelper::InsertOperation::Copy, sTargetName, nNameClash);
    }
    catch(const css::uno::Exception&)
    {
        return AutoRecovery::E_ORIGINAL_FILE_MISSING;
    }

    return AutoRecovery::E_COPIED;
}

sal_Bool SAL_CALL AutoRecovery::convertFastPropertyValue(      css::uno::Any& /*aConvertedValue*/,
                                                               css::uno::Any& /*aOldValue*/      ,
                                                               sal_Int32      /*nHandle*/        ,
                                                         const css::uno::Any& /*aValue*/         )
{
    // not needed currently
    return false;
}

void SAL_CALL AutoRecovery::setFastPropertyValue_NoBroadcast(      sal_Int32      /*nHandle*/,
                                                             const css::uno::Any& /*aValue*/ )
{
    // not needed currently
}

void SAL_CALL AutoRecovery::getFastPropertyValue(css::uno::Any& aValue ,
                                                 sal_Int32      nHandle) const
{
    switch(nHandle)
    {
        case AUTORECOVERY_PROPHANDLE_EXISTS_RECOVERYDATA :
                {
                    bool bSessionData = officecfg::Office::Recovery::RecoveryInfo::SessionData::get(m_xContext);
                    bool bRecoveryData = !m_lDocCache.empty();

                    // exists session data ... => then we can't say, that these
                    // data are valid for recovery. So we have to return sal_False then!
                    if (bSessionData)
                        bRecoveryData = false;

                    aValue <<= bRecoveryData;
                }
                break;

        case AUTORECOVERY_PROPHANDLE_CRASHED :
                aValue <<= officecfg::Office::Recovery::RecoveryInfo::Crashed::get(m_xContext);
                break;

        case AUTORECOVERY_PROPHANDLE_EXISTS_SESSIONDATA :
                aValue <<= officecfg::Office::Recovery::RecoveryInfo::SessionData::get(m_xContext);
                break;
    }
}

const css::uno::Sequence< css::beans::Property > impl_getStaticPropertyDescriptor()
{
    const css::beans::Property pPropertys[] =
    {
        css::beans::Property( AUTORECOVERY_PROPNAME_CRASHED            , AUTORECOVERY_PROPHANDLE_CRASHED            , cppu::UnoType<bool>::get() , css::beans::PropertyAttribute::TRANSIENT | css::beans::PropertyAttribute::READONLY ),
        css::beans::Property( AUTORECOVERY_PROPNAME_EXISTS_RECOVERYDATA, AUTORECOVERY_PROPHANDLE_EXISTS_RECOVERYDATA, cppu::UnoType<bool>::get() , css::beans::PropertyAttribute::TRANSIENT | css::beans::PropertyAttribute::READONLY ),
        css::beans::Property( AUTORECOVERY_PROPNAME_EXISTS_SESSIONDATA , AUTORECOVERY_PROPHANDLE_EXISTS_SESSIONDATA , cppu::UnoType<bool>::get() , css::beans::PropertyAttribute::TRANSIENT | css::beans::PropertyAttribute::READONLY ),
    };
    const css::uno::Sequence< css::beans::Property > lPropertyDescriptor(pPropertys, AUTORECOVERY_PROPCOUNT);
    return lPropertyDescriptor;
}

::cppu::IPropertyArrayHelper& SAL_CALL AutoRecovery::getInfoHelper()
{
    static ::cppu::OPropertyArrayHelper ourInfoHelper(impl_getStaticPropertyDescriptor(), true);

    return ourInfoHelper;
}

css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL AutoRecovery::getPropertySetInfo()
{
    static css::uno::Reference< css::beans::XPropertySetInfo > xInfo(
                    ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper()));

    return xInfo;
}

void AutoRecovery::implts_verifyCacheAgainstDesktopDocumentList()
{
    SAL_INFO("fwk.autorecovery", "AutoRecovery::implts_verifyCacheAgainstDesktopDocumentList() ...");
    try
    {
        css::uno::Reference< css::frame::XDesktop2 > xDesktop = css::frame::Desktop::create(m_xContext);

        css::uno::Reference< css::container::XIndexAccess > xContainer(
            xDesktop->getFrames(),
            css::uno::UNO_QUERY_THROW);

        sal_Int32 i = 0;
        sal_Int32 c = xContainer->getCount();

        for (i=0; i<c; ++i)
        {
            css::uno::Reference< css::frame::XFrame > xFrame;
            try
            {
                xContainer->getByIndex(i) >>= xFrame;
                if (!xFrame.is())
                    continue;
            }
            // can happen in multithreaded environments, that frames was removed from the container during this loop runs!
            // Ignore it.
            catch(const css::lang::IndexOutOfBoundsException&)
            {
                continue;
            }

            // We are interested on visible documents only.
            // Note: It's n optional interface .-(
            css::uno::Reference< css::awt::XWindow2 > xVisibleCheck(
                xFrame->getContainerWindow(),
                css::uno::UNO_QUERY);
            if (
                (!xVisibleCheck.is()        ) ||
                (!xVisibleCheck->isVisible())
               )
            {
                continue;
            }

            // extract the model from the frame.
            // Ignore "view only" frames, which does not have a model.
            css::uno::Reference< css::frame::XController > xController;
            css::uno::Reference< css::frame::XModel >      xModel;

            xController = xFrame->getController();
            if (xController.is())
                xModel = xController->getModel();
            if (!xModel.is())
                continue;

            // insert model into cache ...
            // If the model is already well known inside cache
            // it's information set will be updated by asking the
            // model again for its new states.
            implts_registerDocument(xModel);
        }
    }
    catch(const css::uno::RuntimeException&)
    {
        throw;
    }
    catch(const css::uno::Exception&)
    {
    }

    SAL_INFO("fwk.autorecovery", "... AutoRecovery::implts_verifyCacheAgainstDesktopDocumentList()");
}

bool AutoRecovery::impl_enoughDiscSpace(sal_Int32 nRequiredSpace)
{
#ifdef SIMULATE_FULL_DISC
    return sal_False;
#else  // SIMULATE_FULL_DISC
    // In case an error occurs and we are not able to retrieve the needed information
    // it's better to "disable" the feature ShowErrorOnFullDisc !
    // Otherwise we start a confusing process of error handling ...

    sal_uInt64 nFreeSpace = SAL_MAX_UINT64;

    OUString     sBackupPath(SvtPathOptions().GetBackupPath());
    ::osl::VolumeInfo   aInfo      (osl_VolumeInfo_Mask_FreeSpace);
    ::osl::FileBase::RC aRC         = ::osl::Directory::getVolumeInfo(sBackupPath, aInfo);

    if (
        (aInfo.isValid(osl_VolumeInfo_Mask_FreeSpace)) &&
        (aRC == ::osl::FileBase::E_None         )
       )
    {
        nFreeSpace = aInfo.getFreeSpace();
    }

    sal_uInt64 nFreeMB = nFreeSpace/1048576;
    return (nFreeMB >= static_cast<sal_uInt64>(nRequiredSpace));
#endif // SIMULATE_FULL_DISC
}

void AutoRecovery::impl_showFullDiscError()
{
    OUString sBtn(FwkResId(STR_FULL_DISC_RETRY_BUTTON));
    OUString sMsg(FwkResId(STR_FULL_DISC_MSG));

    OUString sBackupURL(SvtPathOptions().GetBackupPath());
    INetURLObject aConverter(sBackupURL);
    sal_Unicode aDelimiter;
    OUString sBackupPath = aConverter.getFSysPath(FSysStyle::Detect, &aDelimiter);
    if (sBackupPath.getLength() < 1)
        sBackupPath = sBackupURL;

    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(nullptr,
                                              VclMessageType::Error, VclButtonsType::NONE,
                                              sMsg.replaceAll("%PATH", sBackupPath)));
    xBox->add_button(sBtn, RET_OK);
    xBox->run();
}

void AutoRecovery::impl_establishProgress(const AutoRecovery::TDocumentInfo&               rInfo    ,
                                                utl::MediaDescriptor&             rArgs    ,
                                          const css::uno::Reference< css::frame::XFrame >& xNewFrame)
{
    // external well known frame must be preferred (because it was created by ourself
    // for loading documents into this frame)!
    // But if no frame exists... we can try to locate it using any frame bound to the provided
    // document. Of course we must live without any frame in case the document does not exists at this
    // point. But this state should not occur. In such case xNewFrame should be valid ... hopefully .-)
    css::uno::Reference< css::frame::XFrame > xFrame = xNewFrame;
    if (
        (!xFrame.is()       ) &&
        (rInfo.Document.is())
       )
    {
        css::uno::Reference< css::frame::XController > xController = rInfo.Document->getCurrentController();
        if (xController.is())
            xFrame = xController->getFrame();
    }

    // Any outside progress must be used ...
    // Only if there is no progress, we can create our own one.
    css::uno::Reference< css::task::XStatusIndicator > xInternalProgress;
    css::uno::Reference< css::task::XStatusIndicator > xExternalProgress = rArgs.getUnpackedValueOrDefault(
                                                                                utl::MediaDescriptor::PROP_STATUSINDICATOR(),
                                                                                css::uno::Reference< css::task::XStatusIndicator >() );

    // Normally a progress is set from outside (e.g. by the CrashSave/Recovery dialog, which uses our dispatch API).
    // But for a normal auto save we don't have such "external progress"... because this function is triggered by our own timer then.
    // In such case we must create our own progress !
    if (
        (! xExternalProgress.is()) &&
        (xFrame.is()             )
       )
    {
        css::uno::Reference< css::task::XStatusIndicatorFactory > xProgressFactory(xFrame, css::uno::UNO_QUERY);
        if (xProgressFactory.is())
            xInternalProgress = xProgressFactory->createStatusIndicator();
    }

    // HACK
    // An external provided progress (most given by the CrashSave/Recovery dialog)
    // must be preferred. But we know that some application filters query its own progress instance
    // at the frame method Frame::createStatusIndicator().
    // So we use a two step mechanism:
    // 1) we set the progress inside the MediaDescriptor, which will be provided to the filter
    // 2) and we set a special Frame property, which overwrites the normal behaviour of Frame::createStatusIndicator .-)
    // But we suppress 2) in case we uses an internal progress. Because then it doesn't matter
    // if our applications make it wrong. In such case the internal progress resists at the same frame
    // and there is no need to forward progress activities to e.g. an outside dialog .-)
    if (
        (xExternalProgress.is()) &&
        (xFrame.is()           )
       )
    {
        css::uno::Reference< css::beans::XPropertySet > xFrameProps(xFrame, css::uno::UNO_QUERY);
        if (xFrameProps.is())
            xFrameProps->setPropertyValue(FRAME_PROPNAME_ASCII_INDICATORINTERCEPTION, css::uno::makeAny(xExternalProgress));
    }

    // But inside the MediaDescriptor we must set our own create progress ...
    // in case there is not already another progress set.
    rArgs.createItemIfMissing(utl::MediaDescriptor::PROP_STATUSINDICATOR(), xInternalProgress);
}

void AutoRecovery::impl_forgetProgress(const AutoRecovery::TDocumentInfo&               rInfo    ,
                                             utl::MediaDescriptor&             rArgs    ,
                                       const css::uno::Reference< css::frame::XFrame >& xNewFrame)
{
    // external well known frame must be preferred (because it was created by ourself
    // for loading documents into this frame)!
    // But if no frame exists... we can try to locate it using any frame bound to the provided
    // document. Of course we must live without any frame in case the document does not exists at this
    // point. But this state should not occur. In such case xNewFrame should be valid ... hopefully .-)
    css::uno::Reference< css::frame::XFrame > xFrame = xNewFrame;
    if (
        (!xFrame.is()       ) &&
        (rInfo.Document.is())
       )
    {
        css::uno::Reference< css::frame::XController > xController = rInfo.Document->getCurrentController();
        if (xController.is())
            xFrame = xController->getFrame();
    }

    // stop progress interception on corresponding frame.
    css::uno::Reference< css::beans::XPropertySet > xFrameProps(xFrame, css::uno::UNO_QUERY);
    if (xFrameProps.is())
        xFrameProps->setPropertyValue(FRAME_PROPNAME_ASCII_INDICATORINTERCEPTION, css::uno::makeAny(css::uno::Reference< css::task::XStatusIndicator >()));

    // forget progress inside list of arguments.
    utl::MediaDescriptor::iterator pArg = rArgs.find(utl::MediaDescriptor::PROP_STATUSINDICATOR());
    if (pArg != rArgs.end())
    {
        rArgs.erase(pArg);
        pArg = rArgs.end();
    }
}

void AutoRecovery::impl_flushALLConfigChanges()
{
    try
    {
        // SOLAR SAFE ->
        SolarMutexGuard aGuard;
        ::utl::ConfigManager::storeConfigItems();
    }
    catch(const css::uno::Exception&)
    {
    }
}

void AutoRecovery::st_impl_removeFile(const OUString& sURL)
{
    if ( sURL.isEmpty())
        return;

    try
    {
        ::ucbhelper::Content aContent = ::ucbhelper::Content(sURL, css::uno::Reference< css::ucb::XCommandEnvironment >(), m_xContext);
        aContent.executeCommand("delete", css::uno::makeAny(true));
    }
    catch(const css::uno::Exception&)
    {
    }
}

void AutoRecovery::st_impl_removeLockFile()
{
    try
    {
        OUString sUserURL;
        ::utl::Bootstrap::locateUserInstallation( sUserURL );

        OUStringBuffer sLockURLBuf;
        sLockURLBuf.append     (sUserURL);
        sLockURLBuf.append("/.lock");
        OUString sLockURL = sLockURLBuf.makeStringAndClear();

        AutoRecovery::st_impl_removeFile(sLockURL);
    }
    catch(const css::uno::Exception&)
    {
    }
}

struct Instance {
    explicit Instance(
        css::uno::Reference<css::uno::XComponentContext> const & context):
        instance(
            static_cast<cppu::OWeakObject *>(new AutoRecovery(context)))
    {
        // 2nd phase initialization needed
        static_cast<AutoRecovery*>(static_cast<cppu::OWeakObject *>
                (instance.get()))->initListeners();
    }

    css::uno::Reference<css::uno::XInterface> instance;
};

struct Singleton:
    public rtl::StaticWithArg<
        Instance, css::uno::Reference<css::uno::XComponentContext>, Singleton>
{};

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_AutoRecovery_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(static_cast<cppu::OWeakObject *>(
                Singleton::get(context).instance.get()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

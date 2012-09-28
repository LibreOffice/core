/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __FRAMEWORK_SERVICES_AUTORECOVERY_HXX_
#define __FRAMEWORK_SERVICES_AUTORECOVERY_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <general.h>
#include <stdtypes.h>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

#include <comphelper/mediadescriptor.hxx>
#include <vcl/timer.hxx>
#include <vcl/evntpost.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/weak.hxx>

//_______________________________________________
// definition

#ifndef css
namespace css = ::com::sun::star;
#endif

namespace framework
{

//---------------------------------------
/** @short  hold all needed informations for an asynchronous dispatch alive.

    @descr  Because some operations are forced to be executed asynchronously
            (e.g. requested by our CreashSave/Recovery dialog) ... we must make sure
            that these informations wont be set as "normal" members of our AtoRecovery
            instance. Otherwise they can disturb our normal AutoSave-timer handling.
            e.g. it can be unclear then, which progress has to be used for storing documents ...
 */
struct DispatchParams
{
    public:
         DispatchParams();
         DispatchParams(const ::comphelper::SequenceAsHashMap&             lArgs ,
                        const css::uno::Reference< css::uno::XInterface >& xOwner);
         DispatchParams(const DispatchParams& rCopy);
        ~DispatchParams();

         DispatchParams& operator=(const DispatchParams& rCopy);
         void forget();

    public:

        //---------------------------------------
        /** @short  can be set from outside and is provided to
                    our internal started operations.

            @descr  Normaly we use the normal status indicator
                    of the document windows to show a progress.
                    But in case we are used by any special UI,
                    it can provide its own status indicator object
                    to us - so we use it instead of the normal one.
         */
        css::uno::Reference< css::task::XStatusIndicator > m_xProgress;

        //---------------------------------------
        /** TODO document me */
        ::rtl::OUString m_sSavePath;

        //---------------------------------------
        /** @short  define the current cache entry, which should be used for current
                    backup or cleanUp operation ... which is may be done asynchronous */
        sal_Int32 m_nWorkingEntryID;

        //---------------------------------------
        /** @short  used for asyncoperations, to prevent us from dying.

            @descr  If our dispatch() method was forced to start the
                    internal operation asynchronous ... we send an event
                    to start and return immediatly. But we must be shure that
                    our instance live if the event callback reach us.
                    So we hold an uno reference to ourself.
         */
        css::uno::Reference< css::uno::XInterface > m_xHoldRefForAsyncOpAlive;
};

//_______________________________________________
/**
    implements the functionality of AutoSave and AutoRecovery
    of documents - including features of an EmergencySave in
    case a GPF occures.
 */
class AutoRecovery  : public  css::lang::XTypeProvider
                    , public  css::lang::XServiceInfo
                    , public  css::frame::XDispatch
                    , public  css::document::XEventListener         // => css.lang.XEventListener
                    , public  css::util::XChangesListener           // => css.lang.XEventListener
                    , public  css::util::XModifyListener            // => css.lang.XEventListener
                    // attention! Must be the first base class to guarentee right initialize lock ...
                    , private ThreadHelpBase
                    , public  ::cppu::OBroadcastHelper
                    , public  ::cppu::OPropertySetHelper            // => XPropertySet, XFastPropertySet, XMultiPropertySet
                    , public  ::cppu::OWeakObject
{
    //___________________________________________
    // types

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

            /// the Auto/Emergency saved document isnt useable any longer
            E_DAMAGED = 64,
            /// the Auto/Emergency saved document isnt realy up-to-date (some changes can be missing)
            E_INCOMPLETE = 128,
            /// the Auto/Emergency saved document was processed successfully
            E_SUCCEDED = 512
        };

        /** @short  indicates the results of a FAILURE_SAFE operation

            @descr  We must know, which reason was the real one in case
                    we couldnt copy a "failure document" to a user specified path.
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
            /** the timer shouldnt be used next time */
            E_DONT_START_TIMER,
            /** timer (was/must be) started with normal AutoSaveTimeIntervall */
            E_NORMAL_AUTOSAVE_INTERVALL,
            /** timer must be started with special short time intervall,
                to poll for an user idle period */
            E_POLL_FOR_USER_IDLE,
            /** timer mst be started with a very(!) short time intervall,
                to poll for the end of an user action, which does not allow saving documents in general */
            E_POLL_TILL_AUTOSAVE_IS_ALLOWED,
            /** dont start the timer - but calls the same action then before immediatly again! */
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
            E_SESSION_QUIET_QUIT        = 1024
        };

        //---------------------------------------
        /** @short  combine different informations about one office document. */
        struct TDocumentInfo
        {
            public:

                //-------------------------------
                TDocumentInfo()
                    : DocumentState   (E_UNKNOWN)
                    , UsedForSaving   (sal_False)
                    , ListenForModify (sal_False)
                    , IgnoreClosing   (sal_False)
                    , ID              (-1       )
                {}

                //-------------------------------
                /** @short  points to the document. */
                css::uno::Reference< css::frame::XModel > Document;

                //-------------------------------
                /** @short  knows, if the document is realy modified since the last autosave,
                            or  was postponed, because it was an active one etcpp...

                    @descr  Because we have no CHANGE TRACKING mechanism, based on office document,
                            we implements it by ourself. We listen for MODIFIED events
                            of each document and update this state flag here.

                            Further we postpone saving of active documents, e.g. if the user
                            works currently on it. We wait for an idle period then ...
                 */
                sal_Int32 DocumentState;

                //-------------------------------
                /** Because our applications not ready for concurrent save requests at the same time,
                    we have supress our own AutoSave for the moment, a document will be already saved
                    by others.
                 */
                sal_Bool UsedForSaving;

                //-------------------------------
                /** For every user action, which modifies a document (e.g. key input) we get
                    a notification as XModifyListener. That seams to be a "performance issue" .-)
                    So we decided to listen for such modify events only for the time in which the document
                    was stored as temp. file and was not modified again by the user.
                */
                sal_Bool ListenForModify;

                //-------------------------------
                /** For SessionSave we must close all open documents by ourself.
                    But because we are listen for documents events, we get some ...
                    and deregister these documents from our configuration.
                    That's why we mark these documents as "Closed by ourself" so we can
                    ignore these "OnUnload" or disposing() events .-)
                */
                sal_Bool IgnoreClosing;

                //-------------------------------
                /** TODO: document me */
                ::rtl::OUString OrgURL;
                ::rtl::OUString FactoryURL;
                ::rtl::OUString TemplateURL;

                ::rtl::OUString OldTempURL;
                ::rtl::OUString NewTempURL;

                ::rtl::OUString AppModule;      // e.g. com.sun.star.text.TextDocument - used to identify app module
                ::rtl::OUString FactoryService; // the service to create a document of the module
                ::rtl::OUString RealFilter;     // real filter, which was used at loading time
                ::rtl::OUString DefaultFilter;  // supports saving of the default format without loosing data
                ::rtl::OUString Extension;      // file extension of the default filter
                ::rtl::OUString Title;          // can be used as "DisplayName" on every recovery UI!
                ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                ViewNames;      // names of the view which were active at emergency-save time

                sal_Int32 ID;
        };

        //---------------------------------------
        /** @short  used to know every currently open document. */
        typedef ::std::vector< TDocumentInfo > TDocumentList;

    //___________________________________________
    // member

    private:

        //---------------------------------------
        /** @short  the global uno service manager.
            @descr  Must be used to create own needed services.
         */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        //---------------------------------------
        /** @short  points to the underlying recovery configuration.
            @descr  This instance does not cache - it calls directly the
                    configuration API!
          */
        css::uno::Reference< css::container::XNameAccess > m_xRecoveryCFG;

        //---------------------------------------
        /** @short  proxy weak binding to forward Events to ourself without
                    an ownership cycle
          */
        css::uno::Reference< css::util::XChangesListener > m_xRecoveryCFGListener;

        //---------------------------------------
        /** @short  points to the used configuration package or.openoffice.Setup
            @descr  This instance does not cache - it calls directly the
                    configuration API!
          */
        css::uno::Reference< css::container::XNameAccess > m_xModuleCFG;

        //---------------------------------------
        /** @short  holds the global event broadcaster alive,
                    where we listen for new created documents.
          */
        css::uno::Reference< css::document::XEventBroadcaster > m_xNewDocBroadcaster;

        //---------------------------------------
        /** @short  proxy weak binding to forward Events to ourself without
                    an ownership cycle
          */
        css::uno::Reference< css::document::XEventListener > m_xNewDocBroadcasterListener;

        //---------------------------------------
        /** @short  because we stop/restart listening sometimes, it's a good idea to know
                    if we already registered as listener .-)
        */
        sal_Bool m_bListenForDocEvents;
        sal_Bool m_bListenForConfigChanges;

        //---------------------------------------
        /** @short  specify the time intervall between two save actions.
            @descr  Time is measured in [min].
         */
        sal_Int32 m_nAutoSaveTimeIntervall;

        //---------------------------------------
        /** @short  for an asynchronous operation we must know, if there is
                    at least one running job (may be asynchronous!).
         */
        sal_Int32 m_eJob;

        //---------------------------------------
        /** @short  the timer, which is used to be informed about the next
                    saving time ...
         */
        Timer m_aTimer;

        //---------------------------------------
        /** @short  make our dispatch asynchronous ... if required to do so! */
        ::vcl::EventPoster m_aAsyncDispatcher;

        //---------------------------------------
        /** @see    DispatchParams
         */
        DispatchParams m_aDispatchParams;

        //---------------------------------------
        /** @short  indicates, which time period is currently used by the
                    internal timer.
         */
        ETimerType m_eTimerType;

        //---------------------------------------
        /** @short  this cache is used to hold all informations about
                    recovery/emergency save documents alive.
         */
        TDocumentList m_lDocCache;

        //---------------------------------------
        // TODO document me
        sal_Int32 m_nIdPool;

        //---------------------------------------
        /** @short  contains all status listener registered at this instance.
         */
        ListenerHash m_lListener;

        /** @descr  This member is used to prevent us against re-entrance problems.
                    A mutex cant help to prevent us from concurrent using of members
                    inside the same thread. But e.g. our internaly used stl structures
                    are not threadsafe ... and furthermore they cant be used at the same time
                    for iteration and add/remove requests!
                    So we have to detect such states and ... show a warning.
                    May be there will be a better solution next time ... (copying the cache temp.
                    bevor using).

                    And further it's not possible to use a simple boolean value here.
                    Because if more then one operation iterates over the same stl container ...
                    (only to modify it's elements but dont add new or removing existing ones!)
                    it should be possible doing so. But we must guarantee that the last operation reset
                    this lock ... not the first one ! So we use a "ref count" mechanism for that."
         */
        sal_Int32 m_nDocCacheLock;

        /** @descr  These members are used to check the minimum disc space, which must exists
                    to start the corresponding operation.
         */
        sal_Int32 m_nMinSpaceDocSave;
        sal_Int32 m_nMinSpaceConfigSave;

        //---------------------------------------
        /** @short  special debug option to make testing faster.

            @descr  We dont interpret the timer unit as [min] ...
                    we use [ms] instead of that. Further we dont
                    wait 10 s for user idle ...
         */
        #if OSL_DEBUG_LEVEL > 1
        sal_Bool m_dbg_bMakeItFaster;
        #endif

        //---------------------------------------
        // HACK ... TODO
        css::uno::Reference< css::task::XStatusIndicator > m_xExternalProgress;

    //___________________________________________
    // interface

    public:

                 AutoRecovery(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);
        virtual ~AutoRecovery(                                                                   );

        // XInterface, XTypeProvider, XServiceInfo
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        //---------------------------------------
        // css.frame.XDispatch
        virtual void SAL_CALL dispatch(const css::util::URL&                                  aURL      ,
                                       const css::uno::Sequence< css::beans::PropertyValue >& lArguments)
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL addStatusListener(const css::uno::Reference< css::frame::XStatusListener >& xListener,
                                                const css::util::URL&                                     aURL     )
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL removeStatusListener(const css::uno::Reference< css::frame::XStatusListener >& xListener,
                                                   const css::util::URL&                                     aURL     )
            throw(css::uno::RuntimeException);

        //---------------------------------------
        // css.document.XEventListener
        /** @short  informs about created/opened documents.

            @descr  Every new opened/created document will be saved internaly
                    so it can be checked if its modified. This modified state
                    is used later to decide, if it must be saved or not.

            @param  aEvent
                    points to the new created/opened document.
         */
        virtual void SAL_CALL notifyEvent(const css::document::EventObject& aEvent)
            throw(css::uno::RuntimeException);

        //---------------------------------------
        // css.util.XChangesListener
        virtual void SAL_CALL changesOccurred(const css::util::ChangesEvent& aEvent)
            throw(css::uno::RuntimeException);

        //---------------------------------------
        // css.util.XModifyListener
        virtual void SAL_CALL modified(const css::lang::EventObject& aEvent)
            throw(css::uno::RuntimeException);

        //---------------------------------------
        // css.lang.XEventListener
        using cppu::OPropertySetHelper::disposing;
        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent)
            throw(css::uno::RuntimeException);

    //___________________________________________
    // helper

    protected:

        //---------------------------------------
        // OPropertySetHelper

        virtual sal_Bool SAL_CALL convertFastPropertyValue(      css::uno::Any& aConvertedValue,
                                                                 css::uno::Any& aOldValue      ,
                                                                 sal_Int32      nHandle        ,
                                                           const css::uno::Any& aValue         )
            throw(css::lang::IllegalArgumentException);

        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(      sal_Int32      nHandle,
                                                               const css::uno::Any& aValue )
            throw(css::uno::Exception);
        using cppu::OPropertySetHelper::getFastPropertyValue;
        virtual void SAL_CALL getFastPropertyValue(css::uno::Any& aValue ,
                                                   sal_Int32      nHandle) const;

        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
            throw(css::uno::RuntimeException);
    //___________________________________________
    // helper

    private:

        //---------------------------------------
        /** @short  open the underlying configuration.

            @descr  This method must be called everytimes
                    a configuartion call is needed. Because
                    method works together with the member
                    m_xCFG, open it on demand and cache it
                    afterwards.

            @return [com.sun.star.container.XNameAccess]
                    the configuration object

            @throw  [com.sun.star.uno.RuntimeException]
                    if config could not be opened successfully!

            @threadsafe
          */
        css::uno::Reference< css::container::XNameAccess > implts_openConfig();

        //---------------------------------------
        /** @short  read the underlying configuration.

            @descr  After that we know the initial state - means:
                    - if AutoSave was enabled by the user
                    - which time intervall has to be used
                    - which recovery entries may already exists

            @throw  [com.sun.star.uno.RuntimeException]
                    if config could not be opened or readed successfully!

            @threadsafe
          */
        void implts_readConfig();

        //---------------------------------------
        /** @short  read the underlying configuration...

            @descr  ... but only keys related to the AutoSave mechanism.
                    Means: State and Timer intervall.
                    E.g. the recovery list isnt adressed here.

            @throw  [com.sun.star.uno.RuntimeException]
                    if config could not be opened or readed successfully!

            @threadsafe
          */
        void implts_readAutoSaveConfig();

        //---------------------------------------
        // TODO document me
        void implts_flushConfigItem(const AutoRecovery::TDocumentInfo& rInfo                ,
                                          sal_Bool                     bRemoveIt = sal_False);

        //---------------------------------------
        // TODO document me
        void implts_startListening();
        void implts_startModifyListeningOnDoc(AutoRecovery::TDocumentInfo& rInfo);

        //---------------------------------------
        // TODO document me
        void implts_stopListening();
        void implts_stopModifyListeningOnDoc(AutoRecovery::TDocumentInfo& rInfo);

        //---------------------------------------
        /** @short  stops and may be(!) restarts the timer.

            @descr  A running timer is stopped everytimes here.
                    But starting depends from the different internal
                    timer variables (e.g. AutoSaveEnabled, AutoSaveTimeIntervall,
                    TimerType etcpp.)

            @throw  [com.sun.star.uno.RuntimeException]
                    if timer could not be stopped or started!

            @threadsafe
         */
        void implts_updateTimer();

        //---------------------------------------
        /** @short  stop the timer.

            @descr  Double calls will be ignored - means we do
                    nothing here, if the timer is already disabled.

            @throw  [com.sun.star.uno.RuntimeException]
                    if timer could not be stopped!

            @threadsafe
         */
        void implts_stopTimer();

        //---------------------------------------
        /** @short  callback of our internal timer.
         */
        DECL_LINK(implts_timerExpired, void*);

        //---------------------------------------
        /** @short  makes our dispatch() method asynchronous!
         */
        DECL_LINK(implts_asyncDispatch, void*);

        //---------------------------------------
        /** @short  implements the dispatch real. */
        void implts_dispatch(const DispatchParams& aParams);

        //---------------------------------------
        /** @short  validate new detected document and add it into the internal
                    document list.

            @descr  This method should be called only, if its clear that a new
                    document was opened/created during office runtime.
                    This method checks, if its a top level document (means not an embedded one).
                    Only such top level documents can be recognized by this auto save mechanism.

            @param  xDocument
                    the new document, which should be checked and registered.

            @threadsafe
         */
        void implts_registerDocument(const css::uno::Reference< css::frame::XModel >& xDocument);

        //---------------------------------------
        /** @short  remove the specified document from our internal document list.

            @param  xDocument
                    the new document, which should be deregistered.

            @param  bStopListening
                    sal_False: must be used in case this method is called withion disposing() of the document,
                           where it make no sense to deregister our listener. The container dies ...
                    sal_True : must be used in case this method is used on "dergistration" of this document, where
                           we must deregister our listener .-)

            @threadsafe
         */
        void implts_deregisterDocument(const css::uno::Reference< css::frame::XModel >& xDocument                ,
                                             sal_Bool                                   bStopListening = sal_True);

        //---------------------------------------
        // TODO document me
        void implts_markDocumentModifiedAgainstLastBackup(const css::uno::Reference< css::frame::XModel >& xDocument);

        //---------------------------------------
        // TODO document me
        void implts_updateModifiedState(const css::uno::Reference< css::frame::XModel >& xDocument);

        //---------------------------------------
        // TODO document me
        void implts_updateDocumentUsedForSavingState(const css::uno::Reference< css::frame::XModel >& xDocument      ,
                                                           sal_Bool                                   bSaveInProgress);

        //---------------------------------------
        // TODO document me
        void implts_markDocumentAsSaved(const css::uno::Reference< css::frame::XModel >& xDocument);

        //---------------------------------------
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

        //---------------------------------------
        /** TODO document me */
        void implts_changeAllDocVisibility(sal_Bool bVisible);
        void implts_prepareSessionShutdown();

        //---------------------------------------
        /** @short  save all current opened documents to a specific
                    backup directory.

            @descr  Only realy changed documents will be saved here.

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
                                 we dont wish to disturb the user on it's work.
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

            @return A suggestion, how the timer (if its not already disabled!)
                    should be restarted to full fill the requirements.

            @threadsafe
         */
        AutoRecovery::ETimerType implts_saveDocs(      sal_Bool        bAllowUserIdleLoop,
                                                       sal_Bool        bRemoveLockFiles,
                                                 const DispatchParams* pParams        = 0);

        //---------------------------------------
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

                    Note further: It paches the info struct
                    more then ones. E.g. the new temp URL is set
                    before the file is saved. And the old URL is removed
                    only if removing oft he old file was successfully.
                    If this method returns without an exception - everything
                    was OK. Otherwhise the info struct can be analyzed to
                    get more information, e.g. when the problem occures.

            @param  sBackupPath
                    the base path for saving such temp files.

            @param  rInfo
                    points to an informations structure, where
                    e.g. the document, its modified state, the count
                    of autosave-retries etcpp. exists.
                    Its used also to return the new temp file name
                    and some other state values!

            @threadsafe
          */
        void implts_saveOneDoc(const ::rtl::OUString&                                    sBackupPath      ,
                                     AutoRecovery::TDocumentInfo&                        rInfo            ,
                               const css::uno::Reference< css::task::XStatusIndicator >& xExternalProgress);

        //---------------------------------------
        /** @short  recovery all documents, which was saved during
                    a crash before.

            @return A suggestion, how this method must be called back!

            @threadsafe
         */
        AutoRecovery::ETimerType implts_openDocs(const DispatchParams& aParams);

        //---------------------------------------
        // TODO document me
        void implts_openOneDoc(const ::rtl::OUString&               sURL       ,
                                     ::comphelper::MediaDescriptor& lDescriptor,
                                     AutoRecovery::TDocumentInfo&   rInfo      );

        //---------------------------------------
        // TODO document me
        void implts_generateNewTempURL(const ::rtl::OUString&               sBackupPath     ,
                                             ::comphelper::MediaDescriptor& rMediaDescriptor,
                                             AutoRecovery::TDocumentInfo&   rInfo           );

        //---------------------------------------
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

        //---------------------------------------
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
                    and is used to send informations regarding the current handled document.

            @return [css::frame::FeatureStateEvent]
                    the event structure for sending.
         */
        static css::frame::FeatureStateEvent implst_createFeatureStateEvent(      sal_Int32                    eJob      ,
                                                                            const ::rtl::OUString&             sEventType,
                                                                                  AutoRecovery::TDocumentInfo* pInfo     );


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

        //---------------------------------------

        // TODO document me
        void implts_resetHandleStates(sal_Bool bLoadCache);

        //---------------------------------------
        // TODO document me
        void implts_specifyDefaultFilterAndExtension(AutoRecovery::TDocumentInfo& rInfo);

        //---------------------------------------
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

        //---------------------------------------
        // TODO document me
        void implts_prepareEmergencySave();

        //---------------------------------------
        // TODO document me
        void implts_doEmergencySave(const DispatchParams& aParams);

        //---------------------------------------
        // TODO document me
        void implts_doRecovery(const DispatchParams& aParams);

        //---------------------------------------
        // TODO document me
        void implts_doSessionSave(const DispatchParams& aParams);

        //---------------------------------------
        // TODO document me
        void implts_doSessionQuietQuit(const DispatchParams& aParams);

        //---------------------------------------
        // TODO document me
        void implts_doSessionRestore(const DispatchParams& aParams);

        //---------------------------------------
        // TODO document me
        void implts_backupWorkingEntry(const DispatchParams& aParams);

        //---------------------------------------
        // TODO document me
        void implts_cleanUpWorkingEntry(const DispatchParams& aParams);

        //---------------------------------------
        /** try to make sure that all changed config items (not our used
            config access only) will be flushed back to disc.

            E.g. our svtools::ConfigItems() has to be flushed explicitly .-(

            Note: This method cant fail. Flushing of config entries is an
                  optional feature. Errors can be ignored.
         */
        void impl_flushALLConfigChanges();

        //---------------------------------------
        // TODO document me
        AutoRecovery::EFailureSafeResult implts_copyFile(const ::rtl::OUString& sSource    ,
                                                         const ::rtl::OUString& sTargetPath,
                                                         const ::rtl::OUString& sTargetName);

        //---------------------------------------
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
        static ::rtl::OUString implst_getJobDescription(sal_Int32 eJob);

        //---------------------------------------
        /** @short  mape the given URL to an internal int representation.

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
        sal_Bool impl_enoughDiscSpace(sal_Int32 nRequiredSpace);

        /// TODO document me
        static void impl_showFullDiscError();

        //---------------------------------------
        /** @short  try to create/use a progress and set it inside the
                    environment.

            @descr  The problem behind: There exists different use case of this method.
                    a) An external progress is provided by our CrashSave or Recovery dialog.
                    b) We must create our own progress e.g. for an AutoSave
                    c) Sometimes our application filters dont use the progress
                       provided by the MediaDescriptor. They uses the Frame everytime to create
                       it's own progress. So we implemented a HACK for these and now we set
                       an InterceptedProgress there for the time WE use this frame for loading/storing documents .-)

            @param  xNewFrame
                    must be set only in case WE create a new frame (e.g. for loading documents
                    on session restore or recovery). Then search for a frame using rInfo.Document must
                    be supressed and xFrame must be preferred instead .-)

            @param  rInfo
                    used e.g. to find the frame corresponding to a document.
                    This frame must be used to create a new progress e.g. for an AutoSave.

            @param  rArgs
                    is used to set the new created progress as parameter on these set.
         */
        void impl_establishProgress(const AutoRecovery::TDocumentInfo&               rInfo    ,
                                          ::comphelper::MediaDescriptor&             rArgs    ,
                                    const css::uno::Reference< css::frame::XFrame >& xNewFrame);

        void impl_forgetProgress(const AutoRecovery::TDocumentInfo&               rInfo    ,
                                       ::comphelper::MediaDescriptor&             rArgs    ,
                                 const css::uno::Reference< css::frame::XFrame >& xNewFrame);

        //---------------------------------------
        /** try to remove the specified file from disc.

            Every URL supported by our UCB component can be used here.
            Further it doesnt matter if the file realy exists or not.
            Because removing a non exsistent file will have the same
            result at the end ... a non existing file .-)

            On the other side removing of files from disc is an optional
            feature. If we are not able doing so ... its not a real problem.
            Ok - users disc place will be samller then ... but we should produce
            a crash during crash save because we cant delete a temporary file only !

            @param  sURL
                    the url of the file, which should be removed.
         */
        void st_impl_removeFile(const ::rtl::OUString& sURL);

        //---------------------------------------
        /** try to remove ".lock" file from disc if office will be terminated
            not using the offical way .-)

            This method has to be handled "optional". So every error inside
            has to be ignored ! This method CANT FAIL ... it can forget something only .-)
         */
        void st_impl_removeLockFile();
};

} // namespace framework

#endif // __FRAMEWORK_SERVICES_AUTORECOVERY_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

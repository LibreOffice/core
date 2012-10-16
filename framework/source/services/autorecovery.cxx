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


#include "services/autorecovery.hxx"
#include <loadenv/loadenv.hxx>

#include <loadenv/targethelper.hxx>
#include <pattern/frame.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>

#include <classes/resource.hrc>
#include <classes/fwkresid.hxx>
#include <protocols.h>
#include <properties.h>
#include <services.h>

#include "helper/mischelper.hxx"

#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
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
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentRecovery.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>

#include <comphelper/configurationhelper.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <unotools/pathoptions.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/tempfile.hxx>
#include <ucbhelper/content.hxx>

#include <osl/time.h>
#include <vcl/msgbox.hxx>
#include <osl/file.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/configmgr.hxx>
#include <svl/documentlockfile.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <tools/urlobj.hxx>

#include <fwkdllapi.h>

//_______________________________________________
// namespaces

#ifndef css
namespace css = ::com::sun::star;
#endif

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::document::XDocumentRecovery;
using ::com::sun::star::frame::ModuleManager;
using ::com::sun::star::frame::XModel2;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::frame::XFrame;
using ::com::sun::star::frame::XController2;
using ::com::sun::star::frame::XLoadable;
using ::com::sun::star::frame::XStorable;
using ::com::sun::star::lang::XComponent;


namespace framework
{

//-----------------------------------------------
// recovery.xcu
static const char CFG_PACKAGE_RECOVERY[] = "org.openoffice.Office.Recovery/";
static const char CFG_ENTRY_RECOVERYLIST[] = "RecoveryList";
static const char CFG_PATH_RECOVERYINFO[] = "RecoveryInfo";
static const char CFG_ENTRY_CRASHED[] = "Crashed";
static const char CFG_ENTRY_SESSIONDATA[] = "SessionData";

static const char CFG_ENTRY_AUTOSAVE_ENABLED[] = "AutoSave/Enabled";
static const char CFG_ENTRY_AUTOSAVE_TIMEINTERVALL[] = "AutoSave/TimeIntervall"; //sic!

static const char CFG_PATH_AUTOSAVE[] = "AutoSave";
static const char CFG_ENTRY_MINSPACE_DOCSAVE[] = "MinSpaceDocSave";
static const char CFG_ENTRY_MINSPACE_CONFIGSAVE[] = "MinSpaceConfigSave";

static const char CFG_PACKAGE_MODULES[] = "org.openoffice.Setup/Office/Factories";
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
static const sal_Int32       RETRY_STORE_ON_MIGHT_FULL_DISC_USEFULL =   3; // in case FULL DISC does not seam the real problem
static const sal_Int32       GIVE_UP_RETRY                          =   1; // in case FULL DISC does not seam the real problem

#define SAVE_IN_PROGRESS            sal_True
#define SAVE_FINISHED               sal_False

#define LOCK_FOR_CACHE_ADD_REMOVE   sal_True
#define LOCK_FOR_CACHE_USE          sal_False

#define MIN_TIME_FOR_USER_IDLE 10000 // 10s user idle

// enable the following defines in case you whish to simulate a full disc for debug purposes .-)

// this define throws everytime a document is stored or a configuration change
// should be flushed an exception ... so the special error handler for this scenario is triggered
// #define TRIGGER_FULL_DISC_CHECK

// force "return sal_False" for the method impl_enoughDiscSpace().
// #define SIMULATE_FULL_DISC

//-----------------------------------------------
// #define ENABLE_RECOVERY_LOGGING
#undef ENABLE_RECOVERY_LOGGING
#ifdef ENABLE_RECOVERY_LOGGING
    #define LOGFILE_RECOVERY "recovery.log"

    #define LOG_RECOVERY(MSG)                       \
        {                                           \
            WRITE_LOGFILE(LOGFILE_RECOVERY, MSG)    \
            WRITE_LOGFILE(LOGFILE_RECOVERY, "\n")   \
        }
#else
    #undef LOGFILE_RECOVERY
    #define LOG_RECOVERY(MSG)
#endif

//-----------------------------------------------
class CacheLockGuard
{
    private:

        // holds the outside calli alive, so it's shared resources
        // are valid everytimes
        css::uno::Reference< css::uno::XInterface > m_xOwner;

        // mutex shared with outside calli !
        LockHelper& m_rSharedMutex;

        // this variable knows the state of the "cache lock"
        sal_Int32& m_rCacheLock;

        // to prevent increasing/decreasing of m_rCacheLock more then ones
        // we must know if THIS guard has an actual lock set there !
        sal_Bool m_bLockedByThisGuard;

    public:

        CacheLockGuard(AutoRecovery* pOwner                      ,
                       LockHelper&   rMutex                      ,
                       sal_Int32&    rCacheLock                  ,
                       sal_Bool      bLockForAddRemoveVectorItems);
        ~CacheLockGuard();

        void lock(sal_Bool bLockForAddRemoveVectorItems);
        void unlock();
};

//-----------------------------------------------
CacheLockGuard::CacheLockGuard(AutoRecovery* pOwner                      ,
                               LockHelper&   rMutex                      ,
                               sal_Int32&    rCacheLock                  ,
                               sal_Bool      bLockForAddRemoveVectorItems)
    : m_xOwner            (static_cast< css::frame::XDispatch* >(pOwner))
    , m_rSharedMutex      (rMutex                                       )
    , m_rCacheLock        (rCacheLock                                   )
    , m_bLockedByThisGuard(sal_False                                    )
{
    lock(bLockForAddRemoveVectorItems);
}

//-----------------------------------------------
CacheLockGuard::~CacheLockGuard()
{
    unlock();
    m_xOwner.clear();
}

//-----------------------------------------------
void CacheLockGuard::lock(sal_Bool bLockForAddRemoveVectorItems)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_rSharedMutex);

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

    if (
        (m_rCacheLock > 0            ) &&
        (bLockForAddRemoveVectorItems)
       )
    {
        OSL_FAIL("Re-entrance problem detected. Using of an stl structure in combination with iteration, adding, removing of elements etcpp.");
        throw css::uno::RuntimeException(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Re-entrance problem detected. Using of an stl structure in combination with iteration, adding, removing of elements etcpp.")),
                m_xOwner);
    }

    ++m_rCacheLock;
    m_bLockedByThisGuard = sal_True;

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void CacheLockGuard::unlock()
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_rSharedMutex);

    if ( ! m_bLockedByThisGuard)
        return;

    --m_rCacheLock;
    m_bLockedByThisGuard = sal_False;

    if (m_rCacheLock < 0)
    {
        OSL_FAIL("Wrong using of member m_nDocCacheLock detected. A ref counted value shouldn't reach values <0 .-)");
        throw css::uno::RuntimeException(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Wrong using of member m_nDocCacheLock detected. A ref counted value shouldn't reach values <0 .-)")),
                m_xOwner);
    }
    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
DispatchParams::DispatchParams()
    : m_nWorkingEntryID(-1)
{
};

//-----------------------------------------------
DispatchParams::DispatchParams(const ::comphelper::SequenceAsHashMap&             lArgs ,
                               const css::uno::Reference< css::uno::XInterface >& xOwner)
{
    m_nWorkingEntryID         = lArgs.getUnpackedValueOrDefault(PROP_ENTRY_ID, (sal_Int32)-1                                       );
    m_xProgress               = lArgs.getUnpackedValueOrDefault(PROP_PROGRESS, css::uno::Reference< css::task::XStatusIndicator >());
    m_sSavePath               = lArgs.getUnpackedValueOrDefault(PROP_SAVEPATH, ::rtl::OUString()                                   );
    m_xHoldRefForAsyncOpAlive = xOwner;
};

//-----------------------------------------------
DispatchParams::DispatchParams(const DispatchParams& rCopy)
{
    m_xProgress               = rCopy.m_xProgress;
    m_sSavePath               = rCopy.m_sSavePath;
    m_nWorkingEntryID         = rCopy.m_nWorkingEntryID;
    m_xHoldRefForAsyncOpAlive = rCopy.m_xHoldRefForAsyncOpAlive;
};

//-----------------------------------------------
DispatchParams::~DispatchParams()
{};

//-----------------------------------------------
DispatchParams& DispatchParams::operator=(const DispatchParams& rCopy)
{
    m_xProgress               = rCopy.m_xProgress;
    m_sSavePath               = rCopy.m_sSavePath;
    m_nWorkingEntryID         = rCopy.m_nWorkingEntryID;
    m_xHoldRefForAsyncOpAlive = rCopy.m_xHoldRefForAsyncOpAlive;
    return *this;
}

//-----------------------------------------------
void DispatchParams::forget()
{
    m_sSavePath       = ::rtl::OUString();
    m_nWorkingEntryID = -1;
    m_xProgress.clear();
    m_xHoldRefForAsyncOpAlive.clear();
};

//-----------------------------------------------
DEFINE_XINTERFACE_10(AutoRecovery                                                               ,
                     OWeakObject                                                                ,
                     DIRECT_INTERFACE (css::lang::XTypeProvider                                ),
                     DIRECT_INTERFACE (css::lang::XServiceInfo                                 ),
                     DIRECT_INTERFACE (css::frame::XDispatch                                   ),
                     DIRECT_INTERFACE (css::beans::XMultiPropertySet                           ),
                     DIRECT_INTERFACE (css::beans::XFastPropertySet                            ),
                     DIRECT_INTERFACE (css::beans::XPropertySet                                ),
                     DIRECT_INTERFACE (css::document::XEventListener                           ),
                     DIRECT_INTERFACE (css::util::XChangesListener                             ),
                     DIRECT_INTERFACE (css::util::XModifyListener                              ),
                     DERIVED_INTERFACE(css::lang::XEventListener, css::document::XEventListener))

//-----------------------------------------------
DEFINE_XTYPEPROVIDER_6(AutoRecovery                 ,
                       css::lang::XTypeProvider     ,
                       css::lang::XServiceInfo      ,
                       css::frame::XDispatch        ,
                       css::beans::XMultiPropertySet,
                       css::beans::XFastPropertySet ,
                       css::beans::XPropertySet     )

//-----------------------------------------------
DEFINE_XSERVICEINFO_ONEINSTANCESERVICE(AutoRecovery                   ,
                                       ::cppu::OWeakObject            ,
                                       SERVICENAME_AUTORECOVERY       ,
                                       IMPLEMENTATIONNAME_AUTORECOVERY)

//-----------------------------------------------
DEFINE_INIT_SERVICE(
                    AutoRecovery,
                    {
                        /*Attention
                            I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                            to create a new instance of this class by our own supported service factory.
                            see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further informations!
                        */

                        // read configuration to know if autosave/recovery is on/off etcpp...
                        implts_readConfig();

                        implts_startListening();

                        // establish callback for our internal used timer.
                        // Note: Its only active, if the timer will be started ...
                        m_aTimer.SetTimeoutHdl(LINK(this, AutoRecovery, implts_timerExpired));
                    }
                   )

//-----------------------------------------------
AutoRecovery::AutoRecovery(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
    : ThreadHelpBase            (&Application::GetSolarMutex()                      )
    , ::cppu::OBroadcastHelper  ( m_aLock.getShareableOslMutex()                    )
    , ::cppu::OPropertySetHelper( *(static_cast< ::cppu::OBroadcastHelper* >(this)) )
    , ::cppu::OWeakObject       (                                                   )
    , m_xSMGR                   (xSMGR                                              )
    , m_bListenForDocEvents     (sal_False                                          )
    , m_bListenForConfigChanges (sal_False                                          )
    , m_nAutoSaveTimeIntervall  (0                                                  )
    , m_eJob                    (AutoRecovery::E_NO_JOB                             )
    , m_aAsyncDispatcher        ( LINK( this, AutoRecovery, implts_asyncDispatch )  )
    , m_eTimerType              (E_DONT_START_TIMER                                 )
    , m_nIdPool                 (0                                                  )
    , m_lListener               (m_aLock.getShareableOslMutex()                     )
    , m_nDocCacheLock           (0                                                  )
    , m_nMinSpaceDocSave        (MIN_DISCSPACE_DOCSAVE                              )
    , m_nMinSpaceConfigSave     (MIN_DISCSPACE_CONFIGSAVE                           )

    #if OSL_DEBUG_LEVEL > 1
    , m_dbg_bMakeItFaster       (sal_False                                          )
    #endif
{
}

//-----------------------------------------------
AutoRecovery::~AutoRecovery()
{
    implts_stopTimer();
}

//-----------------------------------------------
void SAL_CALL AutoRecovery::dispatch(const css::util::URL&                                  aURL      ,
                                     const css::uno::Sequence< css::beans::PropertyValue >& lArguments)
    throw(css::uno::RuntimeException)
{
    LOG_RECOVERY("AutoRecovery::dispatch() starts ...")
    LOG_RECOVERY(U2B(aURL.Complete).getStr())

    // valid request ?
    sal_Int32 eNewJob = AutoRecovery::implst_classifyJob(aURL);
    if (eNewJob == AutoRecovery::E_NO_JOB)
        return;

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    // still running operation ... ignoring AUTO_SAVE.
    // All other requests has higher prio!
    if (
        ( m_eJob                               != AutoRecovery::E_NO_JOB   ) &&
        ((m_eJob & AutoRecovery::E_AUTO_SAVE ) != AutoRecovery::E_AUTO_SAVE)
       )
    {
        LOG_WARNING("AutoRecovery::dispatch()", "There is already an asynchronous dispatch() running. New request will be ignored!")
        return;
    }

    ::comphelper::SequenceAsHashMap lArgs(lArguments);

    // check if somewhere wish to disable recovery temp. for this office session
    // This can be done immediatly ... must not been done asynchronous.
    if ((eNewJob & AutoRecovery::E_DISABLE_AUTORECOVERY) == AutoRecovery::E_DISABLE_AUTORECOVERY)
    {
        // it's important to set a flag internaly, so AutoRecovery will be supressed - even if it's requested.
        m_eJob |= eNewJob;
        implts_stopTimer();
        implts_stopListening();
        return;
    }

    // disable/enable AutoSave for this office session only
    // independend from the configuration entry.
    if ((eNewJob & AutoRecovery::E_SET_AUTOSAVE_STATE) == AutoRecovery::E_SET_AUTOSAVE_STATE)
    {
        sal_Bool bOn = lArgs.getUnpackedValueOrDefault(PROP_AUTOSAVE_STATE, (sal_Bool)sal_True);
        if (bOn)
        {
            // dont enable AutoSave hardly !
            // reload configuration to know the current state.
            implts_readAutoSaveConfig();
            implts_updateTimer();
            // can it happen that might be the listener was stopped ? .-)
            // make sure it runs always ... even if AutoSave itself was disabled temporarly.
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

    sal_Bool       bAsync  = lArgs.getUnpackedValueOrDefault(PROP_DISPATCH_ASYNCHRON, (sal_Bool)sal_False);
    DispatchParams aParams (lArgs, static_cast< css::frame::XDispatch* >(this));

    // Hold this instance alive till the asynchronous operation will be finished.
    if (bAsync)
        m_aDispatchParams = aParams;

    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    if (bAsync)
        m_aAsyncDispatcher.Post(0);
    else
        implts_dispatch(aParams);
}

void AutoRecovery::ListenerInformer::start()
{
    m_rRecovery.implts_informListener(m_eJob,
        AutoRecovery::implst_createFeatureStateEvent(m_eJob, OPERATION_START, NULL));
}

void AutoRecovery::ListenerInformer::stop()
{
    if (m_bStopped)
        return;
    m_rRecovery.implts_informListener(m_eJob,
        AutoRecovery::implst_createFeatureStateEvent(m_eJob, OPERATION_STOP, NULL));
    m_bStopped = true;
}

//-----------------------------------------------
void AutoRecovery::implts_dispatch(const DispatchParams& aParams)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);
    sal_Int32 eJob = m_eJob;
    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    // in case a new dispatch overwrites a may ba active AutoSave session
    // we must restore this session later. see below ...
    sal_Bool bWasAutoSaveActive = ((eJob & AutoRecovery::E_AUTO_SAVE) == AutoRecovery::E_AUTO_SAVE);

    // On the other side it make no sense to reactivate the AutoSave operation
    // if the new dispatch indicates a final decision ...
    // E.g. an EmergencySave/SessionSave indicates the end of life of the current office session.
    // It make no sense to reactivate an AutoSave then.
    // But a Recovery or SessionRestore should reactivate a may be already active AutoSave.
    sal_Bool bAllowAutoSaveReactivation = sal_True;

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
            LOG_RECOVERY("... prepare emergency save ...")
            bAllowAutoSaveReactivation = sal_False;
            implts_prepareEmergencySave();
        }
        else
        if (
            ((eJob & AutoRecovery::E_EMERGENCY_SAVE  ) == AutoRecovery::E_EMERGENCY_SAVE  ) &&
            ((eJob & AutoRecovery::E_DISABLE_AUTORECOVERY) != AutoRecovery::E_DISABLE_AUTORECOVERY)
           )
        {
            LOG_RECOVERY("... do emergency save ...")
            bAllowAutoSaveReactivation = sal_False;
            implts_doEmergencySave(aParams);
        }
        else
        if (
            ((eJob & AutoRecovery::E_RECOVERY        ) == AutoRecovery::E_RECOVERY        ) &&
            ((eJob & AutoRecovery::E_DISABLE_AUTORECOVERY) != AutoRecovery::E_DISABLE_AUTORECOVERY)
           )
        {
            LOG_RECOVERY("... do recovery ...")
            implts_doRecovery(aParams);
        }
        else
        if (
            ((eJob & AutoRecovery::E_SESSION_SAVE    ) == AutoRecovery::E_SESSION_SAVE    ) &&
            ((eJob & AutoRecovery::E_DISABLE_AUTORECOVERY) != AutoRecovery::E_DISABLE_AUTORECOVERY)
            )
        {
            LOG_RECOVERY("... do session save ...")
            bAllowAutoSaveReactivation = sal_False;
            implts_doSessionSave(aParams);
        }
        else
        if (
            ((eJob & AutoRecovery::E_SESSION_QUIET_QUIT    ) == AutoRecovery::E_SESSION_QUIET_QUIT ) &&
            ((eJob & AutoRecovery::E_DISABLE_AUTORECOVERY) != AutoRecovery::E_DISABLE_AUTORECOVERY)
            )
        {
            LOG_RECOVERY("... do session quiet quit ...")
            bAllowAutoSaveReactivation = sal_False;
            implts_doSessionQuietQuit(aParams);
        }
        else
        if (
            ((eJob & AutoRecovery::E_SESSION_RESTORE ) == AutoRecovery::E_SESSION_RESTORE ) &&
            ((eJob & AutoRecovery::E_DISABLE_AUTORECOVERY) != AutoRecovery::E_DISABLE_AUTORECOVERY)
            )
        {
            LOG_RECOVERY("... do session restore ...")
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

    // SAFE -> ----------------------------------
    aWriteLock.lock();
    m_eJob = E_NO_JOB;
    if (
        (bAllowAutoSaveReactivation) &&
        (bWasAutoSaveActive        )
       )
    {
        m_eJob |= AutoRecovery::E_AUTO_SAVE;
    }

    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    // depends on bAllowAutoSaveReactivation implicitly by looking on m_eJob=E_AUTO_SAVE! see before ...
    implts_updateTimer();

    if (bAllowAutoSaveReactivation)
        implts_startListening();
}

//-----------------------------------------------
void SAL_CALL AutoRecovery::addStatusListener(const css::uno::Reference< css::frame::XStatusListener >& xListener,
                                              const css::util::URL&                                     aURL     )
    throw(css::uno::RuntimeException)
{
    if (!xListener.is())
        throw css::uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Invalid listener reference.")), static_cast< css::frame::XDispatch* >(this));
    // container is threadsafe by using a shared mutex!
    m_lListener.addInterface(aURL.Complete, xListener);

    // REENTRANT !? -> --------------------------------
    CacheLockGuard aCacheLock(this, m_aLock, m_nDocCacheLock, LOCK_FOR_CACHE_USE);

    // THREAD SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    AutoRecovery::TDocumentList::iterator pIt;
    for(  pIt  = m_lDocCache.begin();
          pIt != m_lDocCache.end()  ;
        ++pIt                       )
    {
        AutoRecovery::TDocumentInfo&  rInfo = *pIt;
        css::frame::FeatureStateEvent aEvent = AutoRecovery::implst_createFeatureStateEvent(m_eJob, OPERATION_UPDATE, &rInfo);

        // <- SAFE ------------------------------
        aReadLock.unlock();
        xListener->statusChanged(aEvent);
        aReadLock.lock();
        // SAFE -> ------------------------------
    }

    aReadLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void SAL_CALL AutoRecovery::removeStatusListener(const css::uno::Reference< css::frame::XStatusListener >& xListener,
                                                 const css::util::URL&                                     aURL     )
    throw(css::uno::RuntimeException)
{
    if (!xListener.is())
        throw css::uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Invalid listener reference.")), static_cast< css::frame::XDispatch* >(this));
    // container is threadsafe by using a shared mutex!
    m_lListener.removeInterface(aURL.Complete, xListener);
}

//-----------------------------------------------
void SAL_CALL AutoRecovery::notifyEvent(const css::document::EventObject& aEvent)
    throw(css::uno::RuntimeException)
{
    css::uno::Reference< css::frame::XModel > xDocument(aEvent.Source, css::uno::UNO_QUERY);

    // new document => put it into the internal list
    if (
        (aEvent.EventName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(EVENT_ON_NEW))) ||
        (aEvent.EventName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(EVENT_ON_LOAD)))
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
       Our application code isnt ready for multiple save requests
       at the same time. So we have to supress our AutoSave feature
       for the moment, till this other save requests will be finished.
     */
    else if (
        (aEvent.EventName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(EVENT_ON_SAVE))) ||
        (aEvent.EventName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(EVENT_ON_SAVEAS))) ||
        (aEvent.EventName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(EVENT_ON_SAVETO)))
       )
    {
        implts_updateDocumentUsedForSavingState(xDocument, SAVE_IN_PROGRESS);
    }
    // document saved => remove tmp. files - but hold config entries alive!
    else if (
        (aEvent.EventName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(EVENT_ON_SAVEDONE))) ||
        (aEvent.EventName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(EVENT_ON_SAVEASDONE)))
       )
    {
        implts_markDocumentAsSaved(xDocument);
        implts_updateDocumentUsedForSavingState(xDocument, SAVE_FINISHED);
    }
    /* document saved as copy => mark it as "non used by concurrent save operation".
       so we can try to create a backup copy if next time AutoSave is started too.
       Dont remove temp. files or change the modified state of the document!
       It was not realy saved to the original file ...
    */
    else if ( aEvent.EventName == EVENT_ON_SAVETODONE )
    {
        implts_updateDocumentUsedForSavingState(xDocument, SAVE_FINISHED);
    }
    // If saving of a document failed by an error ... we have to save this document
    // by ourself next time AutoSave or EmergencySave is triggered.
    // But we can reset the state "used for other save requests". Otherwhise
    // these documents will never be saved!
    else if (
        (aEvent.EventName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(EVENT_ON_SAVEFAILED))) ||
        (aEvent.EventName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(EVENT_ON_SAVEASFAILED))) ||
        (aEvent.EventName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(EVENT_ON_SAVETOFAILED)))
       )
    {
        implts_updateDocumentUsedForSavingState(xDocument, SAVE_FINISHED);
    }
    // document closed => remove temp. files and configuration entries
    else if ( aEvent.EventName == EVENT_ON_UNLOAD )
    {
        implts_deregisterDocument(xDocument, sal_True); // sal_True => stop listening for disposing() !
    }
}

//-----------------------------------------------
void SAL_CALL AutoRecovery::changesOccurred(const css::util::ChangesEvent& aEvent)
    throw(css::uno::RuntimeException)
{
    const css::uno::Sequence< css::util::ElementChange > lChanges (aEvent.Changes);
    const css::util::ElementChange*                      pChanges = lChanges.getConstArray();

    sal_Int32 c = lChanges.getLength();
    sal_Int32 i = 0;

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    // Changes of the configuration must be ignored if AutoSave/Recovery was disabled for this
    // office session. That can happen if e.g. the command line arguments "--norestore" or "--headless"
    // was set.
    if ((m_eJob & AutoRecovery::E_DISABLE_AUTORECOVERY) == AutoRecovery::E_DISABLE_AUTORECOVERY)
       return;

    for (i=0; i<c; ++i)
    {
        ::rtl::OUString sPath;
        pChanges[i].Accessor >>= sPath;

        if ( sPath == CFG_ENTRY_AUTOSAVE_ENABLED )
        {
            sal_Bool bEnabled = sal_False;
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

    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    // Note: This call stops the timer and starts it again.
    // But it checks the different timer states internaly and
    // may be supress the restart!
    implts_updateTimer();
}

//-----------------------------------------------
void SAL_CALL AutoRecovery::modified(const css::lang::EventObject& aEvent)
    throw(css::uno::RuntimeException)
{
    css::uno::Reference< css::frame::XModel > xDocument(aEvent.Source, css::uno::UNO_QUERY);
    if (! xDocument.is())
        return;

    implts_markDocumentModifiedAgainstLastBackup(xDocument);
}

//-----------------------------------------------
void SAL_CALL AutoRecovery::disposing(const css::lang::EventObject& aEvent)
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

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
    // Normaly they should send a OnUnload message ...
    // But some stacktraces shows another possible use case .-)
    css::uno::Reference< css::frame::XModel > xDocument(aEvent.Source, css::uno::UNO_QUERY);
    if (xDocument.is())
    {
        implts_deregisterDocument(xDocument, sal_False); // sal_False => dont call removeEventListener() .. because it's not needed here
        return;
    }

    // <- SAFE ----------------------------------
}

//-----------------------------------------------
css::uno::Reference< css::container::XNameAccess > AutoRecovery::implts_openConfig()
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    if (m_xRecoveryCFG.is())
        return m_xRecoveryCFG;
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;

    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    rtl::OUString sCFG_PACKAGE_RECOVERY(RTL_CONSTASCII_USTRINGPARAM(CFG_PACKAGE_RECOVERY));
    // throws a RuntimeException if an error occure!
    css::uno::Reference< css::container::XNameAccess > xCFG(
        ::comphelper::ConfigurationHelper::openConfig(xSMGR, sCFG_PACKAGE_RECOVERY, ::comphelper::ConfigurationHelper::E_STANDARD),
        css::uno::UNO_QUERY);

    sal_Int32 nMinSpaceDocSave    = MIN_DISCSPACE_DOCSAVE;
    sal_Int32 nMinSpaceConfigSave = MIN_DISCSPACE_CONFIGSAVE;

    try
    {
        rtl::OUString sCFG_PATH_AUTOSAVE(CFG_PATH_AUTOSAVE);
        ::comphelper::ConfigurationHelper::readDirectKey(xSMGR,
                                                         sCFG_PACKAGE_RECOVERY,
                                                         sCFG_PATH_AUTOSAVE,
                                                         rtl::OUString(CFG_ENTRY_MINSPACE_DOCSAVE),
                                                         ::comphelper::ConfigurationHelper::E_STANDARD) >>= nMinSpaceDocSave;

        ::comphelper::ConfigurationHelper::readDirectKey(xSMGR,
                                                         sCFG_PACKAGE_RECOVERY,
                                                         sCFG_PATH_AUTOSAVE,
                                                         rtl::OUString(CFG_ENTRY_MINSPACE_CONFIGSAVE),
                                                         ::comphelper::ConfigurationHelper::E_STANDARD) >>= nMinSpaceConfigSave;
    }
    catch(const css::uno::Exception&)
    {
        // These config keys are not sooooo important, that
        // we are interested on errors here realy .-)
        nMinSpaceDocSave    = MIN_DISCSPACE_DOCSAVE;
        nMinSpaceConfigSave = MIN_DISCSPACE_CONFIGSAVE;
    }

    // SAFE -> ----------------------------------
    aWriteLock.lock();
    m_xRecoveryCFG        = xCFG;
    m_nMinSpaceDocSave    = nMinSpaceDocSave;
    m_nMinSpaceConfigSave = nMinSpaceConfigSave;
    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    return xCFG;
}

//-----------------------------------------------
void AutoRecovery::implts_readAutoSaveConfig()
{
    css::uno::Reference< css::container::XHierarchicalNameAccess > xCommonRegistry(implts_openConfig(), css::uno::UNO_QUERY);

    // AutoSave [bool]
    sal_Bool bEnabled = sal_False;
    xCommonRegistry->getByHierarchicalName(rtl::OUString(CFG_ENTRY_AUTOSAVE_ENABLED)) >>= bEnabled;

    // SAFE -> ------------------------------
    WriteGuard aWriteLock(m_aLock);
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
    aWriteLock.unlock();
    // <- SAFE ------------------------------

    // AutoSaveTimeIntervall [int] in min
    sal_Int32 nTimeIntervall = 15;
    xCommonRegistry->getByHierarchicalName(rtl::OUString(CFG_ENTRY_AUTOSAVE_TIMEINTERVALL)) >>= nTimeIntervall;

    // SAFE -> ----------------------------------
    aWriteLock.lock();
    m_nAutoSaveTimeIntervall = nTimeIntervall;
    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void AutoRecovery::implts_readConfig()
{
    implts_readAutoSaveConfig();

    css::uno::Reference< css::container::XHierarchicalNameAccess > xCommonRegistry(implts_openConfig(), css::uno::UNO_QUERY);

    // REENTRANT -> --------------------------------
    CacheLockGuard aCacheLock(this, m_aLock, m_nDocCacheLock, LOCK_FOR_CACHE_ADD_REMOVE);

    // THREADSAFE -> -------------------------------
    WriteGuard aWriteLock(m_aLock);
    // reset current cache load cache
    m_lDocCache.clear();
    m_nIdPool = 0;
    aWriteLock.unlock();
    // <- THREADSAFE -------------------------------

    aCacheLock.unlock();
    // <- REENTRANT --------------------------------

    css::uno::Any aValue;

    // RecoveryList [set]
    aValue = xCommonRegistry->getByHierarchicalName(rtl::OUString(CFG_ENTRY_RECOVERYLIST));
    css::uno::Reference< css::container::XNameAccess > xList;
    aValue >>= xList;
    if (xList.is())
    {
        const rtl::OUString sRECOVERY_ITEM_BASE_IDENTIFIER(RECOVERY_ITEM_BASE_IDENTIFIER);
        const css::uno::Sequence< ::rtl::OUString > lItems = xList->getElementNames();
        const ::rtl::OUString*                      pItems = lItems.getConstArray();
              sal_Int32                             c      = lItems.getLength();
              sal_Int32                             i      = 0;

        // REENTRANT -> --------------------------
        aCacheLock.lock(LOCK_FOR_CACHE_ADD_REMOVE);

        for (i=0; i<c; ++i)
        {
            css::uno::Reference< css::beans::XPropertySet > xItem;
            xList->getByName(pItems[i]) >>= xItem;
            if (!xItem.is())
                continue;

            AutoRecovery::TDocumentInfo aInfo;
            aInfo.NewTempURL = ::rtl::OUString();
            aInfo.Document   = css::uno::Reference< css::frame::XModel >();
            xItem->getPropertyValue(rtl::OUString(CFG_ENTRY_PROP_ORIGINALURL)) >>= aInfo.OrgURL       ;
            xItem->getPropertyValue(rtl::OUString(CFG_ENTRY_PROP_TEMPURL)) >>= aInfo.OldTempURL   ;
            xItem->getPropertyValue(rtl::OUString(CFG_ENTRY_PROP_TEMPLATEURL)) >>= aInfo.TemplateURL  ;
            xItem->getPropertyValue(rtl::OUString(CFG_ENTRY_PROP_FILTER)) >>= aInfo.RealFilter   ;
            xItem->getPropertyValue(rtl::OUString(CFG_ENTRY_PROP_DOCUMENTSTATE)) >>= aInfo.DocumentState;
            xItem->getPropertyValue(rtl::OUString(CFG_ENTRY_PROP_MODULE)) >>= aInfo.AppModule;
            xItem->getPropertyValue(rtl::OUString(CFG_ENTRY_PROP_TITLE)) >>= aInfo.Title;
            xItem->getPropertyValue(rtl::OUString(CFG_ENTRY_PROP_VIEWNAMES)) >>= aInfo.ViewNames;
            implts_specifyAppModuleAndFactory(aInfo);
            implts_specifyDefaultFilterAndExtension(aInfo);

            if (pItems[i].indexOf(sRECOVERY_ITEM_BASE_IDENTIFIER)==0)
            {
                ::rtl::OUString sID = pItems[i].copy(sRECOVERY_ITEM_BASE_IDENTIFIER.getLength());
                aInfo.ID = sID.toInt32();
                // SAFE -> ----------------------
                aWriteLock.lock();
                if (aInfo.ID > m_nIdPool)
                {
                    m_nIdPool = aInfo.ID+1;
                    LOG_ASSERT(m_nIdPool>=0, "AutoRecovery::implts_readConfig()\nOverflow of IDPool detected!")
                }
                aWriteLock.unlock();
                // <- SAFE ----------------------
            }
            #ifdef ENABLE_WARNINGS
            else
                LOG_WARNING("AutoRecovery::implts_readConfig()", "Who changed numbering of recovery items? Cache will be inconsistent then! I do not know, what will happen next time .-)")
            #endif

            // THREADSAFE -> --------------------------
            aWriteLock.lock();
            m_lDocCache.push_back(aInfo);
            aWriteLock.unlock();
            // <- THREADSAFE --------------------------
        }

        aCacheLock.unlock();
        // <- REENTRANT --------------------------
    }

    implts_updateTimer();
}

//-----------------------------------------------
void AutoRecovery::implts_specifyDefaultFilterAndExtension(AutoRecovery::TDocumentInfo& rInfo)
{
    if (rInfo.AppModule.isEmpty())
    {
        throw css::uno::RuntimeException(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Cant find out the default filter and its extension, if no application module is known!")),
                static_cast< css::frame::XDispatch* >(this));
    }

    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    css::uno::Reference< css::container::XNameAccess>      xCFG  = m_xModuleCFG;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    try
    {
        if (! xCFG.is())
        {
            // open module config on demand and cache the update access
            xCFG = css::uno::Reference< css::container::XNameAccess >(
                ::comphelper::ConfigurationHelper::openConfig(xSMGR, rtl::OUString(CFG_PACKAGE_MODULES),
                ::comphelper::ConfigurationHelper::E_STANDARD),
                css::uno::UNO_QUERY_THROW);

            // SAFE -> ----------------------------------
            WriteGuard aWriteLock(m_aLock);
            m_xModuleCFG = xCFG;
            aWriteLock.unlock();
            // <- SAFE ----------------------------------
        }

        css::uno::Reference< css::container::XNameAccess > xModuleProps(
            xCFG->getByName(rInfo.AppModule),
            css::uno::UNO_QUERY_THROW);

        xModuleProps->getByName(rtl::OUString(CFG_ENTRY_REALDEFAULTFILTER)) >>= rInfo.DefaultFilter;

        css::uno::Reference< css::container::XNameAccess > xFilterCFG(xSMGR->createInstance(SERVICENAME_FILTERFACTORY), css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::container::XNameAccess > xTypeCFG  (xSMGR->createInstance(SERVICENAME_TYPEDETECTION), css::uno::UNO_QUERY_THROW);

        ::comphelper::SequenceAsHashMap       lFilterProps        (xFilterCFG->getByName(rInfo.DefaultFilter));
        ::rtl::OUString                       sTypeRegistration   = lFilterProps.getUnpackedValueOrDefault(rtl::OUString(FILTER_PROP_TYPE), ::rtl::OUString());
        ::comphelper::SequenceAsHashMap       lTypeProps          (xTypeCFG->getByName(sTypeRegistration));
        css::uno::Sequence< ::rtl::OUString > lExtensions         = lTypeProps.getUnpackedValueOrDefault(rtl::OUString(TYPE_PROP_EXTENSIONS), css::uno::Sequence< ::rtl::OUString >());
        if (lExtensions.getLength())
        {
            rInfo.Extension  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("."));
            rInfo.Extension += lExtensions[0];
        }
        else
            rInfo.Extension = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".unknown"));
    }
    catch(const css::uno::Exception&)
    {
        rInfo.DefaultFilter = ::rtl::OUString();
        rInfo.Extension     = ::rtl::OUString();
    }
}

//-----------------------------------------------
void AutoRecovery::implts_specifyAppModuleAndFactory(AutoRecovery::TDocumentInfo& rInfo)
{
    ENSURE_OR_THROW2(
        !rInfo.AppModule.isEmpty() || rInfo.Document.is(),
        "Cant find out the application module nor its factory URL, if no application module (or a suitable) document is known!",
        *this );

    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::frame::XModuleManager2 > xManager = ModuleManager::create( comphelper::getComponentContext(xSMGR) );
    css::uno::Reference< css::container::XNameAccess > xModuleConfig(xManager, css::uno::UNO_QUERY_THROW);

    if (rInfo.AppModule.isEmpty())
        rInfo.AppModule = xManager->identify(rInfo.Document);

    ::comphelper::SequenceAsHashMap lModuleDescription(xModuleConfig->getByName(rInfo.AppModule));
    lModuleDescription[rtl::OUString(CFG_ENTRY_PROP_EMPTYDOCUMENTURL)] >>= rInfo.FactoryURL;
    lModuleDescription[rtl::OUString(CFG_ENTRY_PROP_FACTORYSERVICE)] >>= rInfo.FactoryService;
}

//-----------------------------------------------
void AutoRecovery::implts_collectActiveViewNames( AutoRecovery::TDocumentInfo& i_rInfo )
{
    ENSURE_OR_THROW2( i_rInfo.Document.is(), "need at document, at the very least", *this );

    i_rInfo.ViewNames.realloc(0);

    // obtain list of controllers of this document
    ::std::vector< ::rtl::OUString > aViewNames;
    const Reference< XModel2 > xModel( i_rInfo.Document, UNO_QUERY );
    if ( xModel.is() )
    {
        const Reference< XEnumeration > xEnumControllers( xModel->getControllers() );
        while ( xEnumControllers->hasMoreElements() )
        {
            const Reference< XController2 > xController( xEnumControllers->nextElement(), UNO_QUERY );
            ::rtl::OUString sViewName;
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
        ::rtl::OUString sViewName;
        if ( xController.is() )
            sViewName = xController->getViewControllerName();
        OSL_ENSURE( !sViewName.isEmpty(), "AutoRecovery::implts_collectActiveViewNames: (no XController2 ->) no view name -> no recovery of this view!" );

        if ( !sViewName.isEmpty() )
            aViewNames.push_back( sViewName );
    }

    i_rInfo.ViewNames.realloc( aViewNames.size() );
    ::std::copy( aViewNames.begin(), aViewNames.end(), i_rInfo.ViewNames.getArray() );
}

//-----------------------------------------------
void AutoRecovery::implts_persistAllActiveViewNames()
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    // This list will be filled with every document
    AutoRecovery::TDocumentList::iterator pIt;
    for (  pIt  = m_lDocCache.begin();
           pIt != m_lDocCache.end()  ;
         ++pIt                       )
    {
        implts_collectActiveViewNames( *pIt );
        implts_flushConfigItem( *pIt );
    }
}

//-----------------------------------------------
void AutoRecovery::implts_flushConfigItem(const AutoRecovery::TDocumentInfo& rInfo, sal_Bool bRemoveIt)
{
    css::uno::Reference< css::container::XHierarchicalNameAccess > xCFG;

    try
    {
        xCFG = css::uno::Reference< css::container::XHierarchicalNameAccess >(implts_openConfig(), css::uno::UNO_QUERY_THROW);

        css::uno::Reference< css::container::XNameAccess > xCheck;
        xCFG->getByHierarchicalName(rtl::OUString(CFG_ENTRY_RECOVERYLIST)) >>= xCheck;

        css::uno::Reference< css::container::XNameContainer >   xModify(xCheck, css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::lang::XSingleServiceFactory > xCreate(xCheck, css::uno::UNO_QUERY_THROW);

        ::rtl::OUStringBuffer sIDBuf;
        sIDBuf.appendAscii(RTL_CONSTASCII_STRINGPARAM(RECOVERY_ITEM_BASE_IDENTIFIER));
        sIDBuf.append((sal_Int32)rInfo.ID);
        ::rtl::OUString sID = sIDBuf.makeStringAndClear();

        // remove
        if (bRemoveIt)
        {
            // Catch NoSuchElementException.
            // Its not a good idea inside multithreaded environments to call hasElement - removeElement.
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
            sal_Bool                                        bNew = (!xCheck->hasByName(sID));
            if (bNew)
                xSet = css::uno::Reference< css::beans::XPropertySet >(xCreate->createInstance(), css::uno::UNO_QUERY_THROW);
            else
                xCheck->getByName(sID) >>= xSet;

            xSet->setPropertyValue(rtl::OUString(CFG_ENTRY_PROP_ORIGINALURL), css::uno::makeAny(rInfo.OrgURL       ));
            xSet->setPropertyValue(rtl::OUString(CFG_ENTRY_PROP_TEMPURL), css::uno::makeAny(rInfo.OldTempURL   ));
            xSet->setPropertyValue(rtl::OUString(CFG_ENTRY_PROP_TEMPLATEURL), css::uno::makeAny(rInfo.TemplateURL  ));
            xSet->setPropertyValue(rtl::OUString(CFG_ENTRY_PROP_FILTER), css::uno::makeAny(rInfo.RealFilter));
            xSet->setPropertyValue(rtl::OUString(CFG_ENTRY_PROP_DOCUMENTSTATE), css::uno::makeAny(rInfo.DocumentState));
            xSet->setPropertyValue(rtl::OUString(CFG_ENTRY_PROP_MODULE), css::uno::makeAny(rInfo.AppModule));
            xSet->setPropertyValue(rtl::OUString(CFG_ENTRY_PROP_TITLE), css::uno::makeAny(rInfo.Title));
            xSet->setPropertyValue(rtl::OUString(CFG_ENTRY_PROP_VIEWNAMES), css::uno::makeAny(rInfo.ViewNames));

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
            css::uno::Reference< css::util::XChangesBatch > xFlush(xCFG, css::uno::UNO_QUERY_THROW);
            xFlush->commitChanges();

#ifdef TRIGGER_FULL_DISC_CHECK
            throw css::uno::Exception();
#else  // TRIGGER_FULL_DISC_CHECK
            nRetry = 0;
#endif // TRIGGER_FULL_DISC_CHECK
        }
        catch(const css::uno::Exception&)
        {
            // a) FULL DISC seams to be the problem behind                              => show error and retry it forever (e.g. retry=300)
            // b) unknown problem (may be locking problem)                              => reset RETRY value to more usefull value(!) (e.g. retry=3)
            // c) unknown problem (may be locking problem) + 1..2 repeating operations  => throw the original exception to force generation of a stacktrace !

            // SAFE ->
            ReadGuard aReadLock(m_aLock);
            sal_Int32 nMinSpaceConfigSave = m_nMinSpaceConfigSave;
            aReadLock.unlock();
            // <- SAFE

            if (! impl_enoughDiscSpace(nMinSpaceConfigSave))
                AutoRecovery::impl_showFullDiscError();
            else
            if (nRetry > RETRY_STORE_ON_MIGHT_FULL_DISC_USEFULL)
                nRetry = RETRY_STORE_ON_MIGHT_FULL_DISC_USEFULL;
            else
            if (nRetry <= GIVE_UP_RETRY)
                throw; // force stacktrace to know if there exist might other reasons, why an AutoSave can fail !!!

            --nRetry;
        }
    }
    while(nRetry>0);
}

//-----------------------------------------------
void AutoRecovery::implts_startListening()
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory >  xSMGR               = m_xSMGR;
    css::uno::Reference< css::util::XChangesNotifier >      xCFG                (m_xRecoveryCFG, css::uno::UNO_QUERY);
    css::uno::Reference< css::document::XEventBroadcaster > xBroadcaster        = m_xNewDocBroadcaster;
    sal_Bool                                                bListenForDocEvents = m_bListenForDocEvents;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    if (
        (  xCFG.is()                ) &&
        (! m_bListenForConfigChanges)
       )
    {
        m_xRecoveryCFGListener = new WeakChangesListener(this);
        xCFG->addChangesListener(m_xRecoveryCFGListener);
        m_bListenForConfigChanges = sal_True;
    }

    if (!xBroadcaster.is())
    {
        xBroadcaster = css::uno::Reference< css::document::XEventBroadcaster >(xSMGR->createInstance(SERVICENAME_GLOBALEVENTBROADCASTER), css::uno::UNO_QUERY_THROW);
        // SAFE -> ----------------------------------
        WriteGuard aWriteLock(m_aLock);
        m_xNewDocBroadcaster = xBroadcaster;
        aWriteLock.unlock();
        // <- SAFE ----------------------------------
    }

    if (
        (  xBroadcaster.is()  ) &&
        (! bListenForDocEvents)
       )
    {
        m_xNewDocBroadcasterListener = new WeakDocumentEventListener(this);
        xBroadcaster->addEventListener(m_xNewDocBroadcasterListener);
        // SAFE ->
        WriteGuard aWriteLock(m_aLock);
        m_bListenForDocEvents = sal_True;
        aWriteLock.unlock();
        // <- SAFE
    }
}

//-----------------------------------------------
void AutoRecovery::implts_stopListening()
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    // Attention: Dont reset our internal members here too.
    // May be we must work with our configuration, but dont wish to be informed
    // about changes any longer. Needed e.g. during EMERGENCY_SAVE!
    css::uno::Reference< css::util::XChangesNotifier >      xCFG                   (m_xRecoveryCFG      , css::uno::UNO_QUERY);
    css::uno::Reference< css::document::XEventBroadcaster > xGlobalEventBroadcaster(m_xNewDocBroadcaster, css::uno::UNO_QUERY);
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    if (
        (xGlobalEventBroadcaster.is()) &&
        (m_bListenForDocEvents       )
       )
    {
        xGlobalEventBroadcaster->removeEventListener(m_xNewDocBroadcasterListener);
        m_bListenForDocEvents = sal_False;
    }

    if (
        (xCFG.is()                ) &&
        (m_bListenForConfigChanges)
       )
    {
        xCFG->removeChangesListener(m_xRecoveryCFGListener);
        m_bListenForConfigChanges = sal_False;
    }
}

//-----------------------------------------------
void AutoRecovery::implts_startModifyListeningOnDoc(AutoRecovery::TDocumentInfo& rInfo)
{
    if (rInfo.ListenForModify)
        return;

    css::uno::Reference< css::util::XModifyBroadcaster > xBroadcaster(rInfo.Document, css::uno::UNO_QUERY);
    if (xBroadcaster.is())
    {
        css::uno::Reference< css::util::XModifyListener > xThis(static_cast< css::frame::XDispatch* >(this), css::uno::UNO_QUERY);
        xBroadcaster->addModifyListener(xThis);
        rInfo.ListenForModify = sal_True;
    }
}

//-----------------------------------------------
void AutoRecovery::implts_stopModifyListeningOnDoc(AutoRecovery::TDocumentInfo& rInfo)
{
    if (! rInfo.ListenForModify)
        return;

    css::uno::Reference< css::util::XModifyBroadcaster > xBroadcaster(rInfo.Document, css::uno::UNO_QUERY);
    if (xBroadcaster.is())
    {
        css::uno::Reference< css::util::XModifyListener > xThis(static_cast< css::frame::XDispatch* >(this), css::uno::UNO_QUERY);
        xBroadcaster->removeModifyListener(xThis);
        rInfo.ListenForModify = sal_False;
    }
}

//-----------------------------------------------
void AutoRecovery::implts_updateTimer()
{
    implts_stopTimer();

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    if (
        (m_eJob       == AutoRecovery::E_NO_JOB          ) || // TODO may be superflous - E_DONT_START_TIMER should be used only
        (m_eTimerType == AutoRecovery::E_DONT_START_TIMER)
       )
        return;

    sal_uLong nMilliSeconds = 0;
    if (m_eTimerType == AutoRecovery::E_NORMAL_AUTOSAVE_INTERVALL)
    {
        nMilliSeconds = (m_nAutoSaveTimeIntervall*60000); // [min] => 60.000 ms
        #if OSL_DEBUG_LEVEL > 1
        if (m_dbg_bMakeItFaster)
            nMilliSeconds = m_nAutoSaveTimeIntervall;  // [ms]
        #endif
    }
    else
    if (m_eTimerType == AutoRecovery::E_POLL_FOR_USER_IDLE)
    {
        nMilliSeconds = MIN_TIME_FOR_USER_IDLE;
        #if OSL_DEBUG_LEVEL > 1
        if (m_dbg_bMakeItFaster)
            nMilliSeconds = 300; // let us some time, to finish this method .-)
        #endif
    }
    else
    if (m_eTimerType == AutoRecovery::E_POLL_TILL_AUTOSAVE_IS_ALLOWED)
        nMilliSeconds = 300; // there is a minimum time frame, where the user can loose some key input data!

    m_aTimer.SetTimeout(nMilliSeconds);
    m_aTimer.Start();

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void AutoRecovery::implts_stopTimer()
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    if (!m_aTimer.IsActive())
        return;
    m_aTimer.Stop();

    // <- SAFE ----------------------------------
}

//-----------------------------------------------
IMPL_LINK_NOARG(AutoRecovery, implts_timerExpired)
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
        // was set. But normaly the timer was disabled if recovery was disabled ...
        // But so we are more "safe" .-)
        // SAFE -> ----------------------------------
        ReadGuard aReadLock(m_aLock);
        if ((m_eJob & AutoRecovery::E_DISABLE_AUTORECOVERY) == AutoRecovery::E_DISABLE_AUTORECOVERY)
           return 0;
        aReadLock.unlock();
        // <- SAFE ----------------------------------

        // check some "states", where its not allowed (better: not a good idea) to
        // start an AutoSave. (e.g. if the user makes drag & drop ...)
        // Then we poll till this "disallowed" state is gone.
        sal_Bool bAutoSaveNotAllowed = Application::IsUICaptured();
        if (bAutoSaveNotAllowed)
        {
            // SAFE -> ------------------------------
            WriteGuard aWriteLock(m_aLock);
            m_eTimerType = AutoRecovery::E_POLL_TILL_AUTOSAVE_IS_ALLOWED;
            aWriteLock.unlock();
            // <- SAFE ------------------------------
            implts_updateTimer();
            return 0;
        }

        // analyze timer type.
        // If we poll for an user idle period, may be we must
        // do nothing here and start the timer again.
        // SAFE -> ----------------------------------
        WriteGuard aWriteLock(m_aLock);

        if (m_eTimerType == AutoRecovery::E_POLL_FOR_USER_IDLE)
        {
            sal_Bool bUserIdle = (Application::GetLastInputInterval()>MIN_TIME_FOR_USER_IDLE);
            if (!bUserIdle)
            {
                implts_updateTimer();
                return 0;
            }
        }

        aWriteLock.unlock();
        // <- SAFE ----------------------------------

        implts_informListener(AutoRecovery::E_AUTO_SAVE,
            AutoRecovery::implst_createFeatureStateEvent(AutoRecovery::E_AUTO_SAVE, OPERATION_START, NULL));

        // force save of all currently open documents
        // The called method returns an info, if and how this
        // timer must be restarted.
        sal_Bool bAllowUserIdleLoop = sal_True;
        AutoRecovery::ETimerType eSuggestedTimer = implts_saveDocs(bAllowUserIdleLoop, sal_False);

        // If timer isnt used for "short callbacks" (means polling
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
            implts_resetHandleStates(sal_False);
        }

        implts_informListener(AutoRecovery::E_AUTO_SAVE,
            AutoRecovery::implst_createFeatureStateEvent(AutoRecovery::E_AUTO_SAVE, OPERATION_STOP, NULL));

        // restart timer - because it was disabled before ...
        // SAFE -> ----------------------------------
        aWriteLock.lock();
        m_eTimerType = eSuggestedTimer;
        aWriteLock.unlock();
        // <- SAFE ----------------------------------

        implts_updateTimer();
    }
    catch(const css::uno::Exception&)
    {
    }

    return 0;
}

//-----------------------------------------------
IMPL_LINK_NOARG(AutoRecovery, implts_asyncDispatch)
{
    // SAFE ->
    WriteGuard aWriteLock(m_aLock);
    DispatchParams                              aParams                = m_aDispatchParams;
    css::uno::Reference< css::uno::XInterface > xHoldRefForMethodAlive = aParams.m_xHoldRefForAsyncOpAlive;
    m_aDispatchParams.forget(); // clears all members ... including the ref-hold object .-)
    aWriteLock.unlock();
    // <- SAFE

    try
    {
        implts_dispatch(aParams);
    }
    catch (...)
    {
    }
    return 0;
}

//-----------------------------------------------
void AutoRecovery::implts_registerDocument(const css::uno::Reference< css::frame::XModel >& xDocument)
{
    // ignore corrupted events, where no document is given ... Runtime Error ?!
    if (!xDocument.is())
        return;

    CacheLockGuard aCacheLock(this, m_aLock, m_nDocCacheLock, LOCK_FOR_CACHE_USE);

    // notification for already existing document !
    // Can happen if events came in asynchronous on recovery time.
    // Then our cache was filled from the configuration ... but now we get some
    // asynchronous events from the global event broadcaster. We must be sure that
    // we dont add the same document more then once.
    AutoRecovery::TDocumentList::iterator pIt = AutoRecovery::impl_searchDocument(m_lDocCache, xDocument);
    if (pIt != m_lDocCache.end())
    {
        // Normaly nothing must be done for this "late" notification.
        // But may be the modified state was changed inbetween.
        // Check it ...
        implts_updateModifiedState(xDocument);
        return;
    }

    aCacheLock.unlock();

    ::comphelper::MediaDescriptor lDescriptor(xDocument->getArgs());

    // check if this document must be ignored for recovery !
    // Some use cases dont wish support for AutoSave/Recovery ... as e.g. OLE-Server / ActiveX Control etcpp.
    sal_Bool bNoAutoSave = lDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_NOAUTOSAVE(), (sal_Bool)(sal_False));
    if (bNoAutoSave)
        return;

    // Check if doc is well known on the desktop. Otherwhise ignore it!
    // Other frames mostly are used from external programs - e.g. the bean ...
    css::uno::Reference< css::frame::XController > xController = xDocument->getCurrentController();
    if (!xController.is())
        return;

    css::uno::Reference< css::frame::XFrame >   xFrame   = xController->getFrame();
    css::uno::Reference< css::frame::XDesktop > xDesktop (xFrame->getCreator(), css::uno::UNO_QUERY);
    if (!xDesktop.is())
        return;

    // if the document doesn't support the XDocumentRecovery interface, we're not interested in it.
    Reference< XDocumentRecovery > xDocRecovery( xDocument, UNO_QUERY );
    if ( !xDocRecovery.is() )
        return;

    // get all needed informations of this document
    // We need it to update our cache or to locate already existing elements there!
    AutoRecovery::TDocumentInfo aNew;
    aNew.Document = xDocument;

    // TODO replace getLocation() with getURL() ... its a workaround currently only!
    css::uno::Reference< css::frame::XStorable > xDoc(aNew.Document, css::uno::UNO_QUERY_THROW);
    aNew.OrgURL = xDoc->getLocation();

    css::uno::Reference< css::frame::XTitle > xTitle(aNew.Document, css::uno::UNO_QUERY_THROW);
    aNew.Title = xTitle->getTitle ();

    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    // classify the used application module, which is used by this document.
    implts_specifyAppModuleAndFactory(aNew);

    // Hack! Check for "illegal office documents" ... as e.g. the Basic IDE
    // Its not realy a full featured office document. It doesnt provide an URL, any filter, a factory URL etcpp.
    // TODO file bug to Basci IDE developers. They must remove the office document API from its service.
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
    aNew.RealFilter = lDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_FILTERNAME()  , ::rtl::OUString());

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

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    // create a new cache entry ... this document isn't known.
    ++m_nIdPool;
    aNew.ID = m_nIdPool;
    LOG_ASSERT(m_nIdPool>=0, "AutoRecovery::implts_registerDocument()\nOverflow of ID pool detected.")
    m_lDocCache.push_back(aNew);

    AutoRecovery::TDocumentList::iterator pIt1  = AutoRecovery::impl_searchDocument(m_lDocCache, xDocument);
    AutoRecovery::TDocumentInfo&          rInfo = *pIt1;

    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    implts_flushConfigItem(rInfo);
    implts_startModifyListeningOnDoc(rInfo);

    aCacheLock.unlock();
}

//-----------------------------------------------
void AutoRecovery::implts_deregisterDocument(const css::uno::Reference< css::frame::XModel >& xDocument     ,
                                                   sal_Bool                                   bStopListening)
{

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    // Attention: Dont leave SAFE section, if you work with pIt!
    // Because it points directly into the m_lDocCache list ...
    CacheLockGuard aCacheLock(this, m_aLock, m_nDocCacheLock, LOCK_FOR_CACHE_USE);

    AutoRecovery::TDocumentList::iterator pIt = AutoRecovery::impl_searchDocument(m_lDocCache, xDocument);
    if (pIt == m_lDocCache.end())
        return; // unknown document => not a runtime error! Because we register only a few documents. see registration ...

    AutoRecovery::TDocumentInfo aInfo = *pIt;

    aCacheLock.unlock();

    // Sometimes we close documents by ourself.
    // And these documents cant be deregistered.
    // Otherwhise we loos our configuration data ... but need it !
    // see SessionSave !
    if (aInfo.IgnoreClosing)
        return;

    CacheLockGuard aCacheLock2(this, m_aLock, m_nDocCacheLock, LOCK_FOR_CACHE_ADD_REMOVE);
    pIt = AutoRecovery::impl_searchDocument(m_lDocCache, xDocument);
    if (pIt != m_lDocCache.end())
        m_lDocCache.erase(pIt);
    pIt = m_lDocCache.end(); // otherwhise its not specified what pIt means!
    aCacheLock2.unlock();

    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    /* This method is called within disposing() of the document too. But there it's not a good idea to
       deregister us as listener. Furter it make no sense - because the broadcaster dies.
       So we supress deregistration in such case ...
    */
    if (bStopListening)
        implts_stopModifyListeningOnDoc(aInfo);

    AutoRecovery::st_impl_removeFile(aInfo.OldTempURL);
    AutoRecovery::st_impl_removeFile(aInfo.NewTempURL);
    implts_flushConfigItem(aInfo, sal_True); // sal_True => remove it from config
}

//-----------------------------------------------
void AutoRecovery::implts_markDocumentModifiedAgainstLastBackup(const css::uno::Reference< css::frame::XModel >& xDocument)
{
    CacheLockGuard aCacheLock(this, m_aLock, m_nDocCacheLock, LOCK_FOR_CACHE_USE);

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    AutoRecovery::TDocumentList::iterator pIt = AutoRecovery::impl_searchDocument(m_lDocCache, xDocument);
    if (pIt != m_lDocCache.end())
    {
        AutoRecovery::TDocumentInfo& rInfo = *pIt;

        /* Now we know, that this document was modified again and must be saved next time.
           But we dont need this information for every e.g. key input of the user.
           So we stop listening here.
           But if the document was saved as temp. file we start listening for this event again.
        */
        implts_stopModifyListeningOnDoc(rInfo);
    }

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void AutoRecovery::implts_updateModifiedState(const css::uno::Reference< css::frame::XModel >& xDocument)
{
    CacheLockGuard aCacheLock(this, m_aLock, m_nDocCacheLock, LOCK_FOR_CACHE_USE);

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    AutoRecovery::TDocumentList::iterator pIt = AutoRecovery::impl_searchDocument(m_lDocCache, xDocument);
    if (pIt != m_lDocCache.end())
    {
        AutoRecovery::TDocumentInfo& rInfo = *pIt;

        // use sal_True as fallback ... so we recognize every document on EmergencySave/AutoRecovery!
        sal_Bool bModified = sal_True;
        css::uno::Reference< css::util::XModifiable > xModify(xDocument, css::uno::UNO_QUERY);
        if (xModify.is())
            bModified = xModify->isModified();
        if (bModified)
        {
            rInfo.DocumentState |= AutoRecovery::E_MODIFIED;
        }
        else
        {
            rInfo.DocumentState &= ~AutoRecovery::E_MODIFIED;
        }
    }

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void AutoRecovery::implts_updateDocumentUsedForSavingState(const css::uno::Reference< css::frame::XModel >& xDocument      ,
                                                                 sal_Bool                                   bSaveInProgress)
{
    CacheLockGuard aCacheLock(this, m_aLock, m_nDocCacheLock, LOCK_FOR_CACHE_USE);

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    AutoRecovery::TDocumentList::iterator pIt = AutoRecovery::impl_searchDocument(m_lDocCache, xDocument);
    if (pIt == m_lDocCache.end())
        return;
    AutoRecovery::TDocumentInfo& rInfo = *pIt;
    rInfo.UsedForSaving = bSaveInProgress;

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void AutoRecovery::implts_markDocumentAsSaved(const css::uno::Reference< css::frame::XModel >& xDocument)
{
    CacheLockGuard aCacheLock(this, m_aLock, m_nDocCacheLock, LOCK_FOR_CACHE_USE);

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    AutoRecovery::TDocumentList::iterator pIt = AutoRecovery::impl_searchDocument(m_lDocCache, xDocument);
    if (pIt == m_lDocCache.end())
        return;
    AutoRecovery::TDocumentInfo& rInfo = *pIt;

    rInfo.DocumentState = AutoRecovery::E_UNKNOWN;
    // TODO replace getLocation() with getURL() ... its a workaround currently only!
    css::uno::Reference< css::frame::XStorable > xDoc(rInfo.Document, css::uno::UNO_QUERY);
    rInfo.OrgURL = xDoc->getLocation();

    ::rtl::OUString sRemoveURL1 = rInfo.OldTempURL;
    ::rtl::OUString sRemoveURL2 = rInfo.NewTempURL;
    rInfo.OldTempURL = ::rtl::OUString();
    rInfo.NewTempURL = ::rtl::OUString();

    ::comphelper::MediaDescriptor lDescriptor(rInfo.Document->getArgs());
    rInfo.RealFilter = lDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_FILTERNAME(), ::rtl::OUString());

    css::uno::Reference< css::frame::XTitle > xDocTitle(xDocument, css::uno::UNO_QUERY);
    if (xDocTitle.is ())
        rInfo.Title = xDocTitle->getTitle ();
    else
    {
        rInfo.Title      = lDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_TITLE()     , ::rtl::OUString());
        if (rInfo.Title.isEmpty())
            rInfo.Title  = lDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_DOCUMENTTITLE(), ::rtl::OUString());
    }

    rInfo.UsedForSaving = sal_False;

    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    implts_flushConfigItem(rInfo);

    aCacheLock.unlock();

    AutoRecovery::st_impl_removeFile(sRemoveURL1);
    AutoRecovery::st_impl_removeFile(sRemoveURL2);
}

//-----------------------------------------------
AutoRecovery::TDocumentList::iterator AutoRecovery::impl_searchDocument(      AutoRecovery::TDocumentList&               rList    ,
                                                                        const css::uno::Reference< css::frame::XModel >& xDocument)
{
    AutoRecovery::TDocumentList::iterator pIt;
    for (  pIt  = rList.begin();
           pIt != rList.end()  ;
         ++pIt                 )
    {
        const AutoRecovery::TDocumentInfo& rInfo = *pIt;
        if (rInfo.Document == xDocument)
            break;
    }
    return pIt;
}

//-----------------------------------------------
namespace
{
    void lcl_changeVisibility( const css::uno::Reference< css::frame::XFramesSupplier >& i_rFrames, sal_Bool i_bVisible )
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
}

//-----------------------------------------------
void AutoRecovery::implts_changeAllDocVisibility(sal_Bool bVisible)
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::frame::XFramesSupplier > xDesktop(xSMGR->createInstance(SERVICENAME_DESKTOP), css::uno::UNO_QUERY);
    lcl_changeVisibility( xDesktop, bVisible );

    aReadLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
/* Currently the document is not closed in case of crash,
   so the lock file must be removed explicitly
*/
void lc_removeLockFile(AutoRecovery::TDocumentInfo& rInfo)
{
    if ( rInfo.Document.is() )
    {
        try
        {
            css::uno::Reference< css::frame::XStorable > xStore(rInfo.Document, css::uno::UNO_QUERY_THROW);
            ::rtl::OUString aURL = xStore->getLocation();
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
}


//-----------------------------------------------
void AutoRecovery::implts_prepareSessionShutdown()
{
    LOG_RECOVERY("AutoRecovery::implts_prepareSessionShutdown() starts ...")

    // a) reset modified documents (of course the must be saved before this method is called!)
    // b) close it without showing any UI!

    // SAFE ->
    CacheLockGuard aCacheLock(this, m_aLock, m_nDocCacheLock, LOCK_FOR_CACHE_USE);

    AutoRecovery::TDocumentList::iterator pIt;
    for (  pIt  = m_lDocCache.begin();
           pIt != m_lDocCache.end()  ;
         ++pIt                       )
    {
        AutoRecovery::TDocumentInfo& rInfo = *pIt;

        // WORKAROUND... Since the documents are not closed the lock file must be removed explicitly
        // it is not done on documents saving since shutdown can be cancelled
        lc_removeLockFile( rInfo );

        // Prevent us from deregistration of these documents.
        // Because we close these documents by ourself (see XClosable below) ...
        // it's fact, that we reach our deregistration method. There we
        // must not(!) update our configuration ... Otherwhise all
        // session data are lost !!!
        rInfo.IgnoreClosing = sal_True;

        // reset modified flag of these documents (ignoring the notification about it!)
        // Otherwise a message box is shown on closing these models.
        implts_stopModifyListeningOnDoc(rInfo);

        // if the session save is still running the documents should not be thrown away,
        // actually that would be a bad sign, that means that the SessionManager tryes
        // to kill the session before the saving is ready
        if ((m_eJob & AutoRecovery::E_SESSION_SAVE) != AutoRecovery::E_SESSION_SAVE)
        {
            css::uno::Reference< css::util::XModifiable > xModify(rInfo.Document, css::uno::UNO_QUERY);
            if (xModify.is())
                xModify->setModified(sal_False);

            // close the model.
            css::uno::Reference< css::util::XCloseable > xClose(rInfo.Document, css::uno::UNO_QUERY);
            if (xClose.is())
            {
                try
                {
                    xClose->close(sal_False);
                }
                catch(const css::uno::Exception&)
                {
                    // At least it's only a try to close these documents before anybody else it does.
                    // So it seams to be possible to ignore any error here .-)
                }

                rInfo.Document.clear();
            }
        }
    }

    aCacheLock.unlock();
    // <- SAFE
}

//-----------------------------------------------
/* TODO WORKAROUND:

        #i64599#

        Normaly the MediaDescriptor argument NoAutoSave indicates,
        that a document must be ignored for AutoSave and Recovery.
        But sometimes XModel->getArgs() does not contained this information
        if implts_registerDocument() was called.
        So we have to check a second time, if this property is set ....
        Best place doing so is to check it immeditaly before saving
        and supressingd saving the document then.
        Of course removing the corresponding cache entry isnt an option.
        Because it would disturb iteration over the cache !
        So we ignore such documents only ...
        Hopefully next time they are not inserted in our cache.
*/
sal_Bool lc_checkIfSaveForbiddenByArguments(AutoRecovery::TDocumentInfo& rInfo)
{
    if (! rInfo.Document.is())
        return sal_True;

    ::comphelper::MediaDescriptor lDescriptor(rInfo.Document->getArgs());
    sal_Bool bNoAutoSave = lDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_NOAUTOSAVE(), (sal_Bool)(sal_False));

    return bNoAutoSave;
}

//-----------------------------------------------
AutoRecovery::ETimerType AutoRecovery::implts_saveDocs(      sal_Bool        bAllowUserIdleLoop,
                                                             sal_Bool        bRemoveLockFiles,
                                                       const DispatchParams* pParams           )
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::task::XStatusIndicator > xExternalProgress;
    if (pParams)
        xExternalProgress = pParams->m_xProgress;

    css::uno::Reference< css::frame::XFramesSupplier > xDesktop      (xSMGR->createInstance(SERVICENAME_DESKTOP), css::uno::UNO_QUERY);
    ::rtl::OUString                                    sBackupPath   (SvtPathOptions().GetBackupPath());

    css::uno::Reference< css::frame::XController > xActiveController;
    css::uno::Reference< css::frame::XModel >      xActiveModel     ;
    css::uno::Reference< css::frame::XFrame >      xActiveFrame     = xDesktop->getActiveFrame();
    if (xActiveFrame.is())
        xActiveController = xActiveFrame->getController();
    if (xActiveController.is())
        xActiveModel = xActiveController->getModel();

    // Set the default timer action for our calli.
    // Default = NORMAL_AUTOSAVE
    // We return a suggestion for an active timer only.
    // It will be ignored if the timer was disabled by the user ...
    // Further this state can be set to USER_IDLE only later in this method.
    // Its not allowed to reset such state then. Because we must know, if
    // there exists POSTPONED documents. see below ...
    AutoRecovery::ETimerType eTimer = AutoRecovery::E_NORMAL_AUTOSAVE_INTERVALL;

    sal_Int32 eJob = m_eJob;

    CacheLockGuard aCacheLock(this, m_aLock, m_nDocCacheLock, LOCK_FOR_CACHE_USE);

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    // This list will be filled with every document
    // which should be saved as last one. E.g. if it was used
    // already for an UI save operation => crashed ... and
    // now we try to save it again ... which can fail again ( of course .-) ).
    ::std::vector< AutoRecovery::TDocumentList::iterator > lDangerousDocs;

    AutoRecovery::TDocumentList::iterator pIt;
    for (  pIt  = m_lDocCache.begin();
           pIt != m_lDocCache.end()  ;
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
        // if timer is started with normnal AutoSaveTimerIntervall!
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
        //      On the other side it's not neccessary for documents, which are not modified.
        //      They can be handled normaly - means we patch the corresponding configuration entry only.
        // iii) For a SessionSave ... ignore it! There is no time to wait for this save operation.
        //      Because the WindowManager will kill the process if it doesnt react immediatly.
        //      On the other side we cant risk a concurrent save request ... because we know
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
        sal_Bool bActive       = (xActiveModel == aInfo.Document);
        sal_Bool bWasPostponed = ((aInfo.DocumentState & AutoRecovery::E_POSTPONED) == AutoRecovery::E_POSTPONED);

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
        // <- SAFE --------------------------
        aWriteLock.unlock();
        // changing of aInfo and flushing it is done inside implts_saveOneDoc!
        implts_saveOneDoc(sBackupPath, aInfo, xExternalProgress);
        implts_informListener(eJob, AutoRecovery::implst_createFeatureStateEvent(eJob, OPERATION_UPDATE, &aInfo));
        aWriteLock.lock();
        // SAFE -> --------------------------

        *pIt = aInfo;
    }

    // Did we have some "dangerous candidates" ?
    // Try to save it ... but may be it will fail !
    ::std::vector< AutoRecovery::TDocumentList::iterator >::iterator pIt2;
    for (  pIt2  = lDangerousDocs.begin();
           pIt2 != lDangerousDocs.end()  ;
         ++pIt2                          )
    {
        pIt = *pIt2;
        AutoRecovery::TDocumentInfo aInfo = *pIt;

        // <- SAFE --------------------------
        aWriteLock.unlock();
        // changing of aInfo and flushing it is done inside implts_saveOneDoc!
        implts_saveOneDoc(sBackupPath, aInfo, xExternalProgress);
        implts_informListener(eJob, AutoRecovery::implst_createFeatureStateEvent(eJob, OPERATION_UPDATE, &aInfo));
        aWriteLock.lock();
        // SAFE -> --------------------------

        *pIt = aInfo;
    }

    return eTimer;
}

//-----------------------------------------------
void AutoRecovery::implts_saveOneDoc(const ::rtl::OUString&                                    sBackupPath      ,
                                           AutoRecovery::TDocumentInfo&                        rInfo            ,
                                     const css::uno::Reference< css::task::XStatusIndicator >& xExternalProgress)
{
    // no document? => can occure if we loaded our configuration with files,
    // which couldnt be recovered successfully. In such case we have all needed informations
    // excepting the real document instance!

    // TODO: search right place, where such "dead files" can be removed from the configuration!
    if (!rInfo.Document.is())
        return;

    ::comphelper::MediaDescriptor lOldArgs(rInfo.Document->getArgs());
    implts_generateNewTempURL(sBackupPath, lOldArgs, rInfo);

    // if the document was loaded with a password, it should be
    // stored with password
    ::comphelper::MediaDescriptor lNewArgs;
    ::rtl::OUString sPassword = lOldArgs.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_PASSWORD(), ::rtl::OUString());
    if (!sPassword.isEmpty())
        lNewArgs[::comphelper::MediaDescriptor::PROP_PASSWORD()] <<= sPassword;

    // Further it must be saved using the default file format of that application.
    // Otherwhise we will some data lost.
    if (!rInfo.DefaultFilter.isEmpty())
        lNewArgs[::comphelper::MediaDescriptor::PROP_FILTERNAME()] <<= rInfo.DefaultFilter;

    // prepare frame/document/mediadescriptor in a way, that it uses OUR progress .-)
    if (xExternalProgress.is())
        lNewArgs[::comphelper::MediaDescriptor::PROP_STATUSINDICATOR()] <<= xExternalProgress;
    impl_establishProgress(rInfo, lNewArgs, css::uno::Reference< css::frame::XFrame >());

    // #i66598# use special handling of property "DocumentBaseURL" (it must be an empty string!)
    // for make hyperlinks working
    lNewArgs[::comphelper::MediaDescriptor::PROP_DOCUMENTBASEURL()] <<= ::rtl::OUString();

    // try to save this document as a new temp file everytimes.
    // Mark AutoSave state as "INCOMPLETE" if it failed.
    // Because the last temp file is to old and does not include all changes.
    Reference< XDocumentRecovery > xDocRecover(rInfo.Document, css::uno::UNO_QUERY_THROW);

    // safe the state about "trying to save"
    // ... we need it for recovery if e.g. a crash occures inside next line!
    rInfo.DocumentState |= AutoRecovery::E_TRY_SAVE;
    implts_flushConfigItem(rInfo);

    sal_Int32 nRetry = RETRY_STORE_ON_FULL_DISC_FOREVER;
    sal_Bool  bError = sal_False;
    do
    {
        try
        {
            xDocRecover->storeToRecoveryFile( rInfo.NewTempURL, lNewArgs.getAsConstPropertyValueList() );

#ifdef TRIGGER_FULL_DISC_CHECK
            throw css::uno::Exception();
#else  // TRIGGER_FULL_DISC_CHECK

            bError = sal_False;
            nRetry = 0;
#endif // TRIGGER_FULL_DISC_CHECK
        }
        catch(const css::uno::Exception&)
        {
            bError = sal_True;

            // a) FULL DISC seams to be the problem behind                              => show error and retry it forever (e.g. retry=300)
            // b) unknown problem (may be locking problem)                              => reset RETRY value to more usefull value(!) (e.g. retry=3)
            // c) unknown problem (may be locking problem) + 1..2 repeating operations  => throw the original exception to force generation of a stacktrace !

            // SAFE ->
            ReadGuard aReadLock2(m_aLock);
            sal_Int32 nMinSpaceDocSave = m_nMinSpaceDocSave;
            aReadLock2.unlock();
            // <- SAFE

            if (! impl_enoughDiscSpace(nMinSpaceDocSave))
                AutoRecovery::impl_showFullDiscError();
            else
            if (nRetry > RETRY_STORE_ON_MIGHT_FULL_DISC_USEFULL)
                nRetry = RETRY_STORE_ON_MIGHT_FULL_DISC_USEFULL;
            else
            if (nRetry <= GIVE_UP_RETRY)
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
        rInfo.DocumentState |=  AutoRecovery::E_SUCCEDED;
    }
    else
    {
        // safe the state about error ...
        rInfo.NewTempURL     = ::rtl::OUString();
        rInfo.DocumentState &= ~AutoRecovery::E_TRY_SAVE;
        rInfo.DocumentState |=  AutoRecovery::E_HANDLED;
        rInfo.DocumentState |=  AutoRecovery::E_INCOMPLETE;
    }

    // make sure the progress isnt referred any longer
    impl_forgetProgress(rInfo, lNewArgs, css::uno::Reference< css::frame::XFrame >());

    // try to remove the old temp file.
    // Ignore any error here. We have a new temp file, which is up to date.
    // The only thing is: we fill the disk with temp files, if we cant remove old ones :-)
    ::rtl::OUString sRemoveFile      = rInfo.OldTempURL;
                    rInfo.OldTempURL = rInfo.NewTempURL;
                    rInfo.NewTempURL = ::rtl::OUString();

    implts_flushConfigItem(rInfo);

    // We must know if the user modifies the document again ...
    implts_startModifyListeningOnDoc(rInfo);

    AutoRecovery::st_impl_removeFile(sRemoveFile);
}

//-----------------------------------------------
AutoRecovery::ETimerType AutoRecovery::implts_openDocs(const DispatchParams& aParams)
{
    AutoRecovery::ETimerType eTimer = AutoRecovery::E_DONT_START_TIMER;

    CacheLockGuard aCacheLock(this, m_aLock, m_nDocCacheLock, LOCK_FOR_CACHE_USE);

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    sal_Int32                             eJob = m_eJob;
    AutoRecovery::TDocumentList::iterator pIt;
    for (  pIt  = m_lDocCache.begin();
           pIt != m_lDocCache.end()  ;
         ++pIt                       )
    {
        AutoRecovery::TDocumentInfo& rInfo = *pIt;

        // Such documents are already loaded by the last loop.
        // Dont check E_SUCCEDED here! Its may be the final state of an AutoSave
        // operation before!!!
        if ((rInfo.DocumentState & AutoRecovery::E_HANDLED) == AutoRecovery::E_HANDLED)
            continue;

        // a1,b1,c1,d2,e2,f2)
        if ((rInfo.DocumentState & AutoRecovery::E_DAMAGED) == AutoRecovery::E_DAMAGED)
        {
            // dont forget to inform listener! May be this document was
            // damaged on last saving time ...
            // Then our listener need this notification.
            // If it was damaged during last "try to open" ...
            // it will be notified more then once. SH.. HAPPENS ...
            // <- SAFE --------------------------
            aWriteLock.unlock();
            implts_informListener(eJob,
                AutoRecovery::implst_createFeatureStateEvent(eJob, OPERATION_UPDATE, &rInfo));
            aWriteLock.lock();
            // SAFE -> --------------------------
            continue;
        }

        ::comphelper::MediaDescriptor lDescriptor;

        // its an UI feature - so the "USER" itself must be set as referer
        lDescriptor[::comphelper::MediaDescriptor::PROP_REFERRER()] <<= ::rtl::OUString(REFERRER_USER);
        lDescriptor[::comphelper::MediaDescriptor::PROP_SALVAGEDFILE()] <<= ::rtl::OUString();

        // recovered documents are loaded hidden, and shown all at once, later
        lDescriptor[::comphelper::MediaDescriptor::PROP_HIDDEN()] <<= true;

        if (aParams.m_xProgress.is())
            lDescriptor[::comphelper::MediaDescriptor::PROP_STATUSINDICATOR()] <<= aParams.m_xProgress;

        sal_Bool bBackupWasTried   = (
                                        ((rInfo.DocumentState & AutoRecovery::E_TRY_LOAD_BACKUP  ) == AutoRecovery::E_TRY_LOAD_BACKUP) || // temp. state!
                                        ((rInfo.DocumentState & AutoRecovery::E_INCOMPLETE       ) == AutoRecovery::E_INCOMPLETE     )    // transport TRY_LOAD_BACKUP from last loop to this new one!
                                     );
        sal_Bool bOriginalWasTried = ((rInfo.DocumentState & AutoRecovery::E_TRY_LOAD_ORIGINAL) == AutoRecovery::E_TRY_LOAD_ORIGINAL);

        if (bBackupWasTried)
        {
            if (!bOriginalWasTried)
            {
                rInfo.DocumentState |= AutoRecovery::E_INCOMPLETE;
                // try original URL ... ! dont continue with next item here ...
            }
            else
            {
                rInfo.DocumentState |= AutoRecovery::E_DAMAGED;
                continue;
            }
        }

        ::rtl::OUString sLoadOriginalURL;
        ::rtl::OUString sLoadBackupURL  ;

        if (!bBackupWasTried)
            sLoadBackupURL = rInfo.OldTempURL;

        if (!rInfo.OrgURL.isEmpty())
        {
            sLoadOriginalURL = rInfo.OrgURL;
        }
        else
        if (!rInfo.TemplateURL.isEmpty())
        {
            sLoadOriginalURL = rInfo.TemplateURL;
            lDescriptor[::comphelper::MediaDescriptor::PROP_ASTEMPLATE()]   <<= sal_True;
            lDescriptor[::comphelper::MediaDescriptor::PROP_TEMPLATENAME()] <<= rInfo.TemplateURL;
        }
        else
        if (!rInfo.FactoryURL.isEmpty())
        {
            sLoadOriginalURL = rInfo.FactoryURL;
            lDescriptor[::comphelper::MediaDescriptor::PROP_ASTEMPLATE()] <<= sal_True;
        }

        // A "Salvaged" item must exists every time. The core can make something special then for recovery.
        // Of course it should be the real file name of the original file, in case we load the temp. backup here.
        ::rtl::OUString sURL;
        if (!sLoadBackupURL.isEmpty())
        {
            sURL = sLoadBackupURL;
            rInfo.DocumentState |= AutoRecovery::E_TRY_LOAD_BACKUP;
            lDescriptor[::comphelper::MediaDescriptor::PROP_SALVAGEDFILE()] <<= sLoadOriginalURL;
        }
        else if (!sLoadOriginalURL.isEmpty())
        {
            sURL = sLoadOriginalURL;
            rInfo.DocumentState |= AutoRecovery::E_TRY_LOAD_ORIGINAL;
        }
        else
            continue; // TODO ERROR!

        LoadEnv::initializeUIDefaults( m_xSMGR, lDescriptor, true, NULL );

        // <- SAFE ------------------------------
        aWriteLock.unlock();

        implts_flushConfigItem(rInfo);
        implts_informListener(eJob,
            AutoRecovery::implst_createFeatureStateEvent(eJob, OPERATION_UPDATE, &rInfo));

        try
        {
            implts_openOneDoc(sURL, lDescriptor, rInfo);
        }
        catch(const css::uno::Exception&)
        {
            rInfo.DocumentState &= ~AutoRecovery::E_TRY_LOAD_BACKUP;
            rInfo.DocumentState &= ~AutoRecovery::E_TRY_LOAD_ORIGINAL;
            if (!sLoadBackupURL.isEmpty())
            {
                rInfo.DocumentState |= AutoRecovery::E_INCOMPLETE;
                eTimer               = AutoRecovery::E_CALL_ME_BACK;
            }
            else
            {
                rInfo.DocumentState |=  AutoRecovery::E_HANDLED;
                rInfo.DocumentState |=  AutoRecovery::E_DAMAGED;
            }

            implts_flushConfigItem(rInfo, sal_True);
            implts_informListener(eJob,
                AutoRecovery::implst_createFeatureStateEvent(eJob, OPERATION_UPDATE, &rInfo));

            // SAFE -> ------------------------------
            // Needed for next loop!
            aWriteLock.lock();
            continue;
        }

        if (!rInfo.RealFilter.isEmpty())
        {
            ::comphelper::MediaDescriptor lPatchDescriptor(rInfo.Document->getArgs());
            lPatchDescriptor[::comphelper::MediaDescriptor::PROP_FILTERNAME()] <<= rInfo.RealFilter;
            rInfo.Document->attachResource(rInfo.Document->getURL(), lPatchDescriptor.getAsConstPropertyValueList());
                // do *not* use sURL here. In case this points to the recovery file, it has already been passed
                // to recoverFromFile. Also, passing it here is logically wrong, as attachResource is intended
                // to take the logical file URL.
        }

        css::uno::Reference< css::util::XModifiable > xModify(rInfo.Document, css::uno::UNO_QUERY);
        if ( xModify.is() )
        {
            sal_Bool bModified = ((rInfo.DocumentState & AutoRecovery::E_MODIFIED) == AutoRecovery::E_MODIFIED);
            xModify->setModified(bModified);
        }

        rInfo.DocumentState &= ~AutoRecovery::E_TRY_LOAD_BACKUP;
        rInfo.DocumentState &= ~AutoRecovery::E_TRY_LOAD_ORIGINAL;
        rInfo.DocumentState |=  AutoRecovery::E_HANDLED;
        rInfo.DocumentState |=  AutoRecovery::E_SUCCEDED;

        implts_flushConfigItem(rInfo);
        implts_informListener(eJob,
            AutoRecovery::implst_createFeatureStateEvent(eJob, OPERATION_UPDATE, &rInfo));

        /* Normaly we listen as XModifyListener on a document to know if a document was changed
           since our last AutoSave. And we deregister us in case we know this state.
           But directly after one document as recovered ... we must start listening.
           Otherwhise the first "modify" doesnt reach us. Because we ourself called setModified()
           on the document via API. And currently we dont listen for any events (not at the GlobalEventBroadcaster
           nor at any document!).
        */
        implts_startModifyListeningOnDoc(rInfo);

        // SAFE -> ------------------------------
        // Needed for next loop. Dont unlock it again!
        aWriteLock.lock();
    }

    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    return eTimer;
}

//-----------------------------------------------
void AutoRecovery::implts_openOneDoc(const ::rtl::OUString&               sURL       ,
                                           ::comphelper::MediaDescriptor& lDescriptor,
                                           AutoRecovery::TDocumentInfo&   rInfo      )
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::frame::XFrame > xDesktop( xSMGR->createInstance(SERVICENAME_DESKTOP), css::uno::UNO_QUERY_THROW );

    ::std::vector< Reference< XComponent > > aCleanup;
    try
    {
        // create a new document of the desired type
        Reference< XModel2 > xModel( xSMGR->createInstance( rInfo.FactoryService ), UNO_QUERY_THROW );
        aCleanup.push_back( xModel.get() );

        // put the filter name into the descriptor - we're not going to involve any type detection, so
        // the document might be lost without the FilterName property
        if ( (rInfo.DocumentState & AutoRecovery::E_TRY_LOAD_ORIGINAL) == AutoRecovery::E_TRY_LOAD_ORIGINAL)
            lDescriptor[ ::comphelper::MediaDescriptor::PROP_FILTERNAME() ] <<= rInfo.RealFilter;
        else
            lDescriptor[ ::comphelper::MediaDescriptor::PROP_FILTERNAME() ] <<= rInfo.DefaultFilter;

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
                lDescriptor.getUnpackedValueOrDefault( ::comphelper::MediaDescriptor::PROP_SALVAGEDFILE(), ::rtl::OUString() ),
                lDescriptor.getAsConstPropertyValueList()
            );

            // No attachResource needed here. By definition (of XDocumentRecovery), the implementation is responsible
            // for completely initializing the model, which includes attachResource (or equivalent), if required.
        }

        // re-create all the views
        ::std::vector< ::rtl::OUString > aViewsToRestore( rInfo.ViewNames.getLength() );
        if ( rInfo.ViewNames.getLength() )
            ::std::copy( rInfo.ViewNames.getConstArray(), rInfo.ViewNames.getConstArray() + rInfo.ViewNames.getLength(), aViewsToRestore.begin() );
        // if we don't have views for whatever reason, then create a default-view, at least
        if ( aViewsToRestore.empty() )
            aViewsToRestore.push_back( ::rtl::OUString() );

        for (   ::std::vector< ::rtl::OUString >::const_iterator viewName = aViewsToRestore.begin();
                viewName != aViewsToRestore.end();
                ++viewName
            )
        {
            // create a frame
            Reference< XFrame > xTargetFrame = xDesktop->findFrame( SPECIALTARGET_BLANK, 0 );
            aCleanup.push_back( xTargetFrame.get() );

            // create a view to the document
            Reference< XController2 > xController;
            if ( viewName->getLength() )
            {
                xController.set( xModel->createViewController( *viewName, Sequence< PropertyValue >(), xTargetFrame ), UNO_SET_THROW );
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
        for (   ::std::vector< Reference< XComponent > >::const_iterator component = aCleanup.begin();
                component != aCleanup.end();
                ++component
            )
        {
            css::uno::Reference< css::util::XCloseable > xClose( *component, css::uno::UNO_QUERY );
            if ( xClose.is() )
                xClose->close( sal_True );
            else
                (*component)->dispose();
        }

        // re-throw
        ::rtl::OUStringBuffer sMsg(256);
        sMsg.appendAscii("Recovery of \"");
        sMsg.append     (sURL            );
        sMsg.appendAscii("\" failed."    );

        throw css::lang::WrappedTargetException(
            sMsg.makeStringAndClear(),
            static_cast< css::frame::XDispatch* >(this),
            aCaughtException
        );
    }
}

//-----------------------------------------------
void AutoRecovery::implts_generateNewTempURL(const ::rtl::OUString&               sBackupPath     ,
                                                   ::comphelper::MediaDescriptor& /*rMediaDescriptor*/,
                                                   AutoRecovery::TDocumentInfo&   rInfo           )
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    // specify URL for saving (which points to a temp file inside backup directory)
    // and define an unique name, so we can locate it later.
    // This unique name must solve an optimization problem too!
    // In case we are asked to save unmodified documents too - and one of them
    // is an empty one (because it was new created using e.g. an URL private:factory/...)
    // we should not save it realy. Then we put the information about such "empty document"
    // into the configuration and dont create any recovery file on disk.
    // We use the title of the document to make it unique.
    ::rtl::OUStringBuffer sUniqueName;
    if (!rInfo.OrgURL.isEmpty())
    {
        css::uno::Reference< css::util::XURLTransformer > xParser(css::util::URLTransformer::create(::comphelper::getComponentContext(m_xSMGR)));
        css::util::URL aURL;
        aURL.Complete = rInfo.OrgURL;
        xParser->parseStrict(aURL);
        sUniqueName.append(aURL.Name);
    }
    else if (!rInfo.FactoryURL.isEmpty())
        sUniqueName.appendAscii("untitled");
    sUniqueName.appendAscii("_");

    // TODO: Must we strip some illegal signes - if we use the title?

    rtl::OUString sName(sUniqueName.makeStringAndClear());
    String sExtension(rInfo.Extension);
    String sPath(sBackupPath);
    ::utl::TempFile aTempFile(sName, &sExtension, &sPath);

    rInfo.NewTempURL = aTempFile.GetURL();
}

//-----------------------------------------------
void AutoRecovery::implts_informListener(      sal_Int32                      eJob  ,
                                         const css::frame::FeatureStateEvent& aEvent)
{
    // Helper shares mutex with us -> threadsafe!
    ::cppu::OInterfaceContainerHelper* pListenerForURL = 0;
    ::rtl::OUString                    sJob            = AutoRecovery::implst_getJobDescription(eJob);

    // inform listener, which are registered for any URLs(!)
    pListenerForURL = m_lListener.getContainer(sJob);
    if(pListenerForURL != 0)
    {
        ::cppu::OInterfaceIteratorHelper pIt(*pListenerForURL);
        while(pIt.hasMoreElements())
        {
            try
            {
                css::uno::Reference< css::frame::XStatusListener > xListener(((css::frame::XStatusListener*)pIt.next()), css::uno::UNO_QUERY);
                xListener->statusChanged(aEvent);
            }
            catch(const css::uno::RuntimeException&)
            {
                pIt.remove();
            }
        }
    }
}

//-----------------------------------------------
::rtl::OUString AutoRecovery::implst_getJobDescription(sal_Int32 eJob)
{
    // describe the current running operation
    ::rtl::OUStringBuffer sFeature(256);
    sFeature.appendAscii(RTL_CONSTASCII_STRINGPARAM(CMD_PROTOCOL));

    // Attention: Because "eJob" is used as a flag field the order of checking these
    // flags is importent. We must preferr job with higher priorities!
    // E.g. EmergencySave has an higher prio then AutoSave ...
    // On the other side there exist a well defined order between two different jobs.
    // e.g. PrepareEmergencySave must be done before EmergencySave is started of course.

    if ((eJob & AutoRecovery::E_PREPARE_EMERGENCY_SAVE) == AutoRecovery::E_PREPARE_EMERGENCY_SAVE)
        sFeature.appendAscii(RTL_CONSTASCII_STRINGPARAM(CMD_DO_PREPARE_EMERGENCY_SAVE));
    else
    if ((eJob & AutoRecovery::E_EMERGENCY_SAVE) == AutoRecovery::E_EMERGENCY_SAVE)
        sFeature.appendAscii(RTL_CONSTASCII_STRINGPARAM(CMD_DO_EMERGENCY_SAVE));
    else
    if ((eJob & AutoRecovery::E_RECOVERY) == AutoRecovery::E_RECOVERY)
        sFeature.appendAscii(RTL_CONSTASCII_STRINGPARAM(CMD_DO_RECOVERY));
    else
    if ((eJob & AutoRecovery::E_SESSION_SAVE) == AutoRecovery::E_SESSION_SAVE)
        sFeature.appendAscii(RTL_CONSTASCII_STRINGPARAM(CMD_DO_SESSION_SAVE));
    else
    if ((eJob & AutoRecovery::E_SESSION_QUIET_QUIT) == AutoRecovery::E_SESSION_QUIET_QUIT)
        sFeature.appendAscii(RTL_CONSTASCII_STRINGPARAM(CMD_DO_SESSION_QUIET_QUIT));
    else
    if ((eJob & AutoRecovery::E_SESSION_RESTORE) == AutoRecovery::E_SESSION_RESTORE)
        sFeature.appendAscii(RTL_CONSTASCII_STRINGPARAM(CMD_DO_SESSION_RESTORE));
    else
    if ((eJob & AutoRecovery::E_ENTRY_BACKUP) == AutoRecovery::E_ENTRY_BACKUP)
        sFeature.appendAscii(RTL_CONSTASCII_STRINGPARAM(CMD_DO_ENTRY_BACKUP));
    else
    if ((eJob & AutoRecovery::E_ENTRY_CLEANUP) == AutoRecovery::E_ENTRY_CLEANUP)
        sFeature.appendAscii(RTL_CONSTASCII_STRINGPARAM(CMD_DO_ENTRY_CLEANUP));
    else
    if ((eJob & AutoRecovery::E_AUTO_SAVE) == AutoRecovery::E_AUTO_SAVE)
        sFeature.appendAscii(RTL_CONSTASCII_STRINGPARAM(CMD_DO_AUTO_SAVE));
    #ifdef ENABLE_WARNINGS
    else if ( eJob != AutoRecovery::E_NO_JOB )
        LOG_WARNING("AutoRecovery::implst_getJobDescription()", "Invalid job identifier detected.")
    #endif

    return sFeature.makeStringAndClear();
}

//-----------------------------------------------
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

    LOG_WARNING("AutoRecovery::implts_classifyJob()", "Invalid URL (protocol).")
    return AutoRecovery::E_NO_JOB;
}

//-----------------------------------------------
css::frame::FeatureStateEvent AutoRecovery::implst_createFeatureStateEvent(      sal_Int32                    eJob      ,
                                                                           const ::rtl::OUString&             sEventType,
                                                                                 AutoRecovery::TDocumentInfo* pInfo     )
{
    css::frame::FeatureStateEvent aEvent;
    aEvent.FeatureURL.Complete   = AutoRecovery::implst_getJobDescription(eJob);
    aEvent.FeatureDescriptor     = sEventType;

    if (pInfo && sEventType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(OPERATION_UPDATE)))
    {
        // pack rInfo for transport via UNO
        ::comphelper::NamedValueCollection aInfo;
        aInfo.put( rtl::OUString(CFG_ENTRY_PROP_ID), pInfo->ID );
        aInfo.put( rtl::OUString(CFG_ENTRY_PROP_ORIGINALURL), pInfo->OrgURL );
        aInfo.put( rtl::OUString(CFG_ENTRY_PROP_FACTORYURL), pInfo->FactoryURL );
        aInfo.put( rtl::OUString(CFG_ENTRY_PROP_TEMPLATEURL), pInfo->TemplateURL );
        aInfo.put( rtl::OUString(CFG_ENTRY_PROP_TEMPURL), pInfo->OldTempURL.isEmpty() ? pInfo->NewTempURL : pInfo->OldTempURL );
        aInfo.put( rtl::OUString(CFG_ENTRY_PROP_MODULE), pInfo->AppModule) ;
        aInfo.put( rtl::OUString(CFG_ENTRY_PROP_TITLE), pInfo->Title);
        aInfo.put( rtl::OUString(CFG_ENTRY_PROP_VIEWNAMES), pInfo->ViewNames);
        aInfo.put( rtl::OUString(CFG_ENTRY_PROP_DOCUMENTSTATE), pInfo->DocumentState);

        aEvent.State <<= aInfo.getPropertyValues();
    }

    return aEvent;
}

//-----------------------------------------------
void AutoRecovery::implts_resetHandleStates(sal_Bool /*bLoadCache*/)
{
    CacheLockGuard aCacheLock(this, m_aLock, m_nDocCacheLock, LOCK_FOR_CACHE_USE);

    // SAFE -> ------------------------------
    WriteGuard aWriteLock(m_aLock);

    AutoRecovery::TDocumentList::iterator pIt;
    for (  pIt  = m_lDocCache.begin();
           pIt != m_lDocCache.end()  ;
         ++pIt                       )
    {
        AutoRecovery::TDocumentInfo& rInfo = *pIt;
        rInfo.DocumentState &= ~AutoRecovery::E_HANDLED  ;
        rInfo.DocumentState &= ~AutoRecovery::E_POSTPONED;

        // SAFE -> ------------------------------
        aWriteLock.unlock();
        implts_flushConfigItem(rInfo);
        aWriteLock.lock();
        // <- SAFE ------------------------------
    }

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void AutoRecovery::implts_prepareEmergencySave()
{
    // Be sure to know all open documents realy .-)
    implts_verifyCacheAgainstDesktopDocumentList();

    // hide all docs, so the user cant disturb our emergency save .-)
    implts_changeAllDocVisibility(sal_False);
}

//-----------------------------------------------
void AutoRecovery::implts_doEmergencySave(const DispatchParams& aParams)
{
    // Write a hint "we chrashed" into the configuration, so
    // the error report tool is started too in case no recovery
    // documents exists and was saved.
    ::comphelper::ConfigurationHelper::writeDirectKey(
        m_xSMGR,
        rtl::OUString(CFG_PACKAGE_RECOVERY),
        rtl::OUString(CFG_PATH_RECOVERYINFO),
        rtl::OUString(CFG_ENTRY_CRASHED),
        css::uno::makeAny(sal_True),
        ::comphelper::ConfigurationHelper::E_STANDARD);

    // for all docs, store their current view/names in the configurtion
    implts_persistAllActiveViewNames();

    // The called method for saving documents runs
    // during normal AutoSave more then once. Because
    // it postpone active documents and save it later.
    // That is normaly done by recalling it from a timer.
    // Here we must do it immediatly!
    // Of course this method returns the right state -
    // because it knows, that we are running in ERMERGENCY SAVE mode .-)

    sal_Bool                 bAllowUserIdleLoop = sal_False; // not allowed to change that .-)
    AutoRecovery::ETimerType eSuggestedTimer    = AutoRecovery::E_DONT_START_TIMER;
    do
    {
        eSuggestedTimer = implts_saveDocs(bAllowUserIdleLoop, sal_True, &aParams);
    }
    while(eSuggestedTimer == AutoRecovery::E_CALL_ME_BACK);

    // reset the handle state of all
    // cache items. Such handle state indicates, that a document
    // was already saved during the THIS(!) EmergencySave session.
    // Of course following recovery session must be started without
    // any "handle" state ...
    implts_resetHandleStates(sal_False);

    // flush config cached back to disc.
    impl_flushALLConfigChanges();

    // try to make sure next time office will be started user wont be
    // notified about any other might be running office instance
    // remove ".lock" file from disc !
    AutoRecovery::st_impl_removeLockFile();
}

//-----------------------------------------------
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
    implts_resetHandleStates(sal_True);

    // Reset the configuration hint "we was crashed"!
    ::comphelper::ConfigurationHelper::writeDirectKey(
        m_xSMGR,
        rtl::OUString(CFG_PACKAGE_RECOVERY),
        rtl::OUString(CFG_PATH_RECOVERYINFO),
        rtl::OUString(CFG_ENTRY_CRASHED),
        css::uno::makeAny(sal_False),
        ::comphelper::ConfigurationHelper::E_STANDARD);
}

//-----------------------------------------------
void AutoRecovery::implts_doSessionSave(const DispatchParams& aParams)
{
    LOG_RECOVERY("AutoRecovery::implts_doSessionSave()")

    // Be sure to know all open documents realy .-)
    implts_verifyCacheAgainstDesktopDocumentList();

    // for all docs, store their current view/names in the configurtion
    implts_persistAllActiveViewNames();

    // The called method for saving documents runs
    // during normal AutoSave more then once. Because
    // it postpone active documents and save it later.
    // That is normaly done by recalling it from a timer.
    // Here we must do it immediatly!
    // Of course this method returns the right state -
    // because it knows, that we are running in SESSION SAVE mode .-)

    sal_Bool                 bAllowUserIdleLoop = sal_False; // not allowed to change that .-)
    AutoRecovery::ETimerType eSuggestedTimer    = AutoRecovery::E_DONT_START_TIMER;
    do
    {
        // do not remove lock files of the documents, it will be done on session quit
        eSuggestedTimer = implts_saveDocs(bAllowUserIdleLoop, sal_False, &aParams);
    }
    while(eSuggestedTimer == AutoRecovery::E_CALL_ME_BACK);

    // reset the handle state of all
    // cache items. Such handle state indicates, that a document
    // was already saved during the THIS(!) save session.
    // Of course following restore session must be started without
    // any "handle" state ...
    implts_resetHandleStates(sal_False);

    // flush config cached back to disc.
    impl_flushALLConfigChanges();
}

//-----------------------------------------------
void AutoRecovery::implts_doSessionQuietQuit(const DispatchParams& /*aParams*/)
{
    LOG_RECOVERY("AutoRecovery::implts_doSessionQuietQuit()")

    // try to make sure next time office will be started user wont be
    // notified about any other might be running office instance
    // remove ".lock" file from disc !
    // it is done as a first action for session save since Gnome sessions
    // do not provide enough time for shutdown, and the dialog looks to be
    // confusing for the user
    AutoRecovery::st_impl_removeLockFile();

    // reset all modified documents, so the dont show any UI on closing ...
    // and close all documents, so we can shutdown the OS!
    implts_prepareSessionShutdown();

    // Write a hint for "stored session data" into the configuration, so
    // the on next startup we know what's happen last time
    ::comphelper::ConfigurationHelper::writeDirectKey(
        m_xSMGR,
        rtl::OUString(CFG_PACKAGE_RECOVERY),
        rtl::OUString(CFG_PATH_RECOVERYINFO),
        rtl::OUString(CFG_ENTRY_SESSIONDATA),
        css::uno::makeAny(sal_True),
        ::comphelper::ConfigurationHelper::E_STANDARD);

    // flush config cached back to disc.
    impl_flushALLConfigChanges();
}


//-----------------------------------------------
void AutoRecovery::implts_doSessionRestore(const DispatchParams& aParams)
{
    LOG_RECOVERY("AutoRecovery::implts_doSessionRestore() ...")

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
    implts_resetHandleStates(sal_True);

    // make all opened documents visible
    implts_changeAllDocVisibility(sal_True);

    // Reset the configuration hint for "session save"!
    LOG_RECOVERY("... reset config key 'SessionData'")
    ::comphelper::ConfigurationHelper::writeDirectKey(
        m_xSMGR,
        rtl::OUString(CFG_PACKAGE_RECOVERY),
        rtl::OUString(CFG_PATH_RECOVERYINFO),
        rtl::OUString(CFG_ENTRY_SESSIONDATA),
        css::uno::makeAny(sal_False),
        ::comphelper::ConfigurationHelper::E_STANDARD);

    LOG_RECOVERY("... AutoRecovery::implts_doSessionRestore()")
}

//-----------------------------------------------
void AutoRecovery::implts_backupWorkingEntry(const DispatchParams& aParams)
{
    CacheLockGuard aCacheLock(this, m_aLock, m_nDocCacheLock, LOCK_FOR_CACHE_USE);

    AutoRecovery::TDocumentList::iterator pIt;
    for (  pIt  = m_lDocCache.begin();
           pIt != m_lDocCache.end()  ;
         ++pIt                       )
    {
        const AutoRecovery::TDocumentInfo& rInfo = *pIt;
        if (rInfo.ID != aParams.m_nWorkingEntryID)
            continue;

        ::rtl::OUString sSourceURL;
        // Prefer temp file. It contains the changes against the original document!
        if (!rInfo.OldTempURL.isEmpty())
            sSourceURL = rInfo.OldTempURL;
        else if (!rInfo.NewTempURL.isEmpty())
            sSourceURL = rInfo.NewTempURL;
        else if (!rInfo.OrgURL.isEmpty())
            sSourceURL = rInfo.OrgURL;
        else
            continue; // nothing real to save! An unmodified but new created document.

        INetURLObject aParser(sSourceURL);
        // AutoRecovery::EFailureSafeResult eResult =
        implts_copyFile(sSourceURL, aParams.m_sSavePath, aParser.getName());

        // TODO: Check eResult and react for errors (InteractionHandler!?)
        // Currently we ignore it ...
        // DONT UPDATE THE CACHE OR REMOVE ANY TEMP. FILES FROM DISK.
        // That has to be forced from outside explicitly.
        // See implts_cleanUpWorkingEntry() for further details.
    }
}

//-----------------------------------------------
void AutoRecovery::implts_cleanUpWorkingEntry(const DispatchParams& aParams)
{
    CacheLockGuard aCacheLock(this, m_aLock, m_nDocCacheLock, LOCK_FOR_CACHE_ADD_REMOVE);

    AutoRecovery::TDocumentList::iterator pIt;
    for (  pIt  = m_lDocCache.begin();
           pIt != m_lDocCache.end()  ;
         ++pIt                       )
    {
        AutoRecovery::TDocumentInfo& rInfo = *pIt;
        if (rInfo.ID != aParams.m_nWorkingEntryID)
            continue;

        AutoRecovery::st_impl_removeFile(rInfo.OldTempURL);
        AutoRecovery::st_impl_removeFile(rInfo.NewTempURL);
        implts_flushConfigItem(rInfo, sal_True); // sal_True => remove it from xml config!

        m_lDocCache.erase(pIt);
        break; /// !!! pIt is not defined any longer ... further this function has finished it's work
    }
}

//-----------------------------------------------
AutoRecovery::EFailureSafeResult AutoRecovery::implts_copyFile(const ::rtl::OUString& sSource    ,
                                                               const ::rtl::OUString& sTargetPath,
                                                               const ::rtl::OUString& sTargetName)
{
    // create content for the parent folder and call transfer on that content with the source content
    // and the destination file name as parameters

    css::uno::Reference< css::ucb::XCommandEnvironment > xEnvironment;

    ::ucbhelper::Content aSourceContent;
    ::ucbhelper::Content aTargetContent;

    try
    {
        aTargetContent = ::ucbhelper::Content(sTargetPath, xEnvironment, comphelper::getComponentContext(m_xSMGR));
    }
    catch(const css::uno::Exception&)
    {
        return AutoRecovery::E_WRONG_TARGET_PATH;
    }

    sal_Int32 nNameClash;
    nNameClash = css::ucb::NameClash::RENAME;

    try
    {
        ::ucbhelper::Content::create(sSource, xEnvironment, comphelper::getComponentContext(m_xSMGR), aSourceContent);
        aTargetContent.transferContent(aSourceContent, ::ucbhelper::InsertOperation_COPY, sTargetName, nNameClash);
    }
    catch(const css::uno::Exception&)
    {
        return AutoRecovery::E_ORIGINAL_FILE_MISSING;
    }

    return AutoRecovery::E_COPIED;
}

//-----------------------------------------------
sal_Bool SAL_CALL AutoRecovery::convertFastPropertyValue(      css::uno::Any& /*aConvertedValue*/,
                                                               css::uno::Any& /*aOldValue*/      ,
                                                               sal_Int32      /*nHandle*/        ,
                                                         const css::uno::Any& /*aValue*/         )
    throw(css::lang::IllegalArgumentException)
{
    // not needed currently
    return sal_False;
}

//-----------------------------------------------
void SAL_CALL AutoRecovery::setFastPropertyValue_NoBroadcast(      sal_Int32      /*nHandle*/,
                                                             const css::uno::Any& /*aValue*/ )
    throw(css::uno::Exception)
{
    // not needed currently
}

//-----------------------------------------------
void SAL_CALL AutoRecovery::getFastPropertyValue(css::uno::Any& aValue ,
                                                 sal_Int32      nHandle) const
{
    switch(nHandle)
    {
        case AUTORECOVERY_PROPHANDLE_EXISTS_RECOVERYDATA :
                {
                    sal_Bool bSessionData  = sal_False;
                    ::comphelper::ConfigurationHelper::readDirectKey(
                                                    m_xSMGR,
                                                    rtl::OUString(CFG_PACKAGE_RECOVERY),
                                                    rtl::OUString(CFG_PATH_RECOVERYINFO),
                                                    rtl::OUString(CFG_ENTRY_SESSIONDATA),
                                                    ::comphelper::ConfigurationHelper::E_READONLY) >>= bSessionData;

                    sal_Bool bRecoveryData = ((sal_Bool)(m_lDocCache.size()>0));

                    // exists session data ... => then we cant say, that these
                    // data are valid for recovery. So we have to return sal_False then!
                    if (bSessionData)
                        bRecoveryData = sal_False;

                    aValue <<= bRecoveryData;
                }
                break;

        case AUTORECOVERY_PROPHANDLE_CRASHED :
                aValue = ::comphelper::ConfigurationHelper::readDirectKey(
                            m_xSMGR,
                            rtl::OUString(CFG_PACKAGE_RECOVERY),
                            rtl::OUString(CFG_PATH_RECOVERYINFO),
                            rtl::OUString(CFG_ENTRY_CRASHED),
                            ::comphelper::ConfigurationHelper::E_READONLY);
                break;

        case AUTORECOVERY_PROPHANDLE_EXISTS_SESSIONDATA :
                aValue = ::comphelper::ConfigurationHelper::readDirectKey(
                            m_xSMGR,
                            rtl::OUString(CFG_PACKAGE_RECOVERY),
                            rtl::OUString(CFG_PATH_RECOVERYINFO),
                            rtl::OUString(CFG_ENTRY_SESSIONDATA),
                            ::comphelper::ConfigurationHelper::E_READONLY);
                break;
    }
}

//-----------------------------------------------
const css::uno::Sequence< css::beans::Property > impl_getStaticPropertyDescriptor()
{
    const css::beans::Property pPropertys[] =
    {
        css::beans::Property( AUTORECOVERY_PROPNAME_CRASHED            , AUTORECOVERY_PROPHANDLE_CRASHED            , ::getBooleanCppuType() , css::beans::PropertyAttribute::TRANSIENT | css::beans::PropertyAttribute::READONLY ),
        css::beans::Property( AUTORECOVERY_PROPNAME_EXISTS_RECOVERYDATA, AUTORECOVERY_PROPHANDLE_EXISTS_RECOVERYDATA, ::getBooleanCppuType() , css::beans::PropertyAttribute::TRANSIENT | css::beans::PropertyAttribute::READONLY ),
        css::beans::Property( AUTORECOVERY_PROPNAME_EXISTS_SESSIONDATA , AUTORECOVERY_PROPHANDLE_EXISTS_SESSIONDATA , ::getBooleanCppuType() , css::beans::PropertyAttribute::TRANSIENT | css::beans::PropertyAttribute::READONLY ),
    };
    const css::uno::Sequence< css::beans::Property > lPropertyDescriptor(pPropertys, AUTORECOVERY_PROPCOUNT);
    return lPropertyDescriptor;
}

//-----------------------------------------------
::cppu::IPropertyArrayHelper& SAL_CALL AutoRecovery::getInfoHelper()
{
    static ::cppu::OPropertyArrayHelper* pInfoHelper = 0;
    if(!pInfoHelper)
    {
        ::osl::MutexGuard aGuard( LockHelper::getGlobalLock().getShareableOslMutex() );
        if(!pInfoHelper)
        {
            static ::cppu::OPropertyArrayHelper aInfoHelper(impl_getStaticPropertyDescriptor(), sal_True);
            pInfoHelper = &aInfoHelper;
        }
    }

    return (*pInfoHelper);
}

//-----------------------------------------------
css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL AutoRecovery::getPropertySetInfo()
    throw(css::uno::RuntimeException)
{
    static css::uno::Reference< css::beans::XPropertySetInfo >* pInfo = 0;
    if(!pInfo)
    {
        ::osl::MutexGuard aGuard( LockHelper::getGlobalLock().getShareableOslMutex() );
        if(!pInfo)
        {
            static css::uno::Reference< css::beans::XPropertySetInfo > xInfo(createPropertySetInfo(getInfoHelper()));
            pInfo = &xInfo;
        }
    }

    return (*pInfo);
}

//-----------------------------------------------
void AutoRecovery::implts_verifyCacheAgainstDesktopDocumentList()
{
    LOG_RECOVERY("AutoRecovery::implts_verifyCacheAgainstDesktopDocumentList() ...")

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    try
    {
        css::uno::Reference< css::frame::XFramesSupplier > xDesktop(
            xSMGR->createInstance(SERVICENAME_DESKTOP),
            css::uno::UNO_QUERY_THROW);

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
            // model again for it's new states.
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

    LOG_RECOVERY("... AutoRecovery::implts_verifyCacheAgainstDesktopDocumentList()")
}

//-----------------------------------------------
sal_Bool AutoRecovery::impl_enoughDiscSpace(sal_Int32 nRequiredSpace)
{
#ifdef SIMULATE_FULL_DISC
    return sal_False;
#else  // SIMULATE_FULL_DISC
    // In case an error occures and we are not able to retrieve the needed information
    // it's better to "disable" the feature ShowErrorOnFullDisc !
    // Otherwhise we start a confusing process of error handling ...

    sal_uInt64 nFreeSpace = SAL_MAX_UINT64;

    ::rtl::OUString     sBackupPath(SvtPathOptions().GetBackupPath());
    ::osl::VolumeInfo   aInfo      (osl_VolumeInfo_Mask_FreeSpace);
    ::osl::FileBase::RC aRC         = ::osl::Directory::getVolumeInfo(sBackupPath, aInfo);

    if (
        (aInfo.isValid(osl_VolumeInfo_Mask_FreeSpace)) &&
        (aRC == ::osl::FileBase::E_None         )
       )
    {
        nFreeSpace = aInfo.getFreeSpace();
    }

    sal_uInt64 nFreeMB = (nFreeSpace/1048576);
    return (nFreeMB >= (sal_uInt64)nRequiredSpace);
#endif // SIMULATE_FULL_DISC
}

//-----------------------------------------------
void AutoRecovery::impl_showFullDiscError()
{
    rtl::OUString sBtn(FWK_RESSTR(STR_FULL_DISC_RETRY_BUTTON));
    rtl::OUString sMsg(FWK_RESSTR(STR_FULL_DISC_MSG));

    rtl::OUString sBackupURL(SvtPathOptions().GetBackupPath());
    INetURLObject aConverter(sBackupURL);
    sal_Unicode aDelimiter;
    rtl::OUString sBackupPath = aConverter.getFSysPath(INetURLObject::FSYS_DETECT, &aDelimiter);
    if (sBackupPath.getLength() < 1)
        sBackupPath = sBackupURL;

    ErrorBox dlgError(
        0, WB_OK,
        sMsg.replaceAll("%PATH", sBackupPath));
    dlgError.SetButtonText(dlgError.GetButtonId(0), sBtn);
    dlgError.Execute();
}

//-----------------------------------------------
void AutoRecovery::impl_establishProgress(const AutoRecovery::TDocumentInfo&               rInfo    ,
                                                ::comphelper::MediaDescriptor&             rArgs    ,
                                          const css::uno::Reference< css::frame::XFrame >& xNewFrame)
{
    // external well known frame must be preferred (because it was created by ourself
    // for loading documents into this frame)!
    // But if no frame exists ... we can try to locate it using any frame bound to the provided
    // document. Of course we must live without any frame in case the document does not exists at this
    // point. But this state shouldnt occure. In such case xNewFrame should be valid ... hopefully .-)
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
                                                                                ::comphelper::MediaDescriptor::PROP_STATUSINDICATOR(),
                                                                                css::uno::Reference< css::task::XStatusIndicator >() );

    // Normaly a progress is set from outside (e.g. by the CrashSave/Recovery dialog, which uses our dispatch API).
    // But for a normal auto save we dont have such "external progress"... because this function is triggered by our own timer then.
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
    // must be preferred. But we know that some application filters query it's own progress instance
    // at the frame method Frame::createStatusIndicator().
    // So we use a two step mechanism:
    // 1) we set the progress inside the MediaDescriptor, which will be provided to the filter
    // 2) and we set a special Frame property, which overwrites the normal behaviour of Frame::createStatusIndicator .-)
    // But we supress 2) in case we uses an internal progress. Because then it doesnt matter
    // if our applications make it wrong. In such case the internal progress resists at the same frame
    // and there is no need to forward progress activities to e.g. an outside dialog .-)
    if (
        (xExternalProgress.is()) &&
        (xFrame.is()           )
       )
    {
        css::uno::Reference< css::beans::XPropertySet > xFrameProps(xFrame, css::uno::UNO_QUERY);
        if (xFrameProps.is())
            xFrameProps->setPropertyValue(FRAME_PROPNAME_INDICATORINTERCEPTION, css::uno::makeAny(xExternalProgress));
    }

    // But inside the MediaDescriptor we must set our own create progress ...
    // in case there is not already anothe rprogress set.
    rArgs.createItemIfMissing(::comphelper::MediaDescriptor::PROP_STATUSINDICATOR(), xInternalProgress);
}

//-----------------------------------------------
void AutoRecovery::impl_forgetProgress(const AutoRecovery::TDocumentInfo&               rInfo    ,
                                             ::comphelper::MediaDescriptor&             rArgs    ,
                                       const css::uno::Reference< css::frame::XFrame >& xNewFrame)
{
    // external well known frame must be preferred (because it was created by ourself
    // for loading documents into this frame)!
    // But if no frame exists ... we can try to locate it using any frame bound to the provided
    // document. Of course we must live without any frame in case the document does not exists at this
    // point. But this state shouldnt occure. In such case xNewFrame should be valid ... hopefully .-)
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
        xFrameProps->setPropertyValue(FRAME_PROPNAME_INDICATORINTERCEPTION, css::uno::makeAny(css::uno::Reference< css::task::XStatusIndicator >()));

    // forget progress inside list of arguments.
    ::comphelper::MediaDescriptor::iterator pArg = rArgs.find(::comphelper::MediaDescriptor::PROP_STATUSINDICATOR());
    if (pArg != rArgs.end())
    {
        rArgs.erase(pArg);
        pArg = rArgs.end();
    }
}

//-----------------------------------------------
void AutoRecovery::impl_flushALLConfigChanges()
{
    try
    {
        // SAFE ->
        ReadGuard aReadLock(m_aLock);
        css::uno::Reference< css::uno::XInterface > xRecoveryCfg(m_xRecoveryCFG, css::uno::UNO_QUERY);
        aReadLock.unlock();
        // <- SAFE

        if (xRecoveryCfg.is())
            ::comphelper::ConfigurationHelper::flush(xRecoveryCfg);

        // SOLAR SAFE ->
        SolarMutexGuard aGuard;
        ::utl::ConfigManager::storeConfigItems();
    }
    catch(const css::uno::Exception&)
    {
    }
}

//-----------------------------------------------
void AutoRecovery::st_impl_removeFile(const ::rtl::OUString& sURL)
{
    if ( sURL.isEmpty())
        return;

    try
    {
        ::ucbhelper::Content aContent = ::ucbhelper::Content(sURL, css::uno::Reference< css::ucb::XCommandEnvironment >(), comphelper::getComponentContext(m_xSMGR));
        aContent.executeCommand(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("delete")), css::uno::makeAny(sal_True));
    }
    catch(const css::uno::Exception&)
    {
    }
}

//-----------------------------------------------
void AutoRecovery::st_impl_removeLockFile()
{
    try
    {
        ::rtl::OUString sUserURL;
        ::utl::Bootstrap::locateUserInstallation( sUserURL );

        ::rtl::OUStringBuffer sLockURLBuf;
        sLockURLBuf.append     (sUserURL);
        sLockURLBuf.appendAscii("/.lock");
        ::rtl::OUString sLockURL = sLockURLBuf.makeStringAndClear();

        AutoRecovery::st_impl_removeFile(sLockURL);
    }
    catch(const css::uno::Exception&)
    {
    }
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

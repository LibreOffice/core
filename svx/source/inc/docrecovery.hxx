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

#ifndef INCLUDED_SVX_SOURCE_INC_DOCRECOVERY_HXX
#define INCLUDED_SVX_SOURCE_INC_DOCRECOVERY_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/tabdlg.hxx>
#include <vcl/tabpage.hxx>
#include <svtools/simptabl.hxx>
#include <svtools/svlbitm.hxx>
#include <svtools/svmedit2.hxx>
#include <svtools/treelistbox.hxx>

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/task/StatusIndicatorFactory.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/lang/XComponent.hpp>


#define RECOVERY_CMDPART_PROTOCOL                   "vnd.sun.star.autorecovery:"

#define RECOVERY_CMDPART_DO_EMERGENCY_SAVE          "/doEmergencySave"
#define RECOVERY_CMDPART_DO_RECOVERY                "/doAutoRecovery"

#define RECOVERY_CMD_DO_PREPARE_EMERGENCY_SAVE      "vnd.sun.star.autorecovery:/doPrepareEmergencySave"
#define RECOVERY_CMD_DO_EMERGENCY_SAVE              "vnd.sun.star.autorecovery:/doEmergencySave"
#define RECOVERY_CMD_DO_RECOVERY                    "vnd.sun.star.autorecovery:/doAutoRecovery"
#define RECOVERY_CMD_DO_ENTRY_BACKUP                "vnd.sun.star.autorecovery:/doEntryBackup"
#define RECOVERY_CMD_DO_ENTRY_CLEANUP               "vnd.sun.star.autorecovery:/doEntryCleanUp"

#define PROP_STATUSINDICATOR                        "StatusIndicator"
#define PROP_DISPATCHASYNCHRON                      "DispatchAsynchron"
#define PROP_SAVEPATH                               "SavePath"
#define PROP_ENTRYID                                "EntryID"

#define STATEPROP_ID                                "ID"
#define STATEPROP_STATE                             "DocumentState"
#define STATEPROP_ORGURL                            "OriginalURL"
#define STATEPROP_TEMPURL                           "TempURL"
#define STATEPROP_FACTORYURL                        "FactoryURL"
#define STATEPROP_TEMPLATEURL                       "TemplateURL"
#define STATEPROP_TITLE                             "Title"
#define STATEPROP_MODULE                            "Module"

#define RECOVERY_OPERATIONSTATE_START               "start"
#define RECOVERY_OPERATIONSTATE_STOP                "stop"
#define RECOVERY_OPERATIONSTATE_UPDATE              "update"

#define DLG_RET_UNKNOWN                                  -1
#define DLG_RET_OK                                        1
#define DLG_RET_CANCEL                                    0
#define DLG_RET_OK_AUTOLUNCH                            101


namespace svx{
    namespace DocRecovery{


enum EDocStates
{
    /* TEMP STATES */

    /// default state, if a document was new created or loaded
    E_UNKNOWN = 0,
    /** an action was started (saving/loading) ... Can be interesting later if the process may be was interrupted by an exception. */
    E_TRY_LOAD_BACKUP = 16,
    E_TRY_LOAD_ORIGINAL = 32,

    /* FINAL STATES */

    /// the Auto/Emergency saved document isn't useable any longer
    E_DAMAGED = 64,
    /// the Auto/Emergency saved document is not really up-to-date (some changes can be missing)
    E_INCOMPLETE = 128,
    /// the Auto/Emergency saved document was processed successfully
    E_SUCCEDED = 512
};


enum ERecoveryState
{
    E_SUCCESSFULLY_RECOVERED,
    E_ORIGINAL_DOCUMENT_RECOVERED,
    E_RECOVERY_FAILED,
    E_RECOVERY_IS_IN_PROGRESS,
    E_NOT_RECOVERED_YET
};


struct TURLInfo
{
    public:

    /// unique ID, which is specified by the underlying autorecovery core!
    sal_Int32 ID;

    /// the full qualified document URL
    OUString OrgURL;

    /// the full qualified URL of the temp. file (if it's exists)
    OUString TempURL;

    /// a may be existing factory URL (e.g. for untitled documents)
    OUString FactoryURL;

    /// may be the document base on a template file !?
    OUString TemplateURL;

    /// the pure file name, without path, disc etcpp.
    OUString DisplayName;

    /// the application module, where this document was loaded
    OUString Module;

    /// state info as e.g. VALID, CORRUPTED, NON EXISTING ...
    sal_Int32 DocState;

    /// ui representation for DocState!
    ERecoveryState RecoveryState;

    /// standard icon
    Image StandardImage;

    public:

    TURLInfo()
        : ID           (-1                 )
        , DocState     (E_UNKNOWN          )
        , RecoveryState(E_NOT_RECOVERED_YET)
    {}
};


typedef ::std::vector< TURLInfo > TURLList;


class IRecoveryUpdateListener
{
    public:

        // inform listener about changed items, which should be refreshed
        virtual void updateItems() = 0;

        // inform listener about starting of the asynchronous recovery operation
        virtual void start() = 0;

        // inform listener about ending of the asynchronous recovery operation
        virtual void end() = 0;

        // TODO
        virtual void stepNext(TURLInfo* pItem) = 0;

    protected:
        ~IRecoveryUpdateListener() {}
};


class RecoveryCore : public ::cppu::WeakImplHelper< css::frame::XStatusListener >
{

    // types, const
    public:


    // member
    private:

        /// TODO
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

        /// TODO
        css::uno::Reference< css::frame::XDispatch > m_xRealCore;

        /// TODO
        css::uno::Reference< css::task::XStatusIndicator > m_xProgress;

        /// TODO
        TURLList m_lURLs;

        /// TODO
        IRecoveryUpdateListener* m_pListener;

        /** @short  knows the reason, why we listen on our internal m_xRealCore
                    member.

            @descr  Because we listen for different operations
                    on the core dispatch implementation, we must know,
                    which URL we have to use for deregistration!
         */
        bool m_bListenForSaving;


    // native interface
    public:


        /** @short  TODO */
        RecoveryCore(const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                           bool                                            bUsedForSaving);


        /** @short  TODO */
        virtual ~RecoveryCore() override;


        /** @short  TODO */
        const css::uno::Reference< css::uno::XComponentContext >& getComponentContext();


        /** @short  TODO */
        TURLList& getURLListAccess();


        /** @short  TODO */
        static bool isBrokenTempEntry(const TURLInfo& rInfo);
        void saveBrokenTempEntries(const OUString& sSaveDir);
        void saveAllTempEntries(const OUString& sSaveDir);
        void forgetBrokenTempEntries();
        void forgetAllRecoveryEntries();
        void forgetBrokenRecoveryEntries();


        /** @short  TODO */
        void setProgressHandler(const css::uno::Reference< css::task::XStatusIndicator >& xProgress);


        /** @short  TODO */
        void setUpdateListener(IRecoveryUpdateListener* pListener);


        /** @short  TODO */
        void doEmergencySavePrepare();
        void doEmergencySave();
        void doRecovery();


        /** @short  TODO */
        static ERecoveryState mapDocState2RecoverState(sal_Int32 eDocState);


    // uno interface
    public:

        // css.frame.XStatusListener
        virtual void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& aEvent) override;

        // css.lang.XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent) override;


    // helper
    private:


        /** @short  starts listening on the internal EmergencySave/AutoRecovery core.
         */
        void impl_startListening();


        /** @short  stop listening on the internal EmergencySave/AutoRecovery core.
         */
        void impl_stopListening();


        /** @short  TODO */
        css::util::URL impl_getParsedURL(const OUString& sURL);
};


class PluginProgressWindow : public vcl::Window
{
    private:
        css::uno::Reference< css::lang::XComponent > m_xProgress;
    public:
        PluginProgressWindow(      vcl::Window*                                       pParent  ,
                             const css::uno::Reference< css::lang::XComponent >& xProgress);
        virtual ~PluginProgressWindow() override;
        virtual void dispose() override;
};

class PluginProgress : public ::cppu::WeakImplHelper< css::task::XStatusIndicator ,
                                                       css::lang::XComponent       >
{
    // member
    private:
        /** @short  TODO */
        css::uno::Reference< css::task::XStatusIndicatorFactory > m_xProgressFactory;

        css::uno::Reference< css::task::XStatusIndicator > m_xProgress;

        VclPtr<PluginProgressWindow> m_pPlugProgressWindow;


    // native interface
    public:
        /** @short  TODO */
        PluginProgress(      vcl::Window*                                             pParent,
                       const css::uno::Reference< css::uno::XComponentContext >& xContext  );


        /** @short  TODO */
        virtual ~PluginProgress() override;


    // uno interface
    public:


        // XStatusIndicator
        virtual void SAL_CALL start(const OUString& sText ,
                                          sal_Int32        nRange) override;

        virtual void SAL_CALL end() override;

        virtual void SAL_CALL setText(const OUString& sText) override;

        virtual void SAL_CALL setValue(sal_Int32 nValue) override;

        virtual void SAL_CALL reset() override;


        // XComponent
        virtual void SAL_CALL dispose() override;

        virtual void SAL_CALL addEventListener(const css::uno::Reference< css::lang::XEventListener >& xListener) override;

        virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener) override;
};

class SaveDialog : public Dialog
{
    // member
    private:
        VclPtr<ListBox>        m_pFileListLB;
        VclPtr<OKButton>       m_pOkBtn;
        RecoveryCore*   m_pCore;

    // interface
    public:
        /** @short  create all child controls of this dialog.

            @descr  The dialog isn't shown nor it starts any
                    action by itself!

            @param  pParent
                    can point to a parent window.
                    If its set to 0, the defmodal-dialog-parent
                    is used automatically.

            @param  pCore
                    provides access to the recovery core service
                    and the current list of open documents,
                    which should be shown inside this dialog.
         */
        SaveDialog(vcl::Window* pParent, RecoveryCore* pCore);
        virtual ~SaveDialog() override;
        virtual void dispose() override;

        DECL_LINK(OKButtonHdl, Button*, void);
};

class SaveProgressDialog : public ModalDialog
                         , public IRecoveryUpdateListener
{
    // member
    private:
        VclPtr<vcl::Window>       m_pProgrParent;

        // @short   TODO
        RecoveryCore* m_pCore;

        // @short   TODO
        css::uno::Reference< css::task::XStatusIndicator > m_xProgress;
    // interface
    public:
        /** @short  create all child controls of this dialog.

            @descr  The dialog isn't shown nor it starts any
                    action by itself!

            @param  pParent
                    can point to a parent window.
                    If its set to 0, the defmodal-dialog-parent
                    is used automatically.

            @param  pCore
                    used to start emergency save.
         */
        SaveProgressDialog(vcl::Window*       pParent,
                           RecoveryCore* pCore  );
        virtual ~SaveProgressDialog() override;
        virtual void dispose() override;

        /** @short  start the emergency save operation. */
        virtual short Execute() override;

        // IRecoveryUpdateListener
        virtual void updateItems() override;
        virtual void stepNext(TURLInfo* pItem) override;
        virtual void start() override;
        virtual void end() override;
};


class RecovDocListEntry : public SvLBoxString
{
public:

    /** @short TODO */
    RecovDocListEntry( const OUString&      sText );


    /** @short TODO */
    virtual void Paint(const Point& rPos, SvTreeListBox& rOutDev, vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView, const SvTreeListEntry& rEntry) override;
};


class RecovDocList : public SvSimpleTable
{

    // member
    public:

        Image  m_aGreenCheckImg;
        Image  m_aYellowCheckImg;
        Image  m_aRedCrossImg;

        OUString m_aSuccessRecovStr;
        OUString m_aOrigDocRecovStr;
        OUString m_aRecovFailedStr;
        OUString m_aRecovInProgrStr;
        OUString m_aNotRecovYetStr;


    // interface
    public:


        /** @short TODO */
        RecovDocList(SvSimpleTableContainer& rParent, ResMgr& rResMgr);

        /** @short TODO */
        virtual void InitEntry(SvTreeListEntry* pEntry,
                               const OUString& rText,
                               const Image& rImage1,
                               const Image& rImage2,
                               SvLBoxButtonKind eButtonKind) override;
};


class RecoveryDialog : public Dialog
                     , public IRecoveryUpdateListener
{
    // member
    private:
        VclPtr<FixedText>      m_pDescrFT;
        VclPtr<vcl::Window>    m_pProgrParent;
        VclPtr<RecovDocList>   m_pFileListLB;
        VclPtr<PushButton>     m_pNextBtn;
        VclPtr<PushButton>     m_pCancelBtn;
        OUString        m_aTitleRecoveryInProgress;
        OUString        m_aRecoveryOnlyFinish;
        OUString        m_aRecoveryOnlyFinishDescr;

        RecoveryCore*   m_pCore;
        css::uno::Reference< css::task::XStatusIndicator > m_xProgress;
        enum EInternalRecoveryState
        {
            E_RECOVERY_PREPARED,            // dialog started... recovery prepared
            E_RECOVERY_IN_PROGRESS,         // recovery core still in progress
            E_RECOVERY_CORE_DONE,           // recovery core finished it's task
            E_RECOVERY_DONE,                // user clicked "next" button
            E_RECOVERY_CANCELED,            // user clicked "cancel" button
            E_RECOVERY_CANCELED_BEFORE,     // user clicked "cancel" button before recovery was started
            E_RECOVERY_CANCELED_AFTERWARDS, // user clicked "cancel" button after recovery was finished
            E_RECOVERY_HANDLED              // the recovery wizard page was shown already... and will be shown now again...
        };
        sal_Int32 m_eRecoveryState;
        bool  m_bWaitForCore;
        bool  m_bWasRecoveryStarted;

    // member
    public:
        /** @short TODO */
        RecoveryDialog(vcl::Window*       pParent,
                       RecoveryCore* pCore  );

        virtual ~RecoveryDialog() override;
        virtual void dispose() override;

        // IRecoveryUpdateListener
        virtual void updateItems() override;
        virtual void stepNext(TURLInfo* pItem) override;
        virtual void start() override;
        virtual void end() override;

        short execute();

    // helper
    private:
        /** @short TODO */
        DECL_LINK(NextButtonHdl, Button*, void);
        DECL_LINK(CancelButtonHdl, Button*, void);


        /** @short TODO */
        OUString impl_getStatusString( const TURLInfo& rInfo ) const;
};


class BrokenRecoveryDialog : public ModalDialog
{

    // member
    private:
        VclPtr<ListBox>         m_pFileListLB;
        VclPtr<Edit>            m_pSaveDirED;
        VclPtr<PushButton>      m_pSaveDirBtn;
        VclPtr<PushButton>      m_pOkBtn;
        VclPtr<CancelButton>    m_pCancelBtn;

        OUString m_sSavePath;
        RecoveryCore*   m_pCore;
        bool        m_bBeforeRecovery;
        bool        m_bExecutionNeeded;


    // interface
    public:


        /** @short TODO */
        BrokenRecoveryDialog(vcl::Window*       pParent        ,
                             RecoveryCore* pCore          ,
                             bool      bBeforeRecovery);
        virtual ~BrokenRecoveryDialog() override;
        virtual void dispose() override;


        /** @short TODO */
        bool isExecutionNeeded();


        /** @short TODO */
        const OUString& getSaveDirURL();


    // helper
    private:


        /** @short TODO */
        void impl_refresh();


        /** @short TODO */
        DECL_LINK(SaveButtonHdl, Button*, void);


        /** @short TODO */
        DECL_LINK(OkButtonHdl, Button*, void);


        /** @short TODO */
        DECL_LINK(CancelButtonHdl, Button*, void);


        /** @short TODO */
        void impl_askForSavePath();
};
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

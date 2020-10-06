/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <vcl/svapp.hxx>
#include <salhelper/thread.hxx>
#include <rtl/ref.hxx>
#include <vcl/weld.hxx>
#include <strings.hrc>
#include <comphelper/string.hxx>
#include "dialmgr.hxx"

// Detect changes on the UI
#include <vcl/timer.hxx>

// Search and filter
#include <i18nutil/searchopt.hxx>

// Extension Manager Connection
#include <com/sun/star/deployment/XExtensionManager.hpp>
#include <com/sun/star/deployment/ExtensionManager.hpp>
#include <com/sun/star/deployment/XPackageManager.hpp>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <cppuhelper/implbase.hxx>

class AdditionsDialog;
class SearchAndParseThread;
class AdditionsItem;

struct AdditionInfo
{
    OUString sExtensionID;
    OUString sName;
    OUString sAuthorName;
    OUString sExtensionURL;
    OUString sScreenshotURL;
    OUString sIntroduction;
    OUString sDescription;
    OUString sCompatibleVersion;
    OUString sReleaseVersion;
    OUString sLicense;
    OUString sCommentNumber;
    OUString sCommentURL;
    OUString sRating;
    OUString sDownloadNumber;
    OUString sDownloadURL;
};

class AdditionsDialog : public weld::GenericDialogController
{
private:
    Timer m_aSearchDataTimer;

    DECL_LINK(GearHdl, const OString&, void);
    DECL_LINK(SearchUpdateHdl, weld::Entry&, void);
    DECL_LINK(ImplUpdateDataHdl, Timer*, void);
    DECL_LINK(FocusOut_Impl, weld::Widget&, void);
    DECL_LINK(CloseButtonHdl, weld::Button&, void);

public:
    css::uno::Reference<css::deployment::XExtensionManager> m_xExtensionManager;
    std::unique_ptr<weld::Entry> m_xEntrySearch;
    std::unique_ptr<weld::Button> m_xButtonClose;
    std::unique_ptr<weld::MenuButton> m_xMenuButtonSettings;
    std::vector<std::shared_ptr<AdditionsItem>> m_aAdditionsItems; // UI components
    std::vector<AdditionInfo> m_aAllExtensionsVector; // Stores the all extensions' info

    std::unique_ptr<weld::ScrolledWindow> m_xContentWindow;
    std::unique_ptr<weld::Container> m_xContentGrid;

    std::unique_ptr<weld::Label> m_xLabelProgress;
    std::unique_ptr<weld::MenuButton> m_xGearBtn;

    ::rtl::Reference<SearchAndParseThread> m_pSearchThread;

    OString m_sURL;
    OString m_sTag;
    size_t
        m_nMaxItemCount; // Max number of item which will appear on the list before the press to the show more button.
    size_t m_nCurrentListItemCount; // Current number of item on the list
    i18nutil::SearchOptions2 m_searchOptions;

    AdditionsDialog(weld::Window* pParent, const OUString& sAdditionsTag);
    ~AdditionsDialog() override;
    css::uno::Sequence<css::uno::Sequence<css::uno::Reference<css::deployment::XPackage>>>
    getInstalledExtensions();
    void SetProgress(const OUString& rProgress);
    void ClearList();
    void RefreshUI();

    static bool sortByComment(const AdditionInfo& a, const AdditionInfo& b);
    static bool sortByRating(const AdditionInfo& a, const AdditionInfo& b);
    static bool sortByDownload(const AdditionInfo& a, const AdditionInfo& b);
};

class AdditionsItem
{
public:
    AdditionsItem(weld::Widget* pParent, AdditionsDialog* pParentDialog,
                  const AdditionInfo& additionInfo);
    bool getExtensionFile(OUString& sExtensionFile);

    DECL_LINK(ShowMoreHdl, weld::Button&, void);
    DECL_LINK(InstallHdl, weld::Button&, void);

    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Widget> m_xContainer;
    std::unique_ptr<weld::Image> m_xImageScreenshot;
    std::unique_ptr<weld::Button> m_xButtonInstall;
    std::unique_ptr<weld::LinkButton> m_xLinkButtonName;
    std::unique_ptr<weld::Label> m_xLabelAuthor;
    std::unique_ptr<weld::Label> m_xLabelDesc;
    std::unique_ptr<weld::Label> m_xLabelDescription;
    std::unique_ptr<weld::Label> m_xLabelLicense;
    std::unique_ptr<weld::Label> m_xLabelVersion;
    std::unique_ptr<weld::Label> m_xLabelComments;
    std::unique_ptr<weld::LinkButton> m_xLinkButtonComments;
    std::unique_ptr<weld::Image> m_xImageVoting1;
    std::unique_ptr<weld::Image> m_xImageVoting2;
    std::unique_ptr<weld::Image> m_xImageVoting3;
    std::unique_ptr<weld::Image> m_xImageVoting4;
    std::unique_ptr<weld::Image> m_xImageVoting5;
    std::unique_ptr<weld::Label> m_xLabelNoVoting;
    std::unique_ptr<weld::Image> m_xImageDownloadNumber;
    std::unique_ptr<weld::Label> m_xLabelDownloadNumber;
    std::unique_ptr<weld::Button> m_xButtonShowMore;
    AdditionsDialog* m_pParentDialog;
    OUString m_sDownloadURL;
    OUString m_sExtensionID;
};

class SearchAndParseThread : public salhelper::Thread
{
private:
    AdditionsDialog* m_pAdditionsDialog;
    std::atomic<bool> m_bExecute;
    bool m_bIsFirstLoading;

    void Search();
    void Append(AdditionInfo& additionInfo);
    void CheckInstalledExtensions();

    virtual ~SearchAndParseThread() override;
    virtual void execute() override;

public:
    SearchAndParseThread(AdditionsDialog* pDialog, const bool& bIsFirstLoading);

    void StopExecution() { m_bExecute = false; }
};

class TmpRepositoryCommandEnv
    : public ::cppu::WeakImplHelper<css::ucb::XCommandEnvironment, css::task::XInteractionHandler,
                                    css::ucb::XProgressHandler>
{
public:
    virtual ~TmpRepositoryCommandEnv() override;
    TmpRepositoryCommandEnv();

    // XCommandEnvironment
    virtual css::uno::Reference<css::task::XInteractionHandler>
        SAL_CALL getInteractionHandler() override;
    virtual css::uno::Reference<css::ucb::XProgressHandler> SAL_CALL getProgressHandler() override;

    // XInteractionHandler
    virtual void SAL_CALL
    handle(css::uno::Reference<css::task::XInteractionRequest> const& xRequest) override;

    // XProgressHandler
    virtual void SAL_CALL push(css::uno::Any const& Status) override;
    virtual void SAL_CALL update(css::uno::Any const& Status) override;
    virtual void SAL_CALL pop() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

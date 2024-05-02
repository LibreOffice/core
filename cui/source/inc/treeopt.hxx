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

#include <sal/config.h>

#include <memory>

#include <sfx2/basedlgs.hxx>
#include <svtools/restartdialog.hxx>
#include <utility>
#include <i18nutil/searchopt.hxx>
#include <vcl/timer.hxx>

class SfxModule;
class SfxShell;

// struct OrderedEntry ---------------------------------------------------

struct OrderedEntry
{
    sal_Int32       m_nIndex;
    OUString   m_sId;

    OrderedEntry( sal_Int32 nIndex, OUString aId ) :
        m_nIndex( nIndex ), m_sId(std::move( aId )) {}
};


// struct Module ---------------------------------------------------------

struct Module
{
    bool                          m_bActive;
    std::vector< std::unique_ptr<OrderedEntry> >  m_aNodeList;

    Module() : m_bActive( false ) {}
};

// struct OptionsLeaf ----------------------------------------------------

struct OptionsLeaf
{
    OUString   m_sLabel;
    OUString   m_sPageURL;
    OUString   m_sEventHdl;
    OUString   m_sGroupId;
    sal_Int32       m_nGroupIndex;

    OptionsLeaf(    OUString aLabel,
                    OUString aPageURL,
                    OUString aEventHdl,
                    OUString aGroupId,
                    sal_Int32 nGroupIndex ) :
        m_sLabel(std::move( aLabel )),
        m_sPageURL(std::move( aPageURL )),
        m_sEventHdl(std::move( aEventHdl )),
        m_sGroupId(std::move( aGroupId )),
        m_nGroupIndex( nGroupIndex ) {}
};

// struct OptionsNode ----------------------------------------------------

struct OptionsNode
{
    OUString                m_sId;
    OUString                m_sLabel;
    bool                    m_bAllModules;
    std::vector< std::unique_ptr<OptionsLeaf> > m_aLeaves;
    std::vector< std::vector< std::unique_ptr<OptionsLeaf> > >
                            m_aGroupedLeaves;

    OptionsNode(    OUString aId,
                    OUString aLabel,
                    bool bAllModules ) :
        m_sId(std::move( aId )),
        m_sLabel(std::move( aLabel )),
        m_bAllModules( bAllModules ) {}
};

typedef std::vector< std::unique_ptr<OptionsNode> > VectorOfNodes;

struct LastPageSaver
{
    sal_uInt16   m_nLastPageId;
    OUString     m_sLastPageURL_Tools;
    OUString     m_sLastPageURL_ExtMgr;

    LastPageSaver() : m_nLastPageId( USHRT_MAX ) {}
};

// class OfaTreeOptionsDialog --------------------------------------------

namespace com::sun::star::frame { class XFrame; }
namespace com::sun::star::awt { class XContainerWindowProvider; }

struct OptionsPageInfo;
struct Module;
class ExtensionsTabPage;
class SvxColorTabPage;
struct OptionsGroupInfo;
struct OptionsPageIdInfo;

class OfaTreeOptionsDialog final: public SfxOkDialogController
{
private:
    std::unique_ptr<weld::Button> xOkPB;
    std::unique_ptr<weld::Button> xApplyPB;
    std::unique_ptr<weld::Button> xBackPB;

    std::unique_ptr<weld::TreeView> xTreeLB;
    std::unique_ptr<weld::Container> xTabBox;
    std::unique_ptr<weld::Entry> m_xSearchEdit;

    weld::Window*    m_pParent;

    std::unique_ptr<weld::TreeIter> xCurrentPageEntry;

    // For search
    Timer m_aUpdateDataTimer;
    i18nutil::SearchOptions2 m_options;

    bool bIsFirtsInitialize;
    std::vector<OptionsPageIdInfo*> m_aTreePageIds;
    typedef std::vector<std::pair<sal_uInt16, std::vector<sal_uInt16>>> VectorOfMatchedIds;

    void generalOptions(const std::vector<sal_uInt16>& vPageId = {}); // SID_GENERAL_OPTIONS
    void loadAndSaveOptions(const std::vector<sal_uInt16>& vPageId = {}); // SID_FILTER_DLG
    void languageOptions(const std::vector<sal_uInt16>& vPageId = {}); // SID_LANGUAGE_OPTIONS
    void writerOptions(const std::vector<sal_uInt16>& vPageId = {}); // SID_SW_EDITOPTIONS
    void writerWebOptions(const std::vector<sal_uInt16>& vPageId = {}); // SID_SW_ONLINEOPTIONS
    void calcOptions(const std::vector<sal_uInt16>& vPageId = {}); // SID_SC_EDITOPTIONS
    void impressOptions(const std::vector<sal_uInt16>& vPageId = {}); // SID_SD_EDITOPTIONS
    void drawOptions(const std::vector<sal_uInt16>& vPageId = {}); // SID_SD_GRAPHIC_OPTIONS
    void mathOptions(const std::vector<sal_uInt16>& vPageId = {}); // SID_SM_EDITOPTIONS
    void databaseOptions(const std::vector<sal_uInt16>& vPageId = {}); // SID_SB_STARBASEOPTIONS
    void chartOptions(const std::vector<sal_uInt16>& vPageId = {}); // SID_SCH_EDITOPTIONS
    void internetOptions(const std::vector<sal_uInt16>& vPageId = {}); // SID_INET_DLG

    void clearOptionsDialog();
    void selectFirstEntry();
    void storeOptionsTree();
    void showDialog(VectorOfMatchedIds& pSearchIds);

    OUString               sTitle;

    bool                   bForgetSelection;
    bool                   bIsFromExtensionManager;

    // check "for the current document only" and set focus to "Western" languages box
    bool                   bIsForSetDocumentLanguage;

    bool                   bNeedsRestart;
    svtools::RestartReason eRestartReason;

    css::uno::Reference < css::awt::XContainerWindowProvider >
                    m_xContainerWinProvider;
    css::uno::Reference<css::frame::XFrame> m_xFrame;

    static LastPageSaver*   pLastPageSaver;

    std::optional<SfxItemSet> CreateItemSet( sal_uInt16 nId );
    static void     ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet );
    void            Initialize( const css::uno::Reference< css::frame::XFrame >& _xFrame );

    void            LoadExtensionOptions( std::u16string_view rExtensionId );
    static OUString GetModuleIdentifier( const css::uno::Reference<
                                            css::frame::XFrame >& xFrame );
    static std::unique_ptr<Module>  LoadModule( std::u16string_view rModuleIdentifier );
    static VectorOfNodes LoadNodes( Module* pModule, std::u16string_view rExtensionId );
    void            InsertNodes( const VectorOfNodes& rNodeList );

    void            ApplyOptions();

    DECL_LINK(ShowPageHdl_Impl, weld::TreeView&, void);
    DECL_LINK(BackHdl_Impl, weld::Button&, void);
    DECL_LINK(ApplyHdl_Impl, weld::Button&, void);
    DECL_LINK(HelpHdl_Impl, weld::Widget&, bool);
    DECL_LINK(SearchUpdateHdl, weld::Entry&, void);
    DECL_LINK(ImplUpdateDataHdl, Timer*, void);
    DECL_LINK(FocusOut_Impl, weld::Widget&, void);
    void ResetCurrentPageFromConfig();
    void SelectHdl_Impl();
    void initializeCurrentDialog(OptionsPageInfo*& pPageInfo,
                                 std::unique_ptr<weld::TreeIter>& xEntry);

    void InitItemSets(OptionsGroupInfo& rGroupInfo);

    virtual short run() override;

    virtual weld::Button& GetOKButton() const override { return *xOkPB; }
    virtual const SfxItemSet* GetExampleSet() const override { return nullptr; }

    int applySearchFilter(const OUString& rSearchTerm);

    // Common initialization
    OfaTreeOptionsDialog(weld::Window* pParent, bool fromExtensionManager);

public:
    OfaTreeOptionsDialog(weld::Window* pParent,
        const css::uno::Reference< css::frame::XFrame >& _xFrame,
        bool bActivateLastSelection);
    OfaTreeOptionsDialog(weld::Window* pParent, std::u16string_view rExtensionId);
    virtual ~OfaTreeOptionsDialog() override;

    OptionsPageInfo*    AddTabPage( sal_uInt16 nId, const OUString& rPageName, sal_uInt16 nGroup );
    sal_uInt16              AddGroup(   const OUString& rGroupName,  SfxShell* pCreateShell,
                                    SfxModule* pCreateModule, sal_uInt16 nDialogId );

    void                ActivateLastSelection();
    void                ActivatePage( sal_uInt16 nResId );
    void                ActivatePage( const OUString& rPageURL );
    void                ApplyItemSets();

    // initialize all dialogs in "Tools > Options"
    void initializeAllDialogs();

    // helper functions to call the Languages and Locales TabPage from the SpellDialog
    static void         ApplyLanguageOptions(const SfxItemSet& rSet);
    static OUString     getCurrentFactory_Impl( const css::uno::Reference< css::frame::XFrame >& _xFrame );

    void                SetNeedsRestart( svtools::RestartReason eReason );
};

// class ExtensionsTabPage -----------------------------------------------

namespace com::sun::star::awt { class XWindow; }
namespace com::sun::star::awt { class XContainerWindowEventHandler; }

class ExtensionsTabPage
{
private:
    weld::Container* m_pContainer;
    OUString       m_sPageURL;
    css::uno::Reference<css::awt::XWindow> m_xPageParent;
    css::uno::Reference<css::awt::XWindow> m_xPage;
    OUString       m_sEventHdl;
    css::uno::Reference< css::awt::XContainerWindowEventHandler >
                        m_xEventHdl;
    css::uno::Reference< css::awt::XContainerWindowProvider >
                        m_xWinProvider;

    void                CreateDialogWithHandler();
    bool                DispatchAction( const OUString& rAction );

public:
    ExtensionsTabPage(
        weld::Container* pParent,
        OUString  rPageURL, OUString aEvtHdl,
        const css::uno::Reference<
            css::awt::XContainerWindowProvider >& rProvider );

    ~ExtensionsTabPage();

    void Show();
    void Hide();

    void    ActivatePage();
    void    DeactivatePage();

    void            ResetPage();
    void            SavePage();
};

// class TreeOptHelper ---------------------------------------------------

class TreeOptHelper
{
public:
    static void storeStringsOfDialog(sal_uInt16 nPageId, const OUString& sPageStrings);
    static OUString getStringsFromDialog(sal_uInt16 nPageId);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

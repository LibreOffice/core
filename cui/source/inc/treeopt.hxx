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

class SfxModule;
class SfxShell;

// struct OrderedEntry ---------------------------------------------------

struct OrderedEntry
{
    sal_Int32       m_nIndex;
    OUString   m_sId;

    OrderedEntry( sal_Int32 nIndex, const OUString& rId ) :
        m_nIndex( nIndex ), m_sId( rId ) {}
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

    OptionsLeaf(    const OUString& rLabel,
                    const OUString& rPageURL,
                    const OUString& rEventHdl,
                    const OUString& rGroupId,
                    sal_Int32 nGroupIndex ) :
        m_sLabel( rLabel ),
        m_sPageURL( rPageURL ),
        m_sEventHdl( rEventHdl ),
        m_sGroupId( rGroupId ),
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

    OptionsNode(    const OUString& rId,
                    const OUString& rLabel,
                    bool bAllModules ) :
        m_sId( rId ),
        m_sLabel( rLabel ),
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

class OfaTreeOptionsDialog final: public SfxOkDialogController
{
private:
    std::unique_ptr<weld::Button> xOkPB;
    std::unique_ptr<weld::Button> xApplyPB;
    std::unique_ptr<weld::Button> xBackPB;

    std::unique_ptr<weld::TreeView> xTreeLB;
    std::unique_ptr<weld::Container> xTabBox;

    weld::Window*    m_pParent;

    std::unique_ptr<weld::TreeIter> xCurrentPageEntry;

    OUString               sTitle;

    bool                   bForgetSelection;
    bool                   bIsFromExtensionManager;

    // check "for the current document only" and set focus to "Western" languages box
    bool                   bIsForSetDocumentLanguage;

    bool                   bNeedsRestart;
    svtools::RestartReason eRestartReason;

    css::uno::Reference < css::awt::XContainerWindowProvider >
                    m_xContainerWinProvider;

    static LastPageSaver*   pLastPageSaver;

    std::unique_ptr<SfxItemSet> CreateItemSet( sal_uInt16 nId );
    static void     ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet );
    void            InitTreeAndHandler();
    void            Initialize( const css::uno::Reference< css::frame::XFrame >& _xFrame );
    void            InitWidgets();

    void            LoadExtensionOptions( const OUString& rExtensionId );
    static OUString GetModuleIdentifier( const css::uno::Reference<
                                            css::frame::XFrame >& xFrame );
    static std::unique_ptr<Module>  LoadModule( const OUString& rModuleIdentifier );
    static VectorOfNodes LoadNodes( Module* pModule, const OUString& rExtensionId );
    void            InsertNodes( const VectorOfNodes& rNodeList );

    void            ApplyOptions( bool deactivate );

    DECL_LINK(ShowPageHdl_Impl, weld::TreeView&, void);
    DECL_LINK(BackHdl_Impl, weld::Button&, void);
    DECL_LINK(ApplyHdl_Impl, weld::Button&, void);
    DECL_LINK(OKHdl_Impl, weld::Button&, void);
    DECL_LINK(HelpHdl_Impl, weld::Widget&, bool);
    void SelectHdl_Impl();

    virtual short run() override;

    virtual weld::Button& GetOKButton() const override { return *xOkPB; }
    virtual const SfxItemSet* GetExampleSet() const override { return nullptr; }

public:
    OfaTreeOptionsDialog(weld::Window* pParent,
        const css::uno::Reference< css::frame::XFrame >& _xFrame,
        bool bActivateLastSelection);
    OfaTreeOptionsDialog(weld::Window* pParent, const OUString& rExtensionId);
    virtual ~OfaTreeOptionsDialog() override;

    OptionsPageInfo*    AddTabPage( sal_uInt16 nId, const OUString& rPageName, sal_uInt16 nGroup );
    sal_uInt16              AddGroup(   const OUString& rGroupName,  SfxShell* pCreateShell,
                                    SfxModule* pCreateModule, sal_uInt16 nDialogId );

    void                ActivateLastSelection();
    void                ActivatePage( sal_uInt16 nResId );
    void                ActivatePage( const OUString& rPageURL );
    void                ApplyItemSets();

    // helper functions to call the language settings TabPage from the SpellDialog
    static void         ApplyLanguageOptions(const SfxItemSet& rSet);

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
        const OUString& rPageURL, const OUString& rEvtHdl,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

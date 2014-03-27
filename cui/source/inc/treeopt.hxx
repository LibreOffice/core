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

#ifndef INCLUDED_CUI_SOURCE_INC_TREEOPT_HXX
#define INCLUDED_CUI_SOURCE_INC_TREEOPT_HXX

#include <tools/resary.hxx>
#include <vcl/fixed.hxx>

class SfxModule;
class SfxShell;

// static ----------------------------------------------------------------

sal_Bool            EnableSSO();
CreateTabPage   GetSSOCreator( void );

// struct OrderedEntry ---------------------------------------------------

struct OrderedEntry
{
    sal_Int32       m_nIndex;
    OUString   m_sId;

    OrderedEntry( sal_Int32 nIndex, const OUString& rId ) :
        m_nIndex( nIndex ), m_sId( rId ) {}
};

typedef std::vector< OrderedEntry* > VectorOfOrderedEntries;

// struct Module ---------------------------------------------------------

struct Module
{
    OUString           m_sName;
    bool                    m_bActive;
    VectorOfOrderedEntries  m_aNodeList;

    Module( const OUString& rName ) : m_sName( rName ), m_bActive( false ) {}
};

// struct OptionsLeaf ----------------------------------------------------

struct OptionsLeaf
{
    OUString   m_sId;
    OUString   m_sLabel;
    OUString   m_sPageURL;
    OUString   m_sEventHdl;
    OUString   m_sGroupId;
    sal_Int32       m_nGroupIndex;

    OptionsLeaf(    const OUString& rId,
                    const OUString& rLabel,
                    const OUString& rPageURL,
                    const OUString& rEventHdl,
                    const OUString& rGroupId,
                    sal_Int32 nGroupIndex ) :
        m_sId( rId ),
        m_sLabel( rLabel ),
        m_sPageURL( rPageURL ),
        m_sEventHdl( rEventHdl ),
        m_sGroupId( rGroupId ),
        m_nGroupIndex( nGroupIndex ) {}
};

typedef ::std::vector< OptionsLeaf* > VectorOfLeaves;
typedef ::std::vector< VectorOfLeaves > VectorOfGroupedLeaves;

// struct OptionsNode ----------------------------------------------------

struct OptionsNode
{
    OUString           m_sId;
    OUString           m_sLabel;
    OUString           m_sPageURL;
    bool                    m_bAllModules;
    OUString           m_sGroupId;
    sal_Int32               m_nGroupIndex;
    VectorOfLeaves          m_aLeaves;
    VectorOfGroupedLeaves   m_aGroupedLeaves;

    OptionsNode(    const OUString& rId,
                    const OUString& rLabel,
                    const OUString& rPageURL,
                    bool bAllModules,
                    const OUString& rGroupId,
                    sal_Int32 nGroupIndex ) :
        m_sId( rId ),
        m_sLabel( rLabel ),
        m_sPageURL( rPageURL ),
        m_bAllModules( bAllModules ),
        m_sGroupId( rGroupId ),
        m_nGroupIndex( nGroupIndex ) {}

    ~OptionsNode()
    {
        for ( sal_uInt32 i = 0; i < m_aLeaves.size(); ++i )
            delete m_aLeaves[i];
        m_aLeaves.clear();
        m_aGroupedLeaves.clear();
    }
};

typedef ::std::vector< OptionsNode* > VectorOfNodes;

struct LastPageSaver
{
    sal_uInt16          m_nLastPageId;
    OUString   m_sLastPageURL_Tools;
    OUString   m_sLastPageURL_ExtMgr;

    LastPageSaver() : m_nLastPageId( USHRT_MAX ) {}
};

// class OfaTreeOptionsDialog --------------------------------------------

namespace com { namespace sun { namespace star { namespace frame { class XFrame; } } } }
namespace com { namespace sun { namespace star { namespace awt { class XContainerWindowProvider; } } } }

struct OptionsPageInfo;
struct Module;
class ExtensionsTabPage;
class SvxColorTabPage;
typedef std::vector< ExtensionsTabPage* > VectorOfPages;

class OfaTreeOptionsDialog : public SfxModalDialog
{
private:
    SvTreeListEntry*    pCurrentPageEntry;
    Timer           maTreeLayoutTimer;
    DECL_DLLPRIVATE_LINK( ImplHandleTreeLayoutTimerHdl, void* );
    bool hasTreePendingLayout() const;

    OKButton        aOkPB;
    CancelButton    aCancelPB;
    HelpButton      aHelpPB;
    PushButton      aBackPB;

    FixedLine       aSeparatorFL;

    SvTreeListBox   aTreeLB;

    OUString        sTitle;
    OUString        sNotLoadedError;

    // for the ColorTabPage
    SfxItemSet*     pColorPageItemSet;
    SvxColorTabPage *mpColorPage;

    sal_Bool        bForgetSelection;
    sal_Bool        bExternBrowserActive;
    bool            bIsFromExtensionManager;

    // check "for the current document only" and set focus to "Western" languages box
    bool            bIsForSetDocumentLanguage;

    com::sun::star::uno::Reference < com::sun::star::awt::XContainerWindowProvider >
                    m_xContainerWinProvider;

    static LastPageSaver*   pLastPageSaver;

    SfxItemSet*     CreateItemSet( sal_uInt16 nId );
    void            ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet );
    void            InitTreeAndHandler();
    void            Initialize( const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& _xFrame );
    void            ResizeTreeLB( void );   // resizes dialog so that treelistbox has no horizontal scroll bar

    void            LoadExtensionOptions( const OUString& rExtensionId );
    OUString   GetModuleIdentifier( const com::sun::star::uno::Reference<
                                            com::sun::star::frame::XFrame >& xFrame );
    Module*         LoadModule( const OUString& rModuleIdentifier );
    VectorOfNodes   LoadNodes( Module* pModule, const OUString& rExtensionId );
    void            InsertNodes( const VectorOfNodes& rNodeList );

    virtual void queue_resize() SAL_OVERRIDE;
    void SetPaneSize(Window *pPane);

protected:
    DECL_LINK(ExpandedHdl_Impl, SvTreeListBox* );
    DECL_LINK(ShowPageHdl_Impl, void *);
    DECL_LINK(BackHdl_Impl, void *);
    DECL_LINK(OKHdl_Impl, void *);
    DECL_LINK( HintHdl_Impl, Timer * );
    void SelectHdl_Impl();

    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual short   Execute() SAL_OVERRIDE;

public:
    OfaTreeOptionsDialog( Window* pParent,
        const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& _xFrame,
        bool bActivateLastSelection = true );
    OfaTreeOptionsDialog( Window* pParent, const OUString& rExtensionId );
    ~OfaTreeOptionsDialog();

    OptionsPageInfo*    AddTabPage( sal_uInt16 nId, const OUString& rPageName, sal_uInt16 nGroup );
    sal_uInt16              AddGroup(   const OUString& rGroupName,  SfxShell* pCreateShell,
                                    SfxModule* pCreateModule, sal_uInt16 nDialogId );

    void                ActivateLastSelection();
    void                ActivatePage( sal_uInt16 nResId );
    void                ActivatePage( const OUString& rPageURL );
    void                ApplyItemSets();

    // helper functions to call the language settings TabPage from the SpellDialog
    static void         ApplyLanguageOptions(const SfxItemSet& rSet);
};

// class OfaPageResource -------------------------------------------------

class OfaPageResource : public Resource
{
    ResStringArray      aGeneralDlgAry;
    ResStringArray      aInetDlgAry;
    ResStringArray      aLangDlgAry;
    ResStringArray      aTextDlgAry;
    ResStringArray      aHTMLDlgAry;
    ResStringArray      aCalcDlgAry;
    ResStringArray      aStarMathDlgAry;
    ResStringArray      aImpressDlgAry;
    ResStringArray      aDrawDlgAry;
    ResStringArray      aChartDlgAry;
    ResStringArray      aFilterDlgAry;
    ResStringArray      aDatasourcesDlgAry;

public:
    OfaPageResource();

    ResStringArray& GetGeneralArray()       {return aGeneralDlgAry;}
    ResStringArray& GetInetArray()          {return aInetDlgAry;}
    ResStringArray& GetLangArray()          {return aLangDlgAry;}
    ResStringArray& GetTextArray()          {return aTextDlgAry;}
    ResStringArray& GetHTMLArray()          {return aHTMLDlgAry;}
    ResStringArray& GetCalcArray()          {return aCalcDlgAry;}
    ResStringArray& GetStarMathArray()      {return aStarMathDlgAry;}
    ResStringArray& GetImpressArray()       {return aImpressDlgAry;}
    ResStringArray& GetDrawArray()          {return aDrawDlgAry;}
    ResStringArray& GetChartArray()         {return aChartDlgAry;}
    ResStringArray& GetFilterArray()        {return aFilterDlgAry;}
    ResStringArray& GetDatasourcesArray()   {return aDatasourcesDlgAry;}
};

// class ExtensionsTabPage -----------------------------------------------

namespace com { namespace sun { namespace star { namespace awt { class XWindow; } } } }
namespace com { namespace sun { namespace star { namespace awt { class XContainerWindowEventHandler; } } } }

class ExtensionsTabPage : public TabPage
{
private:
    OUString       m_sPageURL;
    com::sun::star::uno::Reference< com::sun::star::awt::XWindow >
                        m_xPage;
    OUString       m_sEventHdl;
    com::sun::star::uno::Reference< com::sun::star::awt::XContainerWindowEventHandler >
                        m_xEventHdl;
    com::sun::star::uno::Reference< com::sun::star::awt::XContainerWindowProvider >
                        m_xWinProvider;
    bool                m_bIsWindowHidden;

    void                CreateDialogWithHandler();
    sal_Bool            DispatchAction( const OUString& rAction );

public:
    ExtensionsTabPage(
        Window* pParent, WinBits nStyle,
        const OUString& rPageURL, const OUString& rEvtHdl,
        const com::sun::star::uno::Reference<
            com::sun::star::awt::XContainerWindowProvider >& rProvider );

    virtual ~ExtensionsTabPage();

    virtual void    ActivatePage() SAL_OVERRIDE;
    virtual void    DeactivatePage() SAL_OVERRIDE;

    void            ResetPage();
    void            SavePage();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

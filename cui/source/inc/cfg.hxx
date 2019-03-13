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
#ifndef INCLUDED_CUI_SOURCE_INC_CFG_HXX
#define INCLUDED_CUI_SOURCE_INC_CFG_HXX

#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/weld.hxx>
#include <svtools/imgdef.hxx>
#include <svtools/miscopt.hxx>
#include <vcl/treelistbox.hxx>
#include <svtools/svmedit2.hxx>
#include <svtools/svmedit.hxx>
#include <svtools/valueset.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/XUIConfigurationListener.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/ui/ImageType.hpp>
#include <com/sun/star/ui/ItemType.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>

#include <sfx2/tabdlg.hxx>
#include <memory>
#include <vector>

#include "cfgutil.hxx"
#include "CommandCategoryListBox.hxx"

static const char ITEM_DESCRIPTOR_COMMANDURL[]  = "CommandURL";
static const char ITEM_DESCRIPTOR_CONTAINER[]   = "ItemDescriptorContainer";
static const char ITEM_DESCRIPTOR_LABEL[]       = "Label";
static const char ITEM_DESCRIPTOR_TYPE[]        = "Type";
static const char ITEM_DESCRIPTOR_STYLE[]       = "Style";
static const char ITEM_DESCRIPTOR_ISVISIBLE[]   = "IsVisible";
static const char ITEM_DESCRIPTOR_RESOURCEURL[] = "ResourceURL";
static const char ITEM_DESCRIPTOR_UINAME[]      = "UIName";

static const char ITEM_MENUBAR_URL[] = "private:resource/menubar/menubar";
static const char ITEM_TOOLBAR_URL[] = "private:resource/toolbar/";

static const char CUSTOM_TOOLBAR_STR[] = "custom_toolbar_";

static const char aMenuSeparatorStr[] = " | ";

class SvxConfigEntry;
class SvxConfigPage;

typedef std::vector< SvxConfigEntry* > SvxEntries;

class SvxConfigDialog : public SfxTabDialogController
{
private:
    css::uno::Reference< css::frame::XFrame > m_xFrame;

public:
    SvxConfigDialog(weld::Window*, const SfxItemSet*);

    virtual void PageCreated(const OString& rId, SfxTabPage &rPage) override;
    void SetFrame(const css::uno::Reference< css::frame::XFrame >& xFrame);
};

class SaveInData
{
private:

    bool        bModified;

    bool        bDocConfig;
    bool        bReadOnly;

    css::uno::Reference
        < css::ui::XUIConfigurationManager > m_xCfgMgr;

    css::uno::Reference
        < css::ui::XUIConfigurationManager > m_xParentCfgMgr;

    css::uno::Reference
        < css::ui::XImageManager > m_xImgMgr;

    css::uno::Reference
        < css::ui::XImageManager > m_xParentImgMgr;

    static css::uno::Reference
        < css::ui::XImageManager >* xDefaultImgMgr;

protected:

    void ApplyMenu(
        css::uno::Reference< css::container::XIndexContainer > const & rMenuBar,
        css::uno::Reference< css::lang::XSingleComponentFactory >& rFactory,
        SvxConfigEntry *pMenuData );

    void LoadSubMenus(
        const css::uno::Reference< css::container::XIndexAccess >& xMenuSettings,
        const OUString& rBaseTitle, SvxConfigEntry const * pParentData, bool bContextMenu );

public:

    SaveInData(
        const css::uno::Reference < css::ui::XUIConfigurationManager >& xCfgMgr,
        const css::uno::Reference < css::ui::XUIConfigurationManager >& xParentCfgMgr,
        const OUString& aModuleId,
        bool docConfig );

    virtual ~SaveInData() {}

    bool PersistChanges(
        const css::uno::Reference< css::uno::XInterface >& xManager );

    void SetModified( bool bValue = true ) { bModified = bValue; }
    bool IsModified( ) { return bModified; }

    bool IsReadOnly( ) { return bReadOnly; }
    bool IsDocConfig( ) { return bDocConfig; }

    const css::uno::Reference
        < css::ui::XUIConfigurationManager >&
            GetConfigManager() { return m_xCfgMgr; };

    const css::uno::Reference
        < css::ui::XUIConfigurationManager >&
            GetParentConfigManager() { return m_xParentCfgMgr; };

    const css::uno::Reference
        < css::ui::XImageManager >&
            GetImageManager() { return m_xImgMgr; };

    const css::uno::Reference
        < css::ui::XImageManager >&
            GetParentImageManager() { return m_xParentImgMgr; };

    css::uno::Reference
        < css::container::XNameAccess > m_xCommandToLabelMap;

    css::uno::Sequence
        < css::beans::PropertyValue > m_aSeparatorSeq;

    css::uno::Reference<css::graphic::XGraphic> GetImage(const OUString& rCommandURL);

    virtual bool HasURL( const OUString& aURL ) = 0;
    virtual bool HasSettings() = 0;
    virtual SvxEntries* GetEntries() = 0;
    virtual void SetEntries( std::unique_ptr<SvxEntries> ) = 0;
    virtual void Reset() = 0;
    virtual bool Apply() = 0;
};

class MenuSaveInData : public SaveInData
{
private:

    OUString               m_aMenuResourceURL;
    OUString               m_aDescriptorContainer;

    css::uno::Reference
        < css::container::XIndexAccess > m_xMenuSettings;

    std::unique_ptr<SvxConfigEntry>      pRootEntry;


    static MenuSaveInData* pDefaultData;    ///< static holder of the default menu data

    static void SetDefaultData( MenuSaveInData* pData ) {pDefaultData = pData;}
    static MenuSaveInData* GetDefaultData() { return pDefaultData; }

    void        Apply(
        css::uno::Reference< css::container::XIndexContainer > const & rNewMenuBar,
        css::uno::Reference< css::lang::XSingleComponentFactory >& rFactory );

public:

    MenuSaveInData(
        const css::uno::Reference< css::ui::XUIConfigurationManager >&,
        const css::uno::Reference< css::ui::XUIConfigurationManager >&,
        const OUString& aModuleId,
        bool docConfig );

    virtual ~MenuSaveInData() override;

    /// methods inherited from SaveInData
    SvxEntries*         GetEntries() override;
    void                SetEntries( std::unique_ptr<SvxEntries> ) override;
    bool                HasURL( const OUString& ) override { return false; }
    bool                HasSettings() override { return m_xMenuSettings.is(); }
    void                Reset() override;
    bool                Apply() override;
};

class ContextMenuSaveInData : public SaveInData
{
private:
    std::unique_ptr< SvxConfigEntry > m_pRootEntry;
    css::uno::Reference< css::container::XNameAccess > m_xPersistentWindowState;
    OUString GetUIName( const OUString& rResourceURL );

public:
    ContextMenuSaveInData(
        const css::uno::Reference< css::ui::XUIConfigurationManager >& xCfgMgr,
        const css::uno::Reference< css::ui::XUIConfigurationManager >& xParentCfgMgr,
        const OUString& aModuleId, bool bIsDocConfig );
    virtual ~ContextMenuSaveInData() override;

    SvxEntries* GetEntries() override;
    void SetEntries( std::unique_ptr<SvxEntries> pNewEntries ) override;
    bool HasSettings() override;
    bool HasURL( const OUString& rURL ) override;
    void Reset() override;
    bool Apply() override;

    void ResetContextMenu( const SvxConfigEntry* pEntry );
};

class SvxConfigEntry
{
private:

    /// common properties
    sal_uInt16                  nId;
    OUString                    aLabel;
    OUString                    aCommand;

    bool                        bPopUp;
    bool                        bStrEdited;
    bool                        bIsUserDefined;
    bool                        bIsMain;
    bool                        bIsParentData;
    bool                        bIsModified;

    /// toolbar specific properties
    bool                        bIsVisible;
    sal_Int32                   nStyle;

    css::uno::Reference<
        css::graphic::XGraphic > xBackupGraphic;

    std::unique_ptr<SvxEntries>  mpEntries;

public:

    SvxConfigEntry( const OUString& rDisplayName,
                    const OUString& rCommandURL,
                    bool bPopup,
                    bool bParentData );

    SvxConfigEntry()
        :
            nId( 0 ),
            bPopUp( false ),
            bStrEdited( false ),
            bIsUserDefined( false ),
            bIsMain( false ),
            bIsParentData( false ),
            bIsModified( false ),
            bIsVisible( true ),
            nStyle( 0 )
    {}

    ~SvxConfigEntry();

    const OUString&      GetCommand() const { return aCommand; }

    const OUString&      GetName() const { return aLabel; }
    void                 SetName( const OUString& rStr ) { aLabel = rStr; bStrEdited = true; }
    bool                 HasChangedName() const { return bStrEdited; }

    bool    IsPopup() const { return bPopUp; }

    void    SetUserDefined( bool bOn = true ) { bIsUserDefined = bOn; }
    bool    IsUserDefined() const { return bIsUserDefined; }

    bool    IsBinding() const { return !bPopUp; }
    bool    IsSeparator() const { return nId == 0; }

    SvxEntries* GetEntries() const { return mpEntries.get(); }
    void    SetEntries( std::unique_ptr<SvxEntries> entries ) { mpEntries = std::move(entries); }

    void    SetMain() { bIsMain = true; }
    bool    IsMain() { return bIsMain; }

    void    SetParentData( bool bValue = true ) { bIsParentData = bValue; }
    bool    IsParentData() { return bIsParentData; }

    void    SetModified( bool bValue = true ) { bIsModified = bValue; }
    bool    IsModified() { return bIsModified; }

    bool    IsMovable();
    bool    IsDeletable();
    bool    IsRenamable();

    void    SetVisible( bool b ) { bIsVisible = b; }
    bool    IsVisible() const { return bIsVisible; }

    void    SetBackupGraphic( css::uno::Reference< css::graphic::XGraphic > const & graphic )
                { xBackupGraphic = graphic; }

    const css::uno::Reference< css::graphic::XGraphic >& GetBackupGraphic()
                { return xBackupGraphic; }

    sal_Int32   GetStyle() const { return nStyle; }
    void        SetStyle( sal_Int32 style ) { nStyle = style; }
};

class SvxMenuEntriesListBox
{
protected:
    std::unique_ptr<weld::TreeView> m_xControl;
    ScopedVclPtr<VirtualDevice> m_xDropDown;
    VclPtr<SvxConfigPage> pPage;

public:
    SvxMenuEntriesListBox(std::unique_ptr<weld::TreeView> xControl, SvxConfigPage* pPage);
    virtual ~SvxMenuEntriesListBox();

    int get_selected_index() const { return m_xControl->get_selected_index(); }
    OUString get_id(int nPos) const { return m_xControl->get_id(nPos); }
    void remove(int nPos) { m_xControl->remove(nPos); }
    int n_children() const { return m_xControl->n_children(); }
    void set_text(int row, const OUString& rText, int col) { m_xControl->set_text(row, rText, col); }
    void set_image(int row, const css::uno::Reference<css::graphic::XGraphic>& rImage, int col) { m_xControl->set_image(row, rImage, col); }
    void set_dropdown(int row, int col) { m_xControl->set_image(row, *m_xDropDown, col); }
    void set_id(int row, const OUString& rId) { m_xControl->set_id(row, rId); }
    void clear() { m_xControl->clear(); } //need frees ?
    void set_toggle(int row, bool bOn, int col) { m_xControl->set_toggle(row, bOn, col); }
    void scroll_to_row(int pos) { m_xControl->scroll_to_row(pos); }
    void select(int pos) { m_xControl->select(pos); }

    weld::TreeView& get_widget() { return *m_xControl; }

    void insert(int pos, const OUString& rId)
    {
        m_xControl->insert(nullptr, pos, nullptr, &rId,
                           nullptr, nullptr, nullptr, false, nullptr);
    }

    void insert(int pos, const OUString& rId, const OUString& rStr, const OUString* pImage = nullptr)
    {
        m_xControl->insert(pos, rStr, &rId, pImage, nullptr);
    }

    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);

    void CreateDropDown();
};

class SvxConfigPage : public SfxTabPage
{
private:

    Timer                               m_aUpdateDataTimer;
    bool                                bInitialised;
    SaveInData*                         pCurrentSaveInData;

    DECL_LINK(SearchUpdateHdl, weld::Entry&, void);

protected:

    /// the ResourceURL to select when opening the dialog
    OUString                                   m_aURLToSelect;

    css::uno::Reference< css::frame::XFrame >  m_xFrame;

    // Left side of the dialog where command categories and the available
    // commands in them are displayed as a searchable list
    std::unique_ptr<weld::Entry>               m_xSearchEdit;
    std::unique_ptr<CommandCategoryListBox>    m_xCommandCategoryListBox;
    std::unique_ptr<CuiConfigFunctionListBox>  m_xFunctions;

    std::unique_ptr<weld::Label>               m_xDescriptionFieldLb;
    std::unique_ptr<weld::TextView>            m_xDescriptionField;

    // Right side of the dialog where the contents of the selected
    // menu or toolbar are displayed
    std::unique_ptr<weld::ComboBox>            m_xTopLevelListBox;
    // Used to add and remove toolbars/menus
    std::unique_ptr<weld::MenuButton>          m_xGearBtn;
    std::unique_ptr<SvxMenuEntriesListBox>     m_xContentsListBox;

    std::unique_ptr<weld::Button>              m_xMoveUpButton;
    std::unique_ptr<weld::Button>              m_xMoveDownButton;

    std::unique_ptr<weld::ComboBox>            m_xSaveInListBox;

    std::unique_ptr<weld::MenuButton>          m_xInsertBtn;
    std::unique_ptr<weld::MenuButton>          m_xModifyBtn;
    // Used to reset the selected toolbar/menu/context menu
    std::unique_ptr<weld::Button>              m_xResetBtn;

    // Middle buttons
    std::unique_ptr<weld::Button>              m_xAddCommandButton;
    std::unique_ptr<weld::Button>              m_xRemoveCommandButton;


    SvxConfigPage(TabPageParent, const SfxItemSet&);

    DECL_LINK(MoveHdl, weld::Button&, void);
    DECL_LINK(SelectFunctionHdl, weld::TreeView&, void);
    DECL_LINK(FunctionDoubleClickHdl, weld::TreeView&, void);
    DECL_LINK(SelectSaveInLocation, weld::ComboBox&, void);
    DECL_LINK(SelectElementHdl, weld::ComboBox&, void);
    DECL_LINK(ImplUpdateDataHdl, Timer*, void);
    DECL_LINK(FocusOut_Impl, weld::Widget&, void);

    virtual SaveInData* CreateSaveInData(
        const css::uno::Reference< css::ui::XUIConfigurationManager >&,
        const css::uno::Reference< css::ui::XUIConfigurationManager >&,
        const OUString& aModuleId,
        bool docConfig ) = 0;

    virtual void            Init() = 0;
    virtual void            UpdateButtonStates() = 0;
    virtual short           QueryReset() = 0;

    virtual void            SelectElement() = 0;

    int                 InsertEntry(SvxConfigEntry* pNewEntryData,
                                    int nTarget,
                                    bool bFront = false);

    void                AddSubMenusToUI(    const OUString& rBaseTitle,
                                        SvxConfigEntry const * pParentData );

    void                InsertEntryIntoUI(SvxConfigEntry* pNewEntryData,
                                          int nPos, int nStartCol);

    SvxEntries*     FindParentForChild( SvxEntries* pParentEntries,
                                        SvxConfigEntry* pChildData );

    void            ReloadTopLevelListBox( SvxConfigEntry const * pSelection = nullptr );

public:

    virtual ~SvxConfigPage() override;

    static bool     CanConfig( const OUString& rModuleId );

    SaveInData*     GetSaveInData() { return pCurrentSaveInData; }

    int             AddFunction(int nTarget = -1,
                                bool bFront = false,
                                bool bAllowDuplicates = false);

    virtual void    MoveEntry( bool bMoveUp );

    bool            MoveEntryData(int SourceEntry, int nTargetEntry);

    bool            FillItemSet( SfxItemSet* ) override;
    void            Reset( const SfxItemSet* ) override;

    virtual void    DeleteSelectedContent() = 0;
    virtual void    DeleteSelectedTopLevel() = 0;

    SvxConfigEntry* GetTopLevelSelection()
    {
        return reinterpret_cast<SvxConfigEntry*>(m_xTopLevelListBox->get_active_id().toInt64());
    }

    /** identifies the module in the given frame. If the frame is <NULL/>, a default
        frame will be determined beforehand.

        If the given frame is <NULL/>, a default frame will be used: The method the active
        frame of the desktop, then the current frame. If both are <NULL/>,
        the SfxViewFrame::Current's XFrame is used. If this is <NULL/>, too, an empty string is returned.

        If the given frame is not <NULL/>, or an default frame could be successfully determined, then
        the ModuleManager is asked for the module ID of the component in the frame.
    */
    static OUString
        GetFrameWithDefaultAndIdentify( css::uno::Reference< css::frame::XFrame >& _inout_rxFrame );

    OUString    GetScriptURL() const;
    OUString    GetSelectedDisplayName();
};

class SvxMainMenuOrganizerDialog : public weld::GenericDialogController
{
    std::unique_ptr<SvxEntries> mpEntries;
    OUString m_sNewMenuEntryId;

    std::unique_ptr<weld::Widget> m_xMenuBox;
    std::unique_ptr<weld::Entry> m_xMenuNameEdit;
    std::unique_ptr<weld::TreeView> m_xMenuListBox;
    std::unique_ptr<weld::Button> m_xMoveUpButton;
    std::unique_ptr<weld::Button> m_xMoveDownButton;

    void UpdateButtonStates();

    DECL_LINK(MoveHdl, weld::Button&, void);
    DECL_LINK(ModifyHdl, weld::Entry&, void);
    DECL_LINK(SelectHdl, weld::TreeView&, void);

public:
    SvxMainMenuOrganizerDialog(
        weld::Window*, SvxEntries*,
        SvxConfigEntry const *, bool bCreateMenu);
    virtual ~SvxMainMenuOrganizerDialog() override;

    std::unique_ptr<SvxEntries> ReleaseEntries() { return std::move(mpEntries);}
    SvxConfigEntry* GetSelectedEntry();
};

class ToolbarSaveInData : public SaveInData
{
private:

    std::unique_ptr<SvxConfigEntry>           pRootEntry;
    OUString                                  m_aDescriptorContainer;

    css::uno::Reference
        < css::container::XNameAccess > m_xPersistentWindowState;

    void        LoadToolbar(
        const css::uno::Reference< css::container::XIndexAccess >& xToolBarSettings,
        SvxConfigEntry const * pParentData );

    void        ApplyToolbar(
        css::uno::Reference< css::container::XIndexContainer > const & rNewToolbarBar,
        css::uno::Reference< css::lang::XSingleComponentFactory >& rFactory,
        SvxConfigEntry const *pToolbar );

public:

    ToolbarSaveInData(
        const css::uno::Reference< css::ui::XUIConfigurationManager >&,
        const css::uno::Reference< css::ui::XUIConfigurationManager >&,
        const OUString& aModuleId,
        bool docConfig );

    virtual ~ToolbarSaveInData() override;

    void            CreateToolbar( SvxConfigEntry* pToolbar );
    void            RestoreToolbar( SvxConfigEntry* pToolbar );
    void            RemoveToolbar( SvxConfigEntry* pToolbar );
    void            ApplyToolbar( SvxConfigEntry* pToolbar );

    OUString        GetSystemUIName( const OUString& rResourceURL );

    sal_Int32       GetSystemStyle( const OUString& rResourceURL );

    void            SetSystemStyle( const OUString& rResourceURL, sal_Int32 nStyle );

    void            SetSystemStyle(
                        const css::uno::Reference< css::frame::XFrame >& xFrame,
                        const OUString& rResourceURL, sal_Int32 nStyle );

    SvxEntries*     GetEntries() override;
    void            SetEntries( std::unique_ptr<SvxEntries> ) override;
    bool            HasSettings() override;
    bool            HasURL( const OUString& rURL ) override;
    void            Reset() override;
    bool            Apply() override;
};

class SvxNewToolbarDialog : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Entry> m_xEdtName;
    std::unique_ptr<weld::Button> m_xBtnOK;
public:
    std::unique_ptr<weld::ComboBox> m_xSaveInListBox;

    SvxNewToolbarDialog(weld::Window* pWindow, const OUString& rName);
    virtual ~SvxNewToolbarDialog() override;

    OUString GetName()
    {
        return m_xEdtName->get_text();
    }
};

class SvxIconSelectorDialog : public weld::GenericDialogController
{
private:
    sal_Int32       m_nExpectedSize;

    css::uno::Reference<
        css::ui::XImageManager > m_xImageManager;

    css::uno::Reference<
        css::ui::XImageManager > m_xParentImageManager;

    css::uno::Reference<
        css::ui::XImageManager > m_xImportedImageManager;

    css::uno::Reference<
        css::graphic::XGraphicProvider > m_xGraphProvider;

    std::vector<css::uno::Reference<css::graphic::XGraphic>> m_aGraphics;

    std::unique_ptr<SvtValueSet> m_xTbSymbol;
    std::unique_ptr<weld::CustomWeld> m_xTbSymbolWin;
    std::unique_ptr<weld::Label>      m_xFtNote;
    std::unique_ptr<weld::Button>     m_xBtnImport;
    std::unique_ptr<weld::Button>     m_xBtnDelete;

    bool ReplaceGraphicItem( const OUString& aURL );

    bool ImportGraphic( const OUString& aURL );

    void ImportGraphics( const css::uno::Sequence< OUString >& aURLs );

public:

    SvxIconSelectorDialog(
        weld::Window *pWindow,
        const css::uno::Reference< css::ui::XImageManager >& rXImageManager,
        const css::uno::Reference< css::ui::XImageManager >& rXParentImageManager);

    virtual ~SvxIconSelectorDialog() override;

    css::uno::Reference< css::graphic::XGraphic >
        GetSelectedIcon();

    DECL_LINK(SelectHdl, SvtValueSet*, void);
    DECL_LINK(ImportHdl, weld::Button&, void);
    DECL_LINK(DeleteHdl, weld::Button&, void);
};

//added for issue83555
class SvxIconChangeDialog : public weld::MessageDialogController
{
private:
    std::unique_ptr<weld::TextView> m_xLineEditDescription;
public:
    SvxIconChangeDialog(weld::Window *pWindow, const OUString& rMessage);
};

#endif // INCLUDED_CUI_SOURCE_INC_CFG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

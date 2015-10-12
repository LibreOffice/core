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
#include <vcl/group.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/toolbox.hxx>
#include <svtools/treelistbox.hxx>
#include <svtools/svmedit2.hxx>
#include <svtools/svmedit.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/ui/XUIConfigurationListener.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>

#include <sfx2/tabdlg.hxx>
#include <vector>
#include <vcl/msgbox.hxx>

#include "selector.hxx"

class SvxConfigEntry;
class SvxConfigPage;
class SvxMenuConfigPage;
class SvxToolbarConfigPage;

typedef std::vector< SvxConfigEntry* > SvxEntries;

class SvxConfigDialog : public SfxTabDialog
{
private:
    css::uno::Reference< css::frame::XFrame > m_xFrame;
    sal_uInt16 m_nMenusPageId;
    sal_uInt16 m_nKeyboardPageId;
    sal_uInt16 m_nToolbarsPageId;
    sal_uInt16 m_nEventsPageId;

public:
    SvxConfigDialog( vcl::Window*, const SfxItemSet* );

    virtual void                PageCreated( sal_uInt16 nId, SfxTabPage &rPage ) override;
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

    css::uno::Reference
        < css::ui::XUIConfigurationManager >
            GetConfigManager() { return m_xCfgMgr; };

    css::uno::Reference
        < css::ui::XUIConfigurationManager >
            GetParentConfigManager() { return m_xParentCfgMgr; };

    css::uno::Reference
        < css::ui::XImageManager >
            GetImageManager() { return m_xImgMgr; };

    css::uno::Reference
        < css::ui::XImageManager >
            GetParentImageManager() { return m_xParentImgMgr; };

    css::uno::Reference
        < css::container::XNameAccess > m_xCommandToLabelMap;

    css::uno::Sequence
        < css::beans::PropertyValue > m_aSeparatorSeq;

    Image GetImage( const OUString& rCommandURL );

    virtual bool HasURL( const OUString& aURL ) = 0;
    virtual bool HasSettings() = 0;
    virtual SvxEntries* GetEntries() = 0;
    virtual void SetEntries( SvxEntries* ) = 0;
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

    SvxConfigEntry* pRootEntry;


    static MenuSaveInData* pDefaultData;    ///< static holder of the default menu data

    static void SetDefaultData( MenuSaveInData* pData ) {pDefaultData = pData;}
    static MenuSaveInData* GetDefaultData() { return pDefaultData; }

    void        Apply(
        SvxConfigEntry* pRootEntry,
        css::uno::Reference< css::container::XIndexContainer >& rNewMenuBar,
        css::uno::Reference< css::lang::XSingleComponentFactory >& rFactory,
        SvTreeListEntry *pParent = NULL );

    void        ApplyMenu(
        css::uno::Reference< css::container::XIndexContainer >& rNewMenuBar,
        css::uno::Reference< css::lang::XSingleComponentFactory >& rFactory,
        SvxConfigEntry *pMenuData = NULL );

    bool        LoadSubMenus(
        const css::uno::Reference< css::container::XIndexAccess >& xMenuBarSettings,
        const OUString& rBaseTitle, SvxConfigEntry* pParentData );

public:

    MenuSaveInData(
        const css::uno::Reference< css::ui::XUIConfigurationManager >&,
        const css::uno::Reference< css::ui::XUIConfigurationManager >&,
        const OUString& aModuleId,
        bool docConfig );

    virtual ~MenuSaveInData();

    /// methods inherited from SaveInData
    SvxEntries*         GetEntries() override;
    void                SetEntries( SvxEntries* ) override;
    bool                HasURL( const OUString& URL ) override { (void)URL; return false; }
    bool                HasSettings() override { return m_xMenuSettings.is(); }
    void                Reset() override;
    bool                Apply() override;
};

class SvxConfigEntry
{
private:

    /// common properties
    sal_uInt16                  nId;
    OUString                    aHelpText;
    OUString                    aLabel;
    OUString                    aCommand;

    bool                        bPopUp;
    bool                        bStrEdited;
    bool                        bIsUserDefined;
    bool                        bIsMain;
    bool                        bIsParentData;

    /// toolbar specific properties
    bool                        bIsVisible;
    sal_Int32                   nStyle;

    css::uno::Reference<
        css::graphic::XGraphic > xBackupGraphic;

    SvxEntries                  *mpEntries;

public:

    SvxConfigEntry( const OUString& rDisplayName,
                    const OUString& rCommandURL,
                    bool bPopup = false,
                    bool bParentData = false );

    SvxConfigEntry()
        :
            nId( 0 ),
            bPopUp( false ),
            bStrEdited( false ),
            bIsUserDefined( false ),
            bIsMain( false ),
            bIsParentData( false ),
            bIsVisible( true ),
            nStyle( 0 ),
            mpEntries( 0 )
    {}

    ~SvxConfigEntry();

    const OUString&      GetCommand() const { return aCommand; }

    const OUString&      GetName() const { return aLabel; }
    void                 SetName( const OUString& rStr ) { aLabel = rStr; bStrEdited = true; }
    bool                 HasChangedName() const { return bStrEdited; }

    const OUString&      GetHelpText();

    bool    IsPopup() const { return bPopUp; }

    void    SetUserDefined( bool bOn = true ) { bIsUserDefined = bOn; }
    bool    IsUserDefined() const { return bIsUserDefined; }

    bool    IsBinding() const { return !bPopUp; }
    bool    IsSeparator() const { return nId == 0; }

    SvxEntries* GetEntries() const { return mpEntries; }
    void    SetEntries( SvxEntries* entries ) { mpEntries = entries; }

    void    SetMain( bool bValue = true ) { bIsMain = bValue; }
    bool    IsMain() { return bIsMain; }

    void    SetParentData( bool bValue = true ) { bIsParentData = bValue; }
    bool    IsParentData() { return bIsParentData; }

    bool    IsMovable();
    bool    IsDeletable();
    bool    IsRenamable();

    void    SetVisible( bool b ) { bIsVisible = b; }
    bool    IsVisible() const { return bIsVisible; }

    void    SetBackupGraphic( css::uno::Reference< css::graphic::XGraphic > graphic )
                { xBackupGraphic = graphic; }

    css::uno::Reference< css::graphic::XGraphic > GetBackupGraphic()
                { return xBackupGraphic; }

    bool    IsIconModified() { return xBackupGraphic.is(); }

    sal_Int32   GetStyle() { return nStyle; }
    void        SetStyle( sal_Int32 style ) { nStyle = style; }
};

class SvxMenuEntriesListBox : public SvTreeListBox
{
private:
    VclPtr<SvxConfigPage>      pPage;

protected:
    bool                m_bIsInternalDrag;

public:
    SvxMenuEntriesListBox(vcl::Window*, SvxConfigPage*);
    virtual ~SvxMenuEntriesListBox();
    virtual void dispose() override;

    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt ) override;

    virtual bool        NotifyAcceptDrop( SvTreeListEntry* pEntry ) override;

    virtual TriState    NotifyMoving( SvTreeListEntry*, SvTreeListEntry*,
                                      SvTreeListEntry*&, sal_uLong& ) override;

    virtual TriState    NotifyCopying( SvTreeListEntry*, SvTreeListEntry*,
                                       SvTreeListEntry*&, sal_uLong&) override;

    virtual DragDropMode    NotifyStartDrag(
        TransferDataContainer&, SvTreeListEntry* ) override;

    virtual void        DragFinished( sal_Int8 ) override;

    void                KeyInput( const KeyEvent& rKeyEvent ) override;
};

class SvxConfigPage : public SfxTabPage
{
private:

    bool                                bInitialised;
    SaveInData*                         pCurrentSaveInData;

    DECL_LINK_TYPED(  SelectSaveInLocation, ListBox&, void );
    DECL_LINK_TYPED( AsyncInfoMsg, void*, void );

protected:

    // the top section of the tab page where top level menus and toolbars
    //  are displayed in a listbox
    VclPtr<VclFrame>                           m_pTopLevel;
    VclPtr<FixedText>                          m_pTopLevelLabel;
    VclPtr<ListBox>                            m_pTopLevelListBox;
    VclPtr<PushButton>                         m_pNewTopLevelButton;
    VclPtr<MenuButton>                         m_pModifyTopLevelButton;

    // the contents section where the contents of the selected
    // menu or toolbar are displayed
    VclPtr<VclFrame>                           m_pContents;
    VclPtr<FixedText>                          m_pContentsLabel;
    VclPtr<VclContainer>                       m_pEntries;
    VclPtr<SvTreeListBox>                      m_pContentsListBox;

    VclPtr<PushButton>                         m_pAddCommandsButton;
    VclPtr<MenuButton>                         m_pModifyCommandButton;

    VclPtr<PushButton>                         m_pMoveUpButton;
    VclPtr<PushButton>                         m_pMoveDownButton;

    VclPtr<ListBox>                            m_pSaveInListBox;

    VclPtr<VclMultiLineEdit>                   m_pDescriptionField;

    VclPtr<SvxScriptSelectorDialog>            m_pSelectorDlg;

    /// the ResourceURL to select when opening the dialog
    OUString                                   m_aURLToSelect;

    css::uno::Reference< css::frame::XFrame >  m_xFrame;

    SvxConfigPage( vcl::Window*, const SfxItemSet& );

    DECL_LINK_TYPED( MoveHdl, Button *, void );

    virtual SaveInData* CreateSaveInData(
        const css::uno::Reference< css::ui::XUIConfigurationManager >&,
        const css::uno::Reference< css::ui::XUIConfigurationManager >&,
        const OUString& aModuleId,
        bool docConfig ) = 0;

    virtual void            Init() = 0;
    virtual void            UpdateButtonStates() = 0;
    virtual short           QueryReset() = 0;

    SvTreeListEntry*    InsertEntry(        SvxConfigEntry* pNewEntryData,
                                        SvTreeListEntry* pTarget = NULL,
                                        bool bFront = false );

    void                AddSubMenusToUI(    const OUString& rBaseTitle,
                                        SvxConfigEntry* pParentData );

    SvTreeListEntry*    InsertEntryIntoUI ( SvxConfigEntry* pNewEntryData,
                                        sal_uLong nPos = TREELIST_APPEND );

    SvxEntries*     FindParentForChild( SvxEntries* pParentEntries,
                                        SvxConfigEntry* pChildData );

    void            ReloadTopLevelListBox( SvxConfigEntry* pSelection = NULL );

public:

    virtual ~SvxConfigPage();
    virtual void dispose() override;

    static bool     CanConfig( const OUString& rModuleId );

    SaveInData*     GetSaveInData() { return pCurrentSaveInData; }

    SvTreeListEntry*    AddFunction( SvTreeListEntry* pTarget = NULL,
                                 bool bFront = false,
                                 bool bAllowDuplicates = false );

    virtual void    MoveEntry( bool bMoveUp );

    bool            MoveEntryData(  SvTreeListEntry* pSourceEntry,
                                    SvTreeListEntry* pTargetEntry );

    bool            FillItemSet( SfxItemSet* ) override;
    void            Reset( const SfxItemSet* ) override;

    virtual bool    DeleteSelectedContent() = 0;
    virtual void    DeleteSelectedTopLevel() = 0;

    SvxConfigEntry* GetTopLevelSelection()
    {
        return static_cast<SvxConfigEntry*>(m_pTopLevelListBox->GetEntryData(
            m_pTopLevelListBox->GetSelectEntryPos() ));
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
};

class SvxMenuConfigPage : public SvxConfigPage
{
private:

    DECL_LINK_TYPED( SelectMenu, ListBox&, void );
    DECL_LINK_TYPED( SelectMenuEntry, SvTreeListBox *, void );
    DECL_LINK_TYPED( NewMenuHdl, Button *, void );
    DECL_LINK_TYPED( MenuSelectHdl, MenuButton *, void );
    DECL_LINK_TYPED( EntrySelectHdl, MenuButton *, void );
    DECL_LINK_TYPED( AddCommandsHdl, Button *, void );
    DECL_LINK_TYPED( AddFunctionHdl, SvxScriptSelectorDialog&, void );

    void            Init() override;
    void            UpdateButtonStates() override;
    short           QueryReset() override;
    bool            DeleteSelectedContent() override;
    void            DeleteSelectedTopLevel() override;

public:
    SvxMenuConfigPage( vcl::Window *pParent, const SfxItemSet& rItemSet );
    virtual ~SvxMenuConfigPage();
    virtual void dispose() override;

    SaveInData* CreateSaveInData(
        const css::uno::Reference <
            css::ui::XUIConfigurationManager >&,
        const css::uno::Reference <
            css::ui::XUIConfigurationManager >&,
        const OUString& aModuleId,
        bool docConfig ) override;
};

class SvxMainMenuOrganizerDialog : public ModalDialog
{
    VclPtr<VclContainer>   m_pMenuBox;
    VclPtr<Edit>           m_pMenuNameEdit;
    VclPtr<SvTreeListBox>  m_pMenuListBox;
    VclPtr<PushButton>     m_pMoveUpButton;
    VclPtr<PushButton>     m_pMoveDownButton;

    SvxEntries*     mpEntries;
    SvTreeListEntry*    pNewMenuEntry;
    bool            bModified;

    void UpdateButtonStates();

    DECL_LINK_TYPED( MoveHdl, Button *, void );
    DECL_LINK( ModifyHdl, Edit * );
    DECL_LINK_TYPED( SelectHdl, SvTreeListBox*, void );

public:
    SvxMainMenuOrganizerDialog (
        vcl::Window*, SvxEntries*,
        SvxConfigEntry*, bool bCreateMenu = false );
    virtual ~SvxMainMenuOrganizerDialog();
    virtual void dispose() override;

    SvxEntries*     GetEntries() { return mpEntries;}
    SvxConfigEntry* GetSelectedEntry();
};

class SvxToolbarEntriesListBox : public SvxMenuEntriesListBox
{
    Size            m_aCheckBoxImageSizePixel;
    SvLBoxButtonData*   m_pButtonData;
    VclPtr<SvxConfigPage>  pPage;

    void            ChangeVisibility( SvTreeListEntry* pEntry );

protected:

    virtual void    CheckButtonHdl() override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
    void            BuildCheckBoxButtonImages( SvLBoxButtonData* );
    Image           GetSizedImage(
        VirtualDevice& aDev, const Size& aNewSize, const Image& aImage );

public:

    SvxToolbarEntriesListBox(vcl::Window* pParent, SvxToolbarConfigPage* pPg);
    virtual ~SvxToolbarEntriesListBox();
    virtual void dispose() override;

    virtual TriState NotifyMoving(
        SvTreeListEntry*, SvTreeListEntry*, SvTreeListEntry*&, sal_uLong& ) override;

    virtual TriState NotifyCopying(
        SvTreeListEntry*, SvTreeListEntry*, SvTreeListEntry*&, sal_uLong&) override;

    void            KeyInput( const KeyEvent& rKeyEvent ) override;
};

class SvxToolbarConfigPage : public SvxConfigPage
{
private:

    DECL_LINK_TYPED( SelectToolbar, ListBox&, void );
    DECL_LINK_TYPED( SelectToolbarEntry, SvTreeListBox*, void );
    DECL_LINK_TYPED( ToolbarSelectHdl, MenuButton *, void );
    DECL_LINK_TYPED( EntrySelectHdl, MenuButton *, void );
    DECL_LINK_TYPED( NewToolbarHdl, Button *, void );
    DECL_LINK_TYPED( AddCommandsHdl, Button *, void );
    DECL_LINK_TYPED( AddFunctionHdl, SvxScriptSelectorDialog&, void );
    DECL_LINK_TYPED( MoveHdl, Button *, void );

    void            UpdateButtonStates() override;
    short           QueryReset() override;
    void            Init() override;
    bool            DeleteSelectedContent() override;
    void            DeleteSelectedTopLevel() override;

public:
    SvxToolbarConfigPage( vcl::Window *pParent, const SfxItemSet& rItemSet );
    virtual ~SvxToolbarConfigPage();
    virtual void dispose() override;

    SvTreeListEntry*    AddFunction( SvTreeListEntry* pTarget = NULL,
                                             bool bFront = false,
                                             bool bAllowDuplicates = true );

    void            MoveEntry( bool bMoveUp ) override;

    SaveInData*     CreateSaveInData(
        const css::uno::Reference <
            css::ui::XUIConfigurationManager >&,
        const css::uno::Reference <
            css::ui::XUIConfigurationManager >&,
        const OUString& aModuleId,
        bool docConfig ) override;
};

class ToolbarSaveInData : public SaveInData
{
private:

    SvxConfigEntry*                                pRootEntry;
    OUString                                  m_aDescriptorContainer;

    css::uno::Reference
        < css::container::XNameAccess > m_xPersistentWindowState;

    bool        LoadToolbar(
        const css::uno::Reference< css::container::XIndexAccess >& xToolBarSettings,
        SvxConfigEntry* pParentData );

    void        ApplyToolbar(
        css::uno::Reference< css::container::XIndexContainer >& rNewToolbarBar,
        css::uno::Reference< css::lang::XSingleComponentFactory >& rFactory,
        SvxConfigEntry *pToolbar = NULL );

public:

    ToolbarSaveInData(
        const css::uno::Reference< css::ui::XUIConfigurationManager >&,
        const css::uno::Reference< css::ui::XUIConfigurationManager >&,
        const OUString& aModuleId,
        bool docConfig );

    virtual ~ToolbarSaveInData();

    void            CreateToolbar( SvxConfigEntry* pToolbar );
    void            RestoreToolbar( SvxConfigEntry* pToolbar );
    void            RemoveToolbar( SvxConfigEntry* pToolbar );
    void            ApplyToolbar( SvxConfigEntry* pToolbar );

    OUString        GetSystemUIName( const OUString& rResourceURL );

    sal_Int32       GetSystemStyle( const OUString& rResourceURL );

    void            SetSystemStyle( const OUString& rResourceURL, sal_Int32 nStyle );

    void            SetSystemStyle(
        css::uno::Reference< css::frame::XFrame > xFrame,
        const OUString& rResourceURL, sal_Int32 nStyle );

    SvxEntries*     GetEntries() override;
    void            SetEntries( SvxEntries* ) override;
    bool            HasSettings() override;
    bool            HasURL( const OUString& rURL ) override;
    void            Reset() override;
    bool            Apply() override;
};

class SvxNewToolbarDialog : public ModalDialog
{
private:
    VclPtr<Edit>           m_pEdtName;
    VclPtr<OKButton>       m_pBtnOK;

public:
    SvxNewToolbarDialog(vcl::Window* pWindow, const OUString& rName);
    virtual ~SvxNewToolbarDialog();
    virtual void dispose() override;

    VclPtr<ListBox>        m_pSaveInListBox;

    OUString GetName()
    {
        return m_pEdtName->GetText();
    }
};

struct SvxIconSelectorToolBoxItem
{
    Image aImg;
    OUString aText;
    void* pData;
};

class SvxIconSelectorDialog : public ModalDialog
{
private:
    VclPtr<ToolBox>        pTbSymbol;
    VclPtr<FixedText>      pFtNote;
    VclPtr<PushButton>     pBtnImport;
    VclPtr<PushButton>     pBtnDelete;
    Size            aTbSize;
    sal_uInt16      m_nNextId;

    sal_Int32       m_nExpectedSize;

    css::uno::Reference<
        css::ui::XImageManager > m_xImageManager;

    css::uno::Reference<
        css::ui::XImageManager > m_xParentImageManager;

    css::uno::Reference<
        css::ui::XImageManager > m_xImportedImageManager;

    css::uno::Reference<
        css::graphic::XGraphicProvider > m_xGraphProvider;

    bool ReplaceGraphicItem( const OUString& aURL );

    bool ImportGraphic( const OUString& aURL );

    void ImportGraphics( const css::uno::Sequence< OUString >& aURLs );

public:

    SvxIconSelectorDialog(
        vcl::Window *pWindow,
        const css::uno::Reference< css::ui::XImageManager >& rXImageManager,
        const css::uno::Reference< css::ui::XImageManager >& rXParentImageManager
            );

    virtual ~SvxIconSelectorDialog();
    virtual void dispose() override;

    css::uno::Reference< css::graphic::XGraphic >
        GetSelectedIcon();

    DECL_LINK_TYPED( SelectHdl, ToolBox *, void );
    DECL_LINK_TYPED( ImportHdl, Button *, void );
    DECL_LINK_TYPED( DeleteHdl, Button *, void );
};

class SvxIconReplacementDialog : public MessBox
{
public:
    SvxIconReplacementDialog(
        vcl::Window *pWindow,
        const OUString& aMessage,
        bool aYestoAll);

    SvxIconReplacementDialog(
        vcl::Window *pWindow,
        const OUString& aMessage );

    static OUString ReplaceIconName( const OUString& );
    sal_uInt16 ShowDialog();
};
//added for issue83555
class SvxIconChangeDialog : public ModalDialog
{
private:
    VclPtr<FixedImage>         pFImageInfo;
    VclPtr<VclMultiLineEdit>   pLineEditDescription;
public:
    SvxIconChangeDialog(vcl::Window *pWindow, const OUString& aMessage);
    virtual ~SvxIconChangeDialog();
    virtual void dispose() override;
};
#endif // INCLUDED_CUI_SOURCE_INC_CFG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

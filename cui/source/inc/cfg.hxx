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
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xFrame;
    sal_uInt16 m_nMenusPageId;
    sal_uInt16 m_nKeyboardPageId;
    sal_uInt16 m_nToolbarsPageId;
    sal_uInt16 m_nEventsPageId;

public:
    SvxConfigDialog( Window*, const SfxItemSet* );

    virtual void                PageCreated( sal_uInt16 nId, SfxTabPage &rPage ) SAL_OVERRIDE;
    void SetFrame(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame);
};

class SaveInData : public ImageProvider
{
private:

    bool        bModified;

    bool        bDocConfig;
    bool        bReadOnly;

    ::com::sun::star::uno::Reference
        < com::sun::star::ui::XUIConfigurationManager > m_xCfgMgr;

    ::com::sun::star::uno::Reference
        < com::sun::star::ui::XUIConfigurationManager > m_xParentCfgMgr;

    ::com::sun::star::uno::Reference
        < com::sun::star::ui::XImageManager > m_xImgMgr;

    ::com::sun::star::uno::Reference
        < com::sun::star::ui::XImageManager > m_xParentImgMgr;

    static ::com::sun::star::uno::Reference
        < com::sun::star::ui::XImageManager >* xDefaultImgMgr;

public:

    SaveInData(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::ui::XUIConfigurationManager >& xCfgMgr,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::ui::XUIConfigurationManager >& xParentCfgMgr,
        const OUString& aModuleId,
        bool docConfig );

    ~SaveInData() {}

    bool PersistChanges(
        const com::sun::star::uno::Reference
            < com::sun::star::uno::XInterface >& xManager );

    void SetModified( bool bValue = true ) { bModified = bValue; }
    bool IsModified( ) { return bModified; }

    bool IsReadOnly( ) { return bReadOnly; }
    bool IsDocConfig( ) { return bDocConfig; }

    ::com::sun::star::uno::Reference
        < ::com::sun::star::ui::XUIConfigurationManager >
            GetConfigManager() { return m_xCfgMgr; };

    ::com::sun::star::uno::Reference
        < ::com::sun::star::ui::XUIConfigurationManager >
            GetParentConfigManager() { return m_xParentCfgMgr; };

    ::com::sun::star::uno::Reference
        < ::com::sun::star::ui::XImageManager >
            GetImageManager() { return m_xImgMgr; };

    ::com::sun::star::uno::Reference
        < ::com::sun::star::ui::XImageManager >
            GetParentImageManager() { return m_xParentImgMgr; };

    ::com::sun::star::uno::Reference
        < com::sun::star::container::XNameAccess > m_xCommandToLabelMap;

    com::sun::star::uno::Sequence
        < com::sun::star::beans::PropertyValue > m_aSeparatorSeq;

    Image GetImage( const OUString& rCommandURL ) SAL_OVERRIDE;

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

    ::com::sun::star::uno::Reference
        < com::sun::star::container::XIndexAccess > m_xMenuSettings;

    SvxConfigEntry* pRootEntry;


    static MenuSaveInData* pDefaultData;    ///< static holder of the default menu data

    static void SetDefaultData( MenuSaveInData* pData ) {pDefaultData = pData;}
    static MenuSaveInData* GetDefaultData() { return pDefaultData; }

    void        Apply( bool bDefault );

    void        Apply(
        SvxConfigEntry* pRootEntry,
        com::sun::star::uno::Reference<
            com::sun::star::container::XIndexContainer >& rNewMenuBar,
        com::sun::star::uno::Reference<
            com::sun::star::lang::XSingleComponentFactory >& rFactory,
        SvTreeListEntry *pParent = NULL );

    void        ApplyMenu(
        com::sun::star::uno::Reference<
            com::sun::star::container::XIndexContainer >& rNewMenuBar,
        com::sun::star::uno::Reference<
            com::sun::star::lang::XSingleComponentFactory >& rFactory,
        SvxConfigEntry *pMenuData = NULL );

    bool        LoadSubMenus(
        const ::com::sun::star::uno::Reference<
            com::sun::star::container::XIndexAccess >& xMenuBarSettings,
        const OUString& rBaseTitle, SvxConfigEntry* pParentData );

public:

    MenuSaveInData(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::ui::XUIConfigurationManager >&,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::ui::XUIConfigurationManager >&,
        const OUString& aModuleId,
        bool docConfig );

    ~MenuSaveInData();

    /// methods inherited from SaveInData
    SvxEntries*         GetEntries() SAL_OVERRIDE;
    void                SetEntries( SvxEntries* ) SAL_OVERRIDE;
    bool                HasURL( const OUString& URL ) SAL_OVERRIDE { (void)URL; return false; }
    bool                HasSettings() SAL_OVERRIDE { return m_xMenuSettings.is(); }
    void                Reset() SAL_OVERRIDE;
    bool                Apply() SAL_OVERRIDE;
};

class SvxConfigEntry
{
private:

    /// common properties
    sal_uInt16                      nId;
    OUString             aHelpText;
    OUString             aLabel;
    OUString             aCommand;
    OUString             aHelpURL;

    bool                        bPopUp;
    bool                        bStrEdited;
    bool                        bIsUserDefined;
    bool                        bIsMain;
    bool                        bIsParentData;

    /// toolbar specific properties
    bool                        bIsVisible;
    sal_Int32                   nStyle;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::graphic::XGraphic > xBackupGraphic;

    SvxEntries                  *pEntries;

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
            pEntries( 0 )
    {}

    ~SvxConfigEntry();

    const OUString&      GetCommand() const { return aCommand; }
    void    SetCommand( const OUString& rCmd ) { aCommand = rCmd; }

    const OUString&      GetName() const { return aLabel; }
    void    SetName( const OUString& rStr ) { aLabel = rStr; bStrEdited = true; }
    bool    HasChangedName() const { return bStrEdited; }

    const OUString&      GetHelpText() ;
    void    SetHelpText( const OUString& rStr ) { aHelpText = rStr; }

    const OUString&      GetHelpURL() const { return aHelpURL; }
    void    SetHelpURL( const OUString& rStr ) { aHelpURL = rStr; }

    void    SetPopup( bool bOn = true ) { bPopUp = bOn; }
    bool    IsPopup() const { return bPopUp; }

    void    SetUserDefined( bool bOn = true ) { bIsUserDefined = bOn; }
    bool    IsUserDefined() const { return bIsUserDefined; }

    bool    IsBinding() const { return !bPopUp; }
    bool    IsSeparator() const { return nId == 0; }

    SvxEntries* GetEntries() const { return pEntries; }
    void    SetEntries( SvxEntries* entries ) { pEntries = entries; }
    bool    HasEntries() const { return pEntries != NULL; }

    void    SetMain( bool bValue = true ) { bIsMain = bValue; }
    bool    IsMain() { return bIsMain; }

    void    SetParentData( bool bValue = true ) { bIsParentData = bValue; }
    bool    IsParentData() { return bIsParentData; }

    bool    IsMovable();
    bool    IsDeletable();
    bool    IsRenamable();

    void    SetVisible( bool b ) { bIsVisible = b; }
    bool    IsVisible() const { return bIsVisible; }

    void    SetBackupGraphic(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::graphic::XGraphic > graphic )
                { xBackupGraphic = graphic; }

    ::com::sun::star::uno::Reference<
        ::com::sun::star::graphic::XGraphic >
            GetBackupGraphic()
                { return xBackupGraphic; }

    bool    IsIconModified() { return xBackupGraphic.is(); }

    sal_Int32   GetStyle() { return nStyle; }
    void        SetStyle( sal_Int32 style ) { nStyle = style; }
};

class SvxMenuEntriesListBox : public SvTreeListBox
{
private:
    SvxConfigPage*      pPage;

protected:
    bool                m_bIsInternalDrag;

public:
    SvxMenuEntriesListBox(Window*, SvxConfigPage*);
    ~SvxMenuEntriesListBox();

    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt ) SAL_OVERRIDE;

    virtual bool        NotifyAcceptDrop( SvTreeListEntry* pEntry ) SAL_OVERRIDE;

    virtual sal_Bool        NotifyMoving( SvTreeListEntry*, SvTreeListEntry*,
                                      SvTreeListEntry*&, sal_uLong& ) SAL_OVERRIDE;

    virtual sal_Bool        NotifyCopying( SvTreeListEntry*, SvTreeListEntry*,
                                       SvTreeListEntry*&, sal_uLong&) SAL_OVERRIDE;

    virtual DragDropMode    NotifyStartDrag(
        TransferDataContainer&, SvTreeListEntry* ) SAL_OVERRIDE;

    virtual void        DragFinished( sal_Int8 ) SAL_OVERRIDE;

    void                KeyInput( const KeyEvent& rKeyEvent ) SAL_OVERRIDE;
};

class SvxConfigPage : public SfxTabPage
{
private:

    bool                                bInitialised;
    SaveInData*                         pCurrentSaveInData;

    DECL_LINK(  SelectSaveInLocation, ListBox * );
    DECL_LINK(  AsyncInfoMsg, OUString* );

    bool        SwapEntryData( SvTreeListEntry* pSourceEntry, SvTreeListEntry* pTargetEntry );
    void        AlignControls();

protected:

    // the top section of the tab page where top level menus and toolbars
    //  are displayed in a listbox
    VclFrame*                           m_pTopLevel;
    FixedText*                          m_pTopLevelLabel;
    ListBox*                            m_pTopLevelListBox;
    PushButton*                         m_pNewTopLevelButton;
    MenuButton*                         m_pModifyTopLevelButton;

    // the contents section where the contents of the selected
    // menu or toolbar are displayed
    VclFrame*                           m_pContents;
    FixedText*                          m_pContentsLabel;
    VclContainer*                       m_pEntries;
    SvTreeListBox*                      m_pContentsListBox;

    PushButton*                         m_pAddCommandsButton;
    MenuButton*                         m_pModifyCommandButton;

    PushButton*                         m_pMoveUpButton;
    PushButton*                         m_pMoveDownButton;

    ListBox*                            m_pSaveInListBox;

    VclMultiLineEdit*                   m_pDescriptionField;

    SvxScriptSelectorDialog*            m_pSelectorDlg;

    /// the ResourceURL to select when opening the dialog
    OUString                            m_aURLToSelect;

    ::com::sun::star::uno::Reference
        < ::com::sun::star::frame::XFrame > m_xFrame;

    SvxConfigPage( Window*, const SfxItemSet& );

    DECL_LINK( MoveHdl, Button * );

    virtual SaveInData* CreateSaveInData(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::ui::XUIConfigurationManager >&,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::ui::XUIConfigurationManager >&,
        const OUString& aModuleId,
        bool docConfig ) = 0;

    virtual void            Init() = 0;
    virtual void            UpdateButtonStates() = 0;
    virtual short           QueryReset() = 0;

    SvTreeListEntry*    InsertEntry(        SvxConfigEntry* pNewEntryData,
                                        SvTreeListEntry* pTarget = NULL,
                                        bool bFront = false );

    void            AddSubMenusToUI(    const OUString& rBaseTitle,
                                        SvxConfigEntry* pParentData );

    SvTreeListEntry*    InsertEntryIntoUI ( SvxConfigEntry* pNewEntryData,
                                        sal_uLong nPos = TREELIST_APPEND );

    SvxEntries*     FindParentForChild( SvxEntries* pParentEntries,
                                        SvxConfigEntry* pChildData );

    void            ReloadTopLevelListBox( SvxConfigEntry* pSelection = NULL );

public:

    static bool     CanConfig( const OUString& rModuleId );

    SaveInData*     GetSaveInData() { return pCurrentSaveInData; }

    SvTreeListEntry*    AddFunction( SvTreeListEntry* pTarget = NULL,
                                 bool bFront = false,
                                 bool bAllowDuplicates = false );

    virtual void    MoveEntry( bool bMoveUp );

    bool            MoveEntryData(  SvTreeListEntry* pSourceEntry,
                                    SvTreeListEntry* pTargetEntry );

    bool            FillItemSet( SfxItemSet& ) SAL_OVERRIDE;
    void            Reset( const SfxItemSet& ) SAL_OVERRIDE;

    virtual bool    DeleteSelectedContent() = 0;
    virtual void    DeleteSelectedTopLevel() = 0;

    SvxConfigEntry* GetTopLevelSelection()
    {
        return (SvxConfigEntry*) m_pTopLevelListBox->GetEntryData(
            m_pTopLevelListBox->GetSelectEntryPos() );
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
        GetFrameWithDefaultAndIdentify( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _inout_rxFrame );
};

class SvxMenuConfigPage : public SvxConfigPage
{
private:

    DECL_LINK( SelectMenu, ListBox * );
    DECL_LINK( SelectMenuEntry, Control * );
    DECL_LINK( NewMenuHdl, Button * );
    DECL_LINK( MenuSelectHdl, MenuButton * );
    DECL_LINK( EntrySelectHdl, MenuButton * );
    DECL_LINK( AddCommandsHdl, Button * );
    DECL_LINK( AddFunctionHdl, SvxScriptSelectorDialog * );

    void            Init() SAL_OVERRIDE;
    void            UpdateButtonStates() SAL_OVERRIDE;
    short           QueryReset() SAL_OVERRIDE;
    bool            DeleteSelectedContent() SAL_OVERRIDE;
    void            DeleteSelectedTopLevel() SAL_OVERRIDE;

public:
    SvxMenuConfigPage( Window *pParent, const SfxItemSet& rItemSet );
    ~SvxMenuConfigPage();

    SaveInData* CreateSaveInData(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::ui::XUIConfigurationManager >&,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::ui::XUIConfigurationManager >&,
        const OUString& aModuleId,
        bool docConfig ) SAL_OVERRIDE;
};

class SvxMainMenuOrganizerDialog : public ModalDialog
{
    VclContainer*   m_pMenuBox;
    Edit*           m_pMenuNameEdit;
    SvTreeListBox*  m_pMenuListBox;
    PushButton*     m_pMoveUpButton;
    PushButton*     m_pMoveDownButton;

    SvxEntries*     pEntries;
    SvTreeListEntry*    pNewMenuEntry;
    bool            bModified;

    void UpdateButtonStates();

    DECL_LINK( MoveHdl, Button * );
    DECL_LINK( ModifyHdl, Edit * );
    DECL_LINK( SelectHdl, Control* );

public:
    SvxMainMenuOrganizerDialog (
        Window*, SvxEntries*,
        SvxConfigEntry*, bool bCreateMenu = false );

    ~SvxMainMenuOrganizerDialog ();

    SvxEntries*     GetEntries();
    void            SetEntries( SvxEntries* );
    SvxConfigEntry* GetSelectedEntry();
};

class SvxToolbarEntriesListBox : public SvxMenuEntriesListBox
{
    Size            m_aCheckBoxImageSizePixel;
    Link            m_aChangedListener;
    SvLBoxButtonData*   m_pButtonData;
    SvxConfigPage*  pPage;

    void            ChangeVisibility( SvTreeListEntry* pEntry );

protected:

    virtual void    CheckButtonHdl() SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
    void            BuildCheckBoxButtonImages( SvLBoxButtonData* );
    Image           GetSizedImage(
        VirtualDevice& aDev, const Size& aNewSize, const Image& aImage );

public:

    SvxToolbarEntriesListBox(Window* pParent, SvxToolbarConfigPage* pPg);
    ~SvxToolbarEntriesListBox();

    void            SetChangedListener( const Link& aChangedListener )
        { m_aChangedListener = aChangedListener; }

    const Link&     GetChangedListener() const { return m_aChangedListener; }

    Size            GetCheckBoxPixelSize() const
        { return m_aCheckBoxImageSizePixel; }

    virtual sal_Bool    NotifyMoving(
        SvTreeListEntry*, SvTreeListEntry*, SvTreeListEntry*&, sal_uLong& ) SAL_OVERRIDE;

    virtual sal_Bool    NotifyCopying(
        SvTreeListEntry*, SvTreeListEntry*, SvTreeListEntry*&, sal_uLong&) SAL_OVERRIDE;

    void            KeyInput( const KeyEvent& rKeyEvent ) SAL_OVERRIDE;
};

class SvxToolbarConfigPage : public SvxConfigPage
{
private:

    DECL_LINK( SelectToolbar, ListBox * );
    DECL_LINK( SelectToolbarEntry, Control * );
    DECL_LINK( ToolbarSelectHdl, MenuButton * );
    DECL_LINK( EntrySelectHdl, MenuButton * );
    DECL_LINK( NewToolbarHdl, Button * );
    DECL_LINK( AddCommandsHdl, Button * );
    DECL_LINK( AddFunctionHdl, SvxScriptSelectorDialog * );
    DECL_LINK( MoveHdl, Button * );

    void            UpdateButtonStates() SAL_OVERRIDE;
    short           QueryReset() SAL_OVERRIDE;
    void            Init() SAL_OVERRIDE;
    bool            DeleteSelectedContent() SAL_OVERRIDE;
    void            DeleteSelectedTopLevel() SAL_OVERRIDE;

public:
    SvxToolbarConfigPage( Window *pParent, const SfxItemSet& rItemSet );
    ~SvxToolbarConfigPage();

    SvTreeListEntry*    AddFunction( SvTreeListEntry* pTarget = NULL,
                                             bool bFront = false,
                                             bool bAllowDuplicates = true );

    void            MoveEntry( bool bMoveUp ) SAL_OVERRIDE;

    SaveInData*     CreateSaveInData(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::ui::XUIConfigurationManager >&,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::ui::XUIConfigurationManager >&,
        const OUString& aModuleId,
        bool docConfig ) SAL_OVERRIDE;
};

class ToolbarSaveInData : public SaveInData
{
private:

    SvxConfigEntry*                                pRootEntry;
    OUString                                  m_aDescriptorContainer;

    ::com::sun::star::uno::Reference
        < com::sun::star::container::XNameAccess > m_xPersistentWindowState;

    bool        LoadToolbar(
        const ::com::sun::star::uno::Reference<
            com::sun::star::container::XIndexAccess >& xToolBarSettings,
        SvxConfigEntry* pParentData );

    void        ApplyToolbar(
        com::sun::star::uno::Reference<
            com::sun::star::container::XIndexContainer >& rNewToolbarBar,
        com::sun::star::uno::Reference<
            com::sun::star::lang::XSingleComponentFactory >& rFactory,
        SvxConfigEntry *pToolbar = NULL );

public:

    ToolbarSaveInData(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::ui::XUIConfigurationManager >&,
            const ::com::sun::star::uno::Reference <
            ::com::sun::star::ui::XUIConfigurationManager >&,
        const OUString& aModuleId,
        bool docConfig );

    ~ToolbarSaveInData();

    void            CreateToolbar( SvxConfigEntry* pToolbar );
    void            RestoreToolbar( SvxConfigEntry* pToolbar );
    void            RemoveToolbar( SvxConfigEntry* pToolbar );
    void            ApplyToolbar( SvxConfigEntry* pToolbar );

    OUString   GetSystemUIName( const OUString& rResourceURL );

    sal_Int32       GetSystemStyle( const OUString& rResourceURL );

    void            SetSystemStyle(
        const OUString& rResourceURL, sal_Int32 nStyle );

    void            SetSystemStyle(
        ::com::sun::star::uno::Reference
            < ::com::sun::star::frame::XFrame > xFrame,
        const OUString& rResourceURL, sal_Int32 nStyle );

    SvxEntries*     GetEntries() SAL_OVERRIDE;
    void            SetEntries( SvxEntries* ) SAL_OVERRIDE;
    bool            HasSettings() SAL_OVERRIDE;
    bool            HasURL( const OUString& rURL ) SAL_OVERRIDE;
    void            Reset() SAL_OVERRIDE;
    bool            Apply() SAL_OVERRIDE;
};

class SvxNewToolbarDialog : public ModalDialog
{
private:
    Edit*           m_pEdtName;
    OKButton*       m_pBtnOK;

    Link            aCheckNameHdl;

    DECL_LINK(ModifyHdl, Edit*);

public:
    SvxNewToolbarDialog(Window* pWindow, const OUString& rName);

    ListBox*        m_pSaveInListBox;

    OUString GetName()
    {
        return m_pEdtName->GetText();
    }

    void SetCheckNameHdl( const Link& rLink, bool bCheckImmediately = false )
    {
        aCheckNameHdl = rLink;
        if ( bCheckImmediately )
            m_pBtnOK->Enable( rLink.Call( this ) > 0 );
    }

    void SetEditHelpId( const OString& aHelpId)
    {
        m_pEdtName->SetHelpId(aHelpId);
    }
};

class SvxIconSelectorDialog : public ModalDialog
{
private:
    FixedText       aFtDescription;
    ToolBox         aTbSymbol;
    FixedText       aFtNote;
    OKButton        aBtnOK;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
    PushButton      aBtnImport;
    PushButton      aBtnDelete;
    FixedLine       aFlSeparator;
    sal_uInt16      m_nNextId;

    sal_Int32       m_nExpectedSize;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::ui::XImageManager > m_xImageManager;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::ui::XImageManager > m_xParentImageManager;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::ui::XImageManager > m_xImportedImageManager;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::graphic::XGraphicProvider > m_xGraphProvider;

    bool ReplaceGraphicItem( const OUString& aURL );

    bool ImportGraphic( const OUString& aURL );

    void ImportGraphics(
        const com::sun::star::uno::Sequence< OUString >& aURLs );

public:

    SvxIconSelectorDialog(
        Window *pWindow,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::ui::XImageManager >& rXImageManager,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::ui::XImageManager >& rXParentImageManager
            );

    ~SvxIconSelectorDialog();

    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >
        GetSelectedIcon();

    DECL_LINK( SelectHdl, ToolBox * );
    DECL_LINK( ImportHdl, PushButton * );
    DECL_LINK( DeleteHdl, PushButton * );
};

class SvxIconReplacementDialog : public MessBox
{
public:
    SvxIconReplacementDialog(
        Window *pWindow,
        const OUString& aMessage,
        bool aYestoAll);

    SvxIconReplacementDialog(
        Window *pWindow,
        const OUString& aMessage );

    OUString ReplaceIconName( const OUString& );
    sal_uInt16 ShowDialog();
};
//added for issue83555
class SvxIconChangeDialog : public ModalDialog
{
private:
    FixedImage       aFImageInfo;
    OKButton         aBtnOK;
    FixedText        aDescriptionLabel;
    VclMultiLineEdit aLineEditDescription;
public:
    SvxIconChangeDialog(Window *pWindow, const OUString& aMessage);
};
#endif // INCLUDED_CUI_SOURCE_INC_CFG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

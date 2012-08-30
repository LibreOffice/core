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
#ifndef _SVXCFG_HXX
#define _SVXCFG_HXX

#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/toolbox.hxx>
#include <svtools/svtreebx.hxx>
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

#include <sfx2/minarray.hxx>
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

public:
    SvxConfigDialog( Window*, const SfxItemSet* );
    ~SvxConfigDialog();

    virtual void                PageCreated( sal_uInt16 nId, SfxTabPage &rPage );
    virtual short               Ok();

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
        const rtl::OUString& aModuleId,
        bool docConfig );

    ~SaveInData() {}

    bool PersistChanges(
        const com::sun::star::uno::Reference
            < com::sun::star::uno::XInterface >& xManager );

    void SetModified( bool bValue = sal_True ) { bModified = bValue; }
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
        < com::sun::star::lang::XMultiServiceFactory > m_xServiceManager;

    ::com::sun::star::uno::Reference
        < com::sun::star::container::XNameAccess > m_xCommandToLabelMap;

    com::sun::star::uno::Reference
        < com::sun::star::uno::XComponentContext > m_xComponentContext;

    com::sun::star::uno::Sequence
        < com::sun::star::beans::PropertyValue > m_aSeparatorSeq;

    Image GetImage( const rtl::OUString& rCommandURL );

    virtual bool HasURL( const rtl::OUString& aURL ) = 0;
    virtual bool HasSettings() = 0;
    virtual SvxEntries* GetEntries() = 0;
    virtual void SetEntries( SvxEntries* ) = 0;
    virtual void Reset() = 0;
    virtual bool Apply() = 0;
};

class MenuSaveInData : public SaveInData
{
private:

    rtl::OUString               m_aMenuResourceURL;
    rtl::OUString               m_aDescriptorContainer;

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
        SvLBoxEntry *pParent = NULL );

    void        ApplyMenu(
        com::sun::star::uno::Reference<
            com::sun::star::container::XIndexContainer >& rNewMenuBar,
        com::sun::star::uno::Reference<
            com::sun::star::lang::XSingleComponentFactory >& rFactory,
        SvxConfigEntry *pMenuData = NULL );

    bool        LoadSubMenus(
        const ::com::sun::star::uno::Reference<
            com::sun::star::container::XIndexAccess >& xMenuBarSettings,
        const rtl::OUString& rBaseTitle, SvxConfigEntry* pParentData );

public:

    MenuSaveInData(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::ui::XUIConfigurationManager >&,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::ui::XUIConfigurationManager >&,
        const rtl::OUString& aModuleId,
        bool docConfig );

    ~MenuSaveInData();

    /// methods inherited from SaveInData
    SvxEntries*         GetEntries();
    void                SetEntries( SvxEntries* );
    bool                HasURL( const rtl::OUString& URL ) { (void)URL; return sal_False; }
    bool                HasSettings() { return m_xMenuSettings.is(); }
    void                Reset();
    bool                Apply();
};

class SvxConfigEntry
{
private:

    /// common properties
    sal_uInt16                      nId;
    ::rtl::OUString             aHelpText;
    ::rtl::OUString             aLabel;
    ::rtl::OUString             aCommand;
    ::rtl::OUString             aHelpURL;

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

    SvxConfigEntry( const ::rtl::OUString& rDisplayName,
                    const ::rtl::OUString& rCommandURL,
                    bool bPopup = sal_False,
                    bool bParentData = sal_False );

    SvxConfigEntry()
        :
            nId( 0 ),
            bPopUp( sal_False ),
            bStrEdited( sal_False ),
            bIsUserDefined( sal_False ),
            bIsMain( sal_False ),
            bIsParentData( sal_False ),
            bIsVisible( sal_True ),
            nStyle( 0 ),
            pEntries( 0 )
    {}

    ~SvxConfigEntry();

    const ::rtl::OUString&      GetCommand() const { return aCommand; }
    void    SetCommand( const String& rCmd ) { aCommand = rCmd; }

    const ::rtl::OUString&      GetName() const { return aLabel; }
    void    SetName( const String& rStr ) { aLabel = rStr; bStrEdited = sal_True; }
    bool    HasChangedName() const { return bStrEdited; }

    const ::rtl::OUString&      GetHelpText() ;
    void    SetHelpText( const String& rStr ) { aHelpText = rStr; }

    const ::rtl::OUString&      GetHelpURL() const { return aHelpURL; }
    void    SetHelpURL( const String& rStr ) { aHelpURL = rStr; }

    void    SetPopup( bool bOn = sal_True ) { bPopUp = bOn; }
    bool    IsPopup() const { return bPopUp; }

    void    SetUserDefined( bool bOn = sal_True ) { bIsUserDefined = bOn; }
    bool    IsUserDefined() const { return bIsUserDefined; }

    bool    IsBinding() const { return !bPopUp; }
    bool    IsSeparator() const { return nId == 0; }

    SvxEntries* GetEntries() const { return pEntries; }
    void    SetEntries( SvxEntries* entries ) { pEntries = entries; }
    bool    HasEntries() const { return pEntries != NULL; }

    void    SetMain( bool bValue = sal_True ) { bIsMain = bValue; }
    bool    IsMain() { return bIsMain; }

    void    SetParentData( bool bValue = sal_True ) { bIsParentData = bValue; }
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
    SvxMenuEntriesListBox( Window*, const ResId& );
    ~SvxMenuEntriesListBox();

    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );

    virtual sal_Bool        NotifyAcceptDrop( SvLBoxEntry* pEntry );

    virtual sal_Bool        NotifyMoving( SvLBoxEntry*, SvLBoxEntry*,
                                      SvLBoxEntry*&, sal_uLong& );

    virtual sal_Bool        NotifyCopying( SvLBoxEntry*, SvLBoxEntry*,
                                       SvLBoxEntry*&, sal_uLong&);

    virtual DragDropMode    NotifyStartDrag(
        TransferDataContainer&, SvLBoxEntry* );

    virtual void        DragFinished( sal_Int8 );

    void                KeyInput( const KeyEvent& rKeyEvent );
};

class SvxDescriptionEdit : public ExtMultiLineEdit
{
private:
    Rectangle           m_aRealRect;

public:
    SvxDescriptionEdit( Window* pParent, const ResId& _rId );
    inline ~SvxDescriptionEdit() {}

    void                SetNewText( const String& _rText );
    inline void         Clear() { SetNewText( String() ); }
};

class SvxConfigPage : public SfxTabPage
{
private:

    bool                                bInitialised;
    SaveInData*                         pCurrentSaveInData;

    DECL_LINK(  SelectSaveInLocation, ListBox * );
    DECL_LINK(  AsyncInfoMsg, String* );

    bool        SwapEntryData( SvLBoxEntry* pSourceEntry, SvLBoxEntry* pTargetEntry );
    void        AlignControls();

protected:

    // the top section of the tab page where top level menus and toolbars
    //  are displayed in a listbox
    FixedLine                           aTopLevelSeparator;
    FixedText                           aTopLevelLabel;
    ListBox                             aTopLevelListBox;
    PushButton                          aNewTopLevelButton;
    MenuButton                          aModifyTopLevelButton;

    // the contents section where the contents of the selected
    // menu or toolbar are displayed
    FixedLine                           aContentsSeparator;
    FixedText                           aContentsLabel;
    SvTreeListBox*                      aContentsListBox;

    PushButton                          aAddCommandsButton;
    MenuButton                          aModifyCommandButton;

    ImageButton                         aMoveUpButton;
    ImageButton                         aMoveDownButton;

    FixedText                           aSaveInText;
    ListBox                             aSaveInListBox;

    FixedText                           aDescriptionLabel;
    SvxDescriptionEdit                  aDescriptionField;

    SvxScriptSelectorDialog*            pSelectorDlg;

    /// the ResourceURL to select when opening the dialog
    rtl::OUString                       m_aURLToSelect;

    ::com::sun::star::uno::Reference
        < ::com::sun::star::frame::XFrame > m_xFrame;

    SvxConfigPage( Window*, const SfxItemSet& );
    virtual ~SvxConfigPage();

    DECL_LINK( MoveHdl, Button * );

    virtual SaveInData* CreateSaveInData(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::ui::XUIConfigurationManager >&,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::ui::XUIConfigurationManager >&,
        const rtl::OUString& aModuleId,
        bool docConfig ) = 0;

    virtual void            Init() = 0;
    virtual void            UpdateButtonStates() = 0;
    virtual short           QueryReset() = 0;

    void            PositionContentsListBox();

    SvLBoxEntry*    InsertEntry(        SvxConfigEntry* pNewEntryData,
                                        SvLBoxEntry* pTarget = NULL,
                                        bool bFront = sal_False );

    void            AddSubMenusToUI(    const String& rBaseTitle,
                                        SvxConfigEntry* pParentData );

    SvLBoxEntry*    InsertEntryIntoUI ( SvxConfigEntry* pNewEntryData,
                                        sal_uLong nPos = LIST_APPEND );

    SvxEntries*     FindParentForChild( SvxEntries* pParentEntries,
                                        SvxConfigEntry* pChildData );

    void            ReloadTopLevelListBox( SvxConfigEntry* pSelection = NULL );

public:

    static bool     CanConfig( const ::rtl::OUString& rModuleId );

    SaveInData*     GetSaveInData() { return pCurrentSaveInData; }

    SvLBoxEntry*    AddFunction( SvLBoxEntry* pTarget = NULL,
                                 bool bFront = sal_False,
                                 bool bAllowDuplicates = sal_False );

    virtual void    MoveEntry( bool bMoveUp );

    bool            MoveEntryData(  SvLBoxEntry* pSourceEntry,
                                    SvLBoxEntry* pTargetEntry );

    sal_Bool            FillItemSet( SfxItemSet& );
    void            Reset( const SfxItemSet& );

    virtual bool    DeleteSelectedContent() = 0;
    virtual void    DeleteSelectedTopLevel() = 0;

    SvxConfigEntry* GetTopLevelSelection()
    {
        return (SvxConfigEntry*) aTopLevelListBox.GetEntryData(
            aTopLevelListBox.GetSelectEntryPos() );
    }

    /** identifies the module in the given frame. If the frame is <NULL/>, a default
        frame will be determined beforehand.

        If the given frame is <NULL/>, a default frame will be used: The method the active
        frame of the desktop, then the current frame. If both are <NULL/>,
        the SfxViewFrame::Current's XFrame is used. If this is <NULL/>, too, an empty string is returned.

        If the given frame is not <NULL/>, or an default frame could be successfully determined, then
        the ModuleManager is asked for the module ID of the component in the frame.
    */
    static ::rtl::OUString
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

    void            Init();
    void            UpdateButtonStates();
    short           QueryReset();
    bool            DeleteSelectedContent();
    void            DeleteSelectedTopLevel();

public:
    SvxMenuConfigPage( Window *pParent, const SfxItemSet& rItemSet );
    ~SvxMenuConfigPage();

    SaveInData* CreateSaveInData(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::ui::XUIConfigurationManager >&,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::ui::XUIConfigurationManager >&,
        const rtl::OUString& aModuleId,
        bool docConfig );
};

class SvxMainMenuOrganizerDialog : public ModalDialog
{
    FixedText       aMenuNameText;
    Edit            aMenuNameEdit;
    FixedText       aMenuListText;
    SvTreeListBox   aMenuListBox;
    ImageButton     aMoveUpButton;
    ImageButton     aMoveDownButton;
    OKButton        aOKButton;
    CancelButton    aCloseButton;
    HelpButton      aHelpButton;

    SvxEntries*     pEntries;
    SvLBoxEntry*    pNewMenuEntry;
    bool            bModified;

    void UpdateButtonStates();

    DECL_LINK( MoveHdl, Button * );
    DECL_LINK( ModifyHdl, Edit * );
    DECL_LINK( SelectHdl, Control* );

public:
    SvxMainMenuOrganizerDialog (
        Window*, SvxEntries*,
        SvxConfigEntry*, bool bCreateMenu = sal_False );

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

    void            ChangeVisibility( SvLBoxEntry* pEntry );

protected:

    virtual void    CheckButtonHdl();
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    void            BuildCheckBoxButtonImages( SvLBoxButtonData* );
    Image           GetSizedImage(
        VirtualDevice& aDev, const Size& aNewSize, const Image& aImage );

public:

    SvxToolbarEntriesListBox(
        Window* pParent, const ResId& );

    ~SvxToolbarEntriesListBox();

    void            SetChangedListener( const Link& aChangedListener )
        { m_aChangedListener = aChangedListener; }

    const Link&     GetChangedListener() const { return m_aChangedListener; }

    Size            GetCheckBoxPixelSize() const
        { return m_aCheckBoxImageSizePixel; }

    virtual sal_Bool    NotifyMoving(
        SvLBoxEntry*, SvLBoxEntry*, SvLBoxEntry*&, sal_uLong& );

    virtual sal_Bool    NotifyCopying(
        SvLBoxEntry*, SvLBoxEntry*, SvLBoxEntry*&, sal_uLong&);

    void            KeyInput( const KeyEvent& rKeyEvent );
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

    void            UpdateButtonStates();
    short           QueryReset();
    void            Init();
    bool            DeleteSelectedContent();
    void            DeleteSelectedTopLevel();

public:
    SvxToolbarConfigPage( Window *pParent, const SfxItemSet& rItemSet );
    ~SvxToolbarConfigPage();

    SvLBoxEntry*    AddFunction( SvLBoxEntry* pTarget = NULL,
                                             bool bFront = sal_False,
                                             bool bAllowDuplicates = sal_True );

    void            MoveEntry( bool bMoveUp );

    SaveInData*     CreateSaveInData(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::ui::XUIConfigurationManager >&,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::ui::XUIConfigurationManager >&,
        const rtl::OUString& aModuleId,
        bool docConfig );
};

class ToolbarSaveInData : public SaveInData
{
private:

    SvxConfigEntry*                                pRootEntry;
    rtl::OUString                                  m_aDescriptorContainer;

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
        const rtl::OUString& aModuleId,
        bool docConfig );

    ~ToolbarSaveInData();

    void            CreateToolbar( SvxConfigEntry* pToolbar );
    void            RestoreToolbar( SvxConfigEntry* pToolbar );
    void            RemoveToolbar( SvxConfigEntry* pToolbar );
    void            ApplyToolbar( SvxConfigEntry* pToolbar );

    rtl::OUString   GetSystemUIName( const rtl::OUString& rResourceURL );

    sal_Int32       GetSystemStyle( const rtl::OUString& rResourceURL );

    void            SetSystemStyle(
        const rtl::OUString& rResourceURL, sal_Int32 nStyle );

    void            SetSystemStyle(
        ::com::sun::star::uno::Reference
            < ::com::sun::star::frame::XFrame > xFrame,
        const rtl::OUString& rResourceURL, sal_Int32 nStyle );

    SvxEntries*     GetEntries();
    void            SetEntries( SvxEntries* );
    bool            HasSettings();
    bool            HasURL( const rtl::OUString& rURL );
    void            Reset();
    bool            Apply();
};

class SvxNewToolbarDialog : public ModalDialog
{
private:
    FixedText       aFtDescription;
    Edit            aEdtName;
    FixedText       aSaveInText;
    OKButton        aBtnOK;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    Link            aCheckNameHdl;

    DECL_LINK(ModifyHdl, Edit*);

public:
    SvxNewToolbarDialog( Window* pWindow, const String& rName );

    ListBox         aSaveInListBox;

    void    GetName( rtl::OUString& rName ){rName = aEdtName.GetText();}

    void    SetCheckNameHdl( const Link& rLink, bool bCheckImmediately = false )
    {
        aCheckNameHdl = rLink;
        if ( bCheckImmediately )
            aBtnOK.Enable( rLink.Call( this ) > 0 );
    }

    void    SetEditHelpId( const rtl::OString& aHelpId) {aEdtName.SetHelpId(aHelpId);}
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

    bool ReplaceGraphicItem( const ::rtl::OUString& aURL );

    bool ImportGraphic( const ::rtl::OUString& aURL );

    void ImportGraphics(
        const com::sun::star::uno::Sequence< rtl::OUString >& aURLs );

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
        const rtl::OUString& aMessage,
        bool aYestoAll);

    SvxIconReplacementDialog(
        Window *pWindow,
        const rtl::OUString& aMessage );

    rtl::OUString ReplaceIconName( const rtl::OUString& );
    sal_uInt16 ShowDialog();
};
//added for issue83555
class SvxIconChangeDialog : public ModalDialog
{
private:
    FixedImage      aFImageInfo;
    OKButton        aBtnOK;
    FixedText         aDescriptionLabel;
    SvxDescriptionEdit aLineEditDescription;
public:
    SvxIconChangeDialog(Window *pWindow, const rtl::OUString& aMessage);
};
#endif // _SVXCFG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

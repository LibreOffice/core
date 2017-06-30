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
#include <svtools/imgdef.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/treelistbox.hxx>
#include <svtools/svmedit2.hxx>
#include <svtools/svmedit.hxx>

#include <comphelper/documentinfo.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/random.hxx>
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
#include <vcl/msgbox.hxx>

#include "cfgutil.hxx"

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

class SvxConfigDialog : public SfxTabDialog
{
private:
    css::uno::Reference< css::frame::XFrame > m_xFrame;
    sal_uInt16 m_nMenusPageId;
    sal_uInt16 m_nContextMenusPageId;
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

protected:

    void ApplyMenu(
        css::uno::Reference< css::container::XIndexContainer >& rMenuBar,
        css::uno::Reference< css::lang::XSingleComponentFactory >& rFactory,
        SvxConfigEntry *pMenuData );

    bool LoadSubMenus(
        const css::uno::Reference< css::container::XIndexAccess >& xMenuSettings,
        const OUString& rBaseTitle, SvxConfigEntry* pParentData, bool bContextMenu );

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

    std::unique_ptr<SvxConfigEntry>      pRootEntry;


    static MenuSaveInData* pDefaultData;    ///< static holder of the default menu data

    static void SetDefaultData( MenuSaveInData* pData ) {pDefaultData = pData;}
    static MenuSaveInData* GetDefaultData() { return pDefaultData; }

    void        Apply(
        css::uno::Reference< css::container::XIndexContainer >& rNewMenuBar,
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
    void                SetEntries( SvxEntries* ) override;
    bool                HasURL( const OUString& URL ) override { (void)URL; return false; }
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
    void SetEntries( SvxEntries* pNewEntries ) override;
    bool HasSettings() override;
    bool HasURL( const OUString& rURL ) override;
    void Reset() override;
    bool Apply() override;
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
    bool                        bIsModified;

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
            bIsModified( false ),
            bIsVisible( true ),
            nStyle( 0 ),
            mpEntries( nullptr )
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
    virtual ~SvxMenuEntriesListBox() override;
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

    DECL_LINK(  SelectSaveInLocation, ListBox&, void );
    DECL_LINK( AsyncInfoMsg, void*, void );

protected:

    // the top section of the tab page where top level menus and toolbars
    //  are displayed in a listbox
    VclPtr<VclFrame>                           m_pTopLevel;
    VclPtr<FixedText>                          m_pTopLevelLabel;
    VclPtr<ListBox>                            m_pTopLevelListBox;
    VclPtr<PushButton>                         m_pNewTopLevelButton;
    VclPtr<MenuButton>                         m_pModifyTopLevelButton;

    VclPtr<FixedText>                          m_pToolbarStyleLabel;
    VclPtr<RadioButton>                        m_pIconsOnlyRB;
    VclPtr<RadioButton>                        m_pTextOnlyRB;
    VclPtr<RadioButton>                        m_pIconsAndTextRB;

    // the contents section where the contents of the selected
    // menu or toolbar are displayed
    VclPtr<VclFrame>                           m_pContents;
    VclPtr<FixedText>                          m_pContentsLabel;
    VclPtr<VclContainer>                       m_pEntries;
    VclPtr<SvTreeListBox>                      m_pContentsListBox;

    VclPtr<PushButton>                         m_pAddCommandsButton;
    VclPtr<PushButton>                         m_pAddSeparatorButton;
    VclPtr<PushButton>                         m_pAddSubmenuButton;
    VclPtr<MenuButton>                         m_pModifyCommandButton;
    VclPtr<PushButton>                         m_pDeleteCommandButton;
    // Resets the top level toolbar to default settings
    VclPtr<PushButton>                         m_pResetTopLevelButton;

    VclPtr<PushButton>                         m_pMoveUpButton;
    VclPtr<PushButton>                         m_pMoveDownButton;

    VclPtr<ListBox>                            m_pSaveInListBox;

    VclPtr<VclMultiLineEdit>                   m_pDescriptionField;

    VclPtr<SvxScriptSelectorDialog>            m_pSelectorDlg;

    /// the ResourceURL to select when opening the dialog
    OUString                                   m_aURLToSelect;

    css::uno::Reference< css::frame::XFrame >  m_xFrame;

    SvxConfigPage( vcl::Window*, const SfxItemSet& );

    DECL_LINK( MoveHdl, Button *, void );

    virtual SaveInData* CreateSaveInData(
        const css::uno::Reference< css::ui::XUIConfigurationManager >&,
        const css::uno::Reference< css::ui::XUIConfigurationManager >&,
        const OUString& aModuleId,
        bool docConfig ) = 0;

    virtual void            Init() = 0;
    virtual void            UpdateButtonStates() = 0;
    virtual short           QueryReset() = 0;

    SvTreeListEntry*    InsertEntry(        SvxConfigEntry* pNewEntryData,
                                        SvTreeListEntry* pTarget = nullptr,
                                        bool bFront = false );

    void                AddSubMenusToUI(    const OUString& rBaseTitle,
                                        SvxConfigEntry* pParentData );

    SvTreeListEntry*    InsertEntryIntoUI ( SvxConfigEntry* pNewEntryData,
                                        sal_uLong nPos = TREELIST_APPEND );

    SvxEntries*     FindParentForChild( SvxEntries* pParentEntries,
                                        SvxConfigEntry* pChildData );

    void            ReloadTopLevelListBox( SvxConfigEntry* pSelection = nullptr );

public:

    virtual ~SvxConfigPage() override;
    virtual void dispose() override;

    static bool     CanConfig( const OUString& rModuleId );

    SaveInData*     GetSaveInData() { return pCurrentSaveInData; }

    SvTreeListEntry*    AddFunction( SvTreeListEntry* pTarget = nullptr,
                                 bool bFront = false,
                                 bool bAllowDuplicates = false );

    virtual void    MoveEntry( bool bMoveUp );

    bool            MoveEntryData(  SvTreeListEntry* pSourceEntry,
                                    SvTreeListEntry* pTargetEntry );

    bool            FillItemSet( SfxItemSet* ) override;
    void            Reset( const SfxItemSet* ) override;

    virtual void    DeleteSelectedContent() = 0;
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

    DECL_LINK( MoveHdl, Button *, void );
    DECL_LINK( ModifyHdl, Edit&, void );
    DECL_LINK( SelectHdl, SvTreeListBox*, void );

public:
    SvxMainMenuOrganizerDialog (
        vcl::Window*, SvxEntries*,
        SvxConfigEntry*, bool bCreateMenu = false );
    virtual ~SvxMainMenuOrganizerDialog() override;
    virtual void dispose() override;

    SvxEntries*     GetEntries() { return mpEntries;}
    SvxConfigEntry* GetSelectedEntry();
};

//SvxToolbarEntriesListBox class was here

//SvxToolbarConfigPage class was here

class ToolbarSaveInData : public SaveInData
{
private:

    std::unique_ptr<SvxConfigEntry>           pRootEntry;
    OUString                                  m_aDescriptorContainer;

    css::uno::Reference
        < css::container::XNameAccess > m_xPersistentWindowState;

    void        LoadToolbar(
        const css::uno::Reference< css::container::XIndexAccess >& xToolBarSettings,
        SvxConfigEntry* pParentData );

    void        ApplyToolbar(
        css::uno::Reference< css::container::XIndexContainer >& rNewToolbarBar,
        css::uno::Reference< css::lang::XSingleComponentFactory >& rFactory,
        SvxConfigEntry *pToolbar );

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
    virtual ~SvxNewToolbarDialog() override;
    virtual void dispose() override;

    VclPtr<ListBox>        m_pSaveInListBox;

    OUString GetName()
    {
        return m_pEdtName->GetText();
    }
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

    virtual ~SvxIconSelectorDialog() override;
    virtual void dispose() override;

    css::uno::Reference< css::graphic::XGraphic >
        GetSelectedIcon();

    DECL_LINK( SelectHdl, ToolBox *, void );
    DECL_LINK( ImportHdl, Button *, void );
    DECL_LINK( DeleteHdl, Button *, void );
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
    virtual ~SvxIconChangeDialog() override;
    virtual void dispose() override;
};

namespace killmelater
{
//TODO:This is copy/pasted from cfg.cxx
inline void RemoveEntry( SvxEntries* pEntries, SvxConfigEntry* pChildEntry )
{
    SvxEntries::iterator iter = pEntries->begin();

    while ( iter != pEntries->end() )
    {
        if ( pChildEntry == *iter )
        {
            pEntries->erase( iter );
            break;
        }
        ++iter;
    }
}

//TODO:This is copy/pasted from cfg.cxx
inline OUString replaceSaveInName(
    const OUString& rMessage,
    const OUString& rSaveInName )
{
    OUString name;
    OUString placeholder("%SAVE IN SELECTION%" );

    sal_Int32 pos = rMessage.indexOf( placeholder );

    if ( pos != -1 )
    {
        name = rMessage.replaceAt(
            pos, placeholder.getLength(), rSaveInName );
    }

    return name;
}

//TODO:This is copy/pasted from cfg.cxx
inline OUString
stripHotKey( const OUString& str )
{
    sal_Int32 index = str.indexOf( '~' );
    if ( index == -1 )
    {
        return str;
    }
    else
    {
        return str.replaceAt( index, 1, OUString() );
    }
}

inline OUString
replaceSixteen( const OUString& str, sal_Int32 nReplacement )
{
    OUString result( str );
    OUString sixteen = OUString::number( 16 );
    OUString expected = OUString::number( nReplacement );

    sal_Int32 len = sixteen.getLength();
    sal_Int32 index = result.indexOf( sixteen );

    while ( index != -1 )
    {
        result = result.replaceAt( index, len, expected );
        index = result.indexOf( sixteen, index );
    }

    return result;
}

static sal_Int16 theImageType =
    css::ui::ImageType::COLOR_NORMAL |
    css::ui::ImageType::SIZE_DEFAULT;

//TODO:This is copy/pasted from cfg.cxx
inline sal_Int16 GetImageType()
{
    return theImageType;
}

inline void InitImageType()
{
    theImageType =
        css::ui::ImageType::COLOR_NORMAL |
        css::ui::ImageType::SIZE_DEFAULT;

    if (SvtMiscOptions().GetSymbolsSize() == SFX_SYMBOLS_SIZE_LARGE)
    {
        theImageType |= css::ui::ImageType::SIZE_LARGE;
    }
    else if (SvtMiscOptions().GetSymbolsSize() == SFX_SYMBOLS_SIZE_32)
    {
        theImageType |= css::ui::ImageType::SIZE_32;
    }
}

//TODO:This is copy/pasted from cfg.cxx
inline css::uno::Reference< css::graphic::XGraphic > GetGraphic(
    const css::uno::Reference< css::ui::XImageManager >& xImageManager,
    const OUString& rCommandURL )
{
    css::uno::Reference< css::graphic::XGraphic > result;

    if ( xImageManager.is() )
    {
        // TODO handle large graphics
        css::uno::Sequence< css::uno::Reference< css::graphic::XGraphic > > aGraphicSeq;

        css::uno::Sequence<OUString> aImageCmdSeq { rCommandURL };

        try
        {
            aGraphicSeq =
                xImageManager->getImages( killmelater::GetImageType(), aImageCmdSeq );

            if ( aGraphicSeq.getLength() > 0 )
            {
                result =  aGraphicSeq[0];
            }
        }
        catch ( css::uno::Exception& )
        {
            // will return empty XGraphic
        }
    }

    return result;
}

//TODO:This is copy/pasted from cfg.cxx
inline OUString
generateCustomName(
    const OUString& prefix,
    SvxEntries* entries,
    sal_Int32 suffix = 1 )
{
    // find and replace the %n placeholder in the prefix string
    OUString name;
    OUString placeholder("%n" );

    sal_Int32 pos = prefix.indexOf( placeholder );

    if ( pos != -1 )
    {
        name = prefix.replaceAt(
            pos, placeholder.getLength(), OUString::number( suffix ) );
    }
    else
    {
        // no placeholder found so just append the suffix
        name = prefix + OUString::number( suffix );
    }

    if (!entries)
        return name;

    // now check is there is an already existing entry with this name
    SvxEntries::const_iterator iter = entries->begin();

    while ( iter != entries->end() )
    {
        SvxConfigEntry* pEntry = *iter;

        if ( name.equals( pEntry->GetName() ) )
        {
            break;
        }
        ++iter;
    }

    if ( iter != entries->end() )
    {
        // name already exists so try the next number up
        return generateCustomName( prefix, entries, ++suffix );
    }

    return name;
}

inline OUString
generateCustomMenuURL(
    SvxEntries* entries,
    sal_Int32 suffix = 1 )
{
    OUString url = "vnd.openoffice.org:CustomMenu" + OUString::number( suffix );
    if (!entries)
        return url;

    // now check is there is an already existing entry with this url
    SvxEntries::const_iterator iter = entries->begin();

    while ( iter != entries->end() )
    {
        SvxConfigEntry* pEntry = *iter;

        if ( url.equals( pEntry->GetCommand() ) )
        {
            break;
        }
        ++iter;
    }

    if ( iter != entries->end() )
    {
        // url already exists so try the next number up
        return generateCustomMenuURL( entries, ++suffix );
    }

    return url;
}

inline sal_uInt32 generateRandomValue()
{
    return comphelper::rng::uniform_uint_distribution(0, std::numeric_limits<unsigned int>::max());
}

inline OUString
generateCustomURL(
    SvxEntries* entries )
{
    OUString url = ITEM_TOOLBAR_URL;
    url += CUSTOM_TOOLBAR_STR;

    // use a random number to minimize possible clash with existing custom toolbars
    url += OUString::number( generateRandomValue(), 16 );

    // now check is there is an already existing entry with this url
    SvxEntries::const_iterator iter = entries->begin();

    while ( iter != entries->end() )
    {
        SvxConfigEntry* pEntry = *iter;

        if ( url.equals( pEntry->GetCommand() ) )
        {
            break;
        }
        ++iter;
    }

    if ( iter != entries->end() )
    {
        // url already exists so try the next number up
        return generateCustomURL( entries );
    }

    return url;
}

inline OUString GetModuleName( const OUString& aModuleId )
{
    if ( aModuleId == "com.sun.star.text.TextDocument" ||
         aModuleId == "com.sun.star.text.GlobalDocument" )
        return OUString("Writer");
    else if ( aModuleId == "com.sun.star.text.WebDocument" )
        return OUString("Writer/Web");
    else if ( aModuleId == "com.sun.star.drawing.DrawingDocument" )
        return OUString("Draw");
    else if ( aModuleId == "com.sun.star.presentation.PresentationDocument" )
        return OUString("Impress");
    else if ( aModuleId == "com.sun.star.sheet.SpreadsheetDocument" )
        return OUString("Calc");
    else if ( aModuleId == "com.sun.star.script.BasicIDE" )
        return OUString("Basic");
    else if ( aModuleId == "com.sun.star.formula.FormulaProperties" )
        return OUString("Math");
    else if ( aModuleId == "com.sun.star.sdb.RelationDesign" )
        return OUString("Relation Design");
    else if ( aModuleId == "com.sun.star.sdb.QueryDesign" )
        return OUString("Query Design");
    else if ( aModuleId == "com.sun.star.sdb.TableDesign" )
        return OUString("Table Design");
    else if ( aModuleId == "com.sun.star.sdb.DataSourceBrowser" )
        return OUString("Data Source Browser" );
    else if ( aModuleId == "com.sun.star.sdb.DatabaseDocument" )
        return OUString("Database" );

    return OUString();
}

inline OUString GetUIModuleName( const OUString& aModuleId, const css::uno::Reference< css::frame::XModuleManager2 >& rModuleManager )
{
    assert(rModuleManager.is());

    OUString aModuleUIName;

    try
    {
        css::uno::Any a = rModuleManager->getByName( aModuleId );
        css::uno::Sequence< css::beans::PropertyValue > aSeq;

        if ( a >>= aSeq )
        {
            for ( sal_Int32 i = 0; i < aSeq.getLength(); ++i )
            {
                if ( aSeq[i].Name == "ooSetupFactoryUIName" )
                {
                    aSeq[i].Value >>= aModuleUIName;
                    break;
                }
            }
        }
    }
    catch ( css::uno::RuntimeException& )
    {
        throw;
    }
    catch ( css::uno::Exception& )
    {
    }

    if ( aModuleUIName.isEmpty() )
        aModuleUIName = GetModuleName( aModuleId );

    return aModuleUIName;
}

inline bool GetMenuItemData(
    const css::uno::Reference< css::container::XIndexAccess >& rItemContainer,
    sal_Int32 nIndex,
    OUString& rCommandURL,
    OUString& rLabel,
    sal_uInt16& rType,
    css::uno::Reference< css::container::XIndexAccess >& rSubMenu )
{
    try
    {
        css::uno::Sequence< css::beans::PropertyValue > aProp;
        if ( rItemContainer->getByIndex( nIndex ) >>= aProp )
        {
            for ( sal_Int32 i = 0; i < aProp.getLength(); ++i )
            {
                if ( aProp[i].Name == ITEM_DESCRIPTOR_COMMANDURL )
                {
                    aProp[i].Value >>= rCommandURL;
                }
                else if ( aProp[i].Name == ITEM_DESCRIPTOR_CONTAINER )
                {
                    aProp[i].Value >>= rSubMenu;
                }
                else if ( aProp[i].Name == ITEM_DESCRIPTOR_LABEL )
                {
                    aProp[i].Value >>= rLabel;
                }
                else if ( aProp[i].Name == ITEM_DESCRIPTOR_TYPE )
                {
                    aProp[i].Value >>= rType;
                }
            }

            return true;
        }
    }
    catch ( css::lang::IndexOutOfBoundsException& )
    {
    }

    return false;
}

inline bool GetToolbarItemData(
    const css::uno::Reference< css::container::XIndexAccess >& rItemContainer,
    sal_Int32 nIndex,
    OUString& rCommandURL,
    OUString& rLabel,
    sal_uInt16& rType,
    bool& rIsVisible,
    sal_Int32& rStyle )
{
    try
    {
        css::uno::Sequence< css::beans::PropertyValue > aProp;
        if ( rItemContainer->getByIndex( nIndex ) >>= aProp )
        {
            for ( sal_Int32 i = 0; i < aProp.getLength(); ++i )
            {
                if ( aProp[i].Name == ITEM_DESCRIPTOR_COMMANDURL )
                {
                    aProp[i].Value >>= rCommandURL;
                }
                else if ( aProp[i].Name == ITEM_DESCRIPTOR_STYLE )
                {
                    aProp[i].Value >>= rStyle;
                }
                else if ( aProp[i].Name == ITEM_DESCRIPTOR_LABEL )
                {
                    aProp[i].Value >>= rLabel;
                }
                else if ( aProp[i].Name == ITEM_DESCRIPTOR_TYPE )
                {
                    aProp[i].Value >>= rType;
                }
                else if ( aProp[i].Name == ITEM_DESCRIPTOR_ISVISIBLE )
                {
                    aProp[i].Value >>= rIsVisible;
                }
            }

            return true;
        }
    }
    catch ( css::lang::IndexOutOfBoundsException& )
    {
    }

    return false;
}

inline css::uno::Sequence< css::beans::PropertyValue >
ConvertSvxConfigEntry( const SvxConfigEntry* pEntry )
{
    css::uno::Sequence< css::beans::PropertyValue > aPropSeq( 3 );

    aPropSeq[0].Name = ITEM_DESCRIPTOR_COMMANDURL;
    aPropSeq[0].Value <<= pEntry->GetCommand();

    aPropSeq[1].Name = ITEM_DESCRIPTOR_TYPE;
    aPropSeq[1].Value <<= css::ui::ItemType::DEFAULT;

    // If the name has not been changed, then the label can be stored
    // as an empty string.
    // It will be initialised again later using the command to label map.
    aPropSeq[2].Name = ITEM_DESCRIPTOR_LABEL;
    if ( !pEntry->HasChangedName() && !pEntry->GetCommand().isEmpty() )
    {
        aPropSeq[2].Value <<= OUString();
    }
    else
    {
        aPropSeq[2].Value <<= pEntry->GetName();
    }

    return aPropSeq;
}

inline css::uno::Sequence< css::beans::PropertyValue >
ConvertToolbarEntry( const SvxConfigEntry* pEntry )
{
    css::uno::Sequence< css::beans::PropertyValue > aPropSeq( 4 );

    aPropSeq[0].Name = ITEM_DESCRIPTOR_COMMANDURL;
    aPropSeq[0].Value <<= pEntry->GetCommand();

    aPropSeq[1].Name = ITEM_DESCRIPTOR_TYPE;
    aPropSeq[1].Value <<= css::ui::ItemType::DEFAULT;

    // If the name has not been changed, then the label can be stored
    // as an empty string.
    // It will be initialised again later using the command to label map.
    aPropSeq[2].Name = ITEM_DESCRIPTOR_LABEL;
    if ( !pEntry->HasChangedName() && !pEntry->GetCommand().isEmpty() )
    {
        aPropSeq[2].Value <<= OUString();
    }
    else
    {
        aPropSeq[2].Value <<= pEntry->GetName();
    }

    aPropSeq[3].Name = ITEM_DESCRIPTOR_ISVISIBLE;
    aPropSeq[3].Value <<= pEntry->IsVisible();

    return aPropSeq;
}

//Was in anonymous namespace in cfg.cxx
inline bool showKeyConfigTabPage( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    if (!xFrame.is())
    {
        return false;
    }
    OUString sModuleId(
        css::frame::ModuleManager::create(
            comphelper::getProcessComponentContext())
        ->identify(xFrame));
    return !sModuleId.isEmpty()
        && sModuleId != "com.sun.star.frame.StartModule";
}

inline bool EntrySort( SvxConfigEntry* a, SvxConfigEntry* b )
{
    return a->GetName().compareTo( b->GetName() ) < 0;
}

inline bool SvxConfigEntryModified( SvxConfigEntry* pEntry )
{
    SvxEntries* pEntries = pEntry->GetEntries();
    if ( !pEntries )
        return false;

    for ( const auto& entry : *pEntries )
    {
        if ( entry->IsModified() || SvxConfigEntryModified( entry ) )
            return true;
    }
    return false;
}

}

#endif // INCLUDED_CUI_SOURCE_INC_CFG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/*************************************************************************
 *
 *  $RCSfile: cfg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 17:04:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SVXCFG_HXX
#define _SVXCFG_HXX

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _SV_MENUBTN_HXX //autogen
#include <vcl/menubtn.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif

#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <drafts/com/sun/star/frame/XModuleManager.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XUICONFIGURATIONLISTENER_HPP_
#include <drafts/com/sun/star/ui/XUIConfigurationListener.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XUICONFIGURATIONMANAGER_HPP_
#include <drafts/com/sun/star/ui/XUIConfigurationManager.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XIMAGEMANAGER_HPP_
#include <drafts/com/sun/star/ui/XImageManager.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLECOMPONENTFACTORY_HPP_
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#endif

#define _SVSTDARR_USHORTS
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>     // SvUShorts
#include <sfx2/minarray.hxx>
#include <sfx2/tabdlg.hxx>
#include <vector>

#include "selector.hxx"

class SvxConfigEntry;
class SvxConfigPage;
class SvxMenuConfigPage;
class SvxToolbarConfigPage;

typedef std::vector< SvxConfigEntry* > SvxEntries;

class SvxConfigDialog : public SfxTabDialog
{
public:
    SvxConfigDialog( Window*, const SfxItemSet*, SfxViewFrame* pFrame );
    ~SvxConfigDialog();

    void                        ActivateTabPage( USHORT );

    virtual void                PageCreated( USHORT nId, SfxTabPage &rPage );
    virtual short               Ok();
};

class SaveInData : public ImageProvider
{
private:

    bool        bModified;

    bool        bDocConfig;
    bool        bReadOnly;

    ::com::sun::star::uno::Reference
        < drafts::com::sun::star::ui::XUIConfigurationManager > m_xCfgMgr;

    ::com::sun::star::uno::Reference
        < drafts::com::sun::star::ui::XImageManager > m_xImgMgr;

    static ::com::sun::star::uno::Reference
        < drafts::com::sun::star::ui::XImageManager >* xDefaultImgMgr;

public:

    SaveInData(
        const ::com::sun::star::uno::Reference <
            drafts::com::sun::star::ui::XUIConfigurationManager >& xCfgMgr,
        const rtl::OUString& aModuleId,
        bool docConfig );

    ~SaveInData() {}

    bool PersistChanges(
        const com::sun::star::uno::Reference
            < com::sun::star::uno::XInterface >& xManager );

    void SetModified( bool bValue = TRUE ) { bModified = bValue; }
    bool IsModified( ) { return bModified; }

    bool IsReadOnly( ) { return bReadOnly; }
    bool IsDocConfig( ) { return bDocConfig; }

    ::com::sun::star::uno::Reference
        < drafts::com::sun::star::ui::XUIConfigurationManager >
            GetConfigManager() { return m_xCfgMgr; };

    ::com::sun::star::uno::Reference
        < drafts::com::sun::star::ui::XImageManager >
            GetImageManager() { return m_xImgMgr; };

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

    // static holder of the default menu data
    static MenuSaveInData* pDefaultData;

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
            drafts::com::sun::star::ui::XUIConfigurationManager >&,
        const rtl::OUString& aModuleId,
        bool docConfig );

    ~MenuSaveInData();

    // methods inherited from SaveInData
    SvxEntries*         GetEntries();
    void                SetEntries( SvxEntries* );
    bool                HasURL( const rtl::OUString& URL ) { return FALSE; }
    bool                HasSettings() { return m_xMenuSettings.is(); }
    void                Reset();
    bool                Apply();
};

class SvxConfigEntry
{
private:

    // common properties
    USHORT                      nId;
    ::rtl::OUString             aHelpText;
    ::rtl::OUString             aLabel;
    ::rtl::OUString             aCommand;
    ::rtl::OUString             aHelpURL;

    bool                        bPopUp;
    bool                        bStrEdited;
    bool                        bIsUserDefined;
    bool                        bIsMain;
    bool                        bIsDeletable;
    bool                        bIsMovable;

    // toolbar specific properties
    bool                        bIsVisible;
    sal_Int32                   nStyle;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::graphic::XGraphic > xBackupGraphic;

    SvxEntries                  *pEntries;

public:

    SvxConfigEntry(
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue >& rProperties,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XNameAccess >& rCommandToLabelMap );

    SvxConfigEntry( USHORT nInitId, const String& rInitStr,
                    const String& rHelpText, bool bPopup = FALSE );

    SvxConfigEntry()
        :   nId( 0 ),
            bPopUp( FALSE ),
            bIsUserDefined( FALSE ),
            bIsMain( FALSE ),
            bStrEdited( FALSE ),
            bIsVisible( TRUE ),
            nStyle( 0 ),
            pEntries( 0 )
    {}

    ~SvxConfigEntry();

    USHORT  GetId() const { return nId; }
    void    SetId( USHORT nNew );

    const ::rtl::OUString&      GetCommand() const { return aCommand; }
    void    SetCommand( const String& rCmd ) { aCommand = rCmd; }

    const ::rtl::OUString&      GetName() const { return aLabel; }
    void    SetName( const String& rStr ) { aLabel = rStr; bStrEdited = TRUE; }
    bool    HasChangedName() const { return bStrEdited; }

    const ::rtl::OUString&      GetHelpText() const { return aHelpText; }
    void    SetHelpText( const String& rStr ) { aHelpText = rStr; }

    const ::rtl::OUString&      GetHelpURL() const { return aHelpURL; }
    void    SetHelpURL( const String& rStr ) { aHelpURL = rStr; }

    void    SetPopup( bool bOn = TRUE ) { bPopUp = bOn; }
    bool    IsPopup() const { return bPopUp; }

    void    SetUserDefined( bool bOn = TRUE ) { bIsUserDefined = bOn; }
    bool    IsUserDefined() const { return bIsUserDefined; }

    bool    IsBinding() const { return nId != 0 && !bPopUp; }
    bool    IsSeparator() const { return nId == 0; }

    SvxEntries* GetEntries() const { return pEntries; }
    void    SetEntries( SvxEntries* entries ) { pEntries = entries; }
    bool    HasEntries() const { return pEntries != NULL; }

    void    SetMain( bool bValue = TRUE ) { bIsMain = bValue; }
    bool    IsMain() { return bIsMain; }

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

    virtual
        com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >
        GetProperties(
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XNameAccess >& rCommandToLabelMap );
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

    virtual BOOL        NotifyAcceptDrop( SvLBoxEntry* pEntry );

    virtual BOOL        NotifyMoving( SvLBoxEntry*, SvLBoxEntry*,
                                      SvLBoxEntry*&, ULONG& );

    virtual BOOL        NotifyCopying( SvLBoxEntry*, SvLBoxEntry*,
                                       SvLBoxEntry*&, ULONG&);

    virtual DragDropMode    NotifyStartDrag(
        TransferDataContainer&, SvLBoxEntry* );

    virtual void        DragFinished( sal_Int8 );

    void                KeyInput( const KeyEvent& rKeyEvent );
};

class SvxConfigPage : public SfxTabPage
{
private:

    bool                                bInitialised;
    SaveInData*                         pCurrentSaveInData;

    DECL_LINK( SelectSaveInLocation, ListBox * );
    DECL_LINK( AsyncInfoMsg, String* );

    bool        SwapEntryData(
        SvLBoxEntry* pSourceEntry, SvLBoxEntry* pTargetEntry );

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

    FixedLine                           aDescriptionLine;
    FixedText                           aDescriptionText;

    SvxScriptSelectorDialog*            pSelectorDlg;

    // the ResourceURL to select when opening the dialog
    rtl::OUString                       m_aURLToSelect;

    SvxConfigPage( Window*, /* const ResId&, */ const SfxItemSet& );
    virtual ~SvxConfigPage();

    DECL_LINK( MoveHdl, Button * );

    virtual SaveInData* CreateSaveInData(
        const ::com::sun::star::uno::Reference <
            drafts::com::sun::star::ui::XUIConfigurationManager >&,
        const rtl::OUString& aModuleId,
        bool docConfig ) = 0;

    virtual void            Init() = 0;
    virtual void            UpdateButtonStates() = 0;

    void            PositionContentsListBox();

    SvLBoxEntry*    InsertEntry(        SvxConfigEntry* pNewEntryData,
                                        SvLBoxEntry* pTarget = NULL,
                                        bool bFront = FALSE );

    void            AddSubMenusToUI(    const String& rBaseTitle,
                                        SvxConfigEntry* pParentData );

    SvLBoxEntry*    InsertEntryIntoUI ( SvxConfigEntry* pNewEntryData,
                                        USHORT nPos = LIST_APPEND );

    SvxEntries*     FindParentForChild( SvxEntries* pParentEntries,
                                        SvxConfigEntry* pChildData );

    void            ReloadTopLevelListBox( SvxConfigEntry* pSelection = NULL );

public:

    SaveInData*     GetSaveInData() { return pCurrentSaveInData; }

    SvLBoxEntry*    AddFunction( SvLBoxEntry* pTarget = NULL,
                                 bool bFront = FALSE,
                                 bool bAllowDuplicates = FALSE );

    virtual void    MoveEntry( bool bMoveUp );

    bool            MoveEntryData(  SvLBoxEntry* pSourceEntry,
                                    SvLBoxEntry* pTargetEntry );

    BOOL            FillItemSet( SfxItemSet& );
    void            Reset( const SfxItemSet& );

    virtual bool    DeleteSelectedContent() = 0;
    virtual void    DeleteSelectedTopLevel() = 0;

    SvxConfigEntry* GetTopLevelSelection()
    {
        return (SvxConfigEntry*) aTopLevelListBox.GetEntryData(
            aTopLevelListBox.GetSelectEntryPos() );
    }
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
    bool            DeleteSelectedContent();
    void            DeleteSelectedTopLevel();

public:
    SvxMenuConfigPage( Window *pParent, const SfxItemSet& rItemSet );
    ~SvxMenuConfigPage();

    SaveInData* CreateSaveInData(
        const ::com::sun::star::uno::Reference <
            drafts::com::sun::star::ui::XUIConfigurationManager >&,
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
        SvxConfigEntry*, bool bCreateMenu = FALSE );

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
    BOOL            m_bHiContrastMode;
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

    virtual BOOL    NotifyMoving(
        SvLBoxEntry*, SvLBoxEntry*, SvLBoxEntry*&, ULONG& );

    virtual BOOL    NotifyCopying(
        SvLBoxEntry*, SvLBoxEntry*, SvLBoxEntry*&, ULONG&);

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
    void            Init();
    bool            DeleteSelectedContent();
    void            DeleteSelectedTopLevel();

public:
    SvxToolbarConfigPage( Window *pParent, const SfxItemSet& rItemSet );
    ~SvxToolbarConfigPage();

    SvLBoxEntry*    AddFunction( SvLBoxEntry* pTarget = NULL,
                                             bool bFront = FALSE,
                                             bool bAllowDuplicates = TRUE );

    void            MoveEntry( bool bMoveUp );

    SaveInData*     CreateSaveInData(
        const ::com::sun::star::uno::Reference <
            drafts::com::sun::star::ui::XUIConfigurationManager >&,
        const rtl::OUString& aModuleId,
        bool docConfig );
};

class ToolbarSaveInData : public SaveInData
{
private:

    SvxConfigEntry* pRootEntry;
    rtl::OUString   m_aDescriptorContainer;

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
            drafts::com::sun::star::ui::XUIConfigurationManager >&,
        const rtl::OUString& aModuleId,
        bool docConfig );

    ~ToolbarSaveInData();

    void            CreateToolbar( SvxConfigEntry* pToolbar );
    void            RestoreToolbar( SvxConfigEntry* pToolbar );
    void            RemoveToolbar( SvxConfigEntry* pToolbar );
    void            ApplyToolbar( SvxConfigEntry* pToolbar );
    void            ReloadToolbar( const rtl::OUString& rURL );

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

    void    SetEditHelpId(ULONG nHelpId) {aEdtName.SetHelpId(nHelpId);}
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

    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::ui::XImageManager > m_xImageManager;

    void ImportGraphic( const rtl::OUString& aURL );

    void ImportGraphics(
        const com::sun::star::uno::Sequence< rtl::OUString >& aURLs );

public:

    SvxIconSelectorDialog(
        Window *pWindow,
        const ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::ui::XImageManager >& rXImageManager
        );

    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >
        GetSelectedIcon();

    DECL_LINK( SelectHdl, ToolBox * );
    DECL_LINK( ImportHdl, PushButton * );
};

#endif // _SVXCFG_HXX

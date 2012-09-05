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
#ifndef _SFX_TEMPDLGI_HXX
#define _SFX_TEMPDLGI_HXX

class SfxTemplateControllerItem;

#include <vcl/button.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/lstbox.hxx>
#include <svl/lstner.hxx>
#include <svtools/svtreebx.hxx>
#include <svl/eitem.hxx>

#include <rsc/rscsfx.hxx>
#include <tools/rtti.hxx>

#include <sfx2/childwin.hxx>
#include <sfx2/templdlg.hxx>

class SfxStyleFamilies;
class SfxStyleFamilyItem;
class SfxTemplateItem;
class SfxBindings;
class SfxStyleSheetBase;
class SfxStyleSheetBasePool;
class SvTreeListBox ;
class StyleTreeListBox_Impl;
class SfxTemplateDialog_Impl;
class SfxCommonTemplateDialog_Impl;
class SfxTemplateDialogWrapper;
class SfxDockingWindow;

namespace com { namespace sun { namespace star { namespace frame { class XModuleManager2; } } } }

// class DropListBox_Impl ------------------------------------------------

class DropListBox_Impl : public SvTreeListBox
{
private:
    DECL_LINK(OnAsyncExecuteDrop, void *);

protected:
    SfxCommonTemplateDialog_Impl* pDialog;
    sal_uInt16                        nModifier;

public:
    DropListBox_Impl( Window* pParent, const ResId& rId, SfxCommonTemplateDialog_Impl* pD ) :
        SvTreeListBox( pParent, rId ), pDialog( pD ) {}
    DropListBox_Impl( Window* pParent, WinBits nWinBits, SfxCommonTemplateDialog_Impl* pD ) :
        SvTreeListBox( pParent, nWinBits ), pDialog( pD ) {}

    virtual void     MouseButtonDown( const MouseEvent& rMEvt );
    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt );
    using SvLBox::ExecuteDrop;
    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt );

    sal_uInt16           GetModifier() const { return nModifier; }

    virtual long     Notify( NotifyEvent& rNEvt );
};

// class SfxActionListBox ------------------------------------------------

class SfxActionListBox : public DropListBox_Impl
{
protected:
public:
    SfxActionListBox( SfxCommonTemplateDialog_Impl* pParent, WinBits nWinBits );
    SfxActionListBox( SfxCommonTemplateDialog_Impl* pParent, const ResId &rResId );

    virtual PopupMenu*  CreateContextMenu( void );
};

// class SfxCommonTemplateDialog_Impl ------------------------------------

class SfxCommonTemplateDialog_Impl : public SfxListener
{
private:
    class DeletionWatcher;
    friend class DeletionWatcher;

    class ISfxTemplateCommon_Impl : public ISfxTemplateCommon
    {
    private:
        SfxCommonTemplateDialog_Impl* pDialog;
    public:
        ISfxTemplateCommon_Impl( SfxCommonTemplateDialog_Impl* pDialogP ) : pDialog( pDialogP ) {}
        virtual ~ISfxTemplateCommon_Impl() {}
        virtual SfxStyleFamily GetActualFamily() const { return pDialog->GetActualFamily(); }
        virtual String GetSelectedEntry() const { return pDialog->GetSelectedEntry(); }
    };

    ISfxTemplateCommon_Impl     aISfxTemplateCommon;

    void    ReadResource();
    void    ClearResource();
    void impl_clear();
    void impl_setDeletionWatcher(DeletionWatcher* pNewWatcher);

protected:
#define MAX_FAMILIES            5
#define COUNT_BOUND_FUNC        13

#define UPDATE_FAMILY_LIST      0x0001
#define UPDATE_FAMILY           0x0002

    friend class DropListBox_Impl;
    friend class SfxTemplateControllerItem;
    friend class SfxTemplateDialogWrapper;

    SfxBindings*                pBindings;
    SfxTemplateControllerItem*  pBoundItems[COUNT_BOUND_FUNC];

    Window*                     pWindow;
    SfxModule*                  pModule;
    Timer*                      pTimer;

    ResId*                      m_pStyleFamiliesId;
    SfxStyleFamilies*           pStyleFamilies;
    SfxTemplateItem*            pFamilyState[MAX_FAMILIES];
    SfxStyleSheetBasePool*      pStyleSheetPool;
    SvTreeListBox*              pTreeBox;
    SfxObjectShell*             pCurObjShell;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModuleManager2 >
                                xModuleManager;
    DeletionWatcher*            m_pDeletionWatcher;

    SfxActionListBox            aFmtLb;
    ListBox                     aFilterLb;
    Size                        aSize;

    sal_uInt16                      nActFamily; // Id in the ToolBox = Position - 1
    sal_uInt16                      nActFilter; // FilterIdx
    sal_uInt16                      nAppFilter; // Filter, which has set the application (for automatic)

    sal_Bool                        bDontUpdate             :1,
                                bIsWater                :1,
                                bEnabled                :1,
                                bUpdate                 :1,
                                bUpdateFamily           :1,
                                bCanEdit                :1,
                                bCanDel                 :1,
                                bCanNew                 :1,
                                bWaterDisabled          :1,
                                bNewByExampleDisabled   :1,
                                bUpdateByExampleDisabled:1,
                                bTreeDrag               :1,
                                bHierarchical           :1,
                                bBindingUpdate          :1;

    DECL_LINK( FilterSelectHdl, ListBox * );
    DECL_LINK( FmtSelectHdl, SvTreeListBox * );
    DECL_LINK( ApplyHdl, Control * );
    DECL_LINK( DropHdl, StyleTreeListBox_Impl * );
    DECL_LINK( TimeOut, Timer * );


    virtual void        EnableItem( sal_uInt16 /*nMesId*/, sal_Bool /*bCheck*/ = sal_True ) {}
    virtual void        CheckItem( sal_uInt16 /*nMesId*/, sal_Bool /*bCheck*/ = sal_True ) {}
    virtual sal_Bool        IsCheckedItem( sal_uInt16 /*nMesId*/ ) { return sal_True; }
    virtual void        LoadedFamilies() {}
    virtual void        Update() { UpdateStyles_Impl(UPDATE_FAMILY_LIST); }
    virtual void        InvalidateBindings();
    virtual void        InsertFamilyItem( sal_uInt16 nId, const SfxStyleFamilyItem* pIten ) = 0;
    virtual void        EnableFamilyItem( sal_uInt16 nId, sal_Bool bEnabled = sal_True ) = 0;
    virtual void        ClearFamilyList() = 0;
    virtual void        ReplaceUpdateButtonByMenu();

        void                NewHdl( void* );
    void                EditHdl( void* );
    void                DeleteHdl( void* );

    sal_Bool                Execute_Impl( sal_uInt16 nId, const String& rStr, const String& rRefStr,
                                      sal_uInt16 nFamily, sal_uInt16 nMask = 0,
                                      sal_uInt16* pIdx = NULL, const sal_uInt16* pModifier = NULL );

    void                        UpdateStyles_Impl(sal_uInt16 nFlags);
    const SfxStyleFamilyItem*   GetFamilyItem_Impl() const;
    sal_Bool                    IsInitialized() const { return nActFamily != 0xffff; }
    void                        ResetFocus();
    void                        EnableDelete();
    void                        Initialize();

    void                FilterSelect( sal_uInt16 nFilterIdx, sal_Bool bForce = sal_False );
    void                SetFamilyState( sal_uInt16 nSlotId, const SfxTemplateItem* );
    void                SetWaterCanState( const SfxBoolItem* pItem );

    void                SelectStyle( const String& rStyle );
    sal_Bool            HasSelectedStyle() const;
    SfxStyleSheetBase  *GetSelectedStyle() const;
    void                FillTreeBox();
    void                Update_Impl();
    void                UpdateFamily_Impl();

    // In which FamilyState do I have to look , in order to get the
    // information of the ith Family in the pStyleFamilies.
    sal_uInt16              StyleNrToInfoOffset( sal_uInt16 i );

    void                Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    void                FamilySelect( sal_uInt16 nId );
    void                SetFamily( sal_uInt16 nId );
    void                ActionSelect( sal_uInt16 nId );

    sal_Int32           LoadFactoryStyleFilter( SfxObjectShell* i_pObjSh );
    void                SaveFactoryStyleFilter( SfxObjectShell* i_pObjSh, sal_Int32 i_nFilter );

public:
    TYPEINFO();

    SfxCommonTemplateDialog_Impl( SfxBindings* pB, SfxDockingWindow* );
    SfxCommonTemplateDialog_Impl( SfxBindings* pB, ModalDialog* );
    ~SfxCommonTemplateDialog_Impl();

    DECL_LINK( MenuSelectHdl, Menu * );

    virtual void        EnableEdit( sal_Bool b = sal_True ) { bCanEdit = b; }
    virtual void        EnableDel( sal_Bool b = sal_True )  { bCanDel = b; }
    virtual void        EnableNew( sal_Bool b = sal_True )  { bCanNew = b; }

    ISfxTemplateCommon* GetISfxTemplateCommon() { return &aISfxTemplateCommon; }
    Window*             GetWindow() { return pWindow; }

    void                EnableTreeDrag( sal_Bool b = sal_True );
    void                ExecuteContextMenu_Impl( const Point& rPos, Window* pWin );
    void                EnableExample_Impl( sal_uInt16 nId, sal_Bool bEnable );
    SfxStyleFamily      GetActualFamily() const;
    String              GetSelectedEntry() const;
    SfxObjectShell*     GetObjectShell() const { return pCurObjShell; }

    virtual void        PrepareDeleteAction();  // disable buttons, change button text, etc. when del is going to happen

    inline sal_Bool         CanEdit( void ) const   { return bCanEdit; }
    inline sal_Bool         CanDel( void ) const    { return bCanDel; }
    inline sal_Bool         CanNew( void ) const    { return bCanNew; }

    // normaly for derivates from SvTreeListBoxes, but in this case the dialog handles context menus
    virtual PopupMenu*  CreateContextMenu( void );

    // converts from SFX_STYLE_FAMILY Ids to 1-5
    static sal_uInt16       SfxFamilyIdToNId( SfxStyleFamily nFamily );

    void                SetAutomaticFilter();
};

class DropToolBox_Impl : public ToolBox, public DropTargetHelper
{
    SfxTemplateDialog_Impl&     rParent;
protected:
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );
public:
    DropToolBox_Impl(Window* pParent, SfxTemplateDialog_Impl* pTemplateDialog);
    ~DropToolBox_Impl();
};
// class SfxTemplateDialog_Impl ------------------------------------------

class SfxTemplateDialog_Impl :  public SfxCommonTemplateDialog_Impl
{
private:
    friend class SfxTemplateControllerItem;
    friend class SfxTemplateDialogWrapper;
    friend class DropToolBox_Impl;

    SfxTemplateDialog*  m_pFloat;
    sal_Bool                m_bZoomIn;
    DropToolBox_Impl    m_aActionTbL;
    ToolBox             m_aActionTbR;

    DECL_LINK( ToolBoxLSelect, ToolBox * );
    DECL_LINK( ToolBoxRSelect, ToolBox * );
    DECL_LINK( ToolBoxRClick, ToolBox * );
    DECL_LINK( MenuSelectHdl, Menu* );

protected:
    virtual void    Command( const CommandEvent& rMEvt );
    virtual void    EnableEdit( sal_Bool = sal_True );
    virtual void    EnableItem( sal_uInt16 nMesId, sal_Bool bCheck = sal_True );
    virtual void    CheckItem( sal_uInt16 nMesId, sal_Bool bCheck = sal_True );
    virtual sal_Bool    IsCheckedItem( sal_uInt16 nMesId );
    virtual void    LoadedFamilies();
    virtual void    InsertFamilyItem( sal_uInt16 nId, const SfxStyleFamilyItem* pIten );
    virtual void    EnableFamilyItem( sal_uInt16 nId, sal_Bool bEnabled = sal_True );
    virtual void    ClearFamilyList();
    virtual void    ReplaceUpdateButtonByMenu();

    void            Resize();
    Size            GetMinOutputSizePixel();

    void            updateFamilyImages();
    void            updateNonFamilyImages();

public:
    friend class SfxTemplateDialog;
    TYPEINFO();

    SfxTemplateDialog_Impl( Window* pParent, SfxBindings*, SfxTemplateDialog* pWindow );
    ~SfxTemplateDialog_Impl();
};

// class SfxTemplateCatalog_Impl -----------------------------------------

class SfxTemplateCatalog_Impl : public SfxCommonTemplateDialog_Impl
{
private:
    friend class SfxTemplateControllerItem;
    friend class SfxCommonTemplateDialog_Impl;

    ListBox                 aFamList;
    OKButton                aOkBtn;
    CancelButton            aCancelBtn;
    PushButton              aNewBtn;
    PushButton              aChangeBtn;
    PushButton              aDelBtn;
    PushButton              aOrgBtn;
    HelpButton              aHelpBtn;

    SfxTemplateCatalog*     pReal;
    std::vector<sal_uInt16> aFamIds;
    SfxModalDefParentHelper aHelper;

protected:
    virtual void    EnableItem( sal_uInt16 nMesId, sal_Bool bCheck = sal_True );
    virtual void    CheckItem( sal_uInt16 nMesId, sal_Bool bCheck = sal_True );
    virtual sal_Bool    IsCheckedItem( sal_uInt16 nMesId );
    virtual void    InsertFamilyItem( sal_uInt16 nId, const SfxStyleFamilyItem* pIten );
    virtual void    EnableFamilyItem( sal_uInt16 nId, sal_Bool bEnabled = sal_True );
    virtual void    ClearFamilyList();
    virtual void    EnableEdit( sal_Bool = sal_True );
    virtual void    EnableDel( sal_Bool = sal_True );
    virtual void    EnableNew( sal_Bool = sal_True );

        using SfxCommonTemplateDialog_Impl::NewHdl;
    DECL_LINK( FamListSelect, ListBox * );
    DECL_LINK( OkHdl, Button * );
    DECL_LINK( CancelHdl, Button * );
    DECL_LINK( NewHdl, Button * );
    DECL_LINK( ChangeHdl, Button * );
    DECL_LINK( DelHdl, Button * );
    DECL_LINK( OrgHdl, Button * );

public:
    TYPEINFO();
    SfxTemplateCatalog_Impl( Window* pParent, SfxBindings*, SfxTemplateCatalog* pWindow );
    ~SfxTemplateCatalog_Impl();

friend class SfxTemplateCatalog;

    virtual void    PrepareDeleteAction();
};

#endif // #ifndef _SFX_TEMPDLGI_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#ifndef INCLUDED_SFX2_SOURCE_INC_TEMPLDGI_HXX
#define INCLUDED_SFX2_SOURCE_INC_TEMPLDGI_HXX

class SfxTemplateControllerItem;

#include <vcl/button.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/lstbox.hxx>
#include <svl/lstner.hxx>
#include <svtools/treelistbox.hxx>
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
    sal_uInt16                    nModifier;

public:
    DropListBox_Impl( Window* pParent, const ResId& rId, SfxCommonTemplateDialog_Impl* pD )
        : SvTreeListBox(pParent, rId)
        , pDialog(pD)
        , nModifier(0)
    {}
    DropListBox_Impl( Window* pParent, WinBits nWinBits, SfxCommonTemplateDialog_Impl* pD )
        : SvTreeListBox(pParent, nWinBits)
        , pDialog(pD)
        , nModifier(0)
    {}
    virtual void     MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) SAL_OVERRIDE;
    using SvTreeListBox::ExecuteDrop;
    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) SAL_OVERRIDE;

    sal_uInt16           GetModifier() const { return nModifier; }

    virtual bool     Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
};

typedef std::vector<OUString> ExpandedEntries_t;

/*  [Description]

    TreeListBox class for displaying the hierarchical view of the templates
*/

class StyleTreeListBox_Impl : public DropListBox_Impl
{
private:
    SvTreeListEntry*                pCurEntry;
    Link                            aDoubleClickLink;
    Link                            aDropLink;
    OUString                        aParent;
    OUString                        aStyle;

protected:
    virtual void    Command( const CommandEvent& rMEvt ) SAL_OVERRIDE;
    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual bool    DoubleClickHdl() SAL_OVERRIDE;
    virtual bool    ExpandingHdl() SAL_OVERRIDE;
    virtual void    ExpandedHdl() SAL_OVERRIDE;
    virtual sal_Bool    NotifyMoving(SvTreeListEntry*  pTarget,
                                     SvTreeListEntry*  pEntry,
                                     SvTreeListEntry*& rpNewParent,
                                     sal_uIntPtr&        rNewChildPos) SAL_OVERRIDE;
public:
    StyleTreeListBox_Impl( SfxCommonTemplateDialog_Impl* pParent, WinBits nWinStyle = 0);

    void            SetDoubleClickHdl(const Link &rLink) { aDoubleClickLink = rLink; }
    void            SetDropHdl(const Link &rLink) { aDropLink = rLink; }
    using SvTreeListBox::GetParent;
    const OUString& GetParent() const { return aParent; }
    const OUString& GetStyle() const { return aStyle; }
    void            MakeExpanded_Impl(ExpandedEntries_t& rEntries) const;

    virtual PopupMenu* CreateContextMenu( void ) SAL_OVERRIDE;
};

// class SfxActionListBox ------------------------------------------------

class SfxActionListBox : public DropListBox_Impl
{
protected:
public:
    SfxActionListBox( SfxCommonTemplateDialog_Impl* pParent, WinBits nWinBits );

    virtual PopupMenu*  CreateContextMenu( void ) SAL_OVERRIDE;
};

// class SfxCommonTemplateDialog_Impl ------------------------------------

class SfxCommonTemplateDialog_Impl : public SfxListener
{
private:
    class DeletionWatcher;
    friend class DeletionWatcher;
    bool mbIgnoreSelect;
    class ISfxTemplateCommon_Impl : public ISfxTemplateCommon
    {
    private:
        SfxCommonTemplateDialog_Impl* pDialog;
    public:
        ISfxTemplateCommon_Impl( SfxCommonTemplateDialog_Impl* pDialogP ) : pDialog( pDialogP ) {}
        virtual ~ISfxTemplateCommon_Impl() {}
        virtual SfxStyleFamily GetActualFamily() const SAL_OVERRIDE { return pDialog->GetActualFamily(); }
        virtual OUString GetSelectedEntry() const SAL_OVERRIDE { return pDialog->GetSelectedEntry(); }
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
    StyleTreeListBox_Impl*      pTreeBox;
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

    bool                        bDontUpdate             :1,
                                bIsWater                :1,
                                bEnabled                :1,
                                bUpdate                 :1,
                                bUpdateFamily           :1,
                                bCanEdit                :1,
                                bCanDel                 :1,
                                bCanNew                 :1,
                                bCanHide                :1,
                                bCanShow                :1,
                                bWaterDisabled          :1,
                                bNewByExampleDisabled   :1,
                                bUpdateByExampleDisabled:1,
                                bTreeDrag               :1,
                                bHierarchical           :1,
                                m_bWantHierarchical     :1,
                                bBindingUpdate          :1;

    DECL_LINK( FilterSelectHdl, ListBox * );
    DECL_LINK( FmtSelectHdl, SvTreeListBox * );
    DECL_LINK( ApplyHdl, Control * );
    DECL_LINK( DropHdl, StyleTreeListBox_Impl * );
    DECL_LINK( TimeOut, Timer * );


    virtual void        EnableItem( sal_uInt16 /*nMesId*/, bool /*bCheck*/ = true ) {}
    virtual void        CheckItem( sal_uInt16 /*nMesId*/, bool /*bCheck*/ = true ) {}
    virtual bool        IsCheckedItem( sal_uInt16 /*nMesId*/ ) { return true; }
    virtual void        LoadedFamilies() {}
    virtual void        Update() { UpdateStyles_Impl(UPDATE_FAMILY_LIST); }
    virtual void        InvalidateBindings();
    virtual void        InsertFamilyItem( sal_uInt16 nId, const SfxStyleFamilyItem* pIten ) = 0;
    virtual void        EnableFamilyItem( sal_uInt16 nId, bool bEnabled = true ) = 0;
    virtual void        ClearFamilyList() = 0;
    virtual void        ReplaceUpdateButtonByMenu();

    void                NewHdl( void* );
    void                EditHdl( void* );
    void                DeleteHdl( void* );
    void                HideHdl( void* );
    void                ShowHdl( void* );

    bool                Execute_Impl( sal_uInt16 nId, const OUString& rStr, const OUString& rRefStr,
                                      sal_uInt16 nFamily, sal_uInt16 nMask = 0,
                                      sal_uInt16* pIdx = NULL, const sal_uInt16* pModifier = NULL );

    void                        UpdateStyles_Impl(sal_uInt16 nFlags);
    const SfxStyleFamilyItem*   GetFamilyItem_Impl() const;
    bool                    IsInitialized() const { return nActFamily != 0xffff; }
    void                        ResetFocus();
    void                        EnableDelete();
    void                        Initialize();
    void                        EnableHierarchical(bool);

    void                FilterSelect( sal_uInt16 nFilterIdx, bool bForce = false );
    void                SetFamilyState( sal_uInt16 nSlotId, const SfxTemplateItem* );
    void                SetWaterCanState( const SfxBoolItem* pItem );

    void                SelectStyle( const OUString& rStyle );
    bool            HasSelectedStyle() const;
    SfxStyleSheetBase  *GetSelectedStyle() const;
    void                FillTreeBox();
    void                Update_Impl();
    void                UpdateFamily_Impl();

    // In which FamilyState do I have to look , in order to get the
    // information of the ith Family in the pStyleFamilies.
    sal_uInt16              StyleNrToInfoOffset( sal_uInt16 i );

    void                Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

    void                FamilySelect( sal_uInt16 nId );
    void                SetFamily( sal_uInt16 nId );
    void                ActionSelect( sal_uInt16 nId );

    sal_Int32           LoadFactoryStyleFilter( SfxObjectShell* i_pObjSh );
    void                SaveFactoryStyleFilter( SfxObjectShell* i_pObjSh, sal_Int32 i_nFilter );
    SfxObjectShell *    SaveSelection();

public:
    TYPEINFO_OVERRIDE();

    SfxCommonTemplateDialog_Impl( SfxBindings* pB, Window*, bool );
    virtual ~SfxCommonTemplateDialog_Impl();

    DECL_LINK( MenuSelectHdl, Menu * );

    virtual void        EnableEdit( bool b = true ) { bCanEdit = b; }
    virtual void        EnableDel( bool b = true )  { bCanDel = b; }
    virtual void        EnableNew( bool b = true )  { bCanNew = b; }
    virtual void        EnableHide( bool b = true )  { bCanHide = b; }
    virtual void        EnableShow( bool b = true )  { bCanShow = b; }

    ISfxTemplateCommon* GetISfxTemplateCommon() { return &aISfxTemplateCommon; }
    Window*             GetWindow() { return pWindow; }

    void                EnableTreeDrag( bool b = true );
    void                ExecuteContextMenu_Impl( const Point& rPos, Window* pWin );
    void                EnableExample_Impl( sal_uInt16 nId, bool bEnable );
    SfxStyleFamily      GetActualFamily() const;
    OUString            GetSelectedEntry() const;
    SfxObjectShell*     GetObjectShell() const { return pCurObjShell; }

    virtual void        PrepareDeleteAction();  // disable buttons, change button text, etc. when del is going to happen

    inline bool         CanEdit( void ) const   { return bCanEdit; }
    inline bool         CanDel( void ) const    { return bCanDel; }
    inline bool         CanNew( void ) const    { return bCanNew; }
    inline bool         CanHide( void ) const    { return bCanHide; }
    inline bool         CanShow( void ) const    { return bCanShow; }

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
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt ) SAL_OVERRIDE;
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt ) SAL_OVERRIDE;
public:
    DropToolBox_Impl(Window* pParent, SfxTemplateDialog_Impl* pTemplateDialog);
    virtual ~DropToolBox_Impl();
};
// class SfxTemplateDialog_Impl ------------------------------------------

class SfxTemplateDialog_Impl :  public SfxCommonTemplateDialog_Impl
{
private:
    friend class SfxTemplateControllerItem;
    friend class SfxTemplateDialogWrapper;
    friend class DropToolBox_Impl;
    friend class SfxTemplatePanelControl;

    Window* m_pFloat;
    bool            m_bZoomIn;
    DropToolBox_Impl    m_aActionTbL;
    ToolBox             m_aActionTbR;

    DECL_LINK( ToolBoxLSelect, ToolBox * );
    DECL_LINK( ToolBoxRSelect, ToolBox * );
    DECL_LINK( ToolBoxRClick, ToolBox * );
    DECL_LINK( MenuSelectHdl, Menu* );

protected:
    virtual void    Command( const CommandEvent& rMEvt );
    virtual void    EnableEdit( bool = true ) SAL_OVERRIDE;
    virtual void    EnableItem( sal_uInt16 nMesId, bool bCheck = true ) SAL_OVERRIDE;
    virtual void    CheckItem( sal_uInt16 nMesId, bool bCheck = true ) SAL_OVERRIDE;
    virtual bool    IsCheckedItem( sal_uInt16 nMesId ) SAL_OVERRIDE;
    virtual void    LoadedFamilies() SAL_OVERRIDE;
    virtual void    InsertFamilyItem( sal_uInt16 nId, const SfxStyleFamilyItem* pIten ) SAL_OVERRIDE;
    virtual void    EnableFamilyItem( sal_uInt16 nId, bool bEnabled = true ) SAL_OVERRIDE;
    virtual void    ClearFamilyList() SAL_OVERRIDE;
    virtual void    ReplaceUpdateButtonByMenu() SAL_OVERRIDE;

    void            Resize();
    Size            GetMinOutputSizePixel();

    void            updateFamilyImages();
    void            updateNonFamilyImages();

public:
    friend class SfxTemplateDialog;
    TYPEINFO_OVERRIDE();

    SfxTemplateDialog_Impl( SfxBindings*, SfxTemplateDialog* pDlgWindow );
    SfxTemplateDialog_Impl( SfxBindings*, SfxTemplatePanelControl* pDlgWindow );
    virtual ~SfxTemplateDialog_Impl();

    void Initialize (void);
};

#endif // INCLUDED_SFX2_SOURCE_INC_TEMPLDGI_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

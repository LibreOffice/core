/*************************************************************************
 *
 *  $RCSfile: templdgi.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:35 $
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
#ifndef _SFX_TEMPDLGI_HXX
#define _SFX_TEMPDLGI_HXX

class SfxTemplateControllerItem;

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif
#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif

#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>     // SvUShorts

#include <rsc/rscsfx.hxx>
#include <tools/rtti.hxx>

#include "childwin.hxx"
#include "templdlg.hxx"

class SfxStyleFamilies;
class SfxStyleFamilyItem;
class SfxTemplateItem;
class SfxBindings;
class SfxStyleSheetBasePool;
class SvTreeListBox ;
class StyleTreeListBox_Impl;
class SfxTemplateDialog_Impl;
class SfxCommonTemplateDialog_Impl;
class SfxTemplateDialogWrapper;
class SfxDockingWindow;

// class DropListBox_Impl ------------------------------------------------

class DropListBox_Impl : public SvTreeListBox
{
protected:
    SvLBoxEntry*                    pPreDropEntry;
    SfxCommonTemplateDialog_Impl*   pDialog;
    USHORT                          nModifier;

public:
    DropListBox_Impl( Window* pParent, const ResId& rId, SfxCommonTemplateDialog_Impl* pD ) :
        SvTreeListBox( pParent, rId ), pDialog( pD ), pPreDropEntry( NULL )
            { EnableDrop( TRUE ); }
    DropListBox_Impl( Window* pParent, WinBits nWinBits, SfxCommonTemplateDialog_Impl* pD ) :
        SvTreeListBox( pParent, nWinBits ), pDialog( pD ), pPreDropEntry( NULL )
            { EnableDrop( TRUE ); }

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual BOOL    Drop( const DropEvent& rEvt );
    virtual BOOL    QueryDrop( DropEvent& rEvt );

    USHORT          GetModifier() const { return nModifier; }
    SvLBoxEntry*    GetPreDropEntry() const { return pPreDropEntry; }
};

// class SfxActionListBox ------------------------------------------------

class SfxActionListBox : public DropListBox_Impl
{
protected:
    virtual void    Command( const CommandEvent& rMEvt );

public:
    SfxActionListBox( SfxCommonTemplateDialog_Impl* pParent, WinBits nWinBits );
    SfxActionListBox( SfxCommonTemplateDialog_Impl* pParent, const ResId &rResId );
};

// class SfxCommonTemplateDialog_Impl ------------------------------------

class SfxCommonTemplateDialog_Impl : public SfxListener
{
private:
    class ISfxTemplateCommon_Impl : public ISfxTemplateCommon
    {
    private:
        SfxCommonTemplateDialog_Impl* pDialog;
    public:
        ISfxTemplateCommon_Impl( SfxCommonTemplateDialog_Impl* pDialogP ) : pDialog( pDialogP ) {}
        virtual SfxStyleFamily GetActualFamily() const { return pDialog->GetActualFamily(); }
        virtual String GetSelectedEntry() const { return pDialog->GetSelectedEntry(); }
    };

    ISfxTemplateCommon_Impl     aISfxTemplateCommon;

    void    ReadResource();
    void    ClearResource();

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

    SfxStyleFamilies*           pStyleFamilies;
    SfxTemplateItem*            pFamilyState[MAX_FAMILIES];
    SfxStyleSheetBasePool*      pStyleSheetPool;
    SvTreeListBox*              pTreeBox;
    SfxObjectShell*             pCurObjShell;

    SfxActionListBox            aFmtLb;
    ListBox                     aFilterLb;
    Size                        aSize;

    USHORT                      nActFamily; // Id in der ToolBox = Position - 1
    USHORT                      nActFilter; // FilterIdx
    USHORT                      nAppFilter; // Filter, den die Applikation gesetzt hat (fuer automatisch)

    BOOL                        bDontUpdate             :1,
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

                        // Rechnet von den SFX_STYLE_FAMILY Ids auf 1-5 um
    static USHORT       SfxFamilyIdToNId( USHORT nFamily );

    virtual void        EnableItem( USHORT nMesId, BOOL bCheck = TRUE ) {}
    virtual void        CheckItem( USHORT nMesId, BOOL bCheck = TRUE ) {}
    virtual BOOL        IsCheckedItem( USHORT nMesId ) { return TRUE; }
    virtual void        Resize() {}
    virtual void        Update() { UpdateStyles_Impl(UPDATE_FAMILY_LIST); }
    virtual void        InvalidateBindings();
    virtual void        InsertFamilyItem( USHORT nId, const SfxStyleFamilyItem* pIten ) = 0;
    virtual void        EnableFamilyItem( USHORT nId, BOOL bEnabled = TRUE ) = 0;
    virtual void        ClearFamilyList() = 0;

    void                NewHdl( void* );
    void                EditHdl( void* );
    void                DeleteHdl( void* );

    BOOL                Execute_Impl( USHORT nId, const String& rStr, const String& rRefStr,
                                      USHORT nFamily, USHORT nMask = 0,
                                      USHORT* pIdx = NULL, const USHORT* pModifier = NULL );

    void                        UpdateStyles_Impl(USHORT nFlags);
    const SfxStyleFamilyItem*   GetFamilyItem_Impl() const;
    BOOL                        IsInitialized() { return nActFamily != 0xffff; }
    void                        ResetFocus();
    void                        EnableDelete();
    void                        Initialize();

    void                FilterSelect( USHORT nFilterIdx, BOOL bForce = FALSE );
    void                SetFamilyState( USHORT nSlotId, const SfxTemplateItem* );
    void                SetWaterCanState( const SfxBoolItem* pItem );

    void                SelectStyle( const String& rStyle );
    BOOL                HasSelectedStyle() const;
    void                FillTreeBox();
    void                Update_Impl();
    void                UpdateFamily_Impl();

    // In welchem FamilyState muss ich nachsehen, um die Info der i-ten
    // Family in der pStyleFamilies zu bekommen.
    USHORT              StyleNrToInfoOffset( USHORT i );
    USHORT              InfoOffsetToStyleNr( USHORT i );

    void                SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                                    const SfxHint& rHint, const TypeId& rHintType );

    void                FamilySelect( USHORT nId );
    void                SetFamily( USHORT nId );
    void                ActionSelect( USHORT nId );

public:
    TYPEINFO();

    SfxCommonTemplateDialog_Impl( SfxBindings* pB, SfxDockingWindow* );
    SfxCommonTemplateDialog_Impl( SfxBindings* pB, ModalDialog* );
    ~SfxCommonTemplateDialog_Impl();

    DECL_LINK( MenuSelectHdl, Menu * );

    virtual void        EnableEdit( BOOL b = TRUE ) { bCanEdit = b; }
    virtual void        EnableDel( BOOL b = TRUE )  { bCanDel = b; }
    virtual void        EnableNew( BOOL b = TRUE )  { bCanNew = b; }

    ISfxTemplateCommon* GetISfxTemplateCommon() { return &aISfxTemplateCommon; }
    Window*             GetWindow() { return pWindow; }

    void                EnableTreeDrag( BOOL b = TRUE );
    void                ExecuteContextMenu_Impl( const Point& rPos, Window* pWin );
    void                EnableExample_Impl( USHORT nId, BOOL bEnable );
    SfxStyleFamily      GetActualFamily() const;
    String              GetSelectedEntry() const;
    SfxObjectShell*     GetObjectShell() const { return pCurObjShell; }
};

// class SfxTemplateDialog_Impl ------------------------------------------

class SfxTemplateDialog_Impl :  public SfxCommonTemplateDialog_Impl
{
private:
    friend class SfxTemplateControllerItem;
    friend class SfxTemplateDialogWrapper;

    SfxTemplateDialog*  pFloat;
    BOOL                bZoomIn;
    ToolBox             aActionTbL;
    ToolBox             aActionTbR;

    DECL_LINK( ToolBoxLSelect, ToolBox * );
    DECL_LINK( ToolBoxRSelect, ToolBox * );

protected:
    virtual void    Command( const CommandEvent& rMEvt );
    virtual void    EnableEdit( BOOL = TRUE );
    virtual void    EnableItem( USHORT nMesId, BOOL bCheck = TRUE );
    virtual void    CheckItem( USHORT nMesId, BOOL bCheck = TRUE );
    virtual BOOL    IsCheckedItem( USHORT nMesId );
    virtual void    InsertFamilyItem( USHORT nId, const SfxStyleFamilyItem* pIten );
    virtual void    EnableFamilyItem( USHORT nId, BOOL bEnabled = TRUE );
    virtual void    ClearFamilyList();

    void            Resize();
    Size            GetMinOutputSizePixel();

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
    SvUShorts               aFamIds;
    SfxModalDefParentHelper aHelper;

protected:
    virtual void    EnableItem( USHORT nMesId, BOOL bCheck = TRUE );
    virtual void    CheckItem( USHORT nMesId, BOOL bCheck = TRUE );
    virtual BOOL    IsCheckedItem( USHORT nMesId );
    virtual void    InsertFamilyItem( USHORT nId, const SfxStyleFamilyItem* pIten );
    virtual void    EnableFamilyItem( USHORT nId, BOOL bEnabled = TRUE );
    virtual void    ClearFamilyList();
    virtual void    EnableEdit( BOOL = TRUE );
    virtual void    EnableDel( BOOL = TRUE );
    virtual void    EnableNew( BOOL = TRUE );

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
};

#endif // #ifndef _SFX_TEMPDLGI_HXX



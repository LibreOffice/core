/*************************************************************************
 *
 *  $RCSfile: cfg.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:33 $
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
#ifndef _SFXCFG_HXX
#define _SFXCFG_HXX

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _SVTABBX_HXX //autogen
#include <svtools/svtabbx.hxx>
#endif
#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif

#define _SVSTDARR_USHORTS
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>     // SvUShorts
#include "minarray.hxx"
#include "tabdlg.hxx"

class SfxMenuManager;
class SfxAcceleratorManager;
class SfxMacroInfoArr_Impl;
class SfxMacroInfoItem;
class SfxMacroInfo;
class SfxConfigManager;

#define SFX_CFGGROUP_FUNCTION 1
#define SFX_CFGGROUP_BASICMGR 2
#define SFX_CFGGROUP_DOCBASICMGR 3
#define SFX_CFGGROUP_BASICLIB 4
#define SFX_CFGGROUP_BASICMOD 5
#define SFX_CFGFUNCTION_MACRO 6
#define SFX_CFGFUNCTION_SLOT  7

struct SfxGroupInfo_Impl
{
    USHORT      nKind;
    USHORT      nOrd;
    void*       pObject;
    BOOL        bWasOpened;

                SfxGroupInfo_Impl( USHORT n, USHORT nr, void* pObj = 0 ) :
                    nKind( n ), nOrd( nr ), pObject( pObj ), bWasOpened(FALSE) {}
};

typedef SfxGroupInfo_Impl* SfxGroupInfoPtr;
SV_DECL_PTRARR_DEL(SfxGroupInfoArr_Impl, SfxGroupInfoPtr, 5, 5);
DECL_2BYTEARRAY(USHORTArr, USHORT, 10, 10);

class SfxConfigFunctionListBox_Impl : public SvTreeListBox
{
friend class SfxConfigGroupListBox_Impl;
    Timer                           aTimer;
    SvLBoxEntry*                    pCurEntry;
    SfxGroupInfoArr_Impl            aArr;

    DECL_LINK(                      TimerHdl, Timer* );
    virtual void                    MouseMove( const MouseEvent& rMEvt );

public:
                                    SfxConfigFunctionListBox_Impl( Window*, const ResId& );
                                    ~SfxConfigFunctionListBox_Impl();
    void                            ClearAll();
    SvLBoxEntry*                    GetEntry_Impl( USHORT nId );
    SvLBoxEntry*                    GetEntry_Impl( const String& );
    USHORT                          GetId( SvLBoxEntry *pEntry );
    String                          GetHelpText( SvLBoxEntry *pEntry );
    USHORT                          GetCurId()
                                    { return GetId( FirstSelected() ); }
    SfxMacroInfo*                   GetMacroInfo();
    void                            FunctionSelected();
};

class SfxConfigGroupListBox_Impl : public SvTreeListBox
{
    SfxConfigFunctionListBox_Impl*  pFunctionListBox;
    SfxGroupInfoArr_Impl            aArr;
    ULONG                           nMode;
    String                          aScriptType;

protected:
    virtual void                    RequestingChilds( SvLBoxEntry *pEntry);
#if SUPD>=527
    virtual BOOL                    Expand( SvLBoxEntry* pParent );
#endif

public:
                                    SfxConfigGroupListBox_Impl ( Window* pParent,
                                        const ResId&, ULONG nConfigMode = 0 );
                                    ~SfxConfigGroupListBox_Impl();

    void                            Init( SvStringsDtor *pArr = 0 );
    void                            SetFunctionListBox( SfxConfigFunctionListBox_Impl *pBox )
                                    { pFunctionListBox = pBox; }
    void                            Open( SvLBoxEntry*, BOOL );
    void                            GroupSelected();
    void                            SelectMacro( const SfxMacroInfoItem* );
    void                            SelectMacro( const String&, const String& );
    String                          GetGroup();
    void                            SetScriptType( const String& rScriptType );
};

class SfxMenuConfigEntry
{
private:
    USHORT                      nId;
    BOOL                        bPopUp;
    String                      aHelpText;
    String                      aStr;

public:
    SfxMenuConfigEntry( USHORT nInitId, const String& rInitStr,
                        const String& rHelpText, BOOL bPopup = FALSE );
    SfxMenuConfigEntry() : nId( 0 ), bPopUp( FALSE ) {}
    ~SfxMenuConfigEntry();

    USHORT                      GetId() const { return nId; }
    void                        SetId( USHORT nNew );
    const String&               GetName() const { return aStr; }
    void                        SetName( const String& rStr ) { aStr = rStr; }
    const String&               GetHelpText() const { return aHelpText; }
    void                        SetHelpText( const String& rStr ) { aHelpText = rStr; }
    void                        SetPopup( BOOL bOn = TRUE ) { bPopUp = bOn; }
    BOOL                        IsBinding() const { return nId != 0 && !bPopUp; }
    BOOL                        IsSeparator() const { return nId == 0; }
    BOOL                        IsPopup() const { return bPopUp; }
};

// class SfxMenuConfigEntryArr *************************************************

SV_DECL_PTRARR_DEL(SfxMenuConfigEntryArr, SfxMenuConfigEntry *, 16, 16)

class SfxMenuConfigPage;
class SfxMenuCfgTabListBox_Impl : public SvTabListBox
{
    SfxMenuConfigPage*          pPage;
    SfxMenuConfigEntryArr       aMenuArr;
    Timer                       aTimer;
    SfxMenuConfigEntry*         pCurEntry;

    DECL_LINK(                  TimerHdl, Timer* );

public :
                                SfxMenuCfgTabListBox_Impl( Window *pParent, const ResId& rResId );
                                ~SfxMenuCfgTabListBox_Impl();

    virtual long                PreNotify( NotifyEvent& );
    virtual void                ModelIsRemoving( SvListEntry* pEntry );
    void                        EntryInserted( SvListEntry* pEntry );
    virtual void                ModelHasCleared();
    virtual void                EditingRequest( SvLBoxEntry* pEntry,
                                                SvLBoxItem* pItem, const Point& rMousePos );
    virtual BOOL                EditedEntry( SvLBoxEntry* pEntry, const String& rNewText );
    virtual void                MouseMove( const MouseEvent& rMEvt );
    virtual BOOL                QueryDrop( DropEvent& );
    virtual BOOL                NotifyMoving(SvLBoxEntry*, SvLBoxEntry*, SvLBoxEntry*&, ULONG& );
    void                        Apply( SfxMenuManager* pMgr, SvLBoxEntry *pParent = NULL );
};

// class SfxMenuConfigPage -----------------------------------------------

class SfxMenuConfigPage : public SfxTabPage
{
private:
    SfxMenuCfgTabListBox_Impl       aEntriesBox;
    GroupBox                        aMenuGroup;
    PushButton                      aNewButton;
    PushButton                      aNewPopupButton;
    PushButton                      aChangeButton;
    PushButton                      aRemoveButton;
    ImageButton                     aUpButton;
    ImageButton                     aDownButton;
    FixedText                       aGroupText;
    SfxConfigGroupListBox_Impl      aGroupBox;
    FixedText                       aFunctionText;
    SfxConfigFunctionListBox_Impl   aFunctionBox;
    GroupBox                        aFunctionsGroup;
    PushButton                      aLoadButton;
    PushButton                      aSaveButton;
    PushButton                      aResetButton;

    SfxMenuManager*             pMgr;

    BOOL                        bModified;
    BOOL                        bDefault;

    DECL_LINK(                  MoveHdl, Button * );
    DECL_LINK(                  NewHdl, Button * );
    DECL_LINK(                  NewPopupHdl, Button * );
    DECL_LINK(                  ChangeHdl, Button * );
    DECL_LINK(                  RemoveHdl, Button * );
    DECL_LINK(                  SelectHdl, Control* );

    void                        CheckEntry( Control* );
    USHORT                      GetCurId();
    String                      MakeEntry( const SfxMenuConfigEntry &rEntry ) const;
    String                      Trim( const String &rStr ) const;

    void                        Init();
    void                        ResetConfig();

    BOOL                        TryMove_Impl( Button*, SvLBoxEntry** pEntry=NULL, ULONG* pPos=NULL );

    DECL_LINK(                  Default, PushButton * );
    DECL_LINK(                  Save, Button * );
    DECL_LINK(                  Load, Button * );

public:
                                SfxMenuConfigPage( Window *pParent, const SfxItemSet& rItemSet );
    virtual                     ~SfxMenuConfigPage();

    virtual BOOL                FillItemSet( SfxItemSet& );
    virtual void                Reset( const SfxItemSet& );
    void                        Apply();
    void                        SetModified( BOOL bSet ) { bModified = bSet; }
    void                        SetDefault( BOOL bSet ) { bDefault = bSet; }
};


class SfxAcceleratorManager;

// class SfxAcceleratorConfigListBox *************************************************

class SfxAccCfgTabListBox_Impl : public SfxMenuCfgTabListBox_Impl
{
    void                        KeyInput( const KeyEvent &rKEvt );

public:
                                SfxAccCfgTabListBox_Impl( Window *pParent,
                                        const ResId &rResId ) :
                                    SfxMenuCfgTabListBox_Impl( pParent, rResId )
                                {}

    void                        ReplaceEntry( USHORT nPos, const String &rStr );
};

// class SfxAcceleratorConfigPage ----------------------------------------

class SfxAcceleratorConfigPage : public SfxTabPage
{
private:
    SfxAccCfgTabListBox_Impl        aEntriesBox;
    GroupBox                        aKeyboardGroup;
    PushButton                      aChangeButton;
    PushButton                      aRemoveButton;
    FixedText                       aGroupText;
    SfxConfigGroupListBox_Impl      aGroupBox;
    FixedText                       aFunctionText;
    SfxConfigFunctionListBox_Impl   aFunctionBox;
    FixedText                       aKeyText;
    ListBox                         aKeyBox;
    GroupBox                        aFunctionsGroup;
    PushButton                      aLoadButton;
    PushButton                      aSaveButton;
    PushButton                      aResetButton;

    SfxAcceleratorManager*      pMgr;

    USHORTArr                   aAccelArr;
    USHORTArr                   aKeyArr;
    BOOL                        bModified;
    BOOL                        bDefault;

    DECL_LINK(                  ChangeHdl, Button * );
    DECL_LINK(                  RemoveHdl, Button * );
    DECL_LINK(                  SelectHdl, Control * );
    DECL_LINK(                  Save, Button * );
    DECL_LINK(                  Load, Button * );
    DECL_LINK(                  Default, PushButton * );

    KeyCode                     PosToKeyCode   ( USHORT nPos )          const;
    USHORT                      KeyCodeToPos   ( const KeyCode &rCode ) const;
    String                      GetFunctionName( KeyFuncType eType )    const;

    void                        Init();
    void                        ResetConfig();

public:
                                SfxAcceleratorConfigPage( Window *pParent, const SfxItemSet& rItemSet );
    virtual                     ~SfxAcceleratorConfigPage();

    virtual BOOL                FillItemSet( SfxItemSet& );
    virtual void                Reset( const SfxItemSet& );

    void                        SelectMacro(const SfxMacroInfoItem*);
    void                        Apply();
};

class SfxConfigDialog : public SfxTabDialog
{
    const SfxMacroInfoItem*     pMacroInfo;
    USHORT                      nObjectBar;

public:
                                SfxConfigDialog( Window*, const SfxItemSet* );
                                ~SfxConfigDialog();

    void                        ActivateToolBoxConfig( USHORT nId );
    void                        ActivateMacroConfig( const SfxMacroInfoItem* );

    virtual void                PageCreated( USHORT nId, SfxTabPage &rPage );
    virtual short               Ok();

    static String               FileDialog_Impl( Window *pParent, WinBits nBits, const String& rTitle );
    static SfxConfigManager*    MakeCfgMgr_Impl( const String& rName, BOOL& bCreated );
};

class SfxStatusBarManager;
class SfxStbInfoArr_Impl;

class SfxStatusBarConfigListBox : public SvTreeListBox
{
friend class SfxStatusBarConfigPage;
    SvLBoxButtonData*           pButton;
    BOOL                        bModified;
    BOOL                        bDefault;
    SvLBoxEntry*                pCurEntry;
    Timer                       aTimer;

    DECL_LINK(                  TimerHdl, Timer* );

public:

                                SfxStatusBarConfigListBox( Window* pParent,
                                        const ResId& );

    virtual BOOL                NotifyQueryDrop( SvLBoxEntry* pEntry );
    virtual BOOL                NotifyMoving(SvLBoxEntry*,
                                    SvLBoxEntry*,SvLBoxEntry*&,ULONG&);
    virtual void                CheckButtonHdl();
    virtual void                MouseMove( const MouseEvent& rMEvt );
    virtual void                KeyInput( const KeyEvent& rKeyEvent );
};

// class SfxStatusBarConfigPage ------------------------------------------

class SfxStatusBarConfigPage : public SfxTabPage
{
private:
    SfxStatusBarConfigListBox   aEntriesBox;
    GroupBox                    aStatusBarGroup;
    PushButton                  aLoadButton;
    PushButton                  aSaveButton;
    PushButton                  aResetButton;

    SvUShorts                   aListOfIds;
    SfxStatusBarManager*        pMgr;
    SfxStbInfoArr_Impl*         pArr;

    DECL_LINK(                  SelectHdl, SvTreeListBox * );
    DECL_LINK(                  Save, Button * );
    DECL_LINK(                  Load, Button * );
    DECL_LINK(                  Default, PushButton * );

    void                        Init();
    void                        ResetConfig();

public:
                                SfxStatusBarConfigPage( Window *pParent, const SfxItemSet& rItemSet );
    virtual                     ~SfxStatusBarConfigPage();

    virtual BOOL                FillItemSet( SfxItemSet& );
    virtual void                Reset( const SfxItemSet& );
    void                        Apply();
};


#endif


/*************************************************************************
 *
 *  $RCSfile: cfg.hxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 16:49:12 $
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

#include <vector>

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
#define SFX_CFGGROUP_SCRIPTCONTAINER  8
#define SFX_CFGFUNCTION_SCRIPT 9

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

class SfxSlotPool;
class SfxConfigGroupListBox_Impl : public SvTreeListBox
{
    SfxConfigFunctionListBox_Impl*  pFunctionListBox;
    SfxGroupInfoArr_Impl            aArr;
    ULONG                           nMode;
    String                          aScriptType;
    SfxSlotPool*                    pSlotPool;
    BOOL                            bShowSF; // show Scripting Framework scripts
    BOOL                            bShowBasic; // show Basic scripts

protected:
    virtual void                    RequestingChilds( SvLBoxEntry *pEntry);
#if SUPD>=527
    virtual BOOL                    Expand( SvLBoxEntry* pParent );
#endif

public:
                                    SfxConfigGroupListBox_Impl ( Window* pParent,
                                        const ResId&, ULONG nConfigMode = 0 );
                                    ~SfxConfigGroupListBox_Impl();
    void                            ClearAll();

    void                            Init( SvStringsDtor *pArr = 0, SfxSlotPool* pSlotPool = 0 );
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
    String                      aCommand;
    BOOL                        bConfigure;

public:
    SfxMenuConfigEntry( USHORT nInitId, const String& rInitStr,
                        const String& rHelpText, BOOL bPopup = FALSE );
    SfxMenuConfigEntry() : nId( 0 ), bPopUp( FALSE ), bConfigure( TRUE ) {}
    ~SfxMenuConfigEntry();

    USHORT                      GetId() const { return nId; }
    void                        SetId( USHORT nNew );
    void                        SetCommand( const String& rCmd ) { aCommand = rCmd; }
    const String&               GetName() const { return aStr; }
    void                        SetName( const String& rStr ) { aStr = rStr; }
    const String&               GetHelpText() const { return aHelpText; }
    void                        SetHelpText( const String& rStr ) { aHelpText = rStr; }
    void                        SetPopup( BOOL bOn = TRUE ) { bPopUp = bOn; }
    void                        SetConfigurable( BOOL bOn = TRUE ) { bConfigure = bOn; }
    BOOL                        IsBinding() const { return nId != 0 && !bPopUp; }
    BOOL                        IsSeparator() const { return nId == 0; }
    BOOL                        IsPopup() const { return bPopUp; }
    BOOL                        IsConfigurable() const { return bConfigure; }
    const String&               GetCommand() const { return aCommand; }
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
    virtual sal_Int8            AcceptDrop( const AcceptDropEvent& rEvt );
    virtual BOOL                NotifyMoving(SvLBoxEntry*, SvLBoxEntry*, SvLBoxEntry*&, ULONG& );
    void                        Apply( SfxMenuManager* pMgr, SvLBoxEntry *pParent = NULL );
};

// class SfxMenuConfigPage -----------------------------------------------

class SfxMenuConfigPage : public SfxTabPage
{
private:
    SfxMenuCfgTabListBox_Impl       aEntriesBox;
    const SfxMacroInfoItem*         m_pMacroInfoItem;
    FixedLine                        aMenuGroup;
    PushButton                      aNewButton;
    PushButton                      aNewPopupButton;
    PushButton                      aChangeButton;
    PushButton                      aRemoveButton;
    ImageButton                     aUpButton;
    ImageButton                     aDownButton;
    FixedText                       aGroupText;
    SfxConfigGroupListBox_Impl      aGroupLBox;
    FixedText                       aFunctionText;
    SfxConfigFunctionListBox_Impl   aFunctionBox;
    FixedLine                       aFunctionsGroup;
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
    void                        Apply( SfxMenuManager*, BOOL );
    void                        SetModified( BOOL bSet ) { bModified = bSet; }
    void                        SetDefault( BOOL bSet ) { bDefault = bSet; }

    void                        SelectMacro(const SfxMacroInfoItem*);
};


class SfxAcceleratorManager;

// class SfxAcceleratorConfigListBox *************************************************

class SfxAcceleratorConfigPage;
class SfxAccCfgTabListBox_Impl : public SfxMenuCfgTabListBox_Impl
{
    SfxAcceleratorConfigPage*   m_pAccelConfigPage;

    void                        KeyInput( const KeyEvent &rKEvt );

protected:
    virtual void                InitEntry( SvLBoxEntry*, const XubString&, const Image&, const Image& );

public:
                                SfxAccCfgTabListBox_Impl(
                                        SfxAcceleratorConfigPage* pAccelConfigPage,
                                        Window *pParent,
                                        const ResId &rResId ) :
                                    SfxMenuCfgTabListBox_Impl( pParent, rResId ),
                                    m_pAccelConfigPage( pAccelConfigPage )
                                {}

    void                        ReplaceEntry( USHORT nPos, const String &rStr );
};

// class SfxAcceleratorConfigPage ----------------------------------------
struct AccelInfo_Impl
{
    SfxAcceleratorManager*      pMgr;
    SfxAcceleratorManager*      pChanged;
    BOOL                        bDefault;
    BOOL                        bModified;
};

struct AccelBackup
{
    AccelBackup( const KeyCode& rKeyCode, USHORT nSlotId ) :
        aKeyCode( rKeyCode ), nId( nSlotId ) {}

    KeyCode aKeyCode;
    USHORT  nId;
};

class SfxAcceleratorConfigPage : public SfxTabPage
{
    friend SfxAccCfgTabListBox_Impl;
private:
    const SfxMacroInfoItem*         m_pMacroInfoItem;
    SfxAccCfgTabListBox_Impl        aEntriesBox;
    FixedLine                       aKeyboardGroup;
     RadioButton                        aOfficeButton;
    RadioButton                     aModuleButton;
    PushButton                      aChangeButton;
    PushButton                      aRemoveButton;
    FixedText                       aGroupText;
    SfxConfigGroupListBox_Impl      aGroupLBox;
    FixedText                       aFunctionText;
    SfxConfigFunctionListBox_Impl   aFunctionBox;
    FixedText                       aKeyText;
    ListBox                         aKeyBox;
    FixedLine                       aFunctionsGroup;
    PushButton                      aLoadButton;
    PushButton                      aSaveButton;
    PushButton                      aResetButton;
    std::vector< AccelBackup >      m_aAccelBackup;

    USHORTArr                   aConfigCodeArr;
    USHORTArr                   aConfigAccelArr;
    USHORTArr                   aAccelArr;
    USHORTArr                   aKeyArr;

    AccelInfo_Impl*             pGlobal;
    AccelInfo_Impl*             pModule;
    AccelInfo_Impl*             pAct;

    DECL_LINK(                  ChangeHdl, Button * );
    DECL_LINK(                  RemoveHdl, Button * );
    DECL_LINK(                  SelectHdl, Control * );
    DECL_LINK(                  Save, Button * );
    DECL_LINK(                  Load, Button * );
    DECL_LINK(                  Default, PushButton * );
    DECL_LINK(                  RadioHdl, RadioButton* );

    KeyCode                     PosToKeyCode_Config( USHORT nPos ) const;
    USHORT                      KeyCodeToPos_Config( const KeyCode &rCode ) const;

    KeyCode                     PosToKeyCode_All( USHORT nPos ) const;
    USHORT                      KeyCodeToPos_All( const KeyCode& rCode ) const;

    String                      GetFunctionName( KeyFuncType eType )    const;

    void                        Init( SfxAcceleratorManager* pAccMgr );
    void                        ResetConfig();

    void                        CreateCustomItems( SvLBoxEntry* pEntry, const String& aCol1, const String& aCol2 );

public:
                                SfxAcceleratorConfigPage( Window *pParent, const SfxItemSet& rItemSet );
    virtual                     ~SfxAcceleratorConfigPage();

    virtual BOOL                FillItemSet( SfxItemSet& );
    virtual void                Reset( const SfxItemSet& );

    void                        SelectMacro(const SfxMacroInfoItem*);
    void                        Apply( SfxAcceleratorManager*, BOOL );
};

class SfxConfigDialog : public SfxTabDialog
{
    const SfxMacroInfoItem*     pMacroInfo;
    SfxViewFrame*               pViewFrame;
    USHORT                      nObjectBar;

public:
                                SfxConfigDialog( Window*, const SfxItemSet*, SfxViewFrame* pFrame );
                                ~SfxConfigDialog();

    void                        ActivateToolBoxConfig( USHORT nId );
    void                        ActivateMacroConfig( const SfxMacroInfoItem* );
    void                        ActivateTabPage( USHORT );

    virtual void                PageCreated( USHORT nId, SfxTabPage &rPage );
    virtual short               Ok();

    static String               FileDialog_Impl( Window *pParent, WinBits nBits, const String& rTitle );
    static SfxConfigManager*    MakeCfgMgr_Impl( const String& rName, BOOL bWriting, BOOL& bCreated );
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
                                ~SfxStatusBarConfigListBox();

    virtual BOOL                NotifyAcceptDrop( SvLBoxEntry* pEntry );
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
    FixedLine                   aStatusBarGroup;
    PushButton                  aLoadButton;
    PushButton                  aSaveButton;
    PushButton                  aResetButton;

    SvUShorts                   aListOfIds;
    SfxStatusBarManager*        pMgr;
    SfxStbInfoArr_Impl*         pArr;
    BOOL                        bMgrCreated;

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
    void                        Apply( SfxStatusBarManager*, BOOL );
};


#endif


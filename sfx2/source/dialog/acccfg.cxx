/*************************************************************************
 *
 *  $RCSfile: acccfg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: as $ $Date: 2000-11-08 14:25:46 $
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

#ifndef _HELP_HXX //autogen
#include <vcl/help.hxx>
#endif
#ifndef VCL
#ifndef _FILDLG_HXX //autogen
#include <vcl/fildlg.hxx>
#endif
#else
#ifndef _SV_FILEDLG_HXX //autogen
#include <svtools/filedlg.hxx>
#endif
#if SUPD<613//MUSTINI
    #ifndef _SFXINIMGR_HXX //autogen
    #include <svtools/iniman.hxx>
    #endif
#endif
#endif
#pragma hdrstop

#include "cfg.hxx"

#include "dialog.hrc"
#include "cfg.hrc"

#ifdef MAC
#ifndef _EXTATTR_HXX //autogen
#include <svtools/extattr.hxx>
#endif
#endif

#include "app.hxx"
#include "msg.hxx"
#include "msgpool.hxx"
#include "accmgr.hxx"
#include "sfxresid.hxx"
#include "macrconf.hxx"
#include "cfgmgr.hxx"
#include "sfxresid.hxx"
#include "objsh.hxx"
#include "dispatch.hxx"
#include "sfxtypes.hxx"

#ifdef VCL
#include "iodlg.hxx"
#endif

//static const char __FAR_DATA pUnknownStr[]    = "???";
static USHORT __FAR_DATA aCodeArr[] =
{
    KEY_F2       ,
    KEY_F3       ,
    KEY_F4       ,
    KEY_F5       ,
    KEY_F6       ,
    KEY_F7       ,
    KEY_F8       ,
    KEY_F9       ,
    KEY_F10      ,
    KEY_F11      ,
    KEY_F12      ,

    KEY_DOWN     ,
    KEY_UP       ,
    KEY_LEFT     ,
    KEY_RIGHT    ,
    KEY_HOME     ,
    KEY_END      ,
    KEY_PAGEUP   ,
    KEY_PAGEDOWN ,
    KEY_RETURN   ,
    KEY_ESCAPE   ,
    KEY_BACKSPACE,
    KEY_INSERT   ,
    KEY_DELETE   ,

    KEY_F2        | KEY_SHIFT,
    KEY_F3        | KEY_SHIFT,
    KEY_F4        | KEY_SHIFT,
    KEY_F5        | KEY_SHIFT,
    KEY_F6        | KEY_SHIFT,
    KEY_F7        | KEY_SHIFT,
    KEY_F8        | KEY_SHIFT,
    KEY_F9        | KEY_SHIFT,
    KEY_F10       | KEY_SHIFT,
    KEY_F11       | KEY_SHIFT,
    KEY_F12       | KEY_SHIFT,

    KEY_DOWN      | KEY_SHIFT,
    KEY_UP        | KEY_SHIFT,
    KEY_LEFT      | KEY_SHIFT,
    KEY_RIGHT     | KEY_SHIFT,
    KEY_HOME      | KEY_SHIFT,
    KEY_END       | KEY_SHIFT,
    KEY_PAGEUP    | KEY_SHIFT,
    KEY_PAGEDOWN  | KEY_SHIFT,
    KEY_RETURN    | KEY_SHIFT,
    KEY_SPACE     | KEY_SHIFT,
    KEY_ESCAPE    | KEY_SHIFT,
    KEY_BACKSPACE | KEY_SHIFT,
    KEY_INSERT    | KEY_SHIFT,
    KEY_DELETE    | KEY_SHIFT,

    KEY_0         | KEY_MOD1 ,
    KEY_1         | KEY_MOD1 ,
    KEY_2         | KEY_MOD1 ,
    KEY_3         | KEY_MOD1 ,
    KEY_4         | KEY_MOD1 ,
    KEY_5         | KEY_MOD1 ,
    KEY_6         | KEY_MOD1 ,
    KEY_7         | KEY_MOD1 ,
    KEY_8         | KEY_MOD1 ,
    KEY_9         | KEY_MOD1 ,
    KEY_A         | KEY_MOD1 ,
    KEY_B         | KEY_MOD1 ,
    KEY_C         | KEY_MOD1 ,
    KEY_D         | KEY_MOD1 ,
    KEY_E         | KEY_MOD1 ,
    KEY_F         | KEY_MOD1 ,
    KEY_G         | KEY_MOD1 ,
    KEY_H         | KEY_MOD1 ,
    KEY_I         | KEY_MOD1 ,
    KEY_J         | KEY_MOD1 ,
    KEY_K         | KEY_MOD1 ,
    KEY_L         | KEY_MOD1 ,
    KEY_M         | KEY_MOD1 ,
    KEY_N         | KEY_MOD1 ,
    KEY_O         | KEY_MOD1 ,
    KEY_P         | KEY_MOD1 ,
    KEY_Q         | KEY_MOD1 ,
    KEY_R         | KEY_MOD1 ,
    KEY_S         | KEY_MOD1 ,
    KEY_T         | KEY_MOD1 ,
    KEY_U         | KEY_MOD1 ,
    KEY_V         | KEY_MOD1 ,
    KEY_W         | KEY_MOD1 ,
    KEY_X         | KEY_MOD1 ,
    KEY_Y         | KEY_MOD1 ,
    KEY_Z         | KEY_MOD1 ,

    KEY_F2        | KEY_MOD1 ,
    KEY_F3        | KEY_MOD1 ,
    KEY_F4        | KEY_MOD1 ,
    KEY_F5        | KEY_MOD1 ,
    KEY_F6        | KEY_MOD1 ,
    KEY_F7        | KEY_MOD1 ,
    KEY_F8        | KEY_MOD1 ,
    KEY_F9        | KEY_MOD1 ,
    KEY_F10       | KEY_MOD1 ,
    KEY_F11       | KEY_MOD1 ,
    KEY_F12       | KEY_MOD1 ,

    KEY_DOWN      | KEY_MOD1 ,
    KEY_UP        | KEY_MOD1 ,
    KEY_LEFT      | KEY_MOD1 ,
    KEY_RIGHT     | KEY_MOD1 ,
    KEY_HOME      | KEY_MOD1 ,
    KEY_END       | KEY_MOD1 ,
    KEY_PAGEUP    | KEY_MOD1 ,
    KEY_PAGEDOWN  | KEY_MOD1 ,
    KEY_RETURN    | KEY_MOD1 ,
    KEY_SPACE     | KEY_MOD1 ,
    KEY_BACKSPACE | KEY_MOD1 ,
    KEY_INSERT    | KEY_MOD1 ,
    KEY_DELETE    | KEY_MOD1 ,

    KEY_ADD       | KEY_MOD1 ,
    KEY_SUBTRACT  | KEY_MOD1 ,
    KEY_MULTIPLY  | KEY_MOD1 ,
    KEY_DIVIDE    | KEY_MOD1 ,

    KEY_0         | KEY_SHIFT | KEY_MOD1,
    KEY_1         | KEY_SHIFT | KEY_MOD1,
    KEY_2         | KEY_SHIFT | KEY_MOD1,
    KEY_3         | KEY_SHIFT | KEY_MOD1,
    KEY_4         | KEY_SHIFT | KEY_MOD1,
    KEY_5         | KEY_SHIFT | KEY_MOD1,
    KEY_6         | KEY_SHIFT | KEY_MOD1,
    KEY_7         | KEY_SHIFT | KEY_MOD1,
    KEY_8         | KEY_SHIFT | KEY_MOD1,
    KEY_9         | KEY_SHIFT | KEY_MOD1,
    KEY_A         | KEY_SHIFT | KEY_MOD1,
    KEY_B         | KEY_SHIFT | KEY_MOD1,
    KEY_C         | KEY_SHIFT | KEY_MOD1,
    KEY_D         | KEY_SHIFT | KEY_MOD1,
    KEY_E         | KEY_SHIFT | KEY_MOD1,
    KEY_F         | KEY_SHIFT | KEY_MOD1,
    KEY_G         | KEY_SHIFT | KEY_MOD1,
    KEY_H         | KEY_SHIFT | KEY_MOD1,
    KEY_I         | KEY_SHIFT | KEY_MOD1,
    KEY_J         | KEY_SHIFT | KEY_MOD1,
    KEY_K         | KEY_SHIFT | KEY_MOD1,
    KEY_L         | KEY_SHIFT | KEY_MOD1,
    KEY_M         | KEY_SHIFT | KEY_MOD1,
    KEY_N         | KEY_SHIFT | KEY_MOD1,
    KEY_O         | KEY_SHIFT | KEY_MOD1,
    KEY_P         | KEY_SHIFT | KEY_MOD1,
    KEY_Q         | KEY_SHIFT | KEY_MOD1,
    KEY_R         | KEY_SHIFT | KEY_MOD1,
    KEY_S         | KEY_SHIFT | KEY_MOD1,
    KEY_T         | KEY_SHIFT | KEY_MOD1,
    KEY_U         | KEY_SHIFT | KEY_MOD1,
    KEY_V         | KEY_SHIFT | KEY_MOD1,
    KEY_W         | KEY_SHIFT | KEY_MOD1,
    KEY_X         | KEY_SHIFT | KEY_MOD1,
    KEY_Y         | KEY_SHIFT | KEY_MOD1,
    KEY_Z         | KEY_SHIFT | KEY_MOD1,

    KEY_F2        | KEY_SHIFT | KEY_MOD1,
    KEY_F3        | KEY_SHIFT | KEY_MOD1,
    KEY_F4        | KEY_SHIFT | KEY_MOD1,
    KEY_F5        | KEY_SHIFT | KEY_MOD1,
    KEY_F6        | KEY_SHIFT | KEY_MOD1,
    KEY_F7        | KEY_SHIFT | KEY_MOD1,
    KEY_F8        | KEY_SHIFT | KEY_MOD1,
    KEY_F9        | KEY_SHIFT | KEY_MOD1,
    KEY_F10       | KEY_SHIFT | KEY_MOD1,
    KEY_F11       | KEY_SHIFT | KEY_MOD1,
    KEY_F12       | KEY_SHIFT | KEY_MOD1,

    KEY_DOWN      | KEY_SHIFT | KEY_MOD1,
    KEY_UP        | KEY_SHIFT | KEY_MOD1,
    KEY_LEFT      | KEY_SHIFT | KEY_MOD1,
    KEY_RIGHT     | KEY_SHIFT | KEY_MOD1,
    KEY_HOME      | KEY_SHIFT | KEY_MOD1,
    KEY_END       | KEY_SHIFT | KEY_MOD1,
    KEY_PAGEUP    | KEY_SHIFT | KEY_MOD1,
    KEY_PAGEDOWN  | KEY_SHIFT | KEY_MOD1,

    KEY_RETURN    | KEY_SHIFT | KEY_MOD1,
    KEY_SPACE     | KEY_SHIFT | KEY_MOD1,
    KEY_BACKSPACE | KEY_SHIFT | KEY_MOD1,
    KEY_INSERT    | KEY_SHIFT | KEY_MOD1,
    KEY_DELETE    | KEY_SHIFT | KEY_MOD1
};

static long nAccCfgTabs[] =
{
        2,                              // Number of Tabs
        0,
        120                             // Function
};

#define ACC_CODEARRSIZE   ( sizeof( aCodeArr ) / sizeof( aCodeArr[ 0 ] ) )
#ifdef VCL
#define ACC_FUNCTIONCOUNT 0
#else
#define ACC_FUNCTIONCOUNT 18
#endif

#ifdef MSC
#pragma warning (disable:4355)
#endif

/* SfxAcceleratorConfigListBox::KeyInput() *******************************************

Springt den Eintrag an, der der gedrueckten Tastenkombination entspricht.
Ausgenommen davon sind die fuer die Dialogsteuerung ueblichen
Tastenkombinationen.

****************************************************************************/

void SfxAccCfgTabListBox_Impl::KeyInput( const KeyEvent &rKEvt )
{
    KeyCode aCode1 = rKEvt.GetKeyCode();
    USHORT  nCode  = aCode1.GetCode();

    if ( nCode != KEY_DOWN   && nCode != KEY_UP       &&
         nCode != KEY_LEFT   && nCode != KEY_RIGHT    &&
         nCode != KEY_PAGEUP && nCode != KEY_PAGEDOWN )
    {
        for ( USHORT i = 0; i < ACC_CODEARRSIZE; i++ )
        {
            KeyCode aCode2( aCodeArr[ i ] );

            if ( aCode1.GetCode    () == aCode2.GetCode    () &&
                 aCode1.GetModifier() == aCode2.GetModifier() )
            {
                SvLBoxEntry *pEntry = GetEntry( 0, i + ACC_FUNCTIONCOUNT );
                Select( pEntry );
                MakeVisible( pEntry );
                return;
            }
        }
    }

    SfxMenuCfgTabListBox_Impl::KeyInput( rKEvt );
}

SfxAcceleratorConfigPage::~SfxAcceleratorConfigPage()
{
}

// SfxAcceleratorConfigPage::Ctor() ************************************************

SfxAcceleratorConfigPage::SfxAcceleratorConfigPage( Window *pParent, const SfxItemSet& rSet ) :

    SfxTabPage( pParent, SfxResId( TP_CONFIG_ACCEL ), rSet ),

    aChangeButton       ( this, ResId( BTN_ACC_CHANGE          ) ),
    aRemoveButton       ( this, ResId( BTN_ACC_REMOVE          ) ),
    aEntriesBox         ( this, ResId( BOX_ACC_ENTRIES         ) ),
    aKeyboardGroup      ( this, ResId( GRP_ACC_KEYBOARD        ) ),
    aGroupText          ( this, ResId( TXT_ACC_GROUP           ) ),
    aGroupBox           ( this, ResId( BOX_ACC_GROUP ), SFX_SLOT_ACCELCONFIG ),
    aFunctionText       ( this, ResId( TXT_ACC_FUNCTION        ) ),
    aFunctionBox        ( this, ResId( BOX_ACC_FUNCTION        ) ),
    aKeyText            ( this, ResId( TXT_ACC_KEY             ) ),
    aKeyBox             ( this, ResId( BOX_ACC_KEY             ) ),
    aFunctionsGroup     ( this, ResId( GRP_ACC_FUNCTIONS       ) ),
    aLoadButton         ( this, ResId( BTN_LOAD ) ),
    aSaveButton         ( this, ResId( BTN_SAVE ) ),
    aResetButton        ( this, ResId( BTN_RESET   ) ),
    pMgr( SFX_APP()->GetAcceleratorManager() ),
    bModified(FALSE),
    bDefault( pMgr->IsDefault() )
{
    FreeResource();

    // Handler installieren
    aChangeButton.SetClickHdl ( LINK( this, SfxAcceleratorConfigPage,
      ChangeHdl ) );
    aRemoveButton.SetClickHdl ( LINK( this, SfxAcceleratorConfigPage,
      RemoveHdl ) );
    aEntriesBox  .SetSelectHdl( LINK( this, SfxAcceleratorConfigPage,
      SelectHdl ) );
    aGroupBox    .SetSelectHdl( LINK( this, SfxAcceleratorConfigPage,
      SelectHdl ) );
    aFunctionBox .SetSelectHdl( LINK( this, SfxAcceleratorConfigPage,
      SelectHdl ) );
    aKeyBox      .SetSelectHdl( LINK( this, SfxAcceleratorConfigPage,
      SelectHdl ) );
    aLoadButton  .SetClickHdl ( LINK( this, SfxAcceleratorConfigPage,
      Load      ) );
    aSaveButton  .SetClickHdl ( LINK( this, SfxAcceleratorConfigPage,
      Save      ) );
    aResetButton .SetClickHdl ( LINK( this, SfxAcceleratorConfigPage,
      Default      ) );

    // aAccelArr dimensionieren
    for ( USHORT i = 0; i < ACC_FUNCTIONCOUNT + ACC_CODEARRSIZE; i++ )
        aAccelArr.Append( 0 );

    // Entriesbox initialisieren
//(mba)/task    SfxWaitCursor aWait;
    aEntriesBox.SetWindowBits( WB_HSCROLL|WB_CLIPCHILDREN );
    aEntriesBox.SetSelectionMode( SINGLE_SELECTION );
    aEntriesBox.SetTabs( &nAccCfgTabs[0], MAP_APPFONT );
    aEntriesBox.Resize(); // OS: Hack fuer richtige Selektion
//  aEntriesBox.SetFont( SFX_APP()->GetAppFont() );
    aEntriesBox.SetSpaceBetweenEntries( 0 );
    aEntriesBox.SetDragDropMode(0);
    Init();

    aGroupBox.SetFunctionListBox( &aFunctionBox );
    aGroupBox.Init();
    aEntriesBox.Select( aEntriesBox.GetEntry( 0, 0 ) );
    aGroupBox.Select( aGroupBox.GetEntry( 0, 0 ) );
}

void SfxAcceleratorConfigPage::Init()
{
    // aEntriesBox initialisieren
    USHORT i;
    for ( i=0; i<ACC_FUNCTIONCOUNT; i++ )
    {
        // F"ur alle "physikalischen" Acceleratoren einen Eintrag anlegen
        String aEntry = GetFunctionName( ( KeyFuncType ) ( i + 1 ) );
        SfxMenuConfigEntry *pEntry = new SfxMenuConfigEntry( 0, aEntry, String(), FALSE );
        SvLBoxEntry *pLBEntry = aEntriesBox.InsertEntry( aEntry, 0L, LIST_APPEND, 0xFFFF );
        aEntriesBox.EntryInserted( pLBEntry );
        pLBEntry->SetUserData( pEntry );
    }

    for ( i=0; i<ACC_CODEARRSIZE; i++ )
    {
        // F"ur alle "logischen" Acceleratoren einen Eintrag anlegen
        String aEntry = PosToKeyCode( i + ACC_FUNCTIONCOUNT ).GetName();
        SfxMenuConfigEntry *pEntry = new SfxMenuConfigEntry( 0, aEntry, String(), FALSE );
        SvLBoxEntry *pLBEntry = aEntriesBox.InsertEntry( aEntry, 0L, LIST_APPEND, 0xFFFF );
        aEntriesBox.EntryInserted( pLBEntry );
        pLBEntry->SetUserData( pEntry );
    }

#ifdef MBA_PUT_ITEMS
    SvFileStream aFile( "c:\\accel.log", STREAM_STD_WRITE );
#endif

    BOOL bItem = pMgr->FirstItem();
    while ( bItem )
    {
        // F"ur alle Eintr"age, die in der Konfiguration enthalten sind, den Funktionsnamen und Hilfetext holen
        USHORT nPos = KeyCodeToPos( pMgr->GetKeyCode() );
        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            USHORT nCol = aEntriesBox.TabCount() - 1;
            String aText ('[');
            aText += SFX_SLOTPOOL().GetSlotName_Impl( pMgr->GetId() );
            aText += ']';
            aEntriesBox.SetEntryText( aText, nPos, nCol );
            aAccelArr[ nPos ] = pMgr->GetId();
            SfxMenuConfigEntry *pEntry = (SfxMenuConfigEntry*) aEntriesBox.GetEntry( 0, nPos )->GetUserData();
            pEntry->SetId( pMgr->GetId() );

#ifdef MBA_PUT_ITEMS
            KeyCode aCode = pMgr->GetKeyCode();
            String aName;
            if ( !aCode.IsFunction() )
                aName = aCode.GetName();
            else
                aName = GetFunctionName( aCode.GetFunction() );
            aFile << aText.GetStr() << '\t' << aName.GetStr() << '\n';
#endif
        }

        bItem = pMgr->NextItem();
    }

//  for ( i=0; i<ACC_FUNCTIONCOUNT; i++ )
//      aEntriesBox.GetModel()->Remove( aEntriesBox.FirstChild( 0 ) );
}

void SfxAcceleratorConfigPage::ResetConfig()
{
    aEntriesBox.Clear();
    for ( USHORT i = 0; i < ACC_FUNCTIONCOUNT + ACC_CODEARRSIZE; i++ )
        aAccelArr[i] = 0;
}

void SfxAcceleratorConfigPage::Apply()
{
    if (!bModified)
        return;

    if ( bDefault )
    {
        pMgr->UseDefault();
        pMgr->SetDefault(TRUE);
        bModified = FALSE;
        return;
    }

    bModified = FALSE;

//(mba)/task    SfxWaitCursor aWait;
    SvUShorts aListOfIds;

    // zaehlen
    USHORT nCount = 0;
    USHORT i;
    for ( i = ACC_FUNCTIONCOUNT + ACC_CODEARRSIZE; i > 0; --i )
    {
        if ( aAccelArr[i-1] )
            ++nCount;
    }

    BOOL bItem = pMgr->FirstItem();
    while ( bItem )
    {
        // Macro-Eintraege referenzieren, da sie sonst beim Clear eventuell
        // entfernt werden koennten !

        if ( SfxMacroConfig::IsMacroSlot( pMgr->GetId() ) )
            aListOfIds.Insert(pMgr->GetId(), aListOfIds.Count());
        bItem = pMgr->NextItem();
    }

    pMgr->Reset(nCount);

    // Liste von hinten durchgehen, damit logische Acceleratoren Vorrang
    // vor physikalischen haben.
    for ( i = ACC_FUNCTIONCOUNT + ACC_CODEARRSIZE; i > 0; --i )
    {
        if ( aAccelArr[i-1] )
        {
            if ( SfxMacroConfig::IsMacroSlot( aAccelArr[i-1] ) )
            {
                USHORT nPos;
                for (nPos=0; nPos<aListOfIds.Count(); nPos++)
                    if (aListOfIds[nPos] == aAccelArr[i-1])
                        break;
                if (nPos < aListOfIds.Count())
                    aListOfIds.Remove(nPos);
                else
                    SFX_APP()->GetMacroConfig()->RegisterSlotId(aAccelArr[i-1]);
            }

            pMgr->AppendItem( aAccelArr[i-1], PosToKeyCode( i-1 ) );
        }
    }

    for (i=0; i<aListOfIds.Count(); i++)
        SFX_APP()->GetMacroConfig()->ReleaseSlotId(aListOfIds[i]);

    pMgr->Reconfigure();
}

IMPL_LINK( SfxAcceleratorConfigPage, Load, Button *, pButton )
{
    String aCfgName = SfxConfigDialog::FileDialog_Impl( this,
        WB_OPEN | WB_STDMODAL | WB_3DLOOK, String( SfxResId( STR_LOADACCELCONFIG) ) );
    if ( aCfgName.Len() )
    {
//(mba)/task        SfxWaitCursor aWait;

        BOOL bCreated = FALSE;
        SfxConfigManager *pCfgMgr =
            SfxConfigDialog::MakeCfgMgr_Impl( aCfgName, bCreated );
        SfxConfigManager *pOldCfgMgr = pMgr->GetConfigManager_Impl();
        if (pOldCfgMgr != pCfgMgr)
        {
            if (pCfgMgr->HasConfigItem(pMgr->GetType()))
            {
                pMgr->ReInitialize(pCfgMgr);
                aEntriesBox.SetUpdateMode(FALSE);
                ResetConfig();
                Init();
                aEntriesBox.SetUpdateMode(TRUE);
                aEntriesBox.Invalidate();
                aEntriesBox.Select( aEntriesBox.GetEntry( 0, 0 ) );
                pMgr->ReInitialize(pOldCfgMgr);
                bDefault = FALSE;
                bModified = TRUE;
            }
        }

        if ( bCreated )
            delete pCfgMgr;
    }

    return 0;
}

IMPL_LINK( SfxAcceleratorConfigPage, Save, Button *, pButton )
{
    String aCfgName = SfxConfigDialog::FileDialog_Impl( this,
        WB_SAVEAS | WB_STDMODAL | WB_3DLOOK, String( SfxResId( STR_SAVEACCELCONFIG) ) );
    if ( aCfgName.Len() )
    {
//(mba)/task        SfxWaitCursor aWait;

        BOOL bCreated = FALSE;
        SfxConfigManager *pCfgMgr =
            SfxConfigDialog::MakeCfgMgr_Impl( aCfgName, bCreated );

        if ( pCfgMgr == SfxObjectShell::Current()->GetConfigManager() )
        {
            pCfgMgr->Activate(SFX_CFGMANAGER());
            pMgr->Connect(pCfgMgr);
            pCfgMgr->AddConfigItem(pMgr);
        }

        SfxConfigManager *pOldCfgMgr = pMgr->GetConfigManager_Impl();
        if (pOldCfgMgr != pCfgMgr)
        {
            pMgr->StoreConfig();
            pMgr->Connect(pCfgMgr);
            pCfgMgr->AddConfigItem(pMgr);
            BOOL bMod = bModified;
            BOOL bItemModified = pMgr->IsModified();
            Apply();
            pMgr->SetModified( TRUE );
            pCfgMgr->StoreConfig();
            pMgr->ReInitialize(pOldCfgMgr);
            pMgr->SetModified( bItemModified );
            bModified = bMod;

            if (bCreated)
            {
                pCfgMgr->SetModified( TRUE );
                pCfgMgr->SaveConfig();
#ifdef MAC
                SvEaMgr aEaMgr(aCfgName);
                aEaMgr.SetFileType("Pref");
#endif
            }

        }
        else
        {
            Apply();
            pMgr->SetModified( TRUE );
            pCfgMgr->StoreConfig();
        }

        if (bCreated)
            delete pCfgMgr;
    }

    return 0;
}

IMPL_LINK( SfxAcceleratorConfigPage, Default, PushButton *, pPushButton )
{
    SfxConfigManager *pOldCfgMgr = pMgr->GetConfigManager_Impl();
    bDefault = TRUE;
    bModified = !pMgr->IsDefault();
    pMgr->StoreConfig();
    pMgr->ReleaseConfigManager();
    pMgr->UseDefault();
    aEntriesBox.SetUpdateMode(FALSE);
    ResetConfig();
    Init();
    aEntriesBox.SetUpdateMode(TRUE);
    aEntriesBox.Invalidate();
    aEntriesBox.Select( aEntriesBox.GetEntry( 0, 0 ) );
    pMgr->ReInitialize(pOldCfgMgr);
    return 0;
}

IMPL_LINK( SfxAcceleratorConfigPage, ChangeHdl, Button *, pButton )
{
    bDefault = FALSE;
    bModified = TRUE;

    // Selektierter Eintrag und selektierte Funktion
    USHORT nPos = (USHORT) aEntriesBox.GetModel()->GetRelPos( aEntriesBox.FirstSelected() );
    USHORT nId  = aFunctionBox.GetCurId();

    String aStr;
    if ( SfxMacroConfig::IsMacroSlot( nId ) )
    {
        // Es ist ein Macro selektiert, f"ur das schon eine SlotId reserviert wurde
        aStr = SFX_APP()->GetMacroConfig()->GetMacroInfo(nId)->GetMacroName();
    }
    else
    {
        // Eine normale Funktion ist selektiert
        aStr = SFX_SLOTPOOL().GetSlotName_Impl( nId );
    }

    // Hilfetext setzen
    SfxMenuConfigEntry *pEntry = (SfxMenuConfigEntry*) aEntriesBox.GetEntry( 0, nPos )->GetUserData();

    // Funktions/Macronamen im Eintrag updaten
    String aText ('[');
    aText += aStr;
    aText += ']';
    USHORT nCol = aEntriesBox.TabCount() - 1;
    aEntriesBox.SetEntryText( aText, nPos, nCol );
    aAccelArr[ nPos ] = nId;
    pEntry->SetId( nId );
    pEntry->SetHelpText( String() );
    ((Link &) aFunctionBox.GetSelectHdl()).Call( &aFunctionBox );
    return 0;
}

IMPL_LINK( SfxAcceleratorConfigPage, RemoveHdl, Button *, pButton )
{
    bDefault = FALSE;
    bModified = TRUE;

    // Selektierter Eintrag
    USHORT nPos = (USHORT) aEntriesBox.GetModel()->GetRelPos( aEntriesBox.FirstSelected() );
    SfxMenuConfigEntry *pEntry = (SfxMenuConfigEntry*) aEntriesBox.GetEntry( 0, nPos )->GetUserData();
    pEntry->SetHelpText( String() );

    // Funktionsnamen aus dem Eintrag l"oschen
    USHORT nCol = aEntriesBox.TabCount() - 1;
    aEntriesBox.SetEntryText( pEntry->GetName(), nPos );  // Nur letzte Spalte auf "" setzen funzt nicht - Bug ??
    aAccelArr[ nPos ] = 0;
    pEntry->SetId( 0 );
    ((Link &) aFunctionBox.GetSelectHdl()).Call( &aFunctionBox );
    return 0;
}

IMPL_LINK( SfxAcceleratorConfigPage, SelectHdl, Control*, pListBox )
{
    // Alle Hilfetexte abschalten
    Help::ShowBalloon( this, Point(), String() );

    if ( pListBox == &aEntriesBox )
    {
        // Eintrag ausgewaehlt: Buttons enablen/disablen
        USHORT nPos = (USHORT) aEntriesBox.GetModel()->GetRelPos( aEntriesBox.FirstSelected() );
        aChangeButton.Enable( nPos >= ACC_FUNCTIONCOUNT && aAccelArr[ nPos ] != aFunctionBox.GetCurId() );
        aRemoveButton.Enable( nPos >= ACC_FUNCTIONCOUNT && aAccelArr[ nPos ] > 0 );
    }
    else if ( pListBox == &aGroupBox )
    {
        aGroupBox.GroupSelected();
        if ( !aFunctionBox.FirstSelected() )
            aChangeButton.Enable( FALSE );
    }
    else if ( pListBox == &aFunctionBox )
    {
        aFunctionBox.FunctionSelected();

        // Zuerst "uberpr"ufen, ob durch den Wechsel der Selektion der Zustand des ChangeButtons wechselt
        USHORT nEntryPos = (USHORT) aEntriesBox.GetModel()->GetRelPos( aEntriesBox.FirstSelected() );
        USHORT nId = aFunctionBox.GetCurId();
        aChangeButton.Enable( nEntryPos >= ACC_FUNCTIONCOUNT && aAccelArr[ nEntryPos ] != nId );
        aRemoveButton.Enable( nEntryPos >= ACC_FUNCTIONCOUNT && aAccelArr[ nEntryPos ] > 0 );

        aKeyBox.Clear();
        aKeyArr.Clear();

        for ( USHORT i = 0; i < ACC_FUNCTIONCOUNT + ACC_CODEARRSIZE; i++ )
        {
            if ( aAccelArr[ i ] == nId )
            {
                KeyCode aCode = PosToKeyCode( i );
                if ( !aCode.IsFunction() )
                    aKeyBox.InsertEntry( aCode.GetName() );
                else
                    aKeyBox.InsertEntry( GetFunctionName( aCode.GetFunction() ) );

                aKeyArr.Append( i );
            }
        }
    }
    else
    {
        // Taste ausgewaehlt: Eintrag anspringen
        USHORT n = aKeyBox.GetSelectEntryPos();
        USHORT nPos = aKeyArr[ n ] /* - ACC_FUNCTIONCOUNT */;
        SvLBoxEntry *pEntry = aEntriesBox.GetEntry( 0, nPos );
        aEntriesBox.Select( pEntry );
        aEntriesBox.MakeVisible( pEntry );
    }
    return 0;
}

KeyCode SfxAcceleratorConfigPage::PosToKeyCode( USHORT nPos ) const
{
    DBG_ASSERT( nPos < ACC_CODEARRSIZE + ACC_FUNCTIONCOUNT, "Invalid position!" );

      if ( nPos < ACC_FUNCTIONCOUNT )
      {
          KeyCode aTmpCode( ( KeyFuncType ) ( nPos + 1 ) );
          return aTmpCode;
      }
      else
      {
        KeyCode aTmpCode( aCodeArr[ nPos - ACC_FUNCTIONCOUNT] & 0xFFF,
                          aCodeArr[ nPos - ACC_FUNCTIONCOUNT] & ( KEY_SHIFT | KEY_MOD1 ) );
        return aTmpCode;
      }
}

USHORT SfxAcceleratorConfigPage::KeyCodeToPos( const KeyCode &rCode ) const
{
      if ( rCode.IsFunction() )
          return ( USHORT ) rCode.GetFunction() - 1;
      else
      {
        USHORT nCode = rCode.GetCode() + rCode.GetModifier();

        for ( USHORT i = 0; i < ACC_CODEARRSIZE; i++ )
        {
            if ( aCodeArr[ i ] == nCode )
                return i + ACC_FUNCTIONCOUNT;
        }

        return LISTBOX_ENTRY_NOTFOUND;
      }
}

String SfxAcceleratorConfigPage::GetFunctionName( KeyFuncType eType ) const
{
    String aStr;

    switch ( eType )
    {
        case KEYFUNC_NEW         : aStr = String( SfxResId( STR_NEW          ) ); break;
        case KEYFUNC_OPEN        : aStr = String( SfxResId( STR_OPEN         ) ); break;
        case KEYFUNC_SAVE        : aStr = String( SfxResId( STR_SAVE         ) ); break;
        case KEYFUNC_SAVEAS      : aStr = String( SfxResId( STR_SAVEAS       ) ); break;
        case KEYFUNC_PRINT       : aStr = String( SfxResId( STR_PRINT        ) ); break;
        case KEYFUNC_CLOSE       : aStr = String( SfxResId( STR_CLOSE        ) ); break;
        case KEYFUNC_QUIT        : aStr = String( SfxResId( STR_QUIT         ) ); break;
        case KEYFUNC_CUT         : aStr = String( SfxResId( STR_CUT          ) ); break;
        case KEYFUNC_COPY        : aStr = String( SfxResId( STR_COPY         ) ); break;
        case KEYFUNC_PASTE       : aStr = String( SfxResId( STR_PASTE        ) ); break;
        case KEYFUNC_UNDO        : aStr = String( SfxResId( STR_UNDO         ) ); break;
        case KEYFUNC_REDO        : aStr = String( SfxResId( STR_REDO         ) ); break;
        case KEYFUNC_DELETE      : aStr = String( SfxResId( STR_DELETE       ) ); break;
        case KEYFUNC_REPEAT      : aStr = String( SfxResId( STR_REPEAT       ) ); break;
        case KEYFUNC_FIND        : aStr = String( SfxResId( STR_FIND         ) ); break;
        case KEYFUNC_FINDBACKWARD: aStr = String( SfxResId( STR_FINDBACKWARD ) ); break;
        case KEYFUNC_PROPERTIES  : aStr = String( SfxResId( STR_PROPERTIES   ) ); break;
        case KEYFUNC_FRONT       : aStr = String( SfxResId( STR_FRONT        ) ); break;

        default: DBG_ERROR( "Invalid KeyFuncType!" );
    }

    aStr = String('\"').Append(aStr).Append('\"');
    return aStr;
}

void SfxAcceleratorConfigPage::SelectMacro(const SfxMacroInfoItem *pItem)
{
    aGroupBox.SelectMacro( pItem );
}



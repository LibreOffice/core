/*************************************************************************
 *
 *  $RCSfile: cfg.cxx,v $
 *
 *  $Revision: 1.3 $
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

#ifndef _SV_HELP_HXX //autogen
#include <vcl/help.hxx>
#endif
#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif
#ifndef __SBX_SBXMETHOD_HXX //autogen
#include <svtools/sbxmeth.hxx>
#endif
#ifndef _SB_SBMOD_HXX //autogen
#include <basic/sbmod.hxx>
#endif
#ifndef _BASMGR_HXX //autogen
#include <basic/basmgr.hxx>
#endif
#if SUPD<613//MUSTINI
    #ifndef _SFXINIMGR_HXX //autogen
    #include <svtools/iniman.hxx>
    #endif
#endif
#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#include <tools/urlobj.hxx>
#include <svtools/pathoptions.hxx>
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
#include "appdata.hxx"
#include "msg.hxx"
#include "msgpool.hxx"
#include "stbmgr.hxx"
#include "sfxresid.hxx"
#include "macrconf.hxx"
#include "minfitem.hxx"
#include "cfgmgr.hxx"
#include "sfxresid.hxx"
#include "objsh.hxx"
#include "dispatch.hxx"
#include "sfxtypes.hxx"
#include "eventdlg.hxx"
#include "tbxopdlg.hxx"
#include "minfitem.hxx"
#include "iodlg.hxx"
#if SUPD<613//MUSTINI
#include "inimgr.hxx"
#endif
#include "viewfrm.hxx"
#include "workwin.hxx"

#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>

struct SfxStatBarInfo_Impl
{
    USHORT          nId;
    String          aHelpText;

    SfxStatBarInfo_Impl(USHORT nID, String aString) :
        nId(nID),
        aHelpText(aString)
    {}
};

DECL_PTRARRAY(SfxStbInfoArr_Impl, SfxStatBarInfo_Impl*, 20, 4);
SV_IMPL_PTRARR(SfxGroupInfoArr_Impl, SfxGroupInfoPtr);

SfxMenuConfigEntry::SfxMenuConfigEntry( USHORT nInitId, const String& rInitStr,
                                    const String& rHelpText, BOOL bPopup )
    : nId( nInitId )
    , bPopUp(bPopup)
    , aHelpText(rHelpText)
    , aStr(rInitStr)
{
    if ( SfxMacroConfig::IsMacroSlot( nId ) )
        SFX_APP()->GetMacroConfig()->RegisterSlotId( nId );
}

void SfxMenuConfigEntry::SetId( USHORT nNew )
{
    if ( SfxMacroConfig::IsMacroSlot( nId ) )
        SFX_APP()->GetMacroConfig()->ReleaseSlotId( nId );
    nId = nNew;
    if ( SfxMacroConfig::IsMacroSlot( nId ) )
        SFX_APP()->GetMacroConfig()->RegisterSlotId( nId );
}

SfxMenuConfigEntry::~SfxMenuConfigEntry()
{
    if ( SfxMacroConfig::IsMacroSlot( nId ) )
        SFX_APP()->GetMacroConfig()->ReleaseSlotId( nId );
}

SfxConfigFunctionListBox_Impl::SfxConfigFunctionListBox_Impl( Window* pParent, const ResId& rResId)
/*  Beschreibung
    Diese Listbox arbeiteitet mit der Grouplistbox zusammen und zeigt die zur Konfigurierung verf"ugbaren
    Funktionen bzw. Macros an, die zur selektierten Gruppe bzw. zum selektierten Modul in der Grouplistbox
    geh"oren.
*/
    : SvTreeListBox( pParent, rResId )
    , pCurEntry( 0 )
{
    SetWindowBits( GetStyle() | WB_CLIPCHILDREN | WB_HSCROLL | WB_SORT );
    GetModel()->SetSortMode( SortAscending );

    // Timer f"ur die BallonHelp
    aTimer.SetTimeout( 200 );
    aTimer.SetTimeoutHdl(
        LINK( this, SfxConfigFunctionListBox_Impl, TimerHdl ) );
}

SfxConfigFunctionListBox_Impl::~SfxConfigFunctionListBox_Impl()
{
    ClearAll();
}

void SfxConfigFunctionListBox_Impl::MouseMove( const MouseEvent& rMEvt )
/*  Beschreibung
    Virtuelle Methode, die gerufen wird, wenn der Mauszeiger "uber der TreeListBox bewegt wurde.
    Wenn die Position des Mauszeigers "uber dem aktuell selektierten Entry liegt, wird ein Timer
    aufgesetzt, um ggf. einen Hilfetext einzublenden.
*/
{
    Point aMousePos = rMEvt.GetPosPixel();
    pCurEntry = GetCurEntry();

    if ( pCurEntry && GetEntry( aMousePos ) == pCurEntry )
        aTimer.Start();
    else
    {
        Help::ShowBalloon( this, aMousePos, String() );
        aTimer.Stop();
    }
}


IMPL_LINK( SfxConfigFunctionListBox_Impl, TimerHdl, Timer*, pTimer)
/*  Beschreibung
    Timer-Handler f"ur die Einblendung eines Hilfetextes. Wenn nach Ablauf des Timers
    der Mauszeiger immer noch auf dem aktuell selektierten Eintrag steht, wird der
    Helptext des Entries als Balloon-Help eingeblendet.
*/
{
    aTimer.Stop();
    Point aMousePos = GetPointerPosPixel();
    SvLBoxEntry *pEntry = GetCurEntry();
    if ( pEntry && GetEntry( aMousePos ) == pEntry && pCurEntry == pEntry )
        Help::ShowBalloon( this, OutputToScreenPixel( aMousePos ), GetHelpText( pEntry ) );
    return 0L;
}

void SfxConfigFunctionListBox_Impl::ClearAll()
/*  Beschreibung
    L"oscht alle Eintr"age in der FunctionListBox, alle UserDaten und alle evtl.
    vorhandenen MacroInfos.
*/
{
    USHORT nCount = aArr.Count();
    for ( USHORT i=0; i<nCount; i++ )
    {
        SfxGroupInfo_Impl *pData = aArr[i];
        if ( pData->nKind == SFX_CFGFUNCTION_MACRO )
        {
            SfxMacroInfo *pInfo = (SfxMacroInfo*) pData->pObject;
            SFX_APP()->GetMacroConfig()->ReleaseSlotId( pInfo->GetSlotId() );
            delete pInfo;
        }
        delete pData;
    }

    aArr.Remove( 0, nCount );
    Clear();
}

SvLBoxEntry* SfxConfigFunctionListBox_Impl::GetEntry_Impl( const String& rName )
/*  Beschreibung
    Ermittelt den SvLBoxEntry zu einem "ubergebenen String. Das setzt voraus, da\s
    die Namen eindeutig sind.
*/
{
    SvLBoxEntry *pEntry = First();
    while ( pEntry )
    {
        if ( GetEntryText( pEntry ) == rName )
            return pEntry;
        pEntry = Next( pEntry );
    }

    return NULL;
}

SvLBoxEntry* SfxConfigFunctionListBox_Impl::GetEntry_Impl( USHORT nId )
/*  Beschreibung
    Ermittelt den SvLBoxEntry zu einer "ubergebenen Id.
*/
{
    SvLBoxEntry *pEntry = First();
    while ( pEntry )
    {
        SfxGroupInfo_Impl *pData = (SfxGroupInfo_Impl*) pEntry->GetUserData();
        if ( pData && pData->nOrd == nId )
            return pEntry;
        pEntry = Next( pEntry );
    }

    return NULL;
}

SfxMacroInfo* SfxConfigFunctionListBox_Impl::GetMacroInfo()
/*  Beschreibung
    Gibt die MacroInfo des selektierten Entry zur"uck ( sofern vorhanden ).
*/
{
    SvLBoxEntry *pEntry = FirstSelected();
    if ( pEntry )
    {
        SfxGroupInfo_Impl *pData = (SfxGroupInfo_Impl*) pEntry->GetUserData();
        if ( pData && pData->nKind == SFX_CFGFUNCTION_MACRO )
            return (SfxMacroInfo*) pData->pObject;
    }

    return 0;
}

USHORT SfxConfigFunctionListBox_Impl::GetId( SvLBoxEntry *pEntry )
/*  Beschreibung
    Gibt die Ordnungsnummer ( SlotId oder Macro-Nummer ) des Eintrags zur"uck.
*/
{
    SfxGroupInfo_Impl *pData = pEntry ?
        (SfxGroupInfo_Impl*) pEntry->GetUserData() : 0;
    if ( pData )
        return pData->nOrd;
    return 0;
}

String SfxConfigFunctionListBox_Impl::GetHelpText( SvLBoxEntry *pEntry )
/*  Beschreibung
    Gibt den Helptext des selektierten Entry zur"uck.
*/
{
    // Information zum selektierten Entry aus den Userdaten holen
    SfxGroupInfo_Impl *pInfo = pEntry ? (SfxGroupInfo_Impl*) pEntry->GetUserData(): 0;
    if ( pInfo )
    {
        if ( pInfo->nKind == SFX_CFGFUNCTION_SLOT )
        {
            // Eintrag ist eine Funktion, Hilfe aus der Office-Hilfe
            USHORT nId = pInfo->nOrd;
            String aText = Application::GetHelp()->GetHelpText( nId );
            if ( !aText.Len() )
                aText = SFX_SLOTPOOL().GetSlotHelpText_Impl( nId );
            return aText;
        }
        else
        {
            // Eintrag ist ein Macro, Hilfe aus der MacroInfo
            SfxMacroInfo *pMacInfo = (SfxMacroInfo*) pInfo->pObject;
            return pMacInfo->GetHelpText();
        }
    }

    return String();
}

void SfxConfigFunctionListBox_Impl::FunctionSelected()
/*  Beschreibung
    Setzt die Balloonhelp zur"uck, da diese immer den Helptext des selektierten
    Entry anzeigen soll.
*/
{
    Help::ShowBalloon( this, Point(), String() );
}

SfxConfigGroupListBox_Impl::SfxConfigGroupListBox_Impl(
    Window* pParent, const ResId& rResId, ULONG nConfigMode )
        : SvTreeListBox( pParent, rResId )
        , aScriptType( DEFINE_CONST_UNICODE("StarBasic") )
        , nMode( nConfigMode )
/*  Beschreibung
    Diese Listbox zeigt alle Funktionsgruppen und Basics an, die zur Konfiguration
    zur Verf"ugung stehen. Basics werden noch in Bibliotheken und Module untergliedert.
*/
{
    SetWindowBits( GetStyle() | WB_CLIPCHILDREN | WB_HSCROLL | WB_HASBUTTONS | WB_HASLINES | WB_HASLINESATROOT | WB_HASBUTTONSATROOT );
    SetNodeBitmaps( Image( BMP_COLLAPSED ), Image( BMP_EXPANDED ) );
}


SfxConfigGroupListBox_Impl::~SfxConfigGroupListBox_Impl()
{
}

void SfxConfigGroupListBox_Impl::SetScriptType( const String& rScriptType )
{
    aScriptType = rScriptType;
    ULONG nPos=0;
    SvLBoxEntry *pEntry = (SvLBoxEntry*) GetModel()->GetEntryAtAbsPos( nPos++ );
    while ( pEntry )
    {
        SfxGroupInfo_Impl *pInfo = (SfxGroupInfo_Impl*) pEntry->GetUserData();
        if ( pInfo->nKind == SFX_CFGGROUP_BASICLIB && ( IsExpanded( pEntry ) || pInfo->bWasOpened ) )
        {
            Collapse( pEntry );
            SvLBoxEntry *pChild = FirstChild( pEntry );
            while (pChild)
            {
                GetModel()->Remove( pChild );
                pChild = FirstChild( pEntry );
            }

            Expand( pEntry );
        }

        pEntry = (SvLBoxEntry*) GetModel()->GetEntryAtAbsPos( nPos++ );
    }
}

String SfxConfigGroupListBox_Impl::GetGroup()
/*  Beschreibung
    Gibt den Namen der selektierten Funktionsgruppe bzw. des selektierten
    Basics zur"uck.
*/
{
    SvLBoxEntry *pEntry = FirstSelected();
    while ( pEntry )
    {
        SfxGroupInfo_Impl *pInfo = (SfxGroupInfo_Impl*) pEntry->GetUserData();
        if ( pInfo->nKind == SFX_CFGGROUP_FUNCTION )
        {
            return GetEntryText( pEntry );
            break;
        }

        if ( pInfo->nKind == SFX_CFGGROUP_BASICMGR )
        {
            BasicManager *pMgr = (BasicManager*) pInfo->pObject;
            return pMgr->GetName();
            break;
        }

        if ( pInfo->nKind == SFX_CFGGROUP_DOCBASICMGR )
        {
            SfxObjectShell *pDoc = (SfxObjectShell*) pInfo->pObject;
            return pDoc->GetTitle();
            break;
        }

        pEntry = GetParent( pEntry );
    }

    return String();
}

void SfxConfigGroupListBox_Impl::Init( SvStringsDtor *pArr )
/*  Beschreibung
    Die Groupbox wird mit allen Funktionsgruppen und allen Basics gef"ullt
*/
{
    SetUpdateMode(FALSE);
    SfxApplication *pSfxApp = SFX_APP();

    // Verwendet wird der aktuelle Slotpool
    if ( nMode )
    {
        for ( USHORT i=1; i<SFX_SLOTPOOL().GetGroupCount(); i++ )
        {
            // Gruppe anw"ahlen ( Gruppe 0 ist intern )
            String aName = pSfxApp->GetSlotPool().SeekGroup( i );
            const SfxSlot *pSfxSlot = SFX_SLOTPOOL().FirstSlot();
            if ( pSfxSlot )
            {
                // Wenn Gruppe nicht leer
                SvLBoxEntry *pEntry = InsertEntry( aName, NULL );
                SfxGroupInfo_Impl *pInfo = new SfxGroupInfo_Impl( SFX_CFGGROUP_FUNCTION, i );
                aArr.Insert( pInfo, aArr.Count() );
                pEntry->SetUserData( pInfo );
            }
        }
    }

    // Basics einsammeln
    pSfxApp->EnterBasicCall();
    String aMacroName(' ');
    aMacroName += String(SfxResId(STR_BASICMACROS));

    // Zuerst AppBasic
    BasicManager *pAppBasicMgr = pSfxApp->GetBasicManager();
    BOOL bInsert = TRUE;
    if ( pArr )
    {
        bInsert = FALSE;
        for ( USHORT n=0; n<pArr->Count(); n++ )
        {
            if ( *(*pArr)[n] == pSfxApp->GetName() )
            {
                bInsert = TRUE;
                break;
            }
        }
    }

    if ( bInsert )
    {
        pAppBasicMgr->SetName( pSfxApp->GetName() );
        if ( pAppBasicMgr->GetLibCount() )
        {
            // Nur einf"ugen, wenn Bibliotheken vorhanden
            String aAppBasTitle( SfxResId( STR_HUMAN_APPNAME ) );
            aAppBasTitle += aMacroName;
            SvLBoxEntry *pEntry = InsertEntry( aAppBasTitle, 0 );
            SfxGroupInfo_Impl *pInfo = new SfxGroupInfo_Impl( SFX_CFGGROUP_BASICMGR, 0, pAppBasicMgr );
            aArr.Insert( pInfo, aArr.Count() );
            pEntry->SetUserData( pInfo );
            pEntry->EnableChildsOnDemand( TRUE );
//          Expand( pEntry );
        }
    }

    SfxObjectShell *pDoc = SfxObjectShell::GetFirst();
    while ( pDoc )
    {
        BOOL bInsert = TRUE;
        if ( pArr )
        {
            bInsert = FALSE;
            for ( USHORT n=0; n<pArr->Count(); n++ )
            {
                if ( *(*pArr)[n] == pDoc->GetTitle() )
                {
                    bInsert = TRUE;
                    break;
                }
            }
        }

        if ( bInsert )
        {
            BasicManager *pBasicMgr = pDoc->GetBasicManager();
            if ( pBasicMgr != pAppBasicMgr && pBasicMgr->GetLibCount() )
            {
                pBasicMgr->SetName( pDoc->GetTitle() );

                // Nur einf"ugen, wenn eigenes Basic mit Bibliotheken
                SvLBoxEntry *pEntry = InsertEntry( pDoc->GetTitle().Append(aMacroName), NULL );
                SfxGroupInfo_Impl *pInfo =
                    new SfxGroupInfo_Impl( SFX_CFGGROUP_DOCBASICMGR, 0, pDoc );
                aArr.Insert( pInfo, aArr.Count() );
                pEntry->SetUserData( pInfo );
                pEntry->EnableChildsOnDemand( TRUE );
//              Expand( pEntry );
            }
        }

        pDoc = SfxObjectShell::GetNext(*pDoc);
    }

    pSfxApp->LeaveBasicCall();
    MakeVisible( GetEntry( 0,0 ) );
    SetUpdateMode( TRUE );
}

void SfxConfigGroupListBox_Impl::GroupSelected()
/*  Beschreibung
    Eine Funktionsgruppe oder eine Basicmodul wurde selektiert. Alle Funktionen bzw.
    Macros werden in der Functionlistbox anzeigt.
*/
{
    SvLBoxEntry *pEntry = FirstSelected();
    SfxGroupInfo_Impl *pInfo = (SfxGroupInfo_Impl*) pEntry->GetUserData();
    pFunctionListBox->SetUpdateMode(FALSE);
    pFunctionListBox->ClearAll();
    if ( pInfo->nKind != SFX_CFGGROUP_FUNCTION &&
         pInfo->nKind != SFX_CFGGROUP_BASICMOD )
    {
        pFunctionListBox->SetUpdateMode(TRUE);
        return;
    }

    switch ( pInfo->nKind )
    {
        case SFX_CFGGROUP_FUNCTION :
        {
            USHORT nGroup = pInfo->nOrd;
            String aSelectedGroup = SFX_SLOTPOOL().SeekGroup( nGroup );
            if ( aSelectedGroup != String() )
            {
                const SfxSlot *pSfxSlot = SFX_SLOTPOOL().FirstSlot();
                while ( pSfxSlot )
                {
                    USHORT nId = pSfxSlot->GetSlotId();
#ifdef UNX
                    if ( nId != SID_DESKTOPMODE && ( pSfxSlot->GetMode() & nMode ) )
#else
                    if ( pSfxSlot->GetMode() & nMode )
#endif
                    {
                        String aName = SFX_SLOTPOOL().GetSlotName_Impl( *pSfxSlot );
                        if ( aName.Len() && !pFunctionListBox->GetEntry_Impl( nId ) )
                        {
#ifdef DBG_UTIL
                            if ( pFunctionListBox->GetEntry_Impl( aName ) )
                                DBG_WARNINGFILE( "function name already exits" );
#endif
                            // Wenn die Namen unterschiedlich sind, dann auch die Funktion, denn zu
                            // einer Id liefert der Slotpool immer den gleichen Namen!
                            SvLBoxEntry* pFuncEntry = pFunctionListBox->InsertEntry( aName, NULL );
                            SfxGroupInfo_Impl *pInfo = new SfxGroupInfo_Impl( SFX_CFGFUNCTION_SLOT, nId );
                            pFunctionListBox->aArr.Insert( pInfo, pFunctionListBox->aArr.Count() );
                            pFuncEntry->SetUserData( pInfo );
                        }
                    }

                    pSfxSlot = SFX_SLOTPOOL().NextSlot();
                }
            }

            break;
        }

        case SFX_CFGGROUP_BASICMOD :
        {
            SvLBoxEntry *pLibEntry = GetParent( pEntry );
            SfxGroupInfo_Impl *pLibInfo =
                (SfxGroupInfo_Impl*) pLibEntry->GetUserData();
            SvLBoxEntry *pBasEntry = GetParent( pLibEntry );
            SfxGroupInfo_Impl *pBasInfo =
                (SfxGroupInfo_Impl*) pBasEntry->GetUserData();

            StarBASIC *pLib = (StarBASIC*) pLibInfo->pObject;
            SfxObjectShell *pDoc = NULL;
            if ( pBasInfo->nKind == SFX_CFGGROUP_DOCBASICMGR )
                pDoc = (SfxObjectShell*) pBasInfo->pObject;

            SbModule *pMod = (SbModule*) pInfo->pObject;
            for ( USHORT nMeth=0; nMeth < pMod->GetMethods()->Count(); nMeth++ )
            {
                SbxMethod *pMeth = (SbxMethod*)pMod->GetMethods()->Get(nMeth);
                SfxMacroInfoPtr pInf = new SfxMacroInfo( pDoc,
                                                         pLib->GetName(),
                                                         pMod->GetName(),
                                                         pMeth->GetName());
                if ( pMeth->GetInfo() )
                    pInf->SetHelpText( pMeth->GetInfo()->GetComment() );
                USHORT nId = SFX_APP()->GetMacroConfig()->GetSlotId( pInf );
                if ( !nId )
                    break;      // Kein Slot mehr frei

                SvLBoxEntry* pFuncEntry =
                    pFunctionListBox->InsertEntry( pMeth->GetName(), NULL );
                SfxGroupInfo_Impl *pInfo =
                    new SfxGroupInfo_Impl( SFX_CFGFUNCTION_MACRO, nId, pInf );
                pFunctionListBox->aArr.Insert( pInfo, pFunctionListBox->aArr.Count() );
                pFuncEntry->SetUserData( pInfo );
            }

            break;
        }

        default:
        {
            return;
            break;
        }
    }

    if ( pFunctionListBox->GetEntryCount() )
        pFunctionListBox->Select( pFunctionListBox->GetEntry( 0, 0 ) );

    pFunctionListBox->SetUpdateMode(TRUE);
}

BOOL SfxConfigGroupListBox_Impl::Expand( SvLBoxEntry* pParent )
{
    BOOL bRet = SvTreeListBox::Expand( pParent );
    if ( bRet )
    {
        // Wieviele Entries k"onnen angezeigt werden ?
        ULONG nEntries = GetOutputSizePixel().Height() / GetEntryHeight();

        // Wieviele Kinder sollen angezeigt werden ?
        ULONG nChildCount = GetVisibleChildCount( pParent );

        // Passen alle Kinder und der parent gleichzeitig in die View ?
        if ( nChildCount+1 > nEntries )
        {
            // Wenn nicht, wenigstens parent ganz nach oben schieben
            MakeVisible( pParent, TRUE );
        }
        else
        {
            // An welcher relativen ViewPosition steht der aufzuklappende parent
            SvLBoxEntry *pEntry = GetFirstEntryInView();
            ULONG nParentPos = 0;
            while ( pEntry && pEntry != pParent )
            {
                nParentPos++;
                pEntry = GetNextEntryInView( pEntry );
            }

            // Ist unter dem parent noch genug Platz f"ur alle Kinder ?
            if ( nParentPos + nChildCount + 1 > nEntries )
                ScrollOutputArea( (short)( nEntries - ( nParentPos + nChildCount + 1 ) ) );
        }
    }

    return bRet;
}

void SfxConfigGroupListBox_Impl::RequestingChilds( SvLBoxEntry *pEntry )
/*  Beschreibung
    Ein Basic oder eine Bibliothek werden ge"offnet
*/
{
    SfxGroupInfo_Impl *pInfo = (SfxGroupInfo_Impl*) pEntry->GetUserData();
    pInfo->bWasOpened = TRUE;
    switch ( pInfo->nKind )
    {
        case SFX_CFGGROUP_BASICMGR :
        case SFX_CFGGROUP_DOCBASICMGR :
        {
            if ( !GetChildCount( pEntry ) )
            {
                // Erstmaliges "Offnen
                BasicManager *pMgr;
                if ( pInfo->nKind == SFX_CFGGROUP_DOCBASICMGR )
                    pMgr = ((SfxObjectShell*)pInfo->pObject)->GetBasicManager();
                else
                    pMgr = (BasicManager*) pInfo->pObject;

                SvLBoxEntry *pLibEntry = 0;
                for ( USHORT nLib=0; nLib<pMgr->GetLibCount(); nLib++)
                {
                    StarBASIC* pLib = pMgr->GetLib( nLib );
                    pLibEntry = InsertEntry( pMgr->GetLibName( nLib ), pEntry );
                    SfxGroupInfo_Impl *pInfo = new SfxGroupInfo_Impl( SFX_CFGGROUP_BASICLIB, nLib, pLib );
                    aArr.Insert( pInfo, aArr.Count() );
                    pLibEntry->SetUserData( pInfo );
                    pLibEntry->EnableChildsOnDemand( TRUE );
                }
            }

            break;
        }

        case SFX_CFGGROUP_BASICLIB :
        {
            if ( !GetChildCount( pEntry ) )
            {
                // Erstmaliges "Offnen
                StarBASIC *pLib = (StarBASIC*) pInfo->pObject;
                if ( !pLib )
                {
                    // Lib mu\s nachgeladen werden
                    SvLBoxEntry *pParent = GetParent( pEntry );
                    SfxGroupInfo_Impl *pInf =
                        (SfxGroupInfo_Impl*) pParent->GetUserData();
                    BasicManager *pMgr;
                    if ( pInf->nKind == SFX_CFGGROUP_DOCBASICMGR )
                        pMgr = ((SfxObjectShell*)pInf->pObject)->GetBasicManager();
                    else
                        pMgr = (BasicManager*) pInf->pObject;

                    if ( pMgr->LoadLib( pInfo->nOrd ) )
                        pInfo->pObject = pLib = pMgr->GetLib( pInfo->nOrd );
                    else
                        break;
                }

                SvLBoxEntry *pModEntry = 0;
                for ( USHORT nMod=0; nMod<pLib->GetModules()->Count(); nMod++ )
                {
                    SbModule* pMod = (SbModule*)pLib->GetModules()->Get( nMod );

                    BOOL bIsStarScript = FALSE; //pMod->ISA( SbJScriptModule );
                    BOOL bWantsStarScript = aScriptType.EqualsAscii("StarScript");
                    if ( bIsStarScript != bWantsStarScript )
                        continue;
                    pModEntry = InsertEntry( pMod->GetName(), pEntry );
                    SfxGroupInfo_Impl *pInfo = new SfxGroupInfo_Impl( SFX_CFGGROUP_BASICMOD, 0, pMod );
                    aArr.Insert( pInfo, aArr.Count() );
                    pModEntry->SetUserData( pInfo );
                }
            }

            break;
        }

        default:
            DBG_ERROR( "Falscher Gruppentyp!" );
            break;
    }
}

void SfxConfigGroupListBox_Impl::SelectMacro( const SfxMacroInfoItem *pItem )
{
    SelectMacro( pItem->GetBasicManager()->GetName(),
                 pItem->GetQualifiedName() );
}

void SfxConfigGroupListBox_Impl::SelectMacro( const String& rBasic,
         const String& rMacro )
{
    String aBasicName(' ');
    aBasicName += rBasic;
    aBasicName += String(SfxResId(STR_BASICMACROS));
    String aLib, aModule, aMethod;
    USHORT nCount = rMacro.GetTokenCount('.');
    aMethod = rMacro.GetToken( nCount-1, '.' );
    if ( nCount > 2 )
    {
        aLib = rMacro.GetToken( 0, '.' );
        aModule = rMacro.GetToken( nCount-2, '.' );
    }

    SvLBoxEntry *pEntry = FirstChild(0);
    while ( pEntry )
    {
        String aEntryBas = GetEntryText( pEntry );
        if ( aEntryBas == aBasicName )
        {
            Expand( pEntry );
            SvLBoxEntry *pLib = FirstChild( pEntry );
            while ( pLib )
            {
                String aEntryLib = GetEntryText( pLib );
                if ( aEntryLib == aLib )
                {
                    Expand( pLib );
                    SvLBoxEntry *pMod = FirstChild( pLib );
                    while ( pMod )
                    {
                        String aEntryMod = GetEntryText( pMod );
                        if ( aEntryMod == aModule )
                        {
                            Expand( pMod );
                            MakeVisible( pMod );
                            Select( pMod );
                            SvLBoxEntry *pMethod = pFunctionListBox->First();
                            while ( pMethod )
                            {
                                String aEntryMethod = GetEntryText( pMethod );
                                if ( aEntryMethod == aMethod )
                                {
                                    pFunctionListBox->Select( pMethod );
                                    pFunctionListBox->MakeVisible( pMethod );
                                    return;
                                }
                                pMethod = pFunctionListBox->Next( pMethod );
                            }
                        }
                        pMod = NextSibling( pMod );
                    }
                }
                pLib = NextSibling( pLib );
            }
        }
        pEntry = NextSibling( pEntry );
    }
}

void SfxConfigDialog::ActivateToolBoxConfig( USHORT nId )
{
    SetCurPageId( TP_CONFIG_OBJECTBAR );
    nObjectBar = nId;
}

void SfxConfigDialog::PageCreated( USHORT nId, SfxTabPage& rPage )
{
    switch ( nId )
    {
        case TP_CONFIG_OBJECTBAR :
            if ( nObjectBar )
                ((SfxObjectBarConfigPage&)rPage).SetObjectBarId( nObjectBar );
            break;
        case TP_CONFIG_ACCEL:
            if ( pMacroInfo )
                ((SfxAcceleratorConfigPage&)rPage).SelectMacro( pMacroInfo );
            break;
        default:
            break;
    }
}

void SfxConfigDialog::ActivateMacroConfig(const SfxMacroInfoItem *pInfo)
{
    // Bei Aufruf "uber die Basic-IDE soll defaultm"a\sig die Tastaturkonfiguration
    // angeboten werden
    SetCurPageId( TP_CONFIG_ACCEL );
    pMacroInfo = pInfo;
}

SfxTabPage *CreateMenuConfigPage( Window *pParent, const SfxItemSet& rSet )
{
    return new SfxMenuConfigPage( pParent, rSet );
}

SfxTabPage *CreateAccelConfigPage( Window *pParent, const SfxItemSet& rSet )
{
    return new SfxAcceleratorConfigPage( pParent, rSet );
}

SfxTabPage *CreateStatusBarConfigPage( Window *pParent, const SfxItemSet& rSet )
{
    return new SfxStatusBarConfigPage( pParent, rSet );
}

SfxTabPage *CreateObjectBarConfigPage( Window *pParent, const SfxItemSet& rSet )
{
    return new SfxObjectBarConfigPage( pParent, rSet );
}

SfxTabPage *CreateEventConfigPage( Window *pParent, const SfxItemSet& rSet )
{
    return new SfxEventConfigPage( pParent, rSet );
}

SfxConfigDialog::SfxConfigDialog( Window * pParent, const SfxItemSet* pSet )
    : SfxTabDialog( pParent, SfxResId( DLG_CONFIG ), pSet )
    , pMacroInfo( 0 )
    , nObjectBar( 0 )
{
    FreeResource();

    AddTabPage( TP_CONFIG_MENU, CreateMenuConfigPage, NULL );
    AddTabPage( TP_CONFIG_ACCEL, CreateAccelConfigPage, NULL );
    AddTabPage( TP_CONFIG_STATBAR, CreateStatusBarConfigPage, NULL );
    AddTabPage( TP_CONFIG_OBJECTBAR, CreateObjectBarConfigPage, NULL );
    AddTabPage( TP_CONFIG_EVENT, CreateEventConfigPage, NULL );
}

SfxConfigDialog::~SfxConfigDialog()
{
}

short SfxConfigDialog::Ok()
{
    return SfxTabDialog::Ok();
}

// SfxStatusBarConfigPage::Ctor() **********************************************

SfxStatusBarConfigPage::SfxStatusBarConfigPage( Window *pParent, const SfxItemSet& rSet ) :

    SfxTabPage( pParent, SfxResId( TP_CONFIG_STATBAR ), rSet ),

    aEntriesBox         ( this, ResId( BOX_STATBAR_ENTRIES         ) ),
    aStatusBarGroup     ( this, ResId( GRP_STATBAR                 ) ),
    aLoadButton         ( this, ResId( BTN_LOAD ) ),
    aSaveButton         ( this, ResId( BTN_SAVE ) ),
    aResetButton        ( this, ResId( BTN_RESET   ) ),
    pMgr( 0 ),
    bMgrCreated( FALSE )
{
    FreeResource();

    SfxViewFrame* pCurrent = SfxViewFrame::Current();
    while ( pCurrent->GetParentViewFrame_Impl() )
        pCurrent = pCurrent->GetParentViewFrame_Impl();

    pMgr = pCurrent->GetFrame()->GetWorkWindow_Impl()->GetStatusBarManager_Impl();
    if ( !pMgr )
    {
        bMgrCreated = TRUE;
        pMgr = new SfxStatusBarManager( this, pCurrent->GetBindings(), SFX_APP() );
    }

    aLoadButton  .SetClickHdl ( LINK( this, SfxStatusBarConfigPage, Load      ) );
    aSaveButton  .SetClickHdl ( LINK( this, SfxStatusBarConfigPage, Save      ) );
    aResetButton .SetClickHdl ( LINK( this, SfxStatusBarConfigPage, Default      ) );

    // aEntriesBox initialisieren
    aEntriesBox.bDefault = pMgr->IsDefault();
//(mba)/task    SfxWaitCursor aWait;

    aEntriesBox.SetSelectHdl( LINK(this,SfxStatusBarConfigPage, SelectHdl));
    USHORT nGroup = 0;
    String aGroupName = SFX_SLOTPOOL().SeekGroup(nGroup);

    while (aGroupName.Len() != 0)
    {
        USHORT n=0;
        for (const SfxSlot *pSlot = SFX_SLOTPOOL().FirstSlot(); pSlot;
                            pSlot = SFX_SLOTPOOL().NextSlot())
        {
            if ( !pSlot->GetType() ||
                 pSlot->GetType()->Type() == TYPE(SfxVoidItem) ||
                 !(pSlot->GetMode() & SFX_SLOT_STATUSBARCONFIG) )
                continue;

            USHORT nId = pSlot->GetSlotId();
            USHORT i;
            for (i=0; i<aListOfIds.Count(); i++)
                if (aListOfIds[i] >= nId) break;

            if (i<aListOfIds.Count() && aListOfIds[i] == nId)
                continue;
            else
                aListOfIds.Insert(nId, i);
        }

        aGroupName = SFX_SLOTPOOL().SeekGroup(++nGroup);
    }

    aEntriesBox.bDefault = pMgr->IsDefault();
    Init();
}

void SfxStatusBarConfigPage::Init()
{
    SvLBoxEntry *pEntry;
    USHORT nUsed=0;
    SvUShorts aListOfPos;
    pArr = new SfxStbInfoArr_Impl;

    for (USHORT i=0; i<aListOfIds.Count(); i++)
    {
        USHORT nId = aListOfIds[i];
        String aName = SFX_SLOTPOOL().GetSlotName_Impl( nId );
        if ( !aName.Len() )
            continue;

        SfxStatBarInfo_Impl* pInfo = new SfxStatBarInfo_Impl (nId, String());
        pArr->Append(pInfo);
        USHORT nPos = pMgr->GetItemPos(nId);
        if (nPos != STATUSBAR_ITEM_NOTFOUND)
        {
            USHORT np;
            for (np=0; np<aListOfPos.Count(); np++)
                if (aListOfPos[np] > nPos) break;
            aListOfPos.Insert(nPos,np);
            pEntry = aEntriesBox.InsertEntry(aName, (SvLBoxEntry*)0L, FALSE, (ULONG) np);
            aEntriesBox.SetCheckButtonState(pEntry, SV_BUTTON_CHECKED);
            nUsed++;
        }
        else
        {
            pEntry = aEntriesBox.InsertEntry(aName, (SvLBoxEntry*)0L);
        }

        pEntry->SetUserData(pInfo);
    }
}

void SfxStatusBarConfigPage::ResetConfig()
{
    aEntriesBox.Clear();
    for (USHORT n=0; n<pArr->Count(); n++)
        delete (*pArr)[n];
    delete pArr;
}

SfxStatusBarConfigPage::~SfxStatusBarConfigPage()
{
    if ( bMgrCreated )
    {
        StatusBar *pBar = pMgr->GetStatusBar();
        delete pMgr;
    }

    ResetConfig();
}

IMPL_LINK( SfxStatusBarConfigPage, SelectHdl, SvTreeListBox *, pBox )
{
    return 0;
}


void SfxStatusBarConfigPage::Apply()
{
    if (!aEntriesBox.bModified)
        return;

    if ( aEntriesBox.bDefault )
    {
        pMgr->UseDefault();
        pMgr->SetDefault(TRUE);
        return;
    }

//(mba)/task    SfxWaitCursor aWait;
    pMgr->Clear();
    long nWidth = 100;

    for (SvLBoxEntry *pEntry = aEntriesBox.First(); pEntry; pEntry = aEntriesBox.Next(pEntry))
    {
        if (aEntriesBox.GetCheckButtonState(pEntry) == SV_BUTTON_CHECKED)
        {
            USHORT nId =
                ((SfxStatBarInfo_Impl*) pEntry->GetUserData())->nId;
            pMgr->AddItem(nId, nWidth);
        }
    }
}

IMPL_LINK( SfxStatusBarConfigPage, Default, PushButton *, pPushButton )
{
    SfxConfigManager *pOldCfgMgr = pMgr->GetConfigManager_Impl();
    aEntriesBox.bDefault = TRUE;
    aEntriesBox.bModified = !pMgr->IsDefault();
    pMgr->StoreConfig();
    pMgr->ReleaseConfigManager();
    pMgr->UseDefault();
    aEntriesBox.SetUpdateMode(FALSE);
    ResetConfig();
    Init();
    aEntriesBox.SetUpdateMode(TRUE);
    aEntriesBox.Invalidate();
    pMgr->ReInitialize(pOldCfgMgr);
    return 0;
}

IMPL_LINK( SfxStatusBarConfigPage, Load, Button *, pButton )
{
    String aCfgName = SfxConfigDialog::FileDialog_Impl( this,
        WB_OPEN | WB_STDMODAL | WB_3DLOOK, String( SfxResId( STR_LOADSTATBARCONFIG) ) );
    if ( aCfgName.Len() )
    {
//(mba)/task    SfxWaitCursor aWait;

        BOOL bCreated = FALSE;
        SfxConfigManager *pCfgMgr =
            SfxConfigDialog::MakeCfgMgr_Impl( aCfgName, bCreated );
        SfxConfigManager *pOldCfgMgr = pMgr->GetConfigManager_Impl();
        if ( pOldCfgMgr != pCfgMgr )
        {
            if ( pCfgMgr->HasConfigItem( pMgr->GetType() ) )
            {
                pMgr->ReInitialize( pCfgMgr );
                aEntriesBox.SetUpdateMode( FALSE );
                ResetConfig();
                Init();
                aEntriesBox.SetUpdateMode( TRUE );
                aEntriesBox.Invalidate();
                pMgr->ReInitialize( pOldCfgMgr );
                aEntriesBox.bDefault = FALSE;
                aEntriesBox.bModified = TRUE;
            }
        }

        if ( bCreated )
            delete pCfgMgr;
    }

    return 0;
}

IMPL_LINK( SfxStatusBarConfigPage, Save, Button *, pButton )
{
    String aCfgName = SfxConfigDialog::FileDialog_Impl( this,
        WB_SAVEAS | WB_STDMODAL | WB_3DLOOK, String( SfxResId( STR_SAVESTATBARCONFIG) ) );
    if ( aCfgName.Len() )
    {
//(mba)/task    SfxWaitCursor aWait;

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
            pMgr->Connect( pCfgMgr );
            pCfgMgr->AddConfigItem( pMgr );
            BOOL bMod = aEntriesBox.bModified;
            pMgr->GetStatusBar()->SetUpdateMode( FALSE );
            BOOL bItemModified = pMgr->IsModified();
            Apply();
            pMgr->SetModified( TRUE );
            pCfgMgr->StoreConfig();
            pMgr->ReInitialize( pOldCfgMgr );
            pMgr->SetModified( bItemModified );
            pMgr->GetStatusBar()->SetUpdateMode( TRUE );
            aEntriesBox.bModified = bMod;

            if ( bCreated )
            {
                pCfgMgr->SetModified( TRUE );
                pCfgMgr->SaveConfig();
#ifdef MAC
                SvEaMgr aEaMgr( aCfgName );
                aEaMgr.SetFileType( "Pref" );
#endif
            }
        }
        else
        {
            Apply();
            pMgr->SetModified( TRUE );
            pCfgMgr->StoreConfig();
        }

        if ( bCreated )
            delete pCfgMgr;
    }
    return 0;
}


SfxStatusBarConfigListBox::SfxStatusBarConfigListBox( Window* pParent, const ResId& rResId)
 : SvTreeListBox(pParent, rResId)
 , bModified(FALSE)
 , pCurEntry(0)
 , bDefault(TRUE)
{
    pButton = new SvLBoxButtonData();
    pButton->aBmps[SV_BMP_UNCHECKED]   = Bitmap(SfxResId(CHKBTN_UNCHECKED));
    pButton->aBmps[SV_BMP_CHECKED]     = Bitmap(SfxResId(CHKBTN_CHECKED));
    pButton->aBmps[SV_BMP_HICHECKED]   = Bitmap(SfxResId(CHKBTN_HICHECKED));
    pButton->aBmps[SV_BMP_HIUNCHECKED] = Bitmap(SfxResId(CHKBTN_HIUNCHECKED));
    pButton->aBmps[SV_BMP_TRISTATE]    = Bitmap(SfxResId(CHKBTN_TRISTATE));
    pButton->aBmps[SV_BMP_HITRISTATE]  = Bitmap(SfxResId(CHKBTN_HITRISTATE));

    EnableCheckButton(pButton);

    DragDropMode aDDMode = SV_DRAGDROP_CTRL_MOVE;
    SetDragDropMode( aDDMode );
    SetHighlightRange(); // OV Selektion ueber gesamte Zeilenbreite

    // Timer f"ur die BallonHelp
    aTimer.SetTimeout( 200 );
    aTimer.SetTimeoutHdl(
        LINK( this, SfxStatusBarConfigListBox, TimerHdl ) );
}

void SfxStatusBarConfigListBox::KeyInput( const KeyEvent& rKEvt )
{
    USHORT aCode = rKEvt.GetKeyCode().GetCode();
    if( aCode == KEY_SPACE )
    {
        SvLBoxEntry* pEntry = (SvLBoxEntry*) FirstSelected();
        if( pEntry && (GetCheckButtonState( pEntry ) == SV_BUTTON_UNCHECKED) )
            SetCheckButtonState(pEntry, SV_BUTTON_CHECKED);
        else
            SetCheckButtonState(pEntry, SV_BUTTON_UNCHECKED);
        GetCheckButtonHdl().Call(this);
    }
    else
        SvTreeListBox::KeyInput( rKEvt );
}

BOOL SfxStatusBarConfigListBox::NotifyQueryDrop( SvLBoxEntry* pEntry )
{
    return SvTreeListBox::NotifyQueryDrop(pEntry);
}

BOOL SfxStatusBarConfigListBox::NotifyMoving(SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
                                SvLBoxEntry*& rpNewParent, ULONG& rNewChildPos)
{
    BOOL bRet =
        SvTreeListBox::NotifyMoving(pTarget, pEntry, rpNewParent, rNewChildPos);
    if (bRet)
    {
        bModified = TRUE;
        bDefault = FALSE;
    }
    return bRet;
}

void SfxStatusBarConfigListBox::CheckButtonHdl()
{
    bDefault = FALSE;
    bModified = TRUE;
}

void SfxStatusBarConfigListBox::MouseMove( const MouseEvent& rMEvt )
/*  Beschreibung
    Virtuelle Methode, die gerufen wird, wenn der Mauszeiger "uber der TreeListBox bewegt wurde.
    Wenn die Position des Mauszeigers "uber dem aktuell selektierten Entry liegt, wird ein Timer
    aufgesetzt, um ggf. einen Hilfetext einzublenden.
*/
{
    Point aMousePos = rMEvt.GetPosPixel();
    SvLBoxEntry *pEntry = GetCurEntry();
    pCurEntry = pEntry;

    if ( pEntry && GetEntry( aMousePos ) == pEntry )
        aTimer.Start();
    else
    {
        Help::ShowBalloon( this, aMousePos, String() );
        aTimer.Stop();
    }
}

IMPL_LINK( SfxStatusBarConfigListBox, TimerHdl, Timer*, pTimer)
/*  Beschreibung
    Timer-Handler f"ur die Einblendung eines Hilfetextes. Wenn nach Ablauf des Timers
    der Mauszeiger immer noch auf dem aktuell selektierten Eintrag steht, wird der
    Helptext des Entries als Balloon-Help eingeblendet.
*/
{
    if ( pTimer )
        pTimer->Stop();

    Point aMousePos = GetPointerPosPixel();
    SvLBoxEntry *pEntry = GetCurEntry();
    if ( pEntry && GetEntry( aMousePos ) == pEntry && pEntry == pCurEntry )
    {
        SfxStatBarInfo_Impl* pInfo = (SfxStatBarInfo_Impl*) pEntry->GetUserData();
        if ( !pInfo->aHelpText.Len() )
            pInfo->aHelpText = Application::GetHelp()->GetHelpText( pInfo->nId );
        Help::ShowBalloon( this, OutputToScreenPixel( aMousePos ), pInfo->aHelpText );
    }

    return 0L;
}



BOOL SfxStatusBarConfigPage::FillItemSet( SfxItemSet& )
{
    if ( aEntriesBox.bModified )
    {
        Apply();
        aEntriesBox.bModified = FALSE;
        return TRUE;
    }
    return FALSE;
}

void SfxStatusBarConfigPage::Reset( const SfxItemSet& )
{
}

BOOL SfxMenuConfigPage::FillItemSet( SfxItemSet& )
{
    if ( bModified )
    {
        Apply();
        bModified = FALSE;
        return TRUE;
    }
    return FALSE;
}

void SfxMenuConfigPage::Reset( const SfxItemSet& )
{
}

BOOL SfxAcceleratorConfigPage::FillItemSet( SfxItemSet& )
{
    if ( bModified )
    {
        Apply();
        bModified = FALSE;
        return TRUE;
    }
    return FALSE;
}

void SfxAcceleratorConfigPage::Reset( const SfxItemSet& )
{
}

String SfxConfigDialog::FileDialog_Impl( Window *pParent, WinBits nBits, const String& rTitle )
{
    SfxSimpleFileDialog aFileDlg( pParent, nBits );
    aFileDlg.SetText( rTitle );
#ifdef MAC
    aFileDlg.AddFilter( String(SfxResId(STR_FILTERNAME_CFG)),DEFINE_CONST_UNICODE("Pref.cfg") );
#else
    aFileDlg.AddFilter( String(SfxResId(STR_FILTERNAME_CFG)),DEFINE_CONST_UNICODE("*.cfg") );
#endif
    aFileDlg.AddFilter( String(SfxResId(STR_FILTERNAME_ALL) ), DEFINE_CONST_UNICODE(FILEDIALOG_FILTER_ALL) );
#if SUPD<613//MUSTINI
    INetURLObject aFilePath( SFX_INIMANAGER()->Get( SFX_KEY_USERCONFIG_PATH ), INET_PROT_FILE );
#else
    INetURLObject aFilePath( SvtPathOptions().GetUserConfigPath(), INET_PROT_FILE );
#endif
    aFilePath.setFinalSlash();
    String aCfgName = aFilePath.PathToFileName();
#ifndef MAC
    aCfgName += DEFINE_CONST_UNICODE( "*.cfg" );
    aFileDlg.SetDefaultExt( DEFINE_CONST_UNICODE( "cfg" ) );
#endif

    aFileDlg.SetPath( aCfgName );
    if ( aFileDlg.Execute() )
        return aFileDlg.GetPath();
    else
        return String();
}

SfxConfigManager* SfxConfigDialog::MakeCfgMgr_Impl( const String& rName, BOOL& bCreated )
{
    // Zuerst feststellen, ob der Storage schon in Benutzung ist
    bCreated = FALSE;
    SfxObjectShell *pDoc = SFX_APP()->DocAlreadyLoaded( rName, TRUE, TRUE );
    SfxConfigManager *pCfgMgr = 0;
    if ( !pDoc )
    {
        if ( rName != SfxConfigManager::GetDefaultName() )
        {
            pCfgMgr = new SfxConfigManager( rName );
            bCreated = TRUE;
        }
        else
            pCfgMgr = SFX_APP()->GetAppConfigManager_Impl();
    }
    else
    {
        pCfgMgr = pDoc->GetConfigManager();
        if ( !pCfgMgr )
        {
            pCfgMgr = new SfxConfigManager( 0, SFX_CFGMANAGER() );
            pDoc->SetConfigManager( pCfgMgr );
        }
    }

    return pCfgMgr;
}



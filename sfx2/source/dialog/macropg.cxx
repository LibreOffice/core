/*************************************************************************
 *
 *  $RCSfile: macropg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:30 $
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

#ifndef _RCID_H
#include <vcl/rcid.h>
#endif
#ifndef _BASMGR_HXX //autogen
#include <basic/basmgr.hxx>
#endif
#pragma hdrstop

#define ITEMID_MACRO 0
#include "macropg.hxx"
#undef ITEMID_MACRO

#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>

#ifdef VCL
#ifndef _SV_MEDIT_HXX
#include <svtools/svmedit.hxx>
#endif
#endif

#include "cfg.hxx"
#include "app.hxx"
#include "objsh.hxx"
#include "macropg.hrc"
#include "sfxresid.hxx"
#include "dialog.hrc"
#include "macrconf.hxx"
#include "sfxdefs.hxx"

#if SUPD>399
SvStringsDtor* _ImpGetRangeHdl( _SfxMacroTabPage*, const String& rLanguage );
SvStringsDtor* _ImpGetMacrosOfRangeHdl( _SfxMacroTabPage*, const String& rLanguage,
                                        const String& rRange );
#else
SvStringsDtor* _ImpGetRangeHdl( _SfxMacroTabPage*, ScriptType eType );
SvStringsDtor* _ImpGetMacrosOfRangeHdl( _SfxMacroTabPage*, ScriptType eType,
                                        const String& rRange );
#endif
static USHORT __FAR_DATA aPageRg[] = {
    SID_ATTR_MACROITEM, SID_ATTR_MACROITEM,
    0
};

// Achtung im Code wird dieses Array direkt (0, 1, ...) indiziert
static long nTabs[] =
    {
        2, // Number of Tabs
        0, 90
    };

#define LB_EVENTS_ITEMPOS 1
#define LB_MACROS_ITEMPOS 2

String ConvertToUIName_Impl( SvxMacro *pMacro, const String& rLanguage )
{
    String aName( pMacro->GetMacName() );
    String aEntry;
    if ( ! pMacro->GetLanguage().EqualsAscii("JavaScript") )
    {
        USHORT nCount = aName.GetTokenCount('.');
        aEntry = aName.GetToken( nCount-1, '.' );
        if ( nCount > 2 )
        {
            aEntry += '(';
            aEntry += aName.GetToken( 0, '.' );
            aEntry += '.';
            aEntry += aName.GetToken( nCount-2, '.' );
            aEntry += ')';
        }
        return aEntry;
    }
    else
        return aName;
}

void _SfxMacroTabPage::EnableButtons( const String& rLangName )
{
    // Solange die Eventbox leer ist, nichts tun
    const SvLBoxEntry* pE = pEventLB->FirstSelected();
    if ( pE )
    {
        // Gebundenes Macro holen
        const SvxMacro* pM = aTbl.Get( (USHORT)(ULONG) pE->GetUserData() );
        pDeletePB->Enable( 0 != pM && !bReadOnly );

        // Bei gleichem ScriptType Zuweisung nur, wenn Macro sich
        // ge"andert hat; bei verschiedenem ScriptType, wenn Script nicht leer
        String sEventMacro;
        sEventMacro = ((SvLBoxString*)pE->GetItem( LB_MACROS_ITEMPOS ))->GetText();
        if ( rLangName.EqualsAscii("JavaScript") )
        {
#if SUPD<582
            String sScript = pJavaED->GetText();
            if( pM && rLangName != pM->GetLanguage() )
                pAssignPB->Enable( TRUE && !bReadOnly );
            else
                pAssignPB->Enable( !bReadOnly && sScript.Len() && ( !sEventMacro.Len() || sScript != sEventMacro ) );
#endif
        }
        else
        {
            SfxMacroInfo *pInfo = pMacroLB->GetMacroInfo();
            String sSelMacro;
            if ( pInfo )
                sSelMacro = pInfo->GetMacroName();
            if( pM && rLangName != pM->GetLanguage() )
                pAssignPB->Enable( pInfo != 0 && !bReadOnly );
            else
                pAssignPB->Enable( pInfo && !bReadOnly && !sSelMacro.EqualsIgnoreCaseAscii( sEventMacro ) );
        }
    }
}

_SfxMacroTabPage::_SfxMacroTabPage( Window* pParent, const ResId& rResId,
                                    const SfxItemSet& rAttrSet )
    : SfxTabPage( pParent, rResId, rAttrSet ),
    pAssignPB( 0 ),
    pDeletePB( 0 ),
//  pBasicRB( 0 ),
//  pJavaScriptRB( 0 ),
    pScriptTypeLB( 0 ),
    pEventLB( 0 ),
    pGroupLB( 0 ),
    pMacroLB( 0 ),
    pEventGB( 0 ),
    pMacroGB( 0 ),
#if SUPD<582
    pJavaStr( 0 ),
    pJavaED( 0 ),
#endif
    pMacroStr( 0 ),
    fnGetRange( &_ImpGetRangeHdl ),
    fnGetMacroOfRange( &_ImpGetMacrosOfRangeHdl ),
    bReadOnly( FALSE )
{
}

_SfxMacroTabPage::~_SfxMacroTabPage()
{
    delete pAssignPB;
    delete pDeletePB;
//  delete pBasicRB;
//  delete pJavaScriptRB;
    delete pScriptTypeLB;
    delete pEventLB;
    delete pGroupLB;
    delete pMacroLB;
    delete pEventGB;
    delete pMacroGB;
#if SUPD<582
    delete pJavaStr;
    delete pJavaED;
#endif
    delete pMacroStr;
}

void _SfxMacroTabPage::AddEvent( const String & rEventName, USHORT nEventId )
{
    String sTmp( rEventName );
    sTmp += '\t';

    // falls die Tabelle schon gueltig ist
    SvxMacro* pM = aTbl.Get( nEventId );
    if( pM )
    {
        String sNew( ConvertToUIName_Impl( pM, pScriptTypeLB->GetSelectEntry() ) );
        sTmp += sNew;
    }

    SvLBoxEntry* pE = pEventLB->InsertEntry( sTmp );
    pE->SetUserData( (void*)nEventId );
}

void _SfxMacroTabPage::ScriptChanged( const String& aLangName )
{
    // neue Bereiche und deren Funktionen besorgen
#if SUPD<582
    if ( aLangName == "JavaScript" )
    {
        const SvLBoxEntry* pE = pEventLB->FirstSelected();
        if( pE )
        {
            const SvxMacro* pM = aTbl.Get( (USHORT)(ULONG)pE->GetUserData() );
            if ( pM && pM->GetMacName().Len() && aLangName == pM->GetLanguage() )
                pJavaED->SetText( pM->GetMacName() );
        }

        pJavaED->Show();
        pGroupLB->Hide();
        pMacroLB->Hide();
        pMacroGB->SetText( *pJavaStr );
    }
    else
#endif
    {
#if SUPD<582
        pJavaED->Hide();
#endif
        pGroupLB->SetScriptType( aLangName );
        pGroupLB->Show();
        pMacroLB->Show();
        pMacroGB->SetText( *pMacroStr );
    }

    EnableButtons( aLangName );
}

BOOL _SfxMacroTabPage::FillItemSet( SfxItemSet& rSet )
{
    SvxMacroItem aItem( GetWhich( aPageRg[0] ) );
    ((SvxMacroTableDtor&)aItem.GetMacroTable()) = aTbl;

    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET != GetItemSet().GetItemState( aItem.Which(), TRUE, &pItem )
        || aItem != *(SvxMacroItem*)pItem )
    {
        rSet.Put( aItem );
        return TRUE;
    }
    return FALSE;
}

void _SfxMacroTabPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rSet.GetItemState( GetWhich( aPageRg[0] ), TRUE, &pItem ))
        aTbl = ((SvxMacroItem*)pItem)->GetMacroTable();

    FillEvents();

    SvLBoxEntry* pE = pEventLB->GetEntry( 0 );
    if( pE )
        pEventLB->SetCurEntry( pE );
}

IMPL_STATIC_LINK( _SfxMacroTabPage, SelectEvent_Impl, SvTabListBox*, EMPTYARG )
{
    SvLBoxEntry* pE = pThis->pEventLB->FirstSelected();
    ULONG nPos;
    if( !pE || LISTBOX_ENTRY_NOTFOUND ==
        ( nPos = pThis->pEventLB->GetModel()->GetAbsPos( pE ) ) )
    {
        DBG_ASSERT( pE, "wo kommt der leere Eintrag her?" );
        return 0;
    }

    USHORT nEventId = (USHORT)(ULONG)pE->GetUserData();
    String aLanguage = pThis->pScriptTypeLB->GetSelectEntry();

    const SvxMacro* pM = pThis->aTbl.Get( nEventId );
    if( pM )
    {
        if( aLanguage != pM->GetLanguage() )
        {
            pThis->pScriptTypeLB->SelectEntry( pM->GetLanguage() );
            pThis->ScriptChanged( pM->GetLanguage() );
        }
        else if ( aLanguage.EqualsAscii("JavaScript") )
        {
#if SUPD<582

            const SvxMacro* pM = 0;
            const SvLBoxEntry* pE = pThis->pEventLB->FirstSelected();
            if( pE &&
                ( pM = pThis->aTbl.Get( (USHORT)(ULONG)pE->GetUserData() ) ) != 0 &&
                pM->GetMacName().Len() )
                    pThis->pJavaED->SetText( pM->GetMacName() );
#endif
        }
    }

    pThis->EnableButtons( aLanguage );
    return 0;
}

IMPL_STATIC_LINK( _SfxMacroTabPage, SelectGroup_Impl, ListBox*, EMPTYARG )
{
    String sSel( pThis->pGroupLB->GetGroup() );
    String aLanguage = pThis->pScriptTypeLB->GetSelectEntry();
    if ( !aLanguage.EqualsAscii("JavaScript") )
    {
        pThis->pGroupLB->GroupSelected();
        SfxMacroInfo *pMacro = pThis->pMacroLB->GetMacroInfo();
        if ( !pMacro )
            // Wenn dort ein Macro drin ist, wurde es selektiert und der
            // AssignButton schon in SelectMacro richtig enabled
            pThis->pAssignPB->Enable( FALSE );
    }
    return 0;
}

IMPL_STATIC_LINK( _SfxMacroTabPage, SelectMacro_Impl, ListBox*, EMPTYARG )
{
    pThis->pMacroLB->FunctionSelected();
    pThis->EnableButtons( pThis->pScriptTypeLB->GetSelectEntry() );
    return 0;
}

IMPL_STATIC_LINK( _SfxMacroTabPage, GetFocus_Impl, Edit*, EMPTYARG )
{
    pThis->EnableButtons( DEFINE_CONST_UNICODE("JavaScript") );
    return 0;
}

IMPL_STATIC_LINK( _SfxMacroTabPage, AssignDeleteHdl_Impl, PushButton*, pBtn )
{
    SvLBoxEntry* pE = pThis->pEventLB->FirstSelected();
    ULONG nPos;
    if( !pE || LISTBOX_ENTRY_NOTFOUND ==
        ( nPos = pThis->pEventLB->GetModel()->GetAbsPos( pE ) ) )
    {
        DBG_ASSERT( pE, "wo kommt der leere Eintrag her?" );
        return 0;
    }

    const BOOL bAssEnabled = pBtn != pThis->pDeletePB &&
                            pThis->pAssignPB->IsEnabled();

    // aus der Tabelle entfernen
    USHORT nEvent = (USHORT)(ULONG)pE->GetUserData();
    SvxMacro *pMacro = pThis->aTbl.Remove( nEvent );
    delete pMacro;

    String aLanguage = pThis->pScriptTypeLB->GetSelectEntry();
    String sNew;
    if( bAssEnabled )
    {
        String sGroup;
        String sMacro;
        String aEntryText( sNew );
        if ( aLanguage.EqualsAscii("JavaScript") )
        {
#if SUPD<582
            sNew = sMacro = pThis->pJavaED->GetText();
#endif
        }
        else
        {
            SfxMacroInfo *pMacro = pThis->pMacroLB->GetMacroInfo();
            sMacro = pMacro->GetQualifiedName();
            sGroup = pThis->pGroupLB->GetGroup();
            sNew = pMacro->GetMacroName();
        }

        if ( aLanguage.EqualsAscii("StarBasic") )
            pThis->aTbl.Insert( nEvent, new SvxMacro( sMacro, sGroup, STARBASIC ) );
        else
            pThis->aTbl.Insert( nEvent, new SvxMacro( sMacro, aLanguage ) );
    }

    pThis->pEventLB->SetUpdateMode( FALSE );
    pE->ReplaceItem( new SvLBoxString( pE, 0, sNew ), LB_MACROS_ITEMPOS );
    pThis->pEventLB->GetModel()->InvalidateEntry( pE );
    pThis->pEventLB->Select( pE );
    pThis->pEventLB->MakeVisible( pE );
    pThis->pEventLB->SetUpdateMode( TRUE );

    pThis->EnableButtons( aLanguage );
    return 0;
}

IMPL_STATIC_LINK( _SfxMacroTabPage, ChangeScriptHdl_Impl, RadioButton*, EMPTYARG )
{
    pThis->ScriptChanged( pThis->pScriptTypeLB->GetSelectEntry() );
    return 0;
}

void _SfxMacroTabPage::InitAndSetHandler()
{
    // Handler installieren
    Link aLnk(STATIC_LINK(this, _SfxMacroTabPage, AssignDeleteHdl_Impl ));
    pMacroLB->SetDoubleClickHdl( aLnk );
    pDeletePB->SetClickHdl( aLnk );
    pAssignPB->SetClickHdl( aLnk );
    pEventLB->SetDoubleClickHdl( aLnk );

    pEventLB->SetSelectHdl( STATIC_LINK( this, _SfxMacroTabPage, SelectEvent_Impl ));
    pGroupLB->SetSelectHdl( STATIC_LINK( this, _SfxMacroTabPage, SelectGroup_Impl ));
    pMacroLB->SetSelectHdl( STATIC_LINK( this, _SfxMacroTabPage, SelectMacro_Impl ));

    pScriptTypeLB->SetSelectHdl( STATIC_LINK( this, _SfxMacroTabPage, ChangeScriptHdl_Impl ));

#if SUPD<582
    pJavaED->SetGetFocusHdl( STATIC_LINK( this, _SfxMacroTabPage, GetFocus_Impl ));
    pJavaED->SetModifyHdl( STATIC_LINK( this, _SfxMacroTabPage, GetFocus_Impl ));
#endif

    pEventLB->SetWindowBits( WB_HSCROLL|WB_CLIPCHILDREN );
    pEventLB->SetSelectionMode( SINGLE_SELECTION );
    pEventLB->SetTabs( &nTabs[0], MAP_APPFONT );
    pEventLB->Resize(); // OS: Hack fuer richtige Selektion
//  pEventLB->SetFont( SFX_APP()->GetAppFont() );
    pEventLB->SetSpaceBetweenEntries( 0 );

    pEventLB->Show();

    pEventLB->Enable( TRUE );
    pGroupLB->Enable( TRUE );
    pMacroLB->Enable( TRUE );

    pScriptTypeLB->SetDropDownLineCount( 3 );
    pScriptTypeLB->InsertEntry( DEFINE_CONST_UNICODE("StarBasic") );

#if SUPD<582
    pScriptTypeLB->InsertEntry( DEFINE_CONST_UNICODE("StarScript") );
    pScriptTypeLB->InsertEntry( DEFINE_CONST_UNICODE("JavaScript") );
#endif

    pScriptTypeLB->SelectEntry( DEFINE_CONST_UNICODE("StarBasic") );

    pGroupLB->SetFunctionListBox( pMacroLB );
    FillMacroList();
}

void _SfxMacroTabPage::FillMacroList()
{
    String aLanguage = pScriptTypeLB->GetSelectEntry();
    if( ! aLanguage.EqualsAscii("JavaScript") )
    {
        // 2 Listboxen
        SvStringsDtor* pArr = (*fnGetRange)( this, DEFINE_CONST_UNICODE("StarBasic") );
        if( pArr )
        {
            pGroupLB->Init( pArr );
            delete pArr;
        }
    }
}

void _SfxMacroTabPage::FillEvents()
{
    String aLanguage = pScriptTypeLB->GetSelectEntry();

    // Events aus der Tabelle holen und die EventListBox entsprechen fuellen
    for( ULONG n = 0; n < pEventLB->GetEntryCount(); ++n )
    {
        SvLBoxEntry* pE = pEventLB->GetEntry( n );
        if( pE )
        {
            SvLBoxString* pLItem = (SvLBoxString*)pE->GetItem( LB_MACROS_ITEMPOS );
            DBG_ASSERT( pLItem && SV_ITEM_ID_LBOXSTRING == pLItem->IsA(), "kein LBoxString" );

            String sOld( pLItem->GetText() ), sNew;
            USHORT nEventId = (USHORT)(ULONG)pE->GetUserData();
            if( aTbl.IsKeyValid( nEventId ) )
            {
                // Event vorhanden
                sNew = ConvertToUIName_Impl( aTbl.Get( nEventId ), aLanguage );
            }

            if( sOld != sNew )
            {
                pE->ReplaceItem( new SvLBoxString( pE, 0, sNew ), LB_MACROS_ITEMPOS );
                pEventLB->GetModel()->InvalidateEntry( pE );
            }
        }
    }
}

void _SfxMacroTabPage::SelectEvent( const String & rEventName, USHORT nEventId )
{
/*
    String sTmp( rEventName );
    sTmp += '\t';

    // falls die Tabelle schon gueltig ist
    SvxMacro* pM = aTbl.Get( nEventId );
    if( pM )
    {
        String sNew( ConvertToUIName_Impl( pM, pScriptTypeLB->GetSelectEntry() ) );
        sTmp += sNew;
    }
*/
    for( ULONG n = 0; n < pEventLB->GetEntryCount(); ++n )
    {
        SvLBoxEntry* pE = pEventLB->GetEntry( n );
//      if( pE && pE->GetEntryText() == sTmp )
        if( pE && (USHORT) (ULONG) pE->GetUserData() == nEventId )
        {
            pEventLB->SetCurEntry( pE );
            pEventLB->MakeVisible( pE );
            break;
        }
    }
}


/*  */

SvStringsDtor* __EXPORT _ImpGetRangeHdl( _SfxMacroTabPage* pTbPg, const String& rLanguage )
{
    SvStringsDtor* pNew = new SvStringsDtor;
    SfxApplication* pSfxApp = SFX_APP();

    if ( !rLanguage.EqualsAscii("JavaScript") )
    {
        pSfxApp->EnterBasicCall();

        // AppBasic einf"ugen
        String* pNewEntry = new String( pSfxApp->GetName() );
        pNew->Insert( pNewEntry, pNew->Count() );

        // Aktuelles Dokument
        SfxObjectShell* pDoc = SfxObjectShell::Current();
        if ( pDoc )
        {
            String aTitle = pDoc->GetTitle();

            // Hack f"ur Aufruf aus der Basic-IDE : das Basic ermitteln, das
            // gerade bearbeitet wird

            String aAppName(DEFINE_CONST_UNICODE("BASIC - "));
            USHORT nLen = aAppName.Len();
            if ( aTitle.CompareIgnoreCaseToAscii( aAppName, nLen ) == COMPARE_EQUAL )
            {
                // Basic-Namensprefix entfernen
                aTitle.Erase( 0, nLen );
                USHORT nIndex=0, nCount=aTitle.GetTokenCount('.');
                if ( nCount > 1 )
                {
                    // Namen der Library entfernen
                    aTitle.GetToken( nCount-2, '.', nIndex );
                    aTitle.Erase( nIndex-1 );
                }

                // Wenn das App-Basic gerade in der Basic-IDE bearbeitet wird, kein
                // Dokument verwenden
                pDoc = SfxObjectShell::GetFirst();
                while( pDoc )
                {
                    if ( aTitle == pDoc->GetTitle() )
                        break;
                    pDoc = SfxObjectShell::GetNext( *pDoc );
                }
            }

            if ( pDoc && pDoc->GetBasicManager() != SFX_APP()->GetBasicManager() &&
                pDoc->GetBasicManager()->GetLibCount() )
            {
                pNewEntry = new String( aTitle );
                pNew->Insert( pNewEntry, pNew->Count() );
            }
        }

        pSfxApp->LeaveBasicCall();
    }

    return pNew;
}

// besorgen der Funktionen eines Bereiches
SvStringsDtor* __EXPORT _ImpGetMacrosOfRangeHdl( _SfxMacroTabPage* pTbPg,
                                const String& rLanguage, const String& rRange )
{
    SvStringsDtor* pNew = new SvStringsDtor;
    return pNew;
}

/*  */

SfxMacroTabPage::SfxMacroTabPage( Window* pParent, const ResId& rResId,
                                const SfxItemSet& rSet )
    : _SfxMacroTabPage( pParent, rResId, rSet )
{
    pEventGB = new GroupBox(this, SfxResId(GB_EVENT) );
    pEventLB = new SvTabListBox(this, SfxResId(LB_EVENT) );
    pAssignPB = new PushButton( this, SfxResId(PB_ASSIGN) );
    pDeletePB = new PushButton( this, SfxResId(PB_DELETE) );
    pScriptTypeLB = new ListBox(this, SfxResId(LB_SCRIPTTYPE) );
    pMacroGB = new GroupBox( this, SfxResId(GB_MACRO) );
    pGroupLB = new SfxConfigGroupListBox_Impl( this, SfxResId(LB_GROUP) );
    pMacroLB = new SfxConfigFunctionListBox_Impl( this, SfxResId(LB_MACROS) );
    pMacroStr = new String( SfxResId(STR_MACROS) );
#if SUPD<582
    pJavaStr = new String( SfxResId(STR_JAVASCRIPT) );
    pJavaED = new MultiLineEdit( this, SfxResId(ED_JAVASCRIPT) );
#endif

    FreeResource();
//  SetExchangeSupport();

    InitAndSetHandler();

    ScriptChanged( DEFINE_CONST_UNICODE("StarBasic") );
}

SfxTabPage* SfxMacroTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new SfxMacroTabPage( pParent, SfxResId( RID_SFX_TP_MACROASSIGN), rAttrSet );
}

/*  */

SfxSmallMacroTabPage::SfxSmallMacroTabPage( Window* pParent, const ResId& rResId,
                                const SfxItemSet& rSet )
    : _SfxMacroTabPage( pParent, rResId, rSet )
{
    pEventGB = new GroupBox(this, SfxResId(GB_EVENT) );
    pEventLB = new SvTabListBox(this, SfxResId(LB_EVENT) );
    pAssignPB = new PushButton( this, SfxResId(PB_ASSIGN) );
    pDeletePB = new PushButton( this, SfxResId(PB_DELETE) );
    pScriptTypeLB = new ListBox(this, SfxResId(LB_SCRIPTTYPE) );
    pMacroGB = new GroupBox( this, SfxResId(GB_MACRO) );
    pGroupLB = new SfxConfigGroupListBox_Impl( this, SfxResId(LB_GROUP) );
    pMacroLB = new SfxConfigFunctionListBox_Impl( this, SfxResId(LB_MACROS) );
    pMacroStr = new String( SfxResId(STR_MACROS) );
#if SUPD<582
    pJavaStr = new String( SfxResId(STR_JAVASCRIPT) );
    pJavaED = new MultiLineEdit( this, SfxResId(ED_JAVASCRIPT) );
#endif
    FreeResource();
//  SetExchangeSupport();

    InitAndSetHandler();

    ScriptChanged( DEFINE_CONST_UNICODE("StarBasic") );
}

SfxTabPage* SfxSmallMacroTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new SfxMacroTabPage( pParent, SfxResId( RID_SFX_SMALLTP_MACROASSIGN), rAttrSet );
}

/*  */

SfxMacroAssignDlg::SfxMacroAssignDlg( Window* pParent, SfxItemSet& rSet )
    : SfxSingleTabDialog( pParent, rSet, 0 )
{
    SetTabPage( SfxMacroTabPage::Create( this, rSet ) );
}

SfxMacroAssignDlg::~SfxMacroAssignDlg()
{
}



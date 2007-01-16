/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basicbox.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: vg $ $Date: 2007-01-16 16:27:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basctl.hxx"

#include <ide_pch.hxx>


#include <basidesh.hrc>
#include <basidesh.hxx>
#include <basobj.hxx>

#include <basicbox.hxx>
#include <iderid.hxx>
#include <iderdll.hxx>
#include <bastypes.hxx>

#ifndef _BASTYPE2_HXX
#include "bastype2.hxx"
#endif
#ifndef _BASDOC_HXX
#include "basdoc.hxx"
#endif

#include "localizationmgr.hxx"
#include "managelang.hxx"
#include "dlgresid.hrc"

#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _SVX_LANGTAB_HXX
#include <svx/langtab.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

SFX_IMPL_TOOLBOX_CONTROL( LibBoxControl, SfxStringItem );

LibBoxControl::LibBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx )
    : SfxToolBoxControl( nSlotId, nId, rTbx )
{
}



LibBoxControl::~LibBoxControl()
{
}



void LibBoxControl::StateChanged( USHORT, SfxItemState eState, const SfxPoolItem* pState )
{
    BasicLibBox* pBox = (BasicLibBox*) GetToolBox().GetItemWindow( GetId() );

    DBG_ASSERT( pBox, "Box not found" );
    if ( !pBox )
        return;

    if ( eState != SFX_ITEM_AVAILABLE )
        pBox->Disable();
    else
    {
        pBox->Enable();

        if ( pState->ISA(SfxStringItem) )
            pBox->Update( (const SfxStringItem*)pState );
        else
            pBox->Update( NULL );
    }
}



Window* LibBoxControl::CreateItemWindow( Window *pParent )
{
    return new BasicLibBox( pParent, m_xFrame );
}

BasicLibBox::BasicLibBox( Window* pParent, const uno::Reference< frame::XFrame >& rFrame ) :
    ListBox( pParent, WinBits( WB_BORDER | WB_DROPDOWN ) ),
    m_xFrame( rFrame )
{
    FillBox();
    bIgnoreSelect = TRUE;   // Select von 0 noch nicht weiterleiten
    bFillBox = TRUE;
    SelectEntryPos( 0 );
    aCurText = GetEntry( 0 );
    SetSizePixel( Size( 250, 200 ) );
    bIgnoreSelect = FALSE;
    StartListening( *SFX_APP(), TRUE /* Nur einmal anmelden */ );
}



__EXPORT BasicLibBox::~BasicLibBox()
{
    ClearBox();
}

void __EXPORT BasicLibBox::Update( const SfxStringItem* pItem )
{
    // Immer auf dem laufenden sein...
//  if ( !pItem  || !pItem->GetValue().Len() )
        FillBox();

    if ( pItem )
    {
        aCurText = pItem->GetValue();
        if ( aCurText.Len() == 0 )
            aCurText = String( IDEResId( RID_STR_ALL ) );
    }

    if ( GetSelectEntry() != aCurText )
        SelectEntry( aCurText );
}

void __EXPORT BasicLibBox::ReleaseFocus()
{
    SfxViewShell* pCurSh = SfxViewShell::Current();
    DBG_ASSERT( pCurSh, "Current ViewShell not found!" );

    if ( pCurSh )
    {
        Window* pShellWin = pCurSh->GetWindow();
        if ( !pShellWin )       // sonst werde ich ihn nicht los
            pShellWin = Application::GetDefDialogParent();

        pShellWin->GrabFocus();
    }
}



void __EXPORT BasicLibBox::SFX_NOTIFY(  SfxBroadcaster&, const TypeId&,
                                        const SfxHint& rHint, const TypeId& )
{
    if ( rHint.IsA( TYPE( SfxEventHint ) ) )
    {
        switch ( ((SfxEventHint&)rHint).GetEventId() )
        {
            case SFX_EVENT_CREATEDOC:
            case SFX_EVENT_OPENDOC:
            {
                FillBox();  // IDE reagiert selbst, wenn == aktuelle Lib
            }
            break;
            case SFX_EVENT_SAVEASDOC:
            {
                FillBox( TRUE );
            }
            break;
            case SFX_EVENT_CLOSEDOC:
            {
                if ( SFX_APP()->IsInBasicCall() )   // Nicht wenn Office beendet
                    FillBox();
            }
            break;
        }
    }
}

void BasicLibBox::FillBox( BOOL bSelect )
{
    SetUpdateMode( FALSE );
    bIgnoreSelect = TRUE;

    aCurText = GetSelectEntry();

    SelectEntryPos( 0 );
    ClearBox();

    // create list box entries
    USHORT nPos = InsertEntry( String( IDEResId( RID_STR_ALL ) ), LISTBOX_APPEND );
    SetEntryData( nPos, new BasicLibEntry( 0, LIBRARY_LOCATION_UNKNOWN, String() ) );
    InsertEntries( 0, LIBRARY_LOCATION_USER );
    InsertEntries( 0, LIBRARY_LOCATION_SHARE );
    SfxObjectShell* pShell = SfxObjectShell::GetFirst();
    while ( pShell )
    {
        // only if there's a corresponding window (not for remote documents)
        if ( SfxViewFrame::GetFirst( pShell ) && !pShell->ISA( BasicDocShell ) )
            InsertEntries( pShell, LIBRARY_LOCATION_DOCUMENT );

        pShell = SfxObjectShell::GetNext( *pShell );
    }

    SetUpdateMode( TRUE );

    if ( bSelect )
    {
        SelectEntry( aCurText );
        if ( !GetSelectEntryCount() )
        {
            SelectEntryPos( GetEntryCount() );  // gibst es nicht => leer?
            aCurText = GetSelectEntry();
        }
    }
    bIgnoreSelect = FALSE;
}

void BasicLibBox::InsertEntries( SfxObjectShell* pShell, LibraryLocation eLocation )
{
    // get a sorted list of library names
    Sequence< ::rtl::OUString > aLibNames = BasicIDE::GetLibraryNames( pShell );
    sal_Int32 nLibCount = aLibNames.getLength();
    const ::rtl::OUString* pLibNames = aLibNames.getConstArray();

    for ( sal_Int32 i = 0 ; i < nLibCount ; ++i )
    {
        String aLibName = pLibNames[ i ];
        if ( eLocation == BasicIDE::GetLibraryLocation( pShell, aLibName ) )
        {
            String aName( BasicIDE::GetTitle( pShell, eLocation, SFX_TITLE_CAPTION ) );
            String aEntryText( CreateMgrAndLibStr( aName, aLibName ) );
            USHORT nPos = InsertEntry( aEntryText, LISTBOX_APPEND );
            SetEntryData( nPos, new BasicLibEntry( pShell, eLocation, aLibName ) );
        }
    }
}

long BasicLibBox::PreNotify( NotifyEvent& rNEvt )
{
    long nDone = 0;
    if( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        KeyEvent aKeyEvt = *rNEvt.GetKeyEvent();
        USHORT nKeyCode = aKeyEvt.GetKeyCode().GetCode();
        switch( nKeyCode )
        {
            case KEY_RETURN:
            {
                NotifyIDE();
                nDone = 1;
            }
            break;

            case KEY_ESCAPE:
            {
                SelectEntry( aCurText );
                ReleaseFocus();
                nDone = 1;
            }
            break;
        }
    }
    else if( rNEvt.GetType() == EVENT_GETFOCUS )
    {
        if ( bFillBox )
        {
            FillBox();
            bFillBox = FALSE;
        }
    }
    else if( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( !HasChildPathFocus( TRUE ) )
        {
            bIgnoreSelect = TRUE;
            bFillBox = TRUE;
        }
    }

    return nDone ? nDone : ListBox::PreNotify( rNEvt );
}

void __EXPORT BasicLibBox::Select()
{
    if ( !IsTravelSelect() )
    {
        if ( !bIgnoreSelect )
            NotifyIDE();
        else
            SelectEntry( aCurText );    // Seit 306... (Select nach Escape)
    }
}

void BasicLibBox::NotifyIDE()
{
    USHORT nSelPos = GetSelectEntryPos();
    BasicLibEntry* pEntry = (BasicLibEntry*)GetEntryData( nSelPos );
    if ( pEntry )
    {
        SfxObjectShell* pShell = pEntry->GetShell();
        SfxObjectShellItem aShellItem( SID_BASICIDE_ARG_SHELL, pShell );
        String aLibName = pEntry->GetLibName();
        SfxStringItem aLibNameItem( SID_BASICIDE_ARG_LIBNAME, aLibName );
        BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
        SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
        SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
        if ( pDispatcher )
        {
            pDispatcher->Execute( SID_BASICIDE_LIBSELECTED,
                                  SFX_CALLMODE_SYNCHRON, &aShellItem, &aLibNameItem, 0L );
        }
    }
    ReleaseFocus();
}

void BasicLibBox::ClearBox()
{
    USHORT nCount = GetEntryCount();
    for ( USHORT i = 0; i < nCount; ++i )
    {
        BasicLibEntry* pEntry = (BasicLibEntry*)GetEntryData( i );
        delete pEntry;
    }
    ListBox::Clear();
}

// class LanguageBoxControl ----------------------------------------------

SFX_IMPL_TOOLBOX_CONTROL( LanguageBoxControl, SfxStringItem );

LanguageBoxControl::LanguageBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx )
    : SfxToolBoxControl( nSlotId, nId, rTbx )
{
}

LanguageBoxControl::~LanguageBoxControl()
{
}

void LanguageBoxControl::StateChanged( USHORT _nID, SfxItemState _eState, const SfxPoolItem* _pItem )
{
    (void)_nID;

    BasicLanguageBox* pBox = (BasicLanguageBox*)( GetToolBox().GetItemWindow( GetId() ) );

    if ( pBox )
    {
        if ( _eState != SFX_ITEM_AVAILABLE )
            pBox->Disable();
        else
        {
            pBox->Enable();
            if ( _pItem->ISA(SfxStringItem) )
                pBox->Update( (const SfxStringItem*)_pItem );
            else
                pBox->Update( NULL );
        }
    }
}

Window* LanguageBoxControl::CreateItemWindow( Window *pParent )
{
    return new BasicLanguageBox( pParent );
}

// class BasicLanguageBox ------------------------------------------------

BasicLanguageBox::BasicLanguageBox( Window* pParent ) :

    ListBox( pParent, WinBits( WB_BORDER | WB_DROPDOWN ) ),

    m_sNotLocalizedStr( IDEResId( RID_STR_TRANSLATION_NOTLOCALIZED ) ),
    m_sDefaultLanguageStr( IDEResId( RID_STR_TRANSLATION_DEFAULT ) ),

    m_bIgnoreSelect( false )

{
    SetSizePixel( Size( 210, 200 ) );

    FillBox();
    StartListening( *SFX_APP(), TRUE /* Nur einmal anmelden */ );
}

BasicLanguageBox::~BasicLanguageBox()
{
    ClearBox();
}

void BasicLanguageBox::FillBox()
{
    SetUpdateMode( FALSE );
    m_bIgnoreSelect = true;
    m_sCurrentText = GetSelectEntry();
    ClearBox();

    LocalizationMgr* pCurMgr = IDE_DLL()->GetShell()->GetCurLocalizationMgr();
    if ( pCurMgr->isLibraryLocalized() )
    {
        Enable();
        SvxLanguageTable aLangTable;
        Locale aDefaultLocale = pCurMgr->getStringResourceManager()->getDefaultLocale();
        Locale aCurrentLocale = pCurMgr->getStringResourceManager()->getCurrentLocale();
        Sequence< Locale > aLocaleSeq = pCurMgr->getStringResourceManager()->getLocales();
        const Locale* pLocale = aLocaleSeq.getConstArray();
        INT32 i, nCount = aLocaleSeq.getLength();
        USHORT nSelPos = LISTBOX_ENTRY_NOTFOUND;
        for ( i = 0;  i < nCount;  ++i )
        {
            bool bIsDefault = localesAreEqual( aDefaultLocale, pLocale[i] );
            bool bIsCurrent = localesAreEqual( aCurrentLocale, pLocale[i] );
            LanguageType eLangType = SvxLocaleToLanguage( pLocale[i] );
            String sLanguage = aLangTable.GetString( eLangType );
            if ( bIsDefault )
            {
                sLanguage += ' ';
                sLanguage += m_sDefaultLanguageStr;
            }
            USHORT nPos = InsertEntry( sLanguage );
            SetEntryData( nPos, new LanguageEntry( sLanguage, pLocale[i], bIsDefault ) );

            if ( bIsCurrent )
                nSelPos = nPos;
        }

        if ( nSelPos != LISTBOX_ENTRY_NOTFOUND )
        {
            SelectEntryPos( nSelPos );
            m_sCurrentText = GetSelectEntry();
        }
    }
    else
    {
        InsertEntry( m_sNotLocalizedStr );
        SelectEntryPos(0);
        Disable();
    }

    SetUpdateMode( TRUE );
    m_bIgnoreSelect = false;
}

void BasicLanguageBox::ClearBox()
{
    USHORT nCount = GetEntryCount();
    for ( USHORT i = 0; i < nCount; ++i )
    {
        LanguageEntry* pEntry = (LanguageEntry*)GetEntryData(i);
        delete pEntry;
    }
    ListBox::Clear();
}

void BasicLanguageBox::SetLanguage()
{
    LanguageEntry* pEntry = (LanguageEntry*)GetEntryData( GetSelectEntryPos() );
    if ( pEntry )
        IDE_DLL()->GetShell()->GetCurLocalizationMgr()->handleSetCurrentLocale( pEntry->m_aLocale );
}

void BasicLanguageBox::Select()
{
    if ( !m_bIgnoreSelect )
        SetLanguage();
    else
        SelectEntry( m_sCurrentText );  // Select after Escape
}

long BasicLanguageBox::PreNotify( NotifyEvent& rNEvt )
{
    long nDone = 0;
    if( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        USHORT nKeyCode = rNEvt.GetKeyEvent()->GetKeyCode().GetCode();
        switch( nKeyCode )
        {
            case KEY_RETURN:
            {
                SetLanguage();
                nDone = 1;
            }
            break;

            case KEY_ESCAPE:
            {
                SelectEntry( m_sCurrentText );
                nDone = 1;
            }
            break;
        }
    }
    else if( rNEvt.GetType() == EVENT_GETFOCUS )
    {
    }
    else if( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
    }

    return nDone ? nDone : ListBox::PreNotify( rNEvt );
}

void BasicLanguageBox::SFX_NOTIFY( SfxBroadcaster&, const TypeId&, const SfxHint& rHint, const TypeId& )
{
    if ( rHint.IsA( TYPE( SfxEventHint ) ) )
    {
        USHORT nEventId = ( (SfxEventHint&)rHint ).GetEventId();
        switch ( nEventId )
        {
            case SFX_EVENT_CREATEDOC:
            case SFX_EVENT_OPENDOC:
            case SFX_EVENT_SAVEASDOC:
            case SFX_EVENT_CLOSEDOC:
            {
                if ( nEventId != SFX_EVENT_CLOSEDOC || SFX_APP()->IsInBasicCall() )
                    FillBox();
            }
            break;
        }
    }
}

void BasicLanguageBox::Update( const SfxStringItem* pItem )
{
    FillBox();

    if ( pItem && pItem->GetValue().Len() > 0 )
    {
        m_sCurrentText = pItem->GetValue();
        if ( GetSelectEntry() != m_sCurrentText )
            SelectEntry( m_sCurrentText );
    }
}


/*************************************************************************
 *
 *  $RCSfile: menu.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obr $ $Date: 2001-02-14 08:29:04 $
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

#define _SV_MENU_CXX

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_MNEMONIC_HXX
#include <mnemonic.hxx>
#endif
#ifndef _SV_IMAGE_HXX
#include <image.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_HELP_HXX
#include <help.hxx>
#endif
#ifndef _SV_SVIDS_HRC
#include <svids.hrc>
#endif
#ifndef _SV_FLOATWIN_HXX
#include <floatwin.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <wrkwin.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <timer.hxx>
#endif
#ifndef _SV_SOUND_HXX
#include <sound.hxx>
#endif
#ifndef _SV_DECOVIEW_HXX
#include <decoview.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <bitmap.hxx>
#endif
#ifndef _SV_RC_H
#include <rc.h>
#endif
#ifndef _SV_MENU_HXX
#include <menu.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <button.hxx>
#endif
#ifndef _SV_GRADIENT_HXX
#include <gradient.hxx>
#endif
#ifndef _SV_ACCESS_HXX
#include <access.hxx>
#endif

#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif

#pragma hdrstop

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_I18N_XCHARACTERCLASSIFICATION_HPP_
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#endif

#include <unohelp.hxx>

using namespace ::com::sun::star;

DBG_NAME( Menu );

#define ITEMPOS_INVALID     0xFFFF

#define EXTRASPACEY         2
#define EXTRAITEMHEIGHT     4

#ifndef TF_SVDATA
DropEvent ImplTranslateDropEvent( const DropEvent& rE, Window* pSource, Window* pDest )
{
    Point aPos = pSource->OutputToScreenPixel( rE.GetPosPixel() );
    aPos = pDest->ScreenToOutputPixel( aPos );
    return DropEvent( aPos, rE.GetData(), rE.GetAction(), rE.GetSourceOptions(),
                rE.GetWindowType(), rE.IsDefaultAction() );
}
#endif

inline BOOL ImplIsMouseFollow()
{
    return ( Application::GetSettings().GetMouseSettings().GetFollow() & MOUSE_FOLLOW_MENU ) ? TRUE : FALSE;
}


struct MenuItemData
{
    USHORT          nId;            // SV Id
    MenuItemType    eType;          // MenuItem-Type
    MenuItemBits    nBits;          // MenuItem-Bits
    Menu*           pSubMenu;       // Pointer auf das SubMenu
    Menu*           pAutoSubMenu;   // Pointer auf SubMenu aus Resource
    XubString       aText;          // Menu-Text
    XubString       aHelpText;      // Help-String
    XubString       aCommandStr;    // CommandString
    ULONG           nHelpId;        // Help-Id
    Image           aImage;         // Image
    KeyCode         aAccelKey;      // Accelerator-Key
    BOOL            bChecked;       // Checked
    BOOL            bEnabled;       // Enabled
    BOOL            bIsTemporary;   // Temporary inserted ('No selection possible')

    Size            aSz;            // nur temporaer gueltig

                    MenuItemData() {}
                    MenuItemData( const XubString& rStr, const Image& rImage ) :
                        aText( rStr ),
                        aImage( rImage )
                    {}
                    ~MenuItemData() { delete pAutoSubMenu; }
};


class MenuItemList : public List
{
    uno::Reference< i18n::XCharacterClassification > xCharClass;
public:
                    MenuItemList() : List( 16, 4 ) {}
                    ~MenuItemList();

    MenuItemData*   Insert( USHORT nId, MenuItemType eType, MenuItemBits nBits,
                            const XubString& rStr, const Image& rImage,
                            Menu* pMenu, USHORT nPos );
    void            InsertSeparator( USHORT nPos );
    void            Remove( USHORT nPos );


    MenuItemData*   GetData( USHORT nSVId, USHORT& rPos ) const;
    MenuItemData*   GetData( USHORT nSVId ) const
                        { USHORT nTemp; return GetData( nSVId, nTemp ); }
    MenuItemData*   GetDataFromPos( ULONG nPos ) const
                        { return (MenuItemData*)List::GetObject( nPos ); }

    MenuItemData*   SearchItem( xub_Unicode cSelectChar, USHORT& rPos ) const;
    uno::Reference< i18n::XCharacterClassification > GetCharClass() const;
};



MenuItemList::~MenuItemList()
{
    for ( ULONG n = Count(); n; )
    {
        MenuItemData* pData = GetDataFromPos( --n );
        delete pData;
    }
}

MenuItemData* MenuItemList::Insert( USHORT nId, MenuItemType eType,
                                    MenuItemBits nBits,
                                    const XubString& rStr, const Image& rImage,
                                    Menu* pMenu, USHORT nPos )
{
    MenuItemData* pData = new MenuItemData( rStr, rImage );
    pData->nId          = nId;
    pData->eType        = eType;
    pData->nBits        = nBits;
    pData->pSubMenu     = NULL;
    pData->pAutoSubMenu = NULL;
    pData->nHelpId      = 0;
    pData->bChecked     = FALSE;
    pData->bEnabled     = TRUE;
    pData->bIsTemporary = FALSE;
    List::Insert( (void*)pData, nPos );
    return pData;
}

void MenuItemList::InsertSeparator( USHORT nPos )
{
    MenuItemData* pData = new MenuItemData;
    pData->nId          = 0;
    pData->eType        = MENUITEM_SEPARATOR;
    pData->nBits        = 0;
    pData->pSubMenu     = NULL;
    pData->pAutoSubMenu = NULL;
    pData->nHelpId      = 0;
    pData->bChecked     = FALSE;
    pData->bEnabled     = TRUE;
    pData->bIsTemporary = FALSE;
    List::Insert( (void*)pData, nPos );
}

void MenuItemList::Remove( USHORT nPos )
{
    MenuItemData* pData = (MenuItemData*)List::Remove( (ULONG)nPos );
    if ( pData )
        delete pData;
}

MenuItemData* MenuItemList::GetData( USHORT nSVId, USHORT& rPos ) const
{
    rPos = 0;
    MenuItemData* pData = (MenuItemData*)GetObject( rPos );
    while ( pData )
    {
        if ( pData->nId == nSVId )
            return pData;

        rPos++;
        pData = (MenuItemData*)GetObject( rPos );
    }

    return NULL;
}

MenuItemData* MenuItemList::SearchItem( xub_Unicode cSelectChar, USHORT& rPos ) const
{
    const ::com::sun::star::lang::Locale& rLocale = Application::GetSettings().GetLocale();
    xub_Unicode cCharCode = GetCharClass()->toUpper( String(cSelectChar), 0, 1, rLocale )[0];
    for ( rPos = (USHORT)Count(); rPos; )
    {
        MenuItemData* pData = GetDataFromPos( --rPos );
        if ( pData->bEnabled )
        {
            USHORT n = pData->aText.Search( '~' );
            if ( n != STRING_NOTFOUND )
            {
                xub_Unicode cCompareChar = pData->aText.GetChar( n+1 );
                cCompareChar = GetCharClass()->toUpper( String(cCompareChar), 0, 1, rLocale )[0];
                if ( cCompareChar == cCharCode )
                    return pData;
            }
        }
    }

    return 0;
}

uno::Reference< i18n::XCharacterClassification > MenuItemList::GetCharClass() const
{
    if ( !xCharClass.is() )
        ((MenuItemList*)this)->xCharClass = vcl::unohelper::CreateCharacterClassification();
    return xCharClass;
}



class MenuFloatingWindow : public FloatingWindow
{
private:
    Menu*           pMenu;
    PopupMenu*      pActivePopup;
    Timer           aHighlightChangedTimer;
    Timer           aScrollTimer;
    ULONG           nSaveFocusId;
    long            nStartY;
    USHORT          nHighlightedItem;       // gehighlightetes/selektiertes Item
    USHORT          nMBDownPos;
    USHORT          nScrollerHeight;
    USHORT          nFirstEntry;
    USHORT          nBorder;
    BOOL            bInExecute;

    BOOL            bScrollMenu;
    BOOL            bScrollUp;
    BOOL            bScrollDown;

                    DECL_LINK( PopupEnd, FloatingWindow* );
                    DECL_LINK( HighlightChanged, Timer* );
                    DECL_LINK( AutoScroll, Timer* );

    void            StateChanged( StateChangedType nType );
    void            DataChanged( const DataChangedEvent& rDCEvt );

protected:
    Region          ImplCalcClipRegion( BOOL bIncludeLogo = TRUE ) const;
    void            ImplInitClipRegion();
    void            ImplDrawScroller( BOOL bUp );
    void            ImplScroll( const Point& rMousePos );
    void            ImplScroll( BOOL bUp );
    void            ImplCursorUpDown( BOOL bUp );
    void            ImplHighlightItem( const MouseEvent& rMEvt, BOOL bMBDown );

public:
                    MenuFloatingWindow( Menu* pMenu, Window* pParent, WinBits nStyle );
                    ~MenuFloatingWindow();

    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    KeyInput( const KeyEvent& rKEvent );
    virtual void    Command( const CommandEvent& rCEvt );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    RequestHelp( const HelpEvent& rHEvt );
    virtual void    Resize();

    void            SetFocusId( ULONG nId ) { nSaveFocusId = nId; }
    ULONG           GetFocusId() const      { return nSaveFocusId; }

    void            EnableScrollMenu( BOOL b );
    BOOL            IsScrollMenu() const        { return bScrollMenu; }
    USHORT          GetScrollerHeight() const   { return nScrollerHeight; }

    void            Execute();
    void            StopExecute( ULONG nFocusId = 0 );
    void            EndExecute();
    void            EndExecute( USHORT nSelectId );

    PopupMenu*      GetActivePopup() const  { return pActivePopup; }
    void            KillActivePopup( PopupMenu* pThisOnly = NULL );

    void            HighlightItem( USHORT nPos, BOOL bHighlight );
    void            ChangeHighlightItem( USHORT n, BOOL bStartPopupTimer );
};


// Eine Basicklasse fuer beide (wegen pActivePopup, Timer, ...) waere nett,
// aber dann musste eine 'Container'-Klasse gemacht werden, da von
// unterschiedlichen Windows abgeleitet...
// In den meisten Funktionen muessen dann sowieso Sonderbehandlungen fuer
// MenuBar, PopupMenu gemacht werden, also doch zwei verschiedene Klassen.

class MenuBarWindow : public Window
{
    friend class MenuBar;

private:
    Menu*           pMenu;
    PopupMenu*      pActivePopup;
    USHORT          nHighlightedItem;
    ULONG           nSaveFocusId;
    BOOL            mbAutoPopup;

    PushButton      aCloser;
    PushButton      aFloatBtn;
    PushButton      aHideBtn;

    void            HighlightItem( USHORT nPos, BOOL bHighlight );
    void            ChangeHighlightItem( USHORT n, BOOL bSelectPopupEntry, BOOL bAllowRestoreFocus = TRUE );

    USHORT          ImplFindEntry( const Point& rMousePos ) const;
    void            ImplCreatePopup( BOOL bPreSelectFirst );
    BOOL            ImplHandleKeyEvent( const KeyEvent& rKEvent, BOOL bFromMenu = TRUE );

                    DECL_LINK( CloserHdl, PushButton* );
                    DECL_LINK( FloatHdl, PushButton* );
                    DECL_LINK( HideHdl, PushButton* );

    void            StateChanged( StateChangedType nType );
    void            DataChanged( const DataChangedEvent& rDCEvt );
    void            LoseFocus();

public:
                    MenuBarWindow( Window* pParent );
                    ~MenuBarWindow();

    void            ShowButtons( BOOL bClose, BOOL bFloat, BOOL bHide );

    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    KeyInput( const KeyEvent& rKEvent );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    Resize();
    virtual void    RequestHelp( const HelpEvent& rHEvt );
#ifndef TF_SVDATA
    virtual BOOL    QueryDrop( DropEvent& rDEvt );
    virtual BOOL    Drop( const DropEvent& rDEvt );
#endif

    void            SetFocusId( ULONG nId ) { nSaveFocusId = nId; }
    ULONG           GetFocusId() const { return nSaveFocusId; }

    void            SetMenu( MenuBar* pMenu );
    void            KillActivePopup();
    PopupMenu*      GetActivePopup() const  { return pActivePopup; }
    void            PopupClosed( Menu* pMenu );
};



static void ImplSetMenuItemData( MenuItemData* pData, USHORT nPos )
{
    // Daten umsetzen
    if ( !pData->aImage )
        pData->eType = MENUITEM_STRING;
    else if ( !pData->aText.Len() )
        pData->eType = MENUITEM_IMAGE;
    else
        pData->eType = MENUITEM_STRINGIMAGE;
}

static BOOL ImplHandleHelpEvent( Window* pMenuWindow, Menu* pMenu, USHORT nHighlightedItem, const HelpEvent& rHEvt )
{
    BOOL bDone = FALSE;
    USHORT nId = 0;
    if ( nHighlightedItem != ITEMPOS_INVALID )
    {
        MenuItemData* pItemData = pMenu->GetItemList()->GetDataFromPos( nHighlightedItem );
        if ( pItemData )
            nId = pItemData->nId;
    }

    if ( ( rHEvt.GetMode() & HELPMODE_BALLOON ) && pMenuWindow )
    {
        Point aPos = rHEvt.GetMousePosPixel();
        // Pos etwas nach unter-rechts korrigieren, wegen Pointer
        aPos.X() += 15;
//      aPos.Y() += 20;
        Help::ShowBalloon( pMenuWindow, aPos, pMenu->GetHelpText( nId ) );
        bDone = TRUE;
    }
    else if ( rHEvt.GetMode() & (HELPMODE_CONTEXT | HELPMODE_EXTENDED) )
    {
        // Ist eine Hilfe in die Applikation selektiert
        Help* pHelp = Application::GetHelp();
        if ( pHelp )
        {
            // Ist eine ID vorhanden, dann Hilfe mit der ID aufrufen, sonst
            // den Hilfe-Index
            ULONG nHelpId = pMenu->GetHelpId( nId );
            if ( nHelpId )
                pHelp->Start( nHelpId );
            else
                pHelp->Start( HELP_INDEX );
        }
        bDone = TRUE;
    }
    return bDone;
}


Menu::Menu()
{
    DBG_CTOR( Menu, NULL );
    ImplInit();
}

Menu::~Menu()
{
    DBG_DTOR( Menu, NULL );

    if ( nEventId )
        Application::RemoveUserEvent( nEventId );

    bKilled = TRUE;

    delete pItemList;
    delete pLogo;
}

void Menu::ImplInit()
{
    nMenuFlags      = 0;
    nDefaultItem    = 0;
    bIsMenuBar      = FALSE;
    nSelectedId     = 0;
    pItemList       = new MenuItemList;
    pLogo           = NULL;
    pStartedFrom    = NULL;
    pWindow         = NULL;
    nEventId        = 0;
    bCanceled       = FALSE;
    bInCallback     = FALSE;
    bKilled         = FALSE;
}

void Menu::ImplLoadRes( const ResId& rResId )
{
    rResId.SetRT( RSC_MENU );
    GetRes( rResId );

    USHORT nObjMask = ReadShortRes();

    if( nObjMask & RSC_MENU_ITEMS )
    {
        USHORT nObjFollows = ReadShortRes();
        // MenuItems einfuegen
        for( USHORT i = 0; i < nObjFollows; i++ )
        {
            InsertItem( ResId( (RSHEADER_TYPE*)GetClassRes() ) );
            IncrementRes( GetObjSizeRes( (RSHEADER_TYPE*)GetClassRes() ) );
        }
    }

    if( nObjMask & RSC_MENU_TEXT )
    {
        if( bIsMenuBar ) // Kein Titel im Menubar
            ReadStringRes();
        else
            aTitleText = ReadStringRes();
    }
    if( nObjMask & RSC_MENU_DEFAULTITEMID )
        SetDefaultItem( ReadShortRes() );
}

void Menu::Activate()
{
    bInCallback = TRUE;
    if ( !aActivateHdl.Call( this ) )
    {
        Menu* pStartMenu = ImplGetStartMenu();
        if ( pStartMenu && ( pStartMenu != this ) )
        {
            pStartMenu->bInCallback = TRUE;
            pStartMenu->aActivateHdl.Call( this );
            pStartMenu->bInCallback = FALSE;
        }
    }
    bInCallback = FALSE;
}

void Menu::Deactivate()
{
    for ( USHORT n = pItemList->Count(); n; )
    {
        MenuItemData* pData = pItemList->GetDataFromPos( --n );
        if ( pData->bIsTemporary )
            pItemList->Remove( n );
    }

    bInCallback = TRUE;
    Menu* pStartMenu = ImplGetStartMenu();
    if ( !aDeactivateHdl.Call( this ) )
    {
        if ( pStartMenu && ( pStartMenu != this ) )
        {
            pStartMenu->bInCallback = TRUE;
            pStartMenu->aDeactivateHdl.Call( this );
            pStartMenu->bInCallback = FALSE;
        }
    }
    bInCallback = FALSE;

    if ( this == pStartMenu )
        GetpApp()->HideHelpStatusText();
}

void Menu::Highlight()
{
    Menu* pStartMenu = ImplGetStartMenu();
    if ( !aHighlightHdl.Call( this ) )
    {
        if ( pStartMenu && ( pStartMenu != this ) )
            pStartMenu->aHighlightHdl.Call( this );
    }

    if ( GetCurItemId() )
        GetpApp()->ShowHelpStatusText( GetHelpText( GetCurItemId() ) );
}

void Menu::ImplSelect()
{
    MenuItemData* pData = GetItemList()->GetData( nSelectedId );
    if ( pData && (pData->nBits & MIB_AUTOCHECK) )
    {
        BOOL bChecked = IsItemChecked( nSelectedId );
        if ( pData->nBits & MIB_RADIOCHECK )
        {
            if ( !bChecked )
                CheckItem( nSelectedId, TRUE );
        }
        else
            CheckItem( nSelectedId, !bChecked );
    }

    // Select rufen
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mpActivePopupMenu = NULL;        // Falls neues Execute im Select()
    Application::PostUserEvent( nEventId, LINK( this, Menu, ImplCallSelect ) );
}

void Menu::Select()
{
    if ( !aSelectHdl.Call( this ) )
    {
        Menu* pStartMenu = ImplGetStartMenu();
        if ( pStartMenu && ( pStartMenu != this ) )
        {
            pStartMenu->nSelectedId = nSelectedId;
            pStartMenu->aSelectHdl.Call( this );
        }
    }
}

void Menu::RequestHelp( const HelpEvent& rHEvt )
{
}

void Menu::InsertItem( USHORT nItemId, const XubString& rStr, MenuItemBits nItemBits, USHORT nPos )
{
    DBG_ASSERT( nItemId, "Menu::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == MENU_ITEM_NOTFOUND,
                "Menu::InsertItem(): ItemId already exists" );

    // Falls Position > ItemCount, dann anheangen
    if ( nPos >= (USHORT)pItemList->Count() )
        nPos = MENU_APPEND;

    // Item in die MenuItemListe aufnehmen
    MenuItemData* pData = pItemList->Insert( nItemId, MENUITEM_STRING,
                             nItemBits, rStr, Image(), this, nPos );

    Window* pWin = ImplGetWindow();
    if ( pWin )
    {
        ImplCalcSize( pWin );
        if ( pWin->IsVisible() )
            pWin->Invalidate();
    }
}

void Menu::InsertItem( USHORT nItemId, const Image& rImage,
                       MenuItemBits nItemBits, USHORT nPos )
{
    InsertItem( nItemId, ImplGetSVEmptyStr(), nItemBits, nPos );
    SetItemImage( nItemId, rImage );
}

void Menu::InsertItem( USHORT nItemId,
                       const XubString& rStr, const Image& rImage,
                       MenuItemBits nItemBits, USHORT nPos )
{
    InsertItem( nItemId, rStr, nItemBits, nPos );
    SetItemImage( nItemId, rImage );
}

void Menu::InsertItem( const ResId& rResId, USHORT nPos )
{
    USHORT              nObjMask;

    GetRes( rResId.SetRT( RSC_MENUITEM ) );
    nObjMask    = ReadShortRes();

    BOOL bSep = FALSE;
    if ( nObjMask & RSC_MENUITEM_SEPARATOR )
        bSep = (BOOL)ReadShortRes();

    USHORT nItemId = 1;
    if ( nObjMask & RSC_MENUITEM_ID )
        nItemId = ReadShortRes();

    USHORT nStatus = 0;
    if ( nObjMask & RSC_MENUITEM_STATUS )
        nStatus = ReadShortRes();

    String aText;
    if ( nObjMask & RSC_MENUITEM_TEXT )
        aText = ReadStringRes();

    // Item erzeugen
    if ( nObjMask & RSC_MENUITEM_BITMAP )
    {
        if ( !bSep )
        {
            Bitmap aBmp( ResId( (RSHEADER_TYPE*)GetClassRes() ) );
            if ( aText.Len() )
                InsertItem( nItemId, aText, aBmp, nStatus, nPos );
            else
                InsertItem( nItemId, aBmp, nStatus, nPos );
        }
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE*)GetClassRes() ) );
    }
    else if ( !bSep )
        InsertItem( nItemId, aText, nStatus, nPos );
    if ( bSep )
        InsertSeparator( nPos );

    String aHelpText;
    if ( nObjMask & RSC_MENUITEM_HELPTEXT )
    {
        aHelpText = ReadStringRes();
        if( !bSep )
            SetHelpText( nItemId, aHelpText );
    }

    ULONG  nHelpId = 0;
    if ( nObjMask & RSC_MENUITEM_HELPID )
    {
        nHelpId = ReadLongRes();
        if ( !bSep )
            SetHelpId( nItemId, nHelpId );
    }

    if( !bSep /* && SvHelpSettings::HelpText( aHelpText, nHelpId ) */ )
        SetHelpText( nItemId, aHelpText );

    if ( nObjMask & RSC_MENUITEM_KEYCODE )
    {
        if ( !bSep )
            SetAccelKey( nItemId, KeyCode( ResId( (RSHEADER_TYPE*)GetClassRes() ) ) );
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE*)GetClassRes() ) );
    }
    if( nObjMask & RSC_MENUITEM_CHECKED )
    {
        if ( !bSep )
            CheckItem( nItemId, (BOOL)ReadShortRes() );
    }
    if ( nObjMask & RSC_MENUITEM_DISABLE )
    {
        if ( !bSep )
            EnableItem( nItemId, !(BOOL)ReadShortRes() );
    }
    if ( nObjMask & RSC_MENUITEM_COMMAND )
    {
        String aCommandStr = ReadStringRes();
        if ( !bSep )
            SetItemCommand( nItemId, aCommandStr );
    }
    if ( nObjMask & RSC_MENUITEM_MENU )
    {
        if ( !bSep )
        {
            MenuItemData* pData = GetItemList()->GetData( nItemId );
            if ( pData )
            {
                PopupMenu* pSubMenu = new PopupMenu( ResId( (RSHEADER_TYPE*)GetClassRes() ) );
                pData->pAutoSubMenu = pSubMenu;
                SetPopupMenu( nItemId, pSubMenu );
            }
        }
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE*)GetClassRes() ) );
    }
}

void Menu::InsertSeparator( USHORT nPos )
{
    // Handelt es sich um einen MenuBar, dann mache nichts
    if ( bIsMenuBar )
        return;

    // Falls Position > ItemCount, dann anheangen
    if ( nPos >= (USHORT)pItemList->Count() )
        nPos = MENU_APPEND;

    // Separator in die Item-Liste einfuegen
    pItemList->InsertSeparator( nPos );
}

void Menu::RemoveItem( USHORT nPos )
{
    if ( nPos < GetItemCount() )
        pItemList->Remove( nPos );

    Window* pWin = ImplGetWindow();
    if ( pWin )
    {
        ImplCalcSize( pWin );
        if ( pWin->IsVisible() )
            pWin->Invalidate();
    }
}

void ImplCopyItem( Menu* pThis, const Menu& rMenu, USHORT nPos, USHORT nNewPos,
                  USHORT nMode = 0 )
{
    MenuItemType eType = rMenu.GetItemType( nPos );

    if ( eType == MENUITEM_DONTKNOW )
        return;

    if ( eType == MENUITEM_SEPARATOR )
        pThis->InsertSeparator( nNewPos );
    else
    {
        USHORT nId = rMenu.GetItemId( nPos );

        DBG_ASSERT( pThis->GetItemPos( nId ) == MENU_ITEM_NOTFOUND,
                    "Menu::CopyItem(): ItemId already exists" );

        MenuItemData* pData = rMenu.GetItemList()->GetData( nId );

        if ( eType == MENUITEM_STRINGIMAGE )
            pThis->InsertItem( nId, pData->aText, pData->aImage, pData->nBits, nNewPos );
        else if ( eType == MENUITEM_STRING )
            pThis->InsertItem( nId, pData->aText, pData->nBits, nNewPos );
        else
            pThis->InsertItem( nId, pData->aImage, pData->nBits, nNewPos );

        if ( rMenu.IsItemChecked( nId ) )
            pThis->CheckItem( nId, TRUE );
        if ( !rMenu.IsItemEnabled( nId ) )
            pThis->EnableItem( nId, FALSE );
        pThis->SetHelpId( nId, pData->nHelpId );
        pThis->SetHelpText( nId, pData->aHelpText );
        pThis->SetAccelKey( nId, pData->aAccelKey );

        PopupMenu* pSubMenu = rMenu.GetPopupMenu( nId );
        if ( pSubMenu )
        {
            // AutoKopie anlegen
            if ( nMode == 1 )
            {
                PopupMenu* pNewMenu = new PopupMenu( *pSubMenu );
                pThis->SetPopupMenu( nId, pNewMenu );
//                SetAutoMenu( pThis, nId, pNewMenu );
            }
            else
                pThis->SetPopupMenu( nId, pSubMenu );
        }
    }
}

void Menu::CopyItem( const Menu& rMenu, USHORT nPos, USHORT nNewPos )
{
    ImplCopyItem( this, rMenu, nPos, nNewPos );
}

void Menu::Clear()
{
    for ( USHORT i = GetItemCount(); i; i-- )
        RemoveItem( 0 );
}

USHORT Menu::GetItemCount() const
{
    return (USHORT)pItemList->Count();
}

USHORT Menu::ImplGetVisibleItemCount() const
{
    USHORT nItems = 0;
    for ( USHORT n = (USHORT)pItemList->Count(); n; )
    {
        if ( ImplIsVisible( --n ) )
            nItems++;
    }
    return nItems;
}

USHORT Menu::GetItemId( USHORT nPos ) const
{
    MenuItemData* pData = pItemList->GetDataFromPos( nPos );

    if ( pData )
        return pData->nId;
    else
        return 0;
}

USHORT Menu::GetItemPos( USHORT nItemId ) const
{
    USHORT          nPos;
    MenuItemData*   pData = pItemList->GetData( nItemId, nPos );

    if ( pData )
        return nPos;
    else
        return MENU_ITEM_NOTFOUND;
}

MenuItemType Menu::GetItemType( USHORT nPos ) const
{
    MenuItemData* pData = pItemList->GetDataFromPos( nPos );

    if ( pData )
        return pData->eType;
    else
        return MENUITEM_DONTKNOW;
}

USHORT Menu::GetCurItemId() const
{
    return nSelectedId;
}

void Menu::SetItemBits( USHORT nItemId, MenuItemBits nBits )
{
    MenuItemData* pData = pItemList->GetData( nItemId );
    if ( pData )
        pData->nBits = nBits;
}

MenuItemBits Menu::GetItemBits( USHORT nItemId ) const
{
    MenuItemBits nBits = 0;
    MenuItemData* pData = pItemList->GetData( nItemId );
    if ( pData )
        nBits = pData->nBits;
    return nBits;
}

void Menu::SetPopupMenu( USHORT nItemId, PopupMenu* pMenu )
{
    USHORT          nPos;
    MenuItemData*   pData = pItemList->GetData( nItemId, nPos );

    // Item nicht vorhanden, dann NULL zurueckgeben
    if ( !pData )
        return;

    // Gleiches Menu, danmn brauchen wir nichts machen
    if ( (PopupMenu*)pData->pSubMenu == pMenu )
        return;

    // Daten austauschen
    pData->pSubMenu = pMenu;
}

PopupMenu* Menu::GetPopupMenu( USHORT nItemId ) const
{
    MenuItemData* pData = pItemList->GetData( nItemId );

    if ( pData )
        return (PopupMenu*)(pData->pSubMenu);
    else
        return NULL;
}

void Menu::SetAccelKey( USHORT nItemId, const KeyCode& rKeyCode )
{
    USHORT          nPos;
    MenuItemData*   pData = pItemList->GetData( nItemId, nPos );

    if ( !pData )
        return;

    if ( pData->aAccelKey == rKeyCode )
        return;

    pData->aAccelKey = rKeyCode;
}

KeyCode Menu::GetAccelKey( USHORT nItemId ) const
{
    MenuItemData* pData = pItemList->GetData( nItemId );

    if ( pData )
        return pData->aAccelKey;
    else
        return KeyCode();
}

void Menu::CheckItem( USHORT nItemId, BOOL bCheck )
{
    USHORT nPos;
    MenuItemData* pData = pItemList->GetData( nItemId, nPos );

    if ( !pData )
        return;

    // Wenn RadioCheck, dann vorherigen unchecken
    if ( bCheck && (pData->nBits & MIB_AUTOCHECK) &&
         (pData->nBits & MIB_RADIOCHECK) )
    {
        MenuItemData*   pGroupData;
        USHORT          nGroupPos;
        USHORT          nItemCount = GetItemCount();
        BOOL            bFound = FALSE;

        nGroupPos = nPos;
        while ( nGroupPos )
        {
            pGroupData = pItemList->GetDataFromPos( nGroupPos-1 );
            if ( pGroupData->nBits & MIB_RADIOCHECK )
            {
                if ( IsItemChecked( pGroupData->nId ) )
                {
                    CheckItem( pGroupData->nId, FALSE );
                    bFound = TRUE;
                    break;
                }
            }
            else
                break;
            nGroupPos--;
        }

        if ( !bFound )
        {
            nGroupPos = nPos+1;
            while ( nGroupPos < nItemCount )
            {
                pGroupData = pItemList->GetDataFromPos( nGroupPos );
                if ( pGroupData->nBits & MIB_RADIOCHECK )
                {
                    if ( IsItemChecked( pGroupData->nId ) )
                    {
                        CheckItem( pGroupData->nId, FALSE );
                        break;
                    }
                }
                else
                    break;
                nGroupPos++;
            }
        }
    }

    pData->bChecked = bCheck;
}

BOOL Menu::IsItemChecked( USHORT nItemId ) const
{
    USHORT          nPos;
    MenuItemData*   pData = pItemList->GetData( nItemId, nPos );

    if ( !pData )
        return FALSE;

    return pData->bChecked;
}

void Menu::EnableItem( USHORT nItemId, BOOL bEnable )
{
    USHORT          nPos;
    MenuItemData*   pData = pItemList->GetData( nItemId, nPos );

    if ( pData && ( pData->bEnabled != bEnable ) )
    {
        pData->bEnabled = bEnable;

        Window* pWin = ImplGetWindow();
        if ( pWin && pWin->IsVisible() )
        {
            DBG_ASSERT( bIsMenuBar, "Menu::EnableItem - Popup visible!" );
            long nX = 0;
            ULONG nCount = pItemList->Count();
            for ( ULONG n = 0; n < nCount; n++ )
            {
                MenuItemData* pData = pItemList->GetDataFromPos( n );
                if ( n == nPos )
                {
                    pWin->Invalidate( Rectangle( Point( nX, 0 ), Size( pData->aSz.Width(), pData->aSz.Height() ) ) );
                    break;
                }
                nX += pData->aSz.Width();
            }
        }
    }
}

BOOL Menu::IsItemEnabled( USHORT nItemId ) const
{
    USHORT          nPos;
    MenuItemData*   pData = pItemList->GetData( nItemId, nPos );

    if ( !pData )
        return FALSE;

    return pData->bEnabled;
}

void Menu::SetItemText( USHORT nItemId, const XubString& rStr )
{
    USHORT          nPos;
    MenuItemData*   pData = pItemList->GetData( nItemId, nPos );

    if ( !pData )
        return;

    pData->aText = rStr;
    ImplSetMenuItemData( pData, nPos );
}

XubString Menu::GetItemText( USHORT nItemId ) const
{
    USHORT          nPos;
    MenuItemData*   pData = pItemList->GetData( nItemId, nPos );

    if ( pData )
        return pData->aText;
    else
        return ImplGetSVEmptyStr();
}

void Menu::SetItemImage( USHORT nItemId, const Image& rImage )
{
    USHORT          nPos;
    MenuItemData*   pData = pItemList->GetData( nItemId, nPos );

    if ( !pData )
        return;

    pData->aImage = rImage;
    ImplSetMenuItemData( pData, nPos );
}

Image Menu::GetItemImage( USHORT nItemId ) const
{
    MenuItemData* pData = pItemList->GetData( nItemId );

    if ( pData )
        return pData->aImage;
    else
        return Image();
}

void Menu::SetItemCommand( USHORT nItemId, const String& rCommand )
{
    MenuItemData* pData = pItemList->GetData( nItemId );

    if ( pData )
        pData->aCommandStr = rCommand;
}

const XubString& Menu::GetItemCommand( USHORT nItemId ) const
{
    MenuItemData* pData = pItemList->GetData( nItemId );

    if ( pData )
        return pData->aCommandStr;
    else
        return ImplGetSVEmptyStr();
}

void Menu::SetHelpText( USHORT nItemId, const XubString& rStr )
{
    MenuItemData* pData = pItemList->GetData( nItemId );

    if ( pData )
        pData->aHelpText = rStr;
}

const XubString& Menu::GetHelpText( USHORT nItemId ) const
{
    MenuItemData* pData = pItemList->GetData( nItemId );

    if ( pData )
    {
        if ( !pData->aHelpText.Len() && pData->nHelpId )
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
                pData->aHelpText = pHelp->GetHelpText( pData->nHelpId );
        }

        return pData->aHelpText;
    }
    else
        return ImplGetSVEmptyStr();
}

void Menu::SetHelpId( USHORT nItemId, ULONG nHelpId )
{
    MenuItemData* pData = pItemList->GetData( nItemId );

    if ( pData )
        pData->nHelpId = nHelpId;
}

ULONG Menu::GetHelpId( USHORT nItemId ) const
{
    MenuItemData* pData = pItemList->GetData( nItemId );

    if ( pData )
        return pData->nHelpId;
    else
        return 0;
}

Menu& Menu::operator=( const Menu& rMenu )
{
    // Aufraeumen
    Clear();

    // Items kopieren
    USHORT nCount = rMenu.GetItemCount();
    for ( USHORT i = 0; i < nCount; i++ )
        ImplCopyItem( this, rMenu, i, MENU_APPEND, 1 );

    nDefaultItem = rMenu.nDefaultItem;
    aActivateHdl = rMenu.aActivateHdl;
    aDeactivateHdl = rMenu.aDeactivateHdl;
    aHighlightHdl = rMenu.aHighlightHdl;
    aSelectHdl = rMenu.aSelectHdl;
    aTitleText = rMenu.aTitleText;
    bIsMenuBar = rMenu.bIsMenuBar;

    return *this;
}

BOOL Menu::ImplIsVisible( USHORT nPos ) const
{
    BOOL bVisible = TRUE;

    // Fuer den Menubar nicht erlaubt, weil ich nicht mitbekomme
    // ob dadurch ein Eintrag verschwindet oder wieder da ist.
    if ( !bIsMenuBar && ( nMenuFlags & MENU_FLAG_HIDEDISABLEDENTRIES ) )
    {
        MenuItemData* pData = pItemList->GetDataFromPos( nPos );
        if ( pData->eType != MENUITEM_SEPARATOR )
        {
            // bVisible = pData->bEnabled && ( !pData->pSubMenu || pData->pSubMenu->HasValidEntries( TRUE ) );
            bVisible = pData->bEnabled; // SubMenus nicht pruefen, weil sie ggf. erst im Activate() gefuellt werden.
        }
        else
        {
            // Ein Separator ist nur dann visible, wenn davor sichtbare Eintraege stehen.
            USHORT nCount = (USHORT) pItemList->Count();
            USHORT n;
            BOOL bPrevVisible = FALSE;
            BOOL bNextVisible = FALSE;
            for ( n = nPos; !bPrevVisible && n; )
            {
                pData = pItemList->GetDataFromPos( --n );
                if ( pData->eType != MENUITEM_SEPARATOR )
                    bPrevVisible = pData->bEnabled; // && ( !pData->pSubMenu || pData->pSubMenu->HasValidEntries( TRUE ) );
                else
                    break;
            }
            if ( bPrevVisible )
            {
                for ( n = nPos+1; !bNextVisible && ( n < nCount ); n++ )
                {
                    pData = pItemList->GetDataFromPos( n );
                    if ( pData->eType != MENUITEM_SEPARATOR )
                        bNextVisible = pData->bEnabled; // && ( !pData->pSubMenu || pData->pSubMenu->HasValidEntries( TRUE ) );
                    // nicht beim naechsten Separator abbrechen...
                }
            }
            bVisible = bPrevVisible && bNextVisible;
        }
    }

    return bVisible;
}

Size Menu::ImplCalcSize( Window* pWin )
{
    // | Checked| Image| Text| Accel/Popup|

    // Fuer Symbole: nFontHeight x nFontHeight
    long nFontHeight = pWin->GetTextHeight();
    long nExtra = nFontHeight/4;

    Size aSz;
    Size aMaxImgSz;
    long nMaxTextWidth = 0;
    long nMaxAccWidth = 0;

    for ( USHORT n = (USHORT)pItemList->Count(); n; )
    {
        MenuItemData* pData = pItemList->GetDataFromPos( --n );

        pData->aSz.Height() = 0;
        pData->aSz.Width() = 0;

        if ( ImplIsVisible( n ) )
        {

            // Separator
            if ( !bIsMenuBar && ( pData->eType == MENUITEM_SEPARATOR ) )
            {
                DBG_ASSERT( !bIsMenuBar, "Separator in MenuBar ?! " );
                pData->aSz.Height() = 4;
            }

            // Image:
            if ( !bIsMenuBar && ( ( pData->eType == MENUITEM_IMAGE ) || ( pData->eType == MENUITEM_STRINGIMAGE ) ) )
            {
                Size aImgSz = pData->aImage.GetSizePixel();
                if ( aImgSz.Width() > aMaxImgSz.Width() )
                    aMaxImgSz.Width() = aImgSz.Width();
                if ( aImgSz.Height() > aMaxImgSz.Height() )
                    aMaxImgSz.Height() = aImgSz.Height();
                if ( aImgSz.Height() > pData->aSz.Height() )
                    pData->aSz.Height() = aImgSz.Height();
            }

            // Text:
            if ( (pData->eType == MENUITEM_STRING) || (pData->eType == MENUITEM_STRINGIMAGE) )
            {
                long nTextWidth = pWin->GetCtrlTextWidth( pData->aText );
                if ( nTextWidth > nMaxTextWidth )
                    nMaxTextWidth = nTextWidth;
                long nTextHeight = pWin->GetTextHeight();
                if ( nTextHeight > pData->aSz.Height() )
                    pData->aSz.Height() = nTextHeight;

                if ( bIsMenuBar )
                {
                    pData->aSz.Width() = nTextWidth + 4*nExtra;
                    aSz.Width() += pData->aSz.Width();
                }
            }

            // Accel
            if ( !bIsMenuBar && pData->aAccelKey.GetCode() )
            {
                String aName = pData->aAccelKey.GetName();
                long nAccWidth = pWin->GetTextWidth( aName );
                nAccWidth += nExtra;
                if ( nAccWidth > nMaxAccWidth )
                    nMaxAccWidth = nAccWidth;
            }

            // SubMenu?
            if ( !bIsMenuBar && pData->pSubMenu )
            {
                if ( nFontHeight > nMaxAccWidth )
                    nMaxAccWidth = nFontHeight;
                if ( nFontHeight > pData->aSz.Height() )
                    pData->aSz.Height() = nFontHeight;
            }

            pData->aSz.Height() += EXTRAITEMHEIGHT; // Etwas mehr Abstand:

            if ( !bIsMenuBar )
                aSz.Height() += (long)pData->aSz.Height();
        }
    }

    if ( !bIsMenuBar )
    {
        nCheckPos = (USHORT)nExtra;
        nImagePos = (USHORT)(nCheckPos + nFontHeight/2 + nExtra );
        nTextPos = (USHORT)(nImagePos+aMaxImgSz.Width());
        if ( aMaxImgSz.Width() )
            nTextPos += (USHORT)nExtra;

        aSz.Width() = nTextPos + nMaxTextWidth + nExtra + nMaxAccWidth;
        aSz.Width() += 10*nExtra;   // etwas mehr...
    }
    else
    {
        nTextPos = (USHORT)(2*nExtra);
        aSz.Height() = nFontHeight+6;
    }

    if ( pLogo )
        aSz.Width() += pLogo->aBitmap.GetSizePixel().Width();

    return aSz;
}

void Menu::ImplPaint( Window* pWin, USHORT nBorder, long nStartY, MenuItemData* pThisItemOnly, BOOL bHighlighted )
{
    // Fuer Symbole: nFontHeight x nFontHeight
    long nFontHeight = pWin->GetTextHeight();
    long nExtra = nFontHeight/4;

    DecorationView aDecoView( pWin );
    const StyleSettings& rSettings = pWin->GetSettings().GetStyleSettings();

    Point aTopLeft, aTmpPos;

    if ( pLogo )
        aTopLeft.X() = pLogo->aBitmap.GetSizePixel().Width();

    Size aOutSz = pWin->GetOutputSizePixel();
    long nMaxY = aOutSz.Height() - nBorder;
    USHORT nCount = (USHORT)pItemList->Count();
    for ( USHORT n = 0; n < nCount; n++ )
    {
        MenuItemData* pData = pItemList->GetDataFromPos( n );
        if ( ImplIsVisible( n ) && ( !pThisItemOnly || ( pData == pThisItemOnly ) ) )
        {
            if ( pThisItemOnly && bHighlighted )
                pWin->SetTextColor( rSettings.GetMenuHighlightTextColor() );

            Point aPos( aTopLeft );
            aPos.Y() += nBorder;
            aPos.Y() += nStartY;

            if ( aPos.Y() >= 0 )
            {
                long    nTextOffsetY = ((pData->aSz.Height()-nFontHeight)/2);
                USHORT  nTextStyle   = 0;
                USHORT  nSymbolStyle = 0;
                USHORT  nImageStyle  = 0;
                // SubMenus ohne Items werden nicht mehr disablte dargestellt,
                // wenn keine Items enthalten sind, da die Anwendung selber
                // darauf achten muss. Ansonsten gibt es Faelle, wo beim
                // asyncronen laden die Eintraege disablte dargestellt werden.
                if ( !pData->bEnabled )
                {
                    nTextStyle   |= TEXT_DRAW_DISABLE;
                    nSymbolStyle |= SYMBOL_DRAW_DISABLE;
                    nImageStyle  |= IMAGE_DRAW_DISABLE;
                }

                // Separator
                if ( !bIsMenuBar && ( pData->eType == MENUITEM_SEPARATOR ) )
                {
                    aTmpPos.Y() = aPos.Y() + ((pData->aSz.Height()-2)/2);
                    aTmpPos.X() = aPos.X() + 1;
                    pWin->SetLineColor( rSettings.GetShadowColor() );
                    pWin->DrawLine( aTmpPos, Point( aOutSz.Width() - 1, aTmpPos.Y() ) );
                    aTmpPos.Y()++;
                    pWin->SetLineColor( rSettings.GetLightColor() );
                    pWin->DrawLine( aTmpPos, Point( aOutSz.Width() - 1, aTmpPos.Y() ) );
                    pWin->SetLineColor();
                }

                // Image:
                if ( !bIsMenuBar && ( ( pData->eType == MENUITEM_IMAGE ) || ( pData->eType == MENUITEM_STRINGIMAGE ) ) )
                {
                    aTmpPos.Y() = aPos.Y();
                    aTmpPos.X() = aPos.X() + nImagePos;
                    aTmpPos.Y() += (pData->aSz.Height()-pData->aImage.GetSizePixel().Height())/2;
                    pWin->DrawImage( aTmpPos, pData->aImage, nImageStyle );
                }

                // Text:
                if ( ( pData->eType == MENUITEM_STRING ) || ( pData->eType == MENUITEM_STRINGIMAGE ) )
                {
                    aTmpPos.X() = aPos.X() + nTextPos;
                    aTmpPos.Y() = aPos.Y();
                    aTmpPos.Y() += nTextOffsetY;
                    USHORT nStyle = nTextStyle|TEXT_DRAW_MNEMONIC;
                    if ( pData->bIsTemporary )
                        nStyle |= TEXT_DRAW_DISABLE;
                    pWin->DrawCtrlText( aTmpPos, pData->aText, 0, pData->aText.Len(), nStyle );
                }

                // Accel
                if ( !bIsMenuBar && pData->aAccelKey.GetCode() )
                {
                    XubString aAccText = pData->aAccelKey.GetName();
                    aTmpPos.X() = aOutSz.Width() - pWin->GetTextWidth( aAccText );
                    aTmpPos.X() -= 3*nExtra;
                    aTmpPos.Y() = aPos.Y();
                    aTmpPos.Y() += nTextOffsetY;
                    pWin->DrawCtrlText( aTmpPos, aAccText, 0, aAccText.Len(), nTextStyle );
                }

                // CheckMark
                if ( !bIsMenuBar && pData->bChecked )
                {
                    Rectangle aRect;
                    SymbolType eSymbol;
                    aTmpPos.Y() = aPos.Y();
                    aTmpPos.Y() += nExtra/2;
                    aTmpPos.Y() += pData->aSz.Height() / 2;
                    if ( pData->nBits & MIB_RADIOCHECK )
                    {
                        aTmpPos.X() = aPos.X() + nCheckPos;
                        eSymbol = SYMBOL_RADIOCHECKMARK;
                        aTmpPos.Y() -= nFontHeight/4;
                        aRect = Rectangle( aTmpPos, Size( nFontHeight/2, nFontHeight/2 ) );
                    }
                    else
                    {
                        aTmpPos.X() = aPos.X() + nCheckPos;
                        eSymbol = SYMBOL_CHECKMARK;
                        aTmpPos.Y() -= nFontHeight/4;
                        aRect = Rectangle( aTmpPos, Size( (nFontHeight*25)/40, nFontHeight/2 ) );
                    }
                    aDecoView.DrawSymbol( aRect, eSymbol, pWin->GetTextColor(), nSymbolStyle );
                }

                // SubMenu?
                if ( !bIsMenuBar && pData->pSubMenu )
                {
                    aTmpPos.X() = aOutSz.Width() - nFontHeight + nExtra;
                    aTmpPos.Y() = aPos.Y();
                    aTmpPos.Y() += nExtra/2;
                    aTmpPos.Y() += ( pData->aSz.Height() / 2 ) - ( nFontHeight/4 );
                    if ( pData->nBits & MIB_POPUPSELECT )
                    {
                        pWin->SetTextColor( rSettings.GetMenuTextColor() );
                        Point aTmpPos2( aPos );
                        aTmpPos2.X() = aOutSz.Width() - nFontHeight - nFontHeight/4;
                        aDecoView.DrawFrame(
                            Rectangle( aTmpPos2, Size( nFontHeight+nFontHeight/4, pData->aSz.Height() ) ), FRAME_DRAW_GROUP );
                    }
                    aDecoView.DrawSymbol(
                        Rectangle( aTmpPos, Size( nFontHeight/2, nFontHeight/2 ) ),
                        SYMBOL_SPIN_RIGHT, pWin->GetTextColor(), nSymbolStyle );
//                  if ( pData->nBits & MIB_POPUPSELECT )
//                  {
//                      aTmpPos.Y() += nFontHeight/2 ;
//                      pWin->SetLineColor( rSettings.GetShadowColor() );
//                      pWin->DrawLine( aTmpPos, Point( aTmpPos.X() + nFontHeight/3, aTmpPos.Y() ) );
//                      pWin->SetLineColor( rSettings.GetLightColor() );
//                      aTmpPos.Y()++;
//                      pWin->DrawLine( aTmpPos, Point( aTmpPos.X() + nFontHeight/3, aTmpPos.Y() ) );
//                      pWin->SetLineColor();
//                  }
                }

                if ( pThisItemOnly && bHighlighted )
                    pWin->SetTextColor( rSettings.GetMenuTextColor() );
            }
        }

        if ( !bIsMenuBar )
        {
            aTopLeft.Y() += pData->aSz.Height();
        }
        else
        {
            aTopLeft.X() += pData->aSz.Width();
        }
    }

    if ( !pThisItemOnly && pLogo )
    {
        Size aLogoSz = pLogo->aBitmap.GetSizePixel();

        Rectangle aRect( Point( 0, 0 ), Point( aLogoSz.Width()-1, aOutSz.Height() ) );
        if ( pWin->GetColorCount() >= 256 )
        {
            Gradient aGrad( GRADIENT_LINEAR, pLogo->aStartColor, pLogo->aEndColor );
            aGrad.SetAngle( 1800 );
            aGrad.SetBorder( 15 );
            pWin->DrawGradient( aRect, aGrad );
        }
        else
        {
            pWin->SetFillColor( pLogo->aStartColor );
            pWin->DrawRect( aRect );
        }

        Point aLogoPos( 0, aOutSz.Height() - aLogoSz.Height() );
        pLogo->aBitmap.Draw( pWin, aLogoPos );
    }
}

Menu* Menu::ImplGetStartMenu()
{
    Menu* pStart = this;
    while ( pStart && pStart->pStartedFrom && ( pStart->pStartedFrom != pStart ) )
        pStart = pStart->pStartedFrom;
    return pStart;
}

void Menu::ImplCallHighlight( USHORT nHighlightedItem )
{
    nSelectedId = 0;
    MenuItemData* pData = pItemList->GetDataFromPos( nHighlightedItem );
    if ( pData )
        nSelectedId = pData->nId;
    Highlight();
    nSelectedId = 0;
}

IMPL_LINK( Menu, ImplCallSelect, Menu*, EMPTYARG )
{
    nEventId = 0;
    Select();
    return 0;
}

Menu* Menu::ImplFindSelectMenu()
{
    Menu* pSelMenu = nEventId ? this : NULL;

    for ( ULONG n = GetItemList()->Count(); n && !pSelMenu; )
    {
        MenuItemData* pData = GetItemList()->GetDataFromPos( --n );

        if ( pData->pSubMenu )
            pSelMenu = pData->pSubMenu->ImplFindSelectMenu();
    }

    return pSelMenu;
}

void Menu::RemoveDisabledEntries( BOOL bCheckPopups, BOOL bRemoveEmptyPopups )
{
    for ( USHORT n = 0; n < GetItemCount(); n++ )
    {
        BOOL bRemove = FALSE;
        MenuItemData* pItem = pItemList->GetDataFromPos( n );
        if ( pItem->eType == MENUITEM_SEPARATOR )
        {
            if ( !n || ( GetItemType( n-1 ) == MENUITEM_SEPARATOR ) )
                bRemove = TRUE;
        }
        else
            bRemove = !pItem->bEnabled;

        if ( bCheckPopups && pItem->pSubMenu )
        {
            pItem->pSubMenu->RemoveDisabledEntries( TRUE );
            if ( bRemoveEmptyPopups && !pItem->pSubMenu->GetItemCount() )
                bRemove = TRUE;
        }

        if ( bRemove )
            RemoveItem( n-- );
    }

    if ( GetItemCount() )
    {
        USHORT nLast = GetItemCount() - 1;
        MenuItemData* pItem = pItemList->GetDataFromPos( nLast );
        if ( pItem->eType == MENUITEM_SEPARATOR )
            RemoveItem( nLast );
    }
}

BOOL Menu::HasValidEntries( BOOL bCheckPopups )
{
    BOOL bValidEntries = FALSE;
    USHORT nCount = GetItemCount();
    for ( USHORT n = 0; !bValidEntries && ( n < nCount ); n++ )
    {
        MenuItemData* pItem = pItemList->GetDataFromPos( n );
        if ( pItem->bEnabled && ( pItem->eType != MENUITEM_SEPARATOR ) )
        {
            if ( bCheckPopups && pItem->pSubMenu )
                bValidEntries = pItem->pSubMenu->HasValidEntries( TRUE );
            else
                bValidEntries = TRUE;
        }
    }
    return bValidEntries;
}

void Menu::SetLogo( const MenuLogo& rLogo )
{
    delete pLogo;
    pLogo = new MenuLogo( rLogo );
}

void Menu::SetLogo()
{
    delete pLogo;
    pLogo = NULL;
}

MenuLogo Menu::GetLogo() const
{
    MenuLogo aLogo;
    if ( pLogo )
        aLogo = *pLogo;
    return aLogo;
}

void Menu::GetAccessObject( AccessObjectRef& rAcc ) const
{
    rAcc = new AccessObject( (void*) this, bIsMenuBar? ACCESS_TYPE_MENUBAR : ACCESS_TYPE_MENU );
}

// -----------
// - MenuBar -
// -----------

MenuBar::MenuBar()
{
    bIsMenuBar          = TRUE;
    mbCloserVisible     = FALSE;
    mbFloatBtnVisible   = FALSE;
    mbHideBtnVisible    = FALSE;
}

MenuBar::MenuBar( const MenuBar& rMenu )
{
    bIsMenuBar          = TRUE;
    mbCloserVisible     = FALSE;
    mbFloatBtnVisible   = FALSE;
    mbHideBtnVisible    = FALSE;
    *this               = rMenu;
    bIsMenuBar          = TRUE;
}

MenuBar::MenuBar( const ResId& rResId )
{
    bIsMenuBar          = TRUE;
    mbCloserVisible     = FALSE;
    mbFloatBtnVisible   = FALSE;
    mbHideBtnVisible    = FALSE;
    ImplLoadRes( rResId );
}

MenuBar::~MenuBar()
{
    ImplDestroy( this, TRUE );
}

void MenuBar::ShowCloser( BOOL bShow )
{
    ShowButtons( bShow, mbFloatBtnVisible, mbHideBtnVisible );
}

void MenuBar::ShowFloatButton( BOOL bShow )
{
    ShowButtons( mbCloserVisible, bShow, mbHideBtnVisible );
}

void MenuBar::ShowHideButton( BOOL bShow )
{
    ShowButtons( mbCloserVisible, mbFloatBtnVisible, bShow );
}

void MenuBar::ShowButtons( BOOL bClose, BOOL bFloat, BOOL bHide )
{
    if ( (bClose != mbCloserVisible)    ||
         (bFloat != mbFloatBtnVisible)  ||
         (bHide  != mbHideBtnVisible) )
    {
        mbCloserVisible     = bClose;
        mbFloatBtnVisible   = bFloat;
        mbHideBtnVisible    = bHide;
        if ( ImplGetWindow() )
            ((MenuBarWindow*)ImplGetWindow())->ShowButtons( bClose, bFloat, bHide );
    }
}

Window* MenuBar::ImplCreate( Window* pParent, Window* pWindow, MenuBar* pMenu )
{
    if ( !pWindow )
        pWindow = new MenuBarWindow( pParent );

    pMenu->pStartedFrom = 0;
    pMenu->pWindow = pWindow;
    ((MenuBarWindow*)pWindow)->SetMenu( pMenu );
    long nHeight = pMenu->ImplCalcSize( pWindow ).Height();
    pWindow->SetPosSizePixel( 0, 0, 0, nHeight, WINDOW_POSSIZE_HEIGHT );
    return pWindow;
}

void MenuBar::ImplDestroy( MenuBar* pMenu, BOOL bDelete )
{
    MenuBarWindow* pWindow = (MenuBarWindow*) pMenu->ImplGetWindow();
    if ( pWindow && bDelete )
    {
        pWindow->KillActivePopup();
        delete pWindow;
    }
    pMenu->pWindow = NULL;
}

BOOL MenuBar::ImplHandleKeyEvent( const KeyEvent& rKEvent, BOOL bFromMenu )
{
    BOOL bDone = FALSE;
    // Enabled-Abfragen, falls diese Methode von einem anderen Fenster gerufen wurde...
    Window* pWin = ImplGetWindow();
    if ( pWin && pWin->IsEnabled() && pWin->IsInputEnabled() )
        bDone = ((MenuBarWindow*)pWin)->ImplHandleKeyEvent( rKEvent, bFromMenu );
    return bDone;
}

// -----------------------------------------------------------------------

void MenuBar::SelectEntry( USHORT nId )
{
    if( ImplGetWindow() )
    {
        MenuBarWindow* pMenuWin = (MenuBarWindow*) ImplGetWindow();

        pMenuWin->GrabFocus();
        nId = GetItemPos( nId );

        if( ITEMPOS_INVALID == pMenuWin->nHighlightedItem )
        {
            if( ( nId != ITEMPOS_INVALID ) && ( nId != pMenuWin->nHighlightedItem ) )
                pMenuWin->ChangeHighlightItem( nId, FALSE );
        }
        else
        {
            pMenuWin->KillActivePopup();
            pMenuWin->ChangeHighlightItem( ITEMPOS_INVALID, FALSE );
        }
    }
}

// BOOL PopupMenu::bAnyPopupInExecute = FALSE;

PopupMenu::PopupMenu()
{
}

PopupMenu::PopupMenu( const ResId& rResId )
{
    ImplLoadRes( rResId );
}

PopupMenu::PopupMenu( const PopupMenu& rMenu )
{
    *this = rMenu;
}

PopupMenu::~PopupMenu()
{
}

BOOL PopupMenu::IsInExecute()
{
    return GetActivePopupMenu() ? TRUE : FALSE;
}

PopupMenu* PopupMenu::GetActivePopupMenu()
{
    ImplSVData* pSVData = ImplGetSVData();
    return pSVData->maAppData.mpActivePopupMenu;
}

void PopupMenu::EndExecute( USHORT nSelectId )
{
    if ( ImplGetWindow() )
        ImplGetFloatingWindow()->EndExecute( nSelectId );
}

void PopupMenu::SelectEntry( USHORT nId )
{
    if ( ImplGetWindow() )
    {
        USHORT nPos;
        MenuItemData* pData = GetItemList()->GetData( nId, nPos );
        if ( pData->pSubMenu )
            ImplGetFloatingWindow()->ChangeHighlightItem( nPos, TRUE );
        else
            ImplGetFloatingWindow()->EndExecute( nId );
    }
}

USHORT PopupMenu::Execute( Window* pWindow, const Point& rPopupPos )
{
    return Execute( pWindow, Rectangle( rPopupPos, rPopupPos ), POPUPMENU_EXECUTE_DOWN );
}

USHORT PopupMenu::Execute( Window* pWindow, const Rectangle& rRect, USHORT nFlags )
{
    ULONG nPopupModeFlags = 0;
    if ( nFlags & POPUPMENU_EXECUTE_DOWN )
        nPopupModeFlags = FLOATWIN_POPUPMODE_DOWN;
    else if ( nFlags & POPUPMENU_EXECUTE_UP )
        nPopupModeFlags = FLOATWIN_POPUPMODE_UP;
    else if ( nFlags & POPUPMENU_EXECUTE_LEFT )
        nPopupModeFlags = FLOATWIN_POPUPMODE_LEFT;
    else if ( nFlags & POPUPMENU_EXECUTE_RIGHT )
        nPopupModeFlags = FLOATWIN_POPUPMODE_RIGHT;
    else
        nPopupModeFlags = FLOATWIN_POPUPMODE_DOWN;
    return ImplExecute( pWindow, rRect, nPopupModeFlags, 0, FALSE );
}

USHORT PopupMenu::ImplExecute( Window* pW, const Rectangle& rRect, ULONG nPopupModeFlags, Menu* pSFrom, BOOL bPreSelectFirst )
{
    // #59614# Mit TH abgesprochen dass die ASSERTION raus kommt,
    // weil es evtl. legitim ist...
//  DBG_ASSERT( !PopupMenu::IsInExecute() || pSFrom, "PopupMenu::Execute() called in PopupMenu::Execute()" );

    if ( !pSFrom && ( PopupMenu::IsInExecute() || !GetItemCount() ) )
        return 0;

    ImplSVData* pSVData = ImplGetSVData();

    pStartedFrom = pSFrom;
    nSelectedId = 0;
    bCanceled = FALSE;

    ULONG nFocusId = 0;
    BOOL bRealExecute = FALSE;
    if ( !pStartedFrom )
    {
        pSVData->maWinData.mbNoDeactivate = TRUE;
        nFocusId = Window::SaveFocus();
        bRealExecute = TRUE;
    }

    DBG_ASSERT( !ImplGetWindow(), "Win?!" );
    Rectangle aRect( rRect );
    aRect.SetPos( pW->OutputToScreenPixel( aRect.TopLeft() ) );

    WinBits nStyle = WB_BORDER;
    if ( bRealExecute )
        nPopupModeFlags |= FLOATWIN_POPUPMODE_NEWLEVEL;
    if ( !pStartedFrom || !pStartedFrom->bIsMenuBar )
        nPopupModeFlags |= FLOATWIN_POPUPMODE_PATHMOUSECANCELCLICK | FLOATWIN_POPUPMODE_ALLMOUSEBUTTONCLOSE;

    // Kann beim Debuggen hilfreich sein.
    // nPopupModeFlags |= FLOATWIN_POPUPMODE_NOFOCUSCLOSE;

    ImplDelData aDelData;
    pW->ImplAddDel( &aDelData );

    bInCallback = TRUE; // hier schon setzen, falls Activate ueberladen
    Activate();
    bInCallback = FALSE;

    if ( aDelData.IsDelete() )
        return 0;   // Error

    pW->ImplRemoveDel( &aDelData );

    if ( bCanceled || bKilled )
        return 0;

    if ( !GetItemCount() )
        return 0;

    // Das Flag MENU_FLAG_HIDEDISABLEDENTRIES wird vererbt.
    if ( pSFrom )
    {
        if ( pSFrom->nMenuFlags & MENU_FLAG_HIDEDISABLEDENTRIES )
            nMenuFlags |= MENU_FLAG_HIDEDISABLEDENTRIES;
        else
            nMenuFlags &= ~MENU_FLAG_HIDEDISABLEDENTRIES;
    }

    USHORT nVisibleEntries = ImplGetVisibleItemCount();
    if ( !nVisibleEntries )
    {
        String aTmpEntryText( ResId( SV_RESID_STRING_NOSELECTIONPOSSIBLE, ImplGetResMgr() ) );
        MenuItemData* pData = pItemList->Insert(
            0xFFFF, MENUITEM_STRING, 0, aTmpEntryText, Image(), NULL, 0xFFFF );
        pData->bIsTemporary = TRUE;
    }
    else if ( pSVData->maAppData.mbAutoMnemonics && !( nMenuFlags & MENU_FLAG_NOAUTOMNEMONICS ) )
    {
        ImplMnemonicGenerator aMnemonicGenerator;
        ULONG n;
        for ( n = 0; n < pItemList->Count(); n++ )
            aMnemonicGenerator.RegisterMnemonic( pItemList->GetDataFromPos(n)->aText );
        for ( n = 0; n < pItemList->Count(); n++ )
            aMnemonicGenerator.CreateMnemonic( pItemList->GetDataFromPos(n)->aText );
    }

    MenuFloatingWindow* pWin = new MenuFloatingWindow( this, pW, nStyle );
    pWindow = pWin;

    Size aSz = ImplCalcSize( pWin );

    long nMaxHeight = pWin->GetDesktopRectPixel().GetHeight();
    if ( pStartedFrom && pStartedFrom->bIsMenuBar )
        nMaxHeight -= pW->GetSizePixel().Height();
    long nLeft, nTop, nRight, nBottom;
    pWindow->GetBorder( nLeft, nTop, nRight, nBottom );
    nMaxHeight -= nTop+nBottom;
    if ( aSz.Height() > nMaxHeight )
    {
        pWin->EnableScrollMenu( TRUE );
        USHORT nEntries = ImplCalcVisEntries( nMaxHeight );
        aSz.Height() = ImplCalcHeight( nEntries );
    }

    pWin->SetFocusId( nFocusId );
    pWin->SetOutputSizePixel( aSz );
    pWin->GrabFocus();
    if ( GetItemCount() )
    {
        pWin->StartPopupMode( aRect, nPopupModeFlags );
    }
    if ( bPreSelectFirst )
    {
        USHORT nCount = (USHORT)pItemList->Count();
        for ( USHORT n = 0; n < nCount; n++ )
        {
            MenuItemData* pData = pItemList->GetDataFromPos( n );
            if ( ( pData->eType != MENUITEM_SEPARATOR ) && ImplIsVisible( n ) )
            {
                pWin->ChangeHighlightItem( n, FALSE );
                break;
            }
        }
    }
    if ( bRealExecute )
    {
        pWin->Execute();

        // Focus wieder herstellen (kann schon im Select wieder
        // hergestellt wurden sein
        nFocusId = pWin->GetFocusId();
        if ( nFocusId )
        {
            pWin->SetFocusId( 0 );
            pSVData->maWinData.mbNoDeactivate = FALSE;
        }
        pWin->ImplEndPopupMode( 0, nFocusId );

        if ( nSelectedId )  // Dann abraeumen... ( sonst macht TH das )
        {
            PopupMenu* pSub = pWin->GetActivePopup();
            while ( pSub )
            {
                pSub->ImplGetFloatingWindow()->EndPopupMode();
                pSub = pSub->ImplGetFloatingWindow()->GetActivePopup();
            }
        }
        delete pWindow;
        pWindow = NULL;

        // Steht noch ein Select aus?
        Menu* pSelect = ImplFindSelectMenu();
        if ( pSelect )
        {
            // Beim Popup-Menu muss das Select vor dem Verlassen von Execute gerufen werden!
            Application::RemoveUserEvent( pSelect->nEventId );
            pSelect->nEventId = 0;
            pSelect->Select();
        }
    }

    return bRealExecute ? nSelectedId : 0;
}

USHORT PopupMenu::ImplCalcVisEntries( long nMaxHeight, USHORT nStartEntry ) const
{
    nMaxHeight -= 2 * ImplGetFloatingWindow()->GetScrollerHeight();

    long nHeight = 0;
    USHORT nEntries = (USHORT) pItemList->Count();
    USHORT nVisEntries = 0;
    for ( USHORT n = nStartEntry; n < nEntries; n++ )
    {
        if ( ImplIsVisible( n ) )
        {
            MenuItemData* pData = pItemList->GetDataFromPos( n );
            nHeight += pData->aSz.Height();
            if ( nHeight > nMaxHeight )
                break;
            nVisEntries++;
        }
    }
    return nVisEntries;
}

long PopupMenu::ImplCalcHeight( USHORT nEntries ) const
{
    long nHeight = 0;

    if ( nEntries > pItemList->Count() )
        nEntries = (USHORT) pItemList->Count();

    for ( ULONG n = 0; n < nEntries; n++ )
    {
        MenuItemData* pData = pItemList->GetDataFromPos( n );
        nHeight += pData->aSz.Height();
    }

    nHeight += 2*ImplGetFloatingWindow()->GetScrollerHeight();

    return nHeight;
}


static void ImplInitMenuWindow( Window* pWin, BOOL bFont )
{
    const StyleSettings& rStyleSettings = pWin->GetSettings().GetStyleSettings();

    if ( bFont )
        pWin->SetPointFont( rStyleSettings.GetMenuFont() );
    pWin->SetBackground( Wallpaper( rStyleSettings.GetMenuColor() ) );
    pWin->SetTextColor( rStyleSettings.GetMenuTextColor() );
    pWin->SetTextFillColor();
    pWin->SetLineColor();
}

MenuFloatingWindow::MenuFloatingWindow( Menu* pMen, Window* pParent, WinBits nStyle ) :
    FloatingWindow( pParent, nStyle )
{
    pMenu               = pMen;
    pActivePopup        = 0;
    nSaveFocusId        = 0;
    bInExecute          = FALSE;
    bScrollMenu         = FALSE;
    nHighlightedItem    = ITEMPOS_INVALID;
    nMBDownPos          = ITEMPOS_INVALID;
    nScrollerHeight     = 0;
    nStartY             = 0;
    nBorder             = EXTRASPACEY;
    nFirstEntry         = 0;
    bScrollUp           = FALSE;
    bScrollDown         = FALSE;

    EnableSaveBackground();
    ImplInitMenuWindow( this, TRUE );

    SetPopupModeEndHdl( LINK( this, MenuFloatingWindow, PopupEnd ) );

    aHighlightChangedTimer.SetTimeoutHdl( LINK( this, MenuFloatingWindow, HighlightChanged ) );
    aHighlightChangedTimer.SetTimeout( GetSettings().GetMouseSettings().GetMenuDelay() );
    aScrollTimer.SetTimeoutHdl( LINK( this, MenuFloatingWindow, AutoScroll ) );

    if ( Application::GetAccessHdlCount() )
        Application::AccessNotify( AccessNotification( ACCESS_EVENT_POPUPMENU_START, pMenu ) );
}

MenuFloatingWindow::~MenuFloatingWindow()
{
    if( Application::GetAccessHdlCount() )
        Application::AccessNotify( AccessNotification( ACCESS_EVENT_POPUPMENU_END, pMenu ) );

    aHighlightChangedTimer.Stop();
}

void MenuFloatingWindow::Resize()
{
    ImplInitClipRegion();
}

Region MenuFloatingWindow::ImplCalcClipRegion( BOOL bIncludeLogo ) const
{
    Size aOutSz = GetOutputSizePixel();
    Point aPos;
    Rectangle aRect( aPos, aOutSz );
    long nBorder = nScrollerHeight;
    aRect.Top() += nBorder;
    aRect.Bottom() -= nBorder;

    if ( pMenu->pLogo && !bIncludeLogo )
        aRect.Left() += pMenu->pLogo->aBitmap.GetSizePixel().Width();

    Region aRegion = aRect;
    if ( pMenu->pLogo && bIncludeLogo && nScrollerHeight )
        aRegion.Union( Rectangle( Point(), Size( pMenu->pLogo->aBitmap.GetSizePixel().Width(), aOutSz.Height() ) ) );

    return aRegion;
}

void MenuFloatingWindow::ImplInitClipRegion()
{
    if ( IsScrollMenu() )
    {
        SetClipRegion( ImplCalcClipRegion() );
    }
    else
    {
        SetClipRegion();
    }
}

void MenuFloatingWindow::ImplHighlightItem( const MouseEvent& rMEvt, BOOL bMBDown )
{
    long nY = nScrollerHeight;
    long nMouseY = rMEvt.GetPosPixel().Y();
    Size aOutSz = GetOutputSizePixel();
    if ( ( nMouseY >= nY ) && ( nMouseY < ( aOutSz.Height() - nY ) ) )
    {
        BOOL bHighlighted = FALSE;
        USHORT nCount = (USHORT)pMenu->pItemList->Count();
        nY += nStartY;  // ggf. gescrollt.
        for ( USHORT n = 0; !bHighlighted && ( n < nCount ); n++ )
        {
            if ( pMenu->ImplIsVisible( n ) )
            {
                MenuItemData* pData = pMenu->pItemList->GetDataFromPos( n );
                long nOldY = nY;
                nY += pData->aSz.Height();
                if ( ( nOldY <= nMouseY ) && ( nY > nMouseY ) )
                {
                    BOOL bPopupArea = TRUE;
                    if ( pData->nBits & MIB_POPUPSELECT )
                    {
                        // Nur wenn ueber dem Pfeil geklickt wurde...
                        Size aSz = GetOutputSizePixel();
                        long nFontHeight = GetTextHeight();
                        bPopupArea = ( rMEvt.GetPosPixel().X() >= ( aSz.Width() - nFontHeight - nFontHeight/4 ) );
                    }

                    if ( bMBDown )
                    {
                        if ( n != nHighlightedItem )
                            ChangeHighlightItem( (USHORT)n, FALSE );

                        if ( pActivePopup )
                            KillActivePopup();
                        else if ( bPopupArea )
                            HighlightChanged( NULL );
                    }
                    else
                    {
                        if ( n != nHighlightedItem )
                        {
                            ChangeHighlightItem( (USHORT)n, TRUE );
                        }
                        else if ( pData->nBits & MIB_POPUPSELECT )
                        {
                            if ( bPopupArea && ( pActivePopup != pData->pSubMenu ) )
                                HighlightChanged( NULL );
                        }
                    }
                    bHighlighted = TRUE;
                }
            }
        }
        if ( !bHighlighted )
            ChangeHighlightItem( ITEMPOS_INVALID, TRUE );
    }
    else
    {
        ImplScroll( rMEvt.GetPosPixel() );
        ChangeHighlightItem( ITEMPOS_INVALID, TRUE );
    }
}

IMPL_LINK( MenuFloatingWindow, PopupEnd, FloatingWindow*, pPopup )
{
    if ( bInExecute )
    {
        if ( pActivePopup )
        {
            DBG_ASSERT( !pActivePopup->ImplGetWindow(), "PopupEnd, obwohl pActivePopup MIT Window!" );
            pActivePopup->bCanceled = TRUE;
        }
        bInExecute = FALSE;
        pMenu->bInCallback = TRUE;
        pMenu->Deactivate();
        pMenu->bInCallback = FALSE;
    }
    else
    {
        // Wenn dies Fenster von TH geschlossen wurde, hat noch ein anderes
        // Menu dieses Fenster als pActivePopup.
        if ( pMenu->pStartedFrom )
        {
            // Das pWin am 'Parent' kann aber schon 0 sein, falls die Kette von
            // vorne abgeraeumt wurde und jetzt die EndPopup-Events eintrudeln
            if ( pMenu->pStartedFrom->bIsMenuBar )
            {
                MenuBarWindow* p = (MenuBarWindow*) pMenu->pStartedFrom->ImplGetWindow();
                if ( p )
                    p->PopupClosed( pMenu );
            }
            else
            {
                MenuFloatingWindow* p = (MenuFloatingWindow*) pMenu->pStartedFrom->ImplGetWindow();
                if ( p )
                    p->KillActivePopup( (PopupMenu*)pMenu );
            }
        }
    }
    return 0;
}

IMPL_LINK( MenuFloatingWindow, AutoScroll, Timer*, EMPTYARG )
{
    ImplScroll( GetPointerPosPixel() );
    return 1;
}

IMPL_LINK( MenuFloatingWindow, HighlightChanged, Timer*, pTimer )
{
    MenuItemData* pData = pMenu->pItemList->GetDataFromPos( nHighlightedItem );
    if ( pData )
    {
        if ( pActivePopup && ( pActivePopup != pData->pSubMenu ) )
        {
            KillActivePopup();
        }
        if ( pData->bEnabled && pData->pSubMenu && pData->pSubMenu->GetItemCount() && ( pData->pSubMenu != pActivePopup ) )
        {
            pActivePopup = (PopupMenu*)pData->pSubMenu;
            long nY = nScrollerHeight+nStartY;
            MenuItemData* pData = 0;
            for ( ULONG n = 0; n < nHighlightedItem; n++ )
            {
                pData = pMenu->pItemList->GetDataFromPos( n );
                nY += pData->aSz.Height();
            }
            pData = pMenu->pItemList->GetDataFromPos( nHighlightedItem );
            Size MySize = GetOutputSizePixel();
//          Point MyPos = GetPosPixel();
//          Point aItemTopLeft( MyPos.X(), MyPos.Y()+nY );
            Point aItemTopLeft( 0, nY );
            Point aItemBottomRight( aItemTopLeft );
            aItemBottomRight.X() += MySize.Width();
            aItemBottomRight.Y() += pData->aSz.Height();

            // Popups leicht versetzen:
            aItemTopLeft.X() += 2;
            aItemBottomRight.X() -= 2;
            if ( nHighlightedItem )
                aItemTopLeft.Y() -= 2;
            else
            {
                long nL, nT, nR, nB;
                GetBorder( nL, nT, nR, nB );
                aItemTopLeft.Y() -= nT;
            }

            // pTest: Wegen Abstuerzen durch Reschedule() im Aufruf von Activate()
            // Ausserdem wird damit auch verhindert, dass SubMenus angezeigt werden,
            // die lange im Activate Rescheduled haben und jetzt schon nicht mehr
            // angezeigt werden sollen.
            Menu* pTest = pActivePopup;
            USHORT nRet = pActivePopup->ImplExecute( this, Rectangle( aItemTopLeft, aItemBottomRight ), FLOATWIN_POPUPMODE_RIGHT, pMenu, pTimer ? FALSE : TRUE  );

            // nRet != 0, wenn es waerend Activate() abgeschossen wurde...
            if ( !nRet && ( pActivePopup == pTest ) && pActivePopup->ImplGetWindow() )
                pActivePopup->ImplGetFloatingWindow()->AddPopupModeWindow( this );
        }
    }

    return 0;
}

void MenuFloatingWindow::EnableScrollMenu( BOOL b )
{
    bScrollMenu = b;
    nScrollerHeight = b ? (USHORT) GetSettings().GetStyleSettings().GetScrollBarSize() /2 : 0;
    bScrollDown = TRUE;
    ImplInitClipRegion();
}

void MenuFloatingWindow::Execute()
{
    ImplSVData* pSVData = ImplGetSVData();

    pSVData->maAppData.mpActivePopupMenu = (PopupMenu*)pMenu;

    bInExecute = TRUE;
//  bCallingSelect = FALSE;

    while ( bInExecute )
        Application::Yield();

    pSVData->maAppData.mpActivePopupMenu = NULL;

//  while ( bCallingSelect )
//      Application::Yield();
}

void MenuFloatingWindow::StopExecute( ULONG nFocusId )
{
    // Focus wieder herstellen
    // (kann schon im Select wieder hergestellt wurden sein)
    if ( nSaveFocusId )
    {
        Window::EndSaveFocus( nFocusId, FALSE );
        nFocusId = nSaveFocusId;
        if ( nFocusId )
        {
            nSaveFocusId = 0;
            ImplGetSVData()->maWinData.mbNoDeactivate = FALSE;
        }
    }
    ImplEndPopupMode( 0, nFocusId );

    aHighlightChangedTimer.Stop();
    bInExecute = FALSE;
    if ( pActivePopup )
    {
        KillActivePopup();
    }
}

void MenuFloatingWindow::KillActivePopup( PopupMenu* pThisOnly )
{
    if ( pActivePopup && ( !pThisOnly || ( pThisOnly == pActivePopup ) ) )
    {
        if ( pActivePopup->bInCallback )
            pActivePopup->bCanceled = TRUE;

        // Vor allen Aktionen schon pActivePopup = 0, falls z.B.
        // PopupModeEndHdl des zu zerstoerenden Popups mal synchron gerufen wird.
        PopupMenu* pPopup = pActivePopup;
        pActivePopup = NULL;
        pPopup->bInCallback = TRUE;
        pPopup->Deactivate();
        pPopup->bInCallback = FALSE;
        if ( pPopup->ImplGetWindow() )
        {
            pPopup->ImplGetFloatingWindow()->StopExecute();
            delete pPopup->pWindow;
            pPopup->pWindow = NULL;
            Update();
        }
    }
}

void MenuFloatingWindow::EndExecute()
{
    Menu* pStart = pMenu->ImplGetStartMenu();
    ULONG nFocusId = 0;
    if ( pStart && pStart->bIsMenuBar )
    {
        nFocusId = ((MenuBarWindow*)((MenuBar*)pStart)->ImplGetWindow())->GetFocusId();
        if ( nFocusId )
        {
            ((MenuBarWindow*)((MenuBar*)pStart)->ImplGetWindow())->SetFocusId( 0 );
            ImplGetSVData()->maWinData.mbNoDeactivate = FALSE;
        }
    }
    // Wenn von woanders gestartet, dann ab dort aufraumen:
    MenuFloatingWindow* pCleanUpFrom = this;
    MenuFloatingWindow* pWin = this;
    while ( pWin && !pWin->bInExecute &&
        pWin->pMenu->pStartedFrom && !pWin->pMenu->pStartedFrom->bIsMenuBar )
    {
        pWin = ((PopupMenu*)pWin->pMenu->pStartedFrom)->ImplGetFloatingWindow();
    }
    if ( pWin )
        pCleanUpFrom = pWin;

    // Dies Fenster wird gleich zerstoert => Daten lokal merken...
    Menu* pM = pMenu;
    USHORT nItem = nHighlightedItem;

    pCleanUpFrom->StopExecute( nFocusId );

    if ( nItem != ITEMPOS_INVALID )
    {
        MenuItemData* pItemData = pM->GetItemList()->GetDataFromPos( nItem );
        if ( pItemData && !pItemData->bIsTemporary )
        {
            pM->nSelectedId = pItemData->nId;
            if ( pStart )
                pStart->nSelectedId = pItemData->nId;

            pM->ImplSelect();
        }
    }
}

void MenuFloatingWindow::EndExecute( USHORT nId )
{
    USHORT nPos;
    if ( pMenu->GetItemList()->GetData( nId, nPos ) )
        nHighlightedItem = nPos;
    else
        nHighlightedItem = ITEMPOS_INVALID;

    EndExecute();
}

void MenuFloatingWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    // TH macht ein ToTop auf dieses Fenster, aber das aktive Popup
    // soll oben bleiben...
    if ( pActivePopup && pActivePopup->ImplGetWindow() && !pActivePopup->ImplGetFloatingWindow()->pActivePopup )
        pActivePopup->ImplGetFloatingWindow()->ToTop();

    if ( !ImplIsMouseFollow() )
    {
        ImplHighlightItem( rMEvt, TRUE );
    }

    nMBDownPos = nHighlightedItem;
}

void MenuFloatingWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    MenuItemData* pData = pMenu->GetItemList()->GetDataFromPos( nHighlightedItem );
    // nMBDownPos in lokaler Variable merken und gleich zuruecksetzen,
    // weil nach EndExecute zu spaet
    USHORT _nMBDownPos = nMBDownPos;
    nMBDownPos = ITEMPOS_INVALID;
    if ( pData && pData->bEnabled && ( pData->eType != MENUITEM_SEPARATOR ) )
    {
        if ( !pData->pSubMenu )
        {
            EndExecute();
        }
        else if ( ( pData->nBits & MIB_POPUPSELECT ) && ( nHighlightedItem == _nMBDownPos ) && ( rMEvt.GetClicks() == 2 ) )
        {
            // Nicht wenn ueber dem Pfeil geklickt wurde...
            Size aSz = GetOutputSizePixel();
            long nFontHeight = GetTextHeight();
            if ( rMEvt.GetPosPixel().X() < ( aSz.Width() - nFontHeight - nFontHeight/4 ) )
                EndExecute();
        }
    }

}

void MenuFloatingWindow::MouseMove( const MouseEvent& rMEvt )
{
    if ( !IsVisible() || rMEvt.IsSynthetic() )
        return;

    if ( rMEvt.IsLeaveWindow() )
    {
        if ( ImplIsMouseFollow() || ( rMEvt.GetButtons() == MOUSE_LEFT ) )
            ChangeHighlightItem( ITEMPOS_INVALID, FALSE );
        if ( IsScrollMenu() )
            ImplScroll( rMEvt.GetPosPixel() );
    }
    else if ( ImplIsMouseFollow() || ( rMEvt.GetButtons() == MOUSE_LEFT ) )
    {
        ImplHighlightItem( rMEvt, FALSE );
    }
}

void MenuFloatingWindow::ImplScroll( BOOL bUp )
{
    KillActivePopup();
    Update();

    HighlightItem( nHighlightedItem, FALSE );

    if ( bScrollUp && bUp )
    {
        MenuItemData* pData = pMenu->GetItemList()->GetDataFromPos( --nFirstEntry );
        long nEntryHeight = pData->aSz.Height();

        nStartY += nEntryHeight;

        if ( !bScrollDown )
        {
            bScrollDown = TRUE;
            ImplDrawScroller( FALSE );
        }

        if ( nFirstEntry == 0 )
        {
            bScrollUp = FALSE;
            ImplDrawScroller( TRUE );
        }
        Scroll( 0, nEntryHeight, ImplCalcClipRegion( FALSE ).GetBoundRect(), SCROLL_CLIP );
    }
    else if ( bScrollDown && !bUp )
    {
        MenuItemData* pData = pMenu->GetItemList()->GetDataFromPos( nFirstEntry++ );
        long nEntryHeight = pData->aSz.Height();

        if ( !bScrollUp )
        {
            bScrollUp = TRUE;
            ImplDrawScroller( TRUE );
        }

        Size aOutSz = GetOutputSizePixel();
        USHORT nVisible = ((PopupMenu*)pMenu)->ImplCalcVisEntries( aOutSz.Height(), nFirstEntry );
        if ( ( nFirstEntry + nVisible ) >= (USHORT) pMenu->GetItemList()->Count() )
        {
            bScrollDown = FALSE;
            ImplDrawScroller( FALSE );
        }

        nStartY -= nEntryHeight;
        Scroll( 0, -nEntryHeight, ImplCalcClipRegion( FALSE ).GetBoundRect(), SCROLL_CLIP );
    }

    HighlightItem( nHighlightedItem, TRUE );
}

void MenuFloatingWindow::ImplScroll( const Point& rMousePos )
{
    Size aOutSz = GetOutputSizePixel();

    long nY = nScrollerHeight;
    long nMouseY = rMousePos.Y();
    long nDelta = 0;

    if ( bScrollUp && ( nMouseY < nY ) )
    {
        ImplScroll( TRUE );
        nDelta = nY - nMouseY;
    }
    else if ( bScrollDown && ( nMouseY > ( aOutSz.Height() - nY ) ) )
    {
        ImplScroll( FALSE );
        nDelta = nMouseY - ( aOutSz.Height() - nY );
    }

    if ( nDelta )
    {
        aScrollTimer.Stop();    // Falls durch MouseMove gescrollt.
        long nTimeout;
        if ( nDelta < 3 )
            nTimeout = 200;
        else if ( nDelta < 5 )
            nTimeout = 100;
        else if ( nDelta < 8 )
            nTimeout = 70;
        else if ( nDelta < 12 )
            nTimeout = 40;
        else
            nTimeout = 20;
        aScrollTimer.SetTimeout( nTimeout );
        aScrollTimer.Start();
    }
}
void MenuFloatingWindow::ChangeHighlightItem( USHORT n, BOOL bStartPopupTimer )
{
    // #57934# ggf. das aktive Popup sofort schliessen, damit TH's Hintergrundsicherung funktioniert.
    // #65750# Dann verzichten wir lieber auf den schmalen Streifen Hintergrundsicherung.
    //         Sonst lassen sich die Menus schlecht bedienen.
//  MenuItemData* pNextData = pMenu->pItemList->GetDataFromPos( n );
//  if ( pActivePopup && pNextData && ( pActivePopup != pNextData->pSubMenu ) )
//      KillActivePopup();

    if ( nHighlightedItem != ITEMPOS_INVALID )
        HighlightItem( nHighlightedItem, FALSE );

    nHighlightedItem = (USHORT)n;
    DBG_ASSERT( ( nHighlightedItem == ITEMPOS_INVALID ) || pMenu->ImplIsVisible( nHighlightedItem ), "ChangeHighlightItem: Not visible!" );
    HighlightItem( nHighlightedItem, TRUE );
    pMenu->ImplCallHighlight( nHighlightedItem );

    if ( bStartPopupTimer )
        aHighlightChangedTimer.Start();
}

void MenuFloatingWindow::HighlightItem( USHORT nPos, BOOL bHighlight )
{
    Size    aSz = GetOutputSizePixel();
    USHORT  nBorder = nScrollerHeight;
    long    nY = nBorder+nStartY;
    long    nX = 0;

    if ( pMenu->pLogo )
        nX = pMenu->pLogo->aBitmap.GetSizePixel().Width();

    USHORT nCount = (USHORT)pMenu->pItemList->Count();
    for ( USHORT n = 0; n < nCount; n++ )
    {
        MenuItemData* pData = pMenu->pItemList->GetDataFromPos( n );
        if ( n == nPos )
        {
            DBG_ASSERT( pMenu->ImplIsVisible( n ), "Highlight: Item not visible!" );
            if ( pData->eType != MENUITEM_SEPARATOR )
            {
                if ( bHighlight )
                    SetFillColor( GetSettings().GetStyleSettings().GetMenuHighlightColor() );
                else
                    SetFillColor( GetSettings().GetStyleSettings().GetMenuColor() );

                Rectangle aRect( Point( nX, nY ), Size( aSz.Width(), pData->aSz.Height() ) );
                if ( pData->nBits & MIB_POPUPSELECT )
                {
                    long nFontHeight = GetTextHeight();
                    aRect.Right() -= nFontHeight + nFontHeight/4;
                }
                DrawRect( aRect );
                pMenu->ImplPaint( this, nBorder, nStartY, pData, bHighlight );
            }
            return;
        }

        nY += pData->aSz.Height();
    }
}

void MenuFloatingWindow::ImplCursorUpDown( BOOL bUp )
{
    USHORT n = nHighlightedItem;
    if ( n == ITEMPOS_INVALID )
    {
        if ( bUp )
            n = 0;
        else
            n = pMenu->GetItemCount()-1;
    }

    USHORT nLoop = n;
    do
    {
        if ( bUp )
        {
            if ( n )
                n--;
            else
                if ( !IsScrollMenu() || ( nHighlightedItem == ITEMPOS_INVALID ) )
                    n = pMenu->GetItemCount()-1;
                else
                    break;
        }
        else
        {
            n++;
            if ( n >= pMenu->GetItemCount() )
                if ( !IsScrollMenu() || ( nHighlightedItem == ITEMPOS_INVALID ) )
                    n = 0;
                else
                    break;
        }

        MenuItemData* pData = (MenuItemData*)pMenu->GetItemList()->GetDataFromPos( n );
        if ( ( pData->eType != MENUITEM_SEPARATOR ) && pMenu->ImplIsVisible( n ) )
        {
            // Selektion noch im sichtbaren Bereich?
            if ( IsScrollMenu() )
            {
                ChangeHighlightItem( ITEMPOS_INVALID, FALSE );

                while ( n < nFirstEntry )
                    ImplScroll( TRUE );

                Size aOutSz = GetOutputSizePixel();
                while ( n >= ( nFirstEntry + ((PopupMenu*)pMenu)->ImplCalcVisEntries( aOutSz.Height(), nFirstEntry ) ) )
                    ImplScroll( FALSE );
            }
            ChangeHighlightItem( n, FALSE );
            break;
        }
    } while ( n != nLoop );
}

void MenuFloatingWindow::KeyInput( const KeyEvent& rKEvent )
{
    USHORT nCode = rKEvent.GetKeyCode().GetCode();
    switch ( nCode )
    {
        case KEY_UP:
        case KEY_DOWN:
        {
            ImplCursorUpDown( nCode == KEY_UP );
        }
        break;
        case KEY_LEFT:
        {
            if ( pMenu->pStartedFrom )
            {
                StopExecute();
                if ( pMenu->pStartedFrom->bIsMenuBar )
                {
                    // Weiterkeiten...
                    ((MenuBarWindow*)((MenuBar*)pMenu->pStartedFrom)->ImplGetWindow())->KeyInput( rKEvent );
                }
                else
                {
                    MenuFloatingWindow* pFloat = ((PopupMenu*)pMenu->pStartedFrom)->ImplGetFloatingWindow();
                    pFloat->GrabFocus();
                    pFloat->KillActivePopup();
                }
            }
        }
        break;
        case KEY_RIGHT:
        {
            BOOL bDone = FALSE;
            if ( nHighlightedItem != ITEMPOS_INVALID )
            {
                MenuItemData* pData = pMenu->GetItemList()->GetDataFromPos( nHighlightedItem );
                if ( pData && pData->pSubMenu )
                {
                    HighlightChanged( 0 );
                    bDone = TRUE;
                }
            }
            if ( !bDone )
            {
                Menu* pStart = pMenu->ImplGetStartMenu();
                if ( pStart && pStart->bIsMenuBar )
                {
                    // Weiterkeiten...
                    pStart->ImplGetWindow()->KeyInput( rKEvent );
                }
            }
        }
        break;
        case KEY_RETURN:
        {
            MenuItemData* pData = pMenu->GetItemList()->GetDataFromPos( nHighlightedItem );
            if ( pData && pData->bEnabled )
            {
                if ( pData->pSubMenu )
                    HighlightChanged( 0 );
                else
                    EndExecute();
            }
            else
                StopExecute();
        }
        break;
        case KEY_MENU:
        {
            Menu* pStart = pMenu->ImplGetStartMenu();
            if ( pStart && pStart->bIsMenuBar )
            {
                // Weiterkeiten...
                pStart->ImplGetWindow()->KeyInput( rKEvent );
            }
        }
        break;
        default:
        {
            xub_Unicode nCharCode = rKEvent.GetCharCode();
            USHORT nPos;
            MenuItemData* pData = nCharCode ? pMenu->GetItemList()->SearchItem( nCharCode, nPos ) : NULL;
            if ( pData )
            {
                if ( pData->pSubMenu )
                {
                    ChangeHighlightItem( nPos, FALSE );
                    HighlightChanged( 0 );
                }
                else
                {
                    nHighlightedItem = nPos;
                    EndExecute();
                }
            }
            else
            {
                // Bei ungueltigen Tasten Beepen, aber nicht bei HELP und F-Tasten
                if ( !rKEvent.GetKeyCode().IsControlMod() && ( nCode != KEY_HELP ) && ( rKEvent.GetKeyCode().GetGroup() != KEYGROUP_FKEYS ) )
                    Sound::Beep();
                FloatingWindow::KeyInput( rKEvent );
            }
        }
    }
}

void MenuFloatingWindow::Paint( const Rectangle& rRect )
{
    if ( IsScrollMenu() )
    {
        ImplDrawScroller( TRUE );
        ImplDrawScroller( FALSE );
    }
    pMenu->ImplPaint( this, nScrollerHeight, nStartY );
    if ( nHighlightedItem != ITEMPOS_INVALID )
        HighlightItem( nHighlightedItem, TRUE );
}

void MenuFloatingWindow::ImplDrawScroller( BOOL bUp )
{
    SetClipRegion();

    Size aOutSz = GetOutputSizePixel();
    long nY = bUp ? 0 : ( aOutSz.Height() - nScrollerHeight );
    long nX = pMenu->pLogo ? pMenu->pLogo->aBitmap.GetSizePixel().Width() : 0;
    Rectangle aRect( Point( nX, nY ), Size( aOutSz.Width()-nX, nScrollerHeight ) );

    DecorationView aDecoView( this );
    SymbolType eSymbol = bUp ? SYMBOL_SPIN_UP : SYMBOL_SPIN_DOWN;

    USHORT nStyle = 0;
    if ( ( bUp && !bScrollUp ) || ( !bUp && !bScrollDown ) )
        nStyle |= SYMBOL_DRAW_DISABLE;

    aDecoView.DrawSymbol( aRect, eSymbol, GetSettings().GetStyleSettings().GetButtonTextColor(), nStyle );

    ImplInitClipRegion();
}

void MenuFloatingWindow::RequestHelp( const HelpEvent& rHEvt )
{
    USHORT nId = nHighlightedItem;
    Menu* pM = pMenu;
    Window* pW = this;
    if ( rHEvt.GetMode() & (HELPMODE_CONTEXT | HELPMODE_EXTENDED) )
    {
        nHighlightedItem = ITEMPOS_INVALID;
        EndExecute();
        pW = NULL;
    }
    if( !ImplHandleHelpEvent( pW, pM, nId, rHEvt ) )
        Window::RequestHelp( rHEvt );
}

void MenuFloatingWindow::StateChanged( StateChangedType nType )
{
    FloatingWindow::StateChanged( nType );

    if ( ( nType == STATE_CHANGE_CONTROLFOREGROUND ) || ( nType == STATE_CHANGE_CONTROLBACKGROUND ) )
    {
        ImplInitMenuWindow( this, FALSE );
        Invalidate();
    }
}

void MenuFloatingWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    FloatingWindow::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        ImplInitMenuWindow( this, FALSE );
        Invalidate();
    }
}

void MenuFloatingWindow::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_WHEEL )
    {
        const CommandWheelData* pData = rCEvt.GetWheelData();
        if( !pData->GetModifier() && ( pData->GetMode() == COMMAND_WHEEL_SCROLL ) )
        {
//          ImplCursorUpDown( pData->GetDelta() > 0L );
            ImplScroll( pData->GetDelta() > 0L );
            MouseMove( MouseEvent( GetPointerPosPixel(), 0 ) );
        }
    }
}



MenuBarWindow::MenuBarWindow( Window* pParent ) :
    Window( pParent, 0 ),
    aCloser( this, WB_NOPOINTERFOCUS | WB_SMALLSTYLE | WB_RECTSTYLE ),
    aFloatBtn( this, WB_NOPOINTERFOCUS | WB_SMALLSTYLE | WB_RECTSTYLE ),
    aHideBtn( this, WB_NOPOINTERFOCUS | WB_SMALLSTYLE | WB_RECTSTYLE )
{
    pMenu = NULL;
    pActivePopup = NULL;
    nSaveFocusId = 0;
    nHighlightedItem = ITEMPOS_INVALID;
    mbAutoPopup = TRUE;
    nSaveFocusId = 0;

    ResMgr* pResMgr = ImplGetResMgr();
    aCloser.SetClickHdl( LINK( this, MenuBarWindow, CloserHdl ) );
    aCloser.SetSymbol( SYMBOL_CLOSE );
    aCloser.SetQuickHelpText( XubString( ResId( SV_HELPTEXT_CLOSE, pResMgr ) ) );
    aFloatBtn.SetClickHdl( LINK( this, MenuBarWindow, FloatHdl ) );
    aFloatBtn.SetSymbol( SYMBOL_FLOAT );
    aFloatBtn.SetQuickHelpText( XubString( ResId( SV_HELPTEXT_RESTORE, pResMgr ) ) );
    aHideBtn.SetClickHdl( LINK( this, MenuBarWindow, HideHdl ) );
    aHideBtn.SetSymbol( SYMBOL_HIDE );
    aHideBtn.SetQuickHelpText( XubString( ResId( SV_HELPTEXT_MINIMIZE, pResMgr ) ) );
}

MenuBarWindow::~MenuBarWindow()
{
}

void MenuBarWindow::SetMenu( MenuBar* pMen )
{
    pMenu = pMen;
    KillActivePopup();
    nHighlightedItem = ITEMPOS_INVALID;
    ImplInitMenuWindow( this, TRUE );
    if ( pMen )
    {
        aCloser.Show( pMen->HasCloser() );
        aFloatBtn.Show( pMen->HasFloatButton() );
        aHideBtn.Show( pMen->HasHideButton() );
    }
    Invalidate();
}

void MenuBarWindow::ShowButtons( BOOL bClose, BOOL bFloat, BOOL bHide )
{
    aCloser.Show( bClose );
    aFloatBtn.Show( bFloat );
    aHideBtn.Show( bHide );
    Resize();
}

IMPL_LINK( MenuBarWindow, CloserHdl, PushButton*, EMPTYARG )
{
    return ((MenuBar*)pMenu)->GetCloserHdl().Call( pMenu );
}

IMPL_LINK( MenuBarWindow, FloatHdl, PushButton*, EMPTYARG )
{
    return ((MenuBar*)pMenu)->GetFloatButtonClickHdl().Call( pMenu );
}

IMPL_LINK( MenuBarWindow, HideHdl, PushButton*, EMPTYARG )
{
    return ((MenuBar*)pMenu)->GetHideButtonClickHdl().Call( pMenu );
}

void MenuBarWindow::ImplCreatePopup( BOOL bPreSelectFirst )
{
    MenuItemData* pData = pMenu->GetItemList()->GetDataFromPos( nHighlightedItem );
    if ( pData )
    {
        if ( pActivePopup && ( pActivePopup != pData->pSubMenu ) )
        {
            KillActivePopup();
        }
        if ( pData->bEnabled && pData->pSubMenu && ( nHighlightedItem != ITEMPOS_INVALID ) && ( pData->pSubMenu != pActivePopup ) )
        {
            pActivePopup = (PopupMenu*)pData->pSubMenu;
            long nX = 0;
            MenuItemData* pData = 0;
            for ( ULONG n = 0; n < nHighlightedItem; n++ )
            {
                pData = pMenu->GetItemList()->GetDataFromPos( n );
                nX += pData->aSz.Width();
            }
            pData = pMenu->pItemList->GetDataFromPos( nHighlightedItem );
//          Point MyPos = GetPosPixel();
            Point aItemTopLeft( nX, 0 );
            Point aItemBottomRight( aItemTopLeft );
            aItemBottomRight.X() += pData->aSz.Width();

            // Im Vollbild-Modus hat die MenuBar ggf. die Hoehe 0:
            // Nicht immer einfach die Window-Hoehe nehmen, weil ItemHeight < WindowHeight.
            if ( GetSizePixel().Height() )
                aItemBottomRight.Y() += pData->aSz.Height();

            // ImplExecute ist doch nicht modal...
            GrabFocus();
            pActivePopup->ImplExecute( this, Rectangle( aItemTopLeft, aItemBottomRight ), FLOATWIN_POPUPMODE_DOWN, pMenu, bPreSelectFirst );
            if ( pActivePopup )
            {
                // Hat kein Window, wenn vorher abgebrochen oder keine Eintraege
                if ( pActivePopup->ImplGetFloatingWindow() )
                    pActivePopup->ImplGetFloatingWindow()->AddPopupModeWindow( this );
                else
                    pActivePopup = NULL;
            }
        }
    }
}


void MenuBarWindow::KillActivePopup()
{
    if ( pActivePopup )
    {
        if ( pActivePopup->bInCallback )
            pActivePopup->bCanceled = TRUE;

        pActivePopup->bInCallback = TRUE;
        pActivePopup->Deactivate();
        pActivePopup->bInCallback = FALSE;
        // Abfrage auf pActivePopup, falls im Deactivate abgeschossen...
        if ( pActivePopup && pActivePopup->ImplGetWindow() )
        {
            pActivePopup->ImplGetFloatingWindow()->StopExecute();
            delete pActivePopup->pWindow;
            pActivePopup->pWindow = NULL;
        }
        pActivePopup = 0;
    }
}

void MenuBarWindow::PopupClosed( Menu* pPopup )
{
    if ( pPopup == pActivePopup )
    {
        KillActivePopup();
        ChangeHighlightItem( ITEMPOS_INVALID, FALSE );
    }
}

void MenuBarWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    mbAutoPopup = TRUE;
    USHORT nEntry = ImplFindEntry( rMEvt.GetPosPixel() );
    if ( ( nEntry != ITEMPOS_INVALID ) && ( nEntry != nHighlightedItem ) )
    {
        ChangeHighlightItem( nEntry, ImplIsMouseFollow() ? FALSE : TRUE );
    }
    else
    {
        KillActivePopup();
        ChangeHighlightItem( ITEMPOS_INVALID, FALSE );
    }
}

void MenuBarWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
}

void MenuBarWindow::MouseMove( const MouseEvent& rMEvt )
{
    // Im Move nur Highlighten, wenn schon eins gehighlightet.
    if ( rMEvt.IsSynthetic() || rMEvt.IsLeaveWindow() || ( nHighlightedItem == ITEMPOS_INVALID ) )
        return;

    USHORT nEntry = ImplFindEntry( rMEvt.GetPosPixel() );
    if ( ( nEntry != ITEMPOS_INVALID ) && ( nEntry != nHighlightedItem )
         && ( ImplIsMouseFollow() || ( rMEvt.GetButtons() == MOUSE_LEFT ) ) )
        ChangeHighlightItem( nEntry, FALSE );
}

void MenuBarWindow::ChangeHighlightItem( USHORT n, BOOL bSelectEntry, BOOL bAllowRestoreFocus )
{
    // #57934# ggf. das aktive Popup sofort schliessen, damit TH's Hintergrundsicherung funktioniert.
    MenuItemData* pNextData = pMenu->pItemList->GetDataFromPos( n );
    if ( pActivePopup && pActivePopup->ImplGetWindow() && ( !pNextData || ( pActivePopup != pNextData->pSubMenu ) ) )
        KillActivePopup(); // pActivePopup ggf. ohne pWin, wenn in Activate() Rescheduled wurde

    // Activate am MenuBar immer nur einmal pro Vorgang...
    BOOL bJustActivated = FALSE;
    if ( ( nHighlightedItem == ITEMPOS_INVALID ) && ( n != ITEMPOS_INVALID ) )
    {
        ImplGetSVData()->maWinData.mbNoDeactivate = TRUE;
        nSaveFocusId = Window::SaveFocus();
        pMenu->bInCallback = TRUE;  // hier schon setzen, falls Activate ueberladen
        pMenu->Activate();
        pMenu->bInCallback = FALSE;
        bJustActivated = TRUE;
    }
    else if ( ( nHighlightedItem != ITEMPOS_INVALID ) && ( n == ITEMPOS_INVALID ) )
    {
        pMenu->bInCallback = TRUE;
        pMenu->Deactivate();
        pMenu->bInCallback = FALSE;
        ULONG nTempFocusId = nSaveFocusId;
        nSaveFocusId = 0;
        ImplGetSVData()->maWinData.mbNoDeactivate = FALSE;
        Window::EndSaveFocus( nTempFocusId, bAllowRestoreFocus );
    }

    if ( nHighlightedItem != ITEMPOS_INVALID )
        HighlightItem( nHighlightedItem, FALSE );

    nHighlightedItem = (USHORT)n;
    DBG_ASSERT( ( nHighlightedItem == ITEMPOS_INVALID ) || pMenu->ImplIsVisible( nHighlightedItem ), "ChangeHighlightItem: Not visible!" );
    HighlightItem( nHighlightedItem, TRUE );
    pMenu->ImplCallHighlight( nHighlightedItem );

    if ( mbAutoPopup )
        ImplCreatePopup( bSelectEntry );

    // #58935# #73659# Focus, wenn kein Popup drunter haengt...
    if ( bJustActivated && !pActivePopup )
        GrabFocus();
}

void MenuBarWindow::HighlightItem( USHORT nPos, BOOL bHighlight )
{
    long nX = 0;
    ULONG nCount = pMenu->pItemList->Count();
    for ( ULONG n = 0; n < nCount; n++ )
    {
        MenuItemData* pData = pMenu->pItemList->GetDataFromPos( n );
        if ( n == nPos )
        {
            if ( pData->eType != MENUITEM_SEPARATOR )
            {
                if ( bHighlight )
                    SetFillColor( GetSettings().GetStyleSettings().GetMenuHighlightColor() );
                else
                    SetFillColor( GetSettings().GetStyleSettings().GetMenuColor() );

                DrawRect( Rectangle( Point( nX, 1 ), Size( pData->aSz.Width(), pData->aSz.Height()-2 ) ) );
                pMenu->ImplPaint( this, 0, 0, pData, bHighlight );
            }
            return;
        }

        nX += pData->aSz.Width();
    }
}

void MenuBarWindow::KeyInput( const KeyEvent& rKEvent )
{
    if ( !ImplHandleKeyEvent( rKEvent ) )
        Window::KeyInput( rKEvent );
}

BOOL MenuBarWindow::ImplHandleKeyEvent( const KeyEvent& rKEvent, BOOL bFromMenu )
{
    if ( pMenu->bInCallback )
        return TRUE;    // schlucken

    BOOL bDone = FALSE;
    USHORT nCode = rKEvent.GetKeyCode().GetCode();
    if ( nCode == KEY_MENU )
    {
        mbAutoPopup = FALSE;
        if ( nHighlightedItem == ITEMPOS_INVALID )
        {
            ChangeHighlightItem( 0, FALSE );
            GrabFocus();
        }
        else
        {
            ChangeHighlightItem( ITEMPOS_INVALID, FALSE );
            nSaveFocusId = 0;
        }
        bDone = TRUE;
    }
    else if ( bFromMenu )
    {
        if ( ( nCode == KEY_LEFT ) || ( nCode == KEY_RIGHT ) )
        {
            USHORT n = nHighlightedItem;
            if ( n == ITEMPOS_INVALID )
            {
                if ( nCode == KEY_LEFT)
                    n = 0;
                else
                    n = pMenu->GetItemCount()-1;
            }

            USHORT nLoop = n;
            do
            {
                if ( nCode == KEY_LEFT )
                {
                    if ( n )
                        n--;
                    else
                        n = pMenu->GetItemCount()-1;
                }
                else
                {
                    n++;
                    if ( n >= pMenu->GetItemCount() )
                        n = 0;
                }

                MenuItemData* pData = (MenuItemData*)pMenu->GetItemList()->GetDataFromPos( n );
                if ( ( pData->eType != MENUITEM_SEPARATOR ) && pMenu->ImplIsVisible( n ) )
                {
                    ChangeHighlightItem( n, TRUE );
                    break;
                }
            } while ( n != nLoop );
            bDone = TRUE;
        }
        else if ( nCode == KEY_RETURN )
        {
            KillActivePopup();
            bDone = TRUE;
        }
        else if ( ( nCode == KEY_UP ) || ( nCode == KEY_DOWN ) )
        {
            if ( !mbAutoPopup )
            {
                ImplCreatePopup( TRUE );
                mbAutoPopup = TRUE;
            }
            bDone = TRUE;
        }
        else if ( nCode == KEY_ESCAPE)
        {
            ChangeHighlightItem( ITEMPOS_INVALID, FALSE );
            bDone = TRUE;
        }
    }

    if ( !bDone && ( bFromMenu || rKEvent.GetKeyCode().IsControlMod() ) )
    {
        xub_Unicode nCharCode = rKEvent.GetCharCode();
        if ( nCharCode )
        {
            USHORT nEntry;
            MenuItemData* pData = pMenu->GetItemList()->SearchItem( nCharCode, nEntry );
            if ( pData && (nEntry != ITEMPOS_INVALID) )
            {
                mbAutoPopup = TRUE;
                ChangeHighlightItem( nEntry, TRUE );
                bDone = TRUE;
            }
            else
            {
                // Wegen Systemmenu und anderen System-HotKeys, nur
                // eigenstaendige Character-Kombinationen auswerten
                USHORT nKeyCode = rKEvent.GetKeyCode().GetCode();
                if ( !nKeyCode ||
                     ((nKeyCode >= KEY_A) && (nKeyCode <= KEY_Z)) )
                    Sound::Beep();
            }
        }
    }
    return bDone;
}

void MenuBarWindow::Paint( const Rectangle& rRect )
{
    pMenu->ImplPaint( this, 0 );
    if ( nHighlightedItem != ITEMPOS_INVALID )
        HighlightItem( nHighlightedItem, TRUE );
}

void MenuBarWindow::Resize()
{
    Size aOutSz = GetOutputSizePixel();
    long n      = aOutSz.Height()-4;
    long nX     = aOutSz.Width()-3;
    long nY     = 2;

    ULONG nStyle = GetSettings().GetStyleSettings().GetOptions();
    if ( nStyle & (STYLE_OPTION_OS2STYLE | STYLE_OPTION_UNIXSTYLE | STYLE_OPTION_MACSTYLE) )
    {
        if ( nStyle & STYLE_OPTION_OS2STYLE )
        {
            nX += 3;
            nY -= 2;
            n  += 4;
        }

        if ( aFloatBtn.IsVisible() )
        {
            nX -= n;
            aFloatBtn.SetPosSizePixel( nX, nY, n, n );
        }
        if ( aHideBtn.IsVisible() )
        {
            nX -= n;
            aHideBtn.SetPosSizePixel( nX, nY, n, n );
        }
        if ( nStyle & (STYLE_OPTION_MACSTYLE | STYLE_OPTION_UNIXSTYLE) )
        {
            if ( aFloatBtn.IsVisible() || aHideBtn.IsVisible() )
                nX -= 3;
        }
        if ( aCloser.IsVisible() )
        {
            nX -= n;
            aCloser.SetPosSizePixel( nX, nY, n, n );
        }
    }
    else
    {
        if ( aCloser.IsVisible() )
        {
            nX -= n;
            aCloser.SetPosSizePixel( nX, nY, n, n );
            nX -= 3;
        }
        if ( aFloatBtn.IsVisible() )
        {
            nX -= n;
            aFloatBtn.SetPosSizePixel( nX, nY, n, n );
        }
        if ( aHideBtn.IsVisible() )
        {
            nX -= n;
            aHideBtn.SetPosSizePixel( nX, nY, n, n );
        }
    }

    if ( nStyle & STYLE_OPTION_OS2STYLE )
        aFloatBtn.SetSymbol( SYMBOL_OS2FLOAT );
    else
        aFloatBtn.SetSymbol( SYMBOL_FLOAT );
    if ( nStyle & STYLE_OPTION_OS2STYLE )
        aHideBtn.SetSymbol( SYMBOL_OS2HIDE );
    else
        aHideBtn.SetSymbol( SYMBOL_HIDE );
    if ( nStyle & STYLE_OPTION_OS2STYLE )
        aCloser.SetSymbol( SYMBOL_OS2CLOSE );
    else
        aCloser.SetSymbol( SYMBOL_CLOSE );
}

USHORT MenuBarWindow::ImplFindEntry( const Point& rMousePos ) const
{
    long nX = 0;
    USHORT nCount = (USHORT)pMenu->pItemList->Count();
    for ( USHORT n = 0; n < nCount; n++ )
    {
        MenuItemData* pData = pMenu->pItemList->GetDataFromPos( n );
        if ( pMenu->ImplIsVisible( n ) )
        {
            nX += pData->aSz.Width();
            if ( nX > rMousePos.X() )
                return (USHORT)n;
        }
    }
    return ITEMPOS_INVALID;
}

void MenuBarWindow::RequestHelp( const HelpEvent& rHEvt )
{
    USHORT nId = nHighlightedItem;
    if ( rHEvt.GetMode() & (HELPMODE_CONTEXT | HELPMODE_EXTENDED) )
        ChangeHighlightItem( ITEMPOS_INVALID, TRUE );
    if( !ImplHandleHelpEvent( this, pMenu, nId, rHEvt ) )
        Window::RequestHelp( rHEvt );
}

#ifndef TF_SVDATA
BOOL MenuBarWindow::QueryDrop( DropEvent& rDEvt )
{
    Window* pW = GetParent()->ImplGetWindow();
    DropEvent aEvent( ImplTranslateDropEvent( rDEvt, this, pW ) );
    BOOL bRet = pW->QueryDrop( aEvent );
    rDEvt = aEvent;
    return bRet;
}

BOOL MenuBarWindow::Drop( const DropEvent& rDEvt )
{
    Window* pW = GetParent()->ImplGetWindow();
    return pW->Drop( ImplTranslateDropEvent( rDEvt, this, pW ) );
}
#endif

void MenuBarWindow::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    if ( ( nType == STATE_CHANGE_CONTROLFOREGROUND ) ||
         ( nType == STATE_CHANGE_CONTROLBACKGROUND ) )
    {
        ImplInitMenuWindow( this, FALSE );
        Invalidate();
    }
}

void MenuBarWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        ImplInitMenuWindow( this, TRUE );
        // Falls sich der Font geaendert hat.
        long nHeight = pMenu->ImplCalcSize( this ).Height();
        SetPosSizePixel( 0, 0, 0, nHeight, WINDOW_POSSIZE_HEIGHT );
        GetParent()->Resize();
        Invalidate();
        Resize();
    }
}

void MenuBarWindow::LoseFocus()
{
    if ( !HasChildPathFocus( TRUE ) )
        ChangeHighlightItem( ITEMPOS_INVALID, FALSE, FALSE );
}

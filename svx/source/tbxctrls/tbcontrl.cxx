/*************************************************************************
 *
 *  $RCSfile: tbcontrl.cxx,v $
 *
 *  $Revision: 1.38 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:04:58 $
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

// include ---------------------------------------------------------------

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX //autogen wg. ToolBox
#include <vcl/toolbox.hxx>
#endif
#ifndef _SV_SYSTEM_HXX //autogen wg. System
#include <vcl/system.hxx>
#endif
#ifndef _VALUESET_HXX
#include <svtools/valueset.hxx>
#endif
#ifndef _CTRLBOX_HXX //autogen
#include <svtools/ctrlbox.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _CTRLTOOL_HXX //autogen
#include <svtools/ctrltool.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFX_TPLPITEM_HXX
#include <sfx2/tplpitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWSH_HXX //autogen
#include <sfx2/viewsh.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFXISETHINT_HXX
#include <svtools/isethint.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_FONTOPTIONS_HXX
#include <svtools/fontoptions.hxx>
#endif
#pragma hdrstop

#define _SVX_TBCONTRL_CXX

#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif

#include "dialogs.hrc"
#include "svxitems.hrc"
#include "helpid.hrc"

#define ITEMID_COLOR_TABLE  0
#define ITEMID_FONTLIST     SID_ATTR_CHAR_FONTLIST
#define ITEMID_FONT         SID_ATTR_CHAR_FONT
#define ITEMID_FONTHEIGHT   SID_ATTR_CHAR_FONTHEIGHT
#define ITEMID_COLOR        SID_ATTR_CHAR_COLOR
#define ITEMID_BRUSH        SID_ATTR_BRUSH
#define ITEMID_BOX          SID_ATTR_BORDER_OUTER
#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#define ITEMID_LINE         SID_FRAME_LINESTYLE

#include "htmlmode.hxx"
#include "xtable.hxx"
#include "fontitem.hxx"
#include "fhgtitem.hxx"
#include "brshitem.hxx"
#include "boxitem.hxx"
#include "colritem.hxx"
#include "flstitem.hxx"
#include "bolnitem.hxx"
#include "drawitem.hxx"
#include "tbcontrl.hxx"
#include "dlgutil.hxx"
#include "dialmgr.hxx"

// ------------------------------------------------------------------------

#define IMAGE_COL_TRANSPARENT       COL_LIGHTMAGENTA
#define MAX_MRU_FONTNAME_ENTRIES    5
#define LOGICAL_EDIT_HEIGHT         12
// STATIC DATA -----------------------------------------------------------

#ifndef DELETEZ
#define DELETEZ(p) (delete (p), (p)=NULL)
#endif
#define MAX_STYLES_ENTRIES  ((USHORT)26)

void lcl_ResizeValueSet( Window &rWin, ValueSet &rValueSet );
void lcl_CalcSizeValueSet( Window &rWin, ValueSet &rValueSet, const Size &aItemSize );
BOOL lcl_FontChangedHint( const SfxHint &rHint );

SFX_IMPL_TOOLBOX_CONTROL( SvxStyleToolBoxControl, SfxTemplateItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxFontNameToolBoxControl, SvxFontItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxFontHeightToolBoxControl, SvxFontHeightItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxFontColorToolBoxControl, SvxColorItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxFontColorExtToolBoxControl, SvxColorItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxColorToolBoxControl, SvxColorItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxFrameToolBoxControl, SvxBoxItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxFrameLineStyleToolBoxControl, SvxLineItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxFrameLineColorToolBoxControl, SvxColorItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxReloadControllerItem,  SfxBoolItem );

//========================================================================
// class SvxStyleBox_Impl -----------------------------------------------------
//========================================================================

class SvxStyleBox_Impl : public ListBox
{
public:
    SvxStyleBox_Impl( Window* pParent, USHORT nSlot, SfxStyleFamily eFamily, SfxBindings& rBind );
    ~SvxStyleBox_Impl();

    void            SetFamily( SfxStyleFamily eNewFamily );
    BOOL            IsVisible() { return bVisible; }

    virtual long    PreNotify( NotifyEvent& rNEvt );
    virtual long    Notify( NotifyEvent& rNEvt );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual void    StateChanged( StateChangedType nStateChange );

    void            SetVisibilityListener( const Link& aVisListener ) { aVisibilityListener = aVisListener; }
    void            RemoveVisibilityListener() { aVisibilityListener = Link(); }

protected:
    virtual void    Select();

private:
    USHORT          nSlotId;
    SfxStyleFamily  eStyleFamily;
    USHORT          nCurSel;
    BOOL            bRelease;
    SfxBindings&    rBindings;
    Size            aLogicalSize;
    Link            aVisibilityListener;
    BOOL            bVisible;

    void            ReleaseFocus();
};

//========================================================================
// class SvxFontNameBox --------------------------------------------------
//========================================================================

class SvxFontNameBox_Impl : public FontNameBox, public SfxListener
{
private:
    const FontList* pFontList;
    Font            aCurFont;
    Size            aLogicalSize;
    String          aCurText;
    USHORT          nFtCount;
    BOOL            bRelease;
    SfxBindings&    rBindings;

    void            ReleaseFocus_Impl();
    void            EnableControls_Impl();

protected:
    virtual void    Select();
    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

public:
    SvxFontNameBox_Impl( Window* pParent, SfxBindings& rBind, WinBits nStyle = WB_SORT );

    void            FillList();
    void            Update( const SvxFontItem* pFontItem );
    USHORT          GetListCount() { return nFtCount; }
    void            Clear() { FontNameBox::Clear(); nFtCount = 0; }
    void            Fill( const FontList* pList )
                        { FontNameBox::Fill( pList );
                          nFtCount = pList->GetFontNameCount(); }
    virtual long    PreNotify( NotifyEvent& rNEvt );
    virtual long    Notify( NotifyEvent& rNEvt );
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > CreateAccessible();
};

//========================================================================
// class SvxFontSizeBox_Impl --------------------------------------------------
//========================================================================

class SvxFontHeightToolBoxControl;

class SvxFontSizeBox_Impl : public FontSizeBox
{
private:
    SvxFontHeightToolBoxControl*    pCtrl;
    String                          aCurText;
    Size                            aLogicalSize;
    BOOL                            bRelease;
    SfxBindings&                    rBindings;

    void                ReleaseFocus_Impl();

protected:
    virtual void        Select();
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

public:
                        SvxFontSizeBox_Impl( Window* pParent,
                                        SvxFontHeightToolBoxControl& rCtrl, SfxBindings& rBind );

    void                StateChanged_Impl( SfxItemState eState,
                                           const SfxPoolItem* pState );
    void                Update( const SvxFontItem& rFontItem );

    virtual long        Notify( NotifyEvent& rNEvt );
};

//========================================================================
// class SvxColorWindow_Impl --------------------------------------------------
//========================================================================

class SvxColorWindow_Impl : public SfxPopupWindow, public SfxListener
{
private:
    const USHORT    theSlotId;
    ValueSet        aColorSet;

#if _SOLAR__PRIVATE
    DECL_LINK( SelectHdl, void * );
#endif

protected:
    virtual void    Resize();
    virtual BOOL    Close();

public:
    SvxColorWindow_Impl( USHORT nId, USHORT nSlotId,
                    const String& rWndTitle,
                    SfxBindings& rBindings );
    ~SvxColorWindow_Impl();
    void            StartSelection();

    virtual void    SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                            const SfxHint& rHint, const TypeId& rHintType );
    virtual void        KeyInput( const KeyEvent& rKEvt );

    virtual SfxPopupWindow* Clone() const;
};

//========================================================================
// class SvxFrameWindow_Impl --------------------------------------------------
//========================================================================

// fuer den SelectHdl werden die Modifier gebraucht, also
// muss man sie im MouseButtonUp besorgen

class SvxFrmValueSet_Impl : public ValueSet
{
    USHORT          nModifier;

    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
public:
    SvxFrmValueSet_Impl(Window* pParent,  WinBits nWinStyle)
        : ValueSet(pParent, nWinStyle), nModifier(0) {}
    USHORT          GetModifier() const {return nModifier;}

};

void SvxFrmValueSet_Impl::MouseButtonUp( const MouseEvent& rMEvt )
{
    nModifier = rMEvt.GetModifier();
    ValueSet::MouseButtonUp(rMEvt);
}

class SvxFrameWindow_Impl : public SfxPopupWindow
{
private:
    SvxFrmValueSet_Impl  aFrameSet;
    ImageList       aImgList;

#if _SOLAR__PRIVATE
    DECL_LINK( SelectHdl, void * );
#endif

protected:
    virtual void    Resize();
    virtual BOOL    Close();
    virtual Window* GetPreferredKeyInputWindow();
    virtual void    GetFocus();

public:
    SvxFrameWindow_Impl( USHORT nId, SfxBindings& rBindings, BOOL bParagraphMode );

    void            StartSelection();

    virtual void    StateChanged( USHORT nSID, SfxItemState eState,
                                  const SfxPoolItem* pState );
    virtual SfxPopupWindow* Clone() const;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    inline BOOL     IsHighContrast( void ) const;
};

inline BOOL SvxFrameWindow_Impl::IsHighContrast( void ) const
{
    return GetDisplayBackground().GetColor().IsDark();
}

//========================================================================
// class SvxLineWindow_Impl ---------------------------------------------------
//========================================================================
class SvxLineWindow_Impl : public SfxPopupWindow
{
private:
    ValueSet        aLineSet;

#if _SOLAR__PRIVATE
    void            MakeLineBitmap( USHORT nNo, Bitmap& rBmp, const Size& rSize, String& rStr,
                                    const Color& rLine, const Color& rBack );
    DECL_LINK( SelectHdl, void * );
#endif

protected:
    virtual void    Resize();
    virtual BOOL    Close();
    virtual Window* GetPreferredKeyInputWindow();
    virtual void    GetFocus();
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    void            CreateBitmaps( void );
public:
    SvxLineWindow_Impl( USHORT nId, SfxBindings& rBindings );

    void                    StartSelection();
    virtual SfxPopupWindow* Clone() const;
};

//########################################################################
// Hilfsklassen:
//========================================================================
// class SvxTbxButtonColorUpdater_Impl ----------------------------------------
//========================================================================
#define TBX_UPDATER_MODE_NONE               0x00
#define TBX_UPDATER_MODE_CHAR_COLOR         0x01
#define TBX_UPDATER_MODE_CHAR_BACKGROUND    0x02
#define TBX_UPDATER_MODE_CHAR_COLOR_NEW     0x03

class SvxTbxButtonColorUpdater_Impl
{
public:
                SvxTbxButtonColorUpdater_Impl( USHORT nTbxBtnId,
                                          ToolBox* ptrTbx, USHORT nMode = 0 );
                ~SvxTbxButtonColorUpdater_Impl();

    void        Update( const Color& rColor );

protected:
    void        DrawChar(VirtualDevice&, const Color&);

private:
    USHORT      nDrawMode;
    USHORT      nBtnId;
    ToolBox*    pTbx;
    Bitmap*     pBtnBmp;
    Color       aCurColor;
    Rectangle   theUpdRect;
    Size        theBmpSize;
    BOOL        bWasHiContrastMode;
};

//========================================================================
// class SfxStyleControllerItem ------------------------------------------
//========================================================================
class SvxStyleToolBoxControl;

class SfxStyleControllerItem_Impl : public SfxControllerItem
{
public:
    SfxStyleControllerItem_Impl( USHORT nId, SfxBindings& rBindings, SvxStyleToolBoxControl& rTbxCtl );

protected:
    virtual void StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );

private:
    SvxStyleToolBoxControl& rControl;
};

//========================================================================
// class SvxStyleBox_Impl -----------------------------------------------------
//========================================================================

SvxStyleBox_Impl::SvxStyleBox_Impl( Window* pParent, USHORT nSlot, SfxStyleFamily eFamily, SfxBindings& rBind ) :

    ListBox( pParent, SVX_RES( RID_SVXTBX_STYLE ) ),

    eStyleFamily( eFamily ),
    nSlotId     ( nSlot ),
    bRelease    ( TRUE ),
    rBindings   ( rBind )

{
    aLogicalSize = PixelToLogic( GetSizePixel(), MAP_APPFONT );
}

SvxStyleBox_Impl::~SvxStyleBox_Impl()
{
}

// -----------------------------------------------------------------------

void SvxStyleBox_Impl::ReleaseFocus()
{
    if ( !bRelease )
    {
        bRelease = TRUE;
        return;
    }
    SfxViewShell* pCurSh = SfxViewShell::Current();

    if ( pCurSh )
    {
        Window* pShellWnd = pCurSh->GetWindow();

        if ( pShellWnd )
            pShellWnd->GrabFocus();
    }
}

// -----------------------------------------------------------------------

void SvxStyleBox_Impl::Select()
{
    if ( !IsTravelSelect() )
    {
        SfxStringItem aItem( nSlotId, GetSelectEntry() );
        SfxUInt16Item aFamily( SID_STYLE_FAMILY, eStyleFamily );
        rBindings.GetDispatcher()->Execute(
            nSlotId, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD, &aItem, &aFamily, 0L );
        ReleaseFocus();
    }
}
// -----------------------------------------------------------------------

void SvxStyleBox_Impl::SetFamily( SfxStyleFamily eNewFamily )
{
    eStyleFamily = eNewFamily;
}

// -----------------------------------------------------------------------

long SvxStyleBox_Impl::PreNotify( NotifyEvent& rNEvt )
{
    USHORT nType = rNEvt.GetType();

    if ( EVENT_MOUSEBUTTONDOWN == nType || EVENT_GETFOCUS == nType )
        nCurSel = GetSelectEntryPos();
    else if(EVENT_LOSEFOCUS == nType)
        SelectEntryPos(GetSavedValue());
    return ListBox::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long SvxStyleBox_Impl::Notify( NotifyEvent& rNEvt )
{
    long nHandled = 0;

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        USHORT nCode = rNEvt.GetKeyEvent()->GetKeyCode().GetCode();

        switch ( nCode )
        {
            case KEY_RETURN:
            case KEY_TAB:
            {
                if ( KEY_TAB == nCode )
                    bRelease = FALSE;
                else
                    nHandled = 1;
                Select();
                break;
            }

            case KEY_ESCAPE:
                SelectEntryPos( nCurSel );
                ReleaseFocus();
                nHandled = 1;
                break;
        }
    }
    return nHandled ? nHandled : ListBox::Notify( rNEvt );
}
/* -----------------------------08.03.2002 13:03------------------------------

 ---------------------------------------------------------------------------*/
void SvxStyleBox_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        SetSizePixel(LogicToPixel(aLogicalSize, MAP_APPFONT));
        Size aDropSize( aLogicalSize.Width(), LOGICAL_EDIT_HEIGHT);
        SetDropDownSizePixel(LogicToPixel(aDropSize, MAP_APPFONT));
    }

    ListBox::DataChanged( rDCEvt );
}

void SvxStyleBox_Impl::StateChanged( StateChangedType nStateChange )
{
    ListBox::StateChanged( nStateChange );

    if ( nStateChange == STATE_CHANGE_VISIBLE )
    {
        bVisible = IsReallyVisible();
        if ( aVisibilityListener.IsSet() )
            aVisibilityListener.Call( this );
    }
    else if ( nStateChange == STATE_CHANGE_INITSHOW )
    {
        bVisible = TRUE;
        if ( aVisibilityListener.IsSet() )
            aVisibilityListener.Call( this );
    }
}


// -----------------------------------------------------------------------

#define BRUSH(style,name) BrushStyle(style),SVX_RESSTR(name)
#define COLOR(color,name) Color(color),SVX_RESSTR(name)

BOOL GetDocFontList_Impl( const FontList** ppFontList, SvxFontNameBox_Impl* pBox )
{
    BOOL bChanged = FALSE;
    const SfxObjectShell* pDocSh = SfxObjectShell::Current();
    SvxFontListItem* pFontListItem = NULL;

    if ( pDocSh )
        pFontListItem =
            (SvxFontListItem*)pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST );

    if ( pFontListItem )
    {
        const FontList* pNewFontList = pFontListItem->GetFontList();
        DBG_ASSERT( pNewFontList, "Doc-FontList not available!" );

        // keine alte Liste, aber neue Liste
        if ( !*ppFontList && pNewFontList )
        {
            // => "ubernehmen
            *ppFontList = pNewFontList;
            bChanged = TRUE;
        }
        else
        {
            // Vergleich der Fontlisten ist nicht vollkommen
            // wird die Fontliste am Doc geaendert, kann man hier
            // nur ueber die Listbox Aenderungen feststellen, weil
            // ppFontList dabei schon upgedatet wurde
            bChanged =
                ( ( *ppFontList != pNewFontList ) ||
                  pBox->GetListCount() != pNewFontList->GetFontNameCount() );
            HACK(vergleich ist unvollstaendig)

            if ( bChanged )
                *ppFontList = pNewFontList;
        }

        if ( pBox )
            pBox->Enable();
    }
    else if ( pBox )
        pBox->Disable();

    // in die FontBox ggf. auch die neue Liste f"ullen
    if ( pBox && bChanged )
    {
        if ( *ppFontList )
            pBox->Fill( *ppFontList );
        else
            pBox->Clear();
    }
    return bChanged;
}

//========================================================================
// class SvxFontNameBox_Impl --------------------------------------------------
//========================================================================

SvxFontNameBox_Impl::SvxFontNameBox_Impl( Window* pParent, SfxBindings& rBind, WinBits nStyle ) :

    FontNameBox ( pParent, nStyle | WinBits( WB_DROPDOWN | WB_AUTOHSCROLL ) ),
    aLogicalSize(55,80),
    pFontList   ( NULL ),
    nFtCount    ( 0 ),
    bRelease    ( TRUE ),
    rBindings   ( rBind )

{
    SetSizePixel(LogicToPixel( aLogicalSize, MAP_APPFONT ));
    EnableControls_Impl();
    StartListening( *SFX_APP() );
}

// -----------------------------------------------------------------------

void SvxFontNameBox_Impl::FillList()
{
    // alte Selektion merken, und am Ende wieder setzen
    Selection aOldSel = GetSelection();
    // hat sich Doc-Fontlist geaendert?
    GetDocFontList_Impl( &pFontList, this );
    aCurText = GetText();
    SetSelection( aOldSel );
}

// -----------------------------------------------------------------------

void SvxFontNameBox_Impl::Update( const SvxFontItem* pFontItem )
{
    if ( pFontItem )
    {
        aCurFont.SetName        ( pFontItem->GetFamilyName() );
        aCurFont.SetFamily      ( pFontItem->GetFamily() );
        aCurFont.SetStyleName   ( pFontItem->GetStyleName() );
        aCurFont.SetPitch       ( pFontItem->GetPitch() );
        aCurFont.SetCharSet     ( pFontItem->GetCharSet() );
    }
    String aCurName = aCurFont.GetName();
    if ( GetText() != aCurName )
        SetText( aCurName );
}

// -----------------------------------------------------------------------

long SvxFontNameBox_Impl::PreNotify( NotifyEvent& rNEvt )
{
    USHORT nType = rNEvt.GetType();

    if ( EVENT_MOUSEBUTTONDOWN == nType || EVENT_GETFOCUS == nType )
        FillList();
    return FontNameBox::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long SvxFontNameBox_Impl::Notify( NotifyEvent& rNEvt )
{
    long nHandled = 0;

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        USHORT nCode = rNEvt.GetKeyEvent()->GetKeyCode().GetCode();

        switch ( nCode )
        {
            case KEY_RETURN:
            case KEY_TAB:
            {
                if ( KEY_TAB == nCode )
                    bRelease = FALSE;
                else
                    nHandled = 1;
                Select();
                break;
            }

            case KEY_ESCAPE:
                SetText( aCurText );
                ReleaseFocus_Impl();
                break;
        }
    }
    else if ( EVENT_LOSEFOCUS == rNEvt.GetType() )
    {
        Window* pFocusWin = Application::GetFocusWindow();
        if ( !HasFocus() && GetSubEdit() != pFocusWin )
            SetText( GetSavedValue() );
    }

    return nHandled ? nHandled : FontNameBox::Notify( rNEvt );
}
/* -----------------------------08.03.2002 13:23------------------------------

 ---------------------------------------------------------------------------*/
void SvxFontNameBox_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        SetSizePixel(LogicToPixel(aLogicalSize, MAP_APPFONT));
        Size aDropSize( aLogicalSize.Width(), LOGICAL_EDIT_HEIGHT);
        SetDropDownSizePixel(LogicToPixel(aDropSize, MAP_APPFONT));
    }

    FontNameBox::DataChanged( rDCEvt );
}

// -----------------------------------------------------------------------

void SvxFontNameBox_Impl::ReleaseFocus_Impl()
{
    if ( !bRelease )
    {
        bRelease = TRUE;
        return;
    }
    SfxViewShell* pCurSh = SfxViewShell::Current();

    if ( pCurSh )
    {
        Window* pShellWnd = pCurSh->GetWindow();

        if ( pShellWnd )
            pShellWnd->GrabFocus();
    }
}

// -----------------------------------------------------------------------

void SvxFontNameBox_Impl::EnableControls_Impl()
{
    SvtFontOptions aFontOpt;
    BOOL bEnable = aFontOpt.IsFontHistoryEnabled();
    USHORT nEntries = bEnable ? MAX_MRU_FONTNAME_ENTRIES : 0;
    if ( GetMaxMRUCount() != nEntries )
    {
        // refill in the next GetFocus-Handler
        pFontList = NULL;
        Clear();
        SetMaxMRUCount( nEntries );
    }

    bEnable = aFontOpt.IsFontWYSIWYGEnabled();
    EnableWYSIWYG( bEnable );
    EnableSymbols( bEnable );
}

// -----------------------------------------------------------------------

void SvxFontNameBox_Impl::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    SfxItemSetHint* pHint = PTR_CAST(SfxItemSetHint, &rHint);
    if ( pHint )
        EnableControls_Impl();
}

// -----------------------------------------------------------------------

void SvxFontNameBox_Impl::Select()
{
    FontNameBox::Select();

    if ( !IsTravelSelect() )
    {
        if ( pFontList )
        {
            FontInfo aInfo( pFontList->Get( GetText(),
                                            aCurFont.GetWeight(),
                                            aCurFont.GetItalic() ) );
            aCurFont = aInfo;

            SvxFontItem aFontItem( aInfo.GetFamily(),
                                   aInfo.GetName(),
                                   aInfo.GetStyleName(),
                                   aInfo.GetPitch(),
                                   aInfo.GetCharSet(),
                                   SID_ATTR_CHAR_FONT );

            rBindings.GetDispatcher()->Execute( SID_ATTR_CHAR_FONT, SFX_CALLMODE_RECORD, &aFontItem, 0L );
        }
        ReleaseFocus_Impl();
    }
}

//========================================================================
// class SvxFontSizeBox_Impl --------------------------------------------------
//========================================================================

SvxFontSizeBox_Impl::SvxFontSizeBox_Impl( Window* pParent, SvxFontHeightToolBoxControl &rCtrl, SfxBindings& rBind ) :

    FontSizeBox( pParent, WinBits( WB_DROPDOWN ) ),

    pCtrl       ( &rCtrl ),
    bRelease    ( TRUE ),
    rBindings   ( rBind ),
    aLogicalSize(20,80)

{
    SetSizePixel(LogicToPixel( aLogicalSize, MAP_APPFONT ));
    SetValue( 0 );
    SetText( String() );
}

// -----------------------------------------------------------------------

void SvxFontSizeBox_Impl::ReleaseFocus_Impl()
{
    if ( !bRelease )
    {
        bRelease = TRUE;
        return;
    }

    SfxViewShell* pCurSh = SfxViewShell::Current();

    if ( pCurSh )
    {
        Window* pShellWnd = pCurSh->GetWindow();

        if ( pShellWnd )
            pShellWnd->GrabFocus();
    }
}

// -----------------------------------------------------------------------

void SvxFontSizeBox_Impl::Select()
{
    FontSizeBox::Select();

    if ( !IsTravelSelect() )
    {
        SfxMapUnit eUnit = pCtrl->GetCoreMetric();
        long nSelVal = GetValue();
        long nVal = LogicToLogic( nSelVal, MAP_POINT, (MapUnit)eUnit ) / 10;
        SvxFontHeightItem aFontHeightItem( nVal, 100, SID_ATTR_CHAR_FONTHEIGHT );
        rBindings.GetDispatcher()->Execute(
            SID_ATTR_CHAR_FONTHEIGHT, SFX_CALLMODE_RECORD, &aFontHeightItem, 0L );
        ReleaseFocus_Impl();
    }
}
// -----------------------------------------------------------------------

void SvxFontSizeBox_Impl::StateChanged_Impl( SfxItemState eState, const SfxPoolItem* pState )
{
    if ( SFX_ITEM_AVAILABLE == eState )
    {
        // Metric Umrechnen
        long nH = ( (const SvxFontHeightItem*)pState )->GetHeight() * 10;
        long nVal = LogicToLogic( nH, (MapUnit)pCtrl->GetCoreMetric(), MAP_POINT );

        // ge"andert => neuen Wert setzen
        if ( GetValue() != nVal )
            SetValue( nVal );
    }
    else
    {
        // Wert in der Anzeige l"oschen
        SetValue( -1L );
        SetText( String() );
    }
    SaveValue();
}

// -----------------------------------------------------------------------

void SvxFontSizeBox_Impl::Update( const SvxFontItem& rFontItem )
{
    // Fontliste vom Document abholen
    const SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SvxFontListItem* pFontListItem = (const SvxFontListItem*)
        ( pDocSh ? pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST ) : NULL );

    // Sizes-Liste auff"ullen
    long nOldVal = GetValue(); // alten Wert merken
    FontInfo aFontInfo;
    FASTBOOL bCreate = FALSE;
    const FontList* pFontList = pFontListItem ? pFontListItem->GetFontList() : NULL;
    if ( pFontList )
    {
        aFontInfo = FontInfo( pFontList->Get( rFontItem.GetFamilyName(), rFontItem.GetStyleName() ) );
        Fill( aFontInfo, pFontList );
    }
    SetValue( nOldVal ); // alten Wert wiederherstellen
    aCurText = GetText(); // zum R"ucksetzen bei ESC merken
}

// -----------------------------------------------------------------------

long SvxFontSizeBox_Impl::Notify( NotifyEvent& rNEvt )
{
    long nHandled = 0;

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        USHORT nCode = rNEvt.GetKeyEvent()->GetKeyCode().GetCode();

        switch ( nCode )
        {
            case KEY_RETURN:
            case KEY_TAB:
            {
                if ( KEY_TAB == nCode )
                    bRelease = FALSE;
                else
                    nHandled = 1;
                Select();
                break;
            }

            case KEY_ESCAPE:
                SetText( aCurText );
                ReleaseFocus_Impl();
                nHandled = 1;
                break;
        }
    }
    else if(EVENT_LOSEFOCUS == rNEvt.GetType())
    {
        Window* pFocusWin = Application::GetFocusWindow();
        if(!HasFocus() && GetSubEdit() != pFocusWin)
            SetText(GetSavedValue());
    }

    return nHandled ? nHandled : FontSizeBox::Notify( rNEvt );
}
/* -----------------------------08.03.2002 13:24------------------------------

 ---------------------------------------------------------------------------*/
void SvxFontSizeBox_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        SetSizePixel(LogicToPixel(aLogicalSize, MAP_APPFONT));
        Size aDropSize( aLogicalSize.Width(), LOGICAL_EDIT_HEIGHT);
        SetDropDownSizePixel(LogicToPixel(aDropSize, MAP_APPFONT));
    }

    FontSizeBox::DataChanged( rDCEvt );
}

//========================================================================
// class SvxColorWindow_Impl --------------------------------------------------
//========================================================================
#ifndef WB_NO_DIRECTSELECT
#define WB_NO_DIRECTSELECT      ((WinBits)0x04000000)
#endif

SvxColorWindow_Impl::SvxColorWindow_Impl( USHORT nId, USHORT nSlotId,
                                const String& rWndTitle,
                                SfxBindings& rBindings ) :

    SfxPopupWindow( nId, WinBits( WB_BORDER | WB_STDFLOATWIN | WB_3DLOOK|WB_DIALOGCONTROL ), rBindings ),

    theSlotId( nSlotId ),
    aColorSet( this, WinBits( WB_ITEMBORDER | WB_NAMEFIELD | WB_3DLOOK | WB_NO_DIRECTSELECT) )

{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SfxPoolItem* pItem = NULL;
    XColorTable* pColorTable = NULL;
    const Size aSize12( 12, 12 );

    if ( pDocSh )
        if ( 0 != ( pItem = pDocSh->GetItem( SID_COLOR_TABLE ) ) )
            pColorTable = ( (SvxColorTableItem*)pItem )->GetColorTable();

    if ( SID_ATTR_CHAR_COLOR_BACKGROUND == theSlotId || SID_BACKGROUND_COLOR == theSlotId )
    {
        aColorSet.SetStyle( aColorSet.GetStyle() | WB_NONEFIELD );
        aColorSet.SetText( SVX_RESSTR( RID_SVXSTR_TRANSPARENT ) );
    }
    else if ( SID_ATTR_CHAR_COLOR == theSlotId || SID_ATTR_CHAR_COLOR2 == theSlotId)
    {
        SfxPoolItem* pDummy;
        SfxItemState eState = rBindings.QueryState(SID_ATTR_AUTO_COLOR_INVALID, pDummy);
        if(SFX_ITEM_DEFAULT > eState)
        {
            aColorSet.SetStyle( aColorSet.GetStyle() | WB_NONEFIELD );
            aColorSet.SetText( SVX_RESSTR( RID_SVXSTR_AUTOMATIC ) );
        }
    }

    if ( pColorTable )
    {
        short i = 0;
        long nCount = pColorTable->Count();
        XColorEntry* pEntry = NULL;
        Color aColWhite( COL_WHITE );
        String aStrWhite( SVX_RES(RID_SVXITEMS_COLOR_WHITE) );

        if ( nCount > 80 )
            // bei mehr als 80 Farben ScrollBar anzeigen
            aColorSet.SetStyle( aColorSet.GetStyle() | WB_VSCROLL );

        for ( i = 0; i < nCount; i++ )
        {
            pEntry = pColorTable->Get(i);
            aColorSet.InsertItem( i+1, pEntry->GetColor(), pEntry->GetName() );
        }

        while ( i < 80 )
        {
            // bei weniger als 80 Farben, mit Weiss auff"ullen
            aColorSet.InsertItem( i+1, aColWhite, aStrWhite );
            i++;
        }
    }

    aColorSet.SetSelectHdl( LINK( this, SvxColorWindow_Impl, SelectHdl ) );
    aColorSet.SetColCount( 8 );
    aColorSet.SetLineCount( 10 );

    lcl_CalcSizeValueSet( *this, aColorSet, aSize12 );

    SetHelpId( HID_POPUP_COLOR );
    aColorSet.SetHelpId( HID_POPUP_COLOR_CTRL );

    SetText( rWndTitle );
    aColorSet.Show();
//    aColorSet.GrabFocus();
    StartListening( rBindings );
}

SvxColorWindow_Impl::~SvxColorWindow_Impl()
{
}
void SvxColorWindow_Impl::KeyInput( const KeyEvent& rKEvt )
{
    aColorSet.KeyInput(rKEvt);
}

SfxPopupWindow* SvxColorWindow_Impl::Clone() const
{
    return new SvxColorWindow_Impl( GetId(), theSlotId, GetText(), (SfxBindings&)GetBindings() );
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxColorWindow_Impl, SelectHdl, void *, EMPTYARG )
{
    USHORT nItemId = aColorSet.GetSelectItemId();
    if ( IsInPopupMode() )
        EndPopupMode();

    if ( !nItemId && ( SID_ATTR_CHAR_COLOR_BACKGROUND == theSlotId  || SID_BACKGROUND_COLOR == theSlotId ) )
        GetBindings().Execute( theSlotId );
    else if ( !nItemId && (SID_ATTR_CHAR_COLOR == theSlotId || SID_ATTR_CHAR_COLOR2  == theSlotId) )
    {
        SvxColorItem aColorItem( COL_AUTO, theSlotId );
        GetBindings().GetDispatcher()->Execute( theSlotId, SFX_CALLMODE_RECORD, &aColorItem, 0L );
    }
    else
    {
        SvxColorItem aColorItem( aColorSet.GetItemColor( nItemId ), theSlotId );
        GetBindings().GetDispatcher()->Execute( theSlotId, SFX_CALLMODE_RECORD, &aColorItem, 0L );
    }

    aColorSet.SetNoSelection();
    return 0;
}

// -----------------------------------------------------------------------

void SvxColorWindow_Impl::Resize()
{
    lcl_ResizeValueSet( *this, aColorSet);
}

// -----------------------------------------------------------------------

void SvxColorWindow_Impl::StartSelection()
{
    aColorSet.StartSelection();
}

// -----------------------------------------------------------------------

BOOL SvxColorWindow_Impl::Close()
{
    return SfxPopupWindow::Close();
}

// -----------------------------------------------------------------------

void SvxColorWindow_Impl::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const SfxPoolItemHint* pItemHint = PTR_CAST( SfxPoolItemHint, &rHint );
    SvxColorTableItem* pItem = pItemHint ?
        PTR_CAST( SvxColorTableItem, pItemHint->GetObject() ) : NULL;
    XColorTable* pColorTable = pItem ? pItem->GetColorTable() : NULL;

    if ( pColorTable )
    {
        // Die Liste der Farben (ColorTable) hat sich ge"andert:
        short i = 0;
        long nCount = pColorTable->Count();
        XColorEntry* pEntry = NULL;
        Color aColWhite( COL_WHITE );
        String aStrWhite( SVX_RES( RID_SVXITEMS_COLOR_WHITE ) );

        // ScrollBar an oder aus
        WinBits nBits = aColorSet.GetStyle();
        if ( nCount > 80 )
            nBits &= ~WB_VSCROLL;
        else
            nBits |= WB_VSCROLL;
        aColorSet.SetStyle( nBits );

        for ( i = 0; i < nCount; ++i )
        {
            pEntry = pColorTable->Get(i);
            aColorSet.SetItemColor( i + 1, pEntry->GetColor() );
            aColorSet.SetItemText ( i + 1, pEntry->GetName() );
        }

        while ( i < 80 )
        {
            aColorSet.SetItemColor( i + 1, aColWhite );
            aColorSet.SetItemText ( i + 1, aStrWhite );
            i++;
        }
    }
}

//========================================================================
// class SvxFrameWindow_Impl --------------------------------------------------
//========================================================================

SvxFrameWindow_Impl::SvxFrameWindow_Impl( USHORT nId, SfxBindings& rBindings, BOOL bParagraphMode ) :

    SfxPopupWindow( nId, WinBits( WB_BORDER | WB_STDFLOATWIN | WB_3DLOOK | WB_DIALOGCONTROL ), rBindings ),

    aFrameSet   ( this, WinBits( WB_ITEMBORDER | WB_DOUBLEBORDER | WB_3DLOOK | WB_NO_DIRECTSELECT ) )

{
    aImgList = ImageList( SVX_RES( IsHighContrast()? RID_SVXIL_FRAME_HC : RID_SVXIL_FRAME ) );

    /*
     *  1       2        3         4
     *  -------------------------------------
     *  NONE    LEFT     RIGHT     LEFTRIGHT
     *  TOP     BOTTOM   TOPBOTTOM OUTER
     *  -------------------------------------
     *  HOR     HORINNER VERINNER   ALL         <- kann ueber bParagraphMode
     *                                             abgeschaltet werden
     */

    USHORT i = 0;

    for ( i=1; i<9; i++ )
        aFrameSet.InsertItem( i, aImgList.GetImage(i) );

    if ( !bParagraphMode )
        for ( i = 9; i < 13; i++ )
            aFrameSet.InsertItem( i, aImgList.GetImage(i) );

    aFrameSet.SetColCount( 4 );
    aFrameSet.SetSelectHdl( LINK( this, SvxFrameWindow_Impl, SelectHdl ) );

    lcl_CalcSizeValueSet( *this, aFrameSet,Size( 20, 20 ));

    SetHelpId( HID_POPUP_FRAME );
    SetText( SVX_RESSTR(RID_SVXSTR_FRAME) );
    aFrameSet.Show();
}

SfxPopupWindow* SvxFrameWindow_Impl::Clone() const
{
    //! HACK: wie bekomme ich den Paragraph-Mode ??
    return new SvxFrameWindow_Impl( GetId(), (SfxBindings&)GetBindings(), FALSE );
}

Window* SvxFrameWindow_Impl::GetPreferredKeyInputWindow()
{
    return &aFrameSet;
}

void SvxFrameWindow_Impl::GetFocus()
{
    aFrameSet.GrabFocus();
}

void SvxFrameWindow_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxPopupWindow::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
    {
        aImgList = ImageList( SVX_RES( IsHighContrast()? RID_SVXIL_FRAME_HC : RID_SVXIL_FRAME ) );

        USHORT  nNumOfItems = aFrameSet.GetItemCount();

        for( USHORT i = 1 ; i <= nNumOfItems ; ++i )
            aFrameSet.SetItemImage( i, aImgList.GetImage( i ) );
    }
}
// -----------------------------------------------------------------------

#define FRM_VALID_LEFT      0x01
#define FRM_VALID_RIGHT     0x02
#define FRM_VALID_TOP       0x04
#define FRM_VALID_BOTTOM    0x08
#define FRM_VALID_HINNER    0x10
#define FRM_VALID_VINNER    0x20
#define FRM_VALID_OUTER     0x0f
#define FRM_VALID_ALL       0xff

//
// Per default bleiben ungesetzte Linien unveraendert
// Mit Shift werden ungesetzte Linien zurueckgsetzt
//
IMPL_LINK( SvxFrameWindow_Impl, SelectHdl, void *, EMPTYARG )
{
    Color               aColBlack( COL_BLACK );
    SvxBoxItem          aBorderOuter( SID_ATTR_BORDER_OUTER );
    SvxBoxInfoItem      aBorderInner( SID_ATTR_BORDER_INNER );
    SvxBorderLine       theDefLine,
                        *pLeft = 0,
                        *pRight = 0,
                        *pTop = 0,
                        *pBottom = 0;
    USHORT              nSel = aFrameSet.GetSelectItemId();
    USHORT              nModifier = aFrameSet.GetModifier();
    BYTE                nValidFlags = 0;

    switch ( nSel )
    {
        case 1: nValidFlags |= FRM_VALID_ALL;
        break;  // NONE
        case 2: pLeft = &theDefLine;
                nValidFlags |= FRM_VALID_LEFT;
        break;  // LEFT
        case 3: pRight = &theDefLine;
                nValidFlags |= FRM_VALID_RIGHT;
        break;  // RIGHT
        case 4: pLeft = pRight = &theDefLine;
                nValidFlags |=  FRM_VALID_RIGHT|FRM_VALID_LEFT;
        break;  // LEFTRIGHT
        case 5: pTop = &theDefLine;
                nValidFlags |= FRM_VALID_TOP;
        break;  // TOP
        case 6: pBottom = &theDefLine;
                nValidFlags |= FRM_VALID_BOTTOM;
        break;  // BOTTOM
        case 7: pTop =  pBottom = &theDefLine;
                nValidFlags |= FRM_VALID_BOTTOM|FRM_VALID_TOP;
        break;  // TOPBOTTOM
        case 8: pLeft = pRight = pTop = pBottom = &theDefLine;
                nValidFlags |= FRM_VALID_OUTER;
        break;  // OUTER

        // Tabelle innen:
        case 9: // HOR
            pTop = pBottom = &theDefLine;
            aBorderInner.SetLine( &theDefLine, BOXINFO_LINE_HORI );
            aBorderInner.SetLine( NULL, BOXINFO_LINE_VERT );
            nValidFlags |= FRM_VALID_HINNER|FRM_VALID_TOP|FRM_VALID_BOTTOM;
            break;

        case 10: // HORINNER
            pLeft = pRight = pTop = pBottom = &theDefLine;
            aBorderInner.SetLine( &theDefLine, BOXINFO_LINE_HORI );
            aBorderInner.SetLine( NULL, BOXINFO_LINE_VERT );
            nValidFlags |= FRM_VALID_RIGHT|FRM_VALID_LEFT|FRM_VALID_HINNER|FRM_VALID_TOP|FRM_VALID_BOTTOM;
            break;

        case 11: // VERINNER
            pLeft = pRight = pTop = pBottom = &theDefLine;
            aBorderInner.SetLine( NULL, BOXINFO_LINE_HORI );
            aBorderInner.SetLine( &theDefLine, BOXINFO_LINE_VERT );
            nValidFlags |= FRM_VALID_RIGHT|FRM_VALID_LEFT|FRM_VALID_VINNER|FRM_VALID_TOP|FRM_VALID_BOTTOM;
        break;

        case 12: // ALL
            pLeft = pRight = pTop = pBottom = &theDefLine;
            aBorderInner.SetLine( &theDefLine, BOXINFO_LINE_HORI );
            aBorderInner.SetLine( &theDefLine, BOXINFO_LINE_VERT );
            nValidFlags |= FRM_VALID_ALL;
            break;

        default:
        break;
    }
    aBorderOuter.SetLine( pLeft, BOX_LINE_LEFT );
    aBorderOuter.SetLine( pRight, BOX_LINE_RIGHT );
    aBorderOuter.SetLine( pTop, BOX_LINE_TOP );
    aBorderOuter.SetLine( pBottom, BOX_LINE_BOTTOM );

    if(nModifier == KEY_SHIFT)
        nValidFlags |= FRM_VALID_ALL;
    aBorderInner.SetValid( VALID_TOP,       0 != (nValidFlags&FRM_VALID_TOP ));
    aBorderInner.SetValid( VALID_BOTTOM,    0 != (nValidFlags&FRM_VALID_BOTTOM ));
    aBorderInner.SetValid( VALID_LEFT,      0 != (nValidFlags&FRM_VALID_LEFT));
    aBorderInner.SetValid( VALID_RIGHT,     0 != (nValidFlags&FRM_VALID_RIGHT ));
    aBorderInner.SetValid( VALID_HORI,      0 != (nValidFlags&FRM_VALID_HINNER ));
    aBorderInner.SetValid( VALID_VERT,      0 != (nValidFlags&FRM_VALID_VINNER));
    aBorderInner.SetValid( VALID_DISTANCE, TRUE );
    aBorderInner.SetValid( VALID_DISABLE, FALSE );

    if ( IsInPopupMode() )
        EndPopupMode();

    GetBindings().GetDispatcher()->Execute(
        SID_ATTR_BORDER, SFX_CALLMODE_RECORD, &aBorderOuter, &aBorderInner, 0L );
    aFrameSet.SetNoSelection();
    return 0;
}

// -----------------------------------------------------------------------

void SvxFrameWindow_Impl::Resize()
{
    lcl_ResizeValueSet( *this, aFrameSet);
}

// -----------------------------------------------------------------------

void SvxFrameWindow_Impl::StateChanged(

    USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )

{
    if ( pState )
    {
        const SfxUInt16Item* pItem = PTR_CAST( SfxUInt16Item, pState );

        if ( pItem )
        {
            BOOL bParMode   = (BOOL)pItem->GetValue();
            BOOL bTableMode = ( aFrameSet.GetItemCount() == 12 );
            BOOL bResize    = FALSE;

            if ( bTableMode && bParMode )
            {
                for ( USHORT i = 9; i < 13; i++ )
                    aFrameSet.RemoveItem(i);
                bResize = TRUE;
            }
            else if ( !bTableMode && !bParMode )
            {
                for ( USHORT i = 9; i < 13; i++ )
                    aFrameSet.InsertItem( i, aImgList.GetImage(i) );
                bResize = TRUE;
            }

            if ( bResize )
            {
                lcl_CalcSizeValueSet( *this, aFrameSet,Size( 20, 20 ));
            }
        }
    }
    SfxPopupWindow::StateChanged( nSID, eState, pState );
}

// -----------------------------------------------------------------------

void SvxFrameWindow_Impl::StartSelection()
{
    aFrameSet.StartSelection();
}

// -----------------------------------------------------------------------

BOOL SvxFrameWindow_Impl::Close()
{
    return SfxPopupWindow::Close();
}

//========================================================================
// class SvxLineWindow_Impl --------------------------------------------------
//========================================================================

SvxLineWindow_Impl::SvxLineWindow_Impl( USHORT nId, SfxBindings &rBindings ) :

    SfxPopupWindow( nId, WinBits( WB_BORDER | WB_STDFLOATWIN | WB_3DLOOK | WB_DIALOGCONTROL ), rBindings ),

    aLineSet( this, WinBits( WB_3DLOOK | WB_ITEMBORDER | WB_DOUBLEBORDER | WB_NAMEFIELD | WB_NONEFIELD | WB_NO_DIRECTSELECT ) )
{
    Size    aBmpSize( 55, 12 );
    CreateBitmaps();

    aLineSet.SetColCount( 2 );
    aLineSet.SetSelectHdl( LINK( this, SvxLineWindow_Impl, SelectHdl ) );
    aLineSet.SetText( SVX_RESSTR(STR_NONE) );

    lcl_CalcSizeValueSet( *this, aLineSet, aBmpSize );

    SetHelpId( HID_POPUP_LINE );
    SetText( SVX_RESSTR(RID_SVXSTR_FRAME_STYLE) );
    aLineSet.Show();
}

SfxPopupWindow* SvxLineWindow_Impl::Clone() const
{
    return new SvxLineWindow_Impl( GetId(), (SfxBindings&)GetBindings() );
}

// -----------------------------------------------------------------------

void SvxLineWindow_Impl::MakeLineBitmap( USHORT nNo, Bitmap& rBmp, const Size& rSize, String& rStr,
                                            const Color& rLineCol, const Color& rBackCol )
{
    VirtualDevice   aVirDev( *this );
    Rectangle       aRect( Point(2,0), Size(rSize.Width()-4,0) );

    // grau einfaerben und Bitmap sichern:
    aVirDev.SetOutputSizePixel( rSize );
    aVirDev.SetLineColor();
    aVirDev.SetFillColor( rBackCol );
    aVirDev.DrawRect( Rectangle( Point(0,0), rSize ) );
    aVirDev.SetFillColor( rLineCol );

    sal_uInt16 nLineWidth = 0;
    switch ( nNo )
    {
        case 1: // DEF_LINE_WIDTH_0
            aRect.Top()     = 6;
            aRect.Bottom()  = 6;
            aVirDev.DrawRect( aRect );
            break;

        case 2: // DEF_LINE_WIDTH_1
            aRect.Top()     = 5;
            aRect.Bottom()  = 6;
            aVirDev.DrawRect( aRect );
            nLineWidth = (USHORT) DEF_LINE_WIDTH_1/20;
            break;

        case 3: // DEF_LINE_WIDTH_2
            aRect.Top()     = 5;
            aRect.Bottom()  = 7;
            aVirDev.DrawRect( aRect );
            nLineWidth = (USHORT) DEF_LINE_WIDTH_2/20;
            break;

        case 4: // DEF_LINE_WIDTH_3
            aRect.Top()     = 4;
            aRect.Bottom()  = 7;
            aVirDev.DrawRect( aRect );
            aVirDev.DrawRect( Rectangle( Point(2,4), Point(37,7) ) );
            nLineWidth = (USHORT) DEF_LINE_WIDTH_3/20;
            break;

        case 5: // DEF_LINE_WIDTH_4
            aRect.Top()     = 4;
            aRect.Bottom()  = 8;
            aVirDev.DrawRect( aRect );
            nLineWidth = (USHORT) DEF_LINE_WIDTH_4/20;
            break;

        case 6: // DEF_DOUBLE_LINE0
            aRect.Top()     = 5;
            aRect.Bottom()  = 5;
            aVirDev.DrawRect( aRect );
            aRect.Top()     = 7;
            aRect.Bottom()  = 7;
            aVirDev.DrawRect( aRect );
            nLineWidth = (USHORT) (DEF_DOUBLE_LINE0_OUT+DEF_DOUBLE_LINE0_IN+DEF_DOUBLE_LINE0_DIST)/20;
            break;

        case 7: // DEF_DOUBLE_LINE7
            aRect.Top()     = 4;
            aRect.Bottom()  = 4;
            aVirDev.DrawRect( aRect );
            aRect.Top()     = 7;
            aRect.Bottom()  = 7;
            aVirDev.DrawRect( aRect );
            nLineWidth = (USHORT) (DEF_DOUBLE_LINE7_OUT+DEF_DOUBLE_LINE7_IN+DEF_DOUBLE_LINE7_DIST)/20;
            break;

        case 8: // DEF_DOUBLE_LINE1
            aRect.Top()     = 4;
            aRect.Bottom()  = 5;
            aVirDev.DrawRect( aRect );
            aRect.Top()     = 7;
            aRect.Bottom()  = 8;
            aVirDev.DrawRect( aRect );
            nLineWidth = (USHORT) (DEF_DOUBLE_LINE1_OUT+DEF_DOUBLE_LINE1_IN+DEF_DOUBLE_LINE1_DIST)/20;
            break;

        case 9: // DEF_DOUBLE_LINE2
            aRect.Top()     = 3;
            aRect.Bottom()  = 5;
            aVirDev.DrawRect( aRect );
            aRect.Top()     = 8;
            aRect.Bottom()  = 10;
            aVirDev.DrawRect( aRect );
            nLineWidth = (USHORT) (DEF_DOUBLE_LINE2_OUT+DEF_DOUBLE_LINE2_IN+DEF_DOUBLE_LINE2_DIST)/20;
            break;

        case 10: // DEF_DOUBLE_LINE8
            aRect.Top()     = 3;
            aRect.Bottom()  = 4;
            aVirDev.DrawRect( aRect );
            aRect.Top()     = 7;
            aRect.Bottom()  = 7;
            aVirDev.DrawRect( aRect );
            nLineWidth = (USHORT) (DEF_DOUBLE_LINE8_OUT+DEF_DOUBLE_LINE8_IN+DEF_DOUBLE_LINE8_DIST)/20;
            break;

        case 11: // DEF_DOUBLE_LINE9
            aRect.Top()     = 3;
            aRect.Bottom()  = 5;
            aVirDev.DrawRect( aRect );
            aRect.Top()     = 8;
            aRect.Bottom()  = 8;
            aVirDev.DrawRect( aRect );
            nLineWidth = (USHORT) (DEF_DOUBLE_LINE9_OUT+DEF_DOUBLE_LINE9_IN+DEF_DOUBLE_LINE9_DIST)/20;
            break;

        case 12: // DEF_DOUBLE_LINE10
            aRect.Top()     = 2;
            aRect.Bottom()  = 5;
            aVirDev.DrawRect( aRect );
            aRect.Top()     = 8;
            aRect.Bottom()  = 8;
            aVirDev.DrawRect( aRect );
            nLineWidth = (USHORT) (DEF_DOUBLE_LINE10_OUT+DEF_DOUBLE_LINE10_IN+DEF_DOUBLE_LINE10_DIST)/20;
            break;

        case 13: // DEF_DOUBLE_LINE3
            aRect.Top()     = 4;
            aRect.Bottom()  = 5;
            aVirDev.DrawRect( aRect );
            aRect.Top()     = 7;
            aRect.Bottom()  = 7;
            aVirDev.DrawRect( aRect );
            nLineWidth = (USHORT) (DEF_DOUBLE_LINE3_OUT+DEF_DOUBLE_LINE3_IN+DEF_DOUBLE_LINE3_DIST)/20;
            break;

        case 14: // DEF_DOUBLE_LINE4
            aRect.Top()     = 4;
            aRect.Bottom()  = 4;
            aVirDev.DrawRect( aRect );
            aRect.Top()     = 6;
            aRect.Bottom()  = 7;
            aVirDev.DrawRect( aRect );
            nLineWidth = (USHORT) (DEF_DOUBLE_LINE4_OUT+DEF_DOUBLE_LINE4_IN+DEF_DOUBLE_LINE4_DIST)/20;
            break;

        case 15: // DEF_DOUBLE_LINE5
            aRect.Top()     = 3;
            aRect.Bottom()  = 5;
            aVirDev.DrawRect( aRect );
            aRect.Top()     = 8;
            aRect.Bottom()  = 9;
            aVirDev.DrawRect( aRect );
            nLineWidth = (USHORT) (DEF_DOUBLE_LINE5_OUT+DEF_DOUBLE_LINE5_IN+DEF_DOUBLE_LINE5_DIST)/20;
            break;

        case 16: // DEF_DOUBLE_LINE6
            aRect.Top()     = 3;
            aRect.Bottom()  = 4;
            aVirDev.DrawRect( aRect );
            aRect.Top()     = 7;
            aRect.Bottom()  = 9;
            aVirDev.DrawRect( aRect );
            nLineWidth = (USHORT) (DEF_DOUBLE_LINE6_OUT+DEF_DOUBLE_LINE6_IN+DEF_DOUBLE_LINE6_DIST)/20;
            break;

        default:
            break;
    }
    if ( nLineWidth )
    {
        rStr = String::CreateFromInt32( nLineWidth );
        rStr.AppendAscii(" pt");
    }
    rBmp = aVirDev.GetBitmap( Point(0,0), rSize );
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxLineWindow_Impl, SelectHdl, void *, EMPTYARG )
{
    SvxLineItem     aLineItem;
    USHORT          n1 = 0,
                    n2 = 0,
                    n3 = 0;
    BOOL            bSetLine = TRUE;

    switch ( aLineSet.GetSelectItemId() )
    {
        case  1: n1 = DEF_LINE_WIDTH_0; break;
        case  2: n1 = DEF_LINE_WIDTH_1; break;
        case  3: n1 = DEF_LINE_WIDTH_2; break;
        case  4: n1 = DEF_LINE_WIDTH_3; break;
        case  5: n1 = DEF_LINE_WIDTH_4; break;

        case  6: n1 = DEF_DOUBLE_LINE0_OUT;
                 n2 = DEF_DOUBLE_LINE0_IN;
                 n3 = DEF_DOUBLE_LINE0_DIST;     break;
        case  7: n1 = DEF_DOUBLE_LINE7_OUT;
                 n2 = DEF_DOUBLE_LINE7_IN;
                 n3 = DEF_DOUBLE_LINE7_DIST;     break;
        case  8: n1 = DEF_DOUBLE_LINE1_OUT;
                 n2 = DEF_DOUBLE_LINE1_IN;
                 n3 = DEF_DOUBLE_LINE1_DIST;     break;
        case  9: n1 = DEF_DOUBLE_LINE2_OUT;
                 n2 = DEF_DOUBLE_LINE2_IN;
                 n3 = DEF_DOUBLE_LINE2_DIST;     break;
        case 10: n1 = DEF_DOUBLE_LINE8_OUT;
                 n2 = DEF_DOUBLE_LINE8_IN;
                 n3 = DEF_DOUBLE_LINE8_DIST;     break;
        case 11: n1 = DEF_DOUBLE_LINE9_OUT;
                 n2 = DEF_DOUBLE_LINE9_IN;
                 n3 = DEF_DOUBLE_LINE9_DIST;     break;
        case 12: n1 = DEF_DOUBLE_LINE10_OUT;
                 n2 = DEF_DOUBLE_LINE10_IN;
                 n3 = DEF_DOUBLE_LINE10_DIST; break;
        case 13: n1 = DEF_DOUBLE_LINE3_OUT;
                 n2 = DEF_DOUBLE_LINE3_IN;
                 n3 = DEF_DOUBLE_LINE3_DIST;     break;
        case 14: n1 = DEF_DOUBLE_LINE4_OUT;
                 n2 = DEF_DOUBLE_LINE4_IN;
                 n3 = DEF_DOUBLE_LINE4_DIST;     break;
        case 15: n1 = DEF_DOUBLE_LINE5_OUT;
                 n2 = DEF_DOUBLE_LINE5_IN;
                 n3 = DEF_DOUBLE_LINE5_DIST;     break;
        case 16: n1 = DEF_DOUBLE_LINE6_OUT;
                 n2 = DEF_DOUBLE_LINE6_IN;
                 n3 = DEF_DOUBLE_LINE6_DIST;     break;
        case  0:
        default:
            bSetLine = FALSE;
            break;
    }
    if ( bSetLine )
    {
        SvxBorderLine aTmp( NULL, n1, n2, n3 );
        aLineItem.SetLine( &aTmp );
    }
    else
        aLineItem.SetLine( 0 );

    if ( IsInPopupMode() )
        EndPopupMode();

    GetBindings().GetDispatcher()->Execute( SID_FRAME_LINESTYLE, SFX_CALLMODE_RECORD, &aLineItem, 0L );
    aLineSet.SetNoSelection();
    return 0;
}

// -----------------------------------------------------------------------

void SvxLineWindow_Impl::Resize()
{
    lcl_ResizeValueSet( *this, aLineSet);
}

// -----------------------------------------------------------------------

void SvxLineWindow_Impl::StartSelection()
{
    aLineSet.StartSelection();
}

// -----------------------------------------------------------------------

BOOL SvxLineWindow_Impl::Close()
{
    return SfxPopupWindow::Close();
}

// -----------------------------------------------------------------------

Window* SvxLineWindow_Impl::GetPreferredKeyInputWindow()
{
    return &aLineSet;
}

// -----------------------------------------------------------------------

void SvxLineWindow_Impl::GetFocus()
{
    aLineSet.GrabFocus();
}

void SvxLineWindow_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxPopupWindow::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
    {
        CreateBitmaps();
        Invalidate();
    }
}

void SvxLineWindow_Impl::CreateBitmaps( void )
{
    Size                    aBmpSize( 55, 12 );
    Bitmap                  aBmp;
    String                  aStr;

    const StyleSettings&    rStyleSettings = Application::GetSettings().GetStyleSettings();
    svtools::ColorConfig aColorConfig;
    Color                   aLineCol( aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor );
    Color                   aBackCol( rStyleSettings.GetWindowColor() );
    aLineSet.Clear();

    for( USHORT i = 1 ; i < 17 ; ++i )
    {
        MakeLineBitmap( i, aBmp, aBmpSize, aStr, aLineCol, aBackCol );
        aLineSet.InsertItem( i, aBmp, aStr );
    }
}

// -----------------------------------------------------------------------

#undef BRUSH
#undef COLOR
#undef GET_DOCFONTLIST

//########################################################################
// Hilfsklassen

//========================================================================
// class SvxTbxButtonColorUpdater_Impl ----------------------------------------
//========================================================================

SvxTbxButtonColorUpdater_Impl::SvxTbxButtonColorUpdater_Impl( USHORT nTbxBtnId,
                                                    ToolBox* ptrTbx,
                                                    USHORT nMode ) :
    nDrawMode         ( nMode ),
    nBtnId            ( nTbxBtnId ),
    pTbx              ( ptrTbx ),
    pBtnBmp           ( NULL ),
    aCurColor         ( COL_TRANSPARENT )
{
    DBG_ASSERT( pTbx, "ToolBox not found :-(" );
    bWasHiContrastMode = pTbx ? ( pTbx->GetBackground().GetColor().IsDark() ) : FALSE;
    Update(nTbxBtnId == SID_ATTR_CHAR_COLOR2 ? COL_BLACK : COL_GRAY);
    return;
}

// -----------------------------------------------------------------------

SvxTbxButtonColorUpdater_Impl::~SvxTbxButtonColorUpdater_Impl()
{
    delete pBtnBmp;
}

// -----------------------------------------------------------------------

void SvxTbxButtonColorUpdater_Impl::Update( const Color& rColor )
{
    Image   aImage( pTbx->GetItemImage( nBtnId ) );
    BOOL    bSizeChanged = ( theBmpSize != aImage.GetSizePixel() );
    BOOL    bDisplayModeChanged = ( bWasHiContrastMode != pTbx->GetBackground().GetColor().IsDark() );

    if ( aCurColor == rColor && !bSizeChanged && !bDisplayModeChanged )
        return;

    VirtualDevice aVirDev( *pTbx );
    Point aNullPnt;

    if ( bSizeChanged || bDisplayModeChanged )
    {
        bWasHiContrastMode = pTbx->GetBackground().GetColor().IsDark();
        theBmpSize = aImage.GetSizePixel();

        if ( theBmpSize.Width() <= 16 )
            theUpdRect = Rectangle( Point(7,7), Size(8,8) );
        else
            theUpdRect = Rectangle( Point(14,14), Size(11,11) );

        aVirDev.SetPen( Pen( PEN_NULL ) );
        aVirDev.SetOutputSizePixel( theBmpSize );
        aVirDev.SetFillColor( Color( IMAGE_COL_TRANSPARENT ) );
        aVirDev.DrawRect( Rectangle( aNullPnt, theBmpSize ) );
        aVirDev.DrawImage( aNullPnt, aImage );
        delete pBtnBmp;
        pBtnBmp = new Bitmap( aVirDev.GetBitmap( aNullPnt, theBmpSize ) );
        if ( nDrawMode != TBX_UPDATER_MODE_CHAR_COLOR_NEW )
            aVirDev.DrawRect( theUpdRect );
    }
    else if ( !pBtnBmp )
        pBtnBmp = new Bitmap( aVirDev.GetBitmap( aNullPnt, theBmpSize ) );

    aVirDev.SetOutputSizePixel( theBmpSize );
    aVirDev.DrawBitmap( aNullPnt, *pBtnBmp );

    // Choose line color according to background color
    if ( pTbx->GetBackground().GetColor().IsDark() )
        aVirDev.SetLineColor( COL_WHITE );
    else
        aVirDev.SetLineColor( COL_BLACK );

    if ( nDrawMode == TBX_UPDATER_MODE_CHAR_COLOR_NEW &&
         ( rColor.GetColor() != COL_AUTO &&
           rColor.GetColor() != IMAGE_COL_TRANSPARENT ))
    {
        // Draw border only if COLOR_AUTO is the new color!
        aVirDev.SetLineColor( rColor );
    }
    aVirDev.SetFillColor( rColor );

    if ( nDrawMode == TBX_UPDATER_MODE_CHAR_COLOR_NEW )
    {
        // New mode for our new high contrast enabled bitmaps
        if ( theBmpSize.Width() <= 16 )
            theUpdRect = Rectangle( Point( 0,12 ), Size(theBmpSize.Width(), 4 ) );
        else
            theUpdRect = Rectangle( Point( 1,19 ), Size( 24,6 ) );
        aVirDev.DrawRect( theUpdRect );
    }
    else if ( nDrawMode != TBX_UPDATER_MODE_NONE )
    {
        DrawChar( aVirDev, rColor );
    }
    else
        aVirDev.DrawRect( theUpdRect );

    aCurColor = rColor;

    const Bitmap    aBmp( aVirDev.GetBitmap( aNullPnt, theBmpSize ) );
    static Color    aTransparentColor;
    static sal_Bool bTransparentColorInitialized = sal_False;

    if( !bTransparentColorInitialized )
    {
        aVirDev.DrawPixel( aNullPnt, IMAGE_COL_TRANSPARENT );
        aTransparentColor = aVirDev.GetPixel( aNullPnt );
        bTransparentColorInitialized = sal_True;
    }

    Bitmap  aMaskBitmap = aBmp.CreateMask( aTransparentColor );
    Image aNewImage( aBmp, aMaskBitmap );
    pTbx->SetItemImage( nBtnId, aNewImage );
}

// -----------------------------------------------------------------------

void SvxTbxButtonColorUpdater_Impl::DrawChar( VirtualDevice& rVirDev, const Color& rCol )
{
    Font aOldFont = rVirDev.GetFont();
    Font aFont = aOldFont;
    Size aSz = aFont.GetSize();
    aSz.Height() = theBmpSize.Height();
    aFont.SetSize( aSz );
    aFont.SetWeight( WEIGHT_BOLD );

    if ( nDrawMode == TBX_UPDATER_MODE_CHAR_COLOR )
    {
        aFont.SetColor( rCol );
        aFont.SetFillColor( Color( IMAGE_COL_TRANSPARENT ) );
    }
    else
    {
        rVirDev.SetPen( Pen( PEN_NULL ) );
        rVirDev.SetFillColor( rCol );
        Rectangle aRect( Point(0,0), theBmpSize );
        rVirDev.DrawRect( aRect );
        aFont.SetFillColor( rCol );
    }
    rVirDev.SetFont( aFont );
    Size aTxtSize(rVirDev.GetTextWidth( 'A' ), rVirDev.GetTextHeight());
    Point aPos( ( theBmpSize.Width() - aTxtSize.Width() ) / 2,
                ( theBmpSize.Height() - aTxtSize.Height() ) / 2 );

    rVirDev.DrawText( aPos, 'A' );
    rVirDev.SetFont( aOldFont );
}
//========================================================================
// class SfxStyleControllerItem_Impl ------------------------------------------
//========================================================================

SfxStyleControllerItem_Impl::SfxStyleControllerItem_Impl(
                USHORT                  nId,        // Family-ID
                SfxBindings&            rBindings,  // the Bindings
                SvxStyleToolBoxControl& rTbxCtl )   // Controller-Instanz, dem
                                                    // dieses Item zugeordnet ist.
    :   SfxControllerItem   ( nId, rBindings ),
        rControl            ( rTbxCtl )
{
}

// -----------------------------------------------------------------------

void SfxStyleControllerItem_Impl::StateChanged(

    USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )

{
    switch ( GetId() )
    {
        case SID_STYLE_FAMILY1:
        case SID_STYLE_FAMILY2:
        case SID_STYLE_FAMILY3:
        case SID_STYLE_FAMILY4:
        case SID_STYLE_FAMILY5:
        {
            const USHORT nIdx = GetId() - SID_STYLE_FAMILY_START;

            if ( SFX_ITEM_AVAILABLE == eState )
            {
                const SfxTemplateItem* pStateItem =
                    PTR_CAST( SfxTemplateItem, pState );
                DBG_ASSERT( pStateItem != NULL, "SfxTemplateItem expected" );
                rControl.SetFamilyState( nIdx, pStateItem );
            }
            else
                rControl.SetFamilyState( nIdx, NULL );
            break;
        }
    }
}

//########################################################################

//========================================================================
// class SvxStyleToolBoxControl ------------------------------------------
//========================================================================

SvxStyleToolBoxControl::SvxStyleToolBoxControl( USHORT       nId,
                                                ToolBox&     rTbx,
                                                SfxBindings& rBindings )

    :   SfxToolBoxControl   ( nId, rTbx, rBindings ),
        pStyleSheetPool     ( NULL ),
        nActFamily          ( 0xffff ),
        bListening          ( FALSE )
{
    rBindings.ENTERREGISTRATIONS();

    for ( USHORT i=0; i<MAX_FAMILIES; i++ )
    {
        pBoundItems [i] = new SfxStyleControllerItem_Impl( SID_STYLE_FAMILY_START + i, rBindings, *this );
        pFamilyState[i] = NULL;
    }
    rBindings.LEAVEREGISTRATIONS();

    // Default now: Unbind() controllers until we get our first visibility notification from our UI control.
    for ( i=0; i<MAX_FAMILIES; i++ )
    {
        pBoundItems[i]->UnBind();
        DBG_ASSERT( !pBoundItems[i]->IsBound(), "Invalidate bindings state!" );
    }
    UnBind();
    DBG_ASSERT( !IsBound(), "Invalidate bindings state!" );
}

// -----------------------------------------------------------------------

SvxStyleToolBoxControl::~SvxStyleToolBoxControl()
{
    for( USHORT i=0; i<MAX_FAMILIES; i++ )
    {
        DELETEZ( pBoundItems [i] );
        DELETEZ( pFamilyState[i] );
    }
    pStyleSheetPool = NULL;
}

// -----------------------------------------------------------------------

SfxStyleFamily SvxStyleToolBoxControl::GetActFamily()
{
    switch ( nActFamily-1 + SID_STYLE_FAMILY_START )
    {
        case SID_STYLE_FAMILY1: return SFX_STYLE_FAMILY_CHAR;
        case SID_STYLE_FAMILY2: return SFX_STYLE_FAMILY_PARA;
        case SID_STYLE_FAMILY3: return SFX_STYLE_FAMILY_FRAME;
        case SID_STYLE_FAMILY4: return SFX_STYLE_FAMILY_PAGE;
        case SID_STYLE_FAMILY5: return SFX_STYLE_FAMILY_PSEUDO;
        default:
            DBG_ERROR( "unknown style family" );
            break;
    }
    return SFX_STYLE_FAMILY_PARA;
}

// -----------------------------------------------------------------------

void SvxStyleToolBoxControl::FillStyleBox()
{
    SvxStyleBox_Impl* pBox = (SvxStyleBox_Impl*)GetToolBox().GetItemWindow( GetId() );

    DBG_ASSERT( pStyleSheetPool, "StyleSheetPool not found!" );
    DBG_ASSERT( pBox,            "Control not found!" );

    if ( pStyleSheetPool && pBox && nActFamily!=0xffff )
    {
        const SfxStyleFamily    eFamily     = GetActFamily();
        USHORT                  nCount      = pStyleSheetPool->Count();
        USHORT                  i           = 0;
        SfxStyleSheetBase*      pStyle      = NULL;
        BOOL                    bDoFill     = FALSE;

        pStyleSheetPool->SetSearchMask( eFamily, SFXSTYLEBIT_USED );

        //------------------------------
        // Ueberpruefen, ob Fill noetig:
        //------------------------------

        pStyle = pStyleSheetPool->First();

        if ( nCount != pBox->GetEntryCount() )
        {
            bDoFill = TRUE;
        }
        else
        {
            while ( pStyle && !bDoFill )
            {
                bDoFill = ( pBox->GetEntry(i) != pStyle->GetName() );
                pStyle = pStyleSheetPool->Next();
                i++;
            }
        }

        if ( bDoFill )
        {
            pBox->SetUpdateMode( FALSE );
            pBox->Clear();

            // Listbox nur so gross, wie Eintraege vorhanden,
            // hoechstens MAX_STYLES_ENTRIES Eintraege
            // mindestens 4 Eintraege

            if ( nCount > MAX_STYLES_ENTRIES )
                nCount = MAX_STYLES_ENTRIES;
            else if (nCount < 4)
                nCount = 4;

            nCount += 1; // +1 fuer Selektion

            Size aSize( pBox->GetOutputSizePixel() );
            long nNewHeight = nCount * (14 + 2) ; // 14 == TextHeight

            if ( aSize.Height() != nNewHeight )
            {
                aSize.Height() = nNewHeight;
                pBox->SetOutputSizePixel( aSize );
            }

            pStyle = pStyleSheetPool->First();

            while ( pStyle )
            {
                pBox->InsertEntry( pStyle->GetName() );
                pStyle = pStyleSheetPool->Next();
            }

            pBox->SetUpdateMode( TRUE );
            pBox->SetFamily( eFamily );
        }
    }
}

// -----------------------------------------------------------------------

void SvxStyleToolBoxControl::SelectStyle( const String& rStyleName )
{
    SvxStyleBox_Impl* pBox = (SvxStyleBox_Impl*)GetToolBox().GetItemWindow( GetId() );
    DBG_ASSERT( pBox, "Control not found!" );

    if ( pBox )
    {
        String aStrSel( pBox->GetSelectEntry() );

        if ( rStyleName.Len() > 0 )
        {
            if ( rStyleName != aStrSel )
                pBox->SelectEntry( rStyleName );
        }
        else
            pBox->SetNoSelection();
        pBox->SaveValue();
    }
}

// -----------------------------------------------------------------------

void SvxStyleToolBoxControl::Update()
{
    SfxStyleSheetBasePool*  pPool     = NULL;
    SfxObjectShell*     pDocShell = SfxObjectShell::Current();

    if ( pDocShell )
        pPool = pDocShell->GetStyleSheetPool();

    USHORT i;
    for ( i=0; i<MAX_FAMILIES; i++ )
        if( pFamilyState[i] )
            break;

    if ( i==MAX_FAMILIES || !pPool )
    {
        pStyleSheetPool = pPool;
        return;
    }

    //--------------------------------------------------------------------
    const SfxTemplateItem* pItem = NULL;

    if ( nActFamily == 0xffff || 0 == (pItem = pFamilyState[nActFamily-1]) )
        // aktueller Bereich nicht innerhalb der erlaubten Bereiche
        // oder Default
    {
        pStyleSheetPool = pPool;
        nActFamily      = 2;

        pItem = pFamilyState[nActFamily-1];
        if ( !pItem )
        {
            nActFamily++;
            pItem = pFamilyState[nActFamily-1];
        }

        if ( !pItem )
            DBG_WARNING( "Unknown Family" ); // can happens
    }
    else if ( pPool != pStyleSheetPool )
        pStyleSheetPool = pPool;

    FillStyleBox(); // entscheidet selbst, ob gefuellt werden muss

    if ( pItem )
        SelectStyle( pItem->GetStyleName() );
}

// -----------------------------------------------------------------------

void SvxStyleToolBoxControl::SFX_NOTIFY( SfxBroadcaster& rBC,
    const TypeId& rBCType, const SfxHint&  rHint, const TypeId& rHintType )
{
    if ( rHint.Type() == TYPE(SfxSimpleHint) &&
         ( (SfxSimpleHint&)rHint ).GetId() == SFX_HINT_UPDATEDONE )
    {
        Update();   // Aktualisierung anstossen
        EndListening( GetBindings() );
        bListening = FALSE;
    }
}

// -----------------------------------------------------------------------

void SvxStyleToolBoxControl::SetFamilyState( USHORT nIdx,
                                             const SfxTemplateItem* pItem )
{
    DELETEZ( pFamilyState[nIdx] );

    if ( pItem )
        pFamilyState[nIdx] = new SfxTemplateItem( *pItem );

    if( !bListening )
    {
        StartListening( GetBindings() );
        bListening = TRUE;
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxStyleToolBoxControl, VisibilityNotification, void*, pVoid )
{
    // Call ReBind() && UnBind() according to visibility
    SvxStyleBox_Impl* pBox = (SvxStyleBox_Impl*)( GetToolBox().GetItemWindow( GetId() ));
    if ( pBox->IsVisible() && !IsBound() )
    {
        // Rebind controllers again
        GetBindings().ENTERREGISTRATIONS();

        for ( USHORT i=0; i<MAX_FAMILIES; i++ )
            pBoundItems [i]->ReBind();

        ReBind();
        GetBindings().LEAVEREGISTRATIONS();

        // Invalidate slots
        for ( i=0; i<MAX_FAMILIES; i++ )
            GetBindings().Invalidate( SID_STYLE_FAMILY_START + i );
        GetBindings().Invalidate( GetId() );
    }
    else if ( !pBox->IsVisible() && IsBound() )
    {
        for ( USHORT i=0; i<MAX_FAMILIES; i++ )
            pBoundItems [i]->UnBind();
        UnBind();
    }

    return 0;
}

// -----------------------------------------------------------------------

void SvxStyleToolBoxControl::StateChanged(

    USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )

{
    USHORT       nId    = GetId();
    ToolBox&     rTbx   = GetToolBox();
    SvxStyleBox_Impl* pBox   = (SvxStyleBox_Impl*)(rTbx.GetItemWindow( nId ));
    TriState     eTri   = STATE_NOCHECK;

    DBG_ASSERT( pBox, "Control not found!" );

    if ( SFX_ITEM_DISABLED == eState )
        pBox->Disable();
    else
        pBox->Enable();

    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );

    switch ( eState )
    {
        case SFX_ITEM_AVAILABLE:
            eTri = ((const SfxBoolItem*)pState)->GetValue()
                        ? STATE_CHECK
                        : STATE_NOCHECK;
            break;

        case SFX_ITEM_DONTCARE:
            eTri = STATE_DONTKNOW;
            break;
    }

    rTbx.SetItemState( nId, eTri );
}

// -----------------------------------------------------------------------

Window* SvxStyleToolBoxControl::CreateItemWindow( Window *pParent )
{
    SvxStyleBox_Impl* pBox = new SvxStyleBox_Impl( pParent,
                                         SID_STYLE_APPLY,
                                         SFX_STYLE_FAMILY_PARA, GetBindings() );

    // Set visibility listener to bind/unbind controller
    pBox->SetVisibilityListener( LINK( this, SvxStyleToolBoxControl, VisibilityNotification ));

    return pBox;
}

//========================================================================
// class SvxFontNameToolBoxControl ---------------------------------------
//========================================================================

SvxFontNameToolBoxControl::SvxFontNameToolBoxControl(
                                            USHORT          nId,
                                            ToolBox&        rTbx ,
                                            SfxBindings&    rBindings)

    :   SfxToolBoxControl( nId, rTbx, rBindings )
{
}

// -----------------------------------------------------------------------

void SvxFontNameToolBoxControl::StateChanged(

    USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )

{
    USHORT          nId = GetId();
    ToolBox&        rTbx   = GetToolBox();
    SvxFontNameBox_Impl* pBox   = (SvxFontNameBox_Impl*)(rTbx.GetItemWindow( nId ));
    TriState        eTri   = STATE_NOCHECK;

    DBG_ASSERT( pBox, "Control not found!" );

    if ( SFX_ITEM_DISABLED == eState )
    {
        pBox->Disable();
        pBox->Update( (const SvxFontItem*)NULL );
    }
    else
    {
        pBox->Enable();

        if ( SFX_ITEM_AVAILABLE == eState )
        {
            DBG_ASSERT( pState->ISA(SvxFontItem), "falscher ItemType" );
            pBox->Update( (const SvxFontItem*) pState );
        }
        else
            pBox->SetText( String() );
        pBox->SaveValue();
    }

    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );

    switch ( eState )
    {
        case SFX_ITEM_AVAILABLE:
            eTri = ((const SfxBoolItem*)pState)->GetValue()
                        ? STATE_CHECK
                        : STATE_NOCHECK;
            break;

        case SFX_ITEM_DONTCARE:
            eTri = STATE_DONTKNOW;
            break;
    }

    rTbx.SetItemState( nId, eTri );
}

// -----------------------------------------------------------------------

Window* SvxFontNameToolBoxControl::CreateItemWindow( Window *pParent )
{
    SvxFontNameBox_Impl* pBox = new SvxFontNameBox_Impl( pParent, GetBindings() );
    return pBox;
}

//========================================================================
// class SvxFontHeightToolBoxControl -------------------------------------
//========================================================================

SvxFontHeightToolBoxControl::SvxFontHeightToolBoxControl( USHORT nId, ToolBox& rTbx,
                                                          SfxBindings& rBindings ) :

    SfxToolBoxControl( nId, rTbx, rBindings ),

    aFontNameForwarder  ( SID_ATTR_CHAR_FONT, *this ),
    pBox                ( NULL ),
    pFontItem           ( NULL )

{
}

// -----------------------------------------------------------------------
SvxFontHeightToolBoxControl::~SvxFontHeightToolBoxControl()
{
    delete pFontItem;
}

// -----------------------------------------------------------------------

void SvxFontHeightToolBoxControl::StateChanged(

    USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )

{
    // FontHeight?
    if ( SID_ATTR_CHAR_FONTHEIGHT == nSID )
    {
        SfxToolBoxControl::StateChanged( nSID, eState, pState );
        pBox->StateChanged_Impl( eState, pState );
    }
    else
    {
        // FontItem (Name) nur zur sp"aterne Verwendung merken
        delete pFontItem;
        pFontItem = (eState == SFX_ITEM_AVAILABLE) ? (SvxFontItem*)pState->Clone() : NULL;

        if ( pFontItem )
            pBox->Update( *pFontItem );
    }
}

// -----------------------------------------------------------------------

Window* SvxFontHeightToolBoxControl::CreateItemWindow( Window *pParent )
{
    pBox = new SvxFontSizeBox_Impl( pParent, *this, GetBindings() );
    return pBox;
}

//========================================================================
// class SvxFontColorToolBoxControl --------------------------------------
//========================================================================

SvxFontColorToolBoxControl::SvxFontColorToolBoxControl(
                                        USHORT          nId,
                                        ToolBox&        rTbx ,
                                        SfxBindings&    rBindings )

    :   SfxToolBoxControl   ( nId, rTbx, rBindings ),
    pBtnUpdater(new SvxTbxButtonColorUpdater_Impl(nId, &GetToolBox(), TBX_UPDATER_MODE_CHAR_COLOR_NEW ))
{
}

// -----------------------------------------------------------------------

SvxFontColorToolBoxControl::~SvxFontColorToolBoxControl()
{
    delete pBtnUpdater;
}

// -----------------------------------------------------------------------

SfxPopupWindowType SvxFontColorToolBoxControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK;
}

// -----------------------------------------------------------------------

SfxPopupWindow* SvxFontColorToolBoxControl::CreatePopupWindow()
{
    SvxColorWindow_Impl* pColorWin =
        new SvxColorWindow_Impl( GetId(), SID_ATTR_CHAR_COLOR,
                            SVX_RESSTR( RID_SVXITEMS_EXTRAS_CHARCOLOR ),
                            GetBindings() );

    pColorWin->StartPopupMode( &GetToolBox(),
        FLOATWIN_POPUPMODE_GRABFOCUS|FLOATWIN_POPUPMODE_ALLOWTEAROFF );
    pColorWin->StartSelection();
    return pColorWin;
}

// -----------------------------------------------------------------------

void SvxFontColorToolBoxControl::StateChanged(

    USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )

{
    USHORT nId = GetId();
    ToolBox& rTbx = GetToolBox();
    const SvxColorItem* pItem = 0;

    if ( SFX_ITEM_DONTCARE != eState )
       pItem = PTR_CAST( SvxColorItem, pState );

    if ( pItem )
        pBtnUpdater->Update( pItem->GetValue());

    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
    rTbx.SetItemState( nId, ( SFX_ITEM_DONTCARE == eState ) ? STATE_DONTKNOW : STATE_NOCHECK );
}

//========================================================================
// class SvxColorToolBoxControl --------------------------------
//========================================================================

SvxColorToolBoxControl::SvxColorToolBoxControl( USHORT  nId, ToolBox& rTbx, SfxBindings& rBindings ) :

    SfxToolBoxControl   ( nId, rTbx, rBindings )
{
    pBtnUpdater = new SvxTbxButtonColorUpdater_Impl( nId, &GetToolBox() );
}

// -----------------------------------------------------------------------

SvxColorToolBoxControl::~SvxColorToolBoxControl()
{
    delete pBtnUpdater;
}

// -----------------------------------------------------------------------

SfxPopupWindowType SvxColorToolBoxControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK;
}

// -----------------------------------------------------------------------

SfxPopupWindow* SvxColorToolBoxControl::CreatePopupWindow()
{
    USHORT nResId = GetId() == SID_BACKGROUND_COLOR ?
                        RID_SVXSTR_BACKGROUND : RID_SVXSTR_COLOR;
    SvxColorWindow_Impl* pColorWin = new SvxColorWindow_Impl( GetId(),
                                    SID_BACKGROUND_COLOR,
                                    SVX_RESSTR(nResId),
                                    GetBindings() );

    pColorWin->StartPopupMode( &GetToolBox(),
        FLOATWIN_POPUPMODE_GRABFOCUS|FLOATWIN_POPUPMODE_ALLOWTEAROFF );
    pColorWin->StartSelection();
    return pColorWin;
}

// -----------------------------------------------------------------------

void SvxColorToolBoxControl::StateChanged(

    USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )

{
    const SvxColorItem* pItem   = 0;
    if ( SFX_ITEM_DONTCARE != eState )
        pItem = PTR_CAST( SvxColorItem, pState );

    if ( pItem )
        pBtnUpdater->Update( pItem->GetValue() );

    USHORT nId = GetId();
    ToolBox& rTbx = GetToolBox();
    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
    rTbx.SetItemState( nId, ( SFX_ITEM_DONTCARE == eState ) ? STATE_DONTKNOW : STATE_NOCHECK );
}

//========================================================================
// class SvxFontColorExtToolBoxControl --------------------------------------
//========================================================================

SvxFontColorExtToolBoxControl::SvxFontColorExtToolBoxControl
(
    USHORT nId,
    ToolBox& rTbx,
    SfxBindings& rBindings
) :

    SfxToolBoxControl( nId, rTbx, rBindings ),
    pBtnUpdater(0),
    aForward( SID_ATTR_CHAR_COLOR2 == nId
                ? SID_ATTR_CHAR_COLOR_EXT
                : SID_ATTR_CHAR_COLOR_BACKGROUND_EXT, *this )

{
    USHORT nMode =  SID_ATTR_CHAR_COLOR2 == nId
        ? TBX_UPDATER_MODE_CHAR_COLOR_NEW : TBX_UPDATER_MODE_CHAR_COLOR_NEW;
    pBtnUpdater = new SvxTbxButtonColorUpdater_Impl( nId, &GetToolBox(), nMode );
}

// -----------------------------------------------------------------------

SvxFontColorExtToolBoxControl::~SvxFontColorExtToolBoxControl()
{
    delete pBtnUpdater;
}

// -----------------------------------------------------------------------

SfxPopupWindowType SvxFontColorExtToolBoxControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONTIMEOUT;
}

// -----------------------------------------------------------------------

SfxPopupWindow* SvxFontColorExtToolBoxControl::CreatePopupWindow()
{
    SvxColorWindow_Impl* pColorWin =
        new SvxColorWindow_Impl( GetId(), GetId(),
                            SVX_RESSTR( RID_SVXITEMS_EXTRAS_CHARCOLOR ),
                            GetBindings() );

    if ( GetId() == SID_ATTR_CHAR_COLOR_BACKGROUND )
        pColorWin->SetText( SVX_RESSTR( RID_SVXSTR_EXTRAS_CHARBACKGROUND ) );

    pColorWin->StartPopupMode( &GetToolBox(),
        FLOATWIN_POPUPMODE_GRABFOCUS|FLOATWIN_POPUPMODE_ALLOWTEAROFF );
    pColorWin->StartSelection();
    return pColorWin;
}

// -----------------------------------------------------------------------

void SvxFontColorExtToolBoxControl::StateChanged(

    USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )

{
    USHORT nId = GetId();
    ToolBox& rTbx = GetToolBox();
    const SvxColorItem* pItem = 0;

    if ( nSID == SID_ATTR_CHAR_COLOR_EXT ||
         nSID == SID_ATTR_CHAR_COLOR_BACKGROUND_EXT )
    {
        if ( SFX_ITEM_DONTCARE != eState )
        {
            const SfxBoolItem* pBool = PTR_CAST( SfxBoolItem, pState );
            rTbx.CheckItem( nId, pBool && pBool->GetValue());
        }
        rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
    }
    else
    {
        if ( SFX_ITEM_DONTCARE != eState )
           pItem = PTR_CAST( SvxColorItem, pState );

        if ( pItem )
            pBtnUpdater->Update( pItem->GetValue() );
    }
}

// -----------------------------------------------------------------------

void SvxFontColorExtToolBoxControl::Select( BOOL bMod1 )
{
    USHORT nId = ( SID_ATTR_CHAR_COLOR2 == GetId() ) ? SID_ATTR_CHAR_COLOR_EXT
                                                     : SID_ATTR_CHAR_COLOR_BACKGROUND_EXT;
    SfxBoolItem aItem( nId, GetToolBox().IsItemChecked( GetId() ) );
    GetBindings().GetDispatcher()->Execute( nId, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );
}

//========================================================================
// class SvxFrameToolBoxControl ------------------------------------------
//========================================================================

SvxFrameToolBoxControl::SvxFrameToolBoxControl( USHORT       nId,
                                                ToolBox&     rTbx,
                                                SfxBindings& rBindings )

    :   SfxToolBoxControl( nId, rTbx, rBindings ),
        bParagraphMode   ( FALSE )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
}

// -----------------------------------------------------------------------

SfxPopupWindowType SvxFrameToolBoxControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK;
}

// -----------------------------------------------------------------------

SfxPopupWindow* SvxFrameToolBoxControl::CreatePopupWindow()
{
    SvxFrameWindow_Impl* pFrameWin = new SvxFrameWindow_Impl( GetId(),
                                                    GetBindings(),
                                                    bParagraphMode );

    pFrameWin->StartPopupMode( &GetToolBox(), FLOATWIN_POPUPMODE_GRABFOCUS | FLOATWIN_POPUPMODE_ALLOWTEAROFF );
    pFrameWin->StartSelection();

    return pFrameWin;
}

// -----------------------------------------------------------------------

void SvxFrameToolBoxControl::StateChanged(

    USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )

{
    USHORT                  nId     = GetId();
    ToolBox&                rTbx    = GetToolBox();
    const SfxUInt16Item*    pItem   = 0;

    if ( SFX_ITEM_DONTCARE != eState )
        pItem = PTR_CAST( SfxUInt16Item, pState );

    if ( pItem )
        bParagraphMode = (BOOL)pItem->GetValue();

    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
    rTbx.SetItemState( nId, (SFX_ITEM_DONTCARE == eState)
                                ? STATE_DONTKNOW
                                : STATE_NOCHECK );
}

//========================================================================
// class SvxFrameLineStyleToolBoxControl ---------------------------------
//========================================================================

SvxFrameLineStyleToolBoxControl::SvxFrameLineStyleToolBoxControl(
                                            USHORT          nId,
                                            ToolBox&        rTbx,
                                            SfxBindings&    rBindings )

    :    SfxToolBoxControl( nId, rTbx, rBindings )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
}

// -----------------------------------------------------------------------

SfxPopupWindowType SvxFrameLineStyleToolBoxControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK;
}

// -----------------------------------------------------------------------

SfxPopupWindow* SvxFrameLineStyleToolBoxControl::CreatePopupWindow()
{
    SvxLineWindow_Impl* pLineWin = new SvxLineWindow_Impl( GetId(), GetBindings() );

    pLineWin->StartPopupMode( &GetToolBox(), TRUE );
    pLineWin->StartSelection();

    return pLineWin;
}

// -----------------------------------------------------------------------

void SvxFrameLineStyleToolBoxControl::StateChanged(

    USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )

{
    USHORT       nId    = GetId();
    ToolBox&     rTbx   = GetToolBox();

    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
    rTbx.SetItemState( nId, (SFX_ITEM_DONTCARE == eState)
                                ? STATE_DONTKNOW
                                : STATE_NOCHECK );
}

//========================================================================
// class SvxFrameLineColorToolBoxControl ---------------------------------
//========================================================================

SvxFrameLineColorToolBoxControl::SvxFrameLineColorToolBoxControl
(
    USHORT nId,
    ToolBox& rTbx,
    SfxBindings& rBindings
) :

    SfxToolBoxControl( nId, rTbx, rBindings ),
    pBtnUpdater(new SvxTbxButtonColorUpdater_Impl(   nId, &GetToolBox() ))
{
}

// -----------------------------------------------------------------------

SvxFrameLineColorToolBoxControl::~SvxFrameLineColorToolBoxControl()
{

    delete pBtnUpdater;
}

// -----------------------------------------------------------------------

SfxPopupWindowType SvxFrameLineColorToolBoxControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK;
}

// -----------------------------------------------------------------------

SfxPopupWindow* SvxFrameLineColorToolBoxControl::CreatePopupWindow()
{
    SvxColorWindow_Impl* pColorWin = new SvxColorWindow_Impl( GetId(),
                                    SID_FRAME_LINECOLOR,
                                    SVX_RESSTR(RID_SVXSTR_FRAME_COLOR),
                                    GetBindings() );

    pColorWin->StartPopupMode( &GetToolBox(),
        FLOATWIN_POPUPMODE_GRABFOCUS|FLOATWIN_POPUPMODE_ALLOWTEAROFF );
    pColorWin->StartSelection();
    return pColorWin;
}

// -----------------------------------------------------------------------

void SvxFrameLineColorToolBoxControl::StateChanged(

    USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )

{
    USHORT nId = GetId();
    ToolBox& rTbx = GetToolBox();
    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
    rTbx.SetItemState( nId, ( SFX_ITEM_DONTCARE == eState ) ? STATE_DONTKNOW : STATE_NOCHECK );

    const SvxColorItem* pItem = 0;
    if ( SFX_ITEM_DONTCARE != eState )
    {
       pItem = PTR_CAST( SvxColorItem, pState );
        if ( pItem )
            pBtnUpdater->Update( pItem->GetValue());
    }
}

// class SvxReloadControllerItem_Impl ------------------------------------

class SvxReloadControllerItem_Impl
{
public:
    Image* pNormalImage;
    Image* pSpecialImage;

    SvxReloadControllerItem_Impl() :
        pSpecialImage( 0 ), pNormalImage( new Image( SVX_RES( RID_SVX_RELOAD_NORMAL ) ) ) {}
    ~SvxReloadControllerItem_Impl() { delete pNormalImage; delete pSpecialImage; }

    Image& GetNormalImage() { return *pNormalImage; }
    Image& GetSpecialImage()
        {
            if ( !pSpecialImage )
                pSpecialImage = new Image( SVX_RES( RID_SVX_RELOAD_SPECIAL ) );
            return *pSpecialImage;
        }
};

// -----------------------------------------------------------------------

SvxReloadControllerItem::SvxReloadControllerItem(
    USHORT nId, ToolBox& rTbx, SfxBindings& rBindings )
    : pImpl( new SvxReloadControllerItem_Impl ),
      SfxToolBoxControl( nId, rTbx, rBindings )
{
    rTbx.SetItemImage( nId, pImpl->GetNormalImage() );
}

// -----------------------------------------------------------------------

SvxReloadControllerItem::~SvxReloadControllerItem()
{
    delete pImpl;
}

// -----------------------------------------------------------------------

void SvxReloadControllerItem::StateChanged(
    USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    SfxBoolItem* pItem = PTR_CAST( SfxBoolItem, pState );
    ToolBox& rBox = GetToolBox();
    if( pItem )
    {
        rBox.SetItemImage( GetId(),
                pItem->GetValue() ? pImpl->GetSpecialImage() :
                pImpl->GetNormalImage() );
    }
    rBox.EnableItem( GetId(), eState != SFX_ITEM_DISABLED );
//  SfxToolBoxControl::StateChanged( nSID, eState, pState );
}

//========================================================================

void lcl_ResizeValueSet( Window &rWin, ValueSet &rValueSet )
{
    Size aSize = rWin.GetOutputSizePixel();
    aSize.Width()  -= 4;
    aSize.Height() -= 4;
    rValueSet.SetPosSizePixel( Point(2,2), aSize );
}

// -----------------------------------------------------------------------

void lcl_CalcSizeValueSet( Window &rWin, ValueSet &rValueSet, const Size &aItemSize ) \
{
    Size aSize = rValueSet.CalcWindowSizePixel( aItemSize );
    aSize.Width()  += 4;
    aSize.Height() += 4;
    rWin.SetOutputSizePixel( aSize );
}

// -----------------------------------------------------------------------

BOOL lcl_FontChangedHint( const SfxHint &rHint )
{
    SfxPoolItemHint *pItemHint = PTR_CAST(SfxPoolItemHint, &rHint);
    if ( pItemHint )
    {
        SfxPoolItem *pItem = pItemHint->GetObject();
        return ( pItem->Which() == SID_ATTR_CHAR_FONTLIST );
    }
    else
    {
        SfxSimpleHint* pSimpleHint = PTR_CAST(SfxSimpleHint, &rHint);
        return pSimpleHint && ( SFX_HINT_DATACHANGED ==
                            ( pSimpleHint->GetId() & SFX_HINT_DATACHANGED ) );
    }
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > SvxFontNameBox_Impl::CreateAccessible()
{
    FillList();
    return FontNameBox::CreateAccessible();
}



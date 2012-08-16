/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <string>
#include <tools/shl.hxx>
#include <tools/color.hxx>
#include <svl/poolitem.hxx>
#include <svl/eitem.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/bmpacc.hxx>
#include <svtools/valueset.hxx>
#include <svtools/ctrlbox.hxx>
#include <svl/style.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/borderhelper.hxx>
#include <svl/stritem.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/tplpitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/templdlg.hxx>
#include <svl/isethint.hxx>
#include <sfx2/querystatus.hxx>
#include <sfx2/sfxstatuslistener.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/viewfrm.hxx>
#include <unotools/fontoptions.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/svapp.hxx>
#include <svl/smplhint.hxx>
#include <svtools/colorcfg.hxx>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/status/ItemStatus.hpp>
#include <svx/dialogs.hrc>
#include <svx/svxitems.hrc>
#include "helpid.hrc"
#include "svx/htmlmode.hxx"
#include <svx/xtable.hxx>
#include "editeng/fontitem.hxx"
#include <editeng/fhgtitem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/colritem.hxx>
#include "editeng/flstitem.hxx"
#include "editeng/bolnitem.hxx"
#include "svx/drawitem.hxx"
#include <svx/tbcontrl.hxx>
#include "svx/dlgutil.hxx"
#include <svx/dialmgr.hxx>
#include "colorwindow.hxx"
#include <memory>

#include <svx/framelink.hxx>
#include <svx/tbxcolorupdate.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/editrids.hrc>

// ------------------------------------------------------------------------

#define MAX_MRU_FONTNAME_ENTRIES    5
#define LOGICAL_EDIT_HEIGHT         12

// STATIC DATA -----------------------------------------------------------

#ifndef DELETEZ
#define DELETEZ(p) (delete (p), (p)=NULL)
#endif
// don't make more than 15 entries visible at once
#define MAX_STYLES_ENTRIES          static_cast< sal_uInt16 >( 15 )

void lcl_ResizeValueSet( Window &rWin, ValueSet &rValueSet );
void lcl_CalcSizeValueSet( Window &rWin, ValueSet &rValueSet, const Size &aItemSize );

// namespaces
using ::rtl::OUString;
using namespace ::editeng;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

SFX_IMPL_TOOLBOX_CONTROL( SvxStyleToolBoxControl, SfxTemplateItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxFontNameToolBoxControl, SvxFontItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxFontColorToolBoxControl, SvxColorItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxColorExtToolBoxControl, SvxColorItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxFrameToolBoxControl, SvxBoxItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxFrameLineStyleToolBoxControl, SvxLineItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxFrameLineColorToolBoxControl, SvxColorItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxSimpleUndoRedoController, SfxStringItem );

//========================================================================
// class SvxStyleBox_Impl -----------------------------------------------------
//========================================================================


class SvxStyleBox_Impl : public ComboBox
{
    using Window::IsVisible;
public:
    SvxStyleBox_Impl( Window* pParent, const OUString& rCommand, SfxStyleFamily eFamily, const Reference< XDispatchProvider >& rDispatchProvider,
                        const Reference< XFrame >& _xFrame,const String& rClearFormatKey, const String& rMoreKey, sal_Bool bInSpecialMode );
    ~SvxStyleBox_Impl();

    void            SetFamily( SfxStyleFamily eNewFamily );
    inline sal_Bool     IsVisible() { return bVisible; }

    virtual long    PreNotify( NotifyEvent& rNEvt );
    virtual long    Notify( NotifyEvent& rNEvt );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual void    StateChanged( StateChangedType nStateChange );

    inline void     SetVisibilityListener( const Link& aVisListener ) { aVisibilityListener = aVisListener; }
    inline void     RemoveVisibilityListener() { aVisibilityListener = Link(); }

    void            SetDefaultStyle( const ::rtl::OUString& rDefault ) { sDefaultStyle = rDefault; }
    DECL_STATIC_LINK( SvxStyleBox_Impl, FocusHdl_Impl, Control* );

protected:
    virtual void    Select();

private:
    SfxStyleFamily                  eStyleFamily;
    sal_uInt16                          nCurSel;
    sal_Bool                            bRelease;
    Size                            aLogicalSize;
    Link                            aVisibilityListener;
    sal_Bool                            bVisible;
    Reference< XDispatchProvider >  m_xDispatchProvider;
    Reference< XFrame >             m_xFrame;
    OUString                        m_aCommand;
    String                          aClearFormatKey;
    String                          aMoreKey;
    String                          sDefaultStyle;
    sal_Bool                            bInSpecialMode;

    void            ReleaseFocus();
};

//========================================================================
// class SvxFontNameBox --------------------------------------------------
//========================================================================

class SvxFontNameBox_Impl : public FontNameBox
{
    using Window::Update;
private:
    const FontList*                pFontList;
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<FontList>      m_aOwnFontList;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    Font                           aCurFont;
    Size                           aLogicalSize;
    String                         aCurText;
    sal_uInt16                         nFtCount;
    sal_Bool                           bRelease;
    Reference< XDispatchProvider > m_xDispatchProvider;
    Reference< XFrame >            m_xFrame;

    void            ReleaseFocus_Impl();
    void            EnableControls_Impl();

protected:
    virtual void    Select();
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

public:
    SvxFontNameBox_Impl( Window* pParent, const Reference< XDispatchProvider >& rDispatchProvider,const Reference< XFrame >& _xFrame
        , WinBits nStyle = WB_SORT
        );

    void            FillList();
    void            Update( const SvxFontItem* pFontItem );
    sal_uInt16          GetListCount() { return nFtCount; }
    void            Clear() { FontNameBox::Clear(); nFtCount = 0; }
    void            Fill( const FontList* pList )
                        { FontNameBox::Fill( pList );
                          nFtCount = pList->GetFontNameCount(); }
    virtual long    PreNotify( NotifyEvent& rNEvt );
    virtual long    Notify( NotifyEvent& rNEvt );
    virtual Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    void     SetOwnFontList(::std::auto_ptr<FontList> _aOwnFontList) { m_aOwnFontList = _aOwnFontList; }
    SAL_WNODEPRECATED_DECLARATIONS_POP
};

//========================================================================
// class SvxFrameWindow_Impl --------------------------------------------------
//========================================================================

// fuer den SelectHdl werden die Modifier gebraucht, also
// muss man sie im MouseButtonUp besorgen

class SvxFrmValueSet_Impl : public ValueSet
{
    sal_uInt16          nModifier;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
public:
    SvxFrmValueSet_Impl(Window* pParent,  WinBits nWinStyle)
        : ValueSet(pParent, nWinStyle), nModifier(0) {}
    sal_uInt16          GetModifier() const {return nModifier;}

};

void SvxFrmValueSet_Impl::MouseButtonUp( const MouseEvent& rMEvt )
{
    nModifier = rMEvt.GetModifier();
    ValueSet::MouseButtonUp(rMEvt);
}

class SvxFrameWindow_Impl : public SfxPopupWindow
{
    using FloatingWindow::StateChanged;

private:
    SvxFrmValueSet_Impl  aFrameSet;
    ImageList       aImgList;
    sal_Bool        bParagraphMode;

#if _SOLAR__PRIVATE
    DECL_LINK( SelectHdl, void * );
#endif

protected:
    virtual void    Resize();
    virtual sal_Bool    Close();
    virtual Window* GetPreferredKeyInputWindow();
    virtual void    GetFocus();

public:
    SvxFrameWindow_Impl( sal_uInt16 nId, const Reference< XFrame >& rFrame, Window* pParentWindow );
    ~SvxFrameWindow_Impl();
    void            StartSelection();

    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                  const SfxPoolItem* pState );
    virtual SfxPopupWindow* Clone() const;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
};

//========================================================================
// class SvxLineWindow_Impl ---------------------------------------------------
//========================================================================
class SvxLineWindow_Impl : public SfxPopupWindow
{
private:
    LineListBox         m_aLineStyleLb;
    bool                m_bIsWriter;

#if _SOLAR__PRIVATE
    DECL_LINK( SelectHdl, void * );
#endif

protected:
    virtual void    Resize();
    virtual sal_Bool    Close();
    virtual Window* GetPreferredKeyInputWindow();
    virtual void    GetFocus();
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
public:
    SvxLineWindow_Impl( sal_uInt16 nId, const Reference< XFrame >& rFrame, Window* pParentWindow );

    virtual SfxPopupWindow* Clone() const;
};

//########################################################################
// Hilfsklassen:
//========================================================================
// class SfxStyleControllerItem ------------------------------------------
//========================================================================
class SvxStyleToolBoxControl;

class SfxStyleControllerItem_Impl : public SfxStatusListener
{
    public:
        SfxStyleControllerItem_Impl( const Reference< XDispatchProvider >& rDispatchProvider,
                                     sal_uInt16 nSlotId,
                                     const rtl::OUString& rCommand,
                                     SvxStyleToolBoxControl& rTbxCtl );

    protected:
        virtual void StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );

    private:
        SvxStyleToolBoxControl& rControl;
};

//========================================================================
// class SvxStyleBox_Impl -----------------------------------------------------
//========================================================================

SvxStyleBox_Impl::SvxStyleBox_Impl(
    Window*                                 pParent,
    const rtl::OUString&                    rCommand,
    SfxStyleFamily                          eFamily,
    const Reference< XDispatchProvider >&   rDispatchProvider,
    const Reference< XFrame >&              _xFrame,
    const String&                           rClearFormatKey,
    const String&                           rMoreKey,
    sal_Bool                                    bInSpec) :

    ComboBox( pParent, SVX_RES( RID_SVXTBX_STYLE ) ),

    eStyleFamily( eFamily ),
    bRelease    ( sal_True ),
    bVisible(sal_False),
    m_xDispatchProvider( rDispatchProvider ),
    m_xFrame(_xFrame),
    m_aCommand  ( rCommand ),
    aClearFormatKey ( rClearFormatKey ),
    aMoreKey        ( rMoreKey ),
    bInSpecialMode  ( bInSpec )
{
    aLogicalSize = PixelToLogic( GetSizePixel(), MAP_APPFONT );
    EnableAutocomplete( sal_True );
}

SvxStyleBox_Impl::~SvxStyleBox_Impl()
{
}

// -----------------------------------------------------------------------

void SvxStyleBox_Impl::ReleaseFocus()
{
    if ( !bRelease )
    {
        bRelease = sal_True;
        return;
    }
    if ( m_xFrame.is() && m_xFrame->getContainerWindow().is() )
        m_xFrame->getContainerWindow()->setFocus();
}

// -----------------------------------------------------------------------

void SvxStyleBox_Impl::Select()
{
    // Tell base class about selection so that AT get informed about it.
    ComboBox::Select();

    if ( !IsTravelSelect() )
    {
        String aSelEntry( GetText() );
        bool bDoIt = true, bClear = false;
        if( bInSpecialMode )
        {
            if( aSelEntry == aClearFormatKey && GetSelectEntryPos() == 0 )
            {
                aSelEntry = sDefaultStyle;
                bClear = true;
                //not only apply default style but also call 'ClearFormatting'
                Sequence< PropertyValue > aEmptyVals;
                SfxToolBoxControl::Dispatch( m_xDispatchProvider, rtl::OUString(".uno:ResetAttributes"),
                    aEmptyVals);
            }
            else if( aSelEntry == aMoreKey && GetSelectEntryPos() == ( GetEntryCount() - 1 ) )
            {
                SfxViewFrame* pViewFrm = SfxViewFrame::Current();
                DBG_ASSERT( pViewFrm, "SvxStyleBox_Impl::Select(): no viewframe" );
                pViewFrm->ShowChildWindow( SID_STYLE_DESIGNER );
                SfxChildWindow* pChildWin = pViewFrm->GetChildWindow( SID_STYLE_DESIGNER );
                if ( pChildWin && pChildWin->GetWindow() )
                {
                    static_cast< SfxTemplateDialogWrapper* >( pChildWin )->SetParagraphFamily();
                    static_cast< SfxDockingWindow* >( pChildWin->GetWindow() )->AutoShow( sal_True );
                    Application::PostUserEvent(
                        STATIC_LINK( 0, SvxStyleBox_Impl, FocusHdl_Impl ), pChildWin->GetWindow() );
                }
                bDoIt = false;
            }
        }

        // #i36723# after ReleaseFocus() the new entry is included into the List
        sal_Bool bCreateNew = GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND;

        /*  #i33380# DR 2004-09-03 Moved the following line above the Dispatch() call.
            This instance may be deleted in the meantime (i.e. when a dialog is opened
            while in Dispatch()), accessing members will crash in this case. */
        ReleaseFocus();

        if( bDoIt )
        {
            if ( bClear )
                SetText( aSelEntry );
            SaveValue();

            Sequence< PropertyValue > aArgs( 2 );
            aArgs[0].Value  = makeAny( OUString( aSelEntry ) );
            aArgs[1].Name   = OUString("Family");
            aArgs[1].Value  = makeAny( sal_Int16( eStyleFamily ));
            if( bCreateNew )
            {
                aArgs[0].Name   = OUString("Param");
                SfxToolBoxControl::Dispatch( m_xDispatchProvider, rtl::OUString(".uno:StyleNewByExample"), aArgs);
            }
            else
            {
                aArgs[0].Name   = OUString("Template");
                SfxToolBoxControl::Dispatch( m_xDispatchProvider, m_aCommand, aArgs );
            }
        }
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
    sal_uInt16 nType = rNEvt.GetType();

    if ( EVENT_MOUSEBUTTONDOWN == nType || EVENT_GETFOCUS == nType )
        nCurSel = GetSelectEntryPos();
    else if ( EVENT_LOSEFOCUS == nType )
    {
        // don't handle before our Select() is called
        if ( !HasFocus() && !HasChildPathFocus() )
            SetText( GetSavedValue() );
    }
    return ComboBox::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long SvxStyleBox_Impl::Notify( NotifyEvent& rNEvt )
{
    long nHandled = 0;

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        sal_uInt16 nCode = rNEvt.GetKeyEvent()->GetKeyCode().GetCode();

        switch ( nCode )
        {
            case KEY_RETURN:
            case KEY_TAB:
            {
                if ( KEY_TAB == nCode )
                    bRelease = sal_False;
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
    return nHandled ? nHandled : ComboBox::Notify( rNEvt );
}

void SvxStyleBox_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        SetSizePixel(LogicToPixel(aLogicalSize, MAP_APPFONT));
        Size aDropSize( aLogicalSize.Width(), LOGICAL_EDIT_HEIGHT);
        SetDropDownSizePixel(LogicToPixel(aDropSize, MAP_APPFONT));
    }

    ComboBox::DataChanged( rDCEvt );
}

void SvxStyleBox_Impl::StateChanged( StateChangedType nStateChange )
{
    ComboBox::StateChanged( nStateChange );

    if ( nStateChange == STATE_CHANGE_VISIBLE )
    {
        bVisible = IsReallyVisible();
        if ( aVisibilityListener.IsSet() )
            aVisibilityListener.Call( this );
    }
    else if ( nStateChange == STATE_CHANGE_INITSHOW )
    {
        bVisible = sal_True;
        if ( aVisibilityListener.IsSet() )
            aVisibilityListener.Call( this );
    }
}

//--------------------------------------------------------------------

IMPL_STATIC_LINK( SvxStyleBox_Impl, FocusHdl_Impl, Control*, _pCtrl )
{
    (void)pThis;
    if ( _pCtrl )
        _pCtrl->GrabFocus();
    return 0;
}

// -----------------------------------------------------------------------

sal_Bool GetDocFontList_Impl( const FontList** ppFontList, SvxFontNameBox_Impl* pBox )
{
    sal_Bool bChanged = sal_False;
    const SfxObjectShell* pDocSh = SfxObjectShell::Current();
    SvxFontListItem* pFontListItem = NULL;

    if ( pDocSh )
        pFontListItem =
            (SvxFontListItem*)pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST );
    else
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<FontList> aFontList(new FontList( pBox ));
        SAL_WNODEPRECATED_DECLARATIONS_POP
        *ppFontList = aFontList.get();
        pBox->SetOwnFontList(aFontList);
        bChanged = sal_True;
    }

    if ( pFontListItem )
    {
        const FontList* pNewFontList = pFontListItem->GetFontList();
        DBG_ASSERT( pNewFontList, "Doc-FontList not available!" );

        // keine alte Liste, aber neue Liste
        if ( !*ppFontList && pNewFontList )
        {
            // => "ubernehmen
            *ppFontList = pNewFontList;
            bChanged = sal_True;
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
    else if ( pBox && ( pDocSh || ( !pDocSh && !ppFontList )))
    {
        // Disable box only when we have a SfxObjectShell and didn't get a font list OR
        // we don't have a SfxObjectShell and no current font list.
        // It's possible that we currently have no SfxObjectShell, but a current font list.
        // See #i58471: When a user set the focus into the font name combo box and opens
        // the help window with F1. After closing the help window, we disable the font name
        // combo box. The SfxObjectShell::Current() method returns in that case zero. But the
        // font list hasn't changed and therefore the combo box shouldn't be disabled!
        pBox->Disable();
    }

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

SvxFontNameBox_Impl::SvxFontNameBox_Impl( Window* pParent, const Reference< XDispatchProvider >& rDispatchProvider,const Reference< XFrame >& _xFrame, WinBits nStyle ) :

    FontNameBox        ( pParent, nStyle | WinBits( WB_DROPDOWN | WB_AUTOHSCROLL ) ),
    pFontList          ( NULL ),
    aLogicalSize       ( 75,160 ),
    nFtCount           ( 0 ),
    bRelease           ( sal_True ),
    m_xDispatchProvider( rDispatchProvider ),
    m_xFrame (_xFrame)
{
    SetSizePixel(LogicToPixel( aLogicalSize, MAP_APPFONT ));
    EnableControls_Impl();
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
    sal_uInt16 nType = rNEvt.GetType();

    if ( EVENT_MOUSEBUTTONDOWN == nType || EVENT_GETFOCUS == nType )
    {
        EnableControls_Impl();
        FillList();
    }
    return FontNameBox::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long SvxFontNameBox_Impl::Notify( NotifyEvent& rNEvt )
{
    long nHandled = 0;

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        sal_uInt16 nCode = rNEvt.GetKeyEvent()->GetKeyCode().GetCode();

        switch ( nCode )
        {
            case KEY_RETURN:
            case KEY_TAB:
            {
                if ( KEY_TAB == nCode )
                    bRelease = sal_False;
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

// ---------------------------------------------------------------------------
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
        bRelease = sal_True;
        return;
    }
    if ( m_xFrame.is() && m_xFrame->getContainerWindow().is() )
        m_xFrame->getContainerWindow()->setFocus();
}

// -----------------------------------------------------------------------

void SvxFontNameBox_Impl::EnableControls_Impl()
{
    SvtFontOptions aFontOpt;
    sal_Bool bEnable = aFontOpt.IsFontHistoryEnabled();
    sal_uInt16 nEntries = bEnable ? MAX_MRU_FONTNAME_ENTRIES : 0;
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

            Any a;
            Sequence< PropertyValue > aArgs( 1 );
            aArgs[0].Name   = OUString( "CharFontName" );
            aFontItem.QueryValue( a );
            aArgs[0].Value  = a;

            //  #i33380# DR 2004-09-03 Moved the following line above the Dispatch() call.
            //  This instance may be deleted in the meantime (i.e. when a dialog is opened
            //  while in Dispatch()), accessing members will crash in this case.
            ReleaseFocus_Impl();

            SfxToolBoxControl::Dispatch( m_xDispatchProvider,
                                         OUString( ".uno:CharFontName" ),
                                         aArgs );
        }
        else
            ReleaseFocus_Impl();
    }
}

//========================================================================
// class SvxColorWindow_Impl --------------------------------------------------
//========================================================================
#ifndef WB_NO_DIRECTSELECT
#define WB_NO_DIRECTSELECT      ((WinBits)0x04000000)
#endif

#define PALETTE_X 8
#define PALETTE_Y 13
#define PALETTE_SIZE (PALETTE_X * PALETTE_Y)

SvxColorWindow_Impl::SvxColorWindow_Impl( const OUString&            rCommand,
                                          sal_uInt16                     nSlotId,
                                          const Reference< XFrame >& rFrame,
                                          const String&              rWndTitle,
                                          Window*                    pParentWindow,
                                          const ::Color              rLastColor ) :

    SfxPopupWindow( nSlotId, rFrame, pParentWindow, WinBits( WB_STDPOPUP | WB_OWNERDRAWDECORATION ) ),

    theSlotId( nSlotId ),
    aColorSet( this, WinBits( WB_ITEMBORDER | WB_NAMEFIELD | WB_3DLOOK | WB_NO_DIRECTSELECT) ),
    maCommand( rCommand ),
    mLastColor( rLastColor )

{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SfxPoolItem* pItem = NULL;
    XColorListRef pColorList;
    const Size aSize12( 13, 13 );

    if ( pDocSh )
        if ( 0 != ( pItem = pDocSh->GetItem( SID_COLOR_TABLE ) ) )
            pColorList = ( (SvxColorListItem*)pItem )->GetColorList();

    if ( !pColorList.is() )
        pColorList = XColorList::CreateStdColorList();

    if ( SID_ATTR_CHAR_COLOR_BACKGROUND == theSlotId || SID_BACKGROUND_COLOR == theSlotId )
    {
        aColorSet.SetStyle( aColorSet.GetStyle() | WB_NONEFIELD );
        aColorSet.SetText( SVX_RESSTR( RID_SVXSTR_TRANSPARENT ) );
        aColorSet.SetAccessibleName( SVX_RESSTR( RID_SVXSTR_BACKGROUND ) );
    }
    else if ( SID_ATTR_CHAR_COLOR == theSlotId || SID_ATTR_CHAR_COLOR2 == theSlotId || SID_EXTRUSION_3D_COLOR == theSlotId )
    {
        SfxPoolItem* pDummy;

        Reference< XDispatchProvider > aDisp( GetFrame()->getController(), UNO_QUERY );
        SfxQueryStatus aQueryStatus( aDisp,
                                     SID_ATTR_AUTO_COLOR_INVALID,
                                     rtl::OUString( ".uno:AutoColorInvalid" ));
        SfxItemState eState = aQueryStatus.QueryState( pDummy );
        if( (SFX_ITEM_DEFAULT > eState) || ( SID_EXTRUSION_3D_COLOR == theSlotId ) )
        {
            aColorSet.SetStyle( aColorSet.GetStyle() | WB_NONEFIELD );
            aColorSet.SetText( SVX_RESSTR( RID_SVXSTR_AUTOMATIC ) );
            aColorSet.SetAccessibleName( SVX_RESSTR( RID_SVXSTR_TEXTCOLOR ) );
        }
    }
    else
    {
        aColorSet.SetAccessibleName( SVX_RESSTR( RID_SVXSTR_FRAME_COLOR ) );
    }

    if ( pColorList.is() )
    {
        short i = 0;
        long nCount = pColorList->Count();
        XColorEntry* pEntry = NULL;
        ::Color aColWhite( COL_WHITE );
        String aStrWhite( EditResId(RID_SVXITEMS_COLOR_WHITE) );

        if ( nCount > PALETTE_SIZE )
            // Show scrollbar if more than PALLETTE_SIZE colors are available
            aColorSet.SetStyle( aColorSet.GetStyle() | WB_VSCROLL );

        for ( i = 0; i < nCount; i++ )
        {
            pEntry = pColorList->GetColor(i);
            aColorSet.InsertItem( i+1, pEntry->GetColor(), pEntry->GetName() );
            if( pEntry->GetColor() == mLastColor )
                aColorSet.SelectItem( i+1 );
        }

        while ( i < PALETTE_SIZE )
        {
            // fill empty elements if less then PALLETTE_SIZE colors are available
            aColorSet.InsertItem( i+1, aColWhite, aStrWhite );
            i++;
        }
    }

    aColorSet.SetSelectHdl( LINK( this, SvxColorWindow_Impl, SelectHdl ) );
    aColorSet.SetColCount( PALETTE_X );
    aColorSet.SetLineCount( PALETTE_Y );

    lcl_CalcSizeValueSet( *this, aColorSet, aSize12 );

    SetHelpId( HID_POPUP_COLOR );
    aColorSet.SetHelpId( HID_POPUP_COLOR_CTRL );

    SetText( rWndTitle );
    aColorSet.Show();

    AddStatusListener( rtl::OUString( ".uno:ColorTableState" ));
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
    return new SvxColorWindow_Impl( maCommand, theSlotId, GetFrame(), GetText(), GetParent(), mLastColor );
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxColorWindow_Impl, SelectHdl)
{
    sal_uInt16 nItemId = aColorSet.GetSelectItemId();
    SvxColorItem aColorItem( aColorSet.GetItemColor( nItemId ), theSlotId );
    /*  #i33380# DR 2004-09-03 Moved the following line above the Dispatch() calls.
        This instance may be deleted in the meantime (i.e. when a dialog is opened
        while in Dispatch()), accessing members will crash in this case. */
    aColorSet.SetNoSelection();

    if ( IsInPopupMode() )
        EndPopupMode();

    if ( !nItemId && ( SID_ATTR_CHAR_COLOR_BACKGROUND == theSlotId  || SID_BACKGROUND_COLOR == theSlotId ) )
    {
        Sequence< PropertyValue > aArgs;
        SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( GetFrame()->getController(), UNO_QUERY ),
                                     maCommand,
                                     aArgs );
    }
    else if ( !nItemId && (SID_ATTR_CHAR_COLOR == theSlotId || SID_ATTR_CHAR_COLOR2  == theSlotId || SID_EXTRUSION_3D_COLOR == theSlotId) )
    {
        SvxColorItem _aColorItem( COL_AUTO, theSlotId );
        INetURLObject aObj( maCommand );

        Any a;
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = aObj.GetURLPath();
        _aColorItem.QueryValue( a );
        aArgs[0].Value = a;
        SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( GetFrame()->getController(), UNO_QUERY ),
                                     maCommand,
                                     aArgs );
    }
    else
    {
        INetURLObject aObj( maCommand );

        Any a;
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = aObj.GetURLPath();
        aColorItem.QueryValue( a );
        aArgs[0].Value = a;
        SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( GetFrame()->getController(), UNO_QUERY ),
                                     maCommand,
                                     aArgs );
    }

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

sal_Bool SvxColorWindow_Impl::Close()
{
    return SfxPopupWindow::Close();
}

// -----------------------------------------------------------------------

void SvxColorWindow_Impl::StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    if (( SFX_ITEM_DISABLED != eState ) && pState )
    {
        if (( nSID == SID_COLOR_TABLE ) && ( pState->ISA( SvxColorListItem )))
        {
            if ( pState )
            {
                XColorListRef pColorList = ((SvxColorListItem *)pState)->GetColorList();

                short i = 0;
                long nCount = pColorList->Count();
                XColorEntry* pEntry = NULL;
                ::Color aColWhite( COL_WHITE );
                String aStrWhite( SVX_RES( RID_SVXITEMS_COLOR_WHITE ) );

                // ScrollBar an oder aus
                WinBits nBits = aColorSet.GetStyle();
                if ( nCount > PALETTE_SIZE )
                    nBits &= ~WB_VSCROLL;
                else
                    nBits |= WB_VSCROLL;
                aColorSet.SetStyle( nBits );

                for ( i = 0; i < nCount; ++i )
                {
                    pEntry = pColorList->GetColor(i);
                    aColorSet.SetItemColor( i + 1, pEntry->GetColor() );
                    aColorSet.SetItemText ( i + 1, pEntry->GetName() );
                }

                while ( i < PALETTE_SIZE )
                {
                    aColorSet.SetItemColor( i + 1, aColWhite );
                    aColorSet.SetItemText ( i + 1, aStrWhite );
                    i++;
                }
            }
        }
    }
}

//========================================================================
// class SvxFrameWindow_Impl --------------------------------------------------
//========================================================================

SvxFrameWindow_Impl::SvxFrameWindow_Impl( sal_uInt16 nId, const Reference< XFrame >& rFrame, Window* pParentWindow ) :

    SfxPopupWindow( nId, rFrame, pParentWindow, WinBits( WB_STDPOPUP | WB_OWNERDRAWDECORATION ) ),
    aFrameSet   ( this, WinBits( WB_ITEMBORDER | WB_DOUBLEBORDER | WB_3DLOOK | WB_NO_DIRECTSELECT ) ),
    bParagraphMode(sal_False)

{
    BindListener();
    AddStatusListener(rtl::OUString(".uno:BorderReducedMode"));
    aImgList = ImageList( SVX_RES( RID_SVXIL_FRAME ) );

    /*
     *  1       2        3         4
     *  -------------------------------------
     *  NONE    LEFT     RIGHT     LEFTRIGHT
     *  TOP     BOTTOM   TOPBOTTOM OUTER
     *  -------------------------------------
     *  HOR     HORINNER VERINNER   ALL         <- kann ueber bParagraphMode
     *                                             abgeschaltet werden
     */

    sal_uInt16 i = 0;

    for ( i=1; i<9; i++ )
        aFrameSet.InsertItem( i, aImgList.GetImage(i) );

    //bParagraphMode should have been set in StateChanged
    if ( !bParagraphMode )
        for ( i = 9; i < 13; i++ )
            aFrameSet.InsertItem( i, aImgList.GetImage(i) );

    aFrameSet.SetColCount( 4 );
    aFrameSet.SetSelectHdl( LINK( this, SvxFrameWindow_Impl, SelectHdl ) );

    lcl_CalcSizeValueSet( *this, aFrameSet,Size( 20, 20 ));

    SetHelpId( HID_POPUP_FRAME );
    SetText( SVX_RESSTR(RID_SVXSTR_FRAME) );
    aFrameSet.SetAccessibleName( SVX_RESSTR(RID_SVXSTR_FRAME) );
    aFrameSet.Show();
}

SvxFrameWindow_Impl::~SvxFrameWindow_Impl()
{
    UnbindListener();
}

SfxPopupWindow* SvxFrameWindow_Impl::Clone() const
{
    //! HACK: wie bekomme ich den Paragraph-Mode ??
    return new SvxFrameWindow_Impl( GetId(), GetFrame(), GetParent() );
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
        aImgList = ImageList( SVX_RES( RID_SVXIL_FRAME ) );

        sal_uInt16  nNumOfItems = aFrameSet.GetItemCount();

        for( sal_uInt16 i = 1 ; i <= nNumOfItems ; ++i )
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
IMPL_LINK_NOARG(SvxFrameWindow_Impl, SelectHdl)
{
    ::Color             aColBlack( COL_BLACK );
    SvxBoxItem          aBorderOuter( SID_ATTR_BORDER_OUTER );
    SvxBoxInfoItem      aBorderInner( SID_ATTR_BORDER_INNER );
    SvxBorderLine       theDefLine;
    SvxBorderLine       *pLeft = 0,
                        *pRight = 0,
                        *pTop = 0,
                        *pBottom = 0;
    sal_uInt16              nSel = aFrameSet.GetSelectItemId();
    sal_uInt16              nModifier = aFrameSet.GetModifier();
    sal_uInt8               nValidFlags = 0;

    theDefLine.GuessLinesWidths(theDefLine.GetBorderLineStyle(),
            DEF_LINE_WIDTH_0);
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
    aBorderInner.SetValid( VALID_DISTANCE, sal_True );
    aBorderInner.SetValid( VALID_DISABLE, sal_False );

    if ( IsInPopupMode() )
        EndPopupMode();

    Any a;
    Sequence< PropertyValue > aArgs( 2 );
    aArgs[0].Name = OUString( "OuterBorder" );
    aBorderOuter.QueryValue( a );
    aArgs[0].Value = a;
    aArgs[1].Name = OUString( "InnerBorder" );
    aBorderInner.QueryValue( a );
    aArgs[1].Value = a;

    /*  #i33380# DR 2004-09-03 Moved the following line above the Dispatch() call.
        This instance may be deleted in the meantime (i.e. when a dialog is opened
        while in Dispatch()), accessing members will crash in this case. */
    aFrameSet.SetNoSelection();

    SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( GetFrame()->getController(), UNO_QUERY ),
                                 OUString( ".uno:SetBorderStyle" ),
                                 aArgs );
    return 0;
}

// -----------------------------------------------------------------------

void SvxFrameWindow_Impl::Resize()
{
    lcl_ResizeValueSet( *this, aFrameSet);
}

// -----------------------------------------------------------------------

void SvxFrameWindow_Impl::StateChanged(

    sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )

{
    if ( pState && nSID == SID_BORDER_REDUCED_MODE)
    {
        const SfxBoolItem* pItem = PTR_CAST( SfxBoolItem, pState );

        if ( pItem )
        {
            bParagraphMode = (sal_Bool)pItem->GetValue();
            //initial calls mustn't insert or remove elements
            if(aFrameSet.GetItemCount())
            {
                sal_Bool bTableMode = ( aFrameSet.GetItemCount() == 12 );
                sal_Bool bResize    = sal_False;

                if ( bTableMode && bParagraphMode )
                {
                    for ( sal_uInt16 i = 9; i < 13; i++ )
                        aFrameSet.RemoveItem(i);
                    bResize = sal_True;
                }
                else if ( !bTableMode && !bParagraphMode )
                {
                    for ( sal_uInt16 i = 9; i < 13; i++ )
                        aFrameSet.InsertItem( i, aImgList.GetImage(i) );
                    bResize = sal_True;
                }

                if ( bResize )
                {
                    lcl_CalcSizeValueSet( *this, aFrameSet,Size( 20, 20 ));
                }
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

sal_Bool SvxFrameWindow_Impl::Close()
{
    return SfxPopupWindow::Close();
}

//========================================================================
// class SvxLineWindow_Impl --------------------------------------------------
//========================================================================
Color lcl_mediumColor( Color aMain, Color /*aDefault*/ )
{
    return SvxBorderLine::threeDMediumColor( aMain );
}

SvxLineWindow_Impl::SvxLineWindow_Impl( sal_uInt16 nId, const Reference< XFrame >& rFrame, Window* pParentWindow ) :

    SfxPopupWindow( nId, rFrame, pParentWindow, WinBits( WB_STDPOPUP | WB_OWNERDRAWDECORATION | WB_AUTOSIZE ) ),
    m_aLineStyleLb( this )
{
    try
    {
        Reference< lang::XServiceInfo > xServices( rFrame->getController()->getModel(), UNO_QUERY_THROW );
        m_bIsWriter = xServices->supportsService(::rtl::OUString("com.sun.star.text.TextDocument"));
    }
    catch(const uno::Exception& )
    {
    }

    m_aLineStyleLb.SetPosSizePixel( 2, 2, 110, 140 );
    SetOutputSizePixel( Size( 114, 144 ) );

    m_aLineStyleLb.SetSourceUnit( FUNIT_TWIP );
    m_aLineStyleLb.SetNone( SVX_RESSTR(STR_NONE) );

    using namespace table::BorderLineStyle;
    m_aLineStyleLb.InsertEntry( SvxBorderLine::getWidthImpl( SOLID ), SOLID );
    m_aLineStyleLb.InsertEntry( SvxBorderLine::getWidthImpl( DOTTED ), DOTTED );
    m_aLineStyleLb.InsertEntry( SvxBorderLine::getWidthImpl( DASHED ), DASHED );

    // Double lines
    m_aLineStyleLb.InsertEntry( SvxBorderLine::getWidthImpl( DOUBLE ), DOUBLE );
    m_aLineStyleLb.InsertEntry( SvxBorderLine::getWidthImpl( THINTHICK_SMALLGAP ), THINTHICK_SMALLGAP, 20 );
    m_aLineStyleLb.InsertEntry( SvxBorderLine::getWidthImpl( THINTHICK_MEDIUMGAP ), THINTHICK_MEDIUMGAP );
    m_aLineStyleLb.InsertEntry( SvxBorderLine::getWidthImpl( THINTHICK_LARGEGAP ), THINTHICK_LARGEGAP );
    m_aLineStyleLb.InsertEntry( SvxBorderLine::getWidthImpl( THICKTHIN_SMALLGAP ), THICKTHIN_SMALLGAP, 20 );
    m_aLineStyleLb.InsertEntry( SvxBorderLine::getWidthImpl( THICKTHIN_MEDIUMGAP ), THICKTHIN_MEDIUMGAP );
    m_aLineStyleLb.InsertEntry( SvxBorderLine::getWidthImpl( THICKTHIN_LARGEGAP ), THICKTHIN_LARGEGAP );

    // Engraved / Embossed
    m_aLineStyleLb.InsertEntry( SvxBorderLine::getWidthImpl( EMBOSSED ), EMBOSSED, 15,
            &SvxBorderLine::threeDLightColor, &SvxBorderLine::threeDDarkColor,
            &lcl_mediumColor );
    m_aLineStyleLb.InsertEntry( SvxBorderLine::getWidthImpl( ENGRAVED ), ENGRAVED, 15,
            &SvxBorderLine::threeDDarkColor, &SvxBorderLine::threeDLightColor,
            &lcl_mediumColor );

    // Inset / Outset
    m_aLineStyleLb.InsertEntry( SvxBorderLine::getWidthImpl( OUTSET ), OUTSET, 10,
           &SvxBorderLine::lightColor, &SvxBorderLine::darkColor );
    m_aLineStyleLb.InsertEntry( SvxBorderLine::getWidthImpl( INSET ), INSET, 10,
           &SvxBorderLine::darkColor, &SvxBorderLine::lightColor );
    m_aLineStyleLb.SetWidth( 20 ); // 1pt by default

    m_aLineStyleLb.SetSelectHdl( LINK( this, SvxLineWindow_Impl, SelectHdl ) );

    SetHelpId( HID_POPUP_LINE );
    SetText( SVX_RESSTR(RID_SVXSTR_FRAME_STYLE) );
    m_aLineStyleLb.Show();
}

SfxPopupWindow* SvxLineWindow_Impl::Clone() const
{
    return new SvxLineWindow_Impl( GetId(), GetFrame(), GetParent() );
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxLineWindow_Impl, SelectHdl)
{
    SvxLineItem     aLineItem( SID_FRAME_LINESTYLE );
    SvxBorderStyle  nStyle = SvxBorderStyle( m_aLineStyleLb.GetSelectEntryStyle() );

    if ( m_aLineStyleLb.GetSelectEntryPos( ) > 0 )
    {
        SvxBorderLine aTmp;
        aTmp.SetBorderLineStyle( nStyle );
        aTmp.SetWidth( 20 ); // TODO Make it depend on a width field
        aLineItem.SetLine( &aTmp );
    }
    else
        aLineItem.SetLine( NULL );

    if ( IsInPopupMode() )
        EndPopupMode();

    Any a;
    Sequence< PropertyValue > aArgs( 1 );
    aArgs[0].Name = OUString( "LineStyle" );
    aLineItem.QueryValue( a, m_bIsWriter ? CONVERT_TWIPS : 0 );
    aArgs[0].Value = a;

    SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( GetFrame()->getController(), UNO_QUERY ),
                                 OUString( ".uno:LineStyle" ),
                                 aArgs );
    return 0;
}

// -----------------------------------------------------------------------

void SvxLineWindow_Impl::Resize()
{
    m_aLineStyleLb.Resize();
}

// -----------------------------------------------------------------------

sal_Bool SvxLineWindow_Impl::Close()
{
    return SfxPopupWindow::Close();
}

// -----------------------------------------------------------------------

Window* SvxLineWindow_Impl::GetPreferredKeyInputWindow()
{
    return &m_aLineStyleLb;
}

// -----------------------------------------------------------------------

void SvxLineWindow_Impl::GetFocus()
{
    m_aLineStyleLb.GrabFocus();
}

void SvxLineWindow_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxPopupWindow::DataChanged( rDCEvt );
#if 0
    if( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
    {
        CreateBitmaps();
        Invalidate();
    }
#endif
}

// -----------------------------------------------------------------------

//########################################################################
// Hilfsklassen

//========================================================================
// class SfxStyleControllerItem_Impl ------------------------------------------
//========================================================================

SfxStyleControllerItem_Impl::SfxStyleControllerItem_Impl(
    const Reference< XDispatchProvider >& rDispatchProvider,
    sal_uInt16                                nSlotId,      // Family-ID
    const rtl::OUString&                  rCommand,     // .uno: command bound to this item
    SvxStyleToolBoxControl&               rTbxCtl )     // Controller-Instanz, dem dieses Item zugeordnet ist.
    :   SfxStatusListener( rDispatchProvider, nSlotId, rCommand ),
        rControl( rTbxCtl )
{
}

// -----------------------------------------------------------------------

void SfxStyleControllerItem_Impl::StateChanged(
    sal_uInt16, SfxItemState eState, const SfxPoolItem* pState )
{
    switch ( GetId() )
    {
        case SID_STYLE_FAMILY1:
        case SID_STYLE_FAMILY2:
        case SID_STYLE_FAMILY3:
        case SID_STYLE_FAMILY4:
        case SID_STYLE_FAMILY5:
        {
            const sal_uInt16 nIdx = GetId() - SID_STYLE_FAMILY_START;

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

struct SvxStyleToolBoxControl::Impl
{
    String                              aClearForm;
    String                              aMore;
    ::std::vector< ::rtl::OUString >    aDefaultStyles;
    sal_Bool                        bListening;
    sal_Bool                        bSpecModeWriter;
    sal_Bool                        bSpecModeCalc;

    inline Impl( void )
        :aClearForm         ( SVX_RESSTR( RID_SVXSTR_CLEARFORM ) )
        ,aMore              ( SVX_RESSTR( RID_SVXSTR_MORE ) )
        ,bListening         ( sal_False )
        ,bSpecModeWriter    ( sal_False )
        ,bSpecModeCalc      ( sal_False )
    {


    }
    void InitializeStyles(Reference < frame::XModel > xModel)
    {
        //now convert the default style names to the localized names
        try
        {
            Reference< style::XStyleFamiliesSupplier > xStylesSupplier( xModel, UNO_QUERY_THROW );
            Reference< lang::XServiceInfo > xServices( xModel, UNO_QUERY_THROW );
            bSpecModeWriter = xServices->supportsService(::rtl::OUString("com.sun.star.text.TextDocument"));
            if(bSpecModeWriter)
            {
                Reference<container::XNameAccess> xParaStyles;
                    xStylesSupplier->getStyleFamilies()->getByName(::rtl::OUString("ParagraphStyles")) >>=
                    xParaStyles;
                static const sal_Char* aWriterStyles[] =
                {
                    "Standard",
                    "Heading 1",
                    "Heading 2",
                    "Heading 3",
                    "Text body"
                };
                for( sal_uInt32 nStyle = 0; nStyle < sizeof( aWriterStyles ) / sizeof( sal_Char*); ++nStyle )
                {
                    try
                    {
                        Reference< beans::XPropertySet > xStyle;
                        xParaStyles->getByName( rtl::OUString::createFromAscii( aWriterStyles[nStyle] )) >>= xStyle;
                        ::rtl::OUString sName;
                        xStyle->getPropertyValue(::rtl::OUString("DisplayName")) >>= sName;
                        if( !sName.isEmpty() )
                            aDefaultStyles.push_back(sName);
                    }
                    catch( const uno::Exception& )
                    {}
                }

            }
            else if( 0 != (
                bSpecModeCalc = xServices->supportsService(::rtl::OUString(
                    "com.sun.star.sheet.SpreadsheetDocument"))))
            {
                static const sal_Char* aCalcStyles[] =
                {
                    "Default",
                    "Heading1",
                    "Result",
                    "Result2"
                };
                Reference<container::XNameAccess> xCellStyles;
                    xStylesSupplier->getStyleFamilies()->getByName(
                        ::rtl::OUString("CellStyles")) >>=
                        xCellStyles;
                for( sal_uInt32 nStyle = 0; nStyle < sizeof( aCalcStyles ) / sizeof( sal_Char*); ++nStyle )
                {
                    try
                    {
                        const rtl::OUString sStyleName( rtl::OUString::createFromAscii( aCalcStyles[nStyle] ) );
                        if( xCellStyles->hasByName( sStyleName ) )
                        {
                            Reference< beans::XPropertySet > xStyle( xCellStyles->getByName( sStyleName), UNO_QUERY_THROW );
                            ::rtl::OUString sName;
                            xStyle->getPropertyValue(::rtl::OUString("DisplayName")) >>= sName;
                            if( !sName.isEmpty() )
                                aDefaultStyles.push_back(sName);
                        }
                    }
                    catch( const uno::Exception& )
                    {}
                }
            }
        }
        catch(const uno::Exception& )
        {
            OSL_FAIL("error while initializing style names");
        }
    }
};


// mapping table from bound items. BE CAREFUL this table must be in the
// same order as the uno commands bound to the slots SID_STYLE_FAMILY1..n
// MAX_FAMILIES must also be correctly set!
static const char* StyleSlotToStyleCommand[MAX_FAMILIES] =
{
    ".uno:CharStyle",
    ".uno:ParaStyle",
    ".uno:FrameStyle",
    ".uno:PageStyle",
    ".uno:TemplateFamily5"
};

SvxStyleToolBoxControl::SvxStyleToolBoxControl(
    sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx )
    :   SfxToolBoxControl   ( nSlotId, nId, rTbx ),
        pStyleSheetPool     ( NULL ),
        nActFamily          ( 0xffff ),
        pImpl               ( new Impl )
{
    for ( sal_uInt16 i=0; i<MAX_FAMILIES; i++ )
    {
        pBoundItems[i] = 0;
        m_xBoundItems[i] = Reference< XComponent >();
        pFamilyState[i]  = NULL;
    }
}

// -----------------------------------------------------------------------
SvxStyleToolBoxControl::~SvxStyleToolBoxControl()
{
}

// -----------------------------------------------------------------------
void SAL_CALL SvxStyleToolBoxControl::initialize( const Sequence< Any >& aArguments )
throw ( Exception, RuntimeException)
{
    SfxToolBoxControl::initialize( aArguments );

    // After initialize we should have a valid frame member where we can retrieve our
    // dispatch provider.
    if ( m_xFrame.is() )
    {
        pImpl->InitializeStyles(m_xFrame->getController()->getModel());
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame->getController(), UNO_QUERY );
        for ( sal_uInt16 i=0; i<MAX_FAMILIES; i++ )
        {
            pBoundItems[i]   = new SfxStyleControllerItem_Impl( xDispatchProvider,
                                                                SID_STYLE_FAMILY_START + i,
                                                                OUString::createFromAscii( StyleSlotToStyleCommand[i] ),
                                                                *this );
            m_xBoundItems[i] = Reference< XComponent >( static_cast< OWeakObject* >( pBoundItems[i] ), UNO_QUERY );
            pFamilyState[i]  = NULL;
        }
    }
}

// XComponent
void SAL_CALL SvxStyleToolBoxControl::dispose()
throw (::com::sun::star::uno::RuntimeException)
{
    SfxToolBoxControl::dispose();

    for( sal_uInt16 i=0; i<MAX_FAMILIES; i++ )
    {
        if ( m_xBoundItems[i].is() )
        {
            try
            {
                m_xBoundItems[i]->dispose();
            }
            catch ( Exception& )
            {
            }

            m_xBoundItems[i].clear();
            pBoundItems[i] = 0;
        }
        DELETEZ( pFamilyState[i] );
    }
    pStyleSheetPool = NULL;
    DELETEZ( pImpl );
}

// -----------------------------------------------------------------------
void SAL_CALL SvxStyleToolBoxControl::update() throw (RuntimeException)
{
    // Do nothing, we will start binding our listener when we are visible.
    // See link SvxStyleToolBoxControl::VisibilityNotification.
    SvxStyleBox_Impl* pBox = (SvxStyleBox_Impl*)GetToolBox().GetItemWindow( GetId() );
    if ( pBox->IsVisible() )
    {
        for ( int i=0; i<MAX_FAMILIES; i++ )
            pBoundItems [i]->ReBind();

        bindListener();
    }
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
            OSL_FAIL( "unknown style family" );
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
        sal_uInt16                  nCount      = pStyleSheetPool->Count();
        SfxStyleSheetBase*      pStyle      = NULL;
        sal_Bool                    bDoFill     = sal_False;

        pStyleSheetPool->SetSearchMask( eFamily, SFXSTYLEBIT_USED );

        //------------------------------
        // Ueberpruefen, ob Fill noetig:
        //------------------------------

        pStyle = pStyleSheetPool->First();
        //!!! TODO: This condition isn't right any longer, because we always show some default entries
        //!!! so the list doesn't show the count
        if ( nCount != pBox->GetEntryCount() )
        {
            bDoFill = sal_True;
        }
        else
        {
            sal_uInt16 i= 0;
            while ( pStyle && !bDoFill )
            {
                bDoFill = ( pBox->GetEntry(i) != pStyle->GetName() );
                pStyle = pStyleSheetPool->Next();
                i++;
            }
        }

        if ( bDoFill )
        {
            pBox->SetUpdateMode( sal_False );
            pBox->Clear();

            {
                sal_uInt16  _i;
                sal_uInt32  nCnt = pImpl->aDefaultStyles.size();

                pStyle = pStyleSheetPool->First();

                if( pImpl->bSpecModeWriter || pImpl->bSpecModeCalc )
                {
                    bool bInsert;
                    while ( pStyle )
                    {
                        // sort out default styles
                        bInsert = true;
                        ::rtl::OUString aName( pStyle->GetName() );
                        for( _i = 0 ; _i < nCnt ; ++_i )
                        {
                            if( pImpl->aDefaultStyles[_i] == aName )
                            {
                                bInsert = false;
                                break;
                            }
                        }

                        if( bInsert )
                            pBox->InsertEntry( aName );
                        pStyle = pStyleSheetPool->Next();
                    }
                }
                else
                {
                    while ( pStyle )
                    {
                        pBox->InsertEntry( pStyle->GetName() );
                        pStyle = pStyleSheetPool->Next();
                    }
                }
            }

            if( pImpl->bSpecModeWriter || pImpl->bSpecModeCalc )
            {
                // insert default styles
                sal_uInt16  _i;
                sal_uInt32  nCnt = pImpl->aDefaultStyles.size();
                sal_uInt16 nPos = 1;
                for( _i = 0 ; _i < nCnt ; ++_i )
                {
                    pBox->InsertEntry( pImpl->aDefaultStyles[_i], nPos );
                    ++nPos;
                }

                // disable sort to preserve special order
                WinBits nWinBits = pBox->GetStyle();
                nWinBits &= ~WB_SORT;
                pBox->SetStyle( nWinBits );

                pBox->InsertEntry( pImpl->aClearForm, 0 );
                pBox->SetSeparatorPos( 0 );

                pBox->InsertEntry( pImpl->aMore );

                // enable sort again
                nWinBits |= WB_SORT;
                pBox->SetStyle( nWinBits );
            }

            pBox->SetUpdateMode( sal_True );
            pBox->SetFamily( eFamily );

            sal_uInt16 nLines = Min( pBox->GetEntryCount(), MAX_STYLES_ENTRIES );
            pBox->SetDropDownLineCount( nLines );
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
//      String aStrSel( pBox->GetSelectEntry() );
        String aStrSel( pBox->GetText() );

        if ( rStyleName.Len() > 0 )
        {
            if ( rStyleName != aStrSel )
//              pBox->SelectEntry( rStyleName );
                pBox->SetText( rStyleName );
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
    SfxObjectShell*         pDocShell = SfxObjectShell::Current();

    if ( pDocShell )
        pPool = pDocShell->GetStyleSheetPool();

    sal_uInt16 i;
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
        {
            DBG_WARNING( "Unknown Family" ); // can happen
        }
    }
    else if ( pPool != pStyleSheetPool )
        pStyleSheetPool = pPool;

    FillStyleBox(); // entscheidet selbst, ob gefuellt werden muss

    if ( pItem )
        SelectStyle( pItem->GetStyleName() );
}

// -----------------------------------------------------------------------

void SvxStyleToolBoxControl::SetFamilyState( sal_uInt16 nIdx,
                                             const SfxTemplateItem* pItem )
{
    DELETEZ( pFamilyState[nIdx] );

    if ( pItem )
        pFamilyState[nIdx] = new SfxTemplateItem( *pItem );

    Update();
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxStyleToolBoxControl, VisibilityNotification)
{

    sal_uInt16 i;

    // Call ReBind() && UnBind() according to visibility
    SvxStyleBox_Impl* pBox = (SvxStyleBox_Impl*)( GetToolBox().GetItemWindow( GetId() ));
    if ( pBox->IsVisible() && !isBound() )
    {
        for ( i=0; i<MAX_FAMILIES; i++ )
            pBoundItems [i]->ReBind();

        bindListener();
    }
    else if ( !pBox->IsVisible() && isBound() )
    {
        for ( i=0; i<MAX_FAMILIES; i++ )
            pBoundItems[i]->UnBind();
        unbindListener();
    }

    return 0;
}

// -----------------------------------------------------------------------

void SvxStyleToolBoxControl::StateChanged(

    sal_uInt16 , SfxItemState eState, const SfxPoolItem* pState )

{
    sal_uInt16       nId    = GetId();
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

    if ( SFX_ITEM_DISABLED != eState )
        Update();
}

// -----------------------------------------------------------------------

Window* SvxStyleToolBoxControl::CreateItemWindow( Window *pParent )
{
    SvxStyleBox_Impl* pBox = new SvxStyleBox_Impl( pParent,
                                                   OUString( ".uno:StyleApply" ),
                                                   SFX_STYLE_FAMILY_PARA,
                                                   Reference< XDispatchProvider >( m_xFrame->getController(), UNO_QUERY ),
                                                   m_xFrame,
                                                   pImpl->aClearForm,
                                                   pImpl->aMore,
                                                   pImpl->bSpecModeWriter || pImpl->bSpecModeCalc );
    if( !pImpl->aDefaultStyles.empty())
        pBox->SetDefaultStyle( pImpl->aDefaultStyles[0] );
    // Set visibility listener to bind/unbind controller
    pBox->SetVisibilityListener( LINK( this, SvxStyleToolBoxControl, VisibilityNotification ));

    return pBox;
}

//========================================================================
// class SvxFontNameToolBoxControl ---------------------------------------
//========================================================================

SvxFontNameToolBoxControl::SvxFontNameToolBoxControl(
                                            sal_uInt16          nSlotId,
                                            sal_uInt16          nId,
                                            ToolBox&        rTbx )

    :   SfxToolBoxControl( nSlotId, nId, rTbx )
{
}

// -----------------------------------------------------------------------

void SvxFontNameToolBoxControl::StateChanged(

    sal_uInt16 , SfxItemState eState, const SfxPoolItem* pState )

{
    sal_uInt16               nId    = GetId();
    ToolBox&             rTbx   = GetToolBox();
    SvxFontNameBox_Impl* pBox   = (SvxFontNameBox_Impl*)(rTbx.GetItemWindow( nId ));

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
            const SvxFontItem* pFontItem = dynamic_cast< const SvxFontItem* >( pState );

            DBG_ASSERT( pFontItem, "svx::SvxFontNameToolBoxControl::StateChanged(), wrong item type!" );
            if( pFontItem )
                pBox->Update( pFontItem );
        }
        else
            pBox->SetText( String() );
        pBox->SaveValue();
    }

    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
}

// -----------------------------------------------------------------------

Window* SvxFontNameToolBoxControl::CreateItemWindow( Window *pParent )
{
    SvxFontNameBox_Impl* pBox = new SvxFontNameBox_Impl( pParent,
                                                         Reference< XDispatchProvider >( m_xFrame->getController(), UNO_QUERY ),
                                                         m_xFrame,0);
    return pBox;
}

//========================================================================
// class SvxFontColorToolBoxControl --------------------------------------
//========================================================================

SvxFontColorToolBoxControl::SvxFontColorToolBoxControl(
    sal_uInt16          nSlotId,
    sal_uInt16          nId,
    ToolBox&        rTbx )

    :   SfxToolBoxControl( nSlotId, nId, rTbx ),
    pBtnUpdater( new ::svx::ToolboxButtonColorUpdater(
                    nSlotId, nId, &GetToolBox(), TBX_UPDATER_MODE_CHAR_COLOR_NEW )),
    mLastColor( COL_AUTO )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWNONLY | rTbx.GetItemBits( nId ) );
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
        new SvxColorWindow_Impl(
                OUString( ".uno:Color" ),
                SID_ATTR_CHAR_COLOR,
                m_xFrame,
                SVX_RESSTR( RID_SVXITEMS_EXTRAS_CHARCOLOR ),
                &GetToolBox(),
                mLastColor);

    pColorWin->StartPopupMode( &GetToolBox(),
        FLOATWIN_POPUPMODE_GRABFOCUS|FLOATWIN_POPUPMODE_ALLOWTEAROFF|FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE );
    pColorWin->StartSelection();
    SetPopupWindow( pColorWin );
    return pColorWin;
}

// -----------------------------------------------------------------------

void SvxFontColorToolBoxControl::StateChanged(

    sal_uInt16 , SfxItemState eState, const SfxPoolItem* pState )

{
    sal_uInt16 nId = GetId();
    ToolBox& rTbx = GetToolBox();
    const SvxColorItem* pItem = 0;

    if ( SFX_ITEM_DONTCARE != eState )
       pItem = PTR_CAST( SvxColorItem, pState );

    if ( pItem )
    {
        pBtnUpdater->Update( pItem->GetValue() );
        mLastColor= pItem->GetValue();
    }

    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
    rTbx.SetItemState( nId, ( SFX_ITEM_DONTCARE == eState ) ? STATE_DONTKNOW : STATE_NOCHECK );
}

//========================================================================
// class SvxColorToolBoxControl --------------------------------
//========================================================================

SvxColorToolBoxControl::SvxColorToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :

    SfxToolBoxControl( nSlotId, nId, rTbx ),
    mLastColor( COL_AUTO )
{
    if ( nSlotId == SID_BACKGROUND_COLOR )
        rTbx.SetItemBits( nId, TIB_DROPDOWNONLY | rTbx.GetItemBits( nId ) );
    else
        rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();
    pBtnUpdater = new ::svx::ToolboxButtonColorUpdater( nSlotId, nId, &GetToolBox() );
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
    sal_uInt16 nResId = GetSlotId() == SID_BACKGROUND_COLOR ?
                        RID_SVXSTR_BACKGROUND : RID_SVXSTR_COLOR;
    SvxColorWindow_Impl* pColorWin = new SvxColorWindow_Impl(
        OUString( ".uno:BackgroundColor" ),
                                    SID_BACKGROUND_COLOR,
                                    m_xFrame,
                                    SVX_RESSTR(nResId),
                                    &GetToolBox(),
                                    mLastColor);

    pColorWin->StartPopupMode( &GetToolBox(),
        FLOATWIN_POPUPMODE_GRABFOCUS|FLOATWIN_POPUPMODE_ALLOWTEAROFF|FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE );
    pColorWin->StartSelection();
    SetPopupWindow( pColorWin );
    return pColorWin;
}

// -----------------------------------------------------------------------

void SvxColorToolBoxControl::StateChanged(
    sal_uInt16 , SfxItemState eState, const SfxPoolItem* pState )
{
    const SvxColorItem* pItem = 0;
    if ( SFX_ITEM_DONTCARE != eState )
        pItem = PTR_CAST( SvxColorItem, pState );

    if ( pItem ) {
        pBtnUpdater->Update( pItem->GetValue() );
        mLastColor= pItem->GetValue();
    }

    sal_uInt16 nId = GetId();
    ToolBox& rTbx = GetToolBox();
    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
    rTbx.SetItemState( nId, ( SFX_ITEM_DONTCARE == eState ) ? STATE_DONTKNOW : STATE_NOCHECK );
}

//========================================================================
// class SvxColorExtToolBoxControl ----------------------------------------
//========================================================================
/* Note:
   The initial color shown on the button is set in /core/svx/source/tbxctrls/tbxcolorupdate.cxx
   (ToolboxButtonColorUpdater::ToolboxButtonColorUpdater()) .
   The initial color used by the button is set in /core/svx/source/tbxcntrls/tbcontrl.cxx
   (SvxColorExtToolBoxControl::SvxColorExtToolBoxControl())
   and in case of writer for text(background)color also in /core/sw/source/ui/docvw/edtwin.cxx
   (SwEditWin::aTextBackColor and SwEditWin::aTextBackColor)
 */

SvxColorExtToolBoxControl::SvxColorExtToolBoxControl(
    sal_uInt16 nSlotId,
    sal_uInt16 nId,
    ToolBox& rTbx ) :

    SfxToolBoxControl( nSlotId, nId, rTbx ),
    pBtnUpdater(0),
    mLastColor( COL_AUTO )
{
    sal_uInt16 nMode;

    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
    bChoiceFromPalette = sal_False;

    // The following commands are available at the various modules
    switch( nSlotId )
    {
        case SID_ATTR_CHAR_COLOR:
            addStatusListener( OUString( ".uno:Color" ));
            nMode = TBX_UPDATER_MODE_CHAR_COLOR_NEW;
            mLastColor = COL_RED;
            break;

        case SID_ATTR_CHAR_COLOR2:
            addStatusListener( OUString( ".uno:CharColorExt" ));
            nMode = TBX_UPDATER_MODE_CHAR_COLOR_NEW;
            mLastColor = COL_RED;
            break;

        case SID_BACKGROUND_COLOR:
        case SID_ATTR_CHAR_COLOR_BACKGROUND:
        default:
            addStatusListener( OUString( ".uno:CharBackgroundExt" ));
            nMode = TBX_UPDATER_MODE_CHAR_COLOR_NEW;
            mLastColor = COL_YELLOW;
            break;

        case SID_FRAME_LINECOLOR:
            addStatusListener( OUString( ".uno:FrameLineColor" ));
            nMode = 0;
            mLastColor = COL_BLUE;
            break;
    }

    pBtnUpdater = new ::svx::ToolboxButtonColorUpdater( nSlotId, nId, &GetToolBox(), nMode );
}

// -----------------------------------------------------------------------

SvxColorExtToolBoxControl::~SvxColorExtToolBoxControl()
{
    delete pBtnUpdater;
}

// -----------------------------------------------------------------------

SfxPopupWindowType SvxColorExtToolBoxControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONTIMEOUT;
}

// -----------------------------------------------------------------------

SfxPopupWindow* SvxColorExtToolBoxControl::CreatePopupWindow()
{
    SvxColorWindow_Impl* pColorWin =
        new SvxColorWindow_Impl(
                            m_aCommandURL,
                            GetSlotId(),
                            m_xFrame,
                            SVX_RESSTR( RID_SVXITEMS_EXTRAS_CHARCOLOR ),
                            &GetToolBox(),
                            mLastColor );

    switch( GetSlotId() )
    {
        case SID_ATTR_CHAR_COLOR_BACKGROUND :
            pColorWin->SetText( SVX_RESSTR( RID_SVXSTR_EXTRAS_CHARBACKGROUND ) );
            break;

        case SID_BACKGROUND_COLOR :
            pColorWin->SetText( SVX_RESSTR( RID_SVXSTR_BACKGROUND ) );
            break;

        case SID_FRAME_LINECOLOR:
            pColorWin->SetText( SVX_RESSTR( RID_SVXSTR_FRAME_COLOR ) );
            break;
    }

    pColorWin->StartPopupMode( &GetToolBox(),
        FLOATWIN_POPUPMODE_GRABFOCUS|FLOATWIN_POPUPMODE_ALLOWTEAROFF|FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE );
    pColorWin->StartSelection();
    SetPopupWindow( pColorWin );
    bChoiceFromPalette = sal_True;
    return pColorWin;
}

// -----------------------------------------------------------------------

void SvxColorExtToolBoxControl::StateChanged(

    sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )

{
    const SvxColorItem* pItem = 0;
    if ( bChoiceFromPalette )
    {
        bChoiceFromPalette = sal_False;
        switch( nSID )
        {
            case SID_ATTR_CHAR_COLOR :
            case SID_ATTR_CHAR_COLOR2 :
            case SID_ATTR_CHAR_COLOR_BACKGROUND :
            case SID_BACKGROUND_COLOR :
                if ( SFX_ITEM_DONTCARE != eState )
                    pItem = PTR_CAST( SvxColorItem, pState );

                if ( pItem )
                {
                    pBtnUpdater->Update( pItem->GetValue() );
                    mLastColor = pItem->GetValue();
                }
                break;

            case SID_FRAME_LINECOLOR :
                ToolBox& rTbx = GetToolBox();
                rTbx.EnableItem( nSID, SFX_ITEM_DISABLED != eState );
                rTbx.SetItemState( nSID, ( SFX_ITEM_DONTCARE == eState ) ? STATE_DONTKNOW : STATE_NOCHECK );

                if ( SFX_ITEM_DONTCARE != eState )
                {
                    pItem = PTR_CAST( SvxColorItem, pState );
                    if ( pItem )
                    {
                        pBtnUpdater->Update( pItem->GetValue());
                        mLastColor = pItem->GetValue();
                    }
                }
                break;
        }
    }
}

// -----------------------------------------------------------------------

void SvxColorExtToolBoxControl::Select( sal_Bool )
{
    OUString aCommand;
    OUString aParamName;
    sal_Bool bNoArgs = sal_False;

    switch( GetSlotId() )
    {
        case SID_ATTR_CHAR_COLOR2 :
            bNoArgs     = sal_True;
            aCommand    = OUString( ".uno:CharColorExt" );
            aParamName  = OUString( "CharColorExt" );
            break;

        case SID_ATTR_CHAR_COLOR  :
            aCommand    = OUString( ".uno:Color" );
            aParamName  = OUString( "Color" );
            break;

        case SID_BACKGROUND_COLOR :
            aCommand    = OUString( ".uno:BackgroundColor" );
            aParamName  = OUString( "BackgroundColor" );
            break;

        case SID_ATTR_CHAR_COLOR_BACKGROUND :
            bNoArgs     = sal_True;
            aCommand    = OUString( ".uno:CharBackgroundExt" );
            aParamName  = OUString( "CharBackgroundExt" );
            break;

        case SID_FRAME_LINECOLOR  :
            aCommand    = OUString( ".uno:FrameLineColor" );
            aParamName  = OUString( "FrameLineColor" );
            break;
    }

    Sequence< PropertyValue > aArgs( 1 );
    aArgs[0].Name  = aParamName;
    if ( bNoArgs )
        aArgs[0].Value = makeAny( GetToolBox().IsItemChecked( GetId() ));
    else
        aArgs[0].Value = makeAny( (sal_uInt32)( mLastColor.GetColor() ));
    Dispatch( aCommand, aArgs );
}

//========================================================================
// class SvxFrameToolBoxControl ------------------------------------------
//========================================================================

SvxFrameToolBoxControl::SvxFrameToolBoxControl(
    sal_uInt16      nSlotId,
    sal_uInt16      nId,
    ToolBox&    rTbx )

    :   SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWNONLY | rTbx.GetItemBits( nId ) );
}

// -----------------------------------------------------------------------

SfxPopupWindowType SvxFrameToolBoxControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK;
}

// -----------------------------------------------------------------------

SfxPopupWindow* SvxFrameToolBoxControl::CreatePopupWindow()
{
    SvxFrameWindow_Impl* pFrameWin = new SvxFrameWindow_Impl(
                                        GetSlotId(), m_xFrame, &GetToolBox() );

    pFrameWin->StartPopupMode( &GetToolBox(),
                               FLOATWIN_POPUPMODE_GRABFOCUS |
                               FLOATWIN_POPUPMODE_ALLOWTEAROFF |
                               FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE );
    pFrameWin->StartSelection();
    SetPopupWindow( pFrameWin );

    return pFrameWin;
}

// -----------------------------------------------------------------------

void SvxFrameToolBoxControl::StateChanged(

    sal_uInt16, SfxItemState eState, const SfxPoolItem*  )

{
    sal_uInt16                  nId     = GetId();
    ToolBox&                rTbx    = GetToolBox();

    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
    rTbx.SetItemState( nId, (SFX_ITEM_DONTCARE == eState)
                            ? STATE_DONTKNOW
                            : STATE_NOCHECK );
}

//========================================================================
// class SvxFrameLineStyleToolBoxControl ---------------------------------
//========================================================================

SvxFrameLineStyleToolBoxControl::SvxFrameLineStyleToolBoxControl(
    sal_uInt16          nSlotId,
    sal_uInt16          nId,
    ToolBox&        rTbx )

    :    SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWNONLY | rTbx.GetItemBits( nId ) );
}

// -----------------------------------------------------------------------

SfxPopupWindowType SvxFrameLineStyleToolBoxControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK;
}

// -----------------------------------------------------------------------

SfxPopupWindow* SvxFrameLineStyleToolBoxControl::CreatePopupWindow()
{
    SvxLineWindow_Impl* pLineWin = new SvxLineWindow_Impl( GetSlotId(), m_xFrame, &GetToolBox() );
    pLineWin->StartPopupMode( &GetToolBox(),
                              FLOATWIN_POPUPMODE_GRABFOCUS |
                              FLOATWIN_POPUPMODE_ALLOWTEAROFF |
                              FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE );
    SetPopupWindow( pLineWin );

    return pLineWin;
}

// -----------------------------------------------------------------------

void SvxFrameLineStyleToolBoxControl::StateChanged(

    sal_uInt16 , SfxItemState eState, const SfxPoolItem*  )
{
    sal_uInt16       nId    = GetId();
    ToolBox&     rTbx   = GetToolBox();

    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
    rTbx.SetItemState( nId, (SFX_ITEM_DONTCARE == eState)
                                ? STATE_DONTKNOW
                                : STATE_NOCHECK );
}

//========================================================================
// class SvxFrameLineColorToolBoxControl ---------------------------------
//========================================================================

SvxFrameLineColorToolBoxControl::SvxFrameLineColorToolBoxControl(
    sal_uInt16      nSlotId,
    sal_uInt16      nId,
    ToolBox&    rTbx ) :

    SfxToolBoxControl( nSlotId, nId, rTbx ),
    pBtnUpdater(new ::svx::ToolboxButtonColorUpdater( nSlotId, nId, &GetToolBox() )),
    mLastColor( COL_AUTO )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWNONLY | rTbx.GetItemBits( nId ) );
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
    SvxColorWindow_Impl* pColorWin = new SvxColorWindow_Impl(
                                        OUString( ".uno:FrameLineColor" ),
                                        SID_FRAME_LINECOLOR,
                                        m_xFrame,
                                        SVX_RESSTR(RID_SVXSTR_FRAME_COLOR),
                                        &GetToolBox(),
                                        mLastColor);

    pColorWin->StartPopupMode( &GetToolBox(),
        FLOATWIN_POPUPMODE_GRABFOCUS|FLOATWIN_POPUPMODE_ALLOWTEAROFF|FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE );
    pColorWin->StartSelection();
    SetPopupWindow( pColorWin );
    return pColorWin;
}

// -----------------------------------------------------------------------

void SvxFrameLineColorToolBoxControl::StateChanged(
    sal_uInt16 , SfxItemState eState, const SfxPoolItem* pState )
{
    sal_uInt16 nId = GetId();
    ToolBox& rTbx = GetToolBox();
    rTbx.EnableItem( nId, SFX_ITEM_DISABLED != eState );
    rTbx.SetItemState( nId, ( SFX_ITEM_DONTCARE == eState ) ? STATE_DONTKNOW : STATE_NOCHECK );

    const SvxColorItem* pItem = 0;
    if ( SFX_ITEM_DONTCARE != eState )
    {
        pItem = PTR_CAST( SvxColorItem, pState );
        if ( pItem )
        {
            pBtnUpdater->Update( pItem->GetValue());
            mLastColor = pItem->GetValue();
        }
    }
}

//========================================================================
// class SvxSimpleUndoRedoController -------------------------------------
//========================================================================

SvxSimpleUndoRedoController::SvxSimpleUndoRedoController( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx  )
    :SfxToolBoxControl( nSlotId, nId, rTbx )
{
    aDefaultText = rTbx.GetItemText( nId );
}

// -----------------------------------------------------------------------

SvxSimpleUndoRedoController::~SvxSimpleUndoRedoController()
{
}

// -----------------------------------------------------------------------

void SvxSimpleUndoRedoController::StateChanged( sal_uInt16, SfxItemState eState, const SfxPoolItem* pState )
{
    SfxStringItem* pItem = PTR_CAST( SfxStringItem, pState );
    ToolBox& rBox = GetToolBox();
    if ( pItem && eState != SFX_ITEM_DISABLED )
    {
        ::rtl::OUString aNewText( MnemonicGenerator::EraseAllMnemonicChars( pItem->GetValue() ) );
        rBox.SetQuickHelpText( GetId(), aNewText );
    }
    if ( eState == SFX_ITEM_DISABLED )
        rBox.SetQuickHelpText( GetId(), aDefaultText );
    rBox.EnableItem( GetId(), eState != SFX_ITEM_DISABLED );
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

void lcl_CalcSizeValueSet( Window &rWin, ValueSet &rValueSet, const Size &aItemSize )
{
    Size aSize = rValueSet.CalcWindowSizePixel( aItemSize );
    aSize.Width()  += 4;
    aSize.Height() += 4;
    rWin.SetOutputSizePixel( aSize );
}

// -----------------------------------------------------------------------------
Reference< ::com::sun::star::accessibility::XAccessible > SvxFontNameBox_Impl::CreateAccessible()
{
    FillList();
    return FontNameBox::CreateAccessible();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

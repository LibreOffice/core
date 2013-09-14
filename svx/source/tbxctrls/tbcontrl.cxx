/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <string>
#include <tools/shl.hxx>
#include <tools/color.hxx>
#include <svl/poolitem.hxx>
#include <svl/eitem.hxx>
#include <svl/itemset.hxx>
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
#include <sfx2/htmlmode.hxx>
#include <svx/xtable.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
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

static void lcl_ResizeValueSet( Window &rWin, ValueSet &rValueSet );
static void lcl_CalcSizeValueSet( Window &rWin, ValueSet &rValueSet, const Size &aItemSize );

// namespaces
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
                        const Reference< XFrame >& _xFrame,const OUString& rClearFormatKey, const OUString& rMoreKey, bool bInSpecialMode );
    ~SvxStyleBox_Impl();

    void            SetFamily( SfxStyleFamily eNewFamily );
    inline bool IsVisible() { return bVisible; }

    virtual long    PreNotify( NotifyEvent& rNEvt );
    virtual long    Notify( NotifyEvent& rNEvt );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual void    StateChanged( StateChangedType nStateChange );

    virtual void    UserDraw( const UserDrawEvent& rUDEvt );

    inline void     SetVisibilityListener( const Link& aVisListener ) { aVisibilityListener = aVisListener; }
    inline void     RemoveVisibilityListener() { aVisibilityListener = Link(); }

    void            SetDefaultStyle( const OUString& rDefault ) { sDefaultStyle = rDefault; }
    DECL_STATIC_LINK( SvxStyleBox_Impl, FocusHdl_Impl, Control* );

protected:
    virtual void    Select();

private:
    SfxStyleFamily                  eStyleFamily;
    sal_uInt16                      nCurSel;
    bool                            bRelease;
    Size                            aLogicalSize;
    Link                            aVisibilityListener;
    bool                            bVisible;
    Reference< XDispatchProvider >  m_xDispatchProvider;
    Reference< XFrame >             m_xFrame;
    OUString                        m_aCommand;
    OUString                        aClearFormatKey;
    OUString                        aMoreKey;
    OUString                        sDefaultStyle;
    bool                            bInSpecialMode;

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
    OUString                       aCurText;
    sal_uInt16                     nFtCount;
    bool                           bRelease;
    Reference< XDispatchProvider > m_xDispatchProvider;
    Reference< XFrame >            m_xFrame;
    bool            mbEndPreview;

    void            ReleaseFocus_Impl();
    void            EnableControls_Impl();

    void            EndPreview()
    {
        Sequence< PropertyValue > aArgs;
        SfxToolBoxControl::Dispatch( m_xDispatchProvider,
                                         OUString( ".uno:CharEndPreviewFontName" ),
                                         aArgs );
    }
    DECL_DLLPRIVATE_LINK( CheckAndMarkUnknownFont, VclWindowEvent* );

protected:
    virtual void    Select();
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

public:
    SvxFontNameBox_Impl( Window* pParent, const Reference< XDispatchProvider >& rDispatchProvider,const Reference< XFrame >& _xFrame
        , WinBits nStyle = WB_SORT
        );
    virtual ~SvxFontNameBox_Impl();

    void            FillList();
    void            Update( const SvxFontItem* pFontItem );
    sal_uInt16      GetListCount() { return nFtCount; }
    void            Clear() { FontNameBox::Clear(); nFtCount = 0; }
    void            Fill( const FontList* pList )
                        { FontNameBox::Fill( pList );
                          nFtCount = pList->GetFontNameCount(); }
    virtual void    UserDraw( const UserDrawEvent& rUDEvt );
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

// SelectHdl needs the Modifiers, get them in MouseButtonUp

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

    DECL_LINK( SelectHdl, void * );

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

    DECL_LINK( SelectHdl, void * );

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
// Helper classes:
//========================================================================
// class SfxStyleControllerItem ------------------------------------------
//========================================================================
class SvxStyleToolBoxControl;

class SfxStyleControllerItem_Impl : public SfxStatusListener
{
    public:
        SfxStyleControllerItem_Impl( const Reference< XDispatchProvider >& rDispatchProvider,
                                     sal_uInt16 nSlotId,
                                     const OUString& rCommand,
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
    const OUString&                         rCommand,
    SfxStyleFamily                          eFamily,
    const Reference< XDispatchProvider >&   rDispatchProvider,
    const Reference< XFrame >&              _xFrame,
    const OUString&                         rClearFormatKey,
    const OUString&                         rMoreKey,
    bool                                    bInSpec) :

    ComboBox( pParent, SVX_RES( RID_SVXTBX_STYLE ) ),

    eStyleFamily( eFamily ),
    bRelease    ( true ),
    bVisible(false),
    m_xDispatchProvider( rDispatchProvider ),
    m_xFrame(_xFrame),
    m_aCommand  ( rCommand ),
    aClearFormatKey ( rClearFormatKey ),
    aMoreKey        ( rMoreKey ),
    bInSpecialMode  ( bInSpec )
{
    aLogicalSize = PixelToLogic( GetSizePixel(), MAP_APPFONT );
    EnableAutocomplete( sal_True );
    EnableUserDraw( true );
    SetUserItemSize( Size( 0, 30 ) );
}

SvxStyleBox_Impl::~SvxStyleBox_Impl()
{
}

// -----------------------------------------------------------------------

void SvxStyleBox_Impl::ReleaseFocus()
{
    if ( !bRelease )
    {
        bRelease = true;
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
        OUString aSearchEntry( GetText() );
        bool bDoIt = true, bClear = false;
        if( bInSpecialMode )
        {
            if( aSearchEntry == aClearFormatKey && GetSelectEntryPos() == 0 )
            {
                aSearchEntry = sDefaultStyle;
                bClear = true;
                //not only apply default style but also call 'ClearFormatting'
                Sequence< PropertyValue > aEmptyVals;
                SfxToolBoxControl::Dispatch( m_xDispatchProvider, OUString(".uno:ResetAttributes"),
                    aEmptyVals);
            }
            else if( aSearchEntry == aMoreKey && GetSelectEntryPos() == ( GetEntryCount() - 1 ) )
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

        //Do we need to create a new style?
        SfxObjectShell *pShell = SfxObjectShell::Current();
        SfxStyleSheetBasePool* pPool = pShell->GetStyleSheetPool();
        SfxStyleSheetBase* pStyle = NULL;

        bool bCreateNew = 0;

        if ( pPool )
        {
            pPool->SetSearchMask( eStyleFamily, SFXSTYLEBIT_ALL );

            pStyle = pPool->First();
            while ( pStyle && OUString( pStyle->GetName() ) != aSearchEntry )
                pStyle = pPool->Next();
        }

        if ( !pStyle )
        {
            // cannot find the style for whatever reason
            // therefore create a new style
            bCreateNew = 1;
        }

        /*  #i33380# DR 2004-09-03 Moved the following line above the Dispatch() call.
            This instance may be deleted in the meantime (i.e. when a dialog is opened
            while in Dispatch()), accessing members will crash in this case. */
        ReleaseFocus();

        if( bDoIt )
        {
            if ( bClear )
                SetText( aSearchEntry );
            SaveValue();

            Sequence< PropertyValue > aArgs( 2 );
            aArgs[0].Value  = makeAny( OUString( aSearchEntry ) );
            aArgs[1].Name   = OUString("Family");
            aArgs[1].Value  = makeAny( sal_Int16( eStyleFamily ));
            if( bCreateNew )
            {
                aArgs[0].Name   = OUString("Param");
                SfxToolBoxControl::Dispatch( m_xDispatchProvider, OUString(".uno:StyleNewByExample"), aArgs);
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
                    bRelease = false;
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
        bVisible = true;
        if ( aVisibilityListener.IsSet() )
            aVisibilityListener.Call( this );
    }
}

void SvxStyleBox_Impl::UserDraw( const UserDrawEvent& rUDEvt )
{
    sal_uInt16 nItem = rUDEvt.GetItemId();

    if ( nItem == 0 || nItem == GetEntryCount() - 1 )
    {
        // draw the non-style entries, ie. "Clear Formatting" or "More..."
        DrawEntry( rUDEvt, true, true );
    }
    else
    {
        SfxObjectShell *pShell = SfxObjectShell::Current();
        SfxStyleSheetBasePool* pPool = pShell->GetStyleSheetPool();
        SfxStyleSheetBase* pStyle = NULL;

        OUString aStyleName( GetEntry( nItem ) );

        if ( pPool )
        {
            pPool->SetSearchMask( eStyleFamily, SFXSTYLEBIT_ALL );

            pStyle = pPool->First();
            while ( pStyle && OUString( pStyle->GetName() ) != aStyleName )
                pStyle = pPool->Next();
        }

        if ( !pStyle )
        {
            // cannot find the style for whatever reason
            DrawEntry( rUDEvt, true, true );
        }
        else
        {
            const SfxItemSet& aItemSet = pStyle->GetItemSet();

            const SvxFontItem *pFontItem = static_cast< const SvxFontItem* >( aItemSet.GetItem( SID_ATTR_CHAR_FONT ) );
            const SvxFontHeightItem *pFontHeightItem = static_cast< const SvxFontHeightItem* >( aItemSet.GetItem( SID_ATTR_CHAR_FONTHEIGHT ) );

            if ( pFontItem && pFontHeightItem )
            {
                OutputDevice *pDevice = rUDEvt.GetDevice();

                Size aFontSize( 0, pFontHeightItem->GetHeight() );
                Size aPixelSize( pDevice->LogicToPixel( aFontSize, pShell->GetMapUnit() ) );

                // setup the font properties
                Font aFont( pFontItem->GetFamilyName(), pFontItem->GetStyleName(), aPixelSize );

                const SfxPoolItem *pItem = aItemSet.GetItem( SID_ATTR_CHAR_WEIGHT );
                if ( pItem )
                    aFont.SetWeight( static_cast< const SvxWeightItem* >( pItem )->GetWeight() );

                pItem = aItemSet.GetItem( SID_ATTR_CHAR_POSTURE );
                if ( pItem )
                    aFont.SetItalic( static_cast< const SvxPostureItem* >( pItem )->GetPosture() );

                pItem = aItemSet.GetItem( SID_ATTR_CHAR_CONTOUR );
                if ( pItem )
                    aFont.SetOutline( static_cast< const SvxContourItem* >( pItem )->GetValue() );

                pItem = aItemSet.GetItem( SID_ATTR_CHAR_SHADOWED );
                if ( pItem )
                    aFont.SetShadow( static_cast< const SvxShadowedItem* >( pItem )->GetValue() );

                pItem = aItemSet.GetItem( SID_ATTR_CHAR_RELIEF );
                if ( pItem )
                    aFont.SetRelief( static_cast< FontRelief >( static_cast< const SvxCharReliefItem* >( pItem )->GetValue() ) );

                pItem = aItemSet.GetItem( SID_ATTR_CHAR_UNDERLINE );
                if ( pItem )
                    aFont.SetUnderline( static_cast< const SvxUnderlineItem* >( pItem )->GetLineStyle() );

                pItem = aItemSet.GetItem( SID_ATTR_CHAR_OVERLINE );
                if ( pItem )
                    aFont.SetOverline( static_cast< FontUnderline >( static_cast< const SvxOverlineItem* >( pItem )->GetValue() ) );

                pItem = aItemSet.GetItem( SID_ATTR_CHAR_STRIKEOUT );
                if ( pItem )
                    aFont.SetStrikeout( static_cast< const SvxCrossedOutItem* >( pItem )->GetStrikeout() );

                pItem = aItemSet.GetItem( SID_ATTR_CHAR_EMPHASISMARK );
                if ( pItem )
                    aFont.SetEmphasisMark( static_cast< const SvxEmphasisMarkItem* >( pItem )->GetEmphasisMark() );

                // setup the device & draw
                Font aOldFont( pDevice->GetFont() );
                Color aOldColor( pDevice->GetTextColor() );
                Color aOldFillColor( pDevice->GetFillColor() );

                pDevice->SetFont( aFont );

                // text color, when we are not selected
                pItem = aItemSet.GetItem( SID_ATTR_CHAR_COLOR );
                if ( pItem && rUDEvt.GetItemId() != GetSelectEntryPos() )
                {
                    Color aColor( static_cast< const SvxColorItem* >( pItem )->GetValue() );
                    if ( aColor != COL_AUTO )
                        pDevice->SetTextColor( aColor );
                }

                // background color
                pItem = aItemSet.GetItem( SID_ATTR_BRUSH );
                if ( pItem && rUDEvt.GetItemId() != GetSelectEntryPos() )
                {
                    Color aColor( static_cast< const SvxBrushItem* >( pItem )->GetColor() );
                    if ( aColor != COL_AUTO )
                    {
                        pDevice->SetFillColor( aColor );
                        pDevice->DrawRect( rUDEvt.GetRect() );
                    }
                }

                // IMG_TXT_DISTANCE in ilstbox.hxx is 6, then 1 is added as
                // nBorder, and we are adding 1 in order to look better when
                // italics is present
                const int nLeftDistance = 8;

                Rectangle aTextRect;
                pDevice->GetTextBoundRect( aTextRect, aStyleName );

                Point aPos( rUDEvt.GetRect().TopLeft() );
                aPos.X() += nLeftDistance;
                if ( aTextRect.Bottom() > rUDEvt.GetRect().GetHeight() )
                {
                    // the text does not fit, adjust the font size
                    double ratio = static_cast< double >( rUDEvt.GetRect().GetHeight() ) / aTextRect.Bottom();
                    aPixelSize.Width() *= ratio;
                    aPixelSize.Height() *= ratio;
                    aFont.SetSize( aPixelSize );
                    pDevice->SetFont( aFont );
                }
                else
                    aPos.Y() += ( rUDEvt.GetRect().GetHeight() - aTextRect.Bottom() ) / 2;

                pDevice->DrawText( aPos, aStyleName );

                pDevice->SetFillColor( aOldFillColor );
                pDevice->SetTextColor( aOldColor );
                pDevice->SetFont( aOldFont );

                // draw separator, if present
                DrawEntry( rUDEvt, false, false );
            }
            else
                DrawEntry( rUDEvt, true, true );
        }
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

static bool lcl_GetDocFontList( const FontList** ppFontList, SvxFontNameBox_Impl* pBox )
{
    bool bChanged = false;
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
        bChanged = true;
    }

    if ( pFontListItem )
    {
        const FontList* pNewFontList = pFontListItem->GetFontList();
        DBG_ASSERT( pNewFontList, "Doc-FontList not available!" );

        // No old list, but a new list
        if ( !*ppFontList && pNewFontList )
        {
            // => take over
            *ppFontList = pNewFontList;
            bChanged = true;
        }
        else
        {
            // Comparing the font lists is not perfect.
            // When you change the font list in the Doc, you can track
            // changes here only on the Listbox, because ppFontList
            // has already been updated.
            bChanged =
                ( ( *ppFontList != pNewFontList ) ||
                  pBox->GetListCount() != pNewFontList->GetFontNameCount() );
            // HACK: Comparing is incomplete

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

    // Fill the FontBox, also the new list if necessary
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
    aLogicalSize       ( 60,160 ),
    nFtCount           ( 0 ),
    bRelease           ( true ),
    m_xDispatchProvider( rDispatchProvider ),
    m_xFrame (_xFrame),
    mbEndPreview(false)
{
    SetSizePixel(LogicToPixel( aLogicalSize, MAP_APPFONT ));
    EnableControls_Impl();
    GetSubEdit()->AddEventListener( LINK( this, SvxFontNameBox_Impl, CheckAndMarkUnknownFont ));
}

SvxFontNameBox_Impl::~SvxFontNameBox_Impl()
{
    GetSubEdit()->RemoveEventListener( LINK( this, SvxFontNameBox_Impl, CheckAndMarkUnknownFont ));
}

// -----------------------------------------------------------------------

void SvxFontNameBox_Impl::FillList()
{
    // Save old Selection, set back in the end
    Selection aOldSel = GetSelection();
    // Did Doc-Fontlist change?
    lcl_GetDocFontList( &pFontList, this );
    aCurText = GetText();
    SetSelection( aOldSel );
}

IMPL_LINK( SvxFontNameBox_Impl, CheckAndMarkUnknownFont, VclWindowEvent*, event )
{
    if( event->GetId() != VCLEVENT_EDIT_MODIFY )
        return 0;
    OUString fontname = GetSubEdit()->GetText();
    lcl_GetDocFontList( &pFontList, this );
    // If the font is unknown, show it in italic.
    Font font = GetControlFont();
    if( pFontList != NULL && pFontList->IsAvailable( fontname ))
    {
        if( font.GetItalic() != ITALIC_NONE )
        {
            font.SetItalic( ITALIC_NONE );
            SetControlFont( font );
            SetQuickHelpText( SVX_RESSTR( RID_SVXSTR_CHARFONTNAME ));
        }
    }
    else
    {
        if( font.GetItalic() != ITALIC_NORMAL )
        {
            font.SetItalic( ITALIC_NORMAL );
            SetControlFont( font );
            SetQuickHelpText( SVX_RESSTR( RID_SVXSTR_CHARFONTNAME_NOTAVAILABLE ));
        }
    }
    return 0;
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
    OUString aCurName = aCurFont.GetName();
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
    mbEndPreview = false;
    if ( rNEvt.GetType() == EVENT_KEYUP )
        mbEndPreview = true;

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        sal_uInt16 nCode = rNEvt.GetKeyEvent()->GetKeyCode().GetCode();

        switch ( nCode )
        {
            case KEY_RETURN:
            case KEY_TAB:
            {
                if ( KEY_TAB == nCode )
                    bRelease = false;
                else
                    nHandled = 1;
                Select();
                break;
            }

            case KEY_ESCAPE:
                SetText( aCurText );
                ReleaseFocus_Impl();
                EndPreview();
                break;
        }
    }
    else if ( EVENT_LOSEFOCUS == rNEvt.GetType() )
    {
        Window* pFocusWin = Application::GetFocusWindow();
        if ( !HasFocus() && GetSubEdit() != pFocusWin )
            SetText( GetSavedValue() );
        // send EndPreview
        EndPreview();
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
        bRelease = true;
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
}

// -----------------------------------------------------------------------

void SvxFontNameBox_Impl::UserDraw( const UserDrawEvent& rUDEvt )
{
    FontNameBox::UserDraw( rUDEvt );

    // Hack - GetStyle now contains the currently
    // selected item in the list box
    // ItemId contains the id of the current item to draw
    // or select
    if (  rUDEvt.GetItemId() == rUDEvt.GetStyle() )
    {
        Sequence< PropertyValue > aArgs( 1 );
        FontInfo aInfo( pFontList->Get( GetEntry( rUDEvt.GetItemId() ),
            aCurFont.GetWeight(),
            aCurFont.GetItalic() ) );

        SvxFontItem aFontItem( aInfo.GetFamily(),
            aInfo.GetName(),
            aInfo.GetStyleName(),
            aInfo.GetPitch(),
            aInfo.GetCharSet(),
            SID_ATTR_CHAR_FONT );
        aFontItem.QueryValue( aArgs[0].Value );
        aArgs[0].Name   = OUString( "CharPreviewFontName" );
        SfxToolBoxControl::Dispatch( m_xDispatchProvider,
            OUString( ".uno:CharPreviewFontName" ),
                aArgs );
    }
}

void SvxFontNameBox_Impl::Select()
{
    FontNameBox::Select();

    Sequence< PropertyValue > aArgs( 1 );
    std::auto_ptr<SvxFontItem> pFontItem;
    if ( pFontList )
    {
        FontInfo aInfo( pFontList->Get( GetText(),
            aCurFont.GetWeight(),
            aCurFont.GetItalic() ) );
        aCurFont = aInfo;

        pFontItem.reset( new SvxFontItem( aInfo.GetFamily(),
            aInfo.GetName(),
            aInfo.GetStyleName(),
            aInfo.GetPitch(),
            aInfo.GetCharSet(),
            SID_ATTR_CHAR_FONT ) );

        Any a;
        pFontItem->QueryValue( a );
        aArgs[0].Value  = a;
    }
    if ( !IsTravelSelect() )
    {
        //  #i33380# DR 2004-09-03 Moved the following line above the Dispatch() call.
        //  This instance may be deleted in the meantime (i.e. when a dialog is opened
        //  while in Dispatch()), accessing members will crash in this case.
        ReleaseFocus_Impl();
        EndPreview();
        if ( pFontItem.get() )
        {
            aArgs[0].Name   = OUString( "CharFontName" );
            SfxToolBoxControl::Dispatch( m_xDispatchProvider,
                                         OUString( ".uno:CharFontName" ),
                                         aArgs );
        }
    }
    else
    {
        if ( mbEndPreview )
        {
            EndPreview();
            return;
        }
        if ( pFontItem.get() )
        {
            aArgs[0].Name   = OUString( "CharPreviewFontName" );
            SfxToolBoxControl::Dispatch( m_xDispatchProvider,
                                         OUString( ".uno:CharPreviewFontName" ),
                                         aArgs );
        }
    }
}

//========================================================================
// class SvxColorWindow_Impl --------------------------------------------------
//========================================================================
#ifndef WB_NO_DIRECTSELECT
#define WB_NO_DIRECTSELECT      ((WinBits)0x04000000)
#endif

SvxColorWindow_Impl::SvxColorWindow_Impl( const OUString&            rCommand,
                                          sal_uInt16                 nSlotId,
                                          const Reference< XFrame >& rFrame,
                                          const OUString&            rWndTitle,
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
                                     OUString( ".uno:AutoColorInvalid" ));
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
        const long nColorCount(pColorList->Count());
        const Size aNewSize(aColorSet.layoutAllVisible(nColorCount));
        aColorSet.SetOutputSizePixel(aNewSize);
        static sal_Int32 nAdd = 4;

        SetOutputSizePixel(Size(aNewSize.Width() + nAdd, aNewSize.Height() + nAdd));
        aColorSet.Clear();
        aColorSet.addEntriesForXColorList(*pColorList);
    }

    aColorSet.SetSelectHdl( LINK( this, SvxColorWindow_Impl, SelectHdl ) );
    SetHelpId( HID_POPUP_COLOR );
    aColorSet.SetHelpId( HID_POPUP_COLOR_CTRL );
    SetText( rWndTitle );
    aColorSet.Show();

    AddStatusListener( OUString( ".uno:ColorTableState" ));
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
                const long nColorCount(pColorList->Count());
                const Size aNewSize(aColorSet.layoutAllVisible(nColorCount));
                aColorSet.SetOutputSizePixel(aNewSize);
                static sal_Int32 nAdd = 4;

                SetOutputSizePixel(Size(aNewSize.Width() + nAdd, aNewSize.Height() + nAdd));
                aColorSet.Clear();
                aColorSet.addEntriesForXColorList(*pColorList);
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
    AddStatusListener(OUString(".uno:BorderReducedMode"));
    aImgList = ImageList( SVX_RES( RID_SVXIL_FRAME ) );

    /*
     *  1       2        3         4
     *  -------------------------------------
     *  NONE    LEFT     RIGHT     LEFTRIGHT
     *  TOP     BOTTOM   TOPBOTTOM OUTER
     *  -------------------------------------
     *  HOR     HORINNER VERINNER   ALL         <- can be switched of via bParagraphMode
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
    //! HACK: How do I get the Paragraph mode?
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

// By default unset lines remain unchanged.
// Via Shift unset lines are reset

IMPL_LINK_NOARG(SvxFrameWindow_Impl, SelectHdl)
{
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

        // Inner Table:
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
                bool bTableMode = ( aFrameSet.GetItemCount() == 12 );
                bool bResize    = false;

                if ( bTableMode && bParagraphMode )
                {
                    for ( sal_uInt16 i = 9; i < 13; i++ )
                        aFrameSet.RemoveItem(i);
                    bResize = true;
                }
                else if ( !bTableMode && !bParagraphMode )
                {
                    for ( sal_uInt16 i = 9; i < 13; i++ )
                        aFrameSet.InsertItem( i, aImgList.GetImage(i) );
                    bResize = true;
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
static Color lcl_mediumColor( Color aMain, Color /*aDefault*/ )
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
        m_bIsWriter = xServices->supportsService("com.sun.star.text.TextDocument");
    }
    catch(const uno::Exception& )
    {
    }

    m_aLineStyleLb.setPosSizePixel( 2, 2, 110, 140 );
    SetOutputSizePixel( Size( 114, 144 ) );

    m_aLineStyleLb.SetSourceUnit( FUNIT_TWIP );
    m_aLineStyleLb.SetNone( SVX_RESSTR(RID_SVXSTR_NONE) );

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
    const OUString&                  rCommand,     // .uno: command bound to this item
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
    OUString                     aClearForm;
    OUString                     aMore;
    ::std::vector< OUString >    aDefaultStyles;
    sal_Bool                     bSpecModeWriter;
    sal_Bool                     bSpecModeCalc;

    inline Impl( void )
        :aClearForm         ( SVX_RESSTR( RID_SVXSTR_CLEARFORM ) )
        ,aMore              ( SVX_RESSTR( RID_SVXSTR_MORE ) )
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
            bSpecModeWriter = xServices->supportsService("com.sun.star.text.TextDocument");
            if(bSpecModeWriter)
            {
                Reference<container::XNameAccess> xParaStyles;
                    xStylesSupplier->getStyleFamilies()->getByName("ParagraphStyles") >>=
                    xParaStyles;
                static const sal_Char* aWriterStyles[] =
                {
                    "Text body",
                    "Quotations",
                    "Title",
                    "Subtitle",
                    "Heading 1",
                    "Heading 2",
                    "Heading 3"
                };
                for( sal_uInt32 nStyle = 0; nStyle < sizeof( aWriterStyles ) / sizeof( sal_Char*); ++nStyle )
                {
                    try
                    {
                        Reference< beans::XPropertySet > xStyle;
                        xParaStyles->getByName( OUString::createFromAscii( aWriterStyles[nStyle] )) >>= xStyle;
                        OUString sName;
                        xStyle->getPropertyValue("DisplayName") >>= sName;
                        if( !sName.isEmpty() )
                            aDefaultStyles.push_back(sName);
                    }
                    catch( const uno::Exception& )
                    {}
                }

            }
            else if( 0 != (
                bSpecModeCalc = xServices->supportsService(OUString(
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
                        OUString("CellStyles")) >>=
                        xCellStyles;
                for( sal_uInt32 nStyle = 0; nStyle < sizeof( aCalcStyles ) / sizeof( sal_Char*); ++nStyle )
                {
                    try
                    {
                        const OUString sStyleName( OUString::createFromAscii( aCalcStyles[nStyle] ) );
                        if( xCellStyles->hasByName( sStyleName ) )
                        {
                            Reference< beans::XPropertySet > xStyle( xCellStyles->getByName( sStyleName), UNO_QUERY_THROW );
                            OUString sName;
                            xStyle->getPropertyValue("DisplayName") >>= sName;
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
        m_xBoundItems[i].clear();
        pFamilyState[i]  = NULL;
    }
}

// -----------------------------------------------------------------------
SvxStyleToolBoxControl::~SvxStyleToolBoxControl()
{
    delete pImpl;
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
        bool                    bDoFill     = false;

        pStyleSheetPool->SetSearchMask( eFamily, SFXSTYLEBIT_USED );

        // Check whether fill is necessary
        pStyle = pStyleSheetPool->First();
        //!!! TODO: This condition isn't right any longer, because we always show some default entries
        //!!! so the list doesn't show the count
        if ( nCount != pBox->GetEntryCount() )
        {
            bDoFill = true;
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
                        OUString aName( pStyle->GetName() );
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
                // disable sort to preserve special order
                WinBits nWinBits = pBox->GetStyle();
                nWinBits &= ~WB_SORT;
                pBox->SetStyle( nWinBits );

                // insert default styles
                sal_uInt16  _i;
                sal_uInt32  nCnt = pImpl->aDefaultStyles.size();
                sal_uInt16 nPos = 1;
                for( _i = 0 ; _i < nCnt ; ++_i )
                {
                    pBox->InsertEntry( pImpl->aDefaultStyles[_i], nPos );
                    ++nPos;
                }

                pBox->InsertEntry( pImpl->aClearForm, 0 );
                pBox->SetSeparatorPos( 0 );

                pBox->InsertEntry( pImpl->aMore );

                // enable sort again
                nWinBits |= WB_SORT;
                pBox->SetStyle( nWinBits );
            }

            pBox->SetUpdateMode( sal_True );
            pBox->SetFamily( eFamily );

            sal_uInt16 nLines = std::min( pBox->GetEntryCount(), MAX_STYLES_ENTRIES );
            pBox->SetDropDownLineCount( nLines );
        }
    }
}

// -----------------------------------------------------------------------

void SvxStyleToolBoxControl::SelectStyle( const OUString& rStyleName )
{
    SvxStyleBox_Impl* pBox = (SvxStyleBox_Impl*)GetToolBox().GetItemWindow( GetId() );
    DBG_ASSERT( pBox, "Control not found!" );

    if ( pBox )
    {
        OUString aStrSel( pBox->GetText() );

        if ( !rStyleName.isEmpty() )
        {
            if ( rStyleName != aStrSel )
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
    // Current range not within allowed ranges or default
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

    FillStyleBox(); // Decides by itself whether Fill is needed

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
            pBox->SetText( "" );
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
                    nSlotId, nId, &GetToolBox() )),
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
    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
    bChoiceFromPalette = sal_False;

    // The following commands are available at the various modules
    switch( nSlotId )
    {
        case SID_ATTR_CHAR_COLOR:
            addStatusListener( OUString( ".uno:Color" ));
            mLastColor = COL_RED;
            break;

        case SID_ATTR_CHAR_COLOR2:
            addStatusListener( OUString( ".uno:CharColorExt" ));
            mLastColor = COL_RED;
            break;

        case SID_BACKGROUND_COLOR:
        case SID_ATTR_CHAR_COLOR_BACKGROUND:
        default:
            addStatusListener( OUString( ".uno:CharBackgroundExt" ));
            mLastColor = COL_YELLOW;
            break;

        case SID_FRAME_LINECOLOR:
            addStatusListener( OUString( ".uno:FrameLineColor" ));
            mLastColor = COL_BLUE;
            break;
    }

    pBtnUpdater = new ::svx::ToolboxButtonColorUpdater( nSlotId, nId, &GetToolBox() );
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
    if ( bChoiceFromPalette && nSID == GetSlotId() )
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
    bool bNoArgs = false;

    switch( GetSlotId() )
    {
        case SID_ATTR_CHAR_COLOR2 :
            bNoArgs     = true;
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
            bNoArgs     = true;
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
        OUString aNewText( MnemonicGenerator::EraseAllMnemonicChars( pItem->GetValue() ) );
        rBox.SetQuickHelpText( GetId(), aNewText );
    }
    if ( eState == SFX_ITEM_DISABLED )
        rBox.SetQuickHelpText( GetId(), aDefaultText );
    rBox.EnableItem( GetId(), eState != SFX_ITEM_DISABLED );
}

//========================================================================

static void lcl_ResizeValueSet( Window &rWin, ValueSet &rValueSet )
{
    Size aSize = rWin.GetOutputSizePixel();
    aSize.Width()  -= 4;
    aSize.Height() -= 4;
    rValueSet.SetPosSizePixel( Point(2,2), aSize );
}

// -----------------------------------------------------------------------

static void lcl_CalcSizeValueSet( Window &rWin, ValueSet &rValueSet, const Size &aItemSize )
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

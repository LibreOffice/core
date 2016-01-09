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
#include <utility>

#include <tools/color.hxx>
#include <svl/poolitem.hxx>
#include <svl/eitem.hxx>
#include <svl/itemset.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/menubtn.hxx>
#include <svtools/valueset.hxx>
#include <svtools/ctrlbox.hxx>
#include <svl/style.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/borderhelper.hxx>
#include <svl/stritem.hxx>
#include <sfx2/tplpitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/imagemgr.hxx>
#include <sfx2/viewsh.hxx>
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
#include <vcl/settings.hxx>
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
#include <sfx2/sidebar/Sidebar.hxx>
#include <sfx2/sidebar/SidebarToolBox.hxx>
#include <svx/xtable.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
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
#include <editeng/svxfont.hxx>
#include <editeng/cmapitem.hxx>
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
#include <svx/xlnclit.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>

#define MAX_MRU_FONTNAME_ENTRIES    5

// don't make more than 15 entries visible at once
#define MAX_STYLES_ENTRIES          15

static void lcl_CalcSizeValueSet( vcl::Window &rWin, ValueSet &rValueSet, const Size &aItemSize );

// namespaces
using namespace ::editeng;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

SFX_IMPL_TOOLBOX_CONTROL( SvxStyleToolBoxControl, SfxTemplateItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxFontNameToolBoxControl, SvxFontItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxFrameToolBoxControl, SvxBoxItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxFrameLineStyleToolBoxControl, SvxLineItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxSimpleUndoRedoController, SfxStringItem );

class SvxStyleBox_Impl : public ComboBox
{
    using Window::IsVisible;
public:
    SvxStyleBox_Impl( vcl::Window* pParent, const OUString& rCommand, SfxStyleFamily eFamily, const Reference< XDispatchProvider >& rDispatchProvider,
                        const Reference< XFrame >& _xFrame,const OUString& rClearFormatKey, const OUString& rMoreKey, bool bInSpecialMode );
    virtual ~SvxStyleBox_Impl();
    virtual void dispose() override;

    void            SetFamily( SfxStyleFamily eNewFamily );
    bool            IsVisible() const { return bVisible; }

    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool    Notify( NotifyEvent& rNEvt ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual void    StateChanged( StateChangedType nStateChange ) override;

    virtual void    UserDraw( const UserDrawEvent& rUDEvt ) override;

    void            SetVisibilityListener( const Link<SvxStyleBox_Impl&,void>& aVisListener ) { aVisibilityListener = aVisListener; }

    void            SetDefaultStyle( const OUString& rDefault ) { sDefaultStyle = rDefault; }

protected:
    /// Calculate the optimal width of the dropdown.  Very expensive operation, triggers lots of font measurement.
    DECL_DLLPRIVATE_LINK_TYPED(CalcOptimalExtraUserWidth, VclWindowEvent&, void);

    virtual void    Select() override;

private:
    SfxStyleFamily                  eStyleFamily;
    sal_Int32                       nCurSel;
    bool                            bRelease;
    Size                            aLogicalSize;
    Link<SvxStyleBox_Impl&,void>    aVisibilityListener;
    bool                            bVisible;
    Reference< XDispatchProvider >  m_xDispatchProvider;
    Reference< XFrame >             m_xFrame;
    OUString                        m_aCommand;
    OUString                        aClearFormatKey;
    OUString                        aMoreKey;
    OUString                        sDefaultStyle;
    bool                            bInSpecialMode;
    VclPtr<MenuButton>              m_pButtons[MAX_STYLES_ENTRIES];
    PopupMenu                       m_aMenu;

    void            ReleaseFocus();
    static Color    TestColorsVisible(const Color &FontCol, const Color &BackCol);
    static void     UserDrawEntry(const UserDrawEvent& rUDEvt, const OUString &rStyleName);
    void            SetupEntry(vcl::RenderContext& rRenderContext, vcl::Window* pParent, sal_Int32 nItem, const Rectangle& rRect, const OUString& rStyleName, bool bIsNotSelected);
    static bool     AdjustFontForItemHeight(OutputDevice* pDevice, Rectangle& rTextRect, long nHeight);
    void            SetOptimalSize();
    DECL_LINK_TYPED( MenuSelectHdl, Menu *, bool );
};

class SvxFontNameBox_Impl : public FontNameBox
{
    using Window::Update;
private:
    const FontList*                pFontList;
    ::std::unique_ptr<FontList>    m_aOwnFontList;
    vcl::Font                      aCurFont;
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
                                         ".uno:CharEndPreviewFontName",
                                         aArgs );
    }
    DECL_DLLPRIVATE_LINK_TYPED( CheckAndMarkUnknownFont, VclWindowEvent&, void );

    void            SetOptimalSize();

protected:
    virtual void    Select() override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

public:
    SvxFontNameBox_Impl( vcl::Window* pParent, const Reference< XDispatchProvider >& rDispatchProvider,const Reference< XFrame >& _xFrame
        , WinBits nStyle = WB_SORT
        );
    virtual ~SvxFontNameBox_Impl();
    virtual void dispose() override;

    void            FillList();
    void            Update( const SvxFontItem* pFontItem );
    sal_uInt16      GetListCount() { return nFtCount; }
    void            Clear() { FontNameBox::Clear(); nFtCount = 0; }
    void            Fill( const FontList* pList )
                        { FontNameBox::Fill( pList );
                          nFtCount = pList->GetFontNameCount(); }
    virtual void    UserDraw( const UserDrawEvent& rUDEvt ) override;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool    Notify( NotifyEvent& rNEvt ) override;
    virtual Reference< css::accessibility::XAccessible > CreateAccessible() override;
    void     SetOwnFontList(::std::unique_ptr<FontList> && _aOwnFontList) { m_aOwnFontList = std::move(_aOwnFontList); }
};

// SelectHdl needs the Modifiers, get them in MouseButtonUp
class SvxFrmValueSet_Impl : public ValueSet
{
    sal_uInt16          nModifier;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
public:
    SvxFrmValueSet_Impl(vcl::Window* pParent,  WinBits nWinStyle)
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
    VclPtr<SvxFrmValueSet_Impl> aFrameSet;
    ImageList                   aImgList;
    bool                        bParagraphMode;

    DECL_LINK_TYPED( SelectHdl, ValueSet*, void );

protected:
    virtual void    Resize() override;
    virtual bool    Close() override;
    virtual vcl::Window* GetPreferredKeyInputWindow() override;
    virtual void    GetFocus() override;

public:
    SvxFrameWindow_Impl( sal_uInt16 nId, const Reference< XFrame >& rFrame, vcl::Window* pParentWindow );
    virtual ~SvxFrameWindow_Impl();
    virtual void dispose() override;

    void            StartSelection();

    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                  const SfxPoolItem* pState ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
};

class SvxLineWindow_Impl : public SfxPopupWindow
{
private:
    VclPtr<LineListBox> m_aLineStyleLb;
    bool                m_bIsWriter;

    DECL_LINK_TYPED( SelectHdl, ListBox&, void );

protected:
    virtual void    Resize() override;
    virtual bool    Close() override;
    virtual vcl::Window* GetPreferredKeyInputWindow() override;
    virtual void    GetFocus() override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
public:
    SvxLineWindow_Impl( sal_uInt16 nId, const Reference< XFrame >& rFrame, vcl::Window* pParentWindow );
    virtual ~SvxLineWindow_Impl() { disposeOnce(); }
    virtual void dispose() override { m_aLineStyleLb.disposeAndClear(); SfxPopupWindow::dispose(); }
};

class SvxStyleToolBoxControl;

class SfxStyleControllerItem_Impl : public SfxStatusListener
{
    public:
        SfxStyleControllerItem_Impl( const Reference< XDispatchProvider >& rDispatchProvider,
                                     sal_uInt16 nSlotId,
                                     const OUString& rCommand,
                                     SvxStyleToolBoxControl& rTbxCtl );

    protected:
        virtual void StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState ) override;

    private:
        SvxStyleToolBoxControl& rControl;
};

#define BUTTON_WIDTH 20
#define BUTTON_PADDING 10
#define ITEM_HEIGHT 30

SvxStyleBox_Impl::SvxStyleBox_Impl(vcl::Window* pParent,
                                   const OUString& rCommand,
                                   SfxStyleFamily eFamily,
                                   const Reference< XDispatchProvider >& rDispatchProvider,
                                   const Reference< XFrame >& _xFrame,
                                   const OUString& rClearFormatKey,
                                   const OUString& rMoreKey,
                                   bool bInSpec)
    : ComboBox( pParent, SVX_RES( RID_SVXTBX_STYLE ) )
    , eStyleFamily( eFamily )
    , nCurSel(0)
    , bRelease( true )
    , bVisible(false)
    , m_xDispatchProvider( rDispatchProvider )
    , m_xFrame(_xFrame)
    , m_aCommand( rCommand )
    , aClearFormatKey( rClearFormatKey )
    , aMoreKey( rMoreKey )
    , bInSpecialMode( bInSpec )
    , m_aMenu ( SVX_RES( RID_SVX_STYLE_MENU ) )
{
    m_aMenu.SetSelectHdl( LINK( this, SvxStyleBox_Impl, MenuSelectHdl ) );
    for(int i = 0; i < MAX_STYLES_ENTRIES; i++)
        m_pButtons[i] = nullptr;
    aLogicalSize = PixelToLogic( GetSizePixel(), MAP_APPFONT );
    SetOptimalSize();
    EnableAutocomplete( true );
    EnableUserDraw( true );
    AddEventListener(LINK(this, SvxStyleBox_Impl, CalcOptimalExtraUserWidth));
    SetUserItemSize( Size( 0, ITEM_HEIGHT ) );
}

SvxStyleBox_Impl::~SvxStyleBox_Impl()
{
    disposeOnce();
}

void SvxStyleBox_Impl::dispose()
{
    RemoveEventListener(LINK(this, SvxStyleBox_Impl, CalcOptimalExtraUserWidth));

    for (int i = 0; i < MAX_STYLES_ENTRIES; i++)
    {
        m_pButtons[i].disposeAndClear();
    }

    ComboBox::dispose();
}

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

IMPL_LINK_TYPED( SvxStyleBox_Impl, MenuSelectHdl, Menu*, pMenu, bool)
{
    OUString sEntry = OUString( GetSelectEntry() );
    ReleaseFocus(); // It must be after getting entry pos!
    Sequence< PropertyValue > aArgs( 2 );
    aArgs[0].Name   = "Param";
    aArgs[0].Value  = makeAny( sEntry );
    aArgs[1].Name   = "Family";
    aArgs[1].Value  = makeAny( sal_Int16( eStyleFamily ));

    sal_uInt16 nMenuId = pMenu->GetCurItemId();
    switch(nMenuId) {
        case RID_SVX_UPDATE_STYLE:
        {
            SfxToolBoxControl::Dispatch( m_xDispatchProvider,
                ".uno:StyleUpdateByExample", aArgs );
            break;
        }
        case RID_SVX_MODIFY_STYLE:
        {
            SfxToolBoxControl::Dispatch( m_xDispatchProvider,
                ".uno:EditStyle", aArgs );
            break;
        }
    }
    return false;
}

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
                SfxToolBoxControl::Dispatch( m_xDispatchProvider, ".uno:ResetAttributes",
                    aEmptyVals);
            }
            else if( aSearchEntry == aMoreKey && GetSelectEntryPos() == ( GetEntryCount() - 1 ) )
            {
                SfxViewFrame* pViewFrm = SfxViewFrame::Current();
                DBG_ASSERT( pViewFrm, "SvxStyleBox_Impl::Select(): no viewframe" );
                pViewFrm->ShowChildWindow( SID_SIDEBAR );
                ::sfx2::sidebar::Sidebar::ShowPanel("StyleListPanel",
                                                    pViewFrm->GetFrame().GetFrameInterface());
                bDoIt = false;
            }
        }

        //Do we need to create a new style?
        SfxObjectShell *pShell = SfxObjectShell::Current();
        SfxStyleSheetBasePool* pPool = pShell->GetStyleSheetPool();
        SfxStyleSheetBase* pStyle = nullptr;

        bool bCreateNew = false;

        if ( pPool )
        {
            pPool->SetSearchMask( eStyleFamily );

            pStyle = pPool->First();
            while ( pStyle && OUString( pStyle->GetName() ) != aSearchEntry )
                pStyle = pPool->Next();
        }

        if ( !pStyle )
        {
            // cannot find the style for whatever reason
            // therefore create a new style
            bCreateNew = true;
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
            aArgs[1].Name   = "Family";
            aArgs[1].Value  = makeAny( sal_Int16( eStyleFamily ));
            if( bCreateNew )
            {
                aArgs[0].Name   = "Param";
                SfxToolBoxControl::Dispatch( m_xDispatchProvider, ".uno:StyleNewByExample", aArgs);
            }
            else
            {
                aArgs[0].Name   = "Template";
                SfxToolBoxControl::Dispatch( m_xDispatchProvider, m_aCommand, aArgs );
            }
        }
    }
}

void SvxStyleBox_Impl::SetFamily( SfxStyleFamily eNewFamily )
{
    eStyleFamily = eNewFamily;
}

bool SvxStyleBox_Impl::PreNotify( NotifyEvent& rNEvt )
{
    MouseNotifyEvent nType = rNEvt.GetType();

    if ( MouseNotifyEvent::MOUSEBUTTONDOWN == nType || MouseNotifyEvent::GETFOCUS == nType )
        nCurSel = GetSelectEntryPos();
    else if ( MouseNotifyEvent::LOSEFOCUS == nType )
    {
        // don't handle before our Select() is called
        if ( !HasFocus() && !HasChildPathFocus() )
            SetText( GetSavedValue() );
    }
    return ComboBox::PreNotify( rNEvt );
}

bool SvxStyleBox_Impl::Notify( NotifyEvent& rNEvt )
{
    bool bHandled = false;

    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        sal_uInt16 nCode = rNEvt.GetKeyEvent()->GetKeyCode().GetCode();

        switch ( nCode )
        {
            case KEY_CONTEXTMENU:
            {
                if(IsInDropDown())
                {
                    const sal_Int32 nItem = GetSelectEntryPos() - 1;
                    if(nItem < MAX_STYLES_ENTRIES)
                        m_pButtons[nItem]->ExecuteMenu();
                    bHandled = true;
                }
                break;
            }
            case KEY_RETURN:
            case KEY_TAB:
            {
                if ( KEY_TAB == nCode )
                    bRelease = false;
                else
                    bHandled = true;
                Select();
                break;
            }

            case KEY_ESCAPE:
                SelectEntryPos( nCurSel );
                ReleaseFocus();
                bHandled = true;
                break;
        }
    }
    return bHandled || ComboBox::Notify( rNEvt );
}

void SvxStyleBox_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        SetOptimalSize();
    }

    ComboBox::DataChanged( rDCEvt );
}

void SvxStyleBox_Impl::StateChanged( StateChangedType nStateChange )
{
    ComboBox::StateChanged( nStateChange );

    if ( nStateChange == StateChangedType::Visible )
    {
        bVisible = IsReallyVisible();
        aVisibilityListener.Call( *this );
    }
    else if ( nStateChange == StateChangedType::InitShow )
    {
        bVisible = true;
        aVisibilityListener.Call( *this );
    }
}

bool SvxStyleBox_Impl::AdjustFontForItemHeight(OutputDevice* pDevice, Rectangle& rTextRect, long nHeight)
{
    if (rTextRect.Bottom() > nHeight)
    {
        // the text does not fit, adjust the font size
        double ratio = static_cast< double >( nHeight ) / rTextRect.Bottom();
        vcl::Font aFont(pDevice->GetFont());
        Size aPixelSize(aFont.GetSize());
        aPixelSize.Width() *= ratio;
        aPixelSize.Height() *= ratio;
        aFont.SetSize(aPixelSize);
        pDevice->SetFont(aFont);
        return true;
    }
    return false;
}

void SvxStyleBox_Impl::SetOptimalSize()
{
    Size aSize(LogicToPixel(aLogicalSize, MAP_APPFONT));
    set_width_request(aSize.Width());
    set_height_request(aSize.Height());
    SetSizePixel(aSize);
}

void SvxStyleBox_Impl::UserDrawEntry(const UserDrawEvent& rUDEvt, const OUString &rStyleName)
{
    vcl::RenderContext *pDevice = rUDEvt.GetRenderContext();

    // IMG_TXT_DISTANCE in ilstbox.hxx is 6, then 1 is added as
    // nBorder, and we are adding 1 in order to look better when
    // italics is present
    const int nLeftDistance = 8;

    Rectangle aTextRect;
    pDevice->GetTextBoundRect(aTextRect, rStyleName);

    Point aPos( rUDEvt.GetRect().TopLeft() );
    aPos.X() += nLeftDistance;

    if (!AdjustFontForItemHeight(pDevice, aTextRect, rUDEvt.GetRect().GetHeight()))
        aPos.Y() += ( rUDEvt.GetRect().GetHeight() - aTextRect.Bottom() ) / 2;

    pDevice->DrawText(aPos, rStyleName);
}

void SvxStyleBox_Impl::SetupEntry(vcl::RenderContext& rRenderContext, vcl::Window* pParent, sal_Int32 nItem, const Rectangle& rRect, const OUString& rStyleName, bool bIsNotSelected)
{
    unsigned int nId = rRect.GetHeight() != 0 ? (rRect.getY() / rRect.GetHeight()) : MAX_STYLES_ENTRIES;
    if (nItem == 0 || nItem == GetEntryCount() - 1)
    {
        if(nId < MAX_STYLES_ENTRIES && m_pButtons[nId])
            m_pButtons[nId]->Hide();
    }
    else
    {
        SfxObjectShell *pShell = SfxObjectShell::Current();
        SfxStyleSheetBasePool* pPool = pShell->GetStyleSheetPool();
        SfxStyleSheetBase* pStyle = nullptr;

        if ( pPool )
        {
            pPool->SetSearchMask( eStyleFamily );

            pStyle = pPool->First();
            while (pStyle && pStyle->GetName() != rStyleName)
                pStyle = pPool->Next();
        }

        if (pStyle )
        {
            std::unique_ptr<const SfxItemSet> const pItemSet(pStyle->GetItemSetForPreview());
            if (!pItemSet) return;

            const SvxFontItem * const pFontItem =
                static_cast<const SvxFontItem*>(pItemSet->GetItem(SID_ATTR_CHAR_FONT));
            const SvxFontHeightItem * const pFontHeightItem =
                static_cast<const SvxFontHeightItem*>(pItemSet->GetItem(SID_ATTR_CHAR_FONTHEIGHT));

            if ( pFontItem && pFontHeightItem )
            {
                Size aFontSize( 0, pFontHeightItem->GetHeight() );
                Size aPixelSize(rRenderContext.LogicToPixel(aFontSize, pShell->GetMapUnit()));

                // setup the font properties
                SvxFont aFont;
                aFont.SetName(pFontItem->GetFamilyName());
                aFont.SetStyleName(pFontItem->GetStyleName());
                aFont.SetSize(aPixelSize);

                const SfxPoolItem *pItem = pItemSet->GetItem( SID_ATTR_CHAR_WEIGHT );
                if ( pItem )
                    aFont.SetWeight( static_cast< const SvxWeightItem* >( pItem )->GetWeight() );

                pItem = pItemSet->GetItem( SID_ATTR_CHAR_POSTURE );
                if ( pItem )
                    aFont.SetItalic( static_cast< const SvxPostureItem* >( pItem )->GetPosture() );

                pItem = pItemSet->GetItem( SID_ATTR_CHAR_CONTOUR );
                if ( pItem )
                    aFont.SetOutline( static_cast< const SvxContourItem* >( pItem )->GetValue() );

                pItem = pItemSet->GetItem( SID_ATTR_CHAR_SHADOWED );
                if ( pItem )
                    aFont.SetShadow( static_cast< const SvxShadowedItem* >( pItem )->GetValue() );

                pItem = pItemSet->GetItem( SID_ATTR_CHAR_RELIEF );
                if ( pItem )
                    aFont.SetRelief( static_cast< FontRelief >( static_cast< const SvxCharReliefItem* >( pItem )->GetValue() ) );

                pItem = pItemSet->GetItem( SID_ATTR_CHAR_UNDERLINE );
                if ( pItem )
                    aFont.SetUnderline( static_cast< const SvxUnderlineItem* >( pItem )->GetLineStyle() );

                pItem = pItemSet->GetItem( SID_ATTR_CHAR_OVERLINE );
                if ( pItem )
                    aFont.SetOverline( static_cast< FontUnderline >( static_cast< const SvxOverlineItem* >( pItem )->GetValue() ) );

                pItem = pItemSet->GetItem( SID_ATTR_CHAR_STRIKEOUT );
                if ( pItem )
                    aFont.SetStrikeout( static_cast< const SvxCrossedOutItem* >( pItem )->GetStrikeout() );

                pItem = pItemSet->GetItem( SID_ATTR_CHAR_CASEMAP );
                if ( pItem )
                    aFont.SetCaseMap(static_cast<const SvxCaseMapItem*>(pItem)->GetCaseMap());

                pItem = pItemSet->GetItem( SID_ATTR_CHAR_EMPHASISMARK );
                if ( pItem )
                    aFont.SetEmphasisMark( static_cast< const SvxEmphasisMarkItem* >( pItem )->GetEmphasisMark() );

                // setup the device & draw
                vcl::Font aOldFont(rRenderContext.GetFont());

                Color aFontCol = COL_AUTO, aBackCol = COL_AUTO;

                rRenderContext.SetFont(aFont);

                pItem = pItemSet->GetItem( SID_ATTR_CHAR_COLOR );
                // text color, when nothing is selected
                if ( (nullptr != pItem) && bIsNotSelected)
                    aFontCol = Color( static_cast< const SvxColorItem* >( pItem )->GetValue() );

                sal_uInt16 style = drawing::FillStyle_NONE;
                // which kind of Fill style is selected
                pItem = pItemSet->GetItem( XATTR_FILLSTYLE );
                // only when ok and not selected
                if ( (nullptr != pItem) && bIsNotSelected)
                    style = static_cast< const XFillStyleItem* >( pItem )->GetValue();

                switch(style)
                {
                    case drawing::FillStyle_SOLID:
                    {
                        // set background color
                        pItem = pItemSet->GetItem( XATTR_FILLCOLOR );
                        if ( nullptr != pItem )
                            aBackCol = Color( static_cast< const XFillColorItem* >( pItem )->GetColorValue() );

                        if ( aBackCol != COL_AUTO )
                        {
                            rRenderContext.SetFillColor(aBackCol);
                            rRenderContext.DrawRect(rRect);
                        }
                    }
                    break;

                    //TODO Draw the other background styles: gradient, hatching and bitmap
                }

                // when the font and background color are too similar, adjust the Font-Color
                if( (aFontCol != COL_AUTO) || (aBackCol != COL_AUTO) )
                    aFontCol = TestColorsVisible(aFontCol, (aBackCol != COL_AUTO) ? aBackCol : rRenderContext.GetBackground().GetColor());

                // set text color
                if ( aFontCol != COL_AUTO )
                    rRenderContext.SetTextColor(aFontCol);

                // handle the push-button
                if (bIsNotSelected)
                {
                    if (nId < MAX_STYLES_ENTRIES && m_pButtons[nId])
                        m_pButtons[nId]->Hide();
                }
                else
                {
                    if (nId < MAX_STYLES_ENTRIES)
                    {
                        if (!m_pButtons[nId] && pParent)
                        {
                            m_pButtons[nId] = VclPtr<MenuButton>::Create(pParent, WB_FLATBUTTON | WB_NOPOINTERFOCUS);
                            m_pButtons[nId]->SetSizePixel(Size(BUTTON_WIDTH, rRect.GetHeight()));
                            m_pButtons[nId]->SetPopupMenu(&m_aMenu);
                        }
                        m_pButtons[nId]->SetPosPixel(Point(rRect.GetWidth() - BUTTON_WIDTH, rRect.getY()));
                        m_pButtons[nId]->Show();
                    }
                }
            }
        }
    }
}

void SvxStyleBox_Impl::UserDraw( const UserDrawEvent& rUDEvt )
{
    sal_uInt16 nItem = rUDEvt.GetItemId();
    OUString aStyleName( GetEntry( nItem ) );

    vcl::RenderContext *pDevice = rUDEvt.GetRenderContext();
    pDevice->Push(PushFlags::FILLCOLOR | PushFlags::FONT | PushFlags::TEXTCOLOR);

    const Rectangle& rRect(rUDEvt.GetRect());
    bool bIsNotSelected = rUDEvt.GetItemId() != GetSelectEntryPos();

    SetupEntry(*pDevice, rUDEvt.GetWindow(), nItem, rRect, aStyleName, bIsNotSelected);

    UserDrawEntry(rUDEvt, aStyleName);

    pDevice->Pop();
    // draw separator, if present
    DrawEntry( rUDEvt, false, false );
}

IMPL_LINK_TYPED(SvxStyleBox_Impl, CalcOptimalExtraUserWidth, VclWindowEvent&, event, void)
{
    // perform the calculation only when we are opening the dropdown
    if (event.GetId() != VCLEVENT_DROPDOWN_PRE_OPEN)
        return;

    long nMaxNormalFontWidth = 0;
    sal_Int32 nEntryCount = GetEntryCount();
    for (sal_Int32 i = 0; i < nEntryCount; ++i)
    {
        OUString sStyleName(GetEntry(i));
        Rectangle aTextRectForDefaultFont;
        GetTextBoundRect(aTextRectForDefaultFont, sStyleName);

        const long nWidth = aTextRectForDefaultFont.GetWidth();

        nMaxNormalFontWidth = std::max(nWidth, nMaxNormalFontWidth);
    }

    long nMaxUserDrawFontWidth = nMaxNormalFontWidth;
    for (sal_Int32 i = 1; i < nEntryCount-1; ++i)
    {
        OUString sStyleName(GetEntry(i));

        Push(PushFlags::FILLCOLOR | PushFlags::FONT | PushFlags::TEXTCOLOR);
        SetupEntry(*this /*FIXME rendercontext*/, this, i, Rectangle(0, 0, RECT_MAX, ITEM_HEIGHT), sStyleName, true);
        Rectangle aTextRectForActualFont;
        GetTextBoundRect(aTextRectForActualFont, sStyleName);
        if (AdjustFontForItemHeight(this, aTextRectForActualFont, ITEM_HEIGHT))
        {
            //Font didn't fit, so it was changed, refetch with final font size
            GetTextBoundRect(aTextRectForActualFont, sStyleName);
        }
        Pop();

        const long nWidth = aTextRectForActualFont.GetWidth() + BUTTON_WIDTH + BUTTON_PADDING;

        nMaxUserDrawFontWidth = std::max(nWidth, nMaxUserDrawFontWidth);
    }

    SetUserItemSize(Size(nMaxUserDrawFontWidth - nMaxNormalFontWidth, ITEM_HEIGHT));
}

// test is the color between Font- and background-color to be identify
// return is always the Font-Color
//        when both light or dark, change the Contrast
//        in other case do not change the origin color
//        when the color is R=G=B=128 the DecreaseContrast make 128 the need a exception
Color SvxStyleBox_Impl::TestColorsVisible(const Color &FontCol, const Color &BackCol)
{
    const sal_uInt8  ChgVal = 60;       // increase/decrease the Contrast

    Color  retCol = FontCol;
    if ((FontCol.IsDark() == BackCol.IsDark()) && (FontCol.IsBright() == BackCol.IsBright()))
    {
        sal_uInt8 lumi = retCol.GetLuminance();

        if((lumi > 120) && (lumi < 140))
            retCol.DecreaseLuminance(ChgVal / 2);
        else
            retCol.DecreaseContrast(ChgVal);
    }

    return retCol;
}



static bool lcl_GetDocFontList( const FontList** ppFontList, SvxFontNameBox_Impl* pBox )
{
    bool bChanged = false;
    const SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SvxFontListItem* pFontListItem = nullptr;

    if ( pDocSh )
        pFontListItem =
            static_cast<const SvxFontListItem*>(pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST ));
    else
    {
        ::std::unique_ptr<FontList> aFontList(new FontList( pBox->GetParent() ));
        *ppFontList = aFontList.get();
        pBox->SetOwnFontList(std::move(aFontList));
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
    else if ( pBox && ( pDocSh || !ppFontList ))
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

SvxFontNameBox_Impl::SvxFontNameBox_Impl( vcl::Window* pParent, const Reference< XDispatchProvider >& rDispatchProvider,const Reference< XFrame >& _xFrame, WinBits nStyle ) :

    FontNameBox        ( pParent, nStyle | WinBits( WB_DROPDOWN | WB_AUTOHSCROLL ) ),
    pFontList          ( nullptr ),
    aLogicalSize       ( 60,160 ),
    nFtCount           ( 0 ),
    bRelease           ( true ),
    m_xDispatchProvider( rDispatchProvider ),
    m_xFrame (_xFrame),
    mbEndPreview(false)
{
    SetOptimalSize();
    EnableControls_Impl();
    GetSubEdit()->AddEventListener( LINK( this, SvxFontNameBox_Impl, CheckAndMarkUnknownFont ));
}

SvxFontNameBox_Impl::~SvxFontNameBox_Impl()
{
    disposeOnce();
}

void SvxFontNameBox_Impl::dispose()
{
    GetSubEdit()->RemoveEventListener( LINK( this, SvxFontNameBox_Impl, CheckAndMarkUnknownFont ));
    FontNameBox::dispose();
}

void SvxFontNameBox_Impl::FillList()
{
    // Save old Selection, set back in the end
    Selection aOldSel = GetSelection();
    // Did Doc-Fontlist change?
    lcl_GetDocFontList( &pFontList, this );
    aCurText = GetText();
    SetSelection( aOldSel );
}

IMPL_LINK_TYPED( SvxFontNameBox_Impl, CheckAndMarkUnknownFont, VclWindowEvent&, event, void )
{
    if( event.GetId() != VCLEVENT_EDIT_MODIFY )
        return;
    OUString fontname = GetSubEdit()->GetText();
    lcl_GetDocFontList( &pFontList, this );
    // If the font is unknown, show it in italic.
    vcl::Font font = GetControlFont();
    if( pFontList != nullptr && pFontList->IsAvailable( fontname ))
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
}

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

bool SvxFontNameBox_Impl::PreNotify( NotifyEvent& rNEvt )
{
    MouseNotifyEvent nType = rNEvt.GetType();

    if ( MouseNotifyEvent::MOUSEBUTTONDOWN == nType || MouseNotifyEvent::GETFOCUS == nType )
    {
        EnableControls_Impl();
        FillList();
    }
    return FontNameBox::PreNotify( rNEvt );
}

bool SvxFontNameBox_Impl::Notify( NotifyEvent& rNEvt )
{
    bool bHandled = false;
    mbEndPreview = false;
    if ( rNEvt.GetType() == MouseNotifyEvent::KEYUP )
        mbEndPreview = true;

    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
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
                    bHandled = true;
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
    else if ( MouseNotifyEvent::LOSEFOCUS == rNEvt.GetType() )
    {
        vcl::Window* pFocusWin = Application::GetFocusWindow();
        if ( !HasFocus() && GetSubEdit() != pFocusWin )
            SetText( GetSavedValue() );
        // send EndPreview
        EndPreview();
    }

    return bHandled || FontNameBox::Notify( rNEvt );
}

void SvxFontNameBox_Impl::SetOptimalSize()
{
    Size aSize(LogicToPixel(aLogicalSize, MAP_APPFONT));
    set_width_request(aSize.Width());
    set_height_request(aSize.Height());
    SetSizePixel(aSize);
}

void SvxFontNameBox_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        SetOptimalSize();
    }
    else if ( ( rDCEvt.GetType() == DataChangedEventType::FONTS ) ||
              ( rDCEvt.GetType() == DataChangedEventType::DISPLAY ) )
    {
        // The old font list in shell has likely been destroyed at this point, so we need to get
        // the new one before doing anything further.
        lcl_GetDocFontList( &pFontList, this );
    }

    FontNameBox::DataChanged( rDCEvt );
}

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

void SvxFontNameBox_Impl::EnableControls_Impl()
{
    SvtFontOptions aFontOpt;
    bool bEnable = aFontOpt.IsFontHistoryEnabled();
    sal_uInt16 nEntries = bEnable ? MAX_MRU_FONTNAME_ENTRIES : 0;
    if ( GetMaxMRUCount() != nEntries )
    {
        // refill in the next GetFocus-Handler
        pFontList = nullptr;
        Clear();
        SetMaxMRUCount( nEntries );
    }

    bEnable = aFontOpt.IsFontWYSIWYGEnabled();
    EnableWYSIWYG( bEnable );
}

void SvxFontNameBox_Impl::UserDraw( const UserDrawEvent& rUDEvt )
{
    FontNameBox::UserDraw( rUDEvt );

    // Hack - GetStyle now contains the currently
    // selected item in the list box
    // ItemId contains the id of the current item to draw
    // or select
    if (  rUDEvt.GetItemId() == rUDEvt.GetStyle() )
    {
        OUString fontName(GetText());
        if (IsInDropDown())
        {
            /*
             * when in dropdown mode the selected item should be
             * used and not the current selection
             */
             fontName = GetEntry(rUDEvt.GetItemId());
        }
        Sequence< PropertyValue > aArgs( 1 );
        FontMetric aFontMetric( pFontList->Get( fontName,
            aCurFont.GetWeight(),
            aCurFont.GetItalic() ) );

        SvxFontItem aFontItem( aFontMetric.GetFamily(),
            aFontMetric.GetName(),
            aFontMetric.GetStyleName(),
            aFontMetric.GetPitch(),
            aFontMetric.GetCharSet(),
            SID_ATTR_CHAR_FONT );
        aFontItem.QueryValue( aArgs[0].Value );
        aArgs[0].Name   = "CharPreviewFontName";
        SfxToolBoxControl::Dispatch( m_xDispatchProvider,
            ".uno:CharPreviewFontName",
                aArgs );
    }
}

void SvxFontNameBox_Impl::Select()
{
    FontNameBox::Select();

    Sequence< PropertyValue > aArgs( 1 );
    std::unique_ptr<SvxFontItem> pFontItem;
    if ( pFontList )
    {
        FontMetric aFontMetric( pFontList->Get( GetText(),
            aCurFont.GetWeight(),
            aCurFont.GetItalic() ) );
        aCurFont = aFontMetric;

        pFontItem.reset( new SvxFontItem( aFontMetric.GetFamily(),
            aFontMetric.GetName(),
            aFontMetric.GetStyleName(),
            aFontMetric.GetPitch(),
            aFontMetric.GetCharSet(),
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
            aArgs[0].Name   = "CharFontName";
            SfxToolBoxControl::Dispatch( m_xDispatchProvider,
                                         ".uno:CharFontName",
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
            aArgs[0].Name   = "CharPreviewFontName";
            SfxToolBoxControl::Dispatch( m_xDispatchProvider,
                                         ".uno:CharPreviewFontName",
                                         aArgs );
        }
    }
}

#ifndef WB_NO_DIRECTSELECT
#define WB_NO_DIRECTSELECT      ((WinBits)0x04000000)
#endif


SvxColorWindow_Impl::SvxColorWindow_Impl( const OUString&            rCommand,
                                          PaletteManager&            rPaletteManager,
                                          BorderColorStatus&         rBorderColorStatus,
                                          sal_uInt16                 nSlotId,
                                          const Reference< XFrame >& rFrame,
                                          const OUString&            rWndTitle,
                                          vcl::Window*                    pParentWindow,
                                          std::function<void(const OUString&, const Color&)> aFunction):

    SfxPopupWindow( nSlotId, pParentWindow,
                    "palette_popup_window", "svx/ui/colorwindow.ui",
                    rFrame ),
    theSlotId( nSlotId ),
    maCommand( rCommand ),
    mrPaletteManager( rPaletteManager ),
    mrBorderColorStatus( rBorderColorStatus ),
    maColorSelectFunction(aFunction)
{
    get(mpPaletteListBox,     "palette_listbox");
    get(mpButtonAutoColor,    "auto_color_button");
    get(mpButtonPicker,       "color_picker_button");
    get(mpColorSet,           "colorset");
    get(mpRecentColorSet,     "recent_colorset");
    get(mpAutomaticSeparator, "separator4");

    mpColorSet->SetStyle( WinBits(WB_FLATVALUESET | WB_ITEMBORDER | WB_3DLOOK | WB_NO_DIRECTSELECT | WB_TABSTOP) );
    mpRecentColorSet->SetStyle( WinBits(WB_FLATVALUESET | WB_ITEMBORDER | WB_3DLOOK | WB_NO_DIRECTSELECT | WB_TABSTOP) );

    switch ( theSlotId )
    {
        case SID_ATTR_CHAR_COLOR_BACKGROUND:
        case SID_BACKGROUND_COLOR:
        case SID_ATTR_CHAR_BACK_COLOR:
        {
            mpButtonAutoColor->SetText( SVX_RESSTR( RID_SVXSTR_TRANSPARENT ) );
            mpColorSet->SetAccessibleName( SVX_RESSTR( RID_SVXSTR_BACKGROUND ) );
            break;
        }
        case SID_ATTR_CHAR_COLOR:
        case SID_ATTR_CHAR_COLOR2:
        case SID_EXTRUSION_3D_COLOR:
        {
            SfxPoolItem* pDummy;

            Reference< XDispatchProvider > aDisp( GetFrame()->getController(), UNO_QUERY );
            SfxQueryStatus aQueryStatus( aDisp,
                                         SID_ATTR_AUTO_COLOR_INVALID,
                                         OUString( ".uno:AutoColorInvalid" ));
            SfxItemState eState = aQueryStatus.QueryState( pDummy );
            if( (SfxItemState::DEFAULT > eState) || ( SID_EXTRUSION_3D_COLOR == theSlotId ) )
            {
                mpButtonAutoColor->SetText( SVX_RESSTR( RID_SVXSTR_AUTOMATIC ) );
                mpColorSet->SetAccessibleName( SVX_RESSTR( RID_SVXSTR_TEXTCOLOR ) );
            }
            break;
        }
        case SID_FRAME_LINECOLOR:
        {
            mpButtonAutoColor->Hide();
            mpAutomaticSeparator->Hide();
            mpColorSet->SetAccessibleName( SVX_RESSTR( RID_SVXSTR_FRAME_COLOR ) );
            break;
        }
        case SID_ATTR_LINE_COLOR:
        {
            mpButtonAutoColor->Hide();
            mpAutomaticSeparator->Hide();
            mpColorSet->SetAccessibleName( SVX_RESSTR( RID_SVXSTR_LINECOLOR ) );
            break;
        }
        case SID_ATTR_FILL_COLOR:
        {
            mpButtonAutoColor->Hide();
            mpAutomaticSeparator->Hide();
            mpColorSet->SetAccessibleName( SVX_RESSTR( RID_SVXSTR_FILLCOLOR ) );
            break;
        }
    }

    mpPaletteListBox->SetStyle( mpPaletteListBox->GetStyle() | WB_BORDER | WB_AUTOSIZE );
    mpPaletteListBox->SetSelectHdl( LINK( this, SvxColorWindow_Impl, SelectPaletteHdl ) );
    mpPaletteListBox->AdaptDropDownLineCountToMaximum();
    std::vector<OUString> aPaletteList = mrPaletteManager.GetPaletteList();
    for( std::vector<OUString>::iterator it = aPaletteList.begin(); it != aPaletteList.end(); ++it )
    {
        mpPaletteListBox->InsertEntry( *it );
    }
    mpPaletteListBox->SelectEntryPos(mrPaletteManager.GetPalette());

    mpButtonAutoColor->SetClickHdl( LINK( this, SvxColorWindow_Impl, AutoColorClickHdl ) );
    mpButtonPicker->SetClickHdl( LINK( this, SvxColorWindow_Impl, OpenPickerClickHdl ) );

    mpColorSet->SetSelectHdl( LINK( this, SvxColorWindow_Impl, SelectHdl ) );
    mpRecentColorSet->SetSelectHdl( LINK( this, SvxColorWindow_Impl, SelectHdl ) );
    SetHelpId( HID_POPUP_COLOR );
    mpColorSet->SetHelpId( HID_POPUP_COLOR_CTRL );
    SetText( rWndTitle );

    mrPaletteManager.ReloadColorSet(*mpColorSet);
    mpColorSet->layoutToGivenHeight(mpColorSet->GetSizePixel().Height(), mrPaletteManager.GetColorCount());

    mrPaletteManager.ReloadRecentColorSet(*mpRecentColorSet);
    mpRecentColorSet->SetLineCount( 1 );
    Size aSize = mpRecentColorSet->layoutAllVisible(mrPaletteManager.GetRecentColorCount());
    mpRecentColorSet->set_height_request(aSize.Height());
    mpRecentColorSet->set_width_request(aSize.Width());

    AddStatusListener( ".uno:ColorTableState" );
    AddStatusListener( maCommand );
    if ( maCommand == ".uno:FrameLineColor" )
    {
        AddStatusListener( ".uno:BorderTLBR" );
        AddStatusListener( ".uno:BorderBLTR" );
    }
}

SvxColorWindow_Impl::~SvxColorWindow_Impl()
{
    disposeOnce();
}

void SvxColorWindow_Impl::dispose()
{
    mpColorSet.clear();
    mpRecentColorSet.clear();
    mpPaletteListBox.clear();
    mpButtonAutoColor.clear();
    mpButtonPicker.clear();
    mpAutomaticSeparator.clear();
    SfxPopupWindow::dispose();
}

void SvxColorWindow_Impl::KeyInput( const KeyEvent& rKEvt )
{
    mpColorSet->KeyInput(rKEvt);
}

IMPL_LINK_TYPED(SvxColorWindow_Impl, SelectHdl, ValueSet*, pColorSet, void)
{
    Color aColor = pColorSet->GetItemColor( pColorSet->GetSelectItemId() );
    /*  #i33380# DR 2004-09-03 Moved the following line above the Dispatch() calls.
        This instance may be deleted in the meantime (i.e. when a dialog is opened
        while in Dispatch()), accessing members will crash in this case. */
    pColorSet->SetNoSelection();

    if ( pColorSet != mpRecentColorSet )
    {
         mrPaletteManager.AddRecentColor( aColor );
         if ( !IsInPopupMode() )
            mrPaletteManager.ReloadRecentColorSet( *mpRecentColorSet );
    }

    if ( IsInPopupMode() )
        EndPopupMode();

    maSelectedLink.Call(aColor);

    maColorSelectFunction(maCommand, aColor);
}

IMPL_LINK_NOARG_TYPED(SvxColorWindow_Impl, SelectPaletteHdl, ListBox&, void)
{
    sal_Int32 nPos = mpPaletteListBox->GetSelectEntryPos();
    mrPaletteManager.SetPalette( nPos );
    mrPaletteManager.ReloadColorSet(*mpColorSet);
    mpColorSet->layoutToGivenHeight(mpColorSet->GetSizePixel().Height(), mrPaletteManager.GetColorCount());
}

IMPL_LINK_NOARG_TYPED(SvxColorWindow_Impl, AutoColorClickHdl, Button*, void)
{
    Color aColor;
    switch ( theSlotId )
    {
        case SID_ATTR_CHAR_COLOR_BACKGROUND:
        case SID_BACKGROUND_COLOR:
        case SID_ATTR_CHAR_BACK_COLOR:
        {
            aColor = COL_TRANSPARENT;
            break;
        }
        case SID_ATTR_CHAR_COLOR:
        case SID_ATTR_CHAR_COLOR2:
        case SID_EXTRUSION_3D_COLOR:
        {
            aColor = COL_AUTO;
            break;
        }
    }

    mpRecentColorSet->SetNoSelection();

    if ( IsInPopupMode() )
        EndPopupMode();

    maSelectedLink.Call(aColor);

    maColorSelectFunction(maCommand, aColor);
}

IMPL_LINK_NOARG_TYPED(SvxColorWindow_Impl, OpenPickerClickHdl, Button*, void)
{
    if ( IsInPopupMode() )
        EndPopupMode();
    mrPaletteManager.PopupColorPicker(maCommand);
}

void SvxColorWindow_Impl::Resize()
{
}

void SvxColorWindow_Impl::StartSelection()
{
    mpColorSet->StartSelection();
}

bool SvxColorWindow_Impl::Close()
{
    return SfxPopupWindow::Close();
}

void SvxColorWindow_Impl::StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    if ( nSID == SID_COLOR_TABLE )
    {
        if ( SfxItemState::DEFAULT == eState && mrPaletteManager.GetPalette() == 0 )
        {
            mrPaletteManager.ReloadColorSet(*mpColorSet);
            mpColorSet->layoutToGivenHeight(mpColorSet->GetSizePixel().Height(), mrPaletteManager.GetColorCount());
        }
    }
    else
    {
        mpColorSet->SetNoSelection();
        Color aColor( COL_TRANSPARENT );

        if ( nSID == SID_FRAME_LINECOLOR
          || nSID == SID_ATTR_BORDER_DIAG_TLBR
          || nSID == SID_ATTR_BORDER_DIAG_BLTR )
        {
            mrBorderColorStatus.StateChanged( nSID, eState, pState );
            aColor = mrBorderColorStatus.GetColor();
        }
        else if ( SfxItemState::DEFAULT <= eState && pState )
        {
            if ( dynamic_cast<const SvxColorItem*>( pState) !=  nullptr )
                aColor = static_cast<const SvxColorItem*>(pState)->GetValue();
            else if ( dynamic_cast<const XLineColorItem*>( pState) !=  nullptr )
                aColor = static_cast<const XLineColorItem*>(pState)->GetColorValue();
            else if ( dynamic_cast<const XFillColorItem*>( pState) !=  nullptr )
                aColor = static_cast<const XFillColorItem*>(pState)->GetColorValue();
            else if ( dynamic_cast<const SvxBackgroundColorItem*>( pState) !=  nullptr )
                aColor = static_cast<const SvxBackgroundColorItem*>(pState)->GetValue();
        }

        if ( aColor == COL_TRANSPARENT )
            return;

        for ( size_t i = 1; i <= mpColorSet->GetItemCount(); ++i )
        {
            if ( aColor == mpColorSet->GetItemColor(i) )
            {
                mpColorSet->SelectItem(i);
                break;
            }
        }
    }
}


BorderColorStatus::BorderColorStatus() :
    maColor( COL_TRANSPARENT ),
    maTLBRColor( COL_TRANSPARENT ),
    maBLTRColor( COL_TRANSPARENT )
{
}

BorderColorStatus::~BorderColorStatus()
{
}

void BorderColorStatus::StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem *pState )
{
    if ( SfxItemState::DEFAULT <= eState && pState )
    {
        if ( nSID == SID_FRAME_LINECOLOR && dynamic_cast<const SvxColorItem*>( pState) !=  nullptr )
        {
            maColor = static_cast< const SvxColorItem* >(pState)->GetValue();
        }
        else if ( dynamic_cast<const SvxLineItem*>( pState) !=  nullptr )
        {
            const SvxBorderLine* pLine = static_cast< const SvxLineItem* >(pState)->GetLine();
            Color aColor ( COL_TRANSPARENT );
            if ( pLine )
                aColor = pLine->GetColor();

            if ( nSID == SID_ATTR_BORDER_DIAG_TLBR )
                maTLBRColor = aColor;
            else if ( nSID == SID_ATTR_BORDER_DIAG_BLTR )
                maBLTRColor = aColor;
        }
    }
    else if ( nSID == SID_FRAME_LINECOLOR )
        maColor = COL_TRANSPARENT;
    else if ( nSID == SID_ATTR_BORDER_DIAG_TLBR )
        maTLBRColor = COL_TRANSPARENT;
    else if ( nSID == SID_ATTR_BORDER_DIAG_BLTR )
        maBLTRColor = COL_TRANSPARENT;
}

Color BorderColorStatus::GetColor()
{
    bool bHasColor = maColor != COL_TRANSPARENT;
    bool bHasTLBRColor = maTLBRColor != COL_TRANSPARENT;
    bool bHasBLTRColor = maBLTRColor != COL_TRANSPARENT;

    if ( !bHasColor && bHasTLBRColor && !bHasBLTRColor )
        return maTLBRColor;
    else if ( !bHasColor && !bHasTLBRColor && bHasBLTRColor )
        return maBLTRColor;
    else if ( bHasColor && bHasTLBRColor && !bHasBLTRColor )
    {
        if ( maColor == maTLBRColor )
            return maColor;
        else
            return maBLTRColor;
    }
    else if ( bHasColor && !bHasTLBRColor && bHasBLTRColor )
    {
        if ( maColor == maBLTRColor )
            return maColor;
        else
            return maTLBRColor;
    }
    else if ( !bHasColor && bHasTLBRColor && bHasBLTRColor )
    {
        if ( maTLBRColor == maBLTRColor )
            return maTLBRColor;
        else
            return maColor;
    }
    else if ( bHasColor && bHasTLBRColor && bHasBLTRColor )
    {
        if ( maColor == maTLBRColor && maColor == maBLTRColor )
            return maColor;
        else
            return COL_TRANSPARENT;
    }
    return maColor;
}


SvxFrameWindow_Impl::SvxFrameWindow_Impl( sal_uInt16 nId, const Reference< XFrame >& rFrame, vcl::Window* pParentWindow ) :
    SfxPopupWindow( nId, rFrame, pParentWindow, WinBits( WB_STDPOPUP | WB_OWNERDRAWDECORATION ) ),
    aFrameSet   ( VclPtr<SvxFrmValueSet_Impl>::Create(this, WinBits( WB_ITEMBORDER | WB_DOUBLEBORDER | WB_3DLOOK | WB_NO_DIRECTSELECT )) ),
    bParagraphMode(false)
{
    BindListener();
    AddStatusListener(".uno:BorderReducedMode");
    aImgList = ImageList( SVX_RES( RID_SVXIL_FRAME ) );

    if( pParentWindow->GetDPIScaleFactor() > 1 )
    {
        for (short i = 0; i < aImgList.GetImageCount(); i++)
        {
            OUString rImageName = aImgList.GetImageName(i);
            BitmapEx b = aImgList.GetImage(rImageName).GetBitmapEx();
            b.Scale(pParentWindow->GetDPIScaleFactor(), pParentWindow->GetDPIScaleFactor());
            aImgList.ReplaceImage(rImageName, Image(b));
        }
    }

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
        aFrameSet->InsertItem( i, aImgList.GetImage(i) );

    //bParagraphMode should have been set in StateChanged
    if ( !bParagraphMode )
        for ( i = 9; i < 13; i++ )
            aFrameSet->InsertItem( i, aImgList.GetImage(i) );

    aFrameSet->SetColCount( 4 );
    aFrameSet->SetSelectHdl( LINK( this, SvxFrameWindow_Impl, SelectHdl ) );

    lcl_CalcSizeValueSet( *this, *aFrameSet.get(), Size( 20 * pParentWindow->GetDPIScaleFactor(), 20 * pParentWindow->GetDPIScaleFactor() ));

    SetHelpId( HID_POPUP_FRAME );
    SetText( SVX_RESSTR(RID_SVXSTR_FRAME) );
    aFrameSet->SetAccessibleName( SVX_RESSTR(RID_SVXSTR_FRAME) );
    aFrameSet->Show();
}

SvxFrameWindow_Impl::~SvxFrameWindow_Impl()
{
    disposeOnce();
}

void SvxFrameWindow_Impl::dispose()
{
    UnbindListener();
    aFrameSet.disposeAndClear();
    SfxPopupWindow::dispose();
}

vcl::Window* SvxFrameWindow_Impl::GetPreferredKeyInputWindow()
{
    return aFrameSet.get();
}

void SvxFrameWindow_Impl::GetFocus()
{
    if (aFrameSet)
        aFrameSet->GrabFocus();
}

void SvxFrameWindow_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxPopupWindow::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DataChangedEventType::SETTINGS ) && ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE ) )
    {
        aImgList = ImageList( SVX_RES( RID_SVXIL_FRAME ) );

        sal_uInt16  nNumOfItems = aFrameSet->GetItemCount();

        for( sal_uInt16 i = 1 ; i <= nNumOfItems ; ++i )
            aFrameSet->SetItemImage( i, aImgList.GetImage( i ) );
    }
}

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

IMPL_LINK_NOARG_TYPED(SvxFrameWindow_Impl, SelectHdl, ValueSet*, void)
{
    SvxBoxItem          aBorderOuter( SID_ATTR_BORDER_OUTER );
    SvxBoxInfoItem      aBorderInner( SID_ATTR_BORDER_INNER );
    SvxBorderLine       theDefLine;
    SvxBorderLine       *pLeft = nullptr,
                        *pRight = nullptr,
                        *pTop = nullptr,
                        *pBottom = nullptr;
    sal_uInt16           nSel = aFrameSet->GetSelectItemId();
    sal_uInt16           nModifier = aFrameSet->GetModifier();
    sal_uInt8            nValidFlags = 0;

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
            aBorderInner.SetLine( &theDefLine, SvxBoxInfoItemLine::HORI );
            aBorderInner.SetLine( nullptr, SvxBoxInfoItemLine::VERT );
            nValidFlags |= FRM_VALID_HINNER|FRM_VALID_TOP|FRM_VALID_BOTTOM;
            break;

        case 10: // HORINNER
            pLeft = pRight = pTop = pBottom = &theDefLine;
            aBorderInner.SetLine( &theDefLine, SvxBoxInfoItemLine::HORI );
            aBorderInner.SetLine( nullptr, SvxBoxInfoItemLine::VERT );
            nValidFlags |= FRM_VALID_RIGHT|FRM_VALID_LEFT|FRM_VALID_HINNER|FRM_VALID_TOP|FRM_VALID_BOTTOM;
            break;

        case 11: // VERINNER
            pLeft = pRight = pTop = pBottom = &theDefLine;
            aBorderInner.SetLine( nullptr, SvxBoxInfoItemLine::HORI );
            aBorderInner.SetLine( &theDefLine, SvxBoxInfoItemLine::VERT );
            nValidFlags |= FRM_VALID_RIGHT|FRM_VALID_LEFT|FRM_VALID_VINNER|FRM_VALID_TOP|FRM_VALID_BOTTOM;
        break;

        case 12: // ALL
            pLeft = pRight = pTop = pBottom = &theDefLine;
            aBorderInner.SetLine( &theDefLine, SvxBoxInfoItemLine::HORI );
            aBorderInner.SetLine( &theDefLine, SvxBoxInfoItemLine::VERT );
            nValidFlags |= FRM_VALID_ALL;
            break;

        default:
        break;
    }
    aBorderOuter.SetLine( pLeft, SvxBoxItemLine::LEFT );
    aBorderOuter.SetLine( pRight, SvxBoxItemLine::RIGHT );
    aBorderOuter.SetLine( pTop, SvxBoxItemLine::TOP );
    aBorderOuter.SetLine( pBottom, SvxBoxItemLine::BOTTOM );

    if(nModifier == KEY_SHIFT)
        nValidFlags |= FRM_VALID_ALL;
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::TOP,       0 != (nValidFlags&FRM_VALID_TOP ));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::BOTTOM,    0 != (nValidFlags&FRM_VALID_BOTTOM ));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::LEFT,      0 != (nValidFlags&FRM_VALID_LEFT));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::RIGHT,     0 != (nValidFlags&FRM_VALID_RIGHT ));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::HORI,      0 != (nValidFlags&FRM_VALID_HINNER ));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::VERT,      0 != (nValidFlags&FRM_VALID_VINNER));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::DISTANCE );
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::DISABLE,   false );

    if ( IsInPopupMode() )
        EndPopupMode();

    Any a;
    Sequence< PropertyValue > aArgs( 2 );
    aArgs[0].Name = "OuterBorder";
    aBorderOuter.QueryValue( a );
    aArgs[0].Value = a;
    aArgs[1].Name = "InnerBorder";
    aBorderInner.QueryValue( a );
    aArgs[1].Value = a;

    /*  #i33380# DR 2004-09-03 Moved the following line above the Dispatch() call.
        This instance may be deleted in the meantime (i.e. when a dialog is opened
        while in Dispatch()), accessing members will crash in this case. */
    aFrameSet->SetNoSelection();

    SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( GetFrame()->getController(), UNO_QUERY ),
                                 ".uno:SetBorderStyle",
                                 aArgs );
}

void SvxFrameWindow_Impl::Resize()
{
    const Size aSize(this->GetOutputSizePixel());
    aFrameSet->SetPosSizePixel(Point(2,2), Size(aSize.Width() - 4, aSize.Height() - 4));
}

void SvxFrameWindow_Impl::StateChanged(
    sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    if ( pState && nSID == SID_BORDER_REDUCED_MODE)
    {
        const SfxBoolItem* pItem = dynamic_cast<const SfxBoolItem*>( pState  );

        if ( pItem )
        {
            bParagraphMode = pItem->GetValue();
            //initial calls mustn't insert or remove elements
            if(aFrameSet->GetItemCount())
            {
                bool bTableMode = ( aFrameSet->GetItemCount() == 12 );
                bool bResize    = false;

                if ( bTableMode && bParagraphMode )
                {
                    for ( sal_uInt16 i = 9; i < 13; i++ )
                        aFrameSet->RemoveItem(i);
                    bResize = true;
                }
                else if ( !bTableMode && !bParagraphMode )
                {
                    for ( sal_uInt16 i = 9; i < 13; i++ )
                        aFrameSet->InsertItem( i, aImgList.GetImage(i) );
                    bResize = true;
                }

                if ( bResize )
                {
                    lcl_CalcSizeValueSet( *this, *aFrameSet.get(), Size( 20, 20 ));
                }
            }
        }
    }
    SfxPopupWindow::StateChanged( nSID, eState, pState );
}

void SvxFrameWindow_Impl::StartSelection()
{
    aFrameSet->StartSelection();
}

bool SvxFrameWindow_Impl::Close()
{
    return SfxPopupWindow::Close();
}

static Color lcl_mediumColor( Color aMain, Color /*aDefault*/ )
{
    return SvxBorderLine::threeDMediumColor( aMain );
}

SvxLineWindow_Impl::SvxLineWindow_Impl( sal_uInt16 nId, const Reference< XFrame >& rFrame, vcl::Window* pParentWindow ) :

    SfxPopupWindow( nId, rFrame, pParentWindow, WinBits( WB_STDPOPUP | WB_OWNERDRAWDECORATION | WB_AUTOSIZE ) ),
    m_aLineStyleLb( VclPtr<LineListBox>::Create(this) )
{
    try
    {
        Reference< lang::XServiceInfo > xServices( rFrame->getController()->getModel(), UNO_QUERY_THROW );
        m_bIsWriter = xServices->supportsService("com.sun.star.text.TextDocument");
    }
    catch(const uno::Exception& )
    {
    }

    m_aLineStyleLb->setPosSizePixel( 2, 2, 110, 140 );
    SetOutputSizePixel( Size( 114, 144 ) );

    m_aLineStyleLb->SetSourceUnit( FUNIT_TWIP );
    m_aLineStyleLb->SetNone( SVX_RESSTR(RID_SVXSTR_NONE) );

    using namespace table::BorderLineStyle;
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SOLID ), SOLID );
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( DOTTED ), DOTTED );
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( DASHED ), DASHED );

    // Double lines
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( DOUBLE ), DOUBLE );
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( THINTHICK_SMALLGAP ), THINTHICK_SMALLGAP, 20 );
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( THINTHICK_MEDIUMGAP ), THINTHICK_MEDIUMGAP );
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( THINTHICK_LARGEGAP ), THINTHICK_LARGEGAP );
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( THICKTHIN_SMALLGAP ), THICKTHIN_SMALLGAP, 20 );
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( THICKTHIN_MEDIUMGAP ), THICKTHIN_MEDIUMGAP );
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( THICKTHIN_LARGEGAP ), THICKTHIN_LARGEGAP );

    // Engraved / Embossed
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( EMBOSSED ), EMBOSSED, 15,
            &SvxBorderLine::threeDLightColor, &SvxBorderLine::threeDDarkColor,
            &lcl_mediumColor );
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( ENGRAVED ), ENGRAVED, 15,
            &SvxBorderLine::threeDDarkColor, &SvxBorderLine::threeDLightColor,
            &lcl_mediumColor );

    // Inset / Outset
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( OUTSET ), OUTSET, 10,
           &SvxBorderLine::lightColor, &SvxBorderLine::darkColor );
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( INSET ), INSET, 10,
           &SvxBorderLine::darkColor, &SvxBorderLine::lightColor );
    m_aLineStyleLb->SetWidth( 20 ); // 1pt by default

    m_aLineStyleLb->SetSelectHdl( LINK( this, SvxLineWindow_Impl, SelectHdl ) );

    SetHelpId( HID_POPUP_LINE );
    SetText( SVX_RESSTR(RID_SVXSTR_FRAME_STYLE) );
    m_aLineStyleLb->Show();
}

IMPL_LINK_NOARG_TYPED(SvxLineWindow_Impl, SelectHdl, ListBox&, void)
{
    SvxLineItem     aLineItem( SID_FRAME_LINESTYLE );
    SvxBorderStyle  nStyle = SvxBorderStyle( m_aLineStyleLb->GetSelectEntryStyle() );

    if ( m_aLineStyleLb->GetSelectEntryPos( ) > 0 )
    {
        SvxBorderLine aTmp;
        aTmp.SetBorderLineStyle( nStyle );
        aTmp.SetWidth( 20 ); // TODO Make it depend on a width field
        aLineItem.SetLine( &aTmp );
    }
    else
        aLineItem.SetLine( nullptr );

    if ( IsInPopupMode() )
        EndPopupMode();

    Any a;
    Sequence< PropertyValue > aArgs( 1 );
    aArgs[0].Name = "LineStyle";
    aLineItem.QueryValue( a, m_bIsWriter ? CONVERT_TWIPS : 0 );
    aArgs[0].Value = a;

    SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( GetFrame()->getController(), UNO_QUERY ),
                                 ".uno:LineStyle",
                                 aArgs );
}

void SvxLineWindow_Impl::Resize()
{
    m_aLineStyleLb->Resize();
}

bool SvxLineWindow_Impl::Close()
{
    return SfxPopupWindow::Close();
}

vcl::Window* SvxLineWindow_Impl::GetPreferredKeyInputWindow()
{
    return m_aLineStyleLb.get();
}

void SvxLineWindow_Impl::GetFocus()
{
    m_aLineStyleLb->GrabFocus();
}

void SvxLineWindow_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxPopupWindow::DataChanged( rDCEvt );
#if 0
    if( ( rDCEvt.GetType() == DataChangedEventType::SETTINGS ) && ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE ) )
    {
        CreateBitmaps();
        Invalidate();
    }
#endif
}

SfxStyleControllerItem_Impl::SfxStyleControllerItem_Impl(
    const Reference< XDispatchProvider >& rDispatchProvider,
    sal_uInt16                                nSlotId,      // Family-ID
    const OUString&                  rCommand,     // .uno: command bound to this item
    SvxStyleToolBoxControl&               rTbxCtl )     // controller instance, which the item is assigned to.
    :   SfxStatusListener( rDispatchProvider, nSlotId, rCommand ),
        rControl( rTbxCtl )
{
}

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

            if ( SfxItemState::DEFAULT == eState )
            {
                const SfxTemplateItem* pStateItem =
                    dynamic_cast<const SfxTemplateItem*>( pState  );
                DBG_ASSERT( pStateItem != nullptr, "SfxTemplateItem expected" );
                rControl.SetFamilyState( nIdx, pStateItem );
            }
            else
                rControl.SetFamilyState( nIdx, nullptr );
            break;
        }
    }
}

struct SvxStyleToolBoxControl::Impl
{
    OUString                     aClearForm;
    OUString                     aMore;
    ::std::vector< OUString >    aDefaultStyles;
    bool                     bSpecModeWriter;
    bool                     bSpecModeCalc;

    inline Impl()
        :aClearForm         ( SVX_RESSTR( RID_SVXSTR_CLEARFORM ) )
        ,aMore              ( SVX_RESSTR( RID_SVXSTR_MORE_STYLES ) )
        ,bSpecModeWriter    ( false )
        ,bSpecModeCalc      ( false )
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
                static const std::vector<OUString> aWriterStyles =
                {
                    "Text body",
                    "Quotations",
                    "Title",
                    "Subtitle",
                    "Heading 1",
                    "Heading 2",
                    "Heading 3"
                };
                for( const OUString& aStyle: aWriterStyles )
                {
                    try
                    {
                        Reference< beans::XPropertySet > xStyle;
                        xParaStyles->getByName( aStyle ) >>= xStyle;
                        OUString sName;
                        xStyle->getPropertyValue("DisplayName") >>= sName;
                        if( !sName.isEmpty() )
                            aDefaultStyles.push_back(sName);
                    }
                    catch( const uno::Exception& )
                    {}
                }

            }
            else if( (
                bSpecModeCalc = xServices->supportsService(
                    "com.sun.star.sheet.SpreadsheetDocument")))
            {
                static const sal_Char* aCalcStyles[] =
                {
                    "Default",
                    "Heading1",
                    "Result",
                    "Result2"
                };
                Reference<container::XNameAccess> xCellStyles;
                xStylesSupplier->getStyleFamilies()->getByName("CellStyles") >>= xCellStyles;
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
        pImpl               ( new Impl ),
        pStyleSheetPool     ( nullptr ),
        nActFamily          ( 0xffff )
{
    for ( sal_uInt16 i=0; i<MAX_FAMILIES; i++ )
    {
        pBoundItems[i] = nullptr;
        m_xBoundItems[i].clear();
        pFamilyState[i]  = nullptr;
    }
}

SvxStyleToolBoxControl::~SvxStyleToolBoxControl()
{
}

void SAL_CALL SvxStyleToolBoxControl::initialize( const Sequence< Any >& aArguments )
throw ( Exception, RuntimeException, std::exception)
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
            m_xBoundItems[i].set( static_cast< OWeakObject* >( pBoundItems[i] ), UNO_QUERY );
            pFamilyState[i]  = nullptr;
        }
    }
}

// XComponent
void SAL_CALL SvxStyleToolBoxControl::dispose()
    throw (css::uno::RuntimeException, std::exception)
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
            pBoundItems[i] = nullptr;
        }
        delete pFamilyState[i];
        pFamilyState[i] = nullptr;
    }
    pStyleSheetPool = nullptr;
    pImpl.reset();
}

void SAL_CALL SvxStyleToolBoxControl::update() throw (RuntimeException, std::exception)
{
    // Do nothing, we will start binding our listener when we are visible.
    // See link SvxStyleToolBoxControl::VisibilityNotification.
    SvxStyleBox_Impl* pBox = static_cast<SvxStyleBox_Impl*>(GetToolBox().GetItemWindow( GetId() ));
    if ( pBox->IsVisible() )
    {
        for ( int i=0; i<MAX_FAMILIES; i++ )
            pBoundItems [i]->ReBind();

        bindListener();
    }
}

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

void SvxStyleToolBoxControl::FillStyleBox()
{
    SvxStyleBox_Impl* pBox = static_cast<SvxStyleBox_Impl*>(GetToolBox().GetItemWindow( GetId() ));

    DBG_ASSERT( pStyleSheetPool, "StyleSheetPool not found!" );
    DBG_ASSERT( pBox,            "Control not found!" );

    if ( pStyleSheetPool && pBox && nActFamily!=0xffff )
    {
        const SfxStyleFamily    eFamily     = GetActFamily();
        sal_uInt16              nCount      = pStyleSheetPool->Count();
        SfxStyleSheetBase*      pStyle      = nullptr;
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
            pBox->SetUpdateMode( false );
            pBox->Clear();

            {
                sal_uInt16  _i;
                sal_uInt32  nCnt = pImpl->aDefaultStyles.size();

                pStyle = pStyleSheetPool->First();

                if( pImpl->bSpecModeWriter || pImpl->bSpecModeCalc )
                {
                    while ( pStyle )
                    {
                        // sort out default styles
                        bool bInsert = true;
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

            pBox->SetUpdateMode( true );
            pBox->SetFamily( eFamily );

            sal_uInt16 nLines = static_cast<sal_uInt16>(
                    std::min( pBox->GetEntryCount(), static_cast<sal_Int32>(MAX_STYLES_ENTRIES)));
            pBox->SetDropDownLineCount( nLines );
        }
    }
}

void SvxStyleToolBoxControl::SelectStyle( const OUString& rStyleName )
{
    SvxStyleBox_Impl* pBox = static_cast<SvxStyleBox_Impl*>(GetToolBox().GetItemWindow( GetId() ));
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

void SvxStyleToolBoxControl::Update()
{
    SfxStyleSheetBasePool*  pPool     = nullptr;
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


    const SfxTemplateItem* pItem = nullptr;

    if ( nActFamily == 0xffff || nullptr == (pItem = pFamilyState[nActFamily-1]) )
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
            SAL_INFO( "svx", "Unknown Family" ); // can happen
        }
    }
    else if ( pPool != pStyleSheetPool )
        pStyleSheetPool = pPool;

    FillStyleBox(); // Decides by itself whether Fill is needed

    if ( pItem )
        SelectStyle( pItem->GetStyleName() );
}

void SvxStyleToolBoxControl::SetFamilyState( sal_uInt16 nIdx,
                                             const SfxTemplateItem* pItem )
{
    delete pFamilyState[nIdx];
    pFamilyState[nIdx] = nullptr;

    if ( pItem )
        pFamilyState[nIdx] = new SfxTemplateItem( *pItem );

    Update();
}

IMPL_LINK_NOARG_TYPED(SvxStyleToolBoxControl, VisibilityNotification, SvxStyleBox_Impl&, void)
{
    // Call ReBind() && UnBind() according to visibility
    SvxStyleBox_Impl* pBox = static_cast<SvxStyleBox_Impl*>( GetToolBox().GetItemWindow( GetId() ));

    if ( pBox && pBox->IsVisible() && !isBound() )
    {
        for ( sal_uInt16 i=0; i<MAX_FAMILIES; i++ )
            pBoundItems [i]->ReBind();

        bindListener();
    }
    else if ( (!pBox || !pBox->IsVisible()) && isBound() )
    {
        for ( sal_uInt16 i=0; i<MAX_FAMILIES; i++ )
            pBoundItems[i]->UnBind();
        unbindListener();
    }
}

void SvxStyleToolBoxControl::StateChanged(
    sal_uInt16 , SfxItemState eState, const SfxPoolItem* pState )
{
    sal_uInt16       nId    = GetId();
    ToolBox&     rTbx   = GetToolBox();
    SvxStyleBox_Impl* pBox   = static_cast<SvxStyleBox_Impl*>(rTbx.GetItemWindow( nId ));
    TriState     eTri   = TRISTATE_FALSE;

    DBG_ASSERT( pBox, "Control not found!" );

    if ( SfxItemState::DISABLED == eState )
        pBox->Disable();
    else
        pBox->Enable();

    rTbx.EnableItem( nId, SfxItemState::DISABLED != eState );

    switch ( eState )
    {
        case SfxItemState::DEFAULT:
            eTri = static_cast<const SfxTemplateItem*>(pState)->GetValue()
                        ? TRISTATE_TRUE
                        : TRISTATE_FALSE;
            break;

        case SfxItemState::DONTCARE:
            eTri = TRISTATE_INDET;
            break;

        default:
            break;
    }

    rTbx.SetItemState( nId, eTri );

    if ( SfxItemState::DISABLED != eState )
        Update();
}

VclPtr<vcl::Window> SvxStyleToolBoxControl::CreateItemWindow( vcl::Window *pParent )
{
    VclPtrInstance<SvxStyleBox_Impl> pBox( pParent,
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

    return pBox.get();
}

SvxFontNameToolBoxControl::SvxFontNameToolBoxControl(
                                            sal_uInt16          nSlotId,
                                            sal_uInt16          nId,
                                            ToolBox&        rTbx )
    :   SfxToolBoxControl( nSlotId, nId, rTbx )
{
}

void SvxFontNameToolBoxControl::StateChanged(
    sal_uInt16 , SfxItemState eState, const SfxPoolItem* pState )
{
    sal_uInt16               nId    = GetId();
    ToolBox&             rTbx   = GetToolBox();
    SvxFontNameBox_Impl* pBox   = static_cast<SvxFontNameBox_Impl*>(rTbx.GetItemWindow( nId ));

    DBG_ASSERT( pBox, "Control not found!" );

    if ( SfxItemState::DISABLED == eState )
    {
        pBox->Disable();
        pBox->Update( nullptr );
    }
    else
    {
        pBox->Enable();

        if ( SfxItemState::DEFAULT == eState )
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

    rTbx.EnableItem( nId, SfxItemState::DISABLED != eState );
}

VclPtr<vcl::Window> SvxFontNameToolBoxControl::CreateItemWindow( vcl::Window *pParent )
{
    VclPtrInstance<SvxFontNameBox_Impl> pBox( pParent,
                                              Reference< XDispatchProvider >( m_xFrame->getController(), UNO_QUERY ),
                                              m_xFrame,0);
    return pBox.get();
}

/* Note:
   The initial color shown on the button is set in /core/svx/source/tbxctrls/tbxcolorupdate.cxx
   (ToolboxButtonColorUpdater::ToolboxButtonColorUpdater()) .
   The initial color used by the button is set in /core/svx/source/tbxcntrls/tbcontrl.cxx
   (SvxColorToolBoxControl::SvxColorToolBoxControl())
   and in case of writer for text(background)color also in /core/sw/source/uibase/docvw/edtwin.cxx
   (SwEditWin::m_aTextBackColor and SwEditWin::m_aTextColor)
 */

SvxColorToolBoxControl::SvxColorToolBoxControl(
    sal_uInt16 nSlotId,
    sal_uInt16 nId,
    ToolBox& rTbx ):
    SfxToolBoxControl( nSlotId, nId, rTbx ),
    maColorSelectFunction(PaletteManager::DispatchColorCommand)
{
    if ( dynamic_cast< sfx2::sidebar::SidebarToolBox* >(&rTbx) )
        bSidebarType = true;
    else
        bSidebarType = false;

    // The following commands are available at the various modules
    switch( nSlotId )
    {
        case SID_ATTR_CHAR_COLOR:
            addStatusListener( ".uno:Color");
            mPaletteManager.SetLastColor( COL_RED );
            bSidebarType = false;
            break;

        case SID_ATTR_CHAR_COLOR2:
            addStatusListener( ".uno:CharColorExt");
            mPaletteManager.SetLastColor( COL_RED );
            bSidebarType = false;
            break;

        case SID_BACKGROUND_COLOR:
            addStatusListener( ".uno:BackgroundColor");
            mPaletteManager.SetLastColor( COL_YELLOW );
            break;

        case SID_ATTR_CHAR_COLOR_BACKGROUND:
            addStatusListener( ".uno:CharBackgroundExt");
            mPaletteManager.SetLastColor( COL_YELLOW );
            bSidebarType = false;
            break;

        case SID_ATTR_CHAR_BACK_COLOR:
            addStatusListener( ".uno:CharBackColor");
            mPaletteManager.SetLastColor( COL_YELLOW );
            break;

        case SID_FRAME_LINECOLOR:
            addStatusListener( ".uno:FrameLineColor");
            addStatusListener( ".uno:BorderTLBR");
            addStatusListener( ".uno:BorderBLTR");
            mPaletteManager.SetLastColor( COL_BLUE );
            break;

        case SID_EXTRUSION_3D_COLOR:
            addStatusListener( ".uno:Extrusion3DColor");
            break;

        case SID_ATTR_LINE_COLOR:
            addStatusListener( ".uno:XLineColor");
            mPaletteManager.SetLastColor( COL_BLACK );
            break;

        case SID_ATTR_FILL_COLOR:
            addStatusListener( ".uno:FillColor");
            mPaletteManager.SetLastColor( COL_DEFAULT_SHAPE_FILLING );
            break;
    }

    if ( bSidebarType )
        rTbx.SetItemBits( nId, ToolBoxItemBits::DROPDOWNONLY | rTbx.GetItemBits( nId ) );
    else
        rTbx.SetItemBits( nId, ToolBoxItemBits::DROPDOWN | rTbx.GetItemBits( nId ) );

    m_xBtnUpdater.reset( new svx::ToolboxButtonColorUpdater( nSlotId, nId, &GetToolBox() ) );
    mPaletteManager.SetBtnUpdater( m_xBtnUpdater.get() );
}

SvxColorToolBoxControl::~SvxColorToolBoxControl()
{
}

void SvxColorToolBoxControl::setColorSelectFunction(ColorSelectFunction aColorSelectFunction)
{
    maColorSelectFunction = aColorSelectFunction;
    mPaletteManager.SetColorSelectFunction(aColorSelectFunction);
}

VclPtr<SfxPopupWindow> SvxColorToolBoxControl::CreatePopupWindow()
{
    SvxColorWindow_Impl* pColorWin =
        VclPtr<SvxColorWindow_Impl>::Create(

                            m_aCommandURL,
                            mPaletteManager,
                            maBorderColorStatus,
                            GetSlotId(),
                            m_xFrame,
                            SVX_RESSTR( RID_SVXITEMS_EXTRAS_CHARCOLOR ),
                            &GetToolBox(),
                            maColorSelectFunction);

    switch( GetSlotId() )
    {
        case SID_ATTR_CHAR_COLOR_BACKGROUND :
        case SID_ATTR_CHAR_BACK_COLOR :
            pColorWin->SetText( SVX_RESSTR( RID_SVXSTR_EXTRAS_CHARBACKGROUND ) );
            break;

        case SID_BACKGROUND_COLOR :
            pColorWin->SetText( SVX_RESSTR( RID_SVXSTR_BACKGROUND ) );
            break;

        case SID_FRAME_LINECOLOR:
            pColorWin->SetText( SVX_RESSTR( RID_SVXSTR_FRAME_COLOR ) );
            break;

        case SID_EXTRUSION_3D_COLOR:
            pColorWin->SetText( SVX_RESSTR( RID_SVXSTR_EXTRUSION_COLOR ) );
            break;

        case SID_ATTR_LINE_COLOR:
            pColorWin->SetText( SVX_RESSTR( RID_SVXSTR_LINECOLOR ) );
            break;

        case SID_ATTR_FILL_COLOR:
            pColorWin->SetText( SVX_RESSTR( RID_SVXSTR_FILLCOLOR ) );
            break;
    }

    pColorWin->StartPopupMode( &GetToolBox(),
        FloatWinPopupFlags::AllowTearOff|FloatWinPopupFlags::NoAppFocusClose );
    pColorWin->StartSelection();
    SetPopupWindow( pColorWin );
    if ( !bSidebarType )
        pColorWin->SetSelectedHdl( LINK( this, SvxColorToolBoxControl, SelectedHdl ) );
    return pColorWin;
}

IMPL_LINK_TYPED(SvxColorToolBoxControl, SelectedHdl, const Color&, rColor, void)
{
    m_xBtnUpdater->Update( rColor );
    mPaletteManager.SetLastColor( rColor );
}

void SvxColorToolBoxControl::StateChanged(
    sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    if ( nSID == SID_ATTR_CHAR_COLOR_EXT || nSID == SID_ATTR_CHAR_COLOR_BACKGROUND_EXT )
        SfxToolBoxControl::StateChanged( nSID, eState, pState );
    else if ( bSidebarType )
    {
        Color aColor( COL_TRANSPARENT );

        if ( nSID == SID_FRAME_LINECOLOR
          || nSID == SID_ATTR_BORDER_DIAG_TLBR
          || nSID == SID_ATTR_BORDER_DIAG_BLTR )
        {
            maBorderColorStatus.StateChanged( nSID, eState, pState );
            aColor = maBorderColorStatus.GetColor();
        }
        else if ( SfxItemState::DEFAULT <= eState && pState )
        {
            if ( dynamic_cast<const SvxColorItem*>( pState) !=  nullptr )
                aColor = static_cast< const SvxColorItem* >(pState)->GetValue();
            else if ( dynamic_cast<const XLineColorItem*>( pState) !=  nullptr )
                aColor = static_cast< const XLineColorItem* >(pState)->GetColorValue();
            else if ( dynamic_cast<const XFillColorItem*>( pState) !=  nullptr )
                aColor = static_cast< const XFillColorItem* >(pState)->GetColorValue();
        }
        m_xBtnUpdater->Update( aColor );
        mPaletteManager.SetLastColor(aColor);
    }
}

void SvxColorToolBoxControl::Select(sal_uInt16 /*nSelectModifier*/)
{
    if ( bSidebarType )
    {
        // Open the popup also when Enter key is pressed.
        css::uno::Reference< css::awt::XWindow > xWin = createPopupWindow();
        if ( xWin.is() )
            xWin->setFocus();
        return;
    }

    OUString aCommand;
    OUString aParamName;

    switch( GetSlotId() )
    {
        case SID_ATTR_CHAR_COLOR2 :
            aCommand    = ".uno:CharColorExt";
            aParamName  = "FontColor";
            break;

        case SID_ATTR_CHAR_COLOR  :
            aCommand    = ".uno:Color";
            aParamName  = "Color";
            break;

        case SID_BACKGROUND_COLOR :
            aCommand    = ".uno:BackgroundColor";
            aParamName  = "BackgroundColor";
            break;

        case SID_ATTR_CHAR_COLOR_BACKGROUND :
            aCommand    = ".uno:CharBackgroundExt";
            aParamName  = "BackColor";
            break;

        case SID_ATTR_CHAR_BACK_COLOR :
            aCommand    = ".uno:CharBackColor";
            aParamName  = "CharBackColor";
            break;

        case SID_FRAME_LINECOLOR  :
            aCommand    = ".uno:FrameLineColor";
            aParamName  = "FrameLineColor";
            break;

        case SID_EXTRUSION_3D_COLOR:
            aCommand    = ".uno:Extrusion3DColor";
            aParamName  = "Extrusion3DColor";
            break;

        case SID_ATTR_LINE_COLOR:
            aCommand    = ".uno:XLineColor";
            aParamName  = "XLineColor";
            break;

        case SID_ATTR_FILL_COLOR:
            aCommand    = ".uno:FillColor";
            aParamName  = "FillColor";
            break;
    }

    Sequence< PropertyValue > aArgs( 1 );
    aArgs[0].Name  = aParamName;
    aArgs[0].Value = makeAny( (sal_uInt32)( mPaletteManager.GetLastColor().GetColor() ));
    Dispatch( aCommand, aArgs );
}

sal_Bool SvxColorToolBoxControl::opensSubToolbar()
    throw (css::uno::RuntimeException, std::exception)
{
    // For a split button (i.e. bSidebarType == false), we mark this controller as
    // a sub-toolbar controller, so we get notified (through updateImage method) on
    // button image changes, and could redraw the last used color on top of it.
    return !bSidebarType;
}

void SvxColorToolBoxControl::updateImage()
    throw (css::uno::RuntimeException, std::exception)
{
    Image aImage = GetImage( m_xFrame, m_aCommandURL, hasBigImages() );
    if ( !!aImage )
    {
        GetToolBox().SetItemImage( GetId(), aImage );
        m_xBtnUpdater->Update( mPaletteManager.GetLastColor(), true );
    }
}

SfxToolBoxControl* SvxColorToolBoxControl::CreateImpl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox &rTbx )
{
    return new SvxColorToolBoxControl( nSlotId, nId, rTbx );
}

void SvxColorToolBoxControl::RegisterControl(sal_uInt16 nSlotId, SfxModule *pMod)
{
    if ( nSlotId == SID_ATTR_LINE_COLOR )
        SfxToolBoxControl::RegisterToolBoxControl( pMod, SfxTbxCtrlFactory( SvxColorToolBoxControl::CreateImpl, typeid(XLineColorItem), nSlotId ) );
    else if ( nSlotId == SID_ATTR_FILL_COLOR )
        SfxToolBoxControl::RegisterToolBoxControl( pMod, SfxTbxCtrlFactory( SvxColorToolBoxControl::CreateImpl, typeid(XFillColorItem), nSlotId ) );
    else if ( nSlotId == SID_ATTR_CHAR_BACK_COLOR )
        SfxToolBoxControl::RegisterToolBoxControl( pMod, SfxTbxCtrlFactory( SvxColorToolBoxControl::CreateImpl, typeid(SvxBackgroundColorItem), nSlotId ) );
    else
        SfxToolBoxControl::RegisterToolBoxControl( pMod, SfxTbxCtrlFactory( SvxColorToolBoxControl::CreateImpl, typeid(SvxColorItem), nSlotId ) );
}

// class SvxFrameToolBoxControl --------------------------------------------

SvxFrameToolBoxControl::SvxFrameToolBoxControl(
    sal_uInt16      nSlotId,
    sal_uInt16      nId,
    ToolBox&    rTbx )
    :   SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, ToolBoxItemBits::DROPDOWNONLY | rTbx.GetItemBits( nId ) );
}

VclPtr<SfxPopupWindow> SvxFrameToolBoxControl::CreatePopupWindow()
{
    VclPtr<SvxFrameWindow_Impl> pFrameWin = VclPtr<SvxFrameWindow_Impl>::Create(
                                        GetSlotId(), m_xFrame, &GetToolBox() );

    pFrameWin->StartPopupMode( &GetToolBox(),
                               FloatWinPopupFlags::GrabFocus |
                               FloatWinPopupFlags::AllowTearOff |
                               FloatWinPopupFlags::NoAppFocusClose );
    pFrameWin->StartSelection();
    SetPopupWindow( pFrameWin );

    return pFrameWin;
}

void SvxFrameToolBoxControl::StateChanged(
    sal_uInt16, SfxItemState eState, const SfxPoolItem*  )
{
    sal_uInt16                  nId     = GetId();
    ToolBox&                rTbx    = GetToolBox();

    rTbx.EnableItem( nId, SfxItemState::DISABLED != eState );
    rTbx.SetItemState( nId, (SfxItemState::DONTCARE == eState)
                            ? TRISTATE_INDET
                            : TRISTATE_FALSE );
}

SvxFrameLineStyleToolBoxControl::SvxFrameLineStyleToolBoxControl(
    sal_uInt16          nSlotId,
    sal_uInt16          nId,
    ToolBox&        rTbx )

    :    SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, ToolBoxItemBits::DROPDOWNONLY | rTbx.GetItemBits( nId ) );
}

VclPtr<SfxPopupWindow> SvxFrameLineStyleToolBoxControl::CreatePopupWindow()
{
    VclPtr<SvxLineWindow_Impl> pLineWin = VclPtr<SvxLineWindow_Impl>::Create( GetSlotId(), m_xFrame, &GetToolBox() );
    pLineWin->StartPopupMode( &GetToolBox(),
                              FloatWinPopupFlags::GrabFocus |
                              FloatWinPopupFlags::AllowTearOff |
                              FloatWinPopupFlags::NoAppFocusClose );
    SetPopupWindow( pLineWin );

    return pLineWin;
}

void SvxFrameLineStyleToolBoxControl::StateChanged(
    sal_uInt16 , SfxItemState eState, const SfxPoolItem*  )
{
    sal_uInt16       nId    = GetId();
    ToolBox&     rTbx   = GetToolBox();

    rTbx.EnableItem( nId, SfxItemState::DISABLED != eState );
    rTbx.SetItemState( nId, (SfxItemState::DONTCARE == eState)
                                ? TRISTATE_INDET
                                : TRISTATE_FALSE );
}

SvxSimpleUndoRedoController::SvxSimpleUndoRedoController( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx  )
    :SfxToolBoxControl( nSlotId, nId, rTbx )
{
    aDefaultText = rTbx.GetItemText( nId );
}

SvxSimpleUndoRedoController::~SvxSimpleUndoRedoController()
{
}

void SvxSimpleUndoRedoController::StateChanged( sal_uInt16, SfxItemState eState, const SfxPoolItem* pState )
{
    const SfxStringItem* pItem = dynamic_cast<const SfxStringItem*>( pState  );
    ToolBox& rBox = GetToolBox();
    if ( pItem && eState != SfxItemState::DISABLED )
    {
        OUString aNewText( MnemonicGenerator::EraseAllMnemonicChars( pItem->GetValue() ) );
        rBox.SetQuickHelpText( GetId(), aNewText );
    }
    if ( eState == SfxItemState::DISABLED )
        rBox.SetQuickHelpText( GetId(), aDefaultText );
    rBox.EnableItem( GetId(), eState != SfxItemState::DISABLED );
}



static void lcl_CalcSizeValueSet( vcl::Window &rWin, ValueSet &rValueSet, const Size &aItemSize )
{
    Size aSize = rValueSet.CalcWindowSizePixel( aItemSize );
    aSize.Width()  += 4;
    aSize.Height() += 4;
    rWin.SetOutputSizePixel( aSize );
}

Reference< css::accessibility::XAccessible > SvxFontNameBox_Impl::CreateAccessible()
{
    FillList();
    return FontNameBox::CreateAccessible();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

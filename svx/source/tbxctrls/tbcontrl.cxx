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
#include <typeinfo>
#include <utility>

#include <comphelper/propertysequence.hxx>
#include <tools/color.hxx>
#include <svl/poolitem.hxx>
#include <svl/eitem.hxx>
#include <svl/itemset.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/vclptr.hxx>
#include <svtools/valueset.hxx>
#include <svtools/ctrlbox.hxx>
#include <svl/style.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/borderhelper.hxx>
#include <svl/stritem.hxx>
#include <sfx2/tplpitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/templdlg.hxx>
#include <svl/isethint.hxx>
#include <sfx2/sfxstatuslistener.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/viewfrm.hxx>
#include <unotools/fontoptions.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <svtools/colorcfg.hxx>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <svx/strings.hrc>
#include <svx/svxitems.hrc>
#include <svx/svxids.hrc>
#include <helpids.h>
#include <sfx2/htmlmode.hxx>
#include <sfx2/sidebar/Sidebar.hxx>
#include <sfx2/sidebar/SidebarToolBox.hxx>
#include <svx/xtable.hxx>
#include <editeng/editids.hrc>
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
#include <svx/colorwindow.hxx>
#include <svx/colorbox.hxx>
#include <svx/drawitem.hxx>
#include <svx/tbcontrl.hxx>
#include <svx/dlgutil.hxx>
#include <svx/dialmgr.hxx>
#include <svx/PaletteManager.hxx>
#include <memory>

#include <svx/framelink.hxx>
#include <svx/tbxcolorupdate.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/editrids.hrc>
#include <svx/xlnclit.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svl/currencytable.hxx>
#include <svtools/langtab.hxx>
#include <cppu/unotype.hxx>
#include <officecfg/Office/Common.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <bitmaps.hlst>

#define MAX_MRU_FONTNAME_ENTRIES    5

// don't make more than 15 entries visible at once
#define MAX_STYLES_ENTRIES          15

// namespaces
using namespace ::editeng;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

SFX_IMPL_TOOLBOX_CONTROL( SvxStyleToolBoxControl, SfxTemplateItem );
SFX_IMPL_TOOLBOX_CONTROL( SvxSimpleUndoRedoController, SfxStringItem );

class SvxStyleBox_Impl : public ComboBox
{
    using Window::IsVisible;
public:
    SvxStyleBox_Impl( vcl::Window* pParent, const OUString& rCommand, SfxStyleFamily eFamily, const Reference< XDispatchProvider >& rDispatchProvider,
                        const Reference< XFrame >& _xFrame,const OUString& rClearFormatKey, const OUString& rMoreKey, bool bInSpecialMode );
    virtual ~SvxStyleBox_Impl() override;
    virtual void dispose() override;

    void            SetFamily( SfxStyleFamily eNewFamily );
    bool            IsVisible() const { return bVisible; }

    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual void    StateChanged( StateChangedType nStateChange ) override;

    virtual void    UserDraw( const UserDrawEvent& rUDEvt ) override;

    void            SetVisibilityListener( const Link<SvxStyleBox_Impl&,void>& aVisListener ) { aVisibilityListener = aVisListener; }

    void            SetDefaultStyle( const OUString& rDefault ) { sDefaultStyle = rDefault; }

protected:
    /// Calculate the optimal width of the dropdown.  Very expensive operation, triggers lots of font measurement.
    DECL_LINK(CalcOptimalExtraUserWidth, VclWindowEvent&, void);

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
    VclBuilder                      m_aBuilder;
    VclPtr<PopupMenu>               m_pMenu;

    void            ReleaseFocus();
    static Color    TestColorsVisible(const Color &FontCol, const Color &BackCol);
    static void     UserDrawEntry(const UserDrawEvent& rUDEvt, const OUString &rStyleName);
    void            SetupEntry(vcl::RenderContext& rRenderContext, vcl::Window* pParent, sal_Int32 nItem, const tools::Rectangle& rRect, const OUString& rStyleName, bool bIsNotSelected);
    static bool     AdjustFontForItemHeight(OutputDevice* pDevice, tools::Rectangle const & rTextRect, long nHeight);
    void            SetOptimalSize();
    DECL_LINK( MenuSelectHdl, Menu *, bool );
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
    bool            mbCheckingUnknownFont;

    void            ReleaseFocus_Impl();
    void            EnableControls_Impl();

    void            EndPreview()
    {
        Sequence< PropertyValue > aArgs;
        SfxToolBoxControl::Dispatch( m_xDispatchProvider,
                                         ".uno:CharEndPreviewFontName",
                                         aArgs );
    }
    DECL_LINK( CheckAndMarkUnknownFont, VclWindowEvent&, void );

    void            SetOptimalSize();

protected:
    virtual void    Select() override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

public:
    SvxFontNameBox_Impl( vcl::Window* pParent, const Reference< XDispatchProvider >& rDispatchProvider,const Reference< XFrame >& _xFrame
        , WinBits nStyle
        );
    virtual ~SvxFontNameBox_Impl() override;
    virtual void dispose() override;

    void            FillList();
    void            Update( const css::awt::FontDescriptor* pFontDesc );
    sal_uInt16      GetListCount() { return nFtCount; }
    void            Clear() { FontNameBox::Clear(); nFtCount = 0; }
    void            Fill( const FontList* pList )
                        { FontNameBox::Fill( pList );
                          nFtCount = pList->GetFontNameCount(); }
    virtual void    UserDraw( const UserDrawEvent& rUDEvt ) override;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;
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

class SvxFrameWindow_Impl : public svtools::ToolbarPopup
{
private:
    VclPtr<SvxFrmValueSet_Impl> aFrameSet;
    svt::ToolboxController&     mrController;
    std::vector<BitmapEx>       aImgVec;
    bool                        bParagraphMode;

    void InitImageList();
    void CalcSizeValueSet();
    DECL_LINK( SelectHdl, ValueSet*, void );

protected:
    virtual void    GetFocus() override;
    virtual void    KeyInput( const KeyEvent& rKEvt ) override;

public:
    SvxFrameWindow_Impl( svt::ToolboxController& rController, vcl::Window* pParentWindow );
    virtual ~SvxFrameWindow_Impl() override;
    virtual void dispose() override;

    virtual void    statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
};

class SvxLineWindow_Impl : public svtools::ToolbarPopup
{
private:
    VclPtr<LineListBox> m_aLineStyleLb;
    svt::ToolboxController& m_rController;
    bool                m_bIsWriter;

    DECL_LINK( SelectHdl, ListBox&, void );

protected:
    virtual void    Resize() override;
    virtual void    GetFocus() override;
public:
    SvxLineWindow_Impl( svt::ToolboxController& rController, vcl::Window* pParentWindow );
    virtual ~SvxLineWindow_Impl() override { disposeOnce(); }
    virtual void dispose() override { m_aLineStyleLb.disposeAndClear(); ToolbarPopup::dispose(); }
};

class SvxCurrencyList_Impl : public svtools::ToolbarPopup
{
private:
    VclPtr<ListBox> m_pCurrencyLb;
    rtl::Reference<SvxCurrencyToolBoxControl> m_xControl;
    OUString&       m_rSelectedFormat;
    LanguageType&   m_eSelectedLanguage;

    std::vector<OUString> m_aFormatEntries;
    LanguageType          m_eFormatLanguage;
    DECL_LINK( SelectHdl, ListBox&, void );

public:
    SvxCurrencyList_Impl( SvxCurrencyToolBoxControl* pControl,
                          vcl::Window* pParentWindow,
                          OUString&     rSelectFormat,
                          LanguageType& eSelectLanguage );
    virtual ~SvxCurrencyList_Impl() override { disposeOnce(); }
    virtual void dispose() override;
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
    : ComboBox(pParent, WB_SORT | WB_BORDER | WB_HIDE | WB_DROPDOWN | WB_AUTOHSCROLL)
    , eStyleFamily( eFamily )
    , nCurSel(0)
    , bRelease( true )
    , aLogicalSize(60, 86)
    , bVisible(false)
    , m_xDispatchProvider( rDispatchProvider )
    , m_xFrame(_xFrame)
    , m_aCommand( rCommand )
    , aClearFormatKey( rClearFormatKey )
    , aMoreKey( rMoreKey )
    , bInSpecialMode( bInSpec )
    , m_aBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "svx/ui/stylemenu.ui", "")
    , m_pMenu(m_aBuilder.get_menu("menu"))
{
    SetHelpId(HID_STYLE_LISTBOX);
    m_pMenu->SetSelectHdl( LINK( this, SvxStyleBox_Impl, MenuSelectHdl ) );
    for(VclPtr<MenuButton> & rpButton : m_pButtons)
        rpButton = nullptr;
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

    for (VclPtr<MenuButton>& rButton : m_pButtons)
    {
        rButton.disposeAndClear();
    }

    m_pMenu.clear();
    m_aBuilder.disposeBuilder();

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

IMPL_LINK( SvxStyleBox_Impl, MenuSelectHdl, Menu*, pMenu, bool)
{
    OUString sEntry = GetSelectedEntry();
    OString sMenuIdent = pMenu->GetCurItemIdent();
    ReleaseFocus(); // It must be after getting entry pos!
    if (IsInDropDown())
        ToggleDropDown();
    Sequence< PropertyValue > aArgs( 2 );
    aArgs[0].Name   = "Param";
    aArgs[0].Value  <<= sEntry;
    aArgs[1].Name   = "Family";
    aArgs[1].Value  <<= sal_Int16( eStyleFamily );

    if (sMenuIdent == "update")
    {
        SfxToolBoxControl::Dispatch( m_xDispatchProvider,
            ".uno:StyleUpdateByExample", aArgs );
    }
    else if (sMenuIdent == "edit")
    {
        SfxToolBoxControl::Dispatch( m_xDispatchProvider,
            ".uno:EditStyle", aArgs );
    }

    return false;
}

void SvxStyleBox_Impl::Select()
{
    // Tell base class about selection so that AT get informed about it.
    ComboBox::Select();

    if ( IsTravelSelect() )
        return;

    OUString aSearchEntry( GetText() );
    bool bDoIt = true, bClear = false;
    if( bInSpecialMode )
    {
        if( aSearchEntry == aClearFormatKey && GetSelectedEntryPos() == 0 )
        {
            aSearchEntry = sDefaultStyle;
            bClear = true;
            //not only apply default style but also call 'ClearFormatting'
            Sequence< PropertyValue > aEmptyVals;
            SfxToolBoxControl::Dispatch( m_xDispatchProvider, ".uno:ResetAttributes",
                aEmptyVals);
        }
        else if( aSearchEntry == aMoreKey && GetSelectedEntryPos() == ( GetEntryCount() - 1 ) )
        {
            SfxViewFrame* pViewFrm = SfxViewFrame::Current();
            DBG_ASSERT( pViewFrm, "SvxStyleBox_Impl::Select(): no viewframe" );
            pViewFrm->ShowChildWindow( SID_SIDEBAR );
            ::sfx2::sidebar::Sidebar::ShowPanel("StyleListPanel",
                                                pViewFrm->GetFrame().GetFrameInterface());
            //tdf#113214 change text back to previous entry
            SetText(GetSavedValue());
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
        while ( pStyle && pStyle->GetName() != aSearchEntry )
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
        aArgs[0].Value  <<= aSearchEntry;
        aArgs[1].Name   = "Family";
        aArgs[1].Value  <<= sal_Int16( eStyleFamily );
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

void SvxStyleBox_Impl::SetFamily( SfxStyleFamily eNewFamily )
{
    eStyleFamily = eNewFamily;
}

bool SvxStyleBox_Impl::PreNotify( NotifyEvent& rNEvt )
{
    MouseNotifyEvent nType = rNEvt.GetType();

    if ( MouseNotifyEvent::MOUSEBUTTONDOWN == nType || MouseNotifyEvent::GETFOCUS == nType )
        nCurSel = GetSelectedEntryPos();
    else if ( MouseNotifyEvent::LOSEFOCUS == nType )
    {
        // don't handle before our Select() is called
        if (!HasFocus() && !HasChildPathFocus() && !IsChild(rNEvt.GetWindow()))
            SetText( GetSavedValue() );
    }
    return ComboBox::PreNotify( rNEvt );
}

bool SvxStyleBox_Impl::EventNotify( NotifyEvent& rNEvt )
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
                    const sal_Int32 nItem = GetSelectedEntryPos() - 1;
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
    return bHandled || ComboBox::EventNotify( rNEvt );
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

bool SvxStyleBox_Impl::AdjustFontForItemHeight(OutputDevice* pDevice, tools::Rectangle const & rTextRect, long nHeight)
{
    if (rTextRect.Bottom() > nHeight)
    {
        // the text does not fit, adjust the font size
        double ratio = static_cast< double >( nHeight ) / rTextRect.Bottom();
        vcl::Font aFont(pDevice->GetFont());
        Size aPixelSize(aFont.GetFontSize());
        aPixelSize.setWidth( aPixelSize.Width() * ratio );
        aPixelSize.setHeight( aPixelSize.Height() * ratio );
        aFont.SetFontSize(aPixelSize);
        pDevice->SetFont(aFont);
        return true;
    }
    return false;
}

void SvxStyleBox_Impl::SetOptimalSize()
{
    Size aSize(LogicToPixel(aLogicalSize, MapMode(MapUnit::MapAppFont)));
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

    tools::Rectangle aTextRect;
    pDevice->GetTextBoundRect(aTextRect, rStyleName);

    Point aPos( rUDEvt.GetRect().TopLeft() );
    aPos.AdjustX(nLeftDistance );

    if (!AdjustFontForItemHeight(pDevice, aTextRect, rUDEvt.GetRect().GetHeight()))
        aPos.AdjustY(( rUDEvt.GetRect().GetHeight() - aTextRect.Bottom() ) / 2 );

    pDevice->DrawText(aPos, rStyleName);
}

void SvxStyleBox_Impl::SetupEntry(vcl::RenderContext& rRenderContext, vcl::Window* pParent, sal_Int32 nItem, const tools::Rectangle& rRect, const OUString& rStyleName, bool bIsNotSelected)
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
                pItemSet->GetItem<SvxFontItem>(SID_ATTR_CHAR_FONT);
            const SvxFontHeightItem * const pFontHeightItem =
                pItemSet->GetItem<SvxFontHeightItem>(SID_ATTR_CHAR_FONTHEIGHT);

            if ( pFontItem && pFontHeightItem )
            {
                Size aFontSize( 0, pFontHeightItem->GetHeight() );
                Size aPixelSize(rRenderContext.LogicToPixel(aFontSize, MapMode(pShell->GetMapUnit())));

                // setup the font properties
                SvxFont aFont;
                aFont.SetFamilyName(pFontItem->GetFamilyName());
                aFont.SetStyleName(pFontItem->GetStyleName());
                aFont.SetFontSize(aPixelSize);

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
                    aFont.SetRelief( static_cast< const SvxCharReliefItem* >( pItem )->GetValue() );

                pItem = pItemSet->GetItem( SID_ATTR_CHAR_UNDERLINE );
                if ( pItem )
                    aFont.SetUnderline( static_cast< const SvxUnderlineItem* >( pItem )->GetLineStyle() );

                pItem = pItemSet->GetItem( SID_ATTR_CHAR_OVERLINE );
                if ( pItem )
                    aFont.SetOverline( static_cast< const SvxOverlineItem* >( pItem )->GetValue() );

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
                    aFontCol = static_cast< const SvxColorItem* >( pItem )->GetValue();

                drawing::FillStyle style = drawing::FillStyle_NONE;
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
                            aBackCol = static_cast< const XFillColorItem* >( pItem )->GetColorValue();

                        if ( aBackCol != COL_AUTO )
                        {
                            rRenderContext.SetFillColor(aBackCol);
                            rRenderContext.DrawRect(rRect);
                        }
                    }
                    break;

                    default: break;
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
                            m_pButtons[nId]->SetPopupMenu(m_pMenu);
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

    const tools::Rectangle& rRect(rUDEvt.GetRect());
    bool bIsNotSelected = rUDEvt.GetItemId() != GetSelectedEntryPos();

    SetupEntry(*pDevice, rUDEvt.GetWindow(), nItem, rRect, aStyleName, bIsNotSelected);

    UserDrawEntry(rUDEvt, aStyleName);

    pDevice->Pop();
    // draw separator, if present
    DrawEntry( rUDEvt, false, false );
}

IMPL_LINK(SvxStyleBox_Impl, CalcOptimalExtraUserWidth, VclWindowEvent&, event, void)
{
    // perform the calculation only when we are opening the dropdown
    if (event.GetId() != VclEventId::DropdownPreOpen)
        return;

    long nMaxNormalFontWidth = 0;
    sal_Int32 nEntryCount = GetEntryCount();
    for (sal_Int32 i = 0; i < nEntryCount; ++i)
    {
        OUString sStyleName(GetEntry(i));
        tools::Rectangle aTextRectForDefaultFont;
        GetTextBoundRect(aTextRectForDefaultFont, sStyleName);

        const long nWidth = aTextRectForDefaultFont.GetWidth();

        nMaxNormalFontWidth = std::max(nWidth, nMaxNormalFontWidth);
    }

    long nMaxUserDrawFontWidth = nMaxNormalFontWidth;
    for (sal_Int32 i = 1; i < nEntryCount-1; ++i)
    {
        OUString sStyleName(GetEntry(i));

        Push(PushFlags::FILLCOLOR | PushFlags::FONT | PushFlags::TEXTCOLOR);
        SetupEntry(*this /*FIXME rendercontext*/, this, i, tools::Rectangle(0, 0, RECT_MAX, ITEM_HEIGHT), sStyleName, true);
        tools::Rectangle aTextRectForActualFont;
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
    mbEndPreview(false),
    mbCheckingUnknownFont(false)
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

IMPL_LINK( SvxFontNameBox_Impl, CheckAndMarkUnknownFont, VclWindowEvent&, event, void )
{
    if( event.GetId() != VclEventId::EditModify )
        return;
    if (mbCheckingUnknownFont) //tdf#117537 block rentry
        return;
    mbCheckingUnknownFont = true;
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
            SetQuickHelpText( SvxResId( RID_SVXSTR_CHARFONTNAME ));
        }
    }
    else
    {
        if( font.GetItalic() != ITALIC_NORMAL )
        {
            font.SetItalic( ITALIC_NORMAL );
            SetControlFont( font );
            SetQuickHelpText( SvxResId( RID_SVXSTR_CHARFONTNAME_NOTAVAILABLE ));
        }
    }
    mbCheckingUnknownFont = false;
}

void SvxFontNameBox_Impl::Update( const css::awt::FontDescriptor* pFontDesc )
{
    if ( pFontDesc )
    {
        aCurFont.SetFamilyName  ( pFontDesc->Name );
        aCurFont.SetFamily      ( FontFamily( pFontDesc->Family ) );
        aCurFont.SetStyleName   ( pFontDesc->StyleName );
        aCurFont.SetPitch       ( FontPitch( pFontDesc->Pitch ) );
        aCurFont.SetCharSet     ( rtl_TextEncoding( pFontDesc->CharSet ) );
    }
    OUString aCurName = aCurFont.GetFamilyName();
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

bool SvxFontNameBox_Impl::EventNotify( NotifyEvent& rNEvt )
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

    return bHandled || FontNameBox::EventNotify( rNEvt );
}

void SvxFontNameBox_Impl::SetOptimalSize()
{
    Size aSize(LogicToPixel(aLogicalSize, MapMode(MapUnit::MapAppFont)));
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

        SvxFontItem aFontItem( aFontMetric.GetFamilyType(),
            aFontMetric.GetFamilyName(),
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

        pFontItem.reset( new SvxFontItem( aFontMetric.GetFamilyType(),
            aFontMetric.GetFamilyName(),
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

SvxColorWindow::SvxColorWindow(const OUString&            rCommand,
                               std::shared_ptr<PaletteManager> const & rPaletteManager,
                               BorderColorStatus&         rBorderColorStatus,
                               sal_uInt16                 nSlotId,
                               const Reference< XFrame >& rFrame,
                               vcl::Window*               pParentWindow,
                               bool                       bReuseParentForPicker,
                               std::function<void(const OUString&, const NamedColor&)> const & aFunction):

    ToolbarPopup( rFrame, pParentWindow, "palette_popup_window", "svx/ui/oldcolorwindow.ui" ),
    theSlotId( nSlotId ),
    maCommand( rCommand ),
    mxParentWindow(pParentWindow),
    mxPaletteManager( rPaletteManager ),
    mrBorderColorStatus( rBorderColorStatus ),
    maColorSelectFunction(aFunction),
    mbReuseParentForPicker(bReuseParentForPicker)
{
    get(mpPaletteListBox,     "palette_listbox");
    get(mpButtonAutoColor,    "auto_color_button");
    get(mpButtonNoneColor,    "none_color_button");
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
            mpButtonAutoColor->SetText( SvxResId( RID_SVXSTR_NOFILL ) );
            break;
        }
        case SID_AUTHOR_COLOR:
        {
            mpButtonAutoColor->SetText( SvxResId( RID_SVXSTR_BY_AUTHOR ) );
            break;
        }
        case SID_BMPMASK_COLOR:
        {
            mpButtonAutoColor->SetText( SvxResId( RID_SVXSTR_TRANSPARENT ) );
            break;
        }
        case SID_ATTR_CHAR_COLOR:
        case SID_ATTR_CHAR_COLOR2:
        case SID_EXTRUSION_3D_COLOR:
        {
            mpButtonAutoColor->SetText(EditResId(RID_SVXSTR_AUTOMATIC));
            break;
        }
        case SID_FM_CTL_PROPERTIES:
        {
            mpButtonAutoColor->SetText( SvxResId( RID_SVXSTR_DEFAULT ) );
            break;
        }
        default:
        {
            mpButtonAutoColor->Hide();
            mpAutomaticSeparator->Hide();
            break;
        }
    }

    mpColorSet->SetAccessibleName( GetText() );

    mpPaletteListBox->SetStyle( mpPaletteListBox->GetStyle() | WB_BORDER );
    mpPaletteListBox->SetSelectHdl( LINK( this, SvxColorWindow, SelectPaletteHdl ) );
    mpPaletteListBox->AdaptDropDownLineCountToMaximum();
    std::vector<OUString> aPaletteList = mxPaletteManager->GetPaletteList();
    for( std::vector<OUString>::iterator it = aPaletteList.begin(); it != aPaletteList.end(); ++it )
    {
        mpPaletteListBox->InsertEntry( *it );
    }
    OUString aPaletteName( officecfg::Office::Common::UserColors::PaletteName::get() );
    mpPaletteListBox->SelectEntry( aPaletteName );
    const sal_Int32 nSelectedEntry(mpPaletteListBox->GetSelectedEntryPos());
    if (nSelectedEntry != LISTBOX_ENTRY_NOTFOUND)
        mxPaletteManager->SetPalette(nSelectedEntry);

    mpButtonAutoColor->SetClickHdl( LINK( this, SvxColorWindow, AutoColorClickHdl ) );
    mpButtonNoneColor->SetClickHdl( LINK( this, SvxColorWindow, AutoColorClickHdl ) );
    mpButtonPicker->SetClickHdl( LINK( this, SvxColorWindow, OpenPickerClickHdl ) );

    mpColorSet->SetSelectHdl( LINK( this, SvxColorWindow, SelectHdl ) );
    mpRecentColorSet->SetSelectHdl( LINK( this, SvxColorWindow, SelectHdl ) );
    SetHelpId( HID_POPUP_COLOR );
    mpColorSet->SetHelpId( HID_POPUP_COLOR_CTRL );

    mxPaletteManager->ReloadColorSet(*mpColorSet);
    const sal_uInt32 nMaxItems(SvxColorValueSet::getMaxRowCount() * SvxColorValueSet::getColumnCount());
    Size aSize = mpColorSet->layoutAllVisible(nMaxItems);
    mpColorSet->set_height_request(aSize.Height());
    mpColorSet->set_width_request(aSize.Width());

    mxPaletteManager->ReloadRecentColorSet(*mpRecentColorSet);
    aSize = mpRecentColorSet->layoutAllVisible(mxPaletteManager->GetRecentColorCount());
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

ColorWindow::ColorWindow(std::shared_ptr<PaletteManager> const & rPaletteManager,
                         BorderColorStatus&         rBorderColorStatus,
                         sal_uInt16                 nSlotId,
                         const Reference< XFrame >& rFrame,
                         weld::Window*              pParentWindow,
                         weld::MenuButton*          pMenuButton,
                         std::function<void(const OUString&, const NamedColor&)> const & aFunction)
    : ToolbarPopupBase(rFrame)
    , m_xBuilder(Application::CreateBuilder(pMenuButton, "svx/ui/colorwindow.ui"))
    , theSlotId(nSlotId)
    , mpParentWindow(pParentWindow)
    , mpMenuButton(pMenuButton)
    , mxPaletteManager(rPaletteManager)
    , mrBorderColorStatus(rBorderColorStatus)
    , maColorSelectFunction(aFunction)
    , mxColorSet(new ColorValueSet(m_xBuilder->weld_scrolled_window("colorsetwin")))
    , mxRecentColorSet(new ColorValueSet(nullptr))
    , mxTopLevel(m_xBuilder->weld_container("palette_popup_window"))
    , mxPaletteListBox(m_xBuilder->weld_combo_box_text("palette_listbox"))
    , mxButtonAutoColor(m_xBuilder->weld_button("auto_color_button"))
    , mxButtonNoneColor(m_xBuilder->weld_button("none_color_button"))
    , mxButtonPicker(m_xBuilder->weld_button("color_picker_button"))
    , mxAutomaticSeparator(m_xBuilder->weld_widget("separator4"))
    , mxColorSetWin(new weld::CustomWeld(*m_xBuilder, "colorset", *mxColorSet))
    , mxRecentColorSetWin(new weld::CustomWeld(*m_xBuilder, "recent_colorset", *mxRecentColorSet))
{
    mxColorSet->SetStyle( WinBits(WB_FLATVALUESET | WB_ITEMBORDER | WB_3DLOOK | WB_NO_DIRECTSELECT | WB_TABSTOP) );
    mxRecentColorSet->SetStyle( WinBits(WB_FLATVALUESET | WB_ITEMBORDER | WB_3DLOOK | WB_NO_DIRECTSELECT | WB_TABSTOP) );

    switch ( theSlotId )
    {
        case SID_ATTR_CHAR_COLOR_BACKGROUND:
        case SID_BACKGROUND_COLOR:
        case SID_ATTR_CHAR_BACK_COLOR:
        {
            mxButtonAutoColor->set_label( SvxResId( RID_SVXSTR_NOFILL ) );
            break;
        }
        case SID_AUTHOR_COLOR:
        {
            mxButtonAutoColor->set_label( SvxResId( RID_SVXSTR_BY_AUTHOR ) );
            break;
        }
        case SID_BMPMASK_COLOR:
        {
            mxButtonAutoColor->set_label( SvxResId( RID_SVXSTR_TRANSPARENT ) );
            break;
        }
        case SID_ATTR_CHAR_COLOR:
        case SID_ATTR_CHAR_COLOR2:
        case SID_EXTRUSION_3D_COLOR:
        {
            mxButtonAutoColor->set_label(EditResId(RID_SVXSTR_AUTOMATIC));
            break;
        }
        case SID_FM_CTL_PROPERTIES:
        {
            mxButtonAutoColor->set_label( SvxResId( RID_SVXSTR_DEFAULT ) );
            break;
        }
        default:
        {
            mxButtonAutoColor->hide();
            mxAutomaticSeparator->hide();
            break;
        }
    }

    mxPaletteListBox->connect_changed(LINK(this, ColorWindow, SelectPaletteHdl));
    std::vector<OUString> aPaletteList = mxPaletteManager->GetPaletteList();
    for (std::vector<OUString>::iterator it = aPaletteList.begin(); it != aPaletteList.end(); ++it)
        mxPaletteListBox->append_text(*it);
    OUString aPaletteName( officecfg::Office::Common::UserColors::PaletteName::get() );
    mxPaletteListBox->set_active_text(aPaletteName);
    const int nSelectedEntry(mxPaletteListBox->get_active());
    if (nSelectedEntry != -1)
        mxPaletteManager->SetPalette(nSelectedEntry);

    mxButtonAutoColor->connect_clicked(LINK(this, ColorWindow, AutoColorClickHdl));
    mxButtonNoneColor->connect_clicked(LINK(this, ColorWindow, AutoColorClickHdl));
    mxButtonPicker->connect_clicked(LINK(this, ColorWindow, OpenPickerClickHdl));

    mxColorSet->SetSelectHdl(LINK( this, ColorWindow, SelectHdl));
    mxRecentColorSet->SetSelectHdl(LINK( this, ColorWindow, SelectHdl));
    mxTopLevel->set_help_id(HID_POPUP_COLOR);
    mxColorSet->SetHelpId(HID_POPUP_COLOR_CTRL);

    mxPaletteManager->ReloadColorSet(*mxColorSet);
    const sal_uInt32 nMaxItems(SvxColorValueSet::getMaxRowCount() * SvxColorValueSet::getColumnCount());
    Size aSize = mxColorSet->layoutAllVisible(nMaxItems);
    mxColorSet->set_size_request(aSize.Width(), aSize.Height());

    mxPaletteManager->ReloadRecentColorSet(*mxRecentColorSet);
    aSize = mxRecentColorSet->layoutAllVisible(mxPaletteManager->GetRecentColorCount());
    mxRecentColorSet->set_size_request(aSize.Width(), aSize.Height());

    AddStatusListener( ".uno:ColorTableState" );
    AddStatusListener( OUString() );
}

void SvxColorWindow::ShowNoneButton()
{
    mpButtonNoneColor->Show();
}

void ColorWindow::ShowNoneButton()
{
    mxButtonNoneColor->show();
}

SvxColorWindow::~SvxColorWindow()
{
    disposeOnce();
}

ColorWindow::~ColorWindow()
{
}

void SvxColorWindow::dispose()
{
    mpColorSet.clear();
    mpRecentColorSet.clear();
    mpPaletteListBox.clear();
    mpButtonAutoColor.clear();
    mpButtonNoneColor.clear();
    mpButtonPicker.clear();
    mpAutomaticSeparator.clear();
    mxParentWindow.clear();
    ToolbarPopup::dispose();
}

void SvxColorWindow::KeyInput( const KeyEvent& rKEvt )
{
    mpColorSet->GrabFocus();
    mpColorSet->KeyInput(rKEvt);
}

NamedColor SvxColorWindow::GetSelectEntryColor(ValueSet const * pColorSet)
{
    Color aColor = pColorSet->GetItemColor(pColorSet->GetSelectedItemId());
    OUString sColorName = pColorSet->GetItemText(pColorSet->GetSelectedItemId());
    return std::make_pair(aColor, sColorName);
}

NamedColor ColorWindow::GetSelectEntryColor(SvtValueSet const * pColorSet)
{
    Color aColor = pColorSet->GetItemColor(pColorSet->GetSelectedItemId());
    OUString sColorName = pColorSet->GetItemText(pColorSet->GetSelectedItemId());
    return std::make_pair(aColor, sColorName);
}

namespace
{
    NamedColor GetAutoColor(sal_uInt16 nSlotId)
    {
        Color aColor;
        OUString sColorName;
        switch (nSlotId)
        {
            case SID_ATTR_CHAR_COLOR_BACKGROUND:
            case SID_BACKGROUND_COLOR:
            case SID_ATTR_CHAR_BACK_COLOR:
                aColor = COL_TRANSPARENT;
                sColorName = SvxResId(RID_SVXSTR_NOFILL);
                break;
            case SID_AUTHOR_COLOR:
                aColor = COL_TRANSPARENT;
                sColorName = SvxResId(RID_SVXSTR_BY_AUTHOR);
                break;
            case SID_BMPMASK_COLOR:
                aColor = COL_TRANSPARENT;
                sColorName = SvxResId(RID_SVXSTR_TRANSPARENT);
                break;
            case SID_FM_CTL_PROPERTIES:
                aColor = COL_TRANSPARENT;
                sColorName = SvxResId(RID_SVXSTR_DEFAULT);
                break;
            case SID_ATTR_CHAR_COLOR:
            case SID_ATTR_CHAR_COLOR2:
            case SID_EXTRUSION_3D_COLOR:
            default:
                aColor = COL_AUTO;
                sColorName = EditResId(RID_SVXSTR_AUTOMATIC);
                break;
        }

        return std::make_pair(aColor, sColorName);
    }

    NamedColor GetNoneColor()
    {
        return std::make_pair(COL_NONE_COLOR, SvxResId(RID_SVXSTR_NONE));
    }
}

NamedColor SvxColorWindow::GetSelectEntryColor() const
{
    if (!mpColorSet->IsNoSelection())
        return GetSelectEntryColor(mpColorSet);
    if (!mpRecentColorSet->IsNoSelection())
        return GetSelectEntryColor(mpRecentColorSet);
    if (mpButtonNoneColor->GetStyle() & WB_DEFBUTTON)
        return GetNoneColor();
    return GetAutoColor();
}

NamedColor ColorWindow::GetSelectEntryColor() const
{
    if (!mxColorSet->IsNoSelection())
        return GetSelectEntryColor(mxColorSet.get());
    if (!mxRecentColorSet->IsNoSelection())
        return GetSelectEntryColor(mxRecentColorSet.get());
    if (mxButtonNoneColor->get_has_default())
        return GetNoneColor();
    return GetAutoColor();
}

IMPL_LINK(SvxColorWindow, SelectHdl, ValueSet*, pColorSet, void)
{
    VclPtr<SvxColorWindow> xThis(this);

    NamedColor aNamedColor = GetSelectEntryColor(pColorSet);
    /*  #i33380# DR 2004-09-03 Moved the following line above the Dispatch() calls.
        This instance may be deleted in the meantime (i.e. when a dialog is opened
        while in Dispatch()), accessing members will crash in this case. */
    pColorSet->SetNoSelection();

    if ( pColorSet != mpRecentColorSet )
    {
         mxPaletteManager->AddRecentColor(aNamedColor.first, aNamedColor.second);
         if ( !IsInPopupMode() )
            mxPaletteManager->ReloadRecentColorSet(*mpRecentColorSet);
    }

    if ( IsInPopupMode() )
        EndPopupMode();

    maSelectedLink.Call(aNamedColor);

    maColorSelectFunction(maCommand, aNamedColor);
}

IMPL_LINK(ColorWindow, SelectHdl, SvtValueSet*, pColorSet, void)
{
    NamedColor aNamedColor = GetSelectEntryColor(pColorSet);
    /*  #i33380# DR 2004-09-03 Moved the following line above the Dispatch() calls.
        This instance may be deleted in the meantime (i.e. when a dialog is opened
        while in Dispatch()), accessing members will crash in this case. */
    pColorSet->SetNoSelection();

    if (pColorSet != mxRecentColorSet.get())
    {
         mxPaletteManager->AddRecentColor(aNamedColor.first, aNamedColor.second);
         if (!mpMenuButton->get_active())
            mxPaletteManager->ReloadRecentColorSet(*mxRecentColorSet);
    }

    if (mpMenuButton->get_active())
        mpMenuButton->set_active(false);

    maColorSelectFunction(OUString(), aNamedColor);
}

IMPL_LINK_NOARG(SvxColorWindow, SelectPaletteHdl, ListBox&, void)
{
    sal_Int32 nPos = mpPaletteListBox->GetSelectedEntryPos();
    mxPaletteManager->SetPalette( nPos );
    mxPaletteManager->ReloadColorSet(*mpColorSet);
    mpColorSet->layoutToGivenHeight(mpColorSet->GetSizePixel().Height(), mxPaletteManager->GetColorCount());
}

IMPL_LINK_NOARG(ColorWindow, SelectPaletteHdl, weld::ComboBoxText&, void)
{
    int nPos = mxPaletteListBox->get_active();
    mxPaletteManager->SetPalette( nPos );
    mxPaletteManager->ReloadColorSet(*mxColorSet);
    mxColorSet->layoutToGivenHeight(mxColorSet->GetOutputSizePixel().Height(), mxPaletteManager->GetColorCount());
}

NamedColor SvxColorWindow::GetAutoColor() const
{
    return ::GetAutoColor(theSlotId);
}

NamedColor ColorWindow::GetAutoColor() const
{
    return ::GetAutoColor(theSlotId);
}

IMPL_LINK(SvxColorWindow, AutoColorClickHdl, Button*, pButton, void)
{
    VclPtr<SvxColorWindow> xThis(this);

    NamedColor aNamedColor = pButton == mpButtonAutoColor ? GetAutoColor() : GetNoneColor();

    mpRecentColorSet->SetNoSelection();

    if ( IsInPopupMode() )
        EndPopupMode();

    maSelectedLink.Call(aNamedColor);

    maColorSelectFunction(maCommand, aNamedColor);
}

IMPL_LINK(ColorWindow, AutoColorClickHdl, weld::Button&, rButton, void)
{
    NamedColor aNamedColor = &rButton == mxButtonAutoColor.get() ? GetAutoColor() : GetNoneColor();

    mxRecentColorSet->SetNoSelection();

    if (mpMenuButton->get_active())
        mpMenuButton->set_active(false);

    maColorSelectFunction(OUString(), aNamedColor);
}

IMPL_LINK_NOARG(SvxColorWindow, OpenPickerClickHdl, Button*, void)
{
    VclPtr<SvxColorWindow> xThis(this);

    if ( IsInPopupMode() )
        EndPopupMode();

    weld::Window* pParentFrame;
    if (mbReuseParentForPicker)
    {
        pParentFrame = mxParentWindow->GetFrameWeld();
    }
    else
    {
        const css::uno::Reference<css::awt::XWindow> xParent(mxFrame->getContainerWindow(), uno::UNO_QUERY);
        pParentFrame = Application::GetFrameWeld(xParent);
    }
    mxPaletteManager->PopupColorPicker(pParentFrame, maCommand, GetSelectEntryColor().first);
}

IMPL_LINK_NOARG(ColorWindow, OpenPickerClickHdl, weld::Button&, void)
{
    if (mpMenuButton->get_active())
        mpMenuButton->set_active(false);
    mxPaletteManager->PopupColorPicker(mpParentWindow, OUString(), GetSelectEntryColor().first);
}

void SvxColorWindow::StartSelection()
{
    mpColorSet->StartSelection();
    mpRecentColorSet->StartSelection();
}

void SvxColorWindow::SetNoSelection()
{
    mpColorSet->SetNoSelection();
    mpRecentColorSet->SetNoSelection();
    mpButtonAutoColor->set_property("has-default", "false");
    mpButtonNoneColor->set_property("has-default", "false");
}

void ColorWindow::SetNoSelection()
{
    mxColorSet->SetNoSelection();
    mxRecentColorSet->SetNoSelection();
    mxButtonAutoColor->set_has_default(false);
    mxButtonNoneColor->set_has_default(false);
}

bool SvxColorWindow::IsNoSelection() const
{
    if (!mpColorSet->IsNoSelection())
        return false;
    if (!mpRecentColorSet->IsNoSelection())
        return false;
    return !mpButtonAutoColor->IsVisible() && !mpButtonNoneColor->IsVisible();
}

bool ColorWindow::IsNoSelection() const
{
    if (!mxColorSet->IsNoSelection())
        return false;
    if (!mxRecentColorSet->IsNoSelection())
        return false;
    return !mxButtonAutoColor->get_visible() && !mxButtonNoneColor->get_visible();
}

void SvxColorWindow::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    if (rEvent.FeatureURL.Complete == ".uno:ColorTableState")
    {
        if (rEvent.IsEnabled && mxPaletteManager->GetPalette() == 0)
        {
            mxPaletteManager->ReloadColorSet(*mpColorSet);
            mpColorSet->layoutToGivenHeight(mpColorSet->GetSizePixel().Height(), mxPaletteManager->GetColorCount());
        }
    }
    else
    {
        Color aColor(COL_TRANSPARENT);

        if (mrBorderColorStatus.statusChanged(rEvent))
        {
            aColor = mrBorderColorStatus.GetColor();
        }
        else if (rEvent.IsEnabled)
        {
            sal_Int32 nValue;
            if (rEvent.State >>= nValue)
                aColor = Color(nValue);
        }

        SelectEntry(aColor);
    }
}

void ColorWindow::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    if (rEvent.FeatureURL.Complete == ".uno:ColorTableState")
    {
        if (rEvent.IsEnabled && mxPaletteManager->GetPalette() == 0)
        {
            mxPaletteManager->ReloadColorSet(*mxColorSet);
            mxColorSet->layoutToGivenHeight(mxColorSet->GetOutputSizePixel().Height(), mxPaletteManager->GetColorCount());
        }
    }
    else
    {
        Color aColor(COL_TRANSPARENT);

        if (mrBorderColorStatus.statusChanged(rEvent))
        {
            aColor = mrBorderColorStatus.GetColor();
        }
        else if (rEvent.IsEnabled)
        {
            sal_Int32 nValue;
            if (rEvent.State >>= nValue)
                aColor = Color(nValue);
        }

        SelectEntry(aColor);
    }
}

bool SvxColorWindow::SelectValueSetEntry(SvxColorValueSet* pColorSet, const Color& rColor)
{
    for (size_t i = 1; i <= pColorSet->GetItemCount(); ++i)
    {
        if (rColor == pColorSet->GetItemColor(i))
        {
            pColorSet->SelectItem(i);
            return true;
        }
    }
    return false;
}

bool ColorWindow::SelectValueSetEntry(ColorValueSet* pColorSet, const Color& rColor)
{
    for (size_t i = 1; i <= pColorSet->GetItemCount(); ++i)
    {
        if (rColor == pColorSet->GetItemColor(i))
        {
            pColorSet->SelectItem(i);
            return true;
        }
    }
    return false;
}

void SvxColorWindow::SelectEntry(const NamedColor& rNamedColor)
{
    SetNoSelection();

    const Color &rColor = rNamedColor.first;

    if (rColor == COL_TRANSPARENT || rColor == COL_AUTO)
    {
        mpButtonAutoColor->set_property("has-default", "true");
        return;
    }

    if (mpButtonNoneColor->IsVisible() && rColor == COL_NONE_COLOR)
    {
        mpButtonNoneColor->set_property("has-default", "true");
        return;
    }

    // try current palette
    bool bFoundColor = SelectValueSetEntry(mpColorSet, rColor);
    // try recently used
    if (!bFoundColor)
        bFoundColor = SelectValueSetEntry(mpRecentColorSet, rColor);
    // if its not there, add it there now to the end of the recently used
    // so its available somewhere handy, but not without trashing the
    // whole recently used
    if (!bFoundColor)
    {
        const OUString& rColorName = rNamedColor.second;
        mxPaletteManager->AddRecentColor(rColor, rColorName, false);
        mxPaletteManager->ReloadRecentColorSet(*mpRecentColorSet);
        SelectValueSetEntry(mpRecentColorSet, rColor);
    }
}

void SvxColorWindow::SelectEntry(const Color& rColor)
{
    OUString sColorName = ("#" + rColor.AsRGBHexString().toAsciiUpperCase());
    SvxColorWindow::SelectEntry(std::make_pair(rColor, sColorName));
}

void ColorWindow::SelectEntry(const NamedColor& rNamedColor)
{
    SetNoSelection();

    const Color &rColor = rNamedColor.first;

    if (mxButtonNoneColor->get_visible() && (rColor == COL_TRANSPARENT || rColor == COL_AUTO))
    {
        mxButtonAutoColor->set_has_default(true);
        return;
    }

    if (mxButtonNoneColor->get_visible() && rColor == COL_NONE_COLOR)
    {
        mxButtonNoneColor->set_has_default(true);
        return;
    }

    // try current palette
    bool bFoundColor = SelectValueSetEntry(mxColorSet.get(), rColor);
    // try recently used
    if (!bFoundColor)
        bFoundColor = SelectValueSetEntry(mxRecentColorSet.get(), rColor);
    // if its not there, add it there now to the end of the recently used
    // so its available somewhere handy, but not without trashing the
    // whole recently used
    if (!bFoundColor)
    {
        const OUString& rColorName = rNamedColor.second;
        mxPaletteManager->AddRecentColor(rColor, rColorName, false);
        mxPaletteManager->ReloadRecentColorSet(*mxRecentColorSet);
        SelectValueSetEntry(mxRecentColorSet.get(), rColor);
    }
}

void ColorWindow::SelectEntry(const Color& rColor)
{
    OUString sColorName = ("#" + rColor.AsRGBHexString().toAsciiUpperCase());
    ColorWindow::SelectEntry(std::make_pair(rColor, sColorName));
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

bool BorderColorStatus::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    Color aColor( COL_TRANSPARENT );

    if ( rEvent.FeatureURL.Complete == ".uno:FrameLineColor" )
    {
        if ( rEvent.IsEnabled )
            rEvent.State >>= aColor;

        maColor = aColor;
        return true;
    }
    else
    {
        css::table::BorderLine2 aTable;
        if ( rEvent.IsEnabled && ( rEvent.State >>= aTable ) )
            aColor = Color(aTable.Color);

        if ( rEvent.FeatureURL.Complete == ".uno:BorderTLBR" )
        {
            maTLBRColor = aColor;
            return true;
        }
        else if ( rEvent.FeatureURL.Complete == ".uno:BorderBLTR" )
        {
            maBLTRColor = aColor;
            return true;
        }
    }

    return false;
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


SvxFrameWindow_Impl::SvxFrameWindow_Impl ( svt::ToolboxController& rController, vcl::Window* pParentWindow ) :
    ToolbarPopup( rController.getFrameInterface(), pParentWindow, WB_STDPOPUP | WB_MOVEABLE | WB_CLOSEABLE ),
    aFrameSet   ( VclPtr<SvxFrmValueSet_Impl>::Create(this, WinBits( WB_ITEMBORDER | WB_DOUBLEBORDER | WB_3DLOOK | WB_NO_DIRECTSELECT )) ),
    mrController( rController ),
    bParagraphMode(false)
{
    AddStatusListener(".uno:BorderReducedMode");
    InitImageList();

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
        aFrameSet->InsertItem(i, Image(aImgVec[i-1]));

    //bParagraphMode should have been set in StateChanged
    if ( !bParagraphMode )
        for ( i = 9; i < 13; i++ )
            aFrameSet->InsertItem(i, Image(aImgVec[i-1]));

    aFrameSet->SetColCount( 4 );
    aFrameSet->SetSelectHdl( LINK( this, SvxFrameWindow_Impl, SelectHdl ) );
    CalcSizeValueSet();

    SetHelpId( HID_POPUP_FRAME );
    SetText( SvxResId(RID_SVXSTR_FRAME) );
    aFrameSet->SetAccessibleName( SvxResId(RID_SVXSTR_FRAME) );
    aFrameSet->Show();
}

SvxFrameWindow_Impl::~SvxFrameWindow_Impl()
{
    disposeOnce();
}

void SvxFrameWindow_Impl::dispose()
{
    aFrameSet.disposeAndClear();
    ToolbarPopup::dispose();
}

void SvxFrameWindow_Impl::GetFocus()
{
    if (aFrameSet)
        aFrameSet->StartSelection();
}

void SvxFrameWindow_Impl::KeyInput( const KeyEvent& rKEvt )
{
    aFrameSet->GrabFocus();
    aFrameSet->KeyInput( rKEvt );
}

void SvxFrameWindow_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    ToolbarPopup::DataChanged( rDCEvt );

    if ( ( rDCEvt.GetType() == DataChangedEventType::SETTINGS ) && ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE ) )
    {
        InitImageList();

        sal_uInt16 nNumOfItems = aFrameSet->GetItemCount();
        for ( sal_uInt16 i = 1 ; i <= nNumOfItems ; ++i )
            aFrameSet->SetItemImage( i, Image(aImgVec[i-1]) );
    }
}

enum class FrmValidFlags {
    NONE      = 0x00,
    Left      = 0x01,
    Right     = 0x02,
    Top       = 0x04,
    Bottom    = 0x08,
    HInner    = 0x10,
    VInner    = 0x20,
    AllMask   = 0x3f,
};
namespace o3tl {
    template<> struct typed_flags<FrmValidFlags> : is_typed_flags<FrmValidFlags, 0x3f> {};
}

// By default unset lines remain unchanged.
// Via Shift unset lines are reset

IMPL_LINK_NOARG(SvxFrameWindow_Impl, SelectHdl, ValueSet*, void)
{
    VclPtr<SvxFrameWindow_Impl> xThis(this);

    SvxBoxItem          aBorderOuter( SID_ATTR_BORDER_OUTER );
    SvxBoxInfoItem      aBorderInner( SID_ATTR_BORDER_INNER );
    SvxBorderLine       theDefLine;
    SvxBorderLine       *pLeft = nullptr,
                        *pRight = nullptr,
                        *pTop = nullptr,
                        *pBottom = nullptr;
    sal_uInt16           nSel = aFrameSet->GetSelectedItemId();
    sal_uInt16           nModifier = aFrameSet->GetModifier();
    FrmValidFlags        nValidFlags = FrmValidFlags::NONE;

    theDefLine.GuessLinesWidths(theDefLine.GetBorderLineStyle(),
            DEF_LINE_WIDTH_0);
    switch ( nSel )
    {
        case 1: nValidFlags |= FrmValidFlags::AllMask;
        break;  // NONE
        case 2: pLeft = &theDefLine;
                nValidFlags |= FrmValidFlags::Left;
        break;  // LEFT
        case 3: pRight = &theDefLine;
                nValidFlags |= FrmValidFlags::Right;
        break;  // RIGHT
        case 4: pLeft = pRight = &theDefLine;
                nValidFlags |=  FrmValidFlags::Right|FrmValidFlags::Left;
        break;  // LEFTRIGHT
        case 5: pTop = &theDefLine;
                nValidFlags |= FrmValidFlags::Top;
        break;  // TOP
        case 6: pBottom = &theDefLine;
                nValidFlags |= FrmValidFlags::Bottom;
        break;  // BOTTOM
        case 7: pTop =  pBottom = &theDefLine;
                nValidFlags |= FrmValidFlags::Bottom|FrmValidFlags::Top;
        break;  // TOPBOTTOM
        case 8: pLeft = pRight = pTop = pBottom = &theDefLine;
                nValidFlags |= FrmValidFlags::Left | FrmValidFlags::Right | FrmValidFlags::Top | FrmValidFlags::Bottom;
        break;  // OUTER

        // Inner Table:
        case 9: // HOR
            pTop = pBottom = &theDefLine;
            aBorderInner.SetLine( &theDefLine, SvxBoxInfoItemLine::HORI );
            aBorderInner.SetLine( nullptr, SvxBoxInfoItemLine::VERT );
            nValidFlags |= FrmValidFlags::HInner|FrmValidFlags::Top|FrmValidFlags::Bottom;
            break;

        case 10: // HORINNER
            pLeft = pRight = pTop = pBottom = &theDefLine;
            aBorderInner.SetLine( &theDefLine, SvxBoxInfoItemLine::HORI );
            aBorderInner.SetLine( nullptr, SvxBoxInfoItemLine::VERT );
            nValidFlags |= FrmValidFlags::Right|FrmValidFlags::Left|FrmValidFlags::HInner|FrmValidFlags::Top|FrmValidFlags::Bottom;
            break;

        case 11: // VERINNER
            pLeft = pRight = pTop = pBottom = &theDefLine;
            aBorderInner.SetLine( nullptr, SvxBoxInfoItemLine::HORI );
            aBorderInner.SetLine( &theDefLine, SvxBoxInfoItemLine::VERT );
            nValidFlags |= FrmValidFlags::Right|FrmValidFlags::Left|FrmValidFlags::VInner|FrmValidFlags::Top|FrmValidFlags::Bottom;
        break;

        case 12: // ALL
            pLeft = pRight = pTop = pBottom = &theDefLine;
            aBorderInner.SetLine( &theDefLine, SvxBoxInfoItemLine::HORI );
            aBorderInner.SetLine( &theDefLine, SvxBoxInfoItemLine::VERT );
            nValidFlags |= FrmValidFlags::AllMask;
            break;

        default:
        break;
    }
    aBorderOuter.SetLine( pLeft, SvxBoxItemLine::LEFT );
    aBorderOuter.SetLine( pRight, SvxBoxItemLine::RIGHT );
    aBorderOuter.SetLine( pTop, SvxBoxItemLine::TOP );
    aBorderOuter.SetLine( pBottom, SvxBoxItemLine::BOTTOM );

    if(nModifier == KEY_SHIFT)
        nValidFlags |= FrmValidFlags::AllMask;
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::TOP,       bool(nValidFlags&FrmValidFlags::Top ));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::BOTTOM,    bool(nValidFlags&FrmValidFlags::Bottom ));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::LEFT,      bool(nValidFlags&FrmValidFlags::Left));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::RIGHT,     bool(nValidFlags&FrmValidFlags::Right ));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::HORI,      bool(nValidFlags&FrmValidFlags::HInner ));
    aBorderInner.SetValid( SvxBoxInfoItemValidFlags::VERT,      bool(nValidFlags&FrmValidFlags::VInner));
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

    if (aFrameSet)
    {
        /* #i33380# Moved the following line above the Dispatch() call.
           This instance may be deleted in the meantime (i.e. when a dialog is opened
           while in Dispatch()), accessing members will crash in this case. */
        aFrameSet->SetNoSelection();
    }

    mrController.dispatchCommand( ".uno:SetBorderStyle", aArgs );
}

void SvxFrameWindow_Impl::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    if ( rEvent.FeatureURL.Complete == ".uno:BorderReducedMode" )
    {
        bool bValue;
        if ( rEvent.State >>= bValue )
        {
            bParagraphMode = bValue;
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
                        aFrameSet->InsertItem(i, Image(aImgVec[i-1]));
                    bResize = true;
                }

                if ( bResize )
                {
                    CalcSizeValueSet();
                }
            }
        }
    }
}

void SvxFrameWindow_Impl::CalcSizeValueSet()
{
    Size aItemSize( 20 * GetParent()->GetDPIScaleFactor(), 20 * GetParent()->GetDPIScaleFactor() );
    Size aSize = aFrameSet->CalcWindowSizePixel( aItemSize );
    aFrameSet->SetPosSizePixel( Point( 2, 2 ), aSize );
    aSize.AdjustWidth(4 );
    aSize.AdjustHeight(4 );
    SetOutputSizePixel( aSize );
}

void SvxFrameWindow_Impl::InitImageList()
{
    aImgVec.clear();
    aImgVec.emplace_back(RID_SVXBMP_FRAME1);
    aImgVec.emplace_back(RID_SVXBMP_FRAME2);
    aImgVec.emplace_back(RID_SVXBMP_FRAME3);
    aImgVec.emplace_back(RID_SVXBMP_FRAME4);
    aImgVec.emplace_back(RID_SVXBMP_FRAME5);
    aImgVec.emplace_back(RID_SVXBMP_FRAME6);
    aImgVec.emplace_back(RID_SVXBMP_FRAME7);
    aImgVec.emplace_back(RID_SVXBMP_FRAME8);
    aImgVec.emplace_back(RID_SVXBMP_FRAME9);
    aImgVec.emplace_back(RID_SVXBMP_FRAME10);
    aImgVec.emplace_back(RID_SVXBMP_FRAME11);
    aImgVec.emplace_back(RID_SVXBMP_FRAME12);
}

static Color lcl_mediumColor( Color aMain, Color /*aDefault*/ )
{
    return SvxBorderLine::threeDMediumColor( aMain );
}

SvxCurrencyList_Impl::SvxCurrencyList_Impl(
    SvxCurrencyToolBoxControl* pControl,
    vcl::Window* pParentWindow,
    OUString& rSelectedFormat,
    LanguageType& eSelectedLanguage ) :
    ToolbarPopup( pControl->getFrameInterface(), pParentWindow, WB_STDPOPUP | WB_MOVEABLE | WB_CLOSEABLE ),
    m_pCurrencyLb( VclPtr<ListBox>::Create(this) ),
    m_xControl( pControl ),
    m_rSelectedFormat( rSelectedFormat ),
    m_eSelectedLanguage( eSelectedLanguage )
{
    m_pCurrencyLb->setPosSizePixel( 2, 2, 300, 140 );
    SetOutputSizePixel( Size( 304, 144 ) );

    std::vector< OUString > aList;
    std::vector< sal_uInt16 > aCurrencyList;
    const NfCurrencyTable& rCurrencyTable = SvNumberFormatter::GetTheCurrencyTable();
    sal_uInt16 nLen = rCurrencyTable.size();

    SvNumberFormatter aFormatter( m_xControl->getContext(), LANGUAGE_SYSTEM );
    m_eFormatLanguage = aFormatter.GetLanguage();

    SvxCurrencyToolBoxControl::GetCurrencySymbols( aList, true, aCurrencyList );

    sal_uInt16 nPos = 0, nCount = 0;
    sal_Int32 nSelectedPos = -1;
    bool bIsSymbol;
    NfWSStringsDtor aStringsDtor;

    for( std::vector< OUString >::iterator i = aList.begin(); i != aList.end(); ++i, ++nCount )
    {
        sal_uInt16& rCurrencyIndex = aCurrencyList[ nCount ];
        if ( rCurrencyIndex < nLen )
        {
            m_pCurrencyLb->InsertEntry( *i );
            const NfCurrencyEntry& aCurrencyEntry = rCurrencyTable[ rCurrencyIndex ];

            bIsSymbol = nPos >= nLen;

            sal_uInt16 nDefaultFormat = aFormatter.GetCurrencyFormatStrings( aStringsDtor, aCurrencyEntry, bIsSymbol );
            const OUString& rFormatStr = aStringsDtor[ nDefaultFormat ];
            m_aFormatEntries.push_back( rFormatStr );
            if( rFormatStr == m_rSelectedFormat )
                nSelectedPos = nPos;
            ++nPos;
        }
    }
    m_pCurrencyLb->SetSelectHdl( LINK( this, SvxCurrencyList_Impl, SelectHdl ) );
    SetText( SvxResId( RID_SVXSTR_TBLAFMT_CURRENCY ) );
    if ( nSelectedPos >= 0 )
        m_pCurrencyLb->SelectEntryPos( nSelectedPos );
    m_pCurrencyLb->Show();
}

void SvxCurrencyList_Impl::dispose()
{
    m_xControl.clear();
    m_pCurrencyLb.disposeAndClear();
    ToolbarPopup::dispose();
}

SvxLineWindow_Impl::SvxLineWindow_Impl( svt::ToolboxController& rController, vcl::Window* pParentWindow ) :
    ToolbarPopup( rController.getFrameInterface(), pParentWindow, WB_STDPOPUP | WB_MOVEABLE | WB_CLOSEABLE ),
    m_aLineStyleLb( VclPtr<LineListBox>::Create(this) ),
    m_rController( rController )
{
    try
    {
        Reference< lang::XServiceInfo > xServices( rController.getFrameInterface()->getController()->getModel(), UNO_QUERY_THROW );
        m_bIsWriter = xServices->supportsService("com.sun.star.text.TextDocument");
    }
    catch(const uno::Exception& )
    {
    }

    m_aLineStyleLb->setPosSizePixel( 2, 2, 110, 140 );
    SetOutputSizePixel( Size( 114, 144 ) );

    m_aLineStyleLb->SetSourceUnit( FUNIT_TWIP );
    m_aLineStyleLb->SetNone( SvxResId(RID_SVXSTR_NONE) );

    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::SOLID ), SvxBorderLineStyle::SOLID );
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::DOTTED ), SvxBorderLineStyle::DOTTED );
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::DASHED ), SvxBorderLineStyle::DASHED );

    // Double lines
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::DOUBLE ), SvxBorderLineStyle::DOUBLE );
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::THINTHICK_SMALLGAP ), SvxBorderLineStyle::THINTHICK_SMALLGAP, 20 );
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::THINTHICK_MEDIUMGAP ), SvxBorderLineStyle::THINTHICK_MEDIUMGAP );
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::THINTHICK_LARGEGAP ), SvxBorderLineStyle::THINTHICK_LARGEGAP );
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::THICKTHIN_SMALLGAP ), SvxBorderLineStyle::THICKTHIN_SMALLGAP, 20 );
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::THICKTHIN_MEDIUMGAP ), SvxBorderLineStyle::THICKTHIN_MEDIUMGAP );
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::THICKTHIN_LARGEGAP ), SvxBorderLineStyle::THICKTHIN_LARGEGAP );

    // Engraved / Embossed
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::EMBOSSED ), SvxBorderLineStyle::EMBOSSED, 15,
            &SvxBorderLine::threeDLightColor, &SvxBorderLine::threeDDarkColor,
            &lcl_mediumColor );
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::ENGRAVED ), SvxBorderLineStyle::ENGRAVED, 15,
            &SvxBorderLine::threeDDarkColor, &SvxBorderLine::threeDLightColor,
            &lcl_mediumColor );

    // Inset / Outset
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::OUTSET ), SvxBorderLineStyle::OUTSET, 10,
           &SvxBorderLine::lightColor, &SvxBorderLine::darkColor );
    m_aLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::INSET ), SvxBorderLineStyle::INSET, 10,
           &SvxBorderLine::darkColor, &SvxBorderLine::lightColor );
    m_aLineStyleLb->SetWidth( 20 ); // 1pt by default

    m_aLineStyleLb->SetSelectHdl( LINK( this, SvxLineWindow_Impl, SelectHdl ) );

    SetHelpId( HID_POPUP_LINE );
    SetText( SvxResId(RID_SVXSTR_FRAME_STYLE) );
    m_aLineStyleLb->Show();
}

IMPL_LINK_NOARG(SvxCurrencyList_Impl, SelectHdl, ListBox&, void)
{
    VclPtr<SvxCurrencyList_Impl> xThis(this);

    if ( IsInPopupMode() )
        EndPopupMode();

    if (!m_xControl.is())
        return;

    m_rSelectedFormat = m_aFormatEntries[ m_pCurrencyLb->GetSelectedEntryPos() ];
    m_eSelectedLanguage = m_eFormatLanguage;

    m_xControl->execute( m_pCurrencyLb->GetSelectedEntryPos() + 1 );
}

IMPL_LINK_NOARG(SvxLineWindow_Impl, SelectHdl, ListBox&, void)
{
    VclPtr<SvxLineWindow_Impl> xThis(this);

    SvxLineItem     aLineItem( SID_FRAME_LINESTYLE );
    SvxBorderLineStyle  nStyle = m_aLineStyleLb->GetSelectEntryStyle();

    if ( m_aLineStyleLb->GetSelectedEntryPos( ) > 0 )
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

    m_rController.dispatchCommand( ".uno:LineStyle", aArgs );
}

void SvxLineWindow_Impl::Resize()
{
    m_aLineStyleLb->Resize();
}

void SvxLineWindow_Impl::GetFocus()
{
    if ( m_aLineStyleLb )
        m_aLineStyleLb->GrabFocus();
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

    Impl()
        :aClearForm         ( SvxResId( RID_SVXSTR_CLEARFORM ) )
        ,aMore              ( SvxResId( RID_SVXSTR_MORE_STYLES ) )
        ,bSpecModeWriter    ( false )
        ,bSpecModeCalc      ( false )
    {


    }
    void InitializeStyles(const Reference < frame::XModel >& xModel)
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
                for(const char* pCalcStyle : aCalcStyles)
                {
                    try
                    {
                        const OUString sStyleName( OUString::createFromAscii( pCalcStyle ) );
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
        pFamilyState[i].reset();
    }
    pStyleSheetPool = nullptr;
    pImpl.reset();
}

void SAL_CALL SvxStyleToolBoxControl::update()
{
    // Do nothing, we will start binding our listener when we are visible.
    // See link SvxStyleToolBoxControl::VisibilityNotification.
    SvxStyleBox_Impl* pBox = static_cast<SvxStyleBox_Impl*>(GetToolBox().GetItemWindow( GetId() ));
    if ( pBox->IsVisible() )
    {
        for (SfxStyleControllerItem_Impl* pBoundItem : pBoundItems)
            pBoundItem->ReBind();

        bindListener();
    }
}

SfxStyleFamily SvxStyleToolBoxControl::GetActFamily()
{
    switch ( nActFamily-1 + SID_STYLE_FAMILY_START )
    {
        case SID_STYLE_FAMILY1: return SfxStyleFamily::Char;
        case SID_STYLE_FAMILY2: return SfxStyleFamily::Para;
        case SID_STYLE_FAMILY3: return SfxStyleFamily::Frame;
        case SID_STYLE_FAMILY4: return SfxStyleFamily::Page;
        case SID_STYLE_FAMILY5: return SfxStyleFamily::Pseudo;
        default:
            OSL_FAIL( "unknown style family" );
            break;
    }
    return SfxStyleFamily::Para;
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

        SfxStyleSheetIterator aIter( pStyleSheetPool, eFamily, SfxStyleSearchBits::Used );

        // Check whether fill is necessary
        pStyle = aIter.First();
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
                pStyle = aIter.Next();
                i++;
            }
        }

        if ( bDoFill )
        {
            pBox->SetUpdateMode( false );
            pBox->Clear();

            {
                pStyle = aIter.First();

                if( pImpl->bSpecModeWriter || pImpl->bSpecModeCalc )
                {
                    while ( pStyle )
                    {
                        // sort out default styles
                        bool bInsert = true;
                        OUString aName( pStyle->GetName() );
                        for( auto const & _i: pImpl->aDefaultStyles )
                        {
                            if( _i == aName )
                            {
                                bInsert = false;
                                break;
                            }
                        }

                        if( bInsert )
                            pBox->InsertEntry( aName );
                        pStyle = aIter.Next();
                    }
                }
                else
                {
                    while ( pStyle )
                    {
                        pBox->InsertEntry( pStyle->GetName() );
                        pStyle = aIter.Next();
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
                sal_uInt16 nPos = 1;
                for( auto const & _i: pImpl->aDefaultStyles )
                {
                    pBox->InsertEntry( _i, nPos );
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

    if ( nActFamily == 0xffff || nullptr == (pItem = pFamilyState[nActFamily-1].get()) )
    // Current range not within allowed ranges or default
    {
        pStyleSheetPool = pPool;
        nActFamily      = 2;

        pItem = pFamilyState[nActFamily-1].get();
        if ( !pItem )
        {
            nActFamily++;
            pItem = pFamilyState[nActFamily-1].get();
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
    pFamilyState[nIdx].reset( pItem == nullptr ? nullptr : new SfxTemplateItem( *pItem ) );
    Update();
}

IMPL_LINK_NOARG(SvxStyleToolBoxControl, VisibilityNotification, SvxStyleBox_Impl&, void)
{
    // Call ReBind() && UnBind() according to visibility
    SvxStyleBox_Impl* pBox = static_cast<SvxStyleBox_Impl*>( GetToolBox().GetItemWindow( GetId() ));

    if ( pBox && pBox->IsVisible() && !isBound() )
    {
        for (SfxStyleControllerItem_Impl* pBoundItem : pBoundItems)
            pBoundItem->ReBind();

        bindListener();
    }
    else if ( (!pBox || !pBox->IsVisible()) && isBound() )
    {
        for (SfxStyleControllerItem_Impl* pBoundItem : pBoundItems)
            pBoundItem->UnBind();
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
            eTri = static_cast<const SfxTemplateItem*>(pState)->GetValue() != SfxStyleSearchBits::Auto
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
                                           SfxStyleFamily::Para,
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

class SvxFontNameToolBoxControl : public cppu::ImplInheritanceHelper< svt::ToolboxController,
                                                                      css::lang::XServiceInfo >
{
public:
    SvxFontNameToolBoxControl();

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;

    // XToolbarController
    virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createItemWindow( const css::uno::Reference< css::awt::XWindow >& rParent ) override;

    // XComponent
    virtual void SAL_CALL dispose() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

private:
    VclPtr<SvxFontNameBox_Impl> m_pBox;
};

SvxFontNameToolBoxControl::SvxFontNameToolBoxControl()
{
}

void SvxFontNameToolBoxControl::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    SolarMutexGuard aGuard;
    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if ( !getToolboxId( nId, &pToolBox ) )
        return;

    if ( !rEvent.IsEnabled )
    {
        m_pBox->Disable();
        m_pBox->Update( nullptr );
    }
    else
    {
        m_pBox->Enable();

        css::awt::FontDescriptor aFontDesc;
        if ( rEvent.State >>= aFontDesc )
            m_pBox->Update( &aFontDesc );
        else
            m_pBox->SetText( "" );
        m_pBox->SaveValue();
    }

    pToolBox->EnableItem( nId, rEvent.IsEnabled );
}

css::uno::Reference< css::awt::XWindow > SvxFontNameToolBoxControl::createItemWindow( const css::uno::Reference< css::awt::XWindow >& rParent )
{
    SolarMutexGuard aGuard;
    m_pBox = VclPtr<SvxFontNameBox_Impl>::Create( VCLUnoHelper::GetWindow( rParent ),
                                                  Reference< XDispatchProvider >( m_xFrame->getController(), UNO_QUERY ),
                                                  m_xFrame, 0);
    return VCLUnoHelper::GetInterface( m_pBox );
}

void SvxFontNameToolBoxControl::dispose()
{
    m_pBox.disposeAndClear();
    ToolboxController::dispose();
}

OUString SvxFontNameToolBoxControl::getImplementationName()
{
    return OUString( "com.sun.star.comp.svx.FontNameToolBoxControl" );
}

sal_Bool SvxFontNameToolBoxControl::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService( this, rServiceName );
}

css::uno::Sequence< OUString > SvxFontNameToolBoxControl::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_svx_FontNameToolBoxControl_get_implementation(
    css::uno::XComponentContext*,
    css::uno::Sequence<css::uno::Any> const & )
{
    return cppu::acquire( new SvxFontNameToolBoxControl() );
}

SvxColorToolBoxControl::SvxColorToolBoxControl( const css::uno::Reference<css::uno::XComponentContext>& rContext ) :
    ImplInheritanceHelper( rContext, nullptr, OUString() ),
    m_bSplitButton(true),
    m_nSlotId(0),
    m_aColorSelectFunction(PaletteManager::DispatchColorCommand)
{
}

namespace {

sal_uInt16 MapCommandToSlotId(const OUString& rCommand)
{
    if (rCommand == ".uno:Color")
        return SID_ATTR_CHAR_COLOR;
    else if (rCommand == ".uno:FontColor")
        return SID_ATTR_CHAR_COLOR2;
    else if (rCommand == ".uno:BackColor")
        return SID_ATTR_CHAR_COLOR_BACKGROUND;
    else if (rCommand == ".uno:CharBackColor")
        return SID_ATTR_CHAR_BACK_COLOR;
    else if (rCommand == ".uno:BackgroundColor")
        return SID_BACKGROUND_COLOR;
    else if (rCommand == ".uno:Extrusion3DColor")
        return SID_EXTRUSION_3D_COLOR;
    else if (rCommand == ".uno:XLineColor")
        return SID_ATTR_LINE_COLOR;
    else if (rCommand == ".uno:FillColor")
        return SID_ATTR_FILL_COLOR;
    else if (rCommand == ".uno:FrameLineColor")
        return SID_FRAME_LINECOLOR;

    SAL_WARN("svx.tbxcrtls", "Unknown color command: " << rCommand);
    return 0;
}

}

void SvxColorToolBoxControl::initialize( const css::uno::Sequence<css::uno::Any>& rArguments )
{
    PopupWindowController::initialize( rArguments );

    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if ( !getToolboxId( nId, &pToolBox ) )
    {
        SAL_WARN("svx.tbxcrtls", "ToolBox not found!");
        return;
    }

    m_nSlotId = MapCommandToSlotId( m_aCommandURL );
    if ( m_nSlotId == SID_ATTR_LINE_COLOR || m_nSlotId == SID_ATTR_FILL_COLOR ||
         m_nSlotId == SID_FRAME_LINECOLOR || m_nSlotId == SID_BACKGROUND_COLOR )
        // Sidebar uses wide buttons for those.
        m_bSplitButton = typeid( *pToolBox ) != typeid( sfx2::sidebar::SidebarToolBox );

    m_xBtnUpdater.reset( new svx::ToolboxButtonColorUpdater( m_nSlotId, nId, pToolBox, !m_bSplitButton ) );
    pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ( m_bSplitButton ? ToolBoxItemBits::DROPDOWN : ToolBoxItemBits::DROPDOWNONLY ) );
}

void SvxColorToolBoxControl::update()
{
    PopupWindowController::update();

    switch( m_nSlotId )
    {
        case SID_ATTR_CHAR_COLOR2:
            addStatusListener( ".uno:CharColorExt");
            break;

        case SID_ATTR_CHAR_COLOR_BACKGROUND:
            addStatusListener( ".uno:CharBackgroundExt");
            break;

        case SID_FRAME_LINECOLOR:
            addStatusListener( ".uno:BorderTLBR");
            addStatusListener( ".uno:BorderBLTR");
            break;
    }
}

void SvxColorToolBoxControl::EnsurePaletteManager()
{
    if (!m_xPaletteManager)
    {
        m_xPaletteManager.reset(new PaletteManager);
        m_xPaletteManager->SetBtnUpdater(m_xBtnUpdater.get());
    }
}

SvxColorToolBoxControl::~SvxColorToolBoxControl()
{
    if (m_xPaletteManager)
        m_xPaletteManager->SetBtnUpdater(nullptr);
}

void SvxColorToolBoxControl::setColorSelectFunction(const ColorSelectFunction& aColorSelectFunction)
{
    m_aColorSelectFunction = aColorSelectFunction;
    if (m_xPaletteManager)
        m_xPaletteManager->SetColorSelectFunction(aColorSelectFunction);
}

VclPtr<vcl::Window> SvxColorToolBoxControl::createPopupWindow( vcl::Window* pParent )
{
    EnsurePaletteManager();

    VclPtrInstance<SvxColorWindow> pColorWin(
                            m_aCommandURL,
                            m_xPaletteManager,
                            m_aBorderColorStatus,
                            m_nSlotId,
                            m_xFrame,
                            pParent,
                            false,
                            m_aColorSelectFunction);

    OUString aWindowTitle = vcl::CommandInfoProvider::GetLabelForCommand( m_aCommandURL, m_sModuleName );
    pColorWin->SetText( aWindowTitle );
    pColorWin->StartSelection();
    if ( m_bSplitButton )
        pColorWin->SetSelectedHdl( LINK( this, SvxColorToolBoxControl, SelectedHdl ) );
    return pColorWin;
}

IMPL_LINK(SvxColorToolBoxControl, SelectedHdl, const NamedColor&, rColor, void)
{
    m_xBtnUpdater->Update(rColor.first);
}

void SvxColorToolBoxControl::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if ( !getToolboxId( nId, &pToolBox ) )
        return;

    if ( rEvent.FeatureURL.Complete == m_aCommandURL )
        pToolBox->EnableItem( nId, rEvent.IsEnabled );

    bool bValue;
    if ( !m_bSplitButton )
    {
        Color aColor( COL_TRANSPARENT );

        if ( m_aBorderColorStatus.statusChanged( rEvent ) )
        {
            aColor = m_aBorderColorStatus.GetColor();
        }
        else if ( rEvent.IsEnabled )
        {
            rEvent.State >>= aColor;
        }
        m_xBtnUpdater->Update( aColor );
    }
    else if ( rEvent.State >>= bValue )
        pToolBox->CheckItem( nId, bValue );
}

void SvxColorToolBoxControl::execute(sal_Int16 /*nSelectModifier*/)
{
    if ( !m_bSplitButton )
    {
        // Open the popup also when Enter key is pressed.
        createPopupWindow();
        return;
    }

    OUString aCommand = m_aCommandURL;
    Color aColor = m_xBtnUpdater->GetCurrentColor();

    switch( m_nSlotId )
    {
        case SID_ATTR_CHAR_COLOR2 :
            aCommand    = ".uno:CharColorExt";
            break;

        case SID_ATTR_CHAR_COLOR_BACKGROUND :
            aCommand    = ".uno:CharBackgroundExt";
            break;
    }

    auto aArgs( comphelper::InitPropertySequence( {
        { m_aCommandURL.copy(5), css::uno::makeAny(aColor) }
    } ) );
    dispatchCommand( aCommand, aArgs );

    EnsurePaletteManager();
    OUString sColorName = ("#" + aColor.AsRGBHexString().toAsciiUpperCase());
    m_xPaletteManager->AddRecentColor(aColor, sColorName);
}

sal_Bool SvxColorToolBoxControl::opensSubToolbar()
{
    // For a split button, we mark this controller as a sub-toolbar controller,
    // so we get notified (through updateImage method) on button image changes,
    // and could redraw the last used color on top of it.
    return m_bSplitButton;
}

void SvxColorToolBoxControl::updateImage()
{
    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if ( !getToolboxId( nId, &pToolBox ) )
        return;

    Image aImage = vcl::CommandInfoProvider::GetImageForCommand(m_aCommandURL, m_xFrame, pToolBox->GetImageSize());
    if ( !!aImage )
    {
        pToolBox->SetItemImage( nId, aImage );
        m_xBtnUpdater->Update(m_xBtnUpdater->GetCurrentColor(), true);
    }
}

OUString SvxColorToolBoxControl::getSubToolbarName()
{
    return OUString();
}

void SvxColorToolBoxControl::functionSelected( const OUString& /*rCommand*/ )
{
}

OUString SvxColorToolBoxControl::getImplementationName()
{
    return OUString( "com.sun.star.comp.svx.ColorToolBoxControl" );
}

css::uno::Sequence<OUString> SvxColorToolBoxControl::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_svx_ColorToolBoxControl_get_implementation(
    css::uno::XComponentContext* rContext,
    css::uno::Sequence<css::uno::Any> const & )
{
    return cppu::acquire( new SvxColorToolBoxControl( rContext ) );
}

// class SvxFrameToolBoxControl --------------------------------------------

class SvxFrameToolBoxControl : public svt::PopupWindowController
{
public:
    explicit SvxFrameToolBoxControl( const css::uno::Reference< css::uno::XComponentContext >& rContext );

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& rArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

private:
    virtual VclPtr<vcl::Window> createPopupWindow( vcl::Window* pParent ) override;
    using svt::ToolboxController::createPopupWindow;
};

SvxFrameToolBoxControl::SvxFrameToolBoxControl( const css::uno::Reference< css::uno::XComponentContext >& rContext )
    : svt::PopupWindowController( rContext, nullptr, OUString() )
{
}

void SvxFrameToolBoxControl::initialize( const css::uno::Sequence< css::uno::Any >& rArguments )
{
    svt::PopupWindowController::initialize( rArguments );
    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if ( getToolboxId( nId, &pToolBox ) )
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWNONLY );
}

VclPtr<vcl::Window> SvxFrameToolBoxControl::createPopupWindow( vcl::Window* pParent )
{
    if ( m_aCommandURL == ".uno:LineStyle" )
        return VclPtr<SvxLineWindow_Impl>::Create( *this, pParent );

    return VclPtr<SvxFrameWindow_Impl>::Create( *this, pParent );
}

OUString SvxFrameToolBoxControl::getImplementationName()
{
    return OUString( "com.sun.star.comp.svx.FrameToolBoxControl" );
}

css::uno::Sequence< OUString > SvxFrameToolBoxControl::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_svx_FrameToolBoxControl_get_implementation(
    css::uno::XComponentContext* rContext,
    css::uno::Sequence<css::uno::Any> const & )
{
    return cppu::acquire( new SvxFrameToolBoxControl( rContext ) );
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

SvxCurrencyToolBoxControl::SvxCurrencyToolBoxControl( const css::uno::Reference<css::uno::XComponentContext>& rContext ) :
    PopupWindowController( rContext, nullptr, OUString() ),
    m_eLanguage( Application::GetSettings().GetLanguageTag().getLanguageType() ),
    m_nFormatKey( NUMBERFORMAT_ENTRY_NOT_FOUND )
{
}

SvxCurrencyToolBoxControl::~SvxCurrencyToolBoxControl() {}

void SvxCurrencyToolBoxControl::initialize( const css::uno::Sequence< css::uno::Any >& rArguments )
{
    PopupWindowController::initialize(rArguments);

    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if (getToolboxId(nId, &pToolBox) && pToolBox->GetItemCommand(nId) == m_aCommandURL)
        pToolBox->SetItemBits(nId, ToolBoxItemBits::DROPDOWN | pToolBox->GetItemBits(nId));
}

VclPtr<vcl::Window> SvxCurrencyToolBoxControl::createPopupWindow( vcl::Window* pParent )
{
    return VclPtr<SvxCurrencyList_Impl>::Create(this, pParent, m_aFormatString, m_eLanguage);
}

void SvxCurrencyToolBoxControl::execute( sal_Int16 nSelectModifier )
{
    sal_uInt32 nFormatKey;
    if (m_aFormatString.isEmpty())
        nFormatKey = NUMBERFORMAT_ENTRY_NOT_FOUND;
    else
    {
        if ( nSelectModifier > 0 )
        {
            try
            {
                uno::Reference< util::XNumberFormatsSupplier > xRef( m_xFrame->getController()->getModel(), uno::UNO_QUERY );
                uno::Reference< util::XNumberFormats > rxNumberFormats( xRef->getNumberFormats(), uno::UNO_QUERY_THROW );
                css::lang::Locale aLocale = LanguageTag::convertToLocale( m_eLanguage );
                nFormatKey = rxNumberFormats->queryKey( m_aFormatString, aLocale, false );
                if ( nFormatKey == NUMBERFORMAT_ENTRY_NOT_FOUND )
                    nFormatKey = rxNumberFormats->addNew( m_aFormatString, aLocale );
                }
                catch( const uno::Exception& )
                {
                    nFormatKey = m_nFormatKey;
                }
        }
        else
            nFormatKey = m_nFormatKey;
    }

    if( nFormatKey != NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = "NumberFormatCurrency";
        aArgs[0].Value <<= nFormatKey;
        dispatchCommand( m_aCommandURL, aArgs );
        m_nFormatKey = nFormatKey;
    }
    else
        PopupWindowController::execute( nSelectModifier );
}

OUString SvxCurrencyToolBoxControl::getImplementationName()
{
    return OUString( "com.sun.star.comp.svx.CurrencyToolBoxControl" );
}

css::uno::Sequence<OUString> SvxCurrencyToolBoxControl::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_svx_CurrencyToolBoxControl_get_implementation(
    css::uno::XComponentContext* rContext,
    css::uno::Sequence<css::uno::Any> const & )
{
    return cppu::acquire( new SvxCurrencyToolBoxControl( rContext ) );
}

Reference< css::accessibility::XAccessible > SvxFontNameBox_Impl::CreateAccessible()
{
    FillList();
    return FontNameBox::CreateAccessible();
}

//static
void SvxCurrencyToolBoxControl::GetCurrencySymbols( std::vector<OUString>& rList, bool bFlag,
                                                    std::vector<sal_uInt16>& rCurrencyList )
{
    rCurrencyList.clear();

    const NfCurrencyTable& rCurrencyTable = SvNumberFormatter::GetTheCurrencyTable();
    sal_uInt16 nCount = rCurrencyTable.size();

    sal_uInt16 nStart = 1;

    OUString aString( ApplyLreOrRleEmbedding( rCurrencyTable[0].GetSymbol() ) );
    aString += " ";
    aString += ApplyLreOrRleEmbedding( SvtLanguageTable::GetLanguageString(
                                       rCurrencyTable[0].GetLanguage() ) );

    rList.push_back( aString );
    rCurrencyList.push_back( sal_uInt16(-1) ); // nAuto

    if( bFlag )
    {
        rList.push_back( aString );
        rCurrencyList.push_back( 0 );
        ++nStart;
    }

    CollatorWrapper aCollator( ::comphelper::getProcessComponentContext() );
    aCollator.loadDefaultCollator( Application::GetSettings().GetLanguageTag().getLocale(), 0 );

    const OUString aTwoSpace("  ");

    for( sal_uInt16 i = 1; i < nCount; ++i )
    {
        OUString aStr( ApplyLreOrRleEmbedding( rCurrencyTable[i].GetBankSymbol() ) );
        aStr += aTwoSpace;
        aStr += ApplyLreOrRleEmbedding( rCurrencyTable[i].GetSymbol() );
        aStr += aTwoSpace;
        aStr += ApplyLreOrRleEmbedding( SvtLanguageTable::GetLanguageString(
                                        rCurrencyTable[i].GetLanguage() ) );

        std::vector<OUString>::size_type j = nStart;
        for( ; j < rList.size(); ++j )
            if ( aCollator.compareString( aStr, rList[j] ) < 0 )
                break;  // insert before first greater than

        rList.insert( rList.begin() + j, aStr );
        rCurrencyList.insert( rCurrencyList.begin() + j, i );
    }

    // Append ISO codes to symbol list.
    // XXX If this is to be changed, various other places would had to be
    // adapted that assume this order!
    std::vector<OUString>::size_type nCont = rList.size();

    for ( sal_uInt16 i = 1; i < nCount; ++i )
    {
        bool bInsert = true;
        OUString aStr( ApplyLreOrRleEmbedding( rCurrencyTable[i].GetBankSymbol() ) );

        std::vector<OUString>::size_type j = nCont;
        for ( ; j < rList.size() && bInsert; ++j )
        {
            if( rList[j] == aStr )
                bInsert = false;
            else if ( aCollator.compareString( aStr, rList[j] ) < 0 )
                break;  // insert before first greater than
        }
        if ( bInsert )
        {
            rList.insert( rList.begin() + j, aStr );
            rCurrencyList.insert( rCurrencyList.begin() + j, i );
        }
    }
}

SvxListBoxColorWrapper::SvxListBoxColorWrapper(SvxColorListBox* pControl)
    : mxControl(pControl)
{
}

void SvxListBoxColorWrapper::operator()(const OUString& /*rCommand*/, const NamedColor& rColor)
{
    if (!mxControl)
        return;
    mxControl->Selected(rColor);
}

void SvxListBoxColorWrapper::dispose()
{
    mxControl.clear();
}

ListBoxColorWrapper::ListBoxColorWrapper(ColorListBox* pControl)
    : mpControl(pControl)
{
}

void ListBoxColorWrapper::operator()(const OUString& /*rCommand*/, const NamedColor& rColor)
{
    mpControl->Selected(rColor);
}

SvxColorListBox::SvxColorListBox(vcl::Window* pParent, WinBits nStyle)
    : MenuButton(pParent, nStyle)
    , m_aColorWrapper(this)
    , m_aAutoDisplayColor(Application::GetSettings().GetStyleSettings().GetDialogColor())
    , m_nSlotId(0)
    , m_bShowNoneButton(false)
{
    m_aSelectedColor = GetAutoColor(m_nSlotId);
    LockWidthRequest();
    ShowPreview(m_aSelectedColor);
    SetActivateHdl(LINK(this, SvxColorListBox, MenuActivateHdl));
}

void SvxColorListBox::EnsurePaletteManager()
{
    if (!m_xPaletteManager)
    {
        m_xPaletteManager.reset(new PaletteManager);
        m_xPaletteManager->SetColorSelectFunction(std::ref(m_aColorWrapper));
    }
}

void ColorListBox::EnsurePaletteManager()
{
    if (!m_xPaletteManager)
    {
        m_xPaletteManager.reset(new PaletteManager);
        m_xPaletteManager->SetColorSelectFunction(std::ref(m_aColorWrapper));
    }
}

void SvxColorListBox::SetSlotId(sal_uInt16 nSlotId, bool bShowNoneButton)
{
    m_nSlotId = nSlotId;
    m_bShowNoneButton = bShowNoneButton;
    m_xColorWindow.disposeAndClear();
    m_aSelectedColor = bShowNoneButton ? GetNoneColor() : GetAutoColor(m_nSlotId);
    ShowPreview(m_aSelectedColor);
    createColorWindow();
}

//to avoid the box resizing every time the color is changed to
//the optimal size of the individual color, get the longest
//standard color and stick with that as the size for all
void SvxColorListBox::LockWidthRequest()
{
    if (get_width_request() != -1)
        return;
    NamedColor aLongestColor;
    long nMaxStandardColorTextWidth = 0;
    XColorListRef const xColorTable = XColorList::CreateStdColorList();
    for (long i = 0; i != xColorTable->Count(); ++i)
    {
        XColorEntry& rEntry = *xColorTable->GetColor(i);
        long nColorTextWidth = GetTextWidth(rEntry.GetName());
        if (nColorTextWidth > nMaxStandardColorTextWidth)
        {
            nMaxStandardColorTextWidth = nColorTextWidth;
            aLongestColor.second = rEntry.GetName();
        }
    }
    ShowPreview(aLongestColor);
    set_width_request(get_preferred_size().Width());
}

void SvxColorListBox::ShowPreview(const NamedColor &rColor)
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Size aImageSize(rStyleSettings.GetListBoxPreviewDefaultPixelSize());

    ScopedVclPtrInstance<VirtualDevice> xDevice;
    xDevice->SetOutputSize(aImageSize);
    const tools::Rectangle aRect(Point(0, 0), aImageSize);
    if (m_bShowNoneButton && rColor.first == COL_NONE_COLOR)
    {
        const Color aW(COL_WHITE);
        const Color aG(0xef, 0xef, 0xef);
        xDevice->DrawCheckered(aRect.TopLeft(), aRect.GetSize(), 8, aW, aG);
        xDevice->SetFillColor();
    }
    else
    {
        if (rColor.first == COL_AUTO)
            xDevice->SetFillColor(m_aAutoDisplayColor);
        else
            xDevice->SetFillColor(rColor.first);
    }

    xDevice->SetLineColor(rStyleSettings.GetDisableColor());
    xDevice->DrawRect(aRect);

    BitmapEx aBitmap(xDevice->GetBitmapEx(Point(0, 0), xDevice->GetOutputSize()));
    SetImageAlign(ImageAlign::Left);
    SetModeImage(Image(aBitmap));
    SetText(rColor.second);
}

IMPL_LINK(SvxColorListBox, WindowEventListener, VclWindowEvent&, rWindowEvent, void)
{
    if (rWindowEvent.GetId() == VclEventId::WindowEndPopupMode)
    {
        m_xColorWindow.disposeAndClear();
        SetPopover(nullptr);
    }
}

IMPL_LINK_NOARG(SvxColorListBox, MenuActivateHdl, MenuButton *, void)
{
    if (!m_xColorWindow || m_xColorWindow->isDisposed())
        createColorWindow();
}

void SvxColorListBox::createColorWindow()
{
    const SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    const SfxFrame* pFrame = pViewFrame ? &pViewFrame->GetFrame() : nullptr;
    css::uno::Reference<css::frame::XFrame> xFrame(pFrame ? pFrame->GetFrameInterface() : uno::Reference<css::frame::XFrame>());

    EnsurePaletteManager();

    m_xColorWindow = VclPtr<SvxColorWindow>::Create(
                            OUString() /*m_aCommandURL*/,
                            m_xPaletteManager,
                            m_aBorderColorStatus,
                            m_nSlotId,
                            xFrame,
                            this,
                            true,
                            m_aColorWrapper);

    m_xColorWindow->AddEventListener(LINK(this, SvxColorListBox, WindowEventListener));

    SetNoSelection();
    if (m_bShowNoneButton)
        m_xColorWindow->ShowNoneButton();
    m_xColorWindow->SelectEntry(m_aSelectedColor);
    SetPopover(m_xColorWindow);
}

void SvxColorListBox::Selected(const NamedColor& rColor)
{
    ShowPreview(rColor);
    m_aSelectedColor = rColor;
    if (m_aSelectedLink.IsSet())
        m_aSelectedLink.Call(*this);
}

VCL_BUILDER_FACTORY(SvxColorListBox)

SvxColorListBox::~SvxColorListBox()
{
    disposeOnce();
}

void SvxColorListBox::dispose()
{
    m_xColorWindow.disposeAndClear();
    m_aColorWrapper.dispose();
    MenuButton::dispose();
}

VclPtr<SvxColorWindow> const & SvxColorListBox::getColorWindow() const
{
    if (!m_xColorWindow || m_xColorWindow->isDisposed())
        const_cast<SvxColorListBox*>(this)->createColorWindow();
    return m_xColorWindow;
}

void SvxColorListBox::SelectEntry(const NamedColor& rColor)
{
    if (rColor.second.trim().isEmpty())
    {
        SelectEntry(rColor.first);
        return;
    }
    VclPtr<SvxColorWindow> xColorWindow = getColorWindow();
    xColorWindow->SelectEntry(rColor);
    m_aSelectedColor = xColorWindow->GetSelectEntryColor();
    ShowPreview(m_aSelectedColor);
}

void SvxColorListBox::SelectEntry(const Color& rColor)
{
    VclPtr<SvxColorWindow> xColorWindow = getColorWindow();
    xColorWindow->SelectEntry(rColor);
    m_aSelectedColor = xColorWindow->GetSelectEntryColor();
    ShowPreview(m_aSelectedColor);
}

ColorListBox::ColorListBox(weld::MenuButton* pControl, weld::Window* pTopLevel)
    : m_xButton(pControl)
    , m_pTopLevel(pTopLevel)
    , m_aColorWrapper(this)
    , m_aAutoDisplayColor(Application::GetSettings().GetStyleSettings().GetDialogColor())
{
    m_aSelectedColor = GetAutoColor(0);
    LockWidthRequest();
    ShowPreview(m_aSelectedColor);
}

ColorListBox::~ColorListBox()
{
}

ColorWindow* ColorListBox::getColorWindow() const
{
    if (!m_xColorWindow)
        const_cast<ColorListBox*>(this)->createColorWindow();
    return m_xColorWindow.get();
}

void ColorListBox::createColorWindow()
{
    const SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    const SfxFrame* pFrame = pViewFrame ? &pViewFrame->GetFrame() : nullptr;
    css::uno::Reference<css::frame::XFrame> xFrame(pFrame ? pFrame->GetFrameInterface() : uno::Reference<css::frame::XFrame>());

    EnsurePaletteManager();

    m_xColorWindow.reset(new ColorWindow(
                            m_xPaletteManager,
                            m_aBorderColorStatus,
                            0, // slotID
                            xFrame,
                            m_pTopLevel,
                            m_xButton.get(),
                            m_aColorWrapper));

    SetNoSelection();
    m_xButton->set_popover(m_xColorWindow->GetWidget());
    m_xColorWindow->SelectEntry(m_aSelectedColor);
}

void ColorListBox::SelectEntry(const Color& rColor)
{
    ColorWindow* pColorWindow = getColorWindow();
    pColorWindow->SelectEntry(rColor);
    m_aSelectedColor = pColorWindow->GetSelectEntryColor();
    ShowPreview(m_aSelectedColor);
}

void ColorListBox::Selected(const NamedColor& rColor)
{
    ShowPreview(rColor);
    m_aSelectedColor = rColor;
}

//to avoid the box resizing every time the color is changed to
//the optimal size of the individual color, get the longest
//standard color and stick with that as the size for all
void ColorListBox::LockWidthRequest()
{
    NamedColor aLongestColor;
    long nMaxStandardColorTextWidth = 0;
    XColorListRef const xColorTable = XColorList::CreateStdColorList();
    for (long i = 0; i != xColorTable->Count(); ++i)
    {
        XColorEntry& rEntry = *xColorTable->GetColor(i);
        auto nColorTextWidth = m_xButton->get_pixel_size(rEntry.GetName()).Width();
        if (nColorTextWidth > nMaxStandardColorTextWidth)
        {
            nMaxStandardColorTextWidth = nColorTextWidth;
            aLongestColor.second = rEntry.GetName();
        }
    }
    ShowPreview(aLongestColor);
    m_xButton->set_size_request(m_xButton->get_preferred_size().Width(), -1);
}

void ColorListBox::ShowPreview(const NamedColor &rColor)
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Size aImageSize(rStyleSettings.GetListBoxPreviewDefaultPixelSize());

    ScopedVclPtrInstance<VirtualDevice> xDevice;
    xDevice->SetOutputSize(aImageSize);
    const tools::Rectangle aRect(Point(0, 0), aImageSize);
    if (rColor.first == COL_AUTO)
        xDevice->SetFillColor(m_aAutoDisplayColor);
    else
        xDevice->SetFillColor(rColor.first);

    xDevice->SetLineColor(rStyleSettings.GetDisableColor());
    xDevice->DrawRect(aRect);

    m_xButton->set_image(*xDevice);
    m_xButton->set_label(rColor.second);
}

SvxColorListBoxWrapper::SvxColorListBoxWrapper(SvxColorListBox& rListBox)
    : sfx::SingleControlWrapper<SvxColorListBox, Color>(rListBox)
{
}

SvxColorListBoxWrapper::~SvxColorListBoxWrapper()
{
}

bool SvxColorListBoxWrapper::IsControlDontKnow() const
{
    return GetControl().IsNoSelection();
}

void SvxColorListBoxWrapper::SetControlDontKnow( bool bSet )
{
    if( bSet ) GetControl().SetNoSelection();
}

Color SvxColorListBoxWrapper::GetControlValue() const
{
    return GetControl().GetSelectEntryColor();
}

void SvxColorListBoxWrapper::SetControlValue( Color aColor )
{
    GetControl().SelectEntry( aColor );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

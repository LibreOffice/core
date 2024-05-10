/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/text/DefaultNumberingProvider.hpp>
#include <com/sun/star/text/XNumberingFormatter.hpp>

#include <comphelper/propertysequence.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <svtools/popupwindowcontroller.hxx>
#include <svtools/toolbarmenu.hxx>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/numvset.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

namespace {

class NumberingToolBoxControl;

class NumberingPopup : public WeldToolbarPopup
{
    NumberingPageType mePageType;
    NumberingToolBoxControl& mrController;
    std::unique_ptr<SvxNumValueSet> mxValueSet;
    std::unique_ptr<weld::CustomWeld> mxValueSetWin;
    std::unique_ptr<weld::Button> mxMoreButton;
    DECL_LINK(VSSelectValueSetHdl, ValueSet*, void);
    DECL_LINK(VSButtonClickSetHdl, weld::Button&, void);

    virtual void GrabFocus() override;

public:
    NumberingPopup(NumberingToolBoxControl& rController, weld::Widget* pParent, NumberingPageType ePageType);

    virtual void statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;
};

class NumberingToolBoxControl : public svt::PopupWindowController
{
    NumberingPageType mePageType;

public:
    explicit NumberingToolBoxControl( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual VclPtr<vcl::Window> createVclPopupWindow( vcl::Window* pParent ) override;
    std::unique_ptr<WeldToolbarPopup> weldPopupWindow() override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

}

NumberingPopup::NumberingPopup(NumberingToolBoxControl& rController,
                               weld::Widget* pParent, NumberingPageType ePageType)
    : WeldToolbarPopup(rController.getFrameInterface(), pParent, u"svx/ui/numberingwindow.ui"_ustr, u"NumberingWindow"_ustr)
    , mePageType(ePageType)
    , mrController(rController)
    , mxValueSet(new SvxNumValueSet(m_xBuilder->weld_scrolled_window(u"valuesetwin"_ustr, true)))
    , mxValueSetWin(new weld::CustomWeld(*m_xBuilder, u"valueset"_ustr, *mxValueSet))
    , mxMoreButton(m_xBuilder->weld_button(u"more"_ustr))
{
    mxValueSet->SetStyle(WB_MENUSTYLEVALUESET | WB_FLATVALUESET | WB_NO_DIRECTSELECT);
    mxValueSet->init(mePageType);

    if ( mePageType != NumberingPageType::BULLET )
    {
        css::uno::Reference< css::text::XDefaultNumberingProvider > xDefNum = css::text::DefaultNumberingProvider::create( mrController.getContext() );
        if ( xDefNum.is() )
        {
            css::lang::Locale aLocale = Application::GetSettings().GetLanguageTag().getLocale();
            css::uno::Reference< css::text::XNumberingFormatter > xFormat( xDefNum, css::uno::UNO_QUERY );

            if ( mePageType == NumberingPageType::SINGLENUM )
            {
                css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > aNumberings(
                    xDefNum->getDefaultContinuousNumberingLevels( aLocale ) );
                mxValueSet->SetNumberingSettings( aNumberings, xFormat, aLocale );
            }
            else if ( mePageType == NumberingPageType::OUTLINE )
            {
                css::uno::Sequence< css::uno::Reference< css::container::XIndexAccess > > aOutline(
                    xDefNum->getDefaultOutlineNumberings( aLocale ) );
                mxValueSet->SetOutlineNumberingSettings( aOutline, xFormat, aLocale );
            }
        }
    }

    weld::DrawingArea* pDrawingArea = mxValueSet->GetDrawingArea();
    OutputDevice& rRefDevice = pDrawingArea->get_ref_device();
    Size aItemSize(rRefDevice.LogicToPixel(Size(30, 42), MapMode(MapUnit::MapAppFont)));
    mxValueSet->SetExtraSpacing( 2 );
    Size aSize(mxValueSet->CalcWindowSizePixel(aItemSize));
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
    mxValueSet->SetOutputSizePixel(aSize);
    mxValueSet->SetColor(Application::GetSettings().GetStyleSettings().GetFieldColor());

    if ( mePageType == NumberingPageType::BULLET )
        AddStatusListener( u".uno:CurrentBulletListType"_ustr );
    else if ( mePageType == NumberingPageType::SINGLENUM )
        AddStatusListener( u".uno:CurrentNumListType"_ustr );
    else
        AddStatusListener( u".uno:CurrentOutlineType"_ustr );

    auto xImage = vcl::CommandInfoProvider::GetXGraphicForCommand(u".uno:OutlineBullet"_ustr, mrController.getFrameInterface());
    mxMoreButton->set_image(xImage);
    mxMoreButton->connect_clicked(LINK(this, NumberingPopup, VSButtonClickSetHdl));

    mxValueSet->SetSelectHdl(LINK(this, NumberingPopup, VSSelectValueSetHdl));
}

void NumberingPopup::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    mxValueSet->SetNoSelection();

    sal_Int32 nSelItem;
    if ( rEvent.State >>= nSelItem )
        mxValueSet->SelectItem( nSelItem );
}

IMPL_LINK_NOARG(NumberingPopup, VSSelectValueSetHdl, ValueSet*, void)
{
    sal_uInt16 nSelItem = mxValueSet->GetSelectedItemId();
    if ( mePageType == NumberingPageType::BULLET )
    {
        auto aArgs( comphelper::InitPropertySequence( { { "SetBullet", css::uno::Any( nSelItem ) } } ) );
        mrController.dispatchCommand( u".uno:SetBullet"_ustr, aArgs );
    }
    else if ( mePageType == NumberingPageType::SINGLENUM )
    {
        auto aArgs( comphelper::InitPropertySequence( { { "SetNumber", css::uno::Any( nSelItem ) } } ) );
        mrController.dispatchCommand( u".uno:SetNumber"_ustr, aArgs );
    }
    else
    {
        auto aArgs( comphelper::InitPropertySequence( { { "SetOutline", css::uno::Any( nSelItem ) } } ) );
        mrController.dispatchCommand( u".uno:SetOutline"_ustr, aArgs );
    }
    mrController.EndPopupMode();
}

void NumberingPopup::GrabFocus()
{
    mxValueSet->GrabFocus();
}

IMPL_LINK_NOARG(NumberingPopup, VSButtonClickSetHdl, weld::Button&, void)
{
    auto aArgs( comphelper::InitPropertySequence( { { "Page", css::uno::Any( u"customize"_ustr ) } } ) );
    mrController.dispatchCommand( u".uno:OutlineBullet"_ustr, aArgs );

    mrController.EndPopupMode();
}

NumberingToolBoxControl::NumberingToolBoxControl( const css::uno::Reference< css::uno::XComponentContext >& rxContext ):
    svt::PopupWindowController( rxContext, css::uno::Reference< css::frame::XFrame >(), OUString() ),
    mePageType( NumberingPageType::SINGLENUM )
{
}

std::unique_ptr<WeldToolbarPopup> NumberingToolBoxControl::weldPopupWindow()
{
    return std::make_unique<NumberingPopup>(*this, m_pToolbar, mePageType);
}

VclPtr<vcl::Window> NumberingToolBoxControl::createVclPopupWindow( vcl::Window* pParent )
{
    mxInterimPopover = VclPtr<InterimToolbarPopup>::Create(getFrameInterface(), pParent,
        std::make_unique<NumberingPopup>(*this, pParent->GetFrameWeld(), mePageType));

    mxInterimPopover->Show();

    return mxInterimPopover;
}

void SAL_CALL NumberingToolBoxControl::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    svt::PopupWindowController::initialize( aArguments );

    if ( m_aCommandURL == ".uno:DefaultBullet" )
        mePageType = NumberingPageType::BULLET;
    else if ( m_aCommandURL == ".uno:SetOutline" )
        mePageType = NumberingPageType::OUTLINE;

    if (m_pToolbar)
    {
        mxPopoverContainer.reset(new ToolbarPopupContainer(m_pToolbar));
        m_pToolbar->set_item_popover(m_aCommandURL, mxPopoverContainer->getTopLevel());
        return;
    }

    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if (getToolboxId(nId, &pToolBox))
    {
        ToolBoxItemBits nBits = mePageType == NumberingPageType::OUTLINE
                                    ? ToolBoxItemBits::DROPDOWNONLY
                                    : ToolBoxItemBits::DROPDOWN;
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | nBits );
    }
}

OUString SAL_CALL NumberingToolBoxControl::getImplementationName()
{
    return u"com.sun.star.comp.svx.NumberingToolBoxControl"_ustr;
}

css::uno::Sequence< OUString > SAL_CALL NumberingToolBoxControl::getSupportedServiceNames()
{
    return { u"com.sun.star.frame.ToolbarController"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_svx_NumberingToolBoxControl_get_implementation(
    css::uno::XComponentContext *rxContext,
    css::uno::Sequence<css::uno::Any> const & )
{
    return cppu::acquire( new NumberingToolBoxControl( rxContext ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#include <i18nlangtag/mslangid.hxx>
#include <svtools/popupwindowcontroller.hxx>
#include <svtools/toolbarmenu.hxx>
#include <sfx2/weldutils.hxx>
#include <sfx2/tbxctrl.hxx>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/numvset.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/settings.hxx>

namespace {

class NumberingToolBoxControl;

class NumberingPopup : public WeldToolbarPopup
{
    NumberingPageType const mePageType;
    NumberingToolBoxControl& mrController;
    std::unique_ptr<NumValueSet> mxValueSet;
    std::unique_ptr<weld::CustomWeld> mxValueSetWin;
    std::unique_ptr<weld::Button> mxMoreButton;
    DECL_LINK(VSSelectValueSetHdl, SvtValueSet*, void);
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
    virtual VclPtr<vcl::Window> createPopupWindow( vcl::Window* pParent ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    using svt::ToolboxController::createPopupWindow;
};

}

NumberingPopup::NumberingPopup(NumberingToolBoxControl& rController,
                               weld::Widget* pParent, NumberingPageType ePageType)
    : WeldToolbarPopup(rController.getFrameInterface(), pParent, "svx/ui/numberingwindow.ui", "NumberingWindow")
    , mePageType(ePageType)
    , mrController(rController)
    , mxValueSet(new NumValueSet(nullptr))
    , mxValueSetWin(new weld::CustomWeld(*m_xBuilder, "valueset", *mxValueSet))
    , mxMoreButton(m_xBuilder->weld_button("more"))
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

    OUString aMoreItemText;
    if ( mePageType == NumberingPageType::BULLET )
    {
        aMoreItemText = SvxResId( RID_SVXSTR_MOREBULLETS );
        AddStatusListener( ".uno:CurrentBulletListType" );
    }
    else if ( mePageType == NumberingPageType::SINGLENUM )
    {
        aMoreItemText = SvxResId( RID_SVXSTR_MORENUMBERING );
        AddStatusListener( ".uno:CurrentNumListType" );
    }
    else
    {
        aMoreItemText = SvxResId( RID_SVXSTR_MORE );
        AddStatusListener( ".uno:CurrentOutlineType" );
    }

    auto xImage = vcl::CommandInfoProvider::GetXGraphicForCommand(".uno:OutlineBullet", mrController.getFrameInterface());
    mxMoreButton->set_image(xImage);
    mxMoreButton->set_label(aMoreItemText);
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

IMPL_LINK_NOARG(NumberingPopup, VSSelectValueSetHdl, SvtValueSet*, void)
{
    mrController.EndPopupMode();

    sal_uInt16 nSelItem = mxValueSet->GetSelectedItemId();
    if ( mePageType == NumberingPageType::BULLET )
    {
        auto aArgs( comphelper::InitPropertySequence( { { "SetBullet", css::uno::makeAny( nSelItem ) } } ) );
        mrController.dispatchCommand( ".uno:SetBullet", aArgs );
    }
    else if ( mePageType == NumberingPageType::SINGLENUM )
    {
        auto aArgs( comphelper::InitPropertySequence( { { "SetNumber", css::uno::makeAny( nSelItem ) } } ) );
        mrController.dispatchCommand( ".uno:SetNumber", aArgs );
    }
    else
    {
        auto aArgs( comphelper::InitPropertySequence( { { "SetOutline", css::uno::makeAny( nSelItem ) } } ) );
        mrController.dispatchCommand( ".uno:SetOutline", aArgs );
    }
}

void NumberingPopup::GrabFocus()
{
    mxValueSet->GrabFocus();
}

IMPL_LINK_NOARG(NumberingPopup, VSButtonClickSetHdl, weld::Button&, void)
{
    mrController.EndPopupMode();

    auto aArgs( comphelper::InitPropertySequence( { { "Page", css::uno::makeAny( OUString("customize") ) } } ) );
    mrController.dispatchCommand( ".uno:OutlineBullet", aArgs );
}

NumberingToolBoxControl::NumberingToolBoxControl( const css::uno::Reference< css::uno::XComponentContext >& rxContext ):
    svt::PopupWindowController( rxContext, css::uno::Reference< css::frame::XFrame >(), OUString() ),
    mePageType( NumberingPageType::SINGLENUM )
{
}

VclPtr<vcl::Window> NumberingToolBoxControl::createPopupWindow( vcl::Window* pParent )
{
    mxInterimPopover = VclPtr<InterimToolbarPopup>::Create(getFrameInterface(), pParent, mxPopover.get());

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

    ToolBoxItemBits nBits = ( mePageType == NumberingPageType::OUTLINE ) ? ToolBoxItemBits::DROPDOWNONLY : ToolBoxItemBits::DROPDOWN;
    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    bool bVcl = getToolboxId(nId, &pToolBox);

    weld::Widget* pParent;
    if (pToolBox)
        pParent = pToolBox->GetFrameWeld();
    else
        pParent = m_pToolbar;
    mxPopover = std::make_unique<NumberingPopup>(*this, pParent, mePageType);

    if (bVcl)
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | nBits );
    else if (m_pToolbar)
    {
        const OString aId(m_aCommandURL.toUtf8());
        m_pToolbar->set_item_popover(aId, mxPopover->getTopLevel());
    }
}

OUString SAL_CALL NumberingToolBoxControl::getImplementationName()
{
    return "com.sun.star.comp.svx.NumberingToolBoxControl";
}

css::uno::Sequence< OUString > SAL_CALL NumberingToolBoxControl::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_svx_NumberingToolBoxControl_get_implementation(
    css::uno::XComponentContext *rxContext,
    css::uno::Sequence<css::uno::Any> const & )
{
    return cppu::acquire( new NumberingToolBoxControl( rxContext ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

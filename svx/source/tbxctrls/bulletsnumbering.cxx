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
#include <com/sun/star/uno/Sequence.hxx>

#include <comphelper/propertysequence.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <officecfg/Office/Common.hxx>
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
    std::unique_ptr<SvxNumValueSet> mxValueSetDoc;
    std::unique_ptr<weld::CustomWeld> mxValueSetWinDoc;
    std::unique_ptr<weld::Button> mxMoreButton;
    std::unique_ptr<weld::Label> mxBulletsLabel;
    std::unique_ptr<weld::Label> mxDocBulletsLabel;
    DECL_LINK(VSSelectValueSetHdl, ValueSet*, void);
    DECL_LINK(VSSelectValueSetDocHdl, ValueSet*, void);
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
    : WeldToolbarPopup(rController.getFrameInterface(), pParent, "svx/ui/numberingwindow.ui", "NumberingWindow")
    , mePageType(ePageType)
    , mrController(rController)
    , mxValueSet(new SvxNumValueSet(m_xBuilder->weld_scrolled_window("valuesetwin", true)))
    , mxValueSetWin(new weld::CustomWeld(*m_xBuilder, "valueset", *mxValueSet))
    , mxValueSetDoc(new SvxNumValueSet(m_xBuilder->weld_scrolled_window(u"valuesetwin_doc"_ustr, true)))
    , mxValueSetWinDoc(new weld::CustomWeld(*m_xBuilder, u"valueset_doc"_ustr, *mxValueSetDoc))
    , mxMoreButton(m_xBuilder->weld_button("more"))
    , mxBulletsLabel(m_xBuilder->weld_label(u"label_default"_ustr))
    , mxDocBulletsLabel(m_xBuilder->weld_label(u"label_doc"_ustr))
{
    mxValueSet->SetStyle(WB_MENUSTYLEVALUESET | WB_FLATVALUESET | WB_NO_DIRECTSELECT);
    mxValueSetDoc->SetStyle(WB_MENUSTYLEVALUESET | WB_FLATVALUESET | WB_NO_DIRECTSELECT);
    mxValueSet->init(mePageType);
    mxValueSetDoc->init(NumberingPageType::DOCBULLET);
    mxValueSetWinDoc->hide();
    mxDocBulletsLabel->hide();

    if ( mePageType != NumberingPageType::BULLET )
    {
        mxBulletsLabel->hide();
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
    weld::DrawingArea* pDrawingAreaDoc = mxValueSetDoc->GetDrawingArea();
    OutputDevice& rRefDevice = pDrawingArea->get_ref_device();
    Size aItemSize(rRefDevice.LogicToPixel(Size(30, 42), MapMode(MapUnit::MapAppFont)));
    mxValueSet->SetExtraSpacing( 2 );
    mxValueSetDoc->SetExtraSpacing( 2 );
    Size aSize(mxValueSet->CalcWindowSizePixel(aItemSize));
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
    pDrawingAreaDoc->set_size_request(aSize.Width(), aSize.Height());
    mxValueSet->SetOutputSizePixel(aSize);
    mxValueSetDoc->SetOutputSizePixel(aSize);
    mxValueSet->SetColor(Application::GetSettings().GetStyleSettings().GetFieldColor());
    mxValueSetDoc->SetColor(Application::GetSettings().GetStyleSettings().GetFieldColor());

    OUString aMoreItemText = SvxResId( RID_SVXSTR_CUSTOMIZE );
    if ( mePageType == NumberingPageType::BULLET )
    {
        AddStatusListener( ".uno:CurrentBulletListType" );
        AddStatusListener( ".uno:DocumentBulletList" );
    }
    else if ( mePageType == NumberingPageType::SINGLENUM )
        AddStatusListener( ".uno:CurrentNumListType" );
    else
        AddStatusListener( ".uno:CurrentOutlineType" );

    auto xImage = vcl::CommandInfoProvider::GetXGraphicForCommand(".uno:OutlineBullet", mrController.getFrameInterface());
    mxMoreButton->set_image(xImage);
    mxMoreButton->set_label(aMoreItemText);
    mxMoreButton->connect_clicked(LINK(this, NumberingPopup, VSButtonClickSetHdl));

    mxValueSet->SetSelectHdl(LINK(this, NumberingPopup, VSSelectValueSetHdl));
    mxValueSetDoc->SetSelectHdl(LINK(this, NumberingPopup, VSSelectValueSetDocHdl));
}

namespace
{
bool lcl_BulletIsDefault(std::u16string_view aSymbol, std::u16string_view aFont)
{
    css::uno::Sequence<OUString> aBulletSymbols
        = officecfg::Office::Common::BulletsNumbering::DefaultBullets::get();
    css::uno::Sequence<OUString> aBulletFonts
        = officecfg::Office::Common::BulletsNumbering::DefaultBulletsFonts::get();
    for (sal_Int32 i = 0; i < aBulletSymbols.getLength(); i++)
    {
        if (aBulletSymbols[i] == aSymbol && aBulletFonts[i] == aFont)
            return true;
    }
    return false;
}
}

void NumberingPopup::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    if (rEvent.FeatureURL.Complete == ".uno:DocumentBulletList")
    {
        css::uno::Sequence<OUString> aSeq;
        if (rEvent.State >>= aSeq)
        {
            std::vector<std::pair<OUString, OUString>> aList;
            mxValueSetDoc->Clear();
            int i = 1;
            // The string contains the bullet as first character, and then the font name
            for (const OUString& sBulletFont : aSeq)
            {
                OUString sBullet(sBulletFont.copy(0, 1));
                OUString sFont(sBulletFont.copy(1, sBulletFont.getLength() - 1));
                if (lcl_BulletIsDefault(sBullet, sFont))
                    continue;
                mxValueSetDoc->InsertItem(i, sBullet, i);
                aList.emplace_back(sBullet, sFont);
                i++;
            }
            if (!aList.empty())
            {
                mxValueSetWinDoc->show();
                mxDocBulletsLabel->show();
                mxValueSetDoc->SetCustomBullets(aList);
            }
            else
            {
                mxValueSetWinDoc->hide();
                mxDocBulletsLabel->hide();
            }
        }
    }
    else
    {
        mxValueSet->SetNoSelection();
        sal_Int32 nSelItem;
        if ( rEvent.State >>= nSelItem )
            mxValueSet->SelectItem( nSelItem );
    }
}

IMPL_LINK_NOARG(NumberingPopup, VSSelectValueSetHdl, ValueSet*, void)
{
    sal_uInt16 nSelItem = mxValueSet->GetSelectedItemId();
    if ( mePageType == NumberingPageType::BULLET )
    {
        auto aArgs( comphelper::InitPropertySequence( { { "BulletIndex", css::uno::Any( nSelItem ) } } ) );
        mrController.dispatchCommand( ".uno:SetBullet", aArgs );
    }
    else if ( mePageType == NumberingPageType::SINGLENUM )
    {
        auto aArgs( comphelper::InitPropertySequence( { { "SetNumber", css::uno::Any( nSelItem ) } } ) );
        mrController.dispatchCommand( ".uno:SetNumber", aArgs );
    }
    else
    {
        auto aArgs( comphelper::InitPropertySequence( { { "SetOutline", css::uno::Any( nSelItem ) } } ) );
        mrController.dispatchCommand( ".uno:SetOutline", aArgs );
    }
    mrController.EndPopupMode();
}

IMPL_LINK_NOARG(NumberingPopup, VSSelectValueSetDocHdl, ValueSet*, void)
{
    sal_uInt16 nSelItem = mxValueSetDoc->GetSelectedItemId() - 1;
    auto aCustomBullets = mxValueSetDoc->GetCustomBullets();
    OUString nChar(aCustomBullets[nSelItem].first);
    OUString sFont(aCustomBullets[nSelItem].second);
    auto aArgs(comphelper::InitPropertySequence(
        { { "BulletChar", css::uno::Any(nChar) }, { "BulletFont", css::uno::Any(sFont) } }));
    mrController.dispatchCommand(".uno:SetBullet", aArgs);
    mrController.EndPopupMode();
}

void NumberingPopup::GrabFocus()
{
    mxValueSet->GrabFocus();
}

IMPL_LINK_NOARG(NumberingPopup, VSButtonClickSetHdl, weld::Button&, void)
{
    auto aArgs( comphelper::InitPropertySequence( { { "Page", css::uno::Any( OUString("customize") ) } } ) );
    mrController.dispatchCommand( ".uno:OutlineBullet", aArgs );

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

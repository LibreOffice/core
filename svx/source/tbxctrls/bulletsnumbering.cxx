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
#include <vcl/virdev.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <editeng/svxenum.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <svx/svxbmpnumiconview.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::text;
using namespace com::sun::star::container;
using namespace com::sun::star::style;

namespace {

class NumberingToolBoxControl;

class NumberingPopup : public WeldToolbarPopup
{
    NumberingPageType mePageType;
    NumberingToolBoxControl& mrController;
    std::unique_ptr<weld::IconView> mxIconView;
    std::unique_ptr<weld::IconView> mxIconViewDoc;
    std::unique_ptr<weld::Button> mxMoreButton;
    std::unique_ptr<weld::Label> mxBulletsLabel;
    std::unique_ptr<weld::Label> mxDocBulletsLabel;
    DECL_LINK(ItemActivatedHdl, weld::IconView&, bool);
    DECL_LINK(QueryTooltipHdl, const weld::TreeIter&, OUString);

    DECL_LINK(DocItemActivatedHdl, weld::IconView&, bool);
    DECL_LINK(VSButtonClickSetHdl, weld::Button&, void);

    virtual void GrabFocus() override;

private:
    std::vector<std::pair<OUString, OUString>> maCustomBullets;
    Size aPreviewSize;

public:
    NumberingPopup(NumberingToolBoxControl& rController, weld::Widget* pParent, NumberingPageType ePageType);

    virtual void statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;
};

class NumberingToolBoxControl : public svt::PopupWindowController
{
    NumberingPageType mePageType;

public:
    explicit NumberingToolBoxControl( const Reference< XComponentContext >& rxContext );
    virtual VclPtr<vcl::Window> createVclPopupWindow( vcl::Window* pParent ) override;
    std::unique_ptr<WeldToolbarPopup> weldPopupWindow() override;

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

}

NumberingPopup::NumberingPopup(NumberingToolBoxControl& rController,
                               weld::Widget* pParent, NumberingPageType ePageType)
    : WeldToolbarPopup(rController.getFrameInterface(), pParent, u"svx/ui/numberingwindow.ui"_ustr, u"NumberingWindow"_ustr)
    , mePageType(ePageType)
    , mrController(rController)
    , mxIconView(m_xBuilder->weld_icon_view(u"numbering_window_iconview"_ustr))
    , mxIconViewDoc(m_xBuilder->weld_icon_view(u"numbering_window_iconview_doc"_ustr))
    , mxMoreButton(m_xBuilder->weld_button(u"more"_ustr))
    , mxBulletsLabel(m_xBuilder->weld_label(u"label_default"_ustr))
    , mxDocBulletsLabel(m_xBuilder->weld_label(u"label_doc"_ustr))
    , aPreviewSize(80, 100)
{
    SvxBmpNumIconView::PopulateIconView(mxIconView.get(), mePageType, aPreviewSize);
    SvxBmpNumIconView::PopulateIconView(mxIconViewDoc.get(), NumberingPageType::DOCBULLET, aPreviewSize);

    mxIconViewDoc->hide();
    mxDocBulletsLabel->hide();

    if ( mePageType != NumberingPageType::BULLET )
    {
        mxBulletsLabel->hide();
        Reference< XDefaultNumberingProvider > xDefNum = DefaultNumberingProvider::create( mrController.getContext() );
        if ( xDefNum.is() )
        {
            Locale aLocale = Application::GetSettings().GetLanguageTag().getLocale();
            Reference< XNumberingFormatter > xFormat( xDefNum, UNO_QUERY );

            if ( mePageType == NumberingPageType::SINGLENUM )
            {
                Sequence< Sequence< PropertyValue > > aNumberings(
                    xDefNum->getDefaultContinuousNumberingLevels( aLocale ) );
                SvxBmpNumIconView::SetNumberingSettings( mxIconView.get(), aPreviewSize, aNumberings, xFormat, aLocale );
            }
            else if ( mePageType == NumberingPageType::OUTLINE )
            {
                Sequence< Reference< XIndexAccess > > aOutline(
                    xDefNum->getDefaultOutlineNumberings( aLocale ) );
                SvxBmpNumIconView::SetOutlineNumberingSettings( mxIconView.get(), aPreviewSize, aOutline, xFormat, aLocale );
            }
        }
    }

    if ( mePageType == NumberingPageType::BULLET )
    {
        AddStatusListener( u".uno:CurrentBulletListType"_ustr );
        AddStatusListener( u".uno:DocumentBulletList"_ustr );
    }
    else if ( mePageType == NumberingPageType::SINGLENUM )
        AddStatusListener( u".uno:CurrentNumListType"_ustr );
    else
        AddStatusListener( u".uno:CurrentOutlineType"_ustr );

    auto xImage = vcl::CommandInfoProvider::GetXGraphicForCommand(u".uno:OutlineBullet"_ustr, mrController.getFrameInterface());
    mxMoreButton->set_image(xImage);
    mxMoreButton->connect_clicked(LINK(this, NumberingPopup, VSButtonClickSetHdl));

    mxIconView->connect_item_activated(LINK(this, NumberingPopup, ItemActivatedHdl));
    mxIconView->connect_query_tooltip(LINK(this, NumberingPopup, QueryTooltipHdl));

    mxIconViewDoc->connect_item_activated(LINK(this, NumberingPopup, DocItemActivatedHdl));
}

namespace
{
bool lcl_BulletIsDefault(std::u16string_view aSymbol, std::u16string_view aFont)
{
    Sequence<OUString> aBulletSymbols
        = officecfg::Office::Common::BulletsNumbering::DefaultBullets::get();
    Sequence<OUString> aBulletFonts
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
        Sequence<OUString> aSeq;
        if (rEvent.State >>= aSeq)
        {
            std::vector<std::pair<OUString, OUString>> aList;
            mxIconViewDoc->clear();

            // The string contains the bullet as first character, and then the font name
            for (const OUString& sBulletFont : aSeq)
            {
                OUString sBullet(sBulletFont.copy(0, 1));
                OUString sFont(sBulletFont.copy(1, sBulletFont.getLength() - 1));
                if (lcl_BulletIsDefault(sBullet, sFont))
                    continue;

                VclPtr<VirtualDevice> pVDev = SvxBmpNumIconView::CreateCustomBulletPreview(sBullet, sFont);
                OUString sId = OUString::number(aList.size());
                mxIconViewDoc->insert(-1, nullptr, &sId, pVDev, nullptr);
                aList.emplace_back(sBullet, sFont);
            }

            if (!aList.empty())
            {
                mxDocBulletsLabel->show();
                mxIconViewDoc->show();
                maCustomBullets = aList;
            }
            else
            {
                mxDocBulletsLabel->hide();
                mxIconViewDoc->hide();
            }
        }
    }
    else
    {
        OUString sId = mxIconView->get_selected_id();
        sal_Int32 nSelItem = !sId.isEmpty() ? sId.toInt32() : -1;
        if(nSelItem == -1) {
            if ( rEvent.State >>= nSelItem )
            {
                nSelItem--; // convert to 0-based index for iconview
                if(nSelItem > -1 && nSelItem < mxIconView->n_children())
                    mxIconView->select( nSelItem );
            }
        }
    }
}

IMPL_LINK(NumberingPopup, ItemActivatedHdl, weld::IconView&, rIconView, bool)
{
    OUString sId = rIconView.get_selected_id();
    if (sId.isEmpty())
        return false;

    sal_Int32 nId = sId.toInt32();

    if ( mePageType == NumberingPageType::BULLET )
    {
        auto aArgs( comphelper::InitPropertySequence( { { "BulletIndex", Any( nId + 1 ) } } ) );
        mrController.dispatchCommand( u".uno:SetBullet"_ustr, aArgs );
    }
    else if ( mePageType == NumberingPageType::SINGLENUM )
    {
        auto aArgs( comphelper::InitPropertySequence( { { "SetNumber", Any( nId + 1 ) } } ) );
        mrController.dispatchCommand( u".uno:SetNumber"_ustr, aArgs );
    }
    else
    {
        auto aArgs( comphelper::InitPropertySequence( { { "SetOutline", Any( nId + 1 ) } } ) );
        mrController.dispatchCommand( u".uno:SetOutline"_ustr, aArgs );
    }
    mrController.EndPopupMode();
    return true;
}

IMPL_LINK(NumberingPopup, DocItemActivatedHdl, weld::IconView&, rIconView, bool)
{
    OUString sId = rIconView.get_selected_id();
    if (sId.isEmpty())
        return false;

    sal_Int32 nId = sId.toInt32();

    if (nId >= 0 && nId < static_cast<sal_Int32>(maCustomBullets.size()))
    {
        OUString nChar(maCustomBullets[nId].first);
        OUString sFont(maCustomBullets[nId].second);
        auto aArgs(comphelper::InitPropertySequence(
            { { "BulletChar", Any(nChar) }, { "BulletFont", Any(sFont) } }));
        mrController.dispatchCommand(u".uno:SetBullet"_ustr, aArgs);
        mrController.EndPopupMode();
    }
    return true;
}

IMPL_LINK(NumberingPopup, QueryTooltipHdl, const weld::TreeIter&, rIter, OUString)
{
    const OUString sId = mxIconView->get_id(rIter);
    if (sId.isEmpty())
        return OUString();

    sal_Int32 nIndex = sId.toInt32();
    return SvxBmpNumIconView::GetNumberingDescription(mePageType, nIndex);
}

void NumberingPopup::GrabFocus()
{
    mxIconView->grab_focus();
}

IMPL_LINK_NOARG(NumberingPopup, VSButtonClickSetHdl, weld::Button&, void)
{
    auto aArgs( comphelper::InitPropertySequence( { { "Page", Any( u"customize"_ustr ) } } ) );
    mrController.dispatchCommand( u".uno:OutlineBullet"_ustr, aArgs );

    mrController.EndPopupMode();
}

NumberingToolBoxControl::NumberingToolBoxControl( const Reference< XComponentContext >& rxContext ):
    svt::PopupWindowController( rxContext, Reference< css::frame::XFrame >(), OUString() ),
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

void SAL_CALL NumberingToolBoxControl::initialize( const Sequence< Any >& aArguments )
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

Sequence< OUString > SAL_CALL NumberingToolBoxControl::getSupportedServiceNames()
{
    return { u"com.sun.star.frame.ToolbarController"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT XInterface *
com_sun_star_comp_svx_NumberingToolBoxControl_get_implementation(
    XComponentContext *rxContext,
    Sequence<Any> const & )
{
    return cppu::acquire( new NumberingToolBoxControl( rxContext ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

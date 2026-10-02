/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/text/DefaultNumberingProvider.hpp>
#include <com/sun/star/text/XNumberingFormatter.hpp>
#include <cpo/uno/Sequence.hxx>

#include <comphelper/propertysequence.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <officecfg/Office/Common.hxx>
#include <svtools/popupwindowcontroller.hxx>
#include <svtools/toolbarmenu.hxx>
#include <svx/dialmgr.hxx>
#include <o3tl/string_view.hxx>
#include <svx/documentbullets.hxx>
#include <svx/strings.hrc>
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

using namespace ::cpo::uno;
using namespace cpo::uno;
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
    /// The character bullets of the document, without the ones the fixed library already offers.
    std::vector<SvxDocumentBullet> maCharacterBullets;
    /// The image bullets of the document.
    std::vector<SvxDocumentBullet> maGraphicBullets;
    /// The two lists one after the other, in the order the icon view shows them.
    std::vector<SvxDocumentBullet> maDocumentBullets;

    /// Draws the document bullets into the second icon view, and hides that view when the
    /// document has no bullets of its own.
    void FillDocumentBullets();

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
    virtual void initialize( const cpo::uno::Sequence< cpo::uno::Any >& rArguments ) override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual Sequence< OUString > getSupportedServiceNames() override;
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
{
    Size aPreviewSize(80, 100);

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
        AddStatusListener(u".uno:DocumentBulletGraphicList"_ustr);
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

void NumberingPopup::FillDocumentBullets()
{
    maDocumentBullets = maCharacterBullets;
    maDocumentBullets.insert(maDocumentBullets.end(), maGraphicBullets.begin(),
                             maGraphicBullets.end());

    mxIconViewDoc->clear();

    // The name is what a screen reader announces for the entry, and an image carries no character
    // to announce.
    for (size_t nBullet = 0; nBullet < maDocumentBullets.size(); ++nBullet)
    {
        const SvxDocumentBullet& rBullet = maDocumentBullets[nBullet];
        auto pVDev = SvxBmpNumIconView::CreateCustomBulletPreview(rBullet, Size(80, 100));
        Bitmap aBitmap = pVDev->GetBitmap(Point(), pVDev->GetOutputSizePixel());
        OUString sId = OUString::number(nBullet);
        OUString sText = rBullet.IsGraphic() ? SvxResId(RID_SVXSTR_DOCUMENT_BULLET_IMAGE)
                                             : rBullet.aBulletCharacter;
        mxIconViewDoc->insert(-1, &sText, &sId, &aBitmap, nullptr);
        mxIconViewDoc->set_item_accessible_name(mxIconViewDoc->n_children() - 1, sText);
    }

    const bool bHasDocumentBullets = !maDocumentBullets.empty();
    mxDocBulletsLabel->set_visible(bHasDocumentBullets);
    mxIconViewDoc->set_visible(bHasDocumentBullets);
}

void NumberingPopup::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    if (rEvent.FeatureURL.Complete == ".uno:DocumentBulletList")
    {
        Sequence<OUString> aSeq;
        if (rEvent.State >>= aSeq)
        {
            maCharacterBullets.clear();

            // The string contains the bullet as first character, and then the font name
            for (const OUString& sBulletFont : aSeq)
            {
                if (sBulletFont.isEmpty())
                    continue;

                // A bullet outside the Basic Multilingual Plane takes two units of the string, so
                // the font name starts after the whole character rather than after the first unit.
                sal_Int32 nFontNameStart = 0;
                o3tl::iterateCodePoints(sBulletFont, &nFontNameStart);

                SvxDocumentBullet aBullet;
                aBullet.aBulletCharacter = sBulletFont.copy(0, nFontNameStart);
                aBullet.aFontName = sBulletFont.copy(nFontNameStart);
                if (aBullet.IsInDefaultLibrary())
                    continue;

                maCharacterBullets.push_back(aBullet);
            }

            FillDocumentBullets();
        }
    }
    else if (rEvent.FeatureURL.Complete == ".uno:DocumentBulletGraphicList")
    {
        Sequence<css::beans::PropertyValue> aSeq;
        if (rEvent.State >>= aSeq)
        {
            maGraphicBullets.clear();

            for (const css::beans::PropertyValue& rEntry : aSeq)
            {
                Reference<css::graphic::XGraphic> xGraphic;
                if (!(rEntry.Value >>= xGraphic) || !xGraphic.is())
                    continue;

                SvxDocumentBullet aBullet;
                aBullet.aGraphic = Graphic(xGraphic);
                maGraphicBullets.push_back(aBullet);
            }

            FillDocumentBullets();
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
        auto aArgs( comphelper::InitPropertySequence( { { u"BulletIndex"_ustr, Any( nId + 1 ) } } ) );
        mrController.dispatchCommand( u".uno:SetBullet"_ustr, aArgs );
    }
    else if ( mePageType == NumberingPageType::SINGLENUM )
    {
        auto aArgs( comphelper::InitPropertySequence( { { u"SetNumber"_ustr, Any( nId + 1 ) } } ) );
        mrController.dispatchCommand( u".uno:SetNumber"_ustr, aArgs );
    }
    else
    {
        auto aArgs( comphelper::InitPropertySequence( { { u"SetOutline"_ustr, Any( nId + 1 ) } } ) );
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

    if (nId >= 0 && nId < static_cast<sal_Int32>(maDocumentBullets.size()))
    {
        const SvxDocumentBullet& rBullet = maDocumentBullets[nId];
        // An image bullet travels by name, and the document looks the image up again by that name.
        auto aArgs = rBullet.IsGraphic()
                         ? comphelper::InitPropertySequence(
                               { { u"BulletGraphic"_ustr, Any(rBullet.GetIdentifier()) } })
                         : comphelper::InitPropertySequence(
                               { { u"BulletChar"_ustr, Any(rBullet.aBulletCharacter) },
                                 { u"BulletFont"_ustr, Any(rBullet.aFontName) } });
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
    auto aArgs( comphelper::InitPropertySequence(
        { { u"Page"_ustr, Any( u"customize"_ustr ) },
          { u"Bullet"_ustr, Any( mePageType == NumberingPageType::BULLET ) } } ) );
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

void NumberingToolBoxControl::initialize( const cpo::uno::Sequence< cpo::uno::Any >& rArguments )
{
    svt::PopupWindowController::initialize( rArguments );

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

OUString NumberingToolBoxControl::getImplementationName()
{
    return u"com.sun.star.comp.svx.NumberingToolBoxControl"_ustr;
}

Sequence< OUString > NumberingToolBoxControl::getSupportedServiceNames()
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

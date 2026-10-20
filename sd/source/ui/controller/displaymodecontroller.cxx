/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svtools/popupwindowcontroller.hxx>
#include <svtools/toolbarmenu.hxx>
#include <vcl/image.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/Frame.hxx>
#include <vcl/weld/IconView.hxx>
#include <vcl/weld/Toolbar.hxx>
#include <vcl/weld/Window.hxx>

#include <strings.hrc>

#include <bitmaps.hlst>
#include <sdresid.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;

namespace sd
{

// Component to select which display mode has to be used.
// Composed of a dropdown button in the toolbar and a
// popup menu to select the value

namespace {

class DisplayModeController : public svt::PopupWindowController
{
public:
    explicit DisplayModeController( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual std::unique_ptr<WeldToolbarPopup> weldPopupWindow() override;
    virtual VclPtr<vcl::Window> createVclPopupWindow( vcl::Window* pParent ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    void setToolboxItemImage(const OUString& rImage);
};

class DisplayModeToolbarMenu final : public WeldToolbarPopup
{
public:
    DisplayModeToolbarMenu(DisplayModeController* pControl, weld::Widget* pParent);
    virtual void GrabFocus() override { mxDisplayModeIconView1->grab_focus(); }

private:
    rtl::Reference<DisplayModeController> mxControl;
    std::unique_ptr<weld::Frame> mxFrame1;
    std::unique_ptr<weld::IconView> mxDisplayModeIconView1;
    std::unique_ptr<weld::Frame> mxFrame2;
    std::unique_ptr<weld::IconView> mxDisplayModeIconView2;

    DECL_LINK(ItemActivatedHdl, const weld::TreeIter&, bool);
};

struct snew_slide_value_info
{
    OUString msBmpResId;
    TranslateId mpStrResId;
    OUString msUnoCommand;
};
}

constexpr snew_slide_value_info editmodes[]
    = { { BMP_DISPLAYMODE_SLIDE, STR_NORMAL_MODE, u".uno:NormalMultiPaneGUI"_ustr },
        { BMP_DISPLAYMODE_OUTLINE, STR_OUTLINE_MODE, u".uno:OutlineMode"_ustr },
        { BMP_DISPLAYMODE_NOTES, STR_NOTES_MODE, u".uno:NotesMode"_ustr },
        { BMP_DISPLAYMODE_SLIDE_SORTER, STR_SLIDE_SORTER_MODE, u".uno:DiaMode"_ustr } };

constexpr snew_slide_value_info mastermodes[]
    = { { BMP_DISPLAYMODE_SLIDE_MASTER, STR_SLIDE_MASTER_MODE, u".uno:SlideMasterPage"_ustr },
        { BMP_DISPLAYMODE_NOTES_MASTER, STR_NOTES_MASTER_MODE, u".uno:NotesMasterPage"_ustr },
        { BMP_DISPLAYMODE_HANDOUT_MASTER, STR_HANDOUT_MASTER_MODE, u".uno:HandoutMode"_ustr } };

static void fillLayoutIconView(weld::IconView& rIconView,
                               std::span<const snew_slide_value_info> aInfos)
{
    int nIndex = 0;
    for (const snew_slide_value_info& rInfo : aInfos)
    {
        OUString aText(SdResId(rInfo.mpStrResId));
        Bitmap aBmp(rInfo.msBmpResId);

        rIconView.insert(nIndex, nullptr, nullptr, &aBmp, nullptr);
        rIconView.set_item_accessible_name(nIndex, aText);
        rIconView.set_item_tooltip_text(nIndex, aText);

        ++nIndex;
    }
}

DisplayModeToolbarMenu::DisplayModeToolbarMenu(DisplayModeController* pControl, weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, u"modules/simpress/ui/displaywindow.ui"_ustr, u"DisplayWindow"_ustr)
    , mxControl(pControl)
    , mxFrame1(m_xBuilder->weld_frame(u"editframe"_ustr))
    , mxDisplayModeIconView1(m_xBuilder->weld_icon_view(u"iconview1"_ustr))
    , mxFrame2(m_xBuilder->weld_frame(u"masterframe"_ustr))
    , mxDisplayModeIconView2(m_xBuilder->weld_icon_view(u"iconview2"_ustr))
{
    mxDisplayModeIconView1->connect_item_activated(
        LINK(this, DisplayModeToolbarMenu, ItemActivatedHdl));
    mxDisplayModeIconView2->connect_item_activated(
        LINK(this, DisplayModeToolbarMenu, ItemActivatedHdl));

    fillLayoutIconView(*mxDisplayModeIconView1, editmodes);
    fillLayoutIconView(*mxDisplayModeIconView2, mastermodes);
}

IMPL_LINK(DisplayModeToolbarMenu, ItemActivatedHdl, const weld::TreeIter&, rIter, bool)
{
    OUString sCommandURL;
    OUString sImage;

    const weld::ItemView& rItemView = rIter.getItemView();
    const int nIndex = rItemView.get_iter_index_in_parent(rIter);
    if (&rItemView == mxDisplayModeIconView1.get())
    {
        sCommandURL = editmodes[nIndex].msUnoCommand;
        sImage = editmodes[nIndex].msBmpResId;
    }
    else if (&rItemView == mxDisplayModeIconView2.get())
    {
        sCommandURL = mastermodes[nIndex].msUnoCommand;
        sImage = mastermodes[nIndex].msBmpResId;
    }

    if (!sCommandURL.isEmpty())
        mxControl->dispatchCommand( sCommandURL, Sequence< PropertyValue >() );

    mxControl->setToolboxItemImage(sImage);
    mxControl->EndPopupMode();

    return true;
}

DisplayModeController::DisplayModeController( const css::uno::Reference< css::uno::XComponentContext >& rxContext )
: svt::PopupWindowController( rxContext, Reference< frame::XFrame >(), OUString() )
{
}

void SAL_CALL DisplayModeController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    svt::PopupWindowController::initialize( aArguments );
    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if ( getToolboxId( nId, &pToolBox ) )
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWNONLY );
    setToolboxItemImage(BMP_DISPLAYMODE_SLIDE);
}

std::unique_ptr<WeldToolbarPopup> DisplayModeController::weldPopupWindow()
{
    return std::make_unique<sd::DisplayModeToolbarMenu>(this, m_pToolbar);
}

VclPtr<vcl::Window> DisplayModeController::createVclPopupWindow( vcl::Window* pParent )
{
    mxInterimPopover = VclPtr<InterimToolbarPopup>::Create(getFrameInterface(), pParent,
        std::make_unique<sd::DisplayModeToolbarMenu>(this, pParent->GetFrameWeld()));

    mxInterimPopover->Show();

    return mxInterimPopover;
}

void DisplayModeController::setToolboxItemImage(const OUString& rImage)
{
    ToolBoxItemId nId;
    ToolBox* pToolBox = nullptr;
    if (!getToolboxId( nId, &pToolBox ))
        return;

    Bitmap aBmp(rImage);
    int targetSize = (pToolBox->GetToolboxButtonSize() == ToolBoxButtonSize::Large) ? 32 : 16;
    double scale = 1.0f;
    Size size = aBmp.GetSizePixel();
    if (size.Width() > targetSize)
        scale = static_cast<double>(targetSize) / static_cast<double>(size.Width());
    if (size.Height() > targetSize)
        scale = ::std::min( scale, static_cast<double>(targetSize) / static_cast<double>(size.Height()) );
    aBmp.Scale( scale, scale );
    pToolBox->SetItemImage( nId, Image( aBmp ) );
}

// XServiceInfo

OUString SAL_CALL DisplayModeController::getImplementationName()
{
    return u"com.sun.star.comp.sd.DisplayModeController"_ustr;
}

Sequence< OUString > SAL_CALL DisplayModeController::getSupportedServiceNames(  )
{
    css::uno::Sequence<OUString> aRet { u"com.sun.star.frame.ToolbarController"_ustr };
    return aRet;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_sd_DisplayModeController_get_implementation( css::uno::XComponentContext* context,
                                                               css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sd::DisplayModeController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

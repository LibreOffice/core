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
#include <svtools/valueset.hxx>
#include <vcl/toolbox.hxx>

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
    virtual void GrabFocus() override
    {
        mxDisplayModeSet1->GrabFocus();
    }

private:
    rtl::Reference<DisplayModeController> mxControl;
    std::unique_ptr<weld::Frame> mxFrame1;
    std::unique_ptr<ValueSet> mxDisplayModeSet1;
    std::unique_ptr<weld::CustomWeld> mxDisplayModeSetWin1;
    std::unique_ptr<weld::Frame> mxFrame2;
    std::unique_ptr<ValueSet> mxDisplayModeSet2;
    std::unique_ptr<weld::CustomWeld> mxDisplayModeSetWin2;

    DECL_LINK(SelectValueSetHdl, ValueSet*, void);
};

struct snewfoil_value_info
{
    sal_uInt16 mnId;
    const char* msBmpResId;
    const char* mpStrResId;
    const char* msUnoCommand;
};

}

const snewfoil_value_info editmodes[] =
{
    {1,
        BMP_DISPLAYMODE_SLIDE,
        STR_NORMAL_MODE,
        ".uno:NormalMultiPaneGUI" },
    {2,
        BMP_DISPLAYMODE_OUTLINE,
        STR_OUTLINE_MODE,
        ".uno:OutlineMode" },
    {3,
        BMP_DISPLAYMODE_NOTES,
        STR_NOTES_MODE,
        ".uno:NotesMode" },
    {4,
        BMP_DISPLAYMODE_SLIDE_SORTER,
        STR_SLIDE_SORTER_MODE,
        ".uno:DiaMode" },
    {0, "", nullptr, "" }
};

const snewfoil_value_info mastermodes[] =
{
    {5,
        BMP_DISPLAYMODE_SLIDE_MASTER,
        STR_SLIDE_MASTER_MODE,
        ".uno:SlideMasterPage" },
    {6,
        BMP_DISPLAYMODE_NOTES_MASTER,
        STR_NOTES_MASTER_MODE,
        ".uno:NotesMasterPage" },
    {7,
        BMP_DISPLAYMODE_HANDOUT_MASTER,
        STR_HANDOUT_MASTER_MODE,
        ".uno:HandoutMode" },
    {0, "", nullptr, "" }
};


static void fillLayoutValueSet(ValueSet* pValue, const snewfoil_value_info* pInfo)
{
    Size aLayoutItemSize;
    for( ; pInfo->mnId; pInfo++ )
    {
        OUString aText(SdResId(pInfo->mpStrResId));
        BitmapEx aBmp(OUString::createFromAscii(pInfo->msBmpResId));

        pValue->InsertItem(pInfo->mnId, Image(aBmp), aText);

        aLayoutItemSize.setWidth( std::max( aLayoutItemSize.Width(),  aBmp.GetSizePixel().Width()  ) );
        aLayoutItemSize.setHeight( std::max( aLayoutItemSize.Height(), aBmp.GetSizePixel().Height() ) );
    }

    aLayoutItemSize = pValue->CalcItemSizePixel( aLayoutItemSize );
    Size aSize(pValue->CalcWindowSizePixel(aLayoutItemSize));

    const sal_Int32 LAYOUT_BORDER_PIX = 7;
    aSize.AdjustWidth((pValue->GetColCount() + 1) * LAYOUT_BORDER_PIX );
    aSize.AdjustHeight((pValue->GetLineCount() +1) * LAYOUT_BORDER_PIX );

    pValue->GetDrawingArea()->set_size_request(aSize.Width(), aSize.Height());
    pValue->SetOutputSizePixel(aSize);
}

DisplayModeToolbarMenu::DisplayModeToolbarMenu(DisplayModeController* pControl, weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, "modules/simpress/ui/displaywindow.ui", "DisplayWindow")
    , mxControl(pControl)
    , mxFrame1(m_xBuilder->weld_frame("editframe"))
    , mxDisplayModeSet1(new ValueSet(nullptr))
    , mxDisplayModeSetWin1(new weld::CustomWeld(*m_xBuilder, "valueset1", *mxDisplayModeSet1))
    , mxFrame2(m_xBuilder->weld_frame("masterframe"))
    , mxDisplayModeSet2(new ValueSet(nullptr))
    , mxDisplayModeSetWin2(new weld::CustomWeld(*m_xBuilder, "valueset2", *mxDisplayModeSet2))
{
    mxDisplayModeSet1->SetStyle(WB_TABSTOP | WB_MENUSTYLEVALUESET | WB_FLATVALUESET | WB_NOBORDER | WB_NO_DIRECTSELECT);
    mxDisplayModeSet1->SetStyle(WB_TABSTOP | WB_MENUSTYLEVALUESET | WB_FLATVALUESET | WB_NOBORDER | WB_NO_DIRECTSELECT);

    mxDisplayModeSet1->SetSelectHdl( LINK( this, DisplayModeToolbarMenu, SelectValueSetHdl ) );
    mxDisplayModeSet2->SetSelectHdl( LINK( this, DisplayModeToolbarMenu, SelectValueSetHdl ) );

    sal_Int16 nColCount = 2;

    mxDisplayModeSet1->SetColCount( nColCount );
    fillLayoutValueSet( mxDisplayModeSet1.get(), &editmodes[0] );

    mxDisplayModeSet2->SetColCount( nColCount );
    fillLayoutValueSet( mxDisplayModeSet2.get(), &mastermodes[0] );
}

IMPL_LINK( DisplayModeToolbarMenu, SelectValueSetHdl, ValueSet*, pControl, void )
{
    OUString sCommandURL;
    OUString sImage;

    if( pControl == mxDisplayModeSet1.get() ) {
        sCommandURL = OUString::createFromAscii(editmodes[mxDisplayModeSet1->GetSelectedItemId() - 1 ].msUnoCommand);
        sImage = OUString::createFromAscii(editmodes[mxDisplayModeSet1->GetSelectedItemId() - 1 ].msBmpResId);
    }
    else if( pControl == mxDisplayModeSet2.get() ) {
        sCommandURL = OUString::createFromAscii(mastermodes[mxDisplayModeSet2->GetSelectedItemId() - 5 ].msUnoCommand);
        sImage = OUString::createFromAscii(mastermodes[mxDisplayModeSet2->GetSelectedItemId() - 5 ].msBmpResId);
    }

    if (!sCommandURL.isEmpty())
        mxControl->dispatchCommand( sCommandURL, Sequence< PropertyValue >() );

    mxControl->setToolboxItemImage(sImage);
    mxControl->EndPopupMode();
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

    BitmapEx aBmp(rImage);
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
    return "com.sun.star.comp.sd.DisplayModeController";
}

Sequence< OUString > SAL_CALL DisplayModeController::getSupportedServiceNames(  )
{
    css::uno::Sequence<OUString> aRet { "com.sun.star.frame.ToolbarController" };
    return aRet;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT ::com::sun::star::uno::XInterface*
com_sun_star_comp_sd_DisplayModeController_get_implementation( css::uno::XComponentContext* context,
                                                               css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sd::DisplayModeController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/DrawViewMode.hpp>

#include <comphelper/propertyvalue.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/toolbox.hxx>

#include <svl/languageoptions.hxx>
#include <svl/cjkoptions.hxx>

#include <svtools/toolbarmenu.hxx>
#include <svtools/valueset.hxx>

#include <xmloff/autolayout.hxx>

#include <strings.hrc>

#include <bitmaps.hlst>
#include <sdresid.hxx>
#include "slidelayoutcontroller.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;

namespace sd
{

namespace {

class LayoutToolbarMenu : public WeldToolbarPopup
{
public:
    LayoutToolbarMenu(SlideLayoutController* pController, weld::Widget* pParent, const bool bInsertPage, const OUString& rCommand);
    virtual void GrabFocus() override
    {
        mxLayoutSet1->GrabFocus();
    }

protected:
    DECL_LINK(SelectToolbarMenuHdl, weld::Button&, void);
    DECL_LINK(SelectValueSetHdl, ValueSet*, void);
    void SelectHdl(AutoLayout eLayout);
private:
    rtl::Reference<SlideLayoutController> mxControl;
    bool const mbInsertPage;
    std::unique_ptr<weld::Frame> mxFrame1;
    std::unique_ptr<ValueSet> mxLayoutSet1;
    std::unique_ptr<weld::CustomWeld> mxLayoutSetWin1;
    std::unique_ptr<weld::Frame> mxFrame2;
    std::unique_ptr<ValueSet> mxLayoutSet2;
    std::unique_ptr<weld::CustomWeld> mxLayoutSetWin2;
    std::unique_ptr<weld::Button> mxMoreButton;
};

struct snewfoil_value_info_layout
{
    rtl::OUStringConstExpr msBmpResId;
    TranslateId mpStrResId;
    AutoLayout maAutoLayout;
};

}

constexpr OUStringLiteral EMPTY = u"";

const snewfoil_value_info_layout notes[] =
{
    {BMP_FOILN_01, STR_AUTOLAYOUT_NOTES, AUTOLAYOUT_NOTES},
    {EMPTY, {}, AUTOLAYOUT_NONE},
};

const snewfoil_value_info_layout handout[] =
{
    {BMP_FOILH_01, STR_AUTOLAYOUT_HANDOUT1, AUTOLAYOUT_HANDOUT1},
    {BMP_FOILH_02, STR_AUTOLAYOUT_HANDOUT2, AUTOLAYOUT_HANDOUT2},
    {BMP_FOILH_03, STR_AUTOLAYOUT_HANDOUT3, AUTOLAYOUT_HANDOUT3},
    {BMP_FOILH_04, STR_AUTOLAYOUT_HANDOUT4, AUTOLAYOUT_HANDOUT4},
    {BMP_FOILH_06, STR_AUTOLAYOUT_HANDOUT6, AUTOLAYOUT_HANDOUT6},
    {BMP_FOILH_09, STR_AUTOLAYOUT_HANDOUT9, AUTOLAYOUT_HANDOUT9},
    {EMPTY, {}, AUTOLAYOUT_NONE},
};

const snewfoil_value_info_layout standard[] =
{
    {BMP_LAYOUT_EMPTY,    STR_AUTOLAYOUT_NONE,                 AUTOLAYOUT_NONE         },
    {BMP_LAYOUT_HEAD03,   STR_AUTOLAYOUT_TITLE,                AUTOLAYOUT_TITLE        },
    {BMP_LAYOUT_HEAD02,   STR_AUTOLAYOUT_CONTENT,              AUTOLAYOUT_TITLE_CONTENT },
    {BMP_LAYOUT_HEAD02A,  STR_AUTOLAYOUT_2CONTENT,             AUTOLAYOUT_TITLE_2CONTENT },
    {BMP_LAYOUT_HEAD01,   STR_AUTOLAYOUT_ONLY_TITLE,           AUTOLAYOUT_TITLE_ONLY   },
    {BMP_LAYOUT_TEXTONLY, STR_AUTOLAYOUT_ONLY_TEXT,            AUTOLAYOUT_ONLY_TEXT    },
    {BMP_LAYOUT_HEAD03B,  STR_AUTOLAYOUT_2CONTENT_CONTENT,     AUTOLAYOUT_TITLE_2CONTENT_CONTENT },
    {BMP_LAYOUT_HEAD03C,  STR_AUTOLAYOUT_CONTENT_2CONTENT,     AUTOLAYOUT_TITLE_CONTENT_2CONTENT },
    {BMP_LAYOUT_HEAD03A,  STR_AUTOLAYOUT_2CONTENT_OVER_CONTENT,AUTOLAYOUT_TITLE_2CONTENT_OVER_CONTENT },
    {BMP_LAYOUT_HEAD02B,  STR_AUTOLAYOUT_CONTENT_OVER_CONTENT, AUTOLAYOUT_TITLE_CONTENT_OVER_CONTENT },
    {BMP_LAYOUT_HEAD04,   STR_AUTOLAYOUT_4CONTENT,             AUTOLAYOUT_TITLE_4CONTENT },
    {BMP_LAYOUT_HEAD06,   STR_AUTOLAYOUT_6CONTENT,             AUTOLAYOUT_TITLE_6CONTENT },
    {EMPTY, {}, AUTOLAYOUT_NONE}
};

const snewfoil_value_info_layout v_standard[] =
{
    // vertical
    {BMP_LAYOUT_VERTICAL02, STR_AL_VERT_TITLE_TEXT_CHART,      AUTOLAYOUT_VTITLE_VCONTENT_OVER_VCONTENT   },
    {BMP_LAYOUT_VERTICAL01, STR_AL_VERT_TITLE_VERT_OUTLINE,    AUTOLAYOUT_VTITLE_VCONTENT                 },
    {BMP_LAYOUT_HEAD02,     STR_AL_TITLE_VERT_OUTLINE,         AUTOLAYOUT_TITLE_VCONTENT                  },
    {BMP_LAYOUT_HEAD02A,    STR_AL_TITLE_VERT_OUTLINE_CLIPART, AUTOLAYOUT_TITLE_2VTEXT                    },
    {EMPTY, {}, AUTOLAYOUT_NONE}
};

static void fillLayoutValueSet( ValueSet* pValue, const snewfoil_value_info_layout* pInfo )
{
    Size aLayoutItemSize;
    for( ; pInfo->mpStrResId; pInfo++ )
    {
        OUString aText(SdResId(pInfo->mpStrResId));
        Image aImg(StockImage::Yes, pInfo->msBmpResId);
        pValue->InsertItem(static_cast<sal_uInt16>(pInfo->maAutoLayout)+1, aImg, aText);
        aLayoutItemSize.setWidth( std::max( aLayoutItemSize.Width(),   aImg.GetSizePixel().Width()  ) );
        aLayoutItemSize.setHeight( std::max( aLayoutItemSize.Height(), aImg.GetSizePixel().Height() ) );
    }

    aLayoutItemSize = pValue->CalcItemSizePixel( aLayoutItemSize );
    Size aSize(pValue->CalcWindowSizePixel(aLayoutItemSize));

    const sal_Int32 LAYOUT_BORDER_PIX = 7;

    aSize.AdjustWidth((pValue->GetColCount() + 1) * LAYOUT_BORDER_PIX);
    aSize.AdjustHeight((pValue->GetLineCount() +1) * LAYOUT_BORDER_PIX);

    pValue->GetDrawingArea()->set_size_request(aSize.Width(), aSize.Height());
    pValue->SetOutputSizePixel(aSize);
}

LayoutToolbarMenu::LayoutToolbarMenu(SlideLayoutController* pControl, weld::Widget* pParent, const bool bInsertPage, const OUString& rCommand)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, "modules/simpress/ui/layoutwindow.ui", "LayoutWindow")
    , mxControl(pControl)
    , mbInsertPage(bInsertPage)
    , mxFrame1(m_xBuilder->weld_frame("horiframe"))
    , mxLayoutSet1(new ValueSet(nullptr))
    , mxLayoutSetWin1(new weld::CustomWeld(*m_xBuilder, "valueset1", *mxLayoutSet1))
    , mxFrame2(m_xBuilder->weld_frame("vertframe"))
    , mxLayoutSet2(new ValueSet(nullptr))
    , mxLayoutSetWin2(new weld::CustomWeld(*m_xBuilder, "valueset2", *mxLayoutSet2))
    , mxMoreButton(m_xBuilder->weld_button("more"))
{
    mxLayoutSet1->SetStyle(WB_TABSTOP | WB_MENUSTYLEVALUESET | WB_FLATVALUESET | WB_NOBORDER | WB_NO_DIRECTSELECT);
    mxLayoutSet2->SetStyle(WB_TABSTOP | WB_MENUSTYLEVALUESET | WB_FLATVALUESET | WB_NOBORDER | WB_NO_DIRECTSELECT);

    DrawViewMode eMode = DrawViewMode_DRAW;

    // find out which view is running
    if( m_xFrame.is() ) try
    {
        Reference< XPropertySet > xControllerSet( m_xFrame->getController(), UNO_QUERY_THROW );
        xControllerSet->getPropertyValue( "DrawViewMode" ) >>= eMode;
    }
    catch( Exception& )
    {
        OSL_ASSERT(false);
    }

    const bool bVerticalEnabled = SvtCJKOptions::IsVerticalTextEnabled();

    mxLayoutSet1->SetSelectHdl( LINK( this, LayoutToolbarMenu, SelectValueSetHdl ) );

    const snewfoil_value_info_layout* pInfo = nullptr;
    sal_Int16 nColCount = 4;
    switch( eMode )
    {
    case DrawViewMode_DRAW: pInfo = &standard[0]; break;
    case DrawViewMode_HANDOUT: pInfo = &handout[0]; nColCount = 2; break;
    case DrawViewMode_NOTES: pInfo = &notes[0]; nColCount = 1; break;
    default: assert(false); // can't happen, will crash later otherwise
    }

    mxLayoutSet1->SetColCount( nColCount );

    fillLayoutValueSet( mxLayoutSet1.get(), pInfo );

    bool bUseUILabel = (bVerticalEnabled && eMode == DrawViewMode_DRAW);
    if (!bUseUILabel)
    {
        auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(rCommand, mxControl->getModuleName());
        mxFrame1->set_label(vcl::CommandInfoProvider::GetLabelForCommand(aProperties));
    }

    if (bVerticalEnabled && eMode == DrawViewMode_DRAW)
    {
        mxLayoutSet2->SetSelectHdl( LINK( this, LayoutToolbarMenu, SelectValueSetHdl ) );
        mxLayoutSet2->SetColCount( 4 );
        mxLayoutSet2->EnableFullItemMode( false );

        fillLayoutValueSet( mxLayoutSet2.get(), &v_standard[0] );

        mxFrame2->show();
    }

    if( eMode != DrawViewMode_DRAW )
        return;

    if( !m_xFrame.is() )
        return;

    OUString sSlotStr;

    if( bInsertPage )
        sSlotStr = ".uno:DuplicatePage";
    else
        sSlotStr = ".uno:Undo";

    css::uno::Reference<css::graphic::XGraphic> xSlotImage = vcl::CommandInfoProvider::GetXGraphicForCommand(sSlotStr, m_xFrame);

    OUString sSlotTitle;
    if( bInsertPage )
    {
        auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(sSlotStr, mxControl->getModuleName());
        sSlotTitle = vcl::CommandInfoProvider::GetLabelForCommand(aProperties);
    }
    else
        sSlotTitle = SdResId( STR_RESET_LAYOUT );

    mxMoreButton->set_label(sSlotTitle);
    mxMoreButton->set_image(xSlotImage);
    mxMoreButton->connect_clicked(LINK(this, LayoutToolbarMenu, SelectToolbarMenuHdl));
    mxMoreButton->show();
}

IMPL_LINK(LayoutToolbarMenu, SelectValueSetHdl, ValueSet*, pLayoutSet, void)
{
    SelectHdl(static_cast<AutoLayout>(pLayoutSet->GetSelectedItemId()-1));
}

IMPL_LINK_NOARG(LayoutToolbarMenu, SelectToolbarMenuHdl, weld::Button&, void)
{
    SelectHdl(AUTOLAYOUT_END);
}

void LayoutToolbarMenu::SelectHdl(AutoLayout eLayout)
{
    Sequence< PropertyValue > aArgs;

    OUString sCommandURL( mxControl->getCommandURL() );

    if( eLayout != AUTOLAYOUT_END )
    {
        aArgs = { comphelper::makePropertyValue("WhatLayout", static_cast<sal_Int32>(eLayout)) };
    }
    else if( mbInsertPage )
    {
        sCommandURL = ".uno:DuplicatePage";
    }

    mxControl->dispatchCommand( sCommandURL, aArgs );

    mxControl->EndPopupMode();
}


/// @throws css::uno::RuntimeException
static OUString SlideLayoutController_getImplementationName()
{
    return "com.sun.star.comp.sd.SlideLayoutController";
}

/// @throws RuntimeException
static Sequence< OUString >  SlideLayoutController_getSupportedServiceNames()
{
    Sequence<OUString> aSNS { "com.sun.star.frame.ToolbarController" };
    return aSNS;
}

/// @throws css::uno::RuntimeException
static OUString InsertSlideController_getImplementationName()
{
    return "com.sun.star.comp.sd.InsertSlideController";
}

/// @throws RuntimeException
static Sequence< OUString >  InsertSlideController_getSupportedServiceNames()
{
    Sequence<OUString> aSNS { "com.sun.star.frame.ToolbarController" };
    return aSNS;
}

SlideLayoutController::SlideLayoutController(const Reference< uno::XComponentContext >& rxContext, bool bInsertPage)
    : svt::PopupWindowController(rxContext, nullptr, OUString())
    , mbInsertPage(bInsertPage)
{
}

void SAL_CALL SlideLayoutController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    svt::PopupWindowController::initialize( aArguments );

    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if ( getToolboxId( nId, &pToolBox ) )
    {
        if ( mbInsertPage )
            pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWN );
        else
            pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWNONLY );
    }
}

std::unique_ptr<WeldToolbarPopup> SlideLayoutController::weldPopupWindow()
{
    return std::make_unique<sd::LayoutToolbarMenu>(this, m_pToolbar, mbInsertPage, m_aCommandURL);
}

VclPtr<vcl::Window> SlideLayoutController::createVclPopupWindow( vcl::Window* pParent )
{
    mxInterimPopover = VclPtr<InterimToolbarPopup>::Create(getFrameInterface(), pParent,
        std::make_unique<sd::LayoutToolbarMenu>(this, pParent->GetFrameWeld(), mbInsertPage, m_aCommandURL));

    mxInterimPopover->Show();

    return mxInterimPopover;
}

// XServiceInfo

OUString SAL_CALL SlideLayoutController::getImplementationName()
{
    if( mbInsertPage )
        return InsertSlideController_getImplementationName();
    else
        return SlideLayoutController_getImplementationName();
}

Sequence< OUString > SAL_CALL SlideLayoutController::getSupportedServiceNames(  )
{
    if( mbInsertPage )
        return InsertSlideController_getSupportedServiceNames();
    else
        return SlideLayoutController_getSupportedServiceNames();
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_sd_SlideLayoutController_get_implementation(css::uno::XComponentContext* context,
                                                              css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sd::SlideLayoutController(context, false));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_sd_InsertSlideController_get_implementation(css::uno::XComponentContext* context,
                                                              css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sd::SlideLayoutController(context, true));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

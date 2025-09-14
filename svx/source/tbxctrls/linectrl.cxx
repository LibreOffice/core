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

#include <tools/debug.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/virdev.hxx>
#include <sfx2/objsh.hxx>

#include <svtools/toolbarmenu.hxx>
#include <svtools/popupwindowcontroller.hxx>
#include <svtools/valueset.hxx>

#include <svx/strings.hrc>
#include <svx/svxids.hrc>
#include <helpids.h>

#include <svx/drawitem.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlncapit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xtable.hxx>
#include <svx/linectrl.hxx>
#include <svx/itemwin.hxx>
#include <svx/dialmgr.hxx>
#include <tbxcolorupdate.hxx>

#include <memory>

#include <comphelper/lok.hxx>
#include <comphelper/propertyvalue.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star;

// For End Line Controller
#define MAX_LINES 12

SvxLineStyleToolBoxControl::SvxLineStyleToolBoxControl( const css::uno::Reference<css::uno::XComponentContext>& rContext )
    : svt::PopupWindowController( rContext, nullptr, OUString() )
{
    addStatusListener(u".uno:LineDash"_ustr);
}

SvxLineStyleToolBoxControl::~SvxLineStyleToolBoxControl()
{
}

void SAL_CALL SvxLineStyleToolBoxControl::statusChanged( const frame::FeatureStateEvent& rEvent )
{
    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if (!getToolboxId(nId, &pToolBox) && !m_pToolbar)
        return;

    if ( rEvent.FeatureURL.Complete == m_aCommandURL )
    {
        if (m_pToolbar)
            m_pToolbar->set_item_sensitive(m_aCommandURL, rEvent.IsEnabled);
        else
            pToolBox->EnableItem( nId, rEvent.IsEnabled );
    }

    m_xBtnUpdater->Update(rEvent);

    SfxObjectShell* pSh = SfxObjectShell::Current();
    if (!pSh)
        return;

    const SvxDashListItem* pItem = pSh->GetItem( SID_DASH_LIST );
    if (!pItem)
        return;

    XDashListRef xList = pItem->GetDashList();
    int nIndex = m_xBtnUpdater->GetStyleIndex();
    bool bNoneLineStyle = false;
    switch (nIndex)
    {
        case -1:
        case 0:
        {
            BitmapEx aEmpty(xList->GetBitmapForUISolidLine());
            aEmpty.Erase(Application::GetSettings().GetStyleSettings().GetFieldColor());
            if (m_pToolbar)
            {
                Graphic aGraf(aEmpty);
                m_pToolbar->set_item_image(m_aCommandURL, aGraf.GetXGraphic());
            }
            else
                pToolBox->SetItemImage(nId, Image(aEmpty));
            bNoneLineStyle = true;
            break;
        }
        case 1:
            if (m_pToolbar)
            {
                Graphic aGraf(xList->GetBitmapForUISolidLine());
                m_pToolbar->set_item_image(m_aCommandURL, aGraf.GetXGraphic());
            }
            else
                pToolBox->SetItemImage(nId, Image(xList->GetBitmapForUISolidLine()));
            break;
        default:
            if (m_pToolbar)
            {
                Graphic aGraf(xList->GetUiBitmap(nIndex - 2));
                m_pToolbar->set_item_image(m_aCommandURL, aGraf.GetXGraphic());
            }
            else
                pToolBox->SetItemImage(nId, Image(xList->GetUiBitmap(nIndex - 2)));
            break;
    }
    if (m_aLineStyleIsNoneFunction)
        m_aLineStyleIsNoneFunction(bNoneLineStyle);
}

void SAL_CALL SvxLineStyleToolBoxControl::execute(sal_Int16 /*KeyModifier*/)
{
    if (m_pToolbar)
    {
        // Toggle the popup also when toolbutton is activated
        m_pToolbar->set_menu_item_active(m_aCommandURL, !m_pToolbar->get_menu_item_active(m_aCommandURL));
    }
    else
    {
        // Open the popup also when Enter key is pressed.
        createPopupWindow();
    }
}

void SvxLineStyleToolBoxControl::initialize( const css::uno::Sequence<css::uno::Any>& rArguments )
{
    svt::PopupWindowController::initialize( rArguments );

    if (m_pToolbar)
    {
        mxPopoverContainer.reset(new ToolbarPopupContainer(m_pToolbar));
        m_pToolbar->set_item_popover(m_aCommandURL, mxPopoverContainer->getTopLevel());
    }

    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if ( getToolboxId( nId, &pToolBox ) )
    {
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWNONLY );
    }

    m_xBtnUpdater.reset(new svx::ToolboxButtonLineStyleUpdater);
}

void SvxLineStyleToolBoxControl::setLineStyleSelectFunction(const LineStyleSelectFunction& rLineStyleSelectFunction)
{
    m_aLineStyleSelectFunction = rLineStyleSelectFunction;
}

void SvxLineStyleToolBoxControl::setLineStyleIsNoneFunction(const LineStyleIsNoneFunction& rLineStyleIsNoneFunction)
{
    m_aLineStyleIsNoneFunction = rLineStyleIsNoneFunction;
}

void SvxLineStyleToolBoxControl::dispatchLineStyleCommand(const OUString& rCommand, const Sequence<PropertyValue>& rArgs)
{
    if (m_aLineStyleSelectFunction && m_aLineStyleSelectFunction(rCommand, rArgs[0].Value))
        return;

    dispatchCommand(rCommand, rArgs);
}

std::unique_ptr<WeldToolbarPopup> SvxLineStyleToolBoxControl::weldPopupWindow()
{
    return std::make_unique<SvxLineBox>(this, m_pToolbar, m_xBtnUpdater->GetStyleIndex());
}

VclPtr<vcl::Window> SvxLineStyleToolBoxControl::createVclPopupWindow( vcl::Window* pParent )
{
    mxInterimPopover = VclPtr<InterimToolbarPopup>::Create(getFrameInterface(), pParent,
        std::make_unique<SvxLineBox>(this, pParent->GetFrameWeld(), m_xBtnUpdater->GetStyleIndex()));

    mxInterimPopover->Show();

    return mxInterimPopover;
}

OUString SvxLineStyleToolBoxControl::getImplementationName()
{
    return u"com.sun.star.comp.svx.LineStyleToolBoxControl"_ustr;
}

css::uno::Sequence<OUString> SvxLineStyleToolBoxControl::getSupportedServiceNames()
{
    return { u"com.sun.star.frame.ToolbarController"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_svx_LineStyleToolBoxControl_get_implementation(
    css::uno::XComponentContext* rContext,
    css::uno::Sequence<css::uno::Any> const & )
{
    return cppu::acquire( new SvxLineStyleToolBoxControl( rContext ) );
}

namespace {

class SvxLineEndToolBoxControl final : public svt::PopupWindowController
{
public:
    explicit SvxLineEndToolBoxControl( const css::uno::Reference<css::uno::XComponentContext>& rContext );

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence<css::uno::Any>& rArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    virtual void SAL_CALL execute(sal_Int16 nKeyModifier) override;

private:
    virtual std::unique_ptr<WeldToolbarPopup> weldPopupWindow() override;
    virtual VclPtr<vcl::Window> createVclPopupWindow( vcl::Window* pParent ) override;
};

class SvxLineEndWindow final : public WeldToolbarPopup
{
private:
    XLineEndListRef mpLineEndList;
    rtl::Reference<SvxLineEndToolBoxControl> mxControl;
    std::unique_ptr<weld::IconView> mxLineEndIV;
    Size maBmpSize;

    DECL_LINK( ItemActivatedHdl, weld::IconView&, bool );
    DECL_LINK(QueryTooltipHdl, const weld::TreeIter&, OUString);
    void FillIconView();

    virtual void GrabFocus() override
    {
        mxLineEndIV->grab_focus();
    }

public:
    SvxLineEndWindow(SvxLineEndToolBoxControl* pControl, weld::Widget* pParent);
    virtual void statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;
};

}

SvxLineEndWindow::SvxLineEndWindow(SvxLineEndToolBoxControl* pControl, weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, u"svx/ui/floatinglineend.ui"_ustr, u"FloatingLineEnd"_ustr)
    , mxControl(pControl)
    , mxLineEndIV(m_xBuilder->weld_icon_view(u"floating_line_end_iconview"_ustr))
{
    mxLineEndIV->set_help_id(HID_POPUP_LINEEND_CTRL);
    m_xTopLevel->set_help_id(HID_POPUP_LINEEND);

    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    if ( pDocSh )
    {
        const SfxPoolItem*  pItem = pDocSh->GetItem( SID_LINEEND_LIST );
        if( pItem )
            mpLineEndList = static_cast<const SvxLineEndListItem*>( pItem )->GetLineEndList();
    }
    DBG_ASSERT( mpLineEndList.is(), "LineEndList not found" );

    mxLineEndIV->connect_item_activated( LINK( this, SvxLineEndWindow, ItemActivatedHdl ) );

    // Avoid LibreOffice Kit crash: tooltip handlers cause segfault during JSDialog
    // serialization when popup widgets are destroyed/recreated during character formatting resets.
    // Tooltip event binding is not needed for LibreOffice Kit
    if (!comphelper::LibreOfficeKit::isActive())
    {
        mxLineEndIV->connect_query_tooltip(LINK(this, SvxLineEndWindow, QueryTooltipHdl));
    }

    // IconView fill with entries of LineEndList
    FillIconView();

    AddStatusListener( u".uno:LineEndListState"_ustr);
}

IMPL_LINK_NOARG(SvxLineEndWindow, ItemActivatedHdl, weld::IconView&, bool)
{
    std::unique_ptr<XLineEndItem> pLineEndItem;
    std::unique_ptr<XLineStartItem> pLineStartItem;

    OUString sId = mxLineEndIV->get_selected_id();
    if (sId.isEmpty())
        return false;

    sal_uInt32 nId = sId.toUInt32();

    if( nId == 1 )
    {
        pLineStartItem.reset(new XLineStartItem());
    }
    else if( nId == 2 )
    {
        pLineEndItem.reset(new XLineEndItem());
    }
    else if( nId % 2 ) // beginning of line
    {
        const XLineEndEntry* pEntry = mpLineEndList->GetLineEnd( (nId - 1) / 2 - 1 );
        pLineStartItem.reset(new XLineStartItem(pEntry->GetName(), pEntry->GetLineEnd()));
    }
    else // end of line
    {
        const XLineEndEntry* pEntry = mpLineEndList->GetLineEnd( nId / 2 - 2 );
        pLineEndItem.reset(new XLineEndItem(pEntry->GetName(), pEntry->GetLineEnd()));
    }

    OUString name;
    Any a;

    if ( pLineStartItem )
    {
        name = "LineStart";
        pLineStartItem->QueryValue( a );
    }
    else
    {
        name = "LineEnd";
        pLineEndItem->QueryValue( a );
    }
    Sequence< PropertyValue > aArgs{ comphelper::makePropertyValue(name, a) };

    /*  #i33380# DR 2004-09-03 Moved the following line above the Dispatch() call.
        This instance may be deleted in the meantime (i.e. when a dialog is opened
        while in Dispatch()), accessing members will crash in this case. */
    mxLineEndIV->unselect_all();

    mxControl->dispatchCommand(mxControl->getCommandURL(), aArgs);

    mxControl->EndPopupMode();

    return true;
}

IMPL_LINK(SvxLineEndWindow, QueryTooltipHdl, const weld::TreeIter&, rIter, OUString)
{
    OUString sId = mxLineEndIV->get_id(rIter);
    if (sId.isEmpty())
        return OUString();

    sal_uInt16 nId = sId.toUInt32();

    if (nId == 1 || nId == 2)
    {
        return SvxResId(RID_SVXSTR_NONE);
    }

    if (!mpLineEndList.is())
        return OUString();

    tools::Long nEntryIndex;
    if (nId % 2) // beginning of line
    {
        nEntryIndex = (nId - 1) / 2 - 1;
    }
    else // end of line
    {
        nEntryIndex = nId / 2 - 2;
    }

    if (nEntryIndex >= 0 && nEntryIndex < mpLineEndList->Count())
    {
        const XLineEndEntry* pEntry = mpLineEndList->GetLineEnd(nEntryIndex);
        if (pEntry)
        {
            return pEntry->GetName();
        }
    }

    return OUString();
}

void SvxLineEndWindow::FillIconView()
{
    if( !mpLineEndList.is() )
        return;

    ScopedVclPtrInstance< VirtualDevice > pVD;

    tools::Long nCount = mpLineEndList->Count();

    // First entry: no line end.
    // An entry is temporarily added to get the UI bitmap
    basegfx::B2DPolyPolygon aNothing;
    mpLineEndList->Insert(std::make_unique<XLineEndEntry>(aNothing,
        comphelper::LibreOfficeKit::isActive() ? SvxResId(RID_SVXSTR_INVISIBLE)
            : SvxResId(RID_SVXSTR_NONE)));
    const XLineEndEntry* pEntry = mpLineEndList->GetLineEnd(nCount);
    BitmapEx aBmp = mpLineEndList->GetUiBitmap( nCount );
    OSL_ENSURE( !aBmp.IsEmpty(), "UI bitmap was not created" );

    maBmpSize = aBmp.GetSizePixel();
    pVD->SetOutputSizePixel( maBmpSize, false );
    maBmpSize.setWidth( maBmpSize.Width() / 2 );
    Point aPt0( 0, 0 );
    Point aPt1( maBmpSize.Width(), 0 );

    pVD->DrawBitmapEx( Point(), aBmp );

    // First half (left side)
    ScopedVclPtrInstance< VirtualDevice > pVD1;
    pVD1->SetOutputSizePixel( maBmpSize, false );
    pVD1->DrawBitmapEx( Point(), pVD->GetBitmapEx(aPt0, maBmpSize) );
    mxLineEndIV->append(u"1"_ustr, pEntry->GetName(), pVD1.get());

    // Second half (right side)
    ScopedVclPtrInstance< VirtualDevice > pVD2;
    pVD2->SetOutputSizePixel( maBmpSize, false );
    pVD2->DrawBitmapEx( Point(), pVD->GetBitmapEx(aPt1, maBmpSize) );
    mxLineEndIV->append(u"2"_ustr, pEntry->GetName(), pVD2.get());

    mpLineEndList->Remove(nCount);

    for( tools::Long i = 0; i < nCount; i++ )
    {
        pEntry = mpLineEndList->GetLineEnd( i );
        assert(pEntry && "Could not access LineEndEntry");
        aBmp = mpLineEndList->GetUiBitmap( i );
        OSL_ENSURE( !aBmp.IsEmpty(), "UI bitmap was not created" );

        pVD->DrawBitmapEx( aPt0, aBmp );

        // Left half for line start
        ScopedVclPtrInstance< VirtualDevice > pVDStart;
        pVDStart->SetOutputSizePixel( maBmpSize, false );
        pVDStart->DrawBitmapEx( Point(), pVD->GetBitmapEx(aPt0, maBmpSize) );
        mxLineEndIV->append(OUString::number((i+1)*2L+1), pEntry->GetName(), pVDStart.get());

        // Right half for line end
        ScopedVclPtrInstance< VirtualDevice > pVDEnd;
        pVDEnd->SetOutputSizePixel( maBmpSize, false );
        pVDEnd->DrawBitmapEx( Point(), pVD->GetBitmapEx(aPt1, maBmpSize) );
        mxLineEndIV->append(OUString::number((i+2)*2L), pEntry->GetName(), pVDEnd.get());
    }
}

void SvxLineEndWindow::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    if ( rEvent.FeatureURL.Complete != ".uno:LineEndListState" )
        return;

    // The list of line ends (LineEndList) has changed
    css::uno::Reference< css::uno::XWeak > xWeak;
    if ( rEvent.State >>= xWeak )
    {
        mpLineEndList.set( static_cast< XLineEndList* >( xWeak.get() ) );
        DBG_ASSERT( mpLineEndList.is(), "LineEndList not found" );

        mxLineEndIV->clear();
        FillIconView();
    }
}

SvxLineEndToolBoxControl::SvxLineEndToolBoxControl( const css::uno::Reference<css::uno::XComponentContext>& rContext )
    : svt::PopupWindowController( rContext, nullptr, OUString() )
{
}

void SAL_CALL SvxLineEndToolBoxControl::execute(sal_Int16 /*KeyModifier*/)
{
    if (m_pToolbar)
    {
        // Toggle the popup also when toolbutton is activated
        m_pToolbar->set_menu_item_active(m_aCommandURL, !m_pToolbar->get_menu_item_active(m_aCommandURL));
    }
    else
    {
        // Open the popup also when Enter key is pressed.
        createPopupWindow();
    }
}

void SvxLineEndToolBoxControl::initialize( const css::uno::Sequence<css::uno::Any>& rArguments )
{
    svt::PopupWindowController::initialize( rArguments );

    if (m_pToolbar)
    {
        mxPopoverContainer.reset(new ToolbarPopupContainer(m_pToolbar));
        m_pToolbar->set_item_popover(m_aCommandURL, mxPopoverContainer->getTopLevel());
    }

    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if ( getToolboxId( nId, &pToolBox ) )
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWNONLY );
}

std::unique_ptr<WeldToolbarPopup> SvxLineEndToolBoxControl::weldPopupWindow()
{
    return std::make_unique<SvxLineEndWindow>(this, m_pToolbar);
}

VclPtr<vcl::Window> SvxLineEndToolBoxControl::createVclPopupWindow( vcl::Window* pParent )
{
    mxInterimPopover = VclPtr<InterimToolbarPopup>::Create(getFrameInterface(), pParent,
        std::make_unique<SvxLineEndWindow>(this, pParent->GetFrameWeld()));

    mxInterimPopover->Show();

    mxInterimPopover->SetText(SvxResId(RID_SVXSTR_LINEEND));

    return mxInterimPopover;
}

OUString SvxLineEndToolBoxControl::getImplementationName()
{
    return u"com.sun.star.comp.svx.LineEndToolBoxControl"_ustr;
}

css::uno::Sequence<OUString> SvxLineEndToolBoxControl::getSupportedServiceNames()
{
    return { u"com.sun.star.frame.ToolbarController"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_svx_LineEndToolBoxControl_get_implementation(
    css::uno::XComponentContext* rContext,
    css::uno::Sequence<css::uno::Any> const & )
{
    return cppu::acquire( new SvxLineEndToolBoxControl( rContext ) );
}

SvxLineBox::SvxLineBox(SvxLineStyleToolBoxControl* pControl, weld::Widget* pParent, int nInitialIndex)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, u"svx/ui/floatinglinestyle.ui"_ustr, u"FloatingLineStyle"_ustr)
    , mxControl(pControl)
    , mxLineStyleSet(new ValueSet(m_xBuilder->weld_scrolled_window(u"valuesetwin"_ustr, true)))
    , mxLineStyleSetWin(new weld::CustomWeld(*m_xBuilder, u"valueset"_ustr, *mxLineStyleSet))
{
    mxLineStyleSet->SetStyle(WB_FLATVALUESET | WB_ITEMBORDER | WB_3DLOOK | WB_NO_DIRECTSELECT);

    FillControl();

    mxLineStyleSet->SelectItem(nInitialIndex + 1);

    mxLineStyleSet->SetSelectHdl( LINK( this, SvxLineBox, SelectHdl ) );
}

void SvxLineBox::GrabFocus()
{
    mxLineStyleSet->GrabFocus();
}

SvxLineBox::~SvxLineBox()
{
}

// Fills the listbox (provisional) with strings

void SvxLineBox::Fill( const XDashListRef &pList )
{
    mxLineStyleSet->Clear();

    if( !pList.is() )
        return;

    // entry for 'none'
    mxLineStyleSet->InsertItem(1, Image(), pList->GetStringForUiNoLine());

    // entry for solid line
    const auto& rBmp = pList->GetBitmapForUISolidLine();
    Size aBmpSize = rBmp.GetSizePixel();
    mxLineStyleSet->InsertItem(2, Image(rBmp), pList->GetStringForUiSolidLine());

    // entries for dashed lines
    tools::Long nCount = pList->Count();
    for( tools::Long i = 0; i < nCount; i++ )
    {
        const XDashEntry* pEntry = pList->GetDash(i);
        const BitmapEx aBitmap = pList->GetUiBitmap(i);

        mxLineStyleSet->InsertItem(i + 3, Image(aBitmap), pEntry->GetName());
    }

    sal_uInt16 nLines = std::min( static_cast<sal_uInt16>(nCount + 2), sal_uInt16(MAX_LINES) );
    mxLineStyleSet->SetLineCount(nLines);

    WinBits nBits = mxLineStyleSet->GetStyle();
    if ( nLines == mxLineStyleSet->GetItemCount() )
        nBits &= ~WB_VSCROLL;
    else
        nBits |= WB_VSCROLL;
    mxLineStyleSet->SetStyle( nBits );

    Size aSize(aBmpSize);
    aSize.AdjustWidth(6);
    aSize.AdjustHeight(6);
    aSize = mxLineStyleSet->CalcWindowSizePixel(aSize);
    mxLineStyleSet->GetDrawingArea()->set_size_request(aSize.Width(), aSize.Height());
    mxLineStyleSet->SetOutputSizePixel(aSize);
}

IMPL_LINK_NOARG(SvxLineBox, SelectHdl, ValueSet*, void)
{
    drawing::LineStyle eXLS;
    sal_Int32 nPos = mxLineStyleSet->GetSelectedItemId();
    --nPos; // ids start at 1, get the pos of the id

    switch ( nPos )
    {
        case 0:
            eXLS = drawing::LineStyle_NONE;
            break;

        case 1:
            eXLS = drawing::LineStyle_SOLID;
            break;

        default:
        {
            eXLS = drawing::LineStyle_DASH;
            const SfxObjectShell* pObjSh = SfxObjectShell::Current();
            if (nPos != -1 && pObjSh && pObjSh->GetItem(SID_DASH_LIST))
            {
                // LineDashItem will only be sent if it also has a dash.
                // Notify cares!
                SvxDashListItem const * pItem = pObjSh->GetItem( SID_DASH_LIST );
                const XDashEntry* pEntry = pItem->GetDashList()->GetDash(nPos - 2);
                XLineDashItem aLineDashItem(pEntry->GetName(), pEntry->GetDash());

                Any a;
                aLineDashItem.QueryValue ( a );
                Sequence< PropertyValue > aArgs{ comphelper::makePropertyValue(u"LineDash"_ustr, a) };
                mxControl->dispatchLineStyleCommand(u".uno:LineDash"_ustr, aArgs);

                // set also cap style using the toolbar line style selection popup
                css::drawing::DashStyle eStyle = pEntry->GetDash().GetDashStyle();
                XLineCapItem aLineCapItem(
                    eStyle == drawing::DashStyle_RECT || eStyle == drawing::DashStyle_RECTRELATIVE
                                ? css::drawing::LineCap_BUTT
                                : css::drawing::LineCap_ROUND );
                aLineCapItem.QueryValue ( a );
                Sequence< PropertyValue > aArgs2{ comphelper::makePropertyValue(u"LineCap"_ustr, a) };
                mxControl->dispatchLineStyleCommand(u".uno:LineCap"_ustr, aArgs2);
            }
        }
        break;
    }

    XLineStyleItem aLineStyleItem( eXLS );
    Any a;
    aLineStyleItem.QueryValue ( a );
    Sequence< PropertyValue > aArgs{ comphelper::makePropertyValue(u"XLineStyle"_ustr, a) };
    mxControl->dispatchLineStyleCommand(u".uno:XLineStyle"_ustr, aArgs);

    mxControl->EndPopupMode();
}

void SvxLineBox::FillControl()
{
    SfxObjectShell* pSh = SfxObjectShell::Current();
    if (pSh)
    {
        const SvxDashListItem* pItem = pSh->GetItem( SID_DASH_LIST );
        if (pItem)
            Fill(pItem->GetDashList());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

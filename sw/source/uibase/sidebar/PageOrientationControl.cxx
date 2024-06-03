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

#include "PageOrientationControl.hxx"
#include "PageMarginControl.hxx"
#include <PageOrientationPopup.hxx>
#include <com/sun/star/document/XUndoManager.hpp>
#include <com/sun/star/document/XUndoManagerSupplier.hpp>
#include <com/sun/star/frame/XFrame.hpp>

#include <sfx2/viewsh.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <cmdid.h>

namespace {
    css::uno::Reference< css::document::XUndoManager > getUndoManager( const css::uno::Reference< css::frame::XFrame >& rxFrame )
    {
        const css::uno::Reference< css::frame::XController >& xController = rxFrame->getController();
        if ( xController.is() )
        {
            const css::uno::Reference< css::frame::XModel >& xModel = xController->getModel();
            if ( xModel.is() )
            {
                const css::uno::Reference< css::document::XUndoManagerSupplier > xSuppUndo( xModel, css::uno::UNO_QUERY_THROW );
                return css::uno::Reference< css::document::XUndoManager >( xSuppUndo->getUndoManager(), css::uno::UNO_SET_THROW );
            }
        }

        return css::uno::Reference< css::document::XUndoManager > ();
    }
}

namespace sw::sidebar {

PageOrientationControl::PageOrientationControl(PageOrientationPopup* pControl, weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, u"modules/swriter/ui/pageorientationcontrol.ui"_ustr, u"PageOrientationControl"_ustr)
    , m_xPortrait(m_xBuilder->weld_button(u"portrait"_ustr))
    , m_xLandscape(m_xBuilder->weld_button(u"landscape"_ustr))
    , m_xControl(pControl)
    , mpPageItem( new SvxPageItem(SID_ATTR_PAGE) )
    , mpPageSizeItem( new SvxSizeItem(SID_ATTR_PAGE_SIZE) )
    , mpPageLRMarginItem( new SvxLongLRSpaceItem( 0, 0, SID_ATTR_PAGE_LRSPACE ) )
    , mpPageULMarginItem( new SvxLongULSpaceItem( 0, 0, SID_ATTR_PAGE_ULSPACE ) )
{
    m_xPortrait->connect_clicked( LINK( this, PageOrientationControl,ImplOrientationHdl ) );
    m_xLandscape->connect_clicked( LINK( this, PageOrientationControl,ImplOrientationHdl ) );
}

void PageOrientationControl::GrabFocus()
{
    m_xPortrait->grab_focus();
}

PageOrientationControl::~PageOrientationControl()
{
}

void PageOrientationControl::ExecuteMarginLRChange(
    const tools::Long nPageLeftMargin,
    const tools::Long nPageRightMargin )
{
    mpPageLRMarginItem->SetLeft( nPageLeftMargin );
    mpPageLRMarginItem->SetRight( nPageRightMargin );
    if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
        pViewFrm->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_LRSPACE,
            SfxCallMode::RECORD, { mpPageLRMarginItem.get() });
}

void PageOrientationControl::ExecuteMarginULChange(
    const tools::Long nPageTopMargin,
    const tools::Long nPageBottomMargin )
{
    mpPageULMarginItem->SetUpper( nPageTopMargin );
    mpPageULMarginItem->SetLower( nPageBottomMargin );
    if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
        pViewFrm->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_ULSPACE,
            SfxCallMode::RECORD, { mpPageULMarginItem.get() });
}

void PageOrientationControl::ExecuteOrientationChange( const bool bLandscape )
{
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (!pViewFrm)
        return;

    css::uno::Reference< css::document::XUndoManager > mxUndoManager(
                getUndoManager( pViewFrm->GetFrame().GetFrameInterface() ) );

    if ( mxUndoManager.is() )
        mxUndoManager->enterUndoContext( u""_ustr );

    SfxPoolItemHolder aResult;
    pViewFrm->GetBindings().GetDispatcher()->QueryState(SID_ATTR_PAGE_SIZE, aResult);
    mpPageSizeItem.reset(static_cast<const SvxSizeItem*>(aResult.getItem())->Clone());

    // Prevent accidental toggling of page orientation
    if ((mpPageSizeItem->GetWidth() > mpPageSizeItem->GetHeight()) == bLandscape)
    {
        if ( mxUndoManager.is() )
            mxUndoManager->leaveUndoContext();
        return;
    }

    pViewFrm->GetBindings().GetDispatcher()->QueryState(SID_ATTR_PAGE_LRSPACE, aResult);
    mpPageLRMarginItem.reset(static_cast<const SvxLongLRSpaceItem*>(aResult.getItem())->Clone());

    pViewFrm->GetBindings().GetDispatcher()->QueryState(SID_ATTR_PAGE_ULSPACE, aResult);
    mpPageULMarginItem.reset(static_cast<const SvxLongULSpaceItem*>(aResult.getItem())->Clone());

    {
        // set new page orientation
        mpPageItem->SetLandscape( bLandscape );

        // swap the width and height of the page size
        const tools::Long nRotatedWidth = mpPageSizeItem->GetSize().Height();
        const tools::Long nRotatedHeight = mpPageSizeItem->GetSize().Width();
        mpPageSizeItem->SetSize(Size(nRotatedWidth, nRotatedHeight));

        // apply changed attributes
        pViewFrm->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_SIZE,
            SfxCallMode::RECORD, { mpPageSizeItem.get(), mpPageItem.get() });
    }

    // check, if margin values still fit to the changed page size.
    // if not, adjust margin values
    {
        const tools::Long nML = mpPageLRMarginItem->GetLeft();
        const tools::Long nMR = mpPageLRMarginItem->GetRight();
        const tools::Long nTmpPW = nML + nMR + MINBODY;

        const tools::Long nPW  = mpPageSizeItem->GetSize().Width();

        if ( nTmpPW > nPW )
        {
            if ( nML <= nMR )
            {
                ExecuteMarginLRChange( mpPageLRMarginItem->GetLeft(), nMR - (nTmpPW - nPW ) );
            }
            else
            {
                ExecuteMarginLRChange( nML - (nTmpPW - nPW ), mpPageLRMarginItem->GetRight() );
            }
        }

        const tools::Long nMT = mpPageULMarginItem->GetUpper();
        const tools::Long nMB = mpPageULMarginItem->GetLower();
        const tools::Long nTmpPH = nMT + nMB + MINBODY;

        const tools::Long nPH  = mpPageSizeItem->GetSize().Height();

        if ( nTmpPH > nPH )
        {
            if ( nMT <= nMB )
            {
                ExecuteMarginULChange( mpPageULMarginItem->GetUpper(), nMB - ( nTmpPH - nPH ) );
            }
            else
            {
                ExecuteMarginULChange( nMT - ( nTmpPH - nPH ), mpPageULMarginItem->GetLower() );
            }
        }
    }

    if ( mxUndoManager.is() )
        mxUndoManager->leaveUndoContext();
}

IMPL_LINK(PageOrientationControl, ImplOrientationHdl, weld::Button&, rControl, void)
{
    if (&rControl == m_xPortrait.get())
        ExecuteOrientationChange( false );
    else
        ExecuteOrientationChange( true );

    m_xControl->EndPopupMode();
}

} // end of namespace sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

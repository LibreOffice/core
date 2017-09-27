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
#include <com/sun/star/document/XUndoManager.hpp>
#include <com/sun/star/document/XUndoManagerSupplier.hpp>

#include <swtypes.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svxids.hrc>
#include <cmdid.h>

namespace {
    const css::uno::Reference< css::document::XUndoManager > getUndoManager( const css::uno::Reference< css::frame::XFrame >& rxFrame )
    {
        const css::uno::Reference< css::frame::XController >& xController = rxFrame->getController();
        if ( xController.is() )
        {
            const css::uno::Reference< css::frame::XModel >& xModel = xController->getModel();
            if ( xModel.is() )
            {
                const css::uno::Reference< css::document::XUndoManagerSupplier > xSuppUndo( xModel, css::uno::UNO_QUERY_THROW );
                if ( xSuppUndo.is() )
                {
                    const css::uno::Reference< css::document::XUndoManager > xUndoManager( xSuppUndo->getUndoManager(), css::uno::UNO_QUERY_THROW );
                    return xUndoManager;
                }
            }
        }

        return css::uno::Reference< css::document::XUndoManager > ();
    }
}

namespace sw { namespace sidebar {

PageOrientationControl::PageOrientationControl( sal_uInt16 nId, vcl::Window* pParent )
    : SfxPopupWindow( nId, pParent, "PageOrientationControl", "modules/swriter/ui/pageorientationcontrol.ui" )
    , mpPageItem( new SvxPageItem(SID_ATTR_PAGE) )
    , mpPageSizeItem( new SvxSizeItem(SID_ATTR_PAGE_SIZE) )
    , mpPageLRMarginItem( new SvxLongLRSpaceItem( 0, 0, SID_ATTR_PAGE_LRSPACE ) )
    , mpPageULMarginItem( new SvxLongULSpaceItem( 0, 0, SID_ATTR_PAGE_ULSPACE ) )
{
    get(m_pPortrait, "portrait");
    get(m_pLandscape, "landscape");

    m_pPortrait->SetClickHdl( LINK( this, PageOrientationControl,ImplOrientationHdl ) );
    m_pLandscape->SetClickHdl( LINK( this, PageOrientationControl,ImplOrientationHdl ) );
}

PageOrientationControl::~PageOrientationControl()
{
    disposeOnce();
}

void PageOrientationControl::dispose()
{
    m_pPortrait.disposeAndClear();
    m_pLandscape.disposeAndClear();

    mpPageItem.reset();
    mpPageLRMarginItem.reset();
    mpPageULMarginItem.reset();
    mpPageSizeItem.reset();

    SfxPopupWindow::dispose();
}

void PageOrientationControl::ExecuteMarginLRChange(
    const long nPageLeftMargin,
    const long nPageRightMargin )
{
    mpPageLRMarginItem->SetLeft( nPageLeftMargin );
    mpPageLRMarginItem->SetRight( nPageRightMargin );
    SfxViewShell::Current()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_LRSPACE,
            SfxCallMode::RECORD, { mpPageLRMarginItem.get() });
}

void PageOrientationControl::ExecuteMarginULChange(
    const long nPageTopMargin,
    const long nPageBottomMargin )
{
    mpPageULMarginItem->SetUpper( nPageTopMargin );
    mpPageULMarginItem->SetLower( nPageBottomMargin );
    SfxViewShell::Current()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_ULSPACE,
            SfxCallMode::RECORD, { mpPageULMarginItem.get() });
}

void PageOrientationControl::ExecuteOrientationChange( const bool bLandscape )
{
    css::uno::Reference< css::document::XUndoManager > mxUndoManager(
                getUndoManager( SfxViewFrame::Current()->GetFrame().GetFrameInterface() ) );

    if ( mxUndoManager.is() )
        mxUndoManager->enterUndoContext( "" );

    const SfxPoolItem* pItem;
    SfxViewFrame::Current()->GetBindings().GetDispatcher()->QueryState(SID_ATTR_PAGE_SIZE, pItem);
    SvxSizeItem* pSizeItem = static_cast<SvxSizeItem*>(pItem->Clone());
    if ( pSizeItem )
        mpPageSizeItem.reset( pSizeItem );

    SfxViewFrame::Current()->GetBindings().GetDispatcher()->QueryState(SID_ATTR_PAGE_LRSPACE, pItem);
    SvxLongLRSpaceItem* pLRItem = static_cast<SvxLongLRSpaceItem*>(pItem->Clone());
    if ( pLRItem )
        mpPageLRMarginItem.reset( pLRItem );

    SfxViewFrame::Current()->GetBindings().GetDispatcher()->QueryState(SID_ATTR_PAGE_ULSPACE, pItem);
    SvxLongULSpaceItem* pULItem = static_cast<SvxLongULSpaceItem*>(pItem->Clone());
    if ( pULItem )
        mpPageULMarginItem.reset( pULItem );

    {
        // set new page orientation
        mpPageItem->SetLandscape( bLandscape );

        // swap the width and height of the page size
        const long nRotatedWidth = mpPageSizeItem->GetSize().Height();
        const long nRotatedHeight = mpPageSizeItem->GetSize().Width();
        mpPageSizeItem->SetSize(Size(nRotatedWidth, nRotatedHeight));

        // apply changed attributes
        if (SfxViewShell::Current())
        {
            SfxViewShell::Current()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_SIZE,
                SfxCallMode::RECORD, { mpPageSizeItem.get(), mpPageItem.get() });
        }
    }

    // check, if margin values still fit to the changed page size.
    // if not, adjust margin values
    {
        const long nML = mpPageLRMarginItem->GetLeft();
        const long nMR = mpPageLRMarginItem->GetRight();
        const long nTmpPW = nML + nMR + MINBODY;

        const long nPW  = mpPageSizeItem->GetSize().Width();

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

        const long nMT = mpPageULMarginItem->GetUpper();
        const long nMB = mpPageULMarginItem->GetLower();
        const long nTmpPH = nMT + nMB + MINBODY;

        const long nPH  = mpPageSizeItem->GetSize().Height();

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

IMPL_LINK(PageOrientationControl, ImplOrientationHdl, Button*, pControl, void)
{
    if ( pControl == m_pPortrait.get() )
        ExecuteOrientationChange( false );
    else
        ExecuteOrientationChange( true );

    EndPopupMode();
}

} } // end of namespace sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

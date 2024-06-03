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

#include <memory>
#include "PageColumnControl.hxx"
#include <PageColumnPopup.hxx>

#include <cmdid.h>

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/intitem.hxx>
#include <svx/pageitem.hxx>

namespace sw::sidebar {

PageColumnControl::PageColumnControl(PageColumnPopup* pControl, weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, u"modules/swriter/ui/pagecolumncontrol.ui"_ustr, u"PageColumnControl"_ustr)
    , m_xMoreButton(m_xBuilder->weld_button(u"moreoptions"_ustr))
    , m_xControl(pControl)
{
    bool bLandscape = false;
    if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
    {
        SfxPoolItemHolder aResult;
        pViewFrm->GetBindings().GetDispatcher()->QueryState(SID_ATTR_PAGE, aResult);
        const SvxPageItem* pPageItem(static_cast<const SvxPageItem*>(aResult.getItem()));
        bLandscape = pPageItem->IsLandscape();
    }

    if ( bLandscape )
    {
        m_xOneColumn = m_xBuilder->weld_button(u"column1L"_ustr);
        m_xTwoColumns = m_xBuilder->weld_button(u"column2L"_ustr);
        m_xThreeColumns = m_xBuilder->weld_button(u"column3L"_ustr);
        m_xLeft = m_xBuilder->weld_button(u"columnleftL"_ustr);
        m_xRight = m_xBuilder->weld_button(u"columnrightL"_ustr);
    }
    else
    {
        m_xOneColumn = m_xBuilder->weld_button(u"column1"_ustr);
        m_xTwoColumns = m_xBuilder->weld_button( u"column2"_ustr);
        m_xThreeColumns = m_xBuilder->weld_button(u"column3"_ustr);
        m_xLeft = m_xBuilder->weld_button(u"columnleft"_ustr);
        m_xRight = m_xBuilder->weld_button(u"columnright"_ustr);
    }

    m_xOneColumn->show();
    m_xTwoColumns->show();
    m_xThreeColumns->show();
    m_xLeft->show();
    m_xRight->show();

    m_xOneColumn->connect_clicked( LINK( this, PageColumnControl, ColumnButtonClickHdl_Impl ) );
    m_xTwoColumns->connect_clicked( LINK( this, PageColumnControl, ColumnButtonClickHdl_Impl ) );
    m_xThreeColumns->connect_clicked( LINK( this, PageColumnControl, ColumnButtonClickHdl_Impl ) );
    m_xLeft->connect_clicked( LINK( this, PageColumnControl, ColumnButtonClickHdl_Impl ) );
    m_xRight->connect_clicked( LINK( this, PageColumnControl, ColumnButtonClickHdl_Impl ) );

    m_xMoreButton->connect_clicked( LINK( this, PageColumnControl, MoreButtonClickHdl_Impl ) );
}

void PageColumnControl::GrabFocus()
{
    m_xMoreButton->grab_focus();
}

PageColumnControl::~PageColumnControl()
{
}

void PageColumnControl::ExecuteColumnChange( const sal_uInt16 nColumnType )
{
    SfxInt16Item aPageColumnTypeItem(SID_ATTR_PAGE_COLUMN);
    aPageColumnTypeItem.SetValue( nColumnType );
    if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
        pViewFrm->GetBindings().GetDispatcher()->ExecuteList(SID_ATTR_PAGE_COLUMN,
            SfxCallMode::RECORD, { &aPageColumnTypeItem });
}

IMPL_LINK( PageColumnControl, ColumnButtonClickHdl_Impl, weld::Button&, rButton, void )
{
    if ( &rButton == m_xOneColumn.get() )
        ExecuteColumnChange( 1 );
    else if ( &rButton == m_xTwoColumns.get() )
        ExecuteColumnChange( 2 );
    else if ( &rButton == m_xThreeColumns.get() )
        ExecuteColumnChange( 3 );
    else if ( &rButton == m_xLeft.get() )
        ExecuteColumnChange( 4 );
    else if ( &rButton == m_xRight.get() )
        ExecuteColumnChange( 5 );

    m_xControl->EndPopupMode();
}

IMPL_LINK_NOARG( PageColumnControl, MoreButtonClickHdl_Impl, weld::Button&, void )
{
    if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
        pViewFrm->GetBindings().GetDispatcher()->Execute( FN_FORMAT_PAGE_COLUMN_DLG, SfxCallMode::ASYNCHRON );
    m_xControl->EndPopupMode();
}

} // end of namespace sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <cmdid.h>
#include <swtypes.hxx>

#include <svx/sidebar/ValueSetWithTextControl.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/settings.hxx>
#include <svx/svxids.hrc>

namespace sw { namespace sidebar {

PageColumnControl::PageColumnControl( sal_uInt16 nId, vcl::Window* pParent )
    : SfxPopupWindow( nId, pParent, "PageColumnControl", "modules/swriter/ui/pagecolumncontrol.ui" )
{
    get( m_pMoreButton, "moreoptions" );

    bool bLandscape = false;
    const SfxPoolItem *pItem;
    if ( SfxViewFrame::Current() )
    {
        SfxViewFrame::Current()->GetBindings().GetDispatcher()->QueryState( SID_ATTR_PAGE, pItem );
        bLandscape = static_cast<const SvxPageItem*>(pItem)->IsLandscape();
    }

    if ( bLandscape )
    {
        get(m_pOneColumn, "column1L");
        get(m_pTwoColumns, "column2L");
        get(m_pThreeColumns, "column3L");
        get(m_pLeft, "columnleftL");
        get(m_pRight, "columnrightL");
    }
    else
    {
        get(m_pOneColumn, "column1");
        get(m_pTwoColumns, "column2");
        get(m_pThreeColumns, "column3");
        get(m_pLeft, "columnleft");
        get(m_pRight, "columnright");
    }

    m_pOneColumn->Show();
    m_pTwoColumns->Show();
    m_pThreeColumns->Show();
    m_pLeft->Show();
    m_pRight->Show();

    m_pOneColumn->SetClickHdl( LINK( this, PageColumnControl, ColumnButtonClickHdl_Impl ) );
    m_pTwoColumns->SetClickHdl( LINK( this, PageColumnControl, ColumnButtonClickHdl_Impl ) );
    m_pThreeColumns->SetClickHdl( LINK( this, PageColumnControl, ColumnButtonClickHdl_Impl ) );
    m_pLeft->SetClickHdl( LINK( this, PageColumnControl, ColumnButtonClickHdl_Impl ) );
    m_pRight->SetClickHdl( LINK( this, PageColumnControl, ColumnButtonClickHdl_Impl ) );

    m_pMoreButton->SetClickHdl( LINK( this, PageColumnControl, MoreButtonClickHdl_Impl ) );
    m_pMoreButton->GrabFocus();
}

PageColumnControl::~PageColumnControl()
{
    disposeOnce();
}

void PageColumnControl::dispose()
{
    m_pOneColumn.disposeAndClear();
    m_pTwoColumns.disposeAndClear();
    m_pThreeColumns.disposeAndClear();
    m_pLeft.disposeAndClear();
    m_pRight.disposeAndClear();
    m_pMoreButton.disposeAndClear();
    SfxPopupWindow::dispose();
}

void PageColumnControl::ExecuteColumnChange( const sal_uInt16 nColumnType )
{
    std::unique_ptr<SfxInt16Item> mpPageColumnTypeItem( new SfxInt16Item(SID_ATTR_PAGE_COLUMN) );
    mpPageColumnTypeItem->SetValue( nColumnType );
    if ( SfxViewFrame::Current() )
        SfxViewFrame::Current()->GetBindings().GetDispatcher()->ExecuteList(SID_ATTR_PAGE_COLUMN,
            SfxCallMode::RECORD, { mpPageColumnTypeItem.get() });
}

IMPL_LINK( PageColumnControl, ColumnButtonClickHdl_Impl, Button*, pButton, void )
{
    if ( pButton == m_pOneColumn.get() )
        ExecuteColumnChange( 1 );
    else if ( pButton == m_pTwoColumns.get() )
        ExecuteColumnChange( 2 );
    else if ( pButton == m_pThreeColumns.get() )
        ExecuteColumnChange( 3 );
    else if ( pButton == m_pLeft.get() )
        ExecuteColumnChange( 4 );
    else if ( pButton == m_pRight.get() )
        ExecuteColumnChange( 5 );

    EndPopupMode();
}

IMPL_LINK_NOARG( PageColumnControl, MoreButtonClickHdl_Impl, Button*, void )
{
    if ( SfxViewFrame::Current() )
        SfxViewFrame::Current()->GetBindings().GetDispatcher()->Execute( FN_FORMAT_PAGE_COLUMN_DLG, SfxCallMode::ASYNCHRON );
    EndPopupMode();
}

} } // end of namespace sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

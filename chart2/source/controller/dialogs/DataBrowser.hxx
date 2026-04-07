/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <vcl/weld.hxx>

#include <memory>
#include <vector>

namespace chart
{

class DataBrowserModel;
class NumberFormatterWrapper;
class ChartModel;

namespace impl
{
class SeriesHeader;
class SeriesHeaderEdit;
}

typedef std::pair<int, OUString> ColumnNamePair;

class DataBrowser
{
public:
    DataBrowser(weld::TreeView& rTreeView, weld::Box* pColumns, weld::Box* pColors);
    ~DataBrowser();

    void SetReadOnly( bool bNewState );
    bool IsReadOnly() const { return m_bIsReadOnly; }

    void SetDataFromModel( const rtl::Reference<::chart::ChartModel> & xChartDoc );

    // predicates to determine what actions are possible at the current cursor
    // position
    bool MayInsertRow() const;
    bool MayInsertColumn() const;
    bool MayDeleteRow() const;
    bool MayDeleteColumn() const;

    bool MayMoveUpRows() const;
    bool MayMoveDownRows() const;
    bool MayMoveRightColumns() const;
    bool MayMoveLeftColumns() const;

    // mutators mutating data
    void InsertRow();
    void InsertColumn();
    void InsertTextColumn();
    void RemoveRow();
    void RemoveColumn();

    void MoveUpRow();
    void MoveDownRow();
    void MoveLeftColumn();
    void MoveRightColumn();

    void SetCursorMovedHdl( const Link<DataBrowser*,void>& rLink );

    /// confirms all pending changes to be ready to be closed
    bool EndEditing();

    bool IsEnableItem() const { return m_bDataValid; }

    void GrabFocus();

private:
    rtl::Reference<::chart::ChartModel> m_xChartDoc;
    std::unique_ptr< DataBrowserModel > m_apDataBrowserModel;

    typedef std::vector< std::shared_ptr< impl::SeriesHeader > > tSeriesHeaderContainer;
    tSeriesHeaderContainer m_aSeriesHeaders;

    std::shared_ptr< NumberFormatterWrapper >  m_spNumberFormatterWrapper;

    weld::TreeView& m_rTreeView;
    weld::Box* m_pColumnsWin;
    weld::Box* m_pColorsWin;

    sal_Int32 m_nCurRow;
    sal_Int32 m_nCurCol;
    bool m_bIsReadOnly;
    bool m_bDataValid;

    Link<DataBrowser*,void> m_aCursorMovedHdlLink;

    void clearHeaders();
    void RenewTable();
    void ImplAdjustHeaderControls();

    OUString GetCellText( sal_Int32 nRow, sal_Int32 nCol ) const;
    bool SaveCellEdit( sal_Int32 nRow, sal_Int32 nCol, const OUString& rText );
    bool isDateTimeString( const OUString& aInputString, double& fOutDateTimeValue );

    sal_Int32 GetColumnCount() const;
    sal_Int32 GetRowCount() const;

    DECL_LINK( SeriesHeaderGotFocus, impl::SeriesHeaderEdit&, void );
    DECL_LINK( SeriesHeaderChanged,  impl::SeriesHeaderEdit&, void );
    DECL_LINK( HeaderNameChangedHdl, const ColumnNamePair&, void );
    DECL_LINK( EditingStartedHdl, const weld::TreeIter&, bool );
    DECL_LINK( EditingDoneHdl, const weld::TreeView::iter_string&, bool );
    DECL_LINK( SelectionChangedHdl, weld::TreeView&, void );

    DataBrowser( const DataBrowser & ) = delete;
};

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

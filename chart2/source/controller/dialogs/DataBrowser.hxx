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

#pragma once

#include <svtools/editbrowsebox.hxx>
#include <vcl/weld.hxx>

#include <memory>
#include <vector>

namespace com::sun::star {
    namespace awt {
        class XWindow;
    }
    namespace chart2 {
        class XChartDocument;
    }
}

namespace com::sun::star::uno { class XComponentContext; }

class OutputDevice;

namespace chart
{

class DataBrowserModel;
class NumberFormatterWrapper;

namespace impl
{
class SeriesHeader;
class SeriesHeaderEdit;
}

class DataBrowser : public ::svt::EditBrowseBox
{
protected:
    // EditBrowseBox overridables
    virtual void PaintCell( OutputDevice& rDev, const tools::Rectangle& rRect, sal_uInt16 nColumnId ) const override;
    virtual bool SeekRow( long nRow ) override;
    virtual bool IsTabAllowed( bool bForward ) const override;
    virtual ::svt::CellController* GetController( long nRow, sal_uInt16 nCol ) override;
    virtual void InitController( ::svt::CellControllerRef& rController, long nRow, sal_uInt16 nCol ) override;
    virtual bool SaveModified() override;
    virtual void CursorMoved() override;
    // called whenever the control of the current cell has been modified
    virtual void CellModified() override;
    virtual void ColumnResized( sal_uInt16 nColId ) override;
    virtual void EndScroll() override;
    virtual void MouseButtonDown( const BrowserMouseEvent& rEvt ) override;

public:
    DataBrowser(const css::uno::Reference<css::awt::XWindow> &rParent,
                weld::Container* pColumns, weld::Container* pColors);

    virtual ~DataBrowser() override;
    virtual void dispose() override;

    /** GetCellText returns the text at the given position
        @param  nRow
            the number of the row
        @param  nColId
            the ID of the column
        @return
            the text out of the cell
    */
    virtual OUString  GetCellText(long nRow, sal_uInt16 nColId) const override;

    /** returns the number in the given cell. If a cell is empty or contains a
        string, the result will be Nan
    */
    double GetCellNumber( long nRow, sal_uInt16 nColumnId ) const;

    bool isDateTimeString( const OUString& aInputString, double& fOutDateTimeValue );

    // Window
    virtual void Resize() override;

    void SetReadOnly( bool bNewState );
    bool IsReadOnly() const { return m_bIsReadOnly;}

    void SetDataFromModel( const css::uno::Reference< css::chart2::XChartDocument > & xChartDoc,
                           const css::uno::Reference< css::uno::XComponentContext > & xContext );

    // predicates to determine what actions are possible at the current cursor
    // position.  This depends on the implementation of the according mutators
    // below.  (They are used for enabling toolbar icons)
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

    using BrowseBox::RemoveColumn;
    using BrowseBox::MouseButtonDown;

    void MoveUpRow();
    void MoveDownRow();
    void MoveLeftColumn();
    void MoveRightColumn();

    void SetCursorMovedHdl( const Link<DataBrowser*,void>& rLink );

    /// confirms all pending changes to be ready to be closed
    bool EndEditing();

    bool CellContainsNumbers( sal_uInt16 nCol ) const;

    sal_uInt32 GetNumberFormatKey( sal_uInt16 nCol ) const;

    bool IsEnableItem() const { return m_bDataValid;}
    bool IsDataValid() const;
    void ShowWarningBox();
    bool ShowQueryBox();

    void RenewSeriesHeaders();

private:
    css::uno::Reference< css::chart2::XChartDocument > m_xChartDoc;
    std::unique_ptr< DataBrowserModel > m_apDataBrowserModel;

    typedef std::vector< std::shared_ptr< impl::SeriesHeader > > tSeriesHeaderContainer;
    tSeriesHeaderContainer m_aSeriesHeaders;

    std::shared_ptr< NumberFormatterWrapper >  m_spNumberFormatterWrapper;

    /// the row that is currently painted
    long                m_nSeekRow;
    bool                m_bIsReadOnly;
    bool                m_bDataValid;

    VclPtr<svt::FormattedControl> m_aNumberEditField;
    VclPtr<svt::EditControl>    m_aTextEditField;
    weld::Container*            m_pColumnsWin;
    weld::Container*            m_pColorsWin;

    /// note: m_aNumberEditField must precede this member!
    ::svt::CellControllerRef    m_rNumberEditController;
    /// note: m_aTextEditField must precede this member!
    ::svt::CellControllerRef    m_rTextEditController;

    Link<DataBrowser*,void>     m_aCursorMovedHdlLink;

    void clearHeaders();
    void RenewTable();
    void ImplAdjustHeaderControls();

    OUString GetColString( sal_Int32 nColumnId ) const;

    DECL_LINK( SeriesHeaderGotFocus, impl::SeriesHeaderEdit&, void );
    DECL_LINK( SeriesHeaderChanged,  impl::SeriesHeaderEdit&, void );

    DataBrowser( const DataBrowser & ) = delete;
};

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

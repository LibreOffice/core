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

#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_DATABROWSER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_DATABROWSER_HXX

#include <svtools/editbrowsebox.hxx>
#include <vcl/outdev.hxx>
#include <svtools/fmtfield.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <memory>
#include <vector>

namespace com { namespace sun { namespace star {
    namespace chart2 {
        class XChartDocument;
    }
}}}

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
    virtual void PaintCell( OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColumnId ) const SAL_OVERRIDE;
    virtual bool SeekRow( long nRow ) SAL_OVERRIDE;
    virtual bool IsTabAllowed( bool bForward ) const SAL_OVERRIDE;
    virtual ::svt::CellController* GetController( long nRow, sal_uInt16 nCol ) SAL_OVERRIDE;
    virtual void InitController( ::svt::CellControllerRef& rController, long nRow, sal_uInt16 nCol ) SAL_OVERRIDE;
    virtual bool SaveModified() SAL_OVERRIDE;
    virtual void CursorMoved() SAL_OVERRIDE;
    // called whenever the control of the current cell has been modified
    virtual void CellModified() SAL_OVERRIDE;
    virtual void ColumnResized( sal_uInt16 nColId ) SAL_OVERRIDE;
    virtual void EndScroll() SAL_OVERRIDE;
    virtual void MouseButtonDown( const BrowserMouseEvent& rEvt ) SAL_OVERRIDE;

    void SetDirty();

public:
    DataBrowser( vcl::Window* pParent, WinBits nStyle, bool bLiveUpdate );
    virtual ~DataBrowser();
    virtual void dispose() SAL_OVERRIDE;

    /** GetCellText returns the text at the given position
        @param  nRow
            the number of the row
        @param  nColId
            the ID of the column
        @return
            the text out of the cell
    */
    virtual OUString  GetCellText(long nRow, sal_uInt16 nColId) const SAL_OVERRIDE;

    /** returns the number in the given cell. If a cell is empty or contains a
        string, the result will be Nan
    */
    double GetCellNumber( long nRow, sal_uInt16 nColumnId ) const;

    bool isDateTimeString( const OUString& aInputString, double& fOutDateTimeValue );

    // Window
    virtual void Resize() SAL_OVERRIDE;

    /// @return old state
    bool SetReadOnly( bool bNewState );
    bool IsReadOnly() const { return m_bIsReadOnly;}

    /// reset the dirty status, if changes have been saved
    void SetClean();

    void SetDataFromModel( const ::com::sun::star::uno::Reference<
                               ::com::sun::star::chart2::XChartDocument > & xChartDoc,
                           const ::com::sun::star::uno::Reference<
                               ::com::sun::star::uno::XComponentContext > & xContext );

    // predicates to determine what actions are possible at the current cursor
    // position.  This depends on the implementation of the according mutators
    // below.  (They are used for enabling toolbar icons)
    bool MayInsertRow() const;
    bool MayInsertColumn() const;
    bool MayDeleteRow() const;
    bool MayDeleteColumn() const;

    bool MaySwapRows() const;
    bool MaySwapColumns() const;

    // mutators mutating data
    void InsertRow();
    void InsertColumn();
    void InsertTextColumn();
    void RemoveRow();
    void RemoveColumn();

    using BrowseBox::RemoveColumn;
    using BrowseBox::MouseButtonDown;

    void SwapRow();
    void SwapColumn();

    void SetCursorMovedHdl( const Link<DataBrowser*,void>& rLink );

    /// confirms all pending changes to be ready to be closed
    bool EndEditing();

    // calls the protected inline-function BrowseBox::GetFirstVisibleColNumber()
    sal_Int16 GetFirstVisibleColumNumber() const;

    bool CellContainsNumbers( sal_Int32 nRow, sal_uInt16 nCol ) const;

    sal_uInt32 GetNumberFormatKey( sal_Int32 nRow, sal_uInt16 nCol ) const;

    bool IsEnableItem() { return m_bDataValid;}
    bool IsDataValid();
    void ShowWarningBox();
    bool ShowQueryBox();

    void RenewSeriesHeaders();

private:
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > m_xChartDoc;
    std::unique_ptr< DataBrowserModel > m_apDataBrowserModel;

    typedef ::std::vector< std::shared_ptr< impl::SeriesHeader > > tSeriesHeaderContainer;
    tSeriesHeaderContainer m_aSeriesHeaders;

    std::shared_ptr< NumberFormatterWrapper >  m_spNumberFormatterWrapper;

    /// the row that is currently painted
    long                m_nSeekRow;
    bool                m_bIsReadOnly;
    bool                m_bIsDirty;
    bool                m_bLiveUpdate;
    bool                m_bDataValid;

    VclPtr<FormattedField>      m_aNumberEditField;
    VclPtr<Edit>                m_aTextEditField;

    /// note: m_aNumberEditField must precede this member!
    ::svt::CellControllerRef    m_rNumberEditController;
    /// note: m_aTextEditField must precede this member!
    ::svt::CellControllerRef    m_rTextEditController;

    Link<DataBrowser*,void>     m_aCursorMovedHdlLink;
    Link<DataBrowser*,void>     m_aCellModifiedLink;

    void clearHeaders();
    void RenewTable();
    void ImplAdjustHeaderControls();

    OUString GetColString( sal_Int32 nColumnId ) const;
    static OUString GetRowString( sal_Int32 nRow );

    DECL_LINK( SeriesHeaderGotFocus, impl::SeriesHeaderEdit* );
    DECL_LINK_TYPED( SeriesHeaderChanged,  impl::SeriesHeaderEdit*, void );

    DataBrowser( const DataBrowser & ) SAL_DELETED_FUNCTION;
};

} // namespace chart

#endif // INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_DATABROWSER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

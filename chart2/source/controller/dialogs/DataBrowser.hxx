/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataBrowser.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 15:40:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef CHART_DATA_BROWSER_HXX
#define CHART_DATA_BROWSER_HXX

#ifndef _SVTOOLS_EDITBROWSEBOX_HXX_
#include <svtools/editbrowsebox.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif
#ifndef _FMTFIELD_HXX_
#include <svtools/fmtfield.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#include <vector>
#include <memory>
#include <boost/shared_ptr.hpp>

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
    virtual void PaintCell( OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColumnId ) const;
    virtual sal_Bool SeekRow( long nRow );
    virtual sal_Bool IsTabAllowed( sal_Bool bForward ) const;
    virtual ::svt::CellController* GetController( long nRow, sal_uInt16 nCol );
    virtual void InitController( ::svt::CellControllerRef& rController, long nRow, sal_uInt16 nCol );
    virtual sal_Bool SaveModified();
    virtual void CursorMoved();
    // called whenever the control of the current cell has been modified
    virtual void CellModified();
    virtual void ColumnResized( USHORT nColId );
    virtual void EndScroll();
    virtual void MouseButtonDown( const BrowserMouseEvent& rEvt );

    void SetDirty();

public:
    DataBrowser( Window* pParent, const ResId & rId, bool bLiveUpdate );
    virtual ~DataBrowser();

    /** GetCellText returns the text at the given position
        @param  nRow
            the number of the row
        @param  nColId
            the ID of the column
        @return
            the text out of the cell
    */
    virtual String  GetCellText(long nRow, USHORT nColId) const;

    /** returns the number in the given cell. If a cell is empty or contains a
        string, the result will be Nan
    */
    double GetCellNumber( long nRow, USHORT nColumnId ) const;

    // Window
    virtual void Resize();

    /// @return old state
    bool SetReadOnly( bool bNewState );
    bool IsReadOnly() const;

    /// @return true, if data has been modified
    bool IsDirty() const;
    /// reset the dirty status, if changes have been saved
    void SetClean();

    void SetDataFromModel( const ::com::sun::star::uno::Reference<
                               ::com::sun::star::chart2::XChartDocument > & xChartDoc,
                           const ::com::sun::star::uno::Reference<
                               ::com::sun::star::uno::XComponentContext > & xContext );

//     void setNumberFormatter();

    // predicates to determine what actions are possible at the current cursor
    // position.  This depends on the implementation of the according mutators
    // below.  (They are used for enabling toolbar icons)
    bool MayInsertRow() const;
    bool MayInsertColumn() const;
    bool MayDeleteRow() const;
    bool MayDeleteColumn() const;

    bool MaySwapRows() const;
    bool MaySwapColumns() const;

//     bool MaySortRow() const;
//     bool MaySortColumn() const;

    // mutators mutating data
    void InsertRow();
    void InsertColumn();
    void RemoveRow();
    void RemoveColumn();

    using BrowseBox::RemoveColumn;
    using BrowseBox::MouseButtonDown;

    void SwapRow();
    void SwapColumn();

//  void QuickSortRow();
//  void QuickSortCol();

    // sorting the entire table
//  void QuickSortTableCols ();
//  void QuickSortTableRows ();

    void SetCursorMovedHdl( const Link& rLink );
    const Link& GetCursorMovedHdl() const;

    void SetCellModifiedHdl( const Link& rLink );

    /// confirms all pending changes to be ready to be closed
    bool EndEditing();

    // calls the protected inline-function BrowseBox::GetFirstVisibleColNumber()
    sal_Int16 GetFirstVisibleColumNumber() const;

    sal_Int32 GetTotalWidth() const;

    bool CellContainsNumbers( sal_Int32 nRow, sal_uInt16 nCol ) const;

    sal_uInt32 GetNumberFormatKey( sal_Int32 nRow, sal_uInt16 nCol ) const;

    bool IsEnableItem();
    bool IsDataValid();
    void ShowWarningBox();
    bool ShowQueryBox();

private:
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > m_xChartDoc;
    ::std::auto_ptr< DataBrowserModel > m_apDataBrowserModel;

    typedef ::std::vector< ::boost::shared_ptr< impl::SeriesHeader > > tSeriesHeaderContainer;
    tSeriesHeaderContainer m_aSeriesHeaders;

    ::boost::shared_ptr< NumberFormatterWrapper >  m_spNumberFormatterWrapper;

    /// the row that is currently painted
    long                m_nSeekRow;
    bool                m_bIsReadOnly;
    bool                m_bIsDirty;
    bool                m_bLiveUpdate;
    bool                m_bDataValid;

    FormattedField      m_aNumberEditField;
    Edit                m_aTextEditField;

    /// note: m_aNumberEditField must precede this member!
    ::svt::CellControllerRef m_rNumberEditController;
    /// note: m_aTextEditField must precede this member!
    ::svt::CellControllerRef m_rTextEditController;

    Link                m_aCursorMovedHdlLink;
    Link                m_aCellModifiedLink;

    void clearHeaders();
    void RenewTable();
    void ImplAdjustHeaderControls();

    String GetColString( sal_Int32 nColumnId ) const;
    String GetRowString( sal_Int32 nRow ) const;

    DECL_LINK( SeriesHeaderGotFocus, impl::SeriesHeaderEdit* );
    DECL_LINK( SeriesHeaderChanged,  impl::SeriesHeaderEdit* );

    /// not implemented: inhibit copy construction
    DataBrowser( const DataBrowser & );
};

} // namespace chart

#endif  // CHART_DATA_BROWSER_HXX
